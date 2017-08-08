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
  //-------------------------------------------------------------------------------- Audio File Data

  struct AudioFileData
  {
    AudioFileData() :
      SamplesPerChannel(0),
      Channels(0),
      SampleRate(0)
    {}

    unsigned SamplesPerChannel;
    unsigned Channels;
    unsigned SampleRate;
  };

  //----------------------------------------------------------------------------------- File Encoder

  class FileEncoder
  {
  public:
    static AudioFileData ProcessFile(Zero::Status& status, Zero::StringParam inputName,
      Zero::StringParam outputName);

  private:
    static AudioFileData ReadWav(Zero::Status& status, Zero::File& file, Zero::StringParam fileName, 
      float**& buffersPerChannel);
    static bool PcmToFloat(byte* inputBuffer, float** samplesPerChannel, const unsigned totalSampleCount, 
      const unsigned channelCount, const unsigned bytesPerSample);
    static AudioFileData ReadOgg(Zero::Status& status, Zero::File& file, Zero::StringParam fileName,
      float**& buffersPerChannel);
  };

  // Define data type that equals a byte. 
  typedef unsigned char byte;

  // 20 ms of audio data at 48000 samples per second
  static const unsigned FrameSize = 960;
  // Recommended max packet size
  static const unsigned MaxPacketSize = 4000;

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