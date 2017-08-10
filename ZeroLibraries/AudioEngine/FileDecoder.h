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
    ~DecodedPacket() { if (Samples) delete[] Samples; }

    unsigned FrameCount;
    float* Samples;
  };

  //----------------------------------------------------------------------------------- File Decoder

  class FileDecoder
  {
  public:
    FileDecoder(Zero::Status& status, const Zero::String& fileName, const bool streaming,
      SoundAssetFromFile* asset);
    ~FileDecoder();

    void DecodeNextPacket();
    bool StreamIsOpen();
    void ResetStream();
    void CloseStream();
    void OpenStream();

    LockFreeQueue<DecodedPacket*> DecodedPacketQueue;

    FileDecoder* Decoding;
    SoundAssetFromFile* Asset;
    short Channels;
    unsigned SamplesPerChannel;

  private:
    byte* InputFileData;
    unsigned DataIndex;
    unsigned DataSize;
    OpusDecoder* Decoders[MaxChannels];
    float DecodedPackets[MaxChannels][FrameSize];
    bool Streaming;
    Zero::String FileName;
    Zero::File InputFile;

    void QueueDecodedPackets(int numberOfFrames);
  };
}

#endif