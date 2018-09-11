///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"
#include "stb_vorbis.h"


namespace Audio
{
  //------------------------------------------------------------------------------------ Sound Asset

  //************************************************************************************************
  SoundAsset::SoundAsset(ExternalNodeInterface* externalInterface, const bool threaded) :
    ThreadedAsset(nullptr),
    Threaded(threaded), 
    ExternalData(externalInterface),
    mReferenceCount(0)
  {
    // If not threaded, add to the system's list
    if (!Threaded)
      gAudioSystem->AddAsset(this);
  }

  //************************************************************************************************
  SoundAsset::~SoundAsset()
  {
    if (!Threaded)
    {
      // Delete the threaded asset (not tracked anywhere else)
      if (ThreadedAsset)
        delete ThreadedAsset;
      
      // Remove from the system's list
      gAudioSystem->RemoveAsset(this);
    }
  }

  //************************************************************************************************
  void SoundAsset::SetExternalInterface(ExternalNodeInterface* extInt)
  {
    if (Threaded)
      return;

    ExternalData = extInt;

    // If interface is now null and is not referenced, can delete now
    if (!extInt && mReferenceCount == 0)
      delete this;
  }

  //************************************************************************************************
  bool SoundAsset::IsPlaying()
  {
    if (Threaded)
      return false;

    return mReferenceCount > 0;
  }

  //************************************************************************************************
  void SoundAsset::AddReference(unsigned instanceID)
  {
    if (Threaded)
      AddInstance(instanceID);
    else
      ++mReferenceCount;
    }

  //************************************************************************************************
  void SoundAsset::ReleaseReference(unsigned instanceID)
  {
    if (Threaded)
      RemoveInstance(instanceID);
    else
    {
      ErrorIf(mReferenceCount == 0, "Trying to release reference on an unreferenced sound asset");

    --mReferenceCount;

    if (mReferenceCount == 0)
    {
      // If there is no external interface, delete now
      if (!ExternalData)
        delete this;
      else
        ExternalData->SendAudioEvent(AudioEventTypes::AssetUnreferenced);
    }
  }
  }

  //----------------------------------------------------------------------- Decompressed Sound Asset

  //************************************************************************************************
  static void DecompressedDecodingCallback(DecodedPacket* packet, void* data)
  {
    ((DecompressedSoundAsset*)data)->DecodingCallback(packet);
  }

  //************************************************************************************************
  DecompressedSoundAsset::DecompressedSoundAsset(Zero::Status& status, const Zero::String& fileName,
    ExternalNodeInterface* externalInterface, const bool isThreaded) :
    SoundAsset(externalInterface, isThreaded),
    mFileLength(0.0f),
    mChannels(0),
    mFrameCount(0),
    mDecoder(nullptr),
    mSamplesAvailableShared(0)
  {
    if (!Threaded)
    {
      // If the threaded asset is created successfully, set the variables
      ThreadedAsset = new DecompressedSoundAsset(status, fileName, externalInterface, true);
      if (!status.Failed())
      {
        DecompressedSoundAsset* sibling = (DecompressedSoundAsset*)ThreadedAsset;
        mFileLength = sibling->mFileLength;
        mChannels = sibling->mChannels;
        mFrameCount = sibling->mFrameCount;
      }
    }
    else
    {
      // Remember that this constructor happens on the game thread

      // Create the decoder
      mDecoder = new DecompressedDecoder(status, fileName, DecompressedDecodingCallback, this);

      if (!status.Failed())
      {
        // Tell the decoder to decode the first packet
        mDecoder->DecodeNextSection();

        // Set the variables
        mFileLength = (float)mDecoder->mSamplesPerChannel / SystemSampleRate;
        mChannels = mDecoder->mChannels;
        mFrameCount = mDecoder->mSamplesPerChannel;
        
        // Set the sample buffer to the full size
        mSamples.Resize(mFrameCount * mChannels);
      }
    }
  }

