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
    DecodedPacket() : Samples(nullptr) {}
    DecodedPacket(const DecodedPacket& copy);

    // Deletes the sample buffer
    void ReleaseSamples();

    // The number of frames of audio data in this packet
    unsigned FrameCount;
    // The buffer of samples for this channel
    float* Samples;
  };

  //----------------------------------------------------------------------------------- File Decoder

  class FileDecoder
  {
  public:
    FileDecoder(Zero::Status& status, const Zero::String& fileName, const bool streaming,
      SoundAssetFromFile* asset);
    ~FileDecoder();

    // Creates a task on the decoding thread to decode the next packet
    void AddDecodingTask();
    // Decodes the next packet of data (assumed that this is called on a decoding thread)
    void DecodePacket();
    // Returns true if it is currently streaming audio data from a file
    bool StreamIsOpen();
    // Resets the streaming file to the beginning (if stream is open)
    void ResetStream();
    // Closes the streaming file (if stream if open)
    void CloseStream();
    // Opens the streaming file (if the decoder was created for streaming)
    void OpenStream();

    // List of decoded packets
    LockFreeQueue<DecodedPacket> DecodedPacketQueue;
    // Number of channels of audio
    short Channels;
    // Number of samples per channel in the audio data
    unsigned SamplesPerChannel;
    // Keeps track of active decoding tasks - shared between threads!
    Type32Bit DecodingTaskCount;
    // Will be set to an object as long as the parent asset is alive, null when deleted
    void* ParentAlive;

  private:
    // The data read in from the file, if not streaming
    byte* InputFileData;
    // The current read position for the file data
    unsigned DataIndex;
    // The size of the file data
    unsigned DataSize;
    // Opus decoders for each channel
    OpusDecoder* Decoders[MaxChannels];
    // Buffers to hold the decoded data for a single packet per channel
    float DecodedPackets[MaxChannels][FileEncoder::PacketFrames];
    // If true, streaming from disk instead of using the saved buffer
    bool Streaming;
    // The name of the file to use for streaming
    Zero::String FileName;
    // The file object to use when streaming
    Zero::File InputFile;

    // Adds decoded packets to the queue and translates the per-channel buffers to
    // an interleaved buffer
    void QueueDecodedPackets(int numberOfFrames);
    // Decrements the DecodingTaskCount and checks if it should delete itself
    void FinishDecodingPacket();
  };

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