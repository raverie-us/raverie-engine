///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef FILE_H
#define FILE_H

struct stb_vorbis;

namespace Audio
{
  class SamplesFromFile;

  // Define data type that equals a byte. 
  typedef unsigned char byte;

  // Types of audio files. 
  enum FileType
  {
    Type_WAV,
    Type_OGG,
    Type_Other
  };

  struct AudioData
  {
    void CloseFile();

    FileType Type;
    unsigned Channels;
    unsigned SampleRate;
    unsigned BytesPerSample;
    unsigned TotalSamples;
    unsigned TotalDataSize;
    void* FilePointer;
    unsigned WavFormat;
  };

  //------------------------------------------------------------------------------------ File Access

  // Object to handle reading data from files and storing it. Also handles streaming from files. 
  class FileAccess
  {
  public:
    // Calls functions to determine file type and read file. Sets failed info if file 
    // type is unsupported, and passes to other functions.
    static SamplesFromFile* LoadAudioFile(Zero::Status& status, const Zero::String& fileName);
    // Sets up an audio file to stream from disk, allocating memory for samples. Sets failed 
    // info if file type is unsupported, and passes to other functions.
    static SamplesFromFile* StartStream(Zero::Status& status, const Zero::String& fileName);
    // Opens an audio file, determines its type, and reads in the audio data. The FilePointer
    // in the AudioData object must be deleted or closed (depending on format) if it is not used.
    static AudioData GetFileData(Zero::Status& status, const Zero::String& fileName);

  private:
    static SamplesFromFile* ReadFile(Zero::Status& status, const Zero::String& fileName, 
      const bool streaming);
    static void GetWavData(Zero::File* file, AudioData& data);
    static void GetOggData(stb_vorbis* OggStream, AudioData& data);
    static void ReadWavFile(AudioData& data, SamplesFromFile* audioData, bool streaming, 
      const Zero::String& fileName);
    static void ReadOggFile(AudioData& data, SamplesFromFile* audioData, bool streaming, 
      const Zero::String& fileName);
  };



}

#endif