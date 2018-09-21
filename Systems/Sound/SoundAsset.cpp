///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------------------------- Sound Asset

//**************************************************************************************************
ZilchDefineType(SoundAsset, builder, Type)
{

}

//**************************************************************************************************
SoundAsset::SoundAsset(const String& assetName, bool streaming) :
  mStreaming(streaming),
  mFileLength(0.0f),
  mChannels(0),
  mFrameCount(0),
  mName(assetName),
  mInstanceReferenceCount(0)
{
  
}

//**************************************************************************************************
SoundAsset::~SoundAsset()
{

}

//**************************************************************************************************
void SoundAsset::AddInstance(unsigned instanceID)
{
  ++mInstanceReferenceCount;
  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundAsset::OnAddInstanceThreaded, this, instanceID), nullptr);
}

//**************************************************************************************************
void SoundAsset::RemoveInstance(unsigned instanceID)
{
  ErrorIf(mInstanceReferenceCount == 0, "Trying to remove instance on an unreferenced sound asset");

  --mInstanceReferenceCount;
  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundAsset::OnRemoveInstanceThreaded, this, instanceID), nullptr);
}

//------------------------------------------------------------------------- Decompressed Sound Asset

//**************************************************************************************************
ZilchDefineType(DecompressedSoundAsset, builder, Type)
{

}

//**************************************************************************************************
static void DecompressedDecodingCallback(DecodedPacket* packet, void* data)
{
  ((DecompressedSoundAsset*)data)->DecodingCallback(packet);
}

//**************************************************************************************************
DecompressedSoundAsset::DecompressedSoundAsset(Status& status, const String& fileName, 
    const String& assetName) :
  SoundAsset(assetName, false),
  mDecoder(status, fileName, DecompressedDecodingCallback, (void*)this),
  mSamplesAvailableShared(0)
{
  // Check if the decoder was created successfully
  if (status.Succeeded())
  {
    // Tell the decoder to decode the first packet
    mDecoder.DecodeNextSection();

    // Set the variables
    mFileLength = (float)mDecoder.mSamplesPerChannel / AudioConstants::cSystemSampleRate;
    mChannels = mDecoder.mChannels;
    mFrameCount = mDecoder.mSamplesPerChannel;

    // Set the sample buffer to the full size
    mSamples.Resize(mFrameCount * mChannels);
  }
}

//**************************************************************************************************
void DecompressedSoundAsset::AppendSamplesThreaded(BufferType* buffer, const unsigned frameIndex, 
  unsigned samplesRequested, unsigned instanceID)
{
  // Translate from frames to sample location
  unsigned sampleIndex = frameIndex * mChannels;

  // Keep the original size of the buffer
  unsigned originalBufferSize = buffer->Size();
  // Resize the buffer to hold the new samples
  buffer->Resize(originalBufferSize + samplesRequested);
  float* bufferStart = buffer->Data() + originalBufferSize;

  unsigned samplesAvailable = mSamplesAvailableShared;

  // Check if we have enough samples available
  if (sampleIndex + samplesRequested < samplesAvailable)
  {
    memcpy(bufferStart, mSamples.Data() + sampleIndex, sizeof(float) * samplesRequested);
  }
  // If not, copy what we can and set the rest to zero
  else
  {
    unsigned samplesToCopy = samplesAvailable - sampleIndex;

    // Check if there are any samples to copy 
    if (samplesToCopy > 0)
    {
      // Copy what we have into the buffer
      memcpy(bufferStart, mSamples.Data() + sampleIndex, sizeof(float) * samplesToCopy);
      // Set the remaining samples to zero
      memset(bufferStart + samplesToCopy, 0, sizeof(float) * (samplesRequested - samplesToCopy));
    }
    // If not, set all samples to zero
    else
      memset(bufferStart, 0, sizeof(float) * samplesRequested);
  }
}

//**************************************************************************************************
void DecompressedSoundAsset::DecodingCallback(DecodedPacket* packet)
{
  // At the end of a file the actual decoded samples could be smaller than the size of the array
  // on the DecodedPacket object, so make sure we don't go past the size of mSamples
  unsigned samplesCopied = Math::Min(packet->mSamples.Size(), mSamples.Size() - mSamplesAvailableShared);
  // Copy the decoded samples into the array
  memcpy(mSamples.Data() + mSamplesAvailableShared, packet->mSamples.Data(),
    sizeof(float) * samplesCopied);
  // Change the samples available value
  AtomicExchange(&mSamplesAvailableShared, mSamplesAvailableShared + samplesCopied);
}

