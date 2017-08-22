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

  //-------------------------------------------------------------------------------- Audio File Data

  // Information about an audio file as well as data read in from the file
  class AudioFileData
  {
  public:

    AudioFileData() :
      SamplesPerChannel(0),
      Channels(0),
      SampleRate(0),
      BuffersPerChannel(nullptr)
    { }

    void ReleaseData();

    unsigned SamplesPerChannel;
    unsigned Channels;
    unsigned SampleRate;
    float** BuffersPerChannel;
  };

  //----------------------------------------------------------------------------------- File Encoder

  class FileEncoder
  {
  public:

    // Opens the specified file and reads in the raw data
    static AudioFileData OpenFile(Zero::Status& status, Zero::StringParam fileName);
    // Encodes the audio file and writes it out to the specified file name
    static void WriteFile(Zero::Status& status, Zero::StringParam outputFileName, 
      AudioFileData& fileData, bool normalize, float maxVolume);

    // 20 ms of audio data at 48000 samples per second
    static const unsigned FrameSize = 960;
    // Recommended max packet size
    static const unsigned MaxPacketSize = 4000;
    
  private:
    // Reads in audio data from a WAV file and puts it into the AudioFileData buffer
    static void ReadWav(Zero::Status& status, Zero::File& file, Zero::StringParam fileName, 
      AudioFileData& data);
    // Reads in audio data from an OGG file and puts it into the AudioFileData buffer
    static void ReadOgg(Zero::Status& status, Zero::File& file, Zero::StringParam fileName,
      AudioFileData& data);
    // Translates PCM audio data to floats
    static bool PcmToFloat(byte* inputBuffer, float** samplesPerChannel, const unsigned totalSampleCount,
      const unsigned channelCount, const unsigned bytesPerSample);
    // Processes the audio data so that its volume peak matches the specified max volume
    static void Normalize(float** samplesPerChannel, const unsigned frames, const unsigned channels,
      float maxVolume);
    // Resamples the audio data to match the system's sample rate
    static unsigned Resample(unsigned fileSampleRate, unsigned channels, unsigned samplesPerChannel,
      float**& buffersPerChannel);
    // Encodes the audio data and writes it out to the file
    static void EncodeFile(Zero::Status& status, Zero::File& outputFile, AudioFileData& data, 
      float** buffersPerChannel);
  };

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