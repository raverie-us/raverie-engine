///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef FILEENCODER_H
#define FILEENCODER_H

namespace Audio
{
  // Define data type that equals a byte. 
  typedef unsigned char byte;

  // 20 ms of audio data at 48000 samples per second
  static const unsigned FrameSize = 960;
  // Recommended max packet size
  static const unsigned MaxPacketSize = 4000;

  //----------------------------------------------------------------------------------- File Encoder

  class FileEncoder
  {
  public:
    static void ProcessFile(Zero::Status& status, Zero::StringParam inputName,
      Zero::StringParam outputName, unsigned& samplesPerChannel, unsigned& channels, unsigned& sampleRate);

  private:
    static bool PcmToFloat(byte* inputBuffer, float** samplesPerChannel, const unsigned totalSampleCount, 
      const unsigned channelCount, const unsigned bytesPerSample);
  };

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
}

#endif