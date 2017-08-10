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
  enum FileTypes { WAV_Type, Ogg_Type, Other_Type };

  // Define data type that equals a byte. 
  typedef unsigned char byte;

  //-------------------------------------------------------------------------------- Audio File Data

  class AudioFileData
  {
  public:

    AudioFileData() :
      SamplesPerChannel(0),
      Channels(0),
      SampleRate(0),
      Type(Other_Type),
      FileData(nullptr),
      BytesPerSample(0),
      FileSize(0)
    {}

    unsigned SamplesPerChannel;
    unsigned Channels;
    unsigned SampleRate;
    unsigned BytesPerSample;
    unsigned FileSize;
    FileTypes Type;
    byte* FileData;
  };

  //----------------------------------------------------------------------------------- File Encoder

  class FileEncoder
  {
  public:

    static AudioFileData OpenFile(Zero::Status& status, Zero::StringParam fileName);
    static void WriteFile(Zero::Status& status, Zero::StringParam outputFileName, 
      AudioFileData& fileData);

  private:
    static void ReadWav(Zero::Status& status, Zero::File& file, AudioFileData& data);
    static void ReadOgg(Zero::Status& status, Zero::File& file, Zero::StringParam fileName,
      AudioFileData& data);
    static bool PcmToFloat(byte* inputBuffer, float** samplesPerChannel, const unsigned totalSampleCount,
      const unsigned channelCount, const unsigned bytesPerSample);
    static void Normalize(float** samplesPerChannel, const unsigned frames, const unsigned channels);
    static unsigned Resample(unsigned fileSampleRate, unsigned channels, unsigned samplesPerChannel,
      float**& buffersPerChannel);
    static void EncodeFile(Zero::Status& status, Zero::File& outputFile, AudioFileData& data, 
      float** buffersPerChannel);
  };

  // 20 ms of audio data at 48000 samples per second
  static const unsigned FrameSize = 960;
  // Recommended max packet size
  static const unsigned MaxPacketSize = 4000;

  static const float MaxVolume = 0.9f;

  struct FileHeader
  {
    const char Name[4] = { 'Z','E','R','O' };
    short Channels;
    unsigned SamplesPerChannel;
  };

  struct PacketHeader
  {
    PacketHeader() : Channel(0), Size(0) {}

    const char Name[4] = { 'p','a','c','k' };
    short Channel;
    unsigned Size;
  };
}

#endif