///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

struct OpusEncoder;

namespace Zero
{

//---------------------------------------------------------------------------------- Audio File Data

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

//------------------------------------------------------------------------------------- File Encoder

class AudioFileEncoder
{
public:
  // Opens the specified file and reads in the raw data
  static AudioFileData OpenFile(Status& status, StringParam fileName);
  // Encodes the audio file and writes it out to the specified file name
  static void WriteFile(Status& status, StringParam outputFileName, AudioFileData& fileData,
    bool normalize, float maxVolume);

  // 20 ms of audio data at 48000 samples per second
  static const unsigned cPacketFrames = 960;
  // Recommended max packet size
  static const unsigned cMaxPacketSize = 4000;

private:
  // Reads in audio data from a WAV file and puts it into the AudioFileData buffer
  static void ReadWav(Status& status, File& file, StringParam fileName, AudioFileData& data);
  // Reads in audio data from an OGG file and puts it into the AudioFileData buffer
  static void ReadOgg(Status& status, File& file, StringParam fileName, AudioFileData& data);
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
  static void EncodeFile(Status& status, File& outputFile, AudioFileData& data,
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

//----------------------------------------------------------------------------------- Packet Encoder

class PacketEncoder
{
public:
  PacketEncoder() : Encoder(nullptr) {}
  ~PacketEncoder();

  // Initializes encoder for use with EncodePacket. 
  // If the encoder already exists, it will be destroyed and re-created.
  void InitializeEncoder();
  // Encodes a single packet of data and allocates a buffer for the encoded data.
  // Number of samples must be the same as PacketFrames
  void EncodePacket(const float* dataBuffer, const unsigned samples, Zero::Array<byte>& encodedData);

  static const unsigned cChannels = 1;

private:
  // Used for repeated calls to EncodePacket
  OpusEncoder* Encoder;

};

} // namespace Zero
