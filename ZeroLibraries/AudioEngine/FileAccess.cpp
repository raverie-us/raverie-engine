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









#define FRAME_SIZE 960 // 20 ms of audio data
#define SAMPLE_RATE 48000
#define MAX_PACKET_SIZE (3 * 1276)


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


  void FileAccess::ProcessFile(Zero::Status& status, Zero::StringParam inputName, Zero::StringParam outputName)
  {
    // Open the input file
    Zero::File file;
    file.Open(inputName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);
    // If didn't open successfully, set the status and return
    if (!file.IsOpen())
    {
      status.SetFailed(Zero::String::Format("Couldn't open input file %s", inputName.c_str()));
      return;
    }

    // Open the output file
    Zero::File outputFile;
    outputFile.Open(outputName, Zero::FileMode::Write, Zero::FileAccessPattern::Sequential);
    // If didn't open successfully, set the status and return
    if (!file.IsOpen())
    {
      status.SetFailed(Zero::String::Format("Couldn't create output file %s", outputName.c_str()));
      return;
    }

    // Read in the header from the input file
    WavRiffHeader header;
    file.Read(status, (byte*)(&header), sizeof(header));

    AudioData data;
    unsigned audioFrames;

    // Check WAV ID - if not WAVE, set status and return
    if (header.wave_fmt[0] != 'W' || header.wave_fmt[1] != 'A')
    {
      status.SetFailed(Zero::String::Format("File %s is an unreadable format", inputName.c_str()));
      return;
    }

    // Get the data about the file
    GetWavData(&file, data);

    audioFrames = data.TotalSamples / data.Channels;

    // Create the buffer for reading in data from the file
    short* inputBuffer = new short[data.TotalSamples];

    // Read in the audio data (file will be in correct position after GetWavData)
    file.Read(status, (byte*)inputBuffer, data.TotalDataSize);

    // Create a buffer of samples for each channel
    float** samplesPerChannel = new float*[data.Channels];
    for (unsigned i = 0; i < data.Channels; ++i)
      samplesPerChannel[i] = new float[audioFrames];

    // Translate the data into floats and split into channel buffers
    for (unsigned i = 0; i < audioFrames; ++i)
    {
      for (unsigned j = 0; j < data.Channels; ++j)
        samplesPerChannel[j][i] = (float)inputBuffer[(i * data.Channels) + j] / MAXSHORT;
    }
    
    if (data.SampleRate != 48000)
    {
      // Resample the file
    }

    // Create the buffer for encoded packets
    unsigned char encodedPacket[MAX_PACKET_SIZE];

    // Set up the file header
    FileHeader fileHeader;
    fileHeader.Channels = data.Channels;
    fileHeader.SamplesPerChannel = audioFrames;

    // Write the header to the output file
    outputFile.Write((byte*)&fileHeader, sizeof(fileHeader));

    // Create the packet header object
    PacketHeader packetHeader;

    int error;
    // Create an opus encoder for each channel
    OpusEncoder** encodersPerChannel = new OpusEncoder*[data.Channels];
    for (unsigned i = 0; i < data.Channels; ++i)
      encodersPerChannel[i] = opus_encoder_create(SAMPLE_RATE, 1, OPUS_APPLICATION_AUDIO, &error);

    if (error < 0)
    {
      status.SetFailed(Zero::String::Format("Error %s\n", opus_strerror(error)));
    }
    else
    {
      // Current buffer position for encoding
      float* buffer(nullptr);
      // Used on the final frame to keep the same frame size
      float finalBuffer[FRAME_SIZE] = { 0 };

      // Encode the samples, in chunks of FRAME_SIZE
      for (unsigned inputIndex = 0; inputIndex < audioFrames; inputIndex += FRAME_SIZE)
      {
        for (unsigned channel = 0; channel < data.Channels; ++channel)
        {
          if (inputIndex + FRAME_SIZE > audioFrames)
          {
            buffer = finalBuffer;
            memcpy(buffer, samplesPerChannel[channel] + inputIndex, (audioFrames - inputIndex) * sizeof(float));
          }
          else
            buffer = samplesPerChannel[channel] + inputIndex;

          // Encode a packet for this channel
          packetHeader.Size = opus_encode_float(encodersPerChannel[channel], buffer, FRAME_SIZE, encodedPacket, MAX_PACKET_SIZE);
          // If there was an error, set the status 
          if (packetHeader.Size < 0)
          {
            status.SetFailed(Zero::String::Format("Encode failed: %s\n", opus_strerror(packetHeader.Size)));
            break;
          }

          ErrorIf(packetHeader.Size > MAX_PACKET_SIZE);

          // Set the channel number on the header
          packetHeader.Channel = data.Channels;

          // Write the header to the file
          outputFile.Write((byte*)&packetHeader, sizeof(packetHeader));
          // Write the encoded packet to the file
          outputFile.Write((byte*)encodedPacket, packetHeader.Size);
        }
      }
    }
  }

  void FileAccess::OpenFile(Zero::Status& status, Zero::StringParam fileName, float*& decodedSamples, unsigned& channels, unsigned& samplesPerChannel)
  {
    // Open the file
    Zero::File inputFile;
    inputFile.Open(fileName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);
    // If failed, set the status and return
    if (!inputFile.IsOpen())
    {
      status.SetFailed(Zero::String::Format("Couldn't open input file %s", fileName.c_str()));
      return;
    }

    // Create the file header object
    FileHeader fileHeader;
    // Read in the header from the file
    inputFile.Read(status, (byte*)&fileHeader, sizeof(fileHeader));
    // If the file is not in the correct format, set the status and return
    if (fileHeader.Name[0] != 'Z' || fileHeader.Name[1] != 'E')
    {
      status.SetFailed(Zero::String::Format("File %s is in an incorrect format", fileName.c_str()));
      return;
    }

    // Set the variables
    channels = fileHeader.Channels;
    samplesPerChannel = fileHeader.SamplesPerChannel;

    // Create a buffer per channel for decoded frames
    float** decodedFramePerChannel = new float*[channels];
    for (unsigned i = 0; i < channels; ++i)
      decodedFramePerChannel[i] = new float[FRAME_SIZE];

    // Create the buffer for the interleaved decoded samples
    decodedSamples = new float[fileHeader.SamplesPerChannel * fileHeader.Channels];

    // Create buffer for reading in a packet from the file
    unsigned char packet[MAX_PACKET_SIZE];

    int error;
    // Create a decoder for each channel
    OpusDecoder** decodersPerChannel = new OpusDecoder*[channels];
    for (unsigned i = 0; i < channels; ++i)
      decodersPerChannel[i] = opus_decoder_create(SAMPLE_RATE, 1, &error);

    PacketHeader packetHeader;
    unsigned decodedBufferIndex(0);
    unsigned samplesRead(0);
    int frameSize(0);

    while (samplesRead < samplesPerChannel)
    {
      for (unsigned channel = 0; channel < channels; ++channel)
      {
        // Read in the packet header
        inputFile.Read(status, (byte*)&packetHeader, sizeof(packetHeader));

        ErrorIf(packetHeader.Name[0] != 'p' || packetHeader.Name[1] != 'a');
        ErrorIf(packetHeader.Size > MAX_PACKET_SIZE);

        // Read in the packet, using the size from the header
        inputFile.Read(status, (byte*)packet, packetHeader.Size);

        frameSize = opus_decode_float(decodersPerChannel[channel], packet, packetHeader.Size, decodedFramePerChannel[channel], FRAME_SIZE, 0);

        ErrorIf(frameSize < 0);
      }

      for (int i = 0; i < frameSize && decodedBufferIndex < samplesPerChannel * channels; ++i)
      {
        for (unsigned channel = 0; channel < channels; ++channel, ++decodedBufferIndex)
          decodedSamples[decodedBufferIndex] = decodedFramePerChannel[channel][i];
      }


      samplesRead += frameSize;
    }

  }

  void FileAccess::RunTest()
  {
    Zero::File file;
    file.Open("C:\\Users\\Andrea Ellinger\\Desktop\\Run.wav", Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);
    ErrorIf(!file.IsOpen());

    Zero::Status status;
    WavRiffHeader header;
    file.Read(status, (byte*)(&header), sizeof(header));

    // Read in the next chunk header
    WavChunkHeader fmtChunkHeader;
    file.Read(status, (byte*)(&fmtChunkHeader), sizeof(fmtChunkHeader));
    // If this isn't the fmt chunk, keep looking
    while (fmtChunkHeader.chunk_name[0] != 'f' || fmtChunkHeader.chunk_name[1] != 'm'
      || fmtChunkHeader.chunk_name[2] != 't')
    {
      file.Seek(fmtChunkHeader.chunk_size, Zero::FileOrigin::Current);
      file.Read(status, (byte*)(&fmtChunkHeader), sizeof(fmtChunkHeader));
    }

    // Read in the fmt chunk data
    WavFmtData fmtChunkData;
    file.Read(status, (byte*)(&fmtChunkData), sizeof(fmtChunkData));

    // If the chunk size is larger than the WavFmtData struct, skip ahead
    if (fmtChunkHeader.chunk_size > sizeof(fmtChunkData))
      file.Seek(fmtChunkHeader.chunk_size - sizeof(fmtChunkData), Zero::FileOrigin::Current);

    // Get the data chunk header
    WavChunkHeader dataChunkHeader;
    file.Read(status, (byte*)(&dataChunkHeader), sizeof(dataChunkHeader));
    // If this isn't the data chunk, keep looking
    while (dataChunkHeader.chunk_name[0] != 'd' || dataChunkHeader.chunk_name[1] != 'a'
      || dataChunkHeader.chunk_name[2] != 't')
    {
      file.Seek(dataChunkHeader.chunk_size, Zero::FileOrigin::Current);
      file.Read(status, (byte*)(&dataChunkHeader), sizeof(dataChunkHeader));
    }

    file.Close();

    ProcessFile(status, "C:\\Users\\Andrea Ellinger\\Desktop\\Run.wav", "C:\\Users\\Andrea Ellinger\\Desktop\\Run.snd");

    float* floatSamples(nullptr);
    unsigned channels, samples;
    OpenFile(status, "C:\\Users\\Andrea Ellinger\\Desktop\\Run.snd", floatSamples, channels, samples);

    short* shortSamples = new short[samples * channels];
    for (unsigned i = 0; i < samples * channels; ++i)
      shortSamples[i] = (short)(floatSamples[i] * MAXSHORT);

    Zero::File outFile;
    outFile.Open("C:\\Users\\Andrea Ellinger\\Desktop\\RunOutput.wav", Zero::FileMode::Write, Zero::FileAccessPattern::Sequential);

    outFile.Write((byte*)&header, sizeof(header));
    outFile.Write((byte*)&fmtChunkHeader, sizeof(fmtChunkHeader));
    outFile.Write((byte*)&fmtChunkData, sizeof(fmtChunkData));
    outFile.Write((byte*)&dataChunkHeader, sizeof(dataChunkHeader));

    outFile.Write((byte*)shortSamples, samples * channels * sizeof(short));

    delete[] floatSamples;
    delete[] shortSamples;
  }

}