///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"
#include "stb_vorbis.h"

namespace Audio
{
  struct WavRiffHeader
  {
    char riff_chunk[4];
    unsigned chunk_size;
    char wave_fmt[4];
  };

  struct WavChunkHeader
  {
    char chunk_name[4];
    unsigned chunk_size;
  };

  struct WavFmtData
  {
    unsigned short audio_format;
    unsigned short number_of_channels;
    unsigned sampling_rate;
    unsigned bytes_per_second;
    unsigned short bytes_per_sample;
    unsigned short bits_per_sample;
  };

  //------------------------------------------------------------------------------------- Audio Data

  //************************************************************************************************
  void AudioData::CloseFile()
  {
    if (!FilePointer)
      return;

    if (Type == Type_WAV)
      delete ((Zero::File*)FilePointer);
    else if (Type == Type_OGG)
      stb_vorbis_close((stb_vorbis*)FilePointer);

    FilePointer = nullptr;
  }

  //------------------------------------------------------------------------------------ File Access

  //************************************************************************************************
  SamplesFromFile* FileAccess::LoadAudioFile(Zero::Status &status, const Zero::String &fileName)
  {
    return ReadFile(status, fileName, false);
  }

  //************************************************************************************************
  SamplesFromFile* FileAccess::StartStream(Zero::Status &status, const Zero::String &fileName)
  {
    return ReadFile(status, fileName, true);
  }

  //************************************************************************************************
  AudioData FileAccess::GetFileData(Zero::Status& status, const Zero::String& fileName)
  {
    AudioData data;
    data.Type = Type_Other;

    Zero::File* file = new Zero::File;
    file->Open(fileName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);

    if (!file->IsOpen())
    {
      status.SetFailed(Zero::String::Format("Couldn't open file %s", fileName.c_str()));
      delete file;
      return data;
    }

    // Read in initial chunk header
    WavRiffHeader riffHeader;
    file->Read(status, (byte*)(&riffHeader), sizeof(riffHeader));

    // WAV file
    if (riffHeader.riff_chunk[0] == 'R')
    {
      // Check the WAVE ID
      if (riffHeader.wave_fmt[0] != 'W' || riffHeader.wave_fmt[1] != 'A')
      {
        status.SetFailed(Zero::String::Format("File %s is an unreadable format.", fileName.c_str()));
        delete file;
        return data;
      }

      GetWavData(file, data);
    }
    // OGG file
    else if ((riffHeader.riff_chunk[0] == 'O' || riffHeader.riff_chunk[0] == 'o')
      && (riffHeader.riff_chunk[1] == 'G' || riffHeader.riff_chunk[1] == 'g'))
    {
      // Close the file so we can open it through stb_vorbis
      delete file;

      // Open the file in ogg format
      int error;
      stb_vorbis* OggStream = stb_vorbis_open_filename(fileName.c_str(), &error, nullptr);
      if (!OggStream)
      {
        status.SetFailed(Zero::String::Format("Couldn't open file %s: stb_vorbis error %d.", 
          fileName.c_str(), error));
        // Don't need to call close because the stream is null
        return data;
      }

      GetOggData(OggStream, data);
    }
    else
    {
      status.SetFailed(Zero::String::Format("File %s is an unreadable format. Only WAV and OGG files are supported.", 
        fileName.c_str()));

      delete file;
    }

    return data;
  }

  //************************************************************************************************
  SamplesFromFile* FileAccess::ReadFile(Zero::Status& status, const Zero::String& fileName, 
    const bool streaming)
  {
    AudioData data = GetFileData(status, fileName);
    SamplesFromFile* audioData(nullptr);
    if (status.Failed())
      return audioData;

    if (data.Type == Type_WAV)
    {
      audioData = new SamplesFromFile(new WavDecoder(data.Channels, data.SampleRate, data.BytesPerSample,
        data.TotalSamples, data.TotalDataSize));

      ReadWavFile(data, audioData, streaming, fileName);
    }
    else if (data.Type == Type_OGG)
    {
      audioData = new SamplesFromFile(new OggDecoder(data.Channels, data.SampleRate, data.BytesPerSample,
        data.TotalSamples, data.TotalDataSize));

      ReadOggFile(data, audioData, streaming, fileName);
    }
    else
      status.SetFailed("Unsupported file type. Files must be in WAV or OGG format.");

    return audioData;
  }

