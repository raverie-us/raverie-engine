///////////////////////////////////////////////////////////////////////////////
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"
#include "opus.h"

namespace Zero
{

using namespace AudioConstants;

//--------------------------------------------------------------------------------- Decoded Packet

//************************************************************************************************
DecodedPacket::DecodedPacket(unsigned bufferSize) :
  mSamples(bufferSize)
{

}

//************************************************************************************************
DecodedPacket::DecodedPacket(const DecodedPacket& copy) :
  mSamples(MoveReference<Array<float>>(const_cast<DecodedPacket&>(copy).mSamples))
{

}

//************************************************************************************************
DecodedPacket& DecodedPacket::operator=(const DecodedPacket& other)
{
  mSamples = MoveReference<Array<float>>(const_cast<DecodedPacket&>(other).mSamples);
  return *this;
}

//--------------------------------------------------------------------------------- Packet Decoder

//************************************************************************************************
SingleChannelPacketDecoder::~SingleChannelPacketDecoder()
{
  if (mDecoder)
    opus_decoder_destroy(mDecoder);
}

//************************************************************************************************
void SingleChannelPacketDecoder::InitializeDecoder()
{
  if (mDecoder)
    opus_decoder_destroy(mDecoder);

  int error;
  mDecoder = opus_decoder_create(cSystemSampleRate, PacketEncoder::cChannels, &error);
}

//************************************************************************************************
void SingleChannelPacketDecoder::DecodePacket(const byte* packetData, const unsigned dataSize,
  float*& decodedData, unsigned& numberOfSamples)
{
  ReturnIf(!mDecoder, , "Tried to decode packet without initializing decoder");

  numberOfSamples = PacketDecoder::DecodePacket(packetData, dataSize, mDecoder, &decodedData);
}

//--------------------------------------------------------------------------------- Packet Decoder

//************************************************************************************************
int PacketDecoder::DecodePacket(const byte* packetData, unsigned dataSize, OpusDecoder* decoder,
  float** decodedData)
{
  *decodedData = new float[AudioFileEncoder::cPacketFrames];
  return opus_decode_float(decoder, packetData, dataSize, *decodedData, AudioFileEncoder::cPacketFrames, 0);
}

//************************************************************************************************
int PacketDecoder::DecodePacket(const byte* packetData, unsigned dataSize, OpusDecoder* decoder,
  float* decodedData)
{
  return opus_decode_float(decoder, packetData, dataSize, decodedData, AudioFileEncoder::cPacketFrames, 0);
}

//************************************************************************************************
int PacketDecoder::GetPacketDataSize(const byte* packetHeader)
{
  // Read in the packet header from the buffer
  PacketHeader packHead;
  memcpy(&packHead, packetHeader, sizeof(PacketHeader));

  // Make sure it's valid before checking the size
  if (packHead.Name[0] != 'p' || packHead.Name[1] != 'a')
    return -1;
  else
    return (int)packHead.Size;
}

//************************************************************************************************
unsigned PacketDecoder::OpenAndReadHeader(Status& status, const String& fileName,
  File* file, FileHeader* header)
{
  // Open the file
  file->Open(fileName, FileMode::Read, FileAccessPattern::Sequential);
  if (!file->IsOpen())
  {
    status.SetFailed(String::Format("Unable to open audio file %s", fileName.c_str()));
    return 0;
  }

  // Get the size of the file
  long long size = file->CurrentFileSize();
  // Check for an invalid size
  if (size < sizeof(FileHeader))
  {
    status.SetFailed(String::Format("Unable to read audio file %s", fileName.c_str()));
    return 0;
  }

  // Read the file header
  file->Read(status, (byte*)header, sizeof(FileHeader));
  // If the read failed, set the status message and return
  if (status.Failed())
  {
    status.SetFailed(String::Format("Unable to read from audio file %s", fileName.c_str()));
    return 0;
  }

  // If this isn't the right type of file, set the failed message, delete the buffer, and return
  if (header->Name[0] != 'Z' || header->Name[1] != 'E')
  {
    status.SetFailed(String::Format("Audio file %s is an incorrect format", fileName.c_str()));
    return 0;
  }

  // Return the size of the file excluding the header
  return (unsigned)size - sizeof(FileHeader);
}

//************************************************************************************************
bool PacketDecoder::CreateDecoders(Status& status, OpusDecoder** decoderArray, int howMany)
{
  int error;
  // Create a decoder for each channel
  for (int i = 0; i < howMany; ++i)
  {
    decoderArray[i] = opus_decoder_create(cSystemSampleRate, 1, &error);

    // Check if there was an error creating the decoder
    if (error < 0)
    {
      // Set the failed message
      status.SetFailed(String::Format("Error creating audio decoder: %s", opus_strerror(error)));

      // Remove any previously created decoders
      for (short j = 0; j < i; ++j)
      {
        opus_decoder_destroy(decoderArray[j]);
        decoderArray[j] = nullptr;
      }

      return false;
    }
  }

  return true;
}

//************************************************************************************************
void PacketDecoder::DestroyDecoders(OpusDecoder** decoderArray, int howMany)
{
  // Destroy each decoder if it exists
  for (int i = 0; i < howMany; ++i)
  {
    if (decoderArray[i])
    {
      opus_decoder_destroy(decoderArray[i]);
      decoderArray[i] = nullptr;
    }
  }
}

//************************************************************************************************
int PacketDecoder::GetPacketFromMemory(byte* packetDataToWrite, const byte* inputData,
  unsigned inputDataSize, unsigned* dataIndex)
{
  if (!inputData || *dataIndex >= inputDataSize)
    return -1;

  // Get the size of the next packet data
  int packetDataSize = PacketDecoder::GetPacketDataSize(inputData + *dataIndex);
  // Move the index forward past the header
  *dataIndex += sizeof(PacketHeader);
  // Read the packet data into the buffer
  memcpy(packetDataToWrite, inputData + *dataIndex, packetDataSize);
  // Move the index forward past the packet data
  *dataIndex += packetDataSize;

  return packetDataSize;
}

//************************************************************************************************
int ReturnError(ThreadLock* lockObject)
{
  lockObject->Unlock();
  return -1;
}

//************************************************************************************************
int PacketDecoder::GetPacketFromFile(byte* packetDataToWrite, File* inputFile,
  FilePosition* filePosition, ThreadLock* lockObject)
{
  if (!inputFile || !inputFile->IsOpen())
    return -1;

  // Lock to prevent reading simultaneously
  lockObject->Lock();

  // Make sure we're at the right location in the file
  if (!inputFile->Seek(*filePosition))
    return ReturnError(lockObject);

  Status status;

  // Read the packet header into memory
  inputFile->Read(status, packetDataToWrite, sizeof(PacketHeader));
  if (status.Failed())
    return ReturnError(lockObject);

  // Get the size of the next packet data
  int packetDataSize = PacketDecoder::GetPacketDataSize(packetDataToWrite);
  if (packetDataSize <= 0)
    return ReturnError(lockObject);

  // Read the packet data into the buffer
  inputFile->Read(status, packetDataToWrite, packetDataSize);
  if (status.Failed())
    return ReturnError(lockObject);

  // Get the new file location
  *filePosition = inputFile->Tell();

  lockObject->Unlock();

  return packetDataSize;
}

//----------------------------------------------------------------------------------- File Decoder

//************************************************************************************************
OsInt StartThreadForDecoding(void* data)
{
  ((AudioFileDecoder*)data)->DecodingLoopThreaded();
  return 0;
}

//************************************************************************************************
AudioFileDecoder::AudioFileDecoder(int channels, unsigned samplesPerChannel, FileDecoderCallback callback,
  void* callbackData) :
  mChannels(channels),
  mSamplesPerChannel(samplesPerChannel),
  mCallback(callback),
  mCallbackData(callbackData),
  mShutDownSignal(0)
{
  // Set all decoder pointers to null
  memset(mDecoders, 0, sizeof(OpusDecoder*) * cMaxChannels);
}

//************************************************************************************************
AudioFileDecoder::~AudioFileDecoder()
{
  StopDecodingThread();
}

//************************************************************************************************
void AudioFileDecoder::DecodeNextSection()
{
  // If the system is threaded, increment the semaphore to trigger another decode
  if (ThreadingEnabled)
    DecodingSemaphore.Increment();
  // Otherwise add this object to the list of tasks to be run on update
  else
    Z::gSound->Mixer.DecodingTasks.PushBack(this);
}

//************************************************************************************************
bool AudioFileDecoder::DecodePacketThreaded()
{
  // Note: This function happens on the decoding thread

  int frames;
  byte packetData[AudioFileEncoder::cMaxPacketSize];
  float decodedPackets[cMaxChannels][AudioFileEncoder::cPacketFrames];

  // Decode a packet for each channel
  for (int i = 0; i < mChannels; ++i)
  {
    // Get the size and fill out the buffer with the data
    int packetDataSize = GetNextPacket(packetData);

    // If no packets are available we are done decoding
    if (packetDataSize < 0)
      return false;

    // Decode the packet into the buffer for this channel
    frames = PacketDecoder::DecodePacket(packetData, packetDataSize, mDecoders[i], decodedPackets[i]);

    ErrorIf(frames < 0, opus_strerror(frames));
  }

  // Create the DecodedPacket object
  DecodedPacket newPacket(frames * mChannels);

  // Step through each frame of samples
  for (int frame = 0, index = 0; frame < frames; ++frame)
  {
    // Copy the sample from each channel to the interleaved sample buffer
    for (short channel = 0; channel < mChannels; ++channel, ++index)
    {
      newPacket.mSamples[index] = decodedPackets[channel][frame];

      // Samples should be between [-1, +1] but it's possible
      // encoding caused the sample to jump beyond 1
      ErrorIf(newPacket.mSamples[index] < -2.0f || newPacket.mSamples[index] > 2.0f);
    }
  }

  // Pass the decoded data to the callback function
  mCallback(&newPacket, mCallbackData);

  return true;
}

//************************************************************************************************
void AudioFileDecoder::StartDecodingThread()
{
  // Start the decoding thread only if threading is enabled
  if (ThreadingEnabled)
  {
    DecodeThread.Initialize(StartThreadForDecoding, this, "Audio decoding");
    DecodeThread.Resume();
  }
}

//************************************************************************************************
void AudioFileDecoder::StopDecodingThread()
{
  if (ThreadingEnabled)
  {
    // Tell the decoding thread to shut down
    mShutDownSignal.Set(cTrue);

    // Increment the semaphore to make sure the shut down signal is seen
    DecodingSemaphore.Increment();

    // Wait for the decoding thread if it isn't finished, and close it
    if (!DecodeThread.IsCompleted())
      DecodeThread.WaitForCompletion();
    DecodeThread.Close();
  }
  else
  {
    // Remove any existing decoding tasks (returns false if value was not found in the array)
    while (Z::gSound->Mixer.DecodingTasks.EraseValue(this))
    {
    }
  }
}

//************************************************************************************************
void AudioFileDecoder::ClearData()
{
  PacketDecoder::DestroyDecoders(mDecoders, mChannels);
}

//---------------------------------------------------------------------- Decompressed File Decoder

//************************************************************************************************
DecompressedDecoder::DecompressedDecoder(Status& status, const String& fileName,
  FileDecoderCallback callback, void* callbackData) :
  AudioFileDecoder(0, 0, callback, callbackData),
  mCompressedData(nullptr),
  mDataIndex(0),
  mDataSize(0)
{
  // If no valid callback was provided, don't do anything
  if (!callback)
    return;

  // Open the file and read in the data
  OpenAndReadFile(status, fileName);
  if (status.Failed())
    return;

  // Create a decoder for each channel
  if (!PacketDecoder::CreateDecoders(status, mDecoders, mChannels))
  {
    // If creating the decoders failed, clear any allocated data and return
    ClearData();
    return;
  }

  StartDecodingThread();
}

//************************************************************************************************
DecompressedDecoder::~DecompressedDecoder()
{
  ClearData();
}

//************************************************************************************************
void DecompressedDecoder::DecodingLoopThreaded()
{
  // Keep looping as long as we are still decoding
  bool decoding = true;
  while (decoding)
  {
    // Wait until signaled that another packet is needed
    DecodingSemaphore.WaitAndDecrement();

    // Check if we are supposed to shut down
    if (mShutDownSignal.Get() == cTrue)
      return;

    // Decode a packet and check if we should keep looping
    decoding = DecodePacketThreaded();

    // We need to keep decoding until we get through everything so trigger another packet
    if (decoding)
      DecodeNextSection();
  }

  // Now that we're done decoding, remove all allocated data
  ClearData();
}

//************************************************************************************************
int DecompressedDecoder::GetNextPacket(byte* packetData)
{
  return PacketDecoder::GetPacketFromMemory(packetData, mCompressedData, mDataSize, &mDataIndex);
}

//************************************************************************************************
void DecompressedDecoder::RunDecodingTask()
{
  DecodePacketThreaded();
}

//************************************************************************************************
void DecompressedDecoder::OpenAndReadFile(Status& status, const String& fileName)
{
  File inputFile;
  FileHeader header;
  mDataSize = PacketDecoder::OpenAndReadHeader(status, fileName, &inputFile, &header);
  if (mDataSize == 0)
    return;

  // Create a buffer for the file data and read it in
  mCompressedData = new byte[mDataSize];
  inputFile.Read(status, mCompressedData, mDataSize);

  // If the read failed, delete the buffer and return
  if (status.Failed())
  {
    ClearData();
    return;
  }

  // Set the data variables
  mSamplesPerChannel = header.SamplesPerChannel;
  mChannels = header.Channels;
}

//************************************************************************************************
void DecompressedDecoder::ClearData()
{
  AudioFileDecoder::ClearData();

  // If there is data in the buffer, delete it
  if (mCompressedData)
  {
    delete[] mCompressedData;
    mCompressedData = nullptr;
  }
}

//------------------------------------------------------------------------------ Streaming Decoder

//************************************************************************************************
StreamingDecoder::StreamingDecoder(Status& status, File* inputFile, ThreadLock* lock,
  unsigned channels, unsigned frames, FileDecoderCallback callback, void* callbackData) :
  AudioFileDecoder(channels, frames, callback, callbackData),
  mCompressedData(nullptr),
  mDataIndex(0),
  mDataSize(0),
  mInputFile(inputFile),
  mFilePosition(sizeof(FileHeader)),
  mLock(lock)
{
  // If no valid callback was provided or the file is not open, don't do anything
  if (!callback || !inputFile->IsOpen())
    return;

  // If creating the decoders fails then return without starting the thread
  if (!PacketDecoder::CreateDecoders(status, mDecoders, mChannels))
    return;

  StartDecodingThread();
}

//************************************************************************************************
StreamingDecoder::StreamingDecoder(Status& status, byte* inputData, unsigned dataSize,
  unsigned channels, unsigned frames, FileDecoderCallback callback, void* callbackData) :
  AudioFileDecoder(channels, frames, callback, callbackData),
  mCompressedData(inputData),
  mDataIndex(0),
  mDataSize(dataSize),
  mInputFile(nullptr),
  mFilePosition(sizeof(FileHeader)),
  mLock(nullptr)
{
  // If no valid callback or data buffer was provided, don't do anything
  if (!callback || !inputData)
    return;

  // If creating the decoders fails then return without starting the thread
  if (!PacketDecoder::CreateDecoders(status, mDecoders, mChannels))
    return;

  StartDecodingThread();
}

//************************************************************************************************
void StreamingDecoder::DecodingLoopThreaded()
{
  // Keep looping as long as we are still decoding
  bool decoding = true;
  while (decoding)
  {
    // Wait until signaled that another packet is needed
    DecodingSemaphore.WaitAndDecrement();

    // Check if we are supposed to shut down
    if (mShutDownSignal.Get() == cTrue)
      return;

    // Decode a packet and check if we should keep looping
    decoding = DecodePacketThreaded();
  }
}

//************************************************************************************************
int StreamingDecoder::GetNextPacket(byte* packetData)
{
  if (mCompressedData)
    return PacketDecoder::GetPacketFromMemory(packetData, mCompressedData, mDataSize, &mDataIndex);
  else
    return PacketDecoder::GetPacketFromFile(packetData, mInputFile, &mFilePosition, mLock);
}

//************************************************************************************************
void StreamingDecoder::RunDecodingTask()
{
  DecodePacketThreaded();
}

//************************************************************************************************
void StreamingDecoder::Reset()
{
  // Stop any current decoding
  StopDecodingThread();

  // Reset the semaphore and shut down signal
  DecodingSemaphore.Reset();
  mShutDownSignal.Set(cFalse);

  // Reset the read positions
  mDataIndex = 0;
  mFilePosition = sizeof(FileHeader);

  // Destroy the current decoders (since they rely on history for decoding, they can't 
  // continue from the beginning of the file)
  ClearData();

  // Create new decoders
  Status status;
  PacketDecoder::CreateDecoders(status, mDecoders, mChannels);

  // Restart the decoding thread
  StartDecodingThread();
}

} // namespace Zero