//---------------------------------------------------------------------- Streaming Data Per Instance

//**************************************************************************************************
static void StreamingDecodingCallback(DecodedPacket* packet, void* data)
{
  ((StreamingDataPerInstance*)data)->DecodingCallback(packet);
}

//**************************************************************************************************
StreamingDataPerInstance::StreamingDataPerInstance(Status& status, File* inputFile, ThreadLock* lock, 
    unsigned channels, unsigned frames, unsigned instanceID) :
  mDecoder(status, inputFile, lock, channels, frames, StreamingDecodingCallback, this),
  mPreviousSamples(0),
  mInstanceID(instanceID)
{

}

//**************************************************************************************************
StreamingDataPerInstance::StreamingDataPerInstance(Status& status, byte* inputData, unsigned dataSize, 
    unsigned channels, unsigned frames, unsigned instanceID) :
  mDecoder(status, inputData, dataSize, channels, frames, StreamingDecodingCallback, this),
  mPreviousSamples(0),
  mInstanceID(instanceID)
{

}

//**************************************************************************************************
void StreamingDataPerInstance::Reset()
{
  mDecoder.Reset();
  mPreviousSamples = 0;

  // Clear any existing data from the decoded packet queue
  DecodedPacket packet;
  while (mDecodedPacketQueue.Read(packet))
  {

  }
}

//**************************************************************************************************
void StreamingDataPerInstance::DecodingCallback(DecodedPacket* packet)
{
  mDecodedPacketQueue.Write(*packet);
}

//---------------------------------------------------------------------------- Streaming Sound Asset

//**************************************************************************************************
ZilchDefineType(StreamingSoundAsset, builder, Type)
{

}

//**************************************************************************************************
StreamingSoundAsset::StreamingSoundAsset(Status& status, const String& fileName, 
    AudioFileLoadType::Enum loadType, const String& assetName) :
  SoundAsset(assetName, true),
  mFileName(fileName)
{
  FileHeader header;
  unsigned fileSize = PacketDecoder::OpenAndReadHeader(status, fileName, &mInputFile, &header);
  if (status.Failed())
    return;

  if (loadType == AudioFileLoadType::StreamFromMemory)
  {
    // Create a buffer for the file data and read it in
    mInputFileData.Resize(fileSize);
    mInputFile.Read(status, mInputFileData.Data(), fileSize);
    // If the read failed, delete the buffer and return
    if (status.Failed())
    {
      mInputFileData.Clear();
      mInputFile.Close();
      return;
    }
  }

  // Close the file (will be opened again if an instance is played)
  mInputFile.Close();

  // Set variables
  mChannels = header.Channels;
  mFrameCount = header.SamplesPerChannel;
  mFileLength = (float)mFrameCount / (float)AudioConstants::cSystemSampleRate;
}

//**************************************************************************************************
StreamingSoundAsset::~StreamingSoundAsset()
{
  // Delete any existing instance data objects (though this shouldn't happen normally since assets
  // shouldn't be deleted when there are any instances)
  while (!mDataPerInstanceList.Empty())
  {
    StreamingDataPerInstance* data = &mDataPerInstanceList.Front();
    mDataPerInstanceList.PopFront();
    delete data;
  }
}

//**************************************************************************************************
void StreamingSoundAsset::AppendSamplesThreaded(BufferType* buffer, const unsigned frameIndex, 
  unsigned samplesRequested, unsigned instanceID)
{
  // Keep the original size of the buffer
  unsigned originalBufferSize = buffer->Size();
  // Resize the buffer to hold the new samples
  buffer->Resize(originalBufferSize + samplesRequested);

  // Get the data for this instance
  StreamingDataPerInstance* data = GetInstanceData(instanceID);
  if (!data)
  {
    ErrorIf(true, "Instance data was not created on a streaming asset");

    // If it doesn't exist, set the requested samples to zero and return
    memset(buffer->Data() + originalBufferSize, 0, sizeof(float) * samplesRequested);
    return;
  }

  // Translate from frames to sample location
  unsigned sampleIndex = frameIndex * mChannels;
  // Adjust the sample index to be within the current buffer
  sampleIndex -= data->mPreviousSamples;

  CopySamplesIntoBuffer(buffer->Data() + originalBufferSize, sampleIndex, samplesRequested, data);
}