  //************************************************************************************************
  void DecompressedSoundAsset::AppendSamples(BufferType* buffer, const unsigned frameIndex,
    unsigned samplesRequested, unsigned instanceID)
  {
    if (!Threaded)
      return;

    // Translate from frames to sample location
    unsigned sampleIndex = frameIndex * mChannels;

    // Keep the original size of the buffer
    unsigned originalBufferSize = buffer->Size();
    // Resize the buffer to hold the new samples
    buffer->Resize(originalBufferSize + samplesRequested);
    float* bufferStart = buffer->Data() + originalBufferSize;

    // Save number of available samples (can change on decoding thread)
    unsigned samplesAvailable = mSamplesAvailableShared;

    // Check if we have enough samples available
    if (sampleIndex + samplesRequested < samplesAvailable)
    {
      memcpy(bufferStart, mSamples.Data() + sampleIndex, sizeof(float) * samplesRequested);
    }
    // If not, copy what we can and set the rest to zero
    else
    {
      int samplesToCopy = (int)samplesAvailable - (int)sampleIndex;

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

  //************************************************************************************************
  unsigned DecompressedSoundAsset::GetNumberOfFrames()
  {
    return mFrameCount;
  }

  //************************************************************************************************
  unsigned DecompressedSoundAsset::GetChannels()
  {
    return mChannels;
  }

  //************************************************************************************************
  float DecompressedSoundAsset::GetLengthOfFile()
  {
    return mFileLength;
  }

  //************************************************************************************************
  void DecompressedSoundAsset::DecodingCallback(DecodedPacket* packet)
  {
    // At the end of a file the actual decoded samples could be smaller than the size of the array
    // on the DecodedPacket object, so make sure we don't go past the size of mSamples
    unsigned samplesCopied = Math::Min(packet->mSamples.Size(), mSamples.Size() - mSamplesAvailableShared);
    // Copy the decoded samples into the array
    memcpy(mSamples.Data() + mSamplesAvailableShared, packet->mSamples.Data(), 
      sizeof(float) * samplesCopied);
    // Change the samples available value
    mSamplesAvailableShared = mSamplesAvailableShared + samplesCopied;
  }

  //************************************************************************************************
  DecompressedSoundAsset::~DecompressedSoundAsset()
  {
    if (mDecoder)
      delete mDecoder;
  }

  //-------------------------------------------------------------------- Streaming Data Per Instance

  //************************************************************************************************
  static void StreamingDecodingCallback(DecodedPacket* packet, void* data)
  {
    ((StreamingDataPerInstance*)data)->DecodingCallback(packet);
  }

  //************************************************************************************************
  StreamingDataPerInstance::StreamingDataPerInstance(Zero::Status& status, Zero::File* inputFile,
      Zero::ThreadLock* lock, unsigned channels, unsigned frames, unsigned instanceID) :
    mPreviousSamples(0),
    mDecoder(status, inputFile, lock, channels, frames, StreamingDecodingCallback, this),
    mInstanceID(instanceID)
    {

    }

  //************************************************************************************************
  StreamingDataPerInstance::StreamingDataPerInstance(Zero::Status& status, byte* inputData,
      unsigned dataSize, unsigned channels, unsigned frames, unsigned instanceID) :
    mPreviousSamples(0),
    mDecoder(status, inputData, dataSize, channels, frames, StreamingDecodingCallback, this),
    mInstanceID(instanceID)
  {

  }

  //************************************************************************************************
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

  //************************************************************************************************
  void StreamingDataPerInstance::DecodingCallback(DecodedPacket* packet)
  {
    mDecodedPacketQueue.Write(*packet); 
  }

  //-------------------------------------------------------------------------- Streaming Sound Asset

  //************************************************************************************************
  StreamingSoundAsset::StreamingSoundAsset(Zero::Status& status, const Zero::String& fileName,
    FileLoadType::Enum loadType, ExternalNodeInterface* externalInterface, const bool isThreaded) :
    SoundAsset(externalInterface, isThreaded),
    mFileLength(0),
    mChannels(0),
    mFrameCount(0),
    mFileName(fileName)
  {
    if (!Threaded)
    {
      // If the threaded asset is created successfully, set the variables
      ThreadedAsset = new StreamingSoundAsset(status, fileName, loadType, externalInterface, true);
      if (!status.Failed())
      {
        StreamingSoundAsset* sibling = (StreamingSoundAsset*)ThreadedAsset;
        mFileLength = sibling->mFileLength;
        mChannels = sibling->mChannels;
        mFrameCount = sibling->mFrameCount;
      }
    }
      else
      {
      // Remember that this constructor happens on the game thread

      FileHeader header;
      unsigned fileSize = PacketDecoder::OpenAndReadHeader(status, fileName, &mInputFile, &header);
      if (fileSize == 0)
        return;

      if (loadType == FileLoadType::StreamedFromMemory)
      {
        // Create a buffer for the file data and read it in
        mInputFileData.Resize(fileSize);
        mInputFile.Read(status, mInputFileData.Data(), fileSize);
        // If the read failed, delete the buffer and return
        if (status.Failed())
        {
          mInputFileData.Clear();
          return;
      }
    }

      // Close the file (will be opened again if an instance is played)
      mInputFile.Close();

      mChannels = header.Channels;
      mFrameCount = header.SamplesPerChannel;
      mFileLength = (float)mFrameCount / (float)SystemSampleRate;
  }
  }

  //************************************************************************************************
  void StreamingSoundAsset::AppendSamples(BufferType* buffer, const unsigned frameIndex,
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

  //************************************************************************************************
  unsigned StreamingSoundAsset::GetNumberOfFrames()
  {
    return mFrameCount;
  }

  //************************************************************************************************
  bool StreamingSoundAsset::GetStreaming()
  {
    return true;
  }

  //************************************************************************************************
  void StreamingSoundAsset::ResetStreamingFile(unsigned instanceID)
  {
    StreamingDataPerInstance* data = GetInstanceData(instanceID);
    if (data)
    {
      data->Reset();
      data->mDecoder.DecodeNextSection();
    }
  }

  //************************************************************************************************
  unsigned StreamingSoundAsset::GetChannels()
    {
    return mChannels;
  }

  //************************************************************************************************
  float StreamingSoundAsset::GetLengthOfFile()
      {
    return mFileLength;
      }

  //************************************************************************************************
  void StreamingSoundAsset::AddInstance(unsigned instanceID)
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
        if (!mInputFile.IsOpen())
          return;
      }

      // Create the instance data for streaming from file
      data = new StreamingDataPerInstance(status, &mInputFile, &mLock, mChannels, mFrameCount, instanceID);
    }

    // If there was a problem creating the instance data, delete it
    if (status.Failed())
      delete data;
    // Make sure the data object was created before adding it to the list
    else if (data)
      mDataPerInstanceList.PushBack(data);
  }

