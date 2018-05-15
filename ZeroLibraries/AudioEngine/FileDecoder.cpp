///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"
#include "stb_vorbis.h"
#include "opus.h"

namespace Audio
{
  Zero::OsInt StartDecodeThread(void* data)
  {
    ((FileDecoder*)data)->DecodingLoop();
    return 0;
  }

  //--------------------------------------------------------------------------------- Decoded Packet

  //************************************************************************************************
  DecodedPacket::DecodedPacket(unsigned bufferSize) :
    Samples(bufferSize)
  {

  }

  //************************************************************************************************
  DecodedPacket::DecodedPacket(const DecodedPacket& copy) :
    Samples(Zero::MoveReference<Zero::Array<float>>(const_cast<DecodedPacket&>(copy).Samples))
  {

  }

  //************************************************************************************************
  DecodedPacket& DecodedPacket::operator=(const DecodedPacket& other)
  {
    Samples = Zero::MoveReference<Zero::Array<float>>(const_cast<DecodedPacket&>(other).Samples);
    return *this;
  }

  //----------------------------------------------------------------------------------- File Decoder

  //************************************************************************************************
  FileDecoder::FileDecoder(Zero::Status& status, const Zero::String& fileName, const bool streaming) :
    mStreaming(streaming),
    mChannels(0),
    mSamplesPerChannel(0),
    mInputFileData(nullptr),
    mDataIndex(0),
    mDataSize(0),
    mStreamingFileName(fileName),
    ShutDownSignal(0)
  {
    // Note: The constructor happens on the game thread

    // Set all decoder pointers to null
    memset(Decoders, 0, sizeof(OpusDecoder*) * MaxChannels);

    // Open the file and read in the data
    OpenAndReadFile(status, fileName);
    if (status.Failed())
      return;

    // Create a decoder for each channel
    int error;
    for (short i = 0; i < mChannels; ++i)
    {
      Decoders[i] = opus_decoder_create(SystemSampleRate, 1, &error);

      // Check if there was an error creating the decoder
      if (error < 0)
      {
        // Set the failed message
        status.SetFailed(Zero::String::Format("Error creating audio decoder: %s", opus_strerror(error)));

        // Remove any previously created decoders
        for (short j = 0; j < i; ++j)
        {
          opus_decoder_destroy(Decoders[j]);
          Decoders[j] = nullptr;
        }

        // Delete the data buffer
        delete[] mInputFileData;
        mInputFileData = nullptr;

        return;
      }
    }

    // Start the decoding thread
    DecodeThread.Initialize(StartDecodeThread, this, "Audio decoding");
    DecodeThread.Resume();

    // If not streaming, start decoding immediately
    if (!mStreaming)
      DecodingSemaphore.Increment();
  }

  //************************************************************************************************
  FileDecoder::~FileDecoder()
  {
    // Tell the decoding thread to shut down
    Zero::AtomicStore(&ShutDownSignal, 1);
    // Increment the semaphore to make sure the thread sees the shut down signal
    DecodingSemaphore.Increment();

    DecodeThread.WaitForCompletion();
    DecodeThread.Close();

    // Destroy any alive decoders
    for (short i = 0; i < mChannels; ++i)
    {
      if (Decoders[i])
        opus_decoder_destroy(Decoders[i]);
    }

    // If there is data in the buffer, delete it
    if (mInputFileData)
      delete[] mInputFileData;
  }

  //************************************************************************************************
  bool FileDecoder::StreamIsOpen()
  {
    return mStreamingInputFile.IsOpen();
  }

