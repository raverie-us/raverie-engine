///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef FILEDECODE_H
#define FILEDECODE_H

#include "opus.h"

namespace Audio
{
  //----------------------------------------------------------------------------------- File Decoder

  struct FileHeader
  {
    const char Name[4] = { 'Z','E','R','O' };
    short Channels;
    unsigned SamplesPerChannel;
  };

  struct PacketHeader
  {
    const char Name[4] = { 'p','a','c','k' };
    short Channel;
    unsigned Size;
  };

  struct DecodedPacket
  {
    DecodedPacket() : Samples(nullptr) {}
    ~DecodedPacket() { if (Samples) delete[] Samples; }

    unsigned FrameCount;
    float* Samples;
  };

  class FileDecoder
  {
  public:
    FileDecoder(Zero::Status& status, const Zero::String& fileName, const bool streaming);

    void DecodeNextPacket();

    LockFreeQueue<DecodedPacket*> DecodedPacketQueue;

    FileDecoder* DecodingCheck;

    // 20 ms of audio data at 48000 samples per second
    static const unsigned FrameSize = 960;
    // Recommended max packet size
    static const unsigned MaxPacketSize = 4000;

    Zero::File InputFile;
    OpusDecoder* Decoders[MaxChannels];
    unsigned char PacketBuffer[MaxPacketSize];
    float DecodedPackets[MaxChannels][FrameSize];
    FileHeader Header;
    bool Streaming;
  };
}

#endif
