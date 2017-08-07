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

  void FileEncoder::ProcessFile(Zero::Status& status, Zero::StringParam inputName, 
    Zero::StringParam outputName, unsigned& samplesPerChannel, unsigned& channels, unsigned& sampleRate)
  {
    sampleRate = AudioSystemInternal::SampleRate;

    // Open the input file
    Zero::File file;
    file.Open(inputName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);
    // If didn't open successfully, set the status and return
    if (!file.IsOpen())
    {
      status.SetFailed(Zero::String::Format("Couldn't open input file %s", inputName.c_str()));
      ErrorIf(true);
      return;
    }

    // Open the output file
    Zero::File outputFile;
    outputFile.Open(outputName, Zero::FileMode::Write, Zero::FileAccessPattern::Sequential);
    // If didn't open successfully, set the status and return
    if (!file.IsOpen())
    {
      status.SetFailed(Zero::String::Format("Couldn't create output file %s", outputName.c_str()));
      ErrorIf(true);
      return;
    }

    // Read in the header from the input file
    WavRiffHeader header;
    file.Read(status, (byte*)(&header), sizeof(header));

    byte* inputBuffer;
    float** buffersPerChannel;
    unsigned fileSampleRate;

    // RIFF file
    if (header.riff_chunk[0] == 'R' && header.riff_chunk[1] == 'I')
    {
      // Check WAV ID - if not WAVE, set status and return
      if (header.wave_fmt[0] != 'W' || header.wave_fmt[1] != 'A')
      {
        status.SetFailed(Zero::String::Format("File %s is an unreadable format", inputName.c_str()));
        ErrorIf(true);
        return;
      }

      // Read in the next chunk header
      WavChunkHeader chunkHeader;
      Zero::Status status;
      file.Read(status, (byte*)(&chunkHeader), sizeof(chunkHeader));
      // If this isn't the fmt chunk, keep looking
      while (chunkHeader.chunk_name[0] != 'f' || chunkHeader.chunk_name[1] != 'm'
        || chunkHeader.chunk_name[2] != 't')
      {
        file.Seek(chunkHeader.chunk_size, Zero::FileOrigin::Current);
        file.Read(status, (byte*)(&chunkHeader), sizeof(chunkHeader));
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
      }

      unsigned totalSamples = chunkHeader.chunk_size / fmtChunkData.bytes_per_sample / fmtChunkData.number_of_channels;
      channels = fmtChunkData.number_of_channels;
      samplesPerChannel = totalSamples / channels;
      fileSampleRate = fmtChunkData.sampling_rate;

      // Create the buffer for reading in data from the file
      inputBuffer = new byte[chunkHeader.chunk_size];

      // Read in the audio data 
      file.Read(status, inputBuffer, chunkHeader.chunk_size);

      // Create a buffer of samples for each channel
      buffersPerChannel = new float*[channels];
      for (unsigned i = 0; i < channels; ++i)
        buffersPerChannel[i] = new float[samplesPerChannel];

      PcmToFloat(inputBuffer, buffersPerChannel, totalSamples, channels, fmtChunkData.bytes_per_sample / fmtChunkData.number_of_channels);
    }
    // OGG file
    else if ((header.riff_chunk[0] == 'O' || header.riff_chunk[0] == 'o') && (header.riff_chunk[1] == 'G' || header.riff_chunk[1] == 'g'))
    {
      // Reset to beginning of file
      file.Seek(0);

      size_t fileSize = (size_t)file.CurrentFileSize();

      // Create the buffer for reading in data from the file
      inputBuffer = new byte[fileSize];

      // Read in the audio data 
      file.Read(status, inputBuffer, fileSize);

      // Create the vorbis stream
      int error;
      stb_vorbis* oggStream = stb_vorbis_open_memory((unsigned char*)inputBuffer, fileSize, &error, nullptr);

      // Get the ogg vorbis file info
      stb_vorbis_info info = stb_vorbis_get_info(oggStream);

      channels = info.channels;
      samplesPerChannel = stb_vorbis_stream_length_in_samples(oggStream);
      fileSampleRate = info.sample_rate;

      // Create a buffer of samples for each channel
      buffersPerChannel = new float*[channels];
      for (unsigned i = 0; i < channels; ++i)
        buffersPerChannel[i] = new float[samplesPerChannel];

      int samplesRead = stb_vorbis_get_samples_float(oggStream, channels, buffersPerChannel, samplesPerChannel);

      ErrorIf(samplesRead < (int)samplesPerChannel);
    }
    else
    {
      status.SetFailed(Zero::String::Format("File %s was not in WAV or OGG format", outputName.c_str()));
      ErrorIf(true);
      return;
    }

    if (fileSampleRate != AudioSystemInternal::SampleRate)
    {
      double ratio = (double)AudioSystemInternal::SampleRate / (double)fileSampleRate;
      unsigned newFrames = (unsigned)(samplesPerChannel * ratio);

      for (unsigned channel = 0; channel < channels; ++channel)
      {
        float* newSamples = new float[newFrames];
        float* oldSamples = buffersPerChannel[channel];
        unsigned oldIndex(0), newIndex(0);
        double resampleIndex(0);

        while (oldIndex + 1 < samplesPerChannel && newIndex < newFrames)
        {
          float firstSample = oldSamples[oldIndex];
          float secondSample = oldSamples[oldIndex + 1];

          newSamples[newIndex] = firstSample + (float)((secondSample - firstSample) * (resampleIndex - oldIndex));

          ++newIndex;
          resampleIndex += ratio;
          oldIndex = (unsigned)resampleIndex;
        }

        delete[] buffersPerChannel[channel];
        buffersPerChannel[channel] = newSamples;
      }

      samplesPerChannel = newFrames;
    }

    // Create the buffer for encoded packets
    unsigned char encodedPacket[MaxPacketSize];

    // Set up the file header
    FileHeader fileHeader;
    fileHeader.Channels = channels;
    fileHeader.SamplesPerChannel = samplesPerChannel;

    // Write the header to the output file
    outputFile.Write((byte*)&fileHeader, sizeof(fileHeader));

    // Create the packet header object
    PacketHeader packetHeader;

    int error;
    // Create an opus encoder for each channel
    OpusEncoder** encodersPerChannel = new OpusEncoder*[channels];
    for (unsigned i = 0; i < channels; ++i)
      encodersPerChannel[i] = opus_encoder_create(AudioSystemInternal::SampleRate, 1, OPUS_APPLICATION_AUDIO, &error);

    if (error < 0)
    {
      status.SetFailed(Zero::String::Format("Error %s\n", opus_strerror(error)));
      ErrorIf(true);
    }
    else
    {
      // Current buffer position for encoding
      float* buffer(nullptr);
      // Used on the final frame to keep the same frame size
      float finalBuffer[FrameSize] = { 0 };

      // Encode the samples, in chunks of FRAME_SIZE
      for (unsigned inputIndex = 0; inputIndex < samplesPerChannel; inputIndex += FrameSize)
      {
        for (unsigned channel = 0; channel < channels; ++channel)
        {
          if (inputIndex + FrameSize > samplesPerChannel)
          {
            buffer = finalBuffer;
            memcpy(buffer, buffersPerChannel[channel] + inputIndex, (samplesPerChannel - inputIndex) * sizeof(float));
          }
          else
            buffer = buffersPerChannel[channel] + inputIndex;

          // Encode a packet for this channel
          packetHeader.Size = opus_encode_float(encodersPerChannel[channel], buffer, FrameSize, encodedPacket, MaxPacketSize);
          // If there was an error, set the status 
          if (packetHeader.Size < 0)
          {
            status.SetFailed(Zero::String::Format("Encode failed: %s\n", opus_strerror(packetHeader.Size)));
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
}