  //************************************************************************************************
  void FileDecoder::ResetStream()
  {
    if (!mStreamingInputFile.IsOpen())
      return;

    // Tell the decoding thread to shut down
    Zero::AtomicStore(&ShutDownSignal, 1);
    // Increment the semaphore to make sure the thread sees the shut down signal
    DecodingSemaphore.Increment();

    DecodeThread.WaitForCompletion();
    DecodeThread.Close();

    // Reset the semaphore and shut down signal
    DecodingSemaphore.Reset();
    ShutDownSignal = 0;
    
    // Remove any current packets from the queue
    DecodedPacketQueue.Clear();

    // Set the file to the start of the data
    mStreamingInputFile.Seek(sizeof(FileHeader));
    // Set the index
    mDataIndex = sizeof(FileHeader);

    // Destroy the current decoders (since they rely on history for decoding, they can't 
    // continue from the beginning of the file)
    for (short i = 0; i < mChannels; ++i)
      opus_decoder_destroy(Decoders[i]);

    // Create new decoders
    int error;
    for (short i = 0; i < mChannels; ++i)
      Decoders[i] = opus_decoder_create(SystemSampleRate, 1, &error);

    // Restart the decoding thread
    DecodeThread.Initialize(StartDecodeThread, this, "Audio decoding");
    DecodeThread.Resume();
    // Signal the thread to decode a packet
    DecodingSemaphore.Increment();
  }

  //************************************************************************************************
  void FileDecoder::CloseStream()
  {
    // Close the streaming file
    if (mStreamingInputFile.IsOpen())
      mStreamingInputFile.Close();
    
    // Remove any existing decoded packets
    DecodedPacketQueue.Clear();
  }

  //************************************************************************************************
  void FileDecoder::OpenStream()
  {
    mStreamingInputFile.Open(mStreamingFileName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);

    ResetStream();
  }

  //************************************************************************************************
  void FileDecoder::DecodeStreamingPacket()
  {
    DecodingSemaphore.Increment();
  }

  //************************************************************************************************
  void FileDecoder::DecodingLoop()
  {
    // Keep looping as long as we are still decoding
    bool running = true;
    while (running)
    {
      // Wait until signaled that another packet is needed
      DecodingSemaphore.WaitAndDecrement();

      // Check if we are supposed to shut down
      if (Zero::AtomicCompareExchange(&ShutDownSignal, 0, 0) != 0)
        return;

      // Decode a packet and check if we should keep looping
      running = DecodePacket();
    }
  }

  //************************************************************************************************
  void FileDecoder::OpenAndReadFile(Zero::Status& status, const Zero::String& fileName)
  {
    // Open the input file
    Zero::File inputFile;
    inputFile.Open(fileName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);
    // If not open, set the failed message and return
    if (!inputFile.IsOpen())
    {
      status.SetFailed(Zero::String::Format("Unable to open audio file %s", fileName.c_str()));
      return;
    }

    // Get the size of the file
    long long size = inputFile.CurrentFileSize();
    // Check for an invalid size
    if (size < sizeof(FileHeader))
    {
      status.SetFailed(Zero::String::Format("Unable to read from audio file %s", fileName.c_str()));
      return;
    }

    // Save the file size 
    mDataSize = (unsigned)size;

    // If not streaming, create a buffer for all data and read it in
    if (!mStreaming)
    {
      mInputFileData = new byte[mDataSize];
      inputFile.Read(status, mInputFileData, mDataSize);
    }
    // Otherwise create a buffer for the maximum packet size and read in the file header
    else
    {
      mInputFileData = new byte[FileEncoder::MaxPacketSize];
      inputFile.Read(status, mInputFileData, sizeof(FileHeader));
    }

    // If the read failed, delete the buffer and return
    if (status.Failed())
    {
      delete[] mInputFileData;
      mInputFileData = nullptr;
      return;
    }

    // Read the file header from the input data
    FileHeader header;
    memcpy(&header, mInputFileData, sizeof(header));
    // Move the index forward
    mDataIndex += sizeof(header);

    // If this isn't the right type of file, set the failed message, delete the buffer, and return
    if (header.Name[0] != 'Z' || header.Name[1] != 'E')
    {
      status.SetFailed(Zero::String::Format("Audio file %s is an incorrect format", fileName.c_str()));
      delete[] mInputFileData;
      mInputFileData = nullptr;
      return;
    }

    // Set the data variables
    mSamplesPerChannel = header.SamplesPerChannel;
    mChannels = header.Channels;
  }