  //************************************************************************************************
  void FileAccess::GetWavData(Zero::File* file, AudioData& data)
  {
    // Read in the next chunk header
    WavChunkHeader chunkHeader;
    Zero::Status status;
    file->Read(status, (byte*)(&chunkHeader), sizeof(chunkHeader));
    // If this isn't the fmt chunk, keep looking
    while (chunkHeader.chunk_name[0] != 'f' || chunkHeader.chunk_name[1] != 'm' 
      || chunkHeader.chunk_name[2] != 't')
    {
      file->Seek(chunkHeader.chunk_size, Zero::FileOrigin::Current);
      file->Read(status, (byte*)(&chunkHeader), sizeof(chunkHeader));
    }

    // Read in the fmt chunk data
    WavFmtData fmtChunkData;
    file->Read(status, (byte*)(&fmtChunkData), sizeof(fmtChunkData));

    // If the chunk size is larger than the WavFmtData struct, skip ahead
    if (chunkHeader.chunk_size > sizeof(fmtChunkData))
      file->Seek(chunkHeader.chunk_size - sizeof(fmtChunkData), Zero::FileOrigin::Current);

    // Get the data chunk header
    file->Read(status, (byte*)(&chunkHeader), sizeof(chunkHeader));
    // If this isn't the data chunk, keep looking
    while (chunkHeader.chunk_name[0] != 'd' || chunkHeader.chunk_name[1] != 'a' 
      || chunkHeader.chunk_name[2] != 't')
    {
      file->Seek(chunkHeader.chunk_size, Zero::FileOrigin::Current);
      file->Read(status, (byte*)(&chunkHeader), sizeof(chunkHeader));
    }

    data.Type = Type_WAV;
    data.Channels = fmtChunkData.number_of_channels;
    data.SampleRate = fmtChunkData.sampling_rate;
    data.BytesPerSample = fmtChunkData.bytes_per_sample / fmtChunkData.number_of_channels;
    data.TotalSamples = chunkHeader.chunk_size / data.BytesPerSample;
    data.TotalDataSize = chunkHeader.chunk_size;
    data.FilePointer = file;
    data.WavFormat = fmtChunkData.audio_format;
  }

  //************************************************************************************************
  void FileAccess::GetOggData(stb_vorbis* OggStream, AudioData& data)
  {
    // Get the ogg vorbis file info
    stb_vorbis_info info = stb_vorbis_get_info(OggStream);

    data.Type = Type_OGG;
    data.Channels = info.channels;
    data.SampleRate = info.sample_rate;
    data.BytesPerSample = 0;
    data.TotalSamples = stb_vorbis_stream_length_in_samples(OggStream) * info.channels;
    data.TotalDataSize = 0;
    data.FilePointer = OggStream;
  }

  //************************************************************************************************
  void FileAccess::ReadWavFile(AudioData& data, SamplesFromFile* audioData, bool streaming, 
    const Zero::String& fileName)
  {
    Zero::File* WavStream = (Zero::File*)data.FilePointer;

    unsigned bufferSize;
    // If not streaming, buffer size is the size of all the audio data
    if (!streaming)
      bufferSize = data.TotalDataSize;
    else
    {
      // If streaming, set to streamed buffer size
      bufferSize = audioData->DecodingData->StreamedBufferSizeInBytes;

      // Set the data for streaming
      audioData->DecodingData->SetStreaming(fileName, WavStream->Tell());
    }

    // Create buffer
    byte* buffer = new byte[bufferSize];

    // If not streaming, read the file data into the buffer
    if (!streaming)
    {
      Zero::Status status;
      WavStream->Read(status, buffer, bufferSize);
    }

    // Add the buffer to the audio data object
    audioData->SetBuffer(buffer, bufferSize);

    // Close the file (will be reopened if needed for streaming)
    delete WavStream;
    data.FilePointer = nullptr;
  }

  //************************************************************************************************
  void FileAccess::ReadOggFile(AudioData& data, SamplesFromFile* audioData, bool streaming, 
    const Zero::String& fileName)
  {
    if (!streaming)
    {
      // Close the vorbis file
      stb_vorbis_close((stb_vorbis*)data.FilePointer);
      data.FilePointer = nullptr;

      // Open the file for reading
      Zero::File file;
      file.Open(fileName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);
      // Create the buffer
      size_t size = (size_t)file.CurrentFileSize();
      byte* buffer = new byte[size];
      // Read the entire file into the buffer
      Zero::Status status;
      file.Read(status, buffer, size);

      // Create a vorbis stream from the buffer
      int error;
      data.FilePointer = stb_vorbis_open_memory(buffer, size, &error, nullptr);
      // Add the buffer to the data object
      audioData->SetBuffer(buffer, size);
    }
    else
    {
      // Set the data for streaming
      audioData->DecodingData->SetStreaming(fileName, 0);

      // Set the buffer on the audio data object to null (won't be used)
      audioData->SetBuffer(nullptr, 0);
    }

    // Set the vorbis stream on the data object
    audioData->DecodingData->SetFile((stb_vorbis*)data.FilePointer);
  }

}