  //************************************************************************************************
  void StreamingSoundAsset::RemoveInstance(unsigned instanceID)
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

  //************************************************************************************************
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

  //************************************************************************************************
  StreamingDataPerInstance* StreamingSoundAsset::GetInstanceData(unsigned instanceID)
  {
    forRange(StreamingDataPerInstance& data, mDataPerInstanceList.All())
    {
      if (data.mInstanceID == instanceID)
        return &data;
    }

    return nullptr;
  }

  //************************************************************************************************
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
    unsigned samplesCopied = Math::Min(samplesRequested, (unsigned)data->mSamples.Size() - sampleIndex);
    memcpy(outputBuffer, data->mSamples.Data() + sampleIndex, sizeof(float) * samplesCopied);

    // If we did not copy enough samples keep trying (in case there is another decoded buffer available)
    if (samplesCopied < samplesRequested)
      CopySamplesIntoBuffer(outputBuffer + samplesCopied, sampleIndex + samplesCopied,
        samplesRequested - samplesCopied, data);
  }

  //--------------------------------------------------------------------- Generated Wave Sound Asset

  //************************************************************************************************
  GeneratedWaveSoundAsset::GeneratedWaveSoundAsset(const OscillatorTypes::Enum waveType, 
      const float frequency, ExternalNodeInterface* extInt, const bool isThreaded) :
    SoundAsset(extInt, isThreaded),
    WaveData(nullptr),
    mFrequency(frequency)
  {
    if (!Threaded)
      ThreadedAsset = new GeneratedWaveSoundAsset(waveType, frequency, extInt, true);
    else
    {
      WaveData = new Oscillator();
      WaveData->SetType(waveType);
      WaveData->SetFrequency(frequency);
      WaveData->SetNoteOn(true);
    }
  }

  //************************************************************************************************
  GeneratedWaveSoundAsset::~GeneratedWaveSoundAsset()
  {
    if (Threaded)
    {
      delete WaveData;
    }
  }

  //************************************************************************************************
  void GeneratedWaveSoundAsset::AppendSamples(BufferType* buffer, const unsigned frameIndex,
    unsigned numberOfSamples, unsigned instanceID)
  {
    if (!Threaded)
      return;

    unsigned startingIndex = buffer->Size();
    buffer->Resize(startingIndex + numberOfSamples);

    for (unsigned i = startingIndex; i < startingIndex + numberOfSamples; ++i)
    {
      (*buffer)[i] = WaveData->GetNextSample() * GeneratedWaveVolume;

      if (!FrequencyInterpolator.Finished())
      {
        mFrequency = FrequencyInterpolator.NextValue();
        WaveData->SetFrequency(mFrequency);
      }
    }
  }

  //************************************************************************************************
  unsigned GeneratedWaveSoundAsset::GetNumberOfFrames()
  {
    return SystemSampleRate;
  }

  //************************************************************************************************
  float GeneratedWaveSoundAsset::GetLengthOfFile()
  {
    return 0.0f;
  }

  //************************************************************************************************
  float GeneratedWaveSoundAsset::GetFrequency()
  {
    return mFrequency;
  }

  //************************************************************************************************
  void GeneratedWaveSoundAsset::SetFrequency(const float newFrequency, const float time)
  {
    if (!Threaded)
    {
      mFrequency = newFrequency;
      if (ThreadedAsset)
        gAudioSystem->AddTask(Zero::CreateFunctor(&GeneratedWaveSoundAsset::SetFrequency,
          (GeneratedWaveSoundAsset*)ThreadedAsset, newFrequency, time));
    }
    else
    {
      if (time == 0)
      {
        mFrequency = newFrequency;
        WaveData->SetFrequency(mFrequency);
      }
      else
        FrequencyInterpolator.SetValues(mFrequency, newFrequency,
          (unsigned)(time * SystemSampleRate));
    }
  }

  //************************************************************************************************
  void GeneratedWaveSoundAsset::SetSquareWavePositiveFraction(float positiveFraction)
  {
    if (!Threaded)
    {
      if (ThreadedAsset)
        gAudioSystem->AddTask(Zero::CreateFunctor(&GeneratedWaveSoundAsset::SetSquareWavePositiveFraction,
          (GeneratedWaveSoundAsset*)ThreadedAsset, positiveFraction));
    }
    else
    {
      WaveData->SetSquareWavePositiveFraction(positiveFraction);
    }
  }

}
