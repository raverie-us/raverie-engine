///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"
#include "stb_vorbis.h"
#include "opus.h"

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

  const static float Normalize16Bit = 1 << 15;
  const static float Normalize24Bit = 1 << 23;

  //----------------------------------------------------------------------------------- File Encoder

  //************************************************************************************************
  Audio::AudioFileData FileEncoder::OpenFile(Zero::Status& status, Zero::StringParam fileName)
  {
    AudioFileData data;
    Zero::File file;

    // Open the input file
    file.Open(fileName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);
    // If didn't open successfully, set the status and return
    if (!file.IsOpen())
    {
      status.SetFailed(Zero::String::Format("Couldn't open input file %s", fileName.c_str()));
      ErrorIf(true);
      return data;
    }

    // Read in the header from the input file
    WavRiffHeader header;
    file.Read(status, (byte*)(&header), sizeof(header));

    if (header.riff_chunk[0] == 'R' && header.riff_chunk[1] == 'I')
    {
      data.Type = WAV_Type;

      // Check WAV ID - if not WAVE, set status and return
      if (header.wave_fmt[0] != 'W' || header.wave_fmt[1] != 'A')
      {
        status.SetFailed(Zero::String::Format("File %s is an unreadable format", fileName.c_str()));
        ErrorIf(true);
        return data;
      }

      ReadWav(status, file, data);
    }
    else if ((header.riff_chunk[0] == 'O' || header.riff_chunk[0] == 'o') &&
      (header.riff_chunk[1] == 'G' || header.riff_chunk[1] == 'g'))
    {
      data.Type = Ogg_Type;

      ReadOgg(status, file, fileName, data);
    }

    return data;
  }

  //************************************************************************************************
  void FileEncoder::WriteFile(Zero::Status& status, Zero::StringParam outputFileName, 
    AudioFileData& data)
  {
    // Open the output file
    Zero::File outputFile;
    outputFile.Open(outputFileName, Zero::FileMode::Write, Zero::FileAccessPattern::Sequential);
    // If didn't open successfully, set the status and return
    if (!outputFile.IsOpen())
    {
      status.SetFailed(Zero::String::Format("Couldn't create output file %s", outputFileName.c_str()));
      ErrorIf(true);
      return;
    }

    // Create sample buffers for each channel
    float** buffersPerChannel = new float*[data.Channels];
    for (unsigned i = 0; i < data.Channels; ++i)
      buffersPerChannel[i] = new float[data.SamplesPerChannel];

    if (data.Type == WAV_Type)
    {
      if (!PcmToFloat(data.FileData, buffersPerChannel, data.SamplesPerChannel * data.Channels,
        data.Channels, data.BytesPerSample))
      {
        status.SetFailed(Zero::String::Format("File %s is in WAV format but is not 16 or 24 bit",
          outputFileName.c_str()));
        ErrorIf(true);
      }
    }
    else if (data.Type == Ogg_Type)
    {
      // Create the vorbis stream
      int error;
      stb_vorbis* oggStream = stb_vorbis_open_memory(data.FileData, data.FileSize, &error, nullptr);

      data.SamplesPerChannel = stb_vorbis_get_samples_float(oggStream, data.Channels, buffersPerChannel,
        data.SamplesPerChannel);

      stb_vorbis_close(oggStream);
    }
    else
    {
      status.SetFailed(Zero::String::Format("File %s was not in WAV or OGG format", 
        outputFileName.c_str()));
      ErrorIf(true);
      return;
    }

    // Make sure the volume of the audio is below the max
    Normalize(buffersPerChannel, data.SamplesPerChannel, data.Channels);

    // If the sample rates don't match, resample the audio
    if (data.SampleRate != AudioSystemInternal::SampleRate)
    {
      data.SamplesPerChannel = Resample(data.SampleRate, data.Channels, data.SamplesPerChannel, 
        buffersPerChannel);
      data.SampleRate = AudioSystemInternal::SampleRate;
    }

    // Encode the file and write it out
    EncodeFile(status, outputFile, data, buffersPerChannel);
  }

  //************************************************************************************************
  void FileEncoder::ReadWav(Zero::Status& status, Zero::File& file, AudioFileData& data)
  {
    // Read in the next chunk header
    WavChunkHeader chunkHeader;
    file.Read(status, (byte*)(&chunkHeader), sizeof(chunkHeader));
    // If this isn't the fmt chunk, keep looking
    while (chunkHeader.chunk_name[0] != 'f' || chunkHeader.chunk_name[1] != 'm'
      || chunkHeader.chunk_name[2] != 't')
    {
      file.Seek(chunkHeader.chunk_size, Zero::FileOrigin::Current);
      file.Read(status, (byte*)(&chunkHeader), sizeof(chunkHeader));

      if (status.Failed())
        return;
    }

    // Read in the fmt chunk data
    WavFmtData fmtChunkData;
    file.Read(status, (byte*)(&fmtChunkData), sizeof(fmtChunkData));

    // If the chunk size is larger than the WavFmtData struct, skip ahead
    if (chunkHeader.chunk_size > sizeof(fmtChunkData))
      file.Seek(chunkHeader.chunk_size - sizeof(fmtChunkData), Zero::FileOrigin::Current);

    // Get the data chunk header
    file.Read(status, (byte*)(&chunkHeader), sizeof(chunkHeader));
    // If this isn't the data chunk, keep looking
    while (chunkHeader.chunk_name[0] != 'd' || chunkHeader.chunk_name[1] != 'a'
      || chunkHeader.chunk_name[2] != 't')
    {
      file.Seek(chunkHeader.chunk_size, Zero::FileOrigin::Current);
      file.Read(status, (byte*)(&chunkHeader), sizeof(chunkHeader));

      if (status.Failed())
        return;
    }

    data.Channels = fmtChunkData.number_of_channels;
    data.SamplesPerChannel = chunkHeader.chunk_size / fmtChunkData.bytes_per_sample;
    data.SampleRate = fmtChunkData.sampling_rate;
    data.BytesPerSample = fmtChunkData.bytes_per_sample / data.Channels;

    // Create the buffer for reading in data from the file
    data.FileData = new byte[chunkHeader.chunk_size];

    // Read in the audio data 
    file.Read(status, data.FileData, chunkHeader.chunk_size);
    if (status.Failed())
    {
      delete[] data.FileData;
      data.FileData = nullptr;
    }
  }

  //************************************************************************************************
  void FileEncoder::ReadOgg(Zero::Status& status, Zero::File& file, Zero::StringParam fileName, 
    AudioFileData& data)
  {
    // Reset to beginning of file
    file.Seek(0);

    data.FileSize = (unsigned)file.CurrentFileSize();

    // Create the buffer for reading in data from the file
    data.FileData = new byte[data.FileSize];

    // Read in the audio data 
    file.Read(status, data.FileData, data.FileSize);
    if (status.Failed())
    {
      delete[] data.FileData;
      data.FileData = nullptr;
      return;
    }

    // Create the vorbis stream
    int error;
    stb_vorbis* oggStream = stb_vorbis_open_memory(data.FileData, data.FileSize, &error, nullptr);

    if (error != VORBIS__no_error)
    {
      status.SetFailed(Zero::String::Format("Error reading file %s: vorbis error %d",
        fileName.c_str(), error));
      delete[] data.FileData;
      data.FileData = nullptr;
    }
    else
    {
      // Get the ogg vorbis file info
      stb_vorbis_info info = stb_vorbis_get_info(oggStream);

      data.Channels = info.channels;
      data.SamplesPerChannel = stb_vorbis_stream_length_in_samples(oggStream);
      data.SampleRate = info.sample_rate;
    }

    stb_vorbis_close(oggStream);
  }

  //************************************************************************************************
  bool FileEncoder::PcmToFloat(byte* inputBuffer, float** samplesPerChannel,
    const unsigned totalSampleCount, const unsigned channelCount, const unsigned bytesPerSample)
  {
    // 16 bit data can just be read as shorts
    if (bytesPerSample == 2)
    {
      short* input = (short*)inputBuffer;

      for (unsigned inputIndex = 0, sampleIndex = 0; inputIndex < totalSampleCount; ++sampleIndex)
      {
        for (unsigned channel = 0; channel < channelCount; ++channel, ++inputIndex)
          samplesPerChannel[channel][sampleIndex] = (float)input[inputIndex] / Normalize16Bit;
      }

      return true;
    }
    // 24 bit data
    else if (bytesPerSample == 3)
    {
      for (unsigned index = 0, sampleIndex = 0; index < totalSampleCount * bytesPerSample; ++sampleIndex)
      {
        for (unsigned channel = 0; channel < channelCount; index += bytesPerSample, ++channel)
        {
          // Get first byte
          int sample = inputBuffer[index];
          // Get next two bytes
          memcpy(&sample, inputBuffer + index, sizeof(char) * 3);
          // Account for negative numbers
          if (sample & 0x800000)
            sample |= 0xff000000;
          else
            sample &= 0x00ffffff;

          samplesPerChannel[channel][sampleIndex] = (float)sample / Normalize24Bit;
        }
      }

      return true;
    }
    else
      return false;
  }

  //************************************************************************************************
  void FileEncoder::Normalize(float** samplesPerChannel, const unsigned frames, const unsigned channels)
  {
    float maxVolume(0.0f);

    for (unsigned i = 0; i < frames; ++i)
    {
      for (unsigned j = 0; j < channels; ++j)
      {
        if (Math::Abs(samplesPerChannel[j][i]) > maxVolume)
          maxVolume = Math::Abs(samplesPerChannel[j][i]);
      }
    }

    if (maxVolume > MaxVolume)
    {
      float multiplier = MaxVolume / maxVolume;

      for (unsigned i = 0; i < frames; ++i)
      {
        for (unsigned j = 0; j < channels; ++j)
        {
          samplesPerChannel[j][i] *= multiplier;
        }
      }
    }
  }

  //************************************************************************************************
  unsigned FileEncoder::Resample(unsigned fileSampleRate, unsigned channels, unsigned samplesPerChannel, 
    float**& buffersPerChannel)
  {
    double ratio = (double)fileSampleRate / (double)AudioSystemInternal::SampleRate;
    unsigned newFrames;
    Zero::Array<float> newSamples;

    for (unsigned i = 0; i < channels; ++i)
    {
      float* oldSamples = buffersPerChannel[i];
      unsigned index(0);
      double resampleIndex(0);

      while (index + 1 < samplesPerChannel)
      {
        float firstSample = oldSamples[index];
        float secondSample = oldSamples[index + 1];

        newSamples.PushBack(firstSample + (float)((secondSample - firstSample) * (resampleIndex - index)));

        resampleIndex += ratio;
        index = (unsigned)resampleIndex;
      }

      newFrames = newSamples.Size();

      delete[] buffersPerChannel[i];
      buffersPerChannel[i] = newSamples.Data();

      newSamples.ReleaseData();
    }

    return newFrames;
  }

  //************************************************************************************************
  void FileEncoder::EncodeFile(Zero::Status& status, Zero::File& outputFile, AudioFileData& data, 
    float** buffersPerChannel)
  {
    // Create the buffer for encoded packets
    unsigned char encodedPacket[MaxPacketSize];

    // Set up the file header
    FileHeader fileHeader;
    fileHeader.Channels = data.Channels;
    fileHeader.SamplesPerChannel = data.SamplesPerChannel;

    ErrorIf(fileHeader.Channels < 0);

    // Write the header to the output file
    outputFile.Write((byte*)&fileHeader, sizeof(fileHeader));

    // Create the packet header object
    PacketHeader packetHeader;

    int error;
    // Create an opus encoder for each channel
    OpusEncoder** encodersPerChannel = new OpusEncoder*[data.Channels];
    for (unsigned i = 0; i < data.Channels; ++i)
      encodersPerChannel[i] = opus_encoder_create(AudioSystemInternal::SampleRate, 1,
        OPUS_APPLICATION_AUDIO, &error);

    if (error < 0)
    {
      status.SetFailed(Zero::String::Format("Error encoding file: %s\n", opus_strerror(error)));
      ErrorIf(true);
    }
    else
    {
      // Current buffer position for encoding
      float* buffer(nullptr);
      // Used on the final frame to keep the same frame size
      float finalBuffer[FrameSize] = { 0 };

      // Encode the samples, in chunks of FrameSize
      for (unsigned inputIndex = 0; inputIndex < data.SamplesPerChannel; inputIndex += FrameSize)
      {
        // Handle each channel separately
        for (unsigned channel = 0; channel < data.Channels; ++channel)
        {
          // If the next FrameSize chunk would go past the end of the audio data, fill in what's
          // available to the finalBuffer, the rest will be zeros
          if (inputIndex + FrameSize > data.SamplesPerChannel)
          {
            buffer = finalBuffer;
            memcpy(buffer, buffersPerChannel[channel] + inputIndex,
              (data.SamplesPerChannel - inputIndex) * sizeof(float));

            // TODO pick closest acceptable frame size instead of full size
          }
          else
            buffer = buffersPerChannel[channel] + inputIndex;

          // Encode a packet for this channel
          packetHeader.Size = opus_encode_float(encodersPerChannel[channel], buffer, FrameSize,
            encodedPacket, MaxPacketSize);
          // If there was an error, set the status 
          if (packetHeader.Size <= 0)
          {
            status.SetFailed(Zero::String::Format("Encode failed: %s\n", opus_strerror(packetHeader.Size)));
            ErrorIf(true);
            break;
          }

          ErrorIf(packetHeader.Size > MaxPacketSize);

          // Set the channel number on the header
          packetHeader.Channel = channel;

          // Write the header to the file
          outputFile.Write((byte*)&packetHeader, sizeof(packetHeader));
          // Write the encoded packet to the file
          outputFile.Write((byte*)encodedPacket, packetHeader.Size);
        }
      }
    }

    for (unsigned i = 0; i < data.Channels; ++i)
      opus_encoder_destroy(encodersPerChannel[i]);
    delete[] encodersPerChannel;
  }

}