  //************************************************************************************************
  bool FileDecoder::DecodePacket()
  {
    // Note: This function happens on the decoding thread

    // If no data, can't do anything more
    if (!mInputFileData || (mStreaming && !mStreamingInputFile.IsOpen()))
      return false;

    // Check if the data index is outside the buffer
    if (mDataIndex >= mDataSize)
    {
      // If not streaming this always means decoding is finished
      if (!mStreaming)
        return false;
      // If streaming, file could still be reset to beginning
      else
        return true;
    }

    int frames;
    Zero::Status status;

    // Get a packet for each channel
    for (short i = 0; i < mChannels; ++i)
    {
      PacketHeader packHead;

      // Read in the packet header
      if (!mStreaming)
        memcpy(&packHead, mInputFileData + mDataIndex, sizeof(packHead));
      else
        mStreamingInputFile.Read(status, (byte*)&packHead, sizeof(packHead));

      // Move the data index forward
      mDataIndex += sizeof(packHead);

      ErrorIf(mDataIndex + packHead.Size > mDataSize);

      // If not a valid packet, set frames to zero
      if (packHead.Size == 0 || (packHead.Name[0] != 'p' || packHead.Name[1] != 'a'))
        frames = 0;
      else
      {
        // If not streaming, decode the packet from the buffer
        if (!mStreaming)
          frames = opus_decode_float(Decoders[i], mInputFileData + mDataIndex, packHead.Size,
            DecodedPackets[i], FileEncoder::PacketFrames, 0);
        // Otherwise read in the data from the file before decoding
        else
        {
          mStreamingInputFile.Read(status, mInputFileData, packHead.Size);

          frames = opus_decode_float(Decoders[i], mInputFileData, packHead.Size,
            DecodedPackets[i], FileEncoder::PacketFrames, 0);
        }
      }

      ErrorIf(frames < 0, opus_strerror(frames));

      // Move the data index forward
      mDataIndex += packHead.Size;
    }

    bool returnValue;

    // Check if we're not streaming
    if (!mStreaming)
    {
      // If we've reached the end of the file, we are finished decoding
      if (mDataIndex >= mDataSize)
        returnValue = false;
      // Otherwise continue decoding the file and signal for another packet
      else
      {
        returnValue = true;
        DecodingSemaphore.Increment();
      }
    }
    // If streaming, always continue decoding but don't signal for another packet yet
    else
      returnValue = true;

    // Add the decoded packets to the queue. 
    QueueDecodedPackets(frames);

    return returnValue;
  }

  //************************************************************************************************
  void FileDecoder::QueueDecodedPackets(unsigned numberOfFrames)
  {
    // Create the DecodedPacket object
    DecodedPacket newPacket(numberOfFrames * mChannels);

    // Step through each frame of samples
    for (unsigned frame = 0, index = 0; frame < numberOfFrames; ++frame)
    {
      // Copy the sample from each channel to the interleaved sample buffer
      for (short channel = 0; channel < mChannels; ++channel, ++index)
      {
        float sample = DecodedPackets[channel][frame];
        newPacket.Samples[index] = sample;

        // Samples should be between [-1, +1] but it's possible
        // encoding caused the sample to jump beyond 1
        ErrorIf(sample < -2.0f || sample > 2.0f);
      }
    }

    // Add the DecodedPacket object to the queue
    DecodedPacketQueue.Write(newPacket);
  }

  //--------------------------------------------------------------------------------- Packet Decoder

  //************************************************************************************************
  PacketDecoder::~PacketDecoder()
  {
    if (Decoder)
      opus_decoder_destroy(Decoder);
  }

  //************************************************************************************************
  void PacketDecoder::InitializeDecoder()
  {
    if (Decoder)
      opus_decoder_destroy(Decoder);

    int error;
    Decoder = opus_decoder_create(SystemSampleRate, PacketEncoder::Channels, &error);
  }

  //************************************************************************************************
  void PacketDecoder::DecodePacket(const byte* packetData, const unsigned dataSize, 
    float*& decodedData, unsigned& numberOfSamples)
  {
    ReturnIf(!Decoder, , "Tried to decode packet without initializing decoder");

    decodedData = new float[PacketEncoder::PacketFrames];
    numberOfSamples = opus_decode_float(Decoder, packetData, dataSize, decodedData, 
      PacketEncoder::PacketFrames, 0);
  }

}