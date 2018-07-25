///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef FILEDECODER_H
#define FILEDECODER_H

struct OpusDecoder;

namespace Audio
{

  //--------------------------------------------------------------------------------- Decoded Packet

  struct DecodedPacket
  {
    DecodedPacket() {}
    DecodedPacket(unsigned bufferSize);
    DecodedPacket(const DecodedPacket& copy);

    DecodedPacket& operator=(const DecodedPacket& other);

    // The buffer of samples for this channel
    Zero::Array<float> Samples;
  };

  //----------------------------------------------------------------------------------- File Decoder

  class FileDecoder
  {
  public:
    FileDecoder(Zero::Status& status, const Zero::String& fileName, FileLoadType::Enum loadType);
    ~FileDecoder();

    // Returns true if it is currently streaming audio data from a file
    bool StreamIsOpen();
    // Resets the streaming file to the beginning (if stream is open)
    void ResetStream();
    // Closes the streaming file (if stream is open)
    void CloseStream();
    // Opens the streaming file (if the decoder was created for streaming)
    void OpenStream();
    // Tells the decoder to start decoding another packet from a streaming file
    void DecodeStreamingPacket();
    // Should only be called when starting the decoding thread
    void DecodingLoop();

    // List of decoded packets
    LockFreeQueue<DecodedPacket> DecodedPacketQueue;
    // Number of channels of audio
    short mChannels;
    // Number of samples per channel in the audio data
    unsigned mSamplesPerChannel;
    // If true, streaming from disk instead of using the saved buffer
    bool mStreaming;

  private:
    // The data read in from the file, if not streaming
    byte* mInputFileData;
    // The current read position for the file data
    unsigned mDataIndex;
    // The size of the file data
    unsigned mDataSize;
    // Opus decoders for each channel
    OpusDecoder* Decoders[MaxChannels];
    // Buffers to hold the decoded data for a single packet per channel
    float DecodedPackets[MaxChannels][FileEncoder::PacketFrames];
    // The name of the file to use for streaming
    Zero::String mStreamingFileName;
    // The file object to use when streaming
    Zero::File mStreamingInputFile;
    // Thread for decoding tasks
    Zero::Thread DecodeThread;
    // Tells the decoding thread it should decode another packet
    Zero::Semaphore DecodingSemaphore;
    // Tells the decoding thread it should shut down
    AtomicType ShutDownSignal;
    // The length of a file, in samples, at which it should stream if FileLoadType is Auto
    const static unsigned mLengthForStreaming = SystemSampleRate * 60;
    
    // Opens a file and reads in its data
    void OpenAndReadFile(Zero::Status& status, const Zero::String& fileName, FileLoadType::Enum loadType);
    // Decodes the next packet of data (assumed that this is called on a decoding thread)
    bool DecodePacket();
    // Adds decoded packets to the queue and translates the per-channel buffers to
    // an interleaved buffer
    void QueueDecodedPackets(unsigned numberOfFrames);
    // Triggers a new decoding task 
    void AddDecodingTask();
    // Starts up the decoding thread, and starts a decoding tasks if decodeNow is True
    void StartDecodingThread(bool decodeNow);
    // Stops and shuts down the decoding thread
    void StopDecodingThread();

    friend class AudioSystemInterface;
  };

  Zero::OsInt StartThreadForDecoding(void* data);

  //--------------------------------------------------------------------------------- Packet Decoder

  class PacketDecoder
  {
  public:
    PacketDecoder() : Decoder(nullptr) {}
    ~PacketDecoder();

    // Initializes decoder for use with DecodePacket. 
    // If the decoder already exists, it will be destroyed and re-created.
    void InitializeDecoder();
    // Decodes a single packet of data and allocates a buffer for the decoded data.
    void DecodePacket(const byte* packetData, const unsigned dataSize, float*& decodedData, 
      unsigned& numberOfSamples);

  private:
    // Used for repeated calls to DecodePacket
    OpusDecoder* Decoder;
  };
}

#endif