//**************************************************************************************************
void StreamingSoundAsset::ResetStreamingFile(unsigned instanceID)
{
  StreamingDataPerInstance* data = GetInstanceData(instanceID);
  if (data)
  {
    data->Reset();
    data->mDecoder.DecodeNextSection();
  }
}

//**************************************************************************************************
void StreamingSoundAsset::OnAddInstanceThreaded(unsigned instanceID)
{
  Zero::Status status;
  StreamingDataPerInstance* data = nullptr;

  // If there is data in the buffer, create the instance data for streaming from memory
  if (!mInputFileData.Empty())
    data = new StreamingDataPerInstance(status, mInputFileData.Data(), mInputFileData.Size(), mChannels,
      mFrameCount, instanceID);
  // Otherwise, check if the file name is set
  else if (!mFileName.Empty())
  {
    // If the input file is not open (because there are no current instances) open it
    if (!mInputFile.IsOpen())
    {
      mInputFile.Open(mFileName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);
      ErrorIf(!mInputFile.IsOpen(), "Could not open streaming audio file to play a new instance");
      if (!mInputFile.IsOpen())
        return;
    }

    // Create the instance data for streaming from file
    data = new StreamingDataPerInstance(status, &mInputFile, &mLock, mChannels, mFrameCount, instanceID);
  }
  
  ErrorIf(!data, "No data or file name to play streaming audio asset");

  // If there was a problem creating the instance data, delete it
  if (status.Failed())
    delete data;
  // Make sure the data object was created before adding it to the list
  else if (data)
    mDataPerInstanceList.PushBack(data);
}

//**************************************************************************************************
void StreamingSoundAsset::OnRemoveInstanceThreaded(unsigned instanceID)
{
  // Look for the data for this instance ID
  StreamingDataPerInstance* data = GetInstanceData(instanceID);
  if (data)
  {
    // Remove it from the list and delete it
    mDataPerInstanceList.Erase(data);
    delete data;

    // If there are no current instances playing, close the input file
    if (mDataPerInstanceList.Empty())
      mInputFile.Close();
  }
}

//**************************************************************************************************
Zero::StreamingDataPerInstance* StreamingSoundAsset::GetInstanceData(unsigned instanceID)
{
  forRange(StreamingDataPerInstance& data, mDataPerInstanceList.All())
  {
    if (data.mInstanceID == instanceID)
      return &data;
  }

  return nullptr;
}

//**************************************************************************************************
void StreamingSoundAsset::CopySamplesIntoBuffer(float* outputBuffer, unsigned sampleIndex, 
  unsigned samplesRequested, StreamingDataPerInstance* data)
{
  // Check if the requested index is past the end of the Samples buffer
  while (sampleIndex >= data->mSamples.Size())
  {
    DecodedPacket packet;
    // If there are no packets available, set the buffer to zero and return
    if (!data->mDecodedPacketQueue.Read(packet))
    {
      // Trigger another decoded buffer
      data->mDecoder.DecodeNextSection();

      memset(outputBuffer, 0, sizeof(float) * samplesRequested);
      return;
    }

    // Increase the previous samples variable
    data->mPreviousSamples += data->mSamples.Size();
    // Adjust the sampleIndex
    sampleIndex -= data->mSamples.Size();
    // Remove the old samples
    data->mSamples.Clear();
    // Move the decoded data into the Samples buffer
    data->mSamples.Swap(packet.mSamples);

    // Trigger another decoded buffer
    data->mDecoder.DecodeNextSection();
  }

  // Copy either the number of samples requested or the samples available, whichever is smaller
  unsigned samplesCopied = Math::Min(samplesRequested, data->mSamples.Size() - sampleIndex);
  memcpy(outputBuffer, data->mSamples.Data() + sampleIndex, sizeof(float) * samplesCopied);

  // If we did not copy enough samples, keep trying (in case there is another decoded buffer available)
  if (samplesCopied < samplesRequested)
    CopySamplesIntoBuffer(outputBuffer + samplesCopied, sampleIndex + samplesCopied,
      samplesRequested - samplesCopied, data);
}

} // namespace Zero
