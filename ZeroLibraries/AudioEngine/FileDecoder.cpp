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
  //----------------------------------------------------------------------------------- File Decoder

  //************************************************************************************************
  FileDecoder::FileDecoder(AudioFileTypes type, const unsigned channels, const unsigned rate, 
      const unsigned sampleBytes, const unsigned sampleCount, const unsigned dataBytes) :
    RawSamples(nullptr),
    BytesInBuffer(0),
    BytesPerSample(sampleBytes),
    DeleteOnFinishing(false),
    Channels(channels),
    TotalSamples(sampleCount),
    SampleRate(rate),
    FrameCount(sampleCount / channels),
    DataSizeBytes(dataBytes),
    SamplesToDecode(gAudioSystem->SystemSampleRate * channels / 2),
    FileDataBeginPosition(0),
    FinishedDecoding(true),
    Type(type),
    Streaming(false),
    TotalBytesRead(0)
  {
    StreamedBufferSizeInBytes = SamplesToDecode;
    if (BytesPerSample > 0)
      StreamedBufferSizeInBytes *= BytesPerSample;
  }

  //************************************************************************************************
  FileDecoder::~FileDecoder()
  {
    if (RawSamples)
      delete[] RawSamples;
  }

  //************************************************************************************************
  void FileDecoder::DecodeNextSamples(unsigned howManySamples)
  {
    LockObject.Lock();
    FinishedDecoding = false;
    LockObject.Unlock();

    FrameSamples frame;
    // Decode the desired number of samples, a frame at a time
    for (unsigned i = 0; i < howManySamples && !IsFinished(); i += Channels)
    {
      // Get a frame of audio samples
      DecodeFrame(frame.Samples);

      // Add the samples to the buffer
      for (unsigned j = 0; j < Channels; ++j)
        DecodedSamples.PushBack(frame.Samples[j]);
    }

    // Add the finished buffer to the queue
    DecodedBuffers.Write(&DecodedSamples);

    LockObject.Lock();
    FinishedDecoding = true;
    LockObject.Unlock();

    // This will be true if the parent object has been deleted
    if (DeleteOnFinishing)
      delete this;
  }

  //************************************************************************************************
  void FileDecoder::SetStreaming(const Zero::String& fileName, const Zero::FilePosition beginPosition)
  {
    Streaming = true;
    FileDataBeginPosition = beginPosition;
    FileName = fileName;
  }

  //------------------------------------------------------------------------------------ WAV Decoder

  //************************************************************************************************
  void WavDecoder::DecodeFrame(float* samples)
  {
    // Check if we've already decoded the whole file
    if (IsFinished())
      return;

    for (unsigned i = 0; i < Channels; ++i)
    {
      // Adjust index for bytes
      unsigned adjIndex = DecodingIndex++ * BytesPerSample;

      // Translate 16 bit data to a float
      if (BytesPerSample == 2)
      {
        // Get first byte
        int sample = RawSamples[adjIndex];
        // Get second byte
        sample |= (int)RawSamples[adjIndex + 1] << 8;
        // Account for negative numbers
        if ((unsigned char)(RawSamples[adjIndex + 1] & 0x80) == 0x80)
          sample |= 0xffff0000;

        // Save normalized value
        samples[i] = (float)sample / Normalize16Bit;
      }
      // Translate 24 bit data to a float
      else if (BytesPerSample == 3)
      {
        // Get first byte
        int sample = RawSamples[adjIndex];
        // Get next two bytes
        memcpy(&sample, RawSamples + adjIndex, sizeof(char) * 3);
        // Account for negative numbers
        if (sample & 0x800000)
          sample |= 0xff000000;
        else
          sample &= 0x00ffffff;

        // Save normalized value
        samples[i] = (float)sample / Normalize24Bit;
      }
    }
  }

  //************************************************************************************************
  bool WavDecoder::IsFinished()
  {
    return DecodingIndex >= BytesInBuffer / BytesPerSample;
  }

  //************************************************************************************************
  bool WavDecoder::GetStreamingBuffer()
  {
    Zero::Status status;
    BytesInBuffer = StreamingFile.Read(status, RawSamples, StreamedBufferSizeInBytes);
    TotalBytesRead += BytesInBuffer;

    // Check if we're at the end of the file
    if (BytesInBuffer == 0 || TotalBytesRead >= DataSizeBytes)
    {
      // Reset the file to the beginning
      StreamingFile.Seek(FileDataBeginPosition);
      // Reset index to beginning of buffer
      DecodingIndex = 0;
      // Reset bytes read counter
      TotalBytesRead = 0;

      return true;
    }
    else
    {
      // Reset index to beginning of buffer
      DecodingIndex = 0;

      return false;
    }
  }

  //************************************************************************************************
  void WavDecoder::ResetStreamingFile()
  {
    TotalBytesRead = 0;
    DecodingIndex = 0;
    StreamingFile.Seek(FileDataBeginPosition);

    Zero::Status status;
    BytesInBuffer = StreamingFile.Read(status, RawSamples, StreamedBufferSizeInBytes);
    TotalBytesRead += BytesInBuffer;
  }

  //************************************************************************************************
  void WavDecoder::CloseStreamingFile()
  {
    if (StreamingFile.IsOpen())
    {
      StreamingFile.Close();
    }
  }

  //************************************************************************************************
  void WavDecoder::ReopenStreamingFile()
  {
    StreamingFile.Open(FileName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);

    ErrorIf(!StreamingFile.IsOpen(), "Audio Engine: Problem re-opening streaming file");

    TotalBytesRead = 0;
    DecodingIndex = 0;
    BytesInBuffer = 0;
  }

  //************************************************************************************************
  bool WavDecoder::StreamingFileIsOpen()
  {
    return StreamingFile.IsOpen();
  }

  //------------------------------------------------------------------------------------ Ogg Decoder

  //************************************************************************************************
  OggDecoder::~OggDecoder()
  {
    if (OggData)
      stb_vorbis_close(OggData);
  }

  //************************************************************************************************
  void OggDecoder::SetFile(void* file)
  {
    OggData = (stb_vorbis*)file;
  }

  //************************************************************************************************
  void OggDecoder::DecodeFrame(float* samples)
  {
    // Already read entire file or file isn't open
    if (IsFinished() || !OggData)
      return;

    // No more samples available, read another chunk
    if (SampleIndex >= SamplesRead)
    {
      SampleArray = nullptr;
      SampleIndex = 0;

      SamplesRead = stb_vorbis_get_frame_float(OggData, nullptr, &SampleArray);
      TotalSamplesRead += SamplesRead * Channels;
    }

    // If there are samples, copy them into array to return
    if (SampleArray)
    {
      for (unsigned i = 0; i < Channels; ++i)
      {
        samples[i] = SampleArray[i][SampleIndex];
      }
      ++SampleIndex;
    }

  }

  //************************************************************************************************
  bool OggDecoder::IsFinished()
  {
    return TotalSamplesRead - (SamplesRead - SampleIndex) >= TotalSamples;
  }

  //************************************************************************************************
  bool OggDecoder::GetStreamingBuffer()
  {
    // Check if we are at the end of the file
    if (SamplesRead == 0 || TotalSamplesRead >= TotalSamples)
    {
      ResetStreamingFile();

      return true;
    }
    else
      return false;
  }

  //************************************************************************************************
  void OggDecoder::ResetStreamingFile()
  {
    stb_vorbis_seek_start(OggData);
    TotalSamplesRead = 0;
    SamplesRead = 0;
    SampleIndex = 0;
  }

  //************************************************************************************************
  void OggDecoder::CloseStreamingFile()
  {
    if (Streaming && OggData)
    {
      stb_vorbis_close(OggData);
      OggData = nullptr;

      SamplesRead = 0;
      TotalSamplesRead = 0;
      SampleIndex = 0;
    }
  }

  //************************************************************************************************
  void OggDecoder::ReopenStreamingFile()
  {
    int error;
    OggData = stb_vorbis_open_filename(const_cast<char*>(FileName.c_str()), &error, nullptr);

    ErrorIf(!OggData, "Audio Engine: Problem re-opening streaming OGG file");
  }

  //************************************************************************************************
  bool OggDecoder::StreamingFileIsOpen()
  {
    return OggData != nullptr;
  }

  //------------------------------------------------------------------------------ Samples From File

  //************************************************************************************************
  SamplesFromFile::SamplesFromFile(FileDecoder* decoder) :
    DecodedSamples(nullptr),
    LastAvailableIndex(0),
    DecodingData(decoder),
    WaitingForDecoder(false),
    PreviousBufferSamples(0),
    ResetPreviousSamples(false)
  {

  }

  //************************************************************************************************
  SamplesFromFile::~SamplesFromFile()
  {
    // Check if we're waiting for the decoder and it's not finished
    // (have to let it finish executing function on decoder thread)
    if (WaitingForDecoder)
    {
      DecodingData->LockObject.Lock();
      // Not finished yet, mark to delete when done
      if (!DecodingData->FinishedDecoding)
      {
        DecodingData->DeleteOnFinishing = true;
        DecodingData->LockObject.Unlock();
      }
      // Finished, can delete now
      else
        delete DecodingData;
    }
    // Not waiting, go ahead and delete it
    else
      delete DecodingData;

    if (DecodedSamples)
      delete[] DecodedSamples;
  }

  //************************************************************************************************
  float SamplesFromFile::operator[](const unsigned index)
  {
    // Check if there are decoded samples available 
    CheckForDecodedSamples();

    // Check if we are streaming and need a new buffer
    CheckForNeedingStreamedBuffer(index);

    // If not streaming and index is available, return sample at that index
    if (!DecodingData->Streaming)
    {
      if (index > LastAvailableIndex)
        return 0.0f;
      else
        return DecodedSamples[index];
    }
    // If streaming and index is available, return sample at adjusted index
    else
    {
      if (index - PreviousBufferSamples >= StreamedBuffer.Size())
        return 0.0f;
      else
        return StreamedBuffer[index - PreviousBufferSamples];
    }
  }

  //************************************************************************************************
  void SamplesFromFile::SetBuffer(byte* rawSampleBuffer, const unsigned bufferSizeInBytes)
  {
    DecodingData->RawSamples = rawSampleBuffer;
    DecodingData->BytesInBuffer = bufferSizeInBytes;
    DecodingData->TotalBytesRead = bufferSizeInBytes;

    if (!DecodingData->Streaming)
      DecodedSamples = new float[DecodingData->TotalSamples];

    if (!DecodingData->Streaming)
    {
      WaitingForDecoder = true;

      gAudioSystem->AddDecodingTask(Zero::CreateFunctor(&FileDecoder::DecodeNextSamples, DecodingData,
        DecodingData->SamplesToDecode));
    }
  }

  //************************************************************************************************
  Audio::AudioFileTypes SamplesFromFile::GetFileType()
  {
    return DecodingData->Type;
  }

  //************************************************************************************************
  void SamplesFromFile::CloseStreamingFile()
  {
    if (!DecodingData->Streaming)
      return;

    // Clear out any existing decoded buffers
    if (WaitingForDecoder)
    {
      Zero::Array<float>* buffer;
      while (!DecodingData->DecodedBuffers.Read(buffer))
      {

      }

      WaitingForDecoder = false;
    }

    DecodingData->CloseStreamingFile();
  }

  //************************************************************************************************
  void SamplesFromFile::ReopenStreamingFile()
  {
    if (!DecodingData->Streaming || DecodingData->StreamingFileIsOpen())
      return;

    DecodingData->ReopenStreamingFile();

    ResetStreamingFile();
  }

  //************************************************************************************************
  void SamplesFromFile::CheckForDecodedSamples()
  {
    // Are we waiting for the decoder?
    if (WaitingForDecoder)
    {
      Zero::Array<float>* buffer;
      // Check if there is a finished buffer
      if (DecodingData->DecodedBuffers.Read(buffer))
      {
        if (!DecodingData->Streaming)
        {
          // Copy decoded samples 
          memcpy(DecodedSamples + LastAvailableIndex, buffer->Data(), sizeof(float) * buffer->Size());
          // Advance last available index
          LastAvailableIndex += buffer->Size();

          // Are there more samples to decode?
          if (LastAvailableIndex < DecodingData->TotalSamples)
          {
            // Clear the buffer before it's reused
            buffer->Clear();
            // Send another task to the decoding thread
            gAudioSystem->AddDecodingTask(Zero::CreateFunctor(&FileDecoder::DecodeNextSamples, 
              DecodingData, DecodingData->SamplesToDecode));
          }
          else
            WaitingForDecoder = false;
        }
        else
        {
          // Move the buffer's data (clears the other array)
          NextStreamedBuffer = Zero::MoveReference<Zero::Array<float>>(*buffer);

          WaitingForDecoder = false;
        }
      }
    }
  }

  //************************************************************************************************
  void SamplesFromFile::CheckForNeedingStreamedBuffer(const unsigned index)
  {
    // Are we streaming, not waiting, and at the end of the buffer?
    if (DecodingData->Streaming && DecodingData->StreamingFileIsOpen() && !WaitingForDecoder 
      && (index < PreviousBufferSamples || index - PreviousBufferSamples >= StreamedBuffer.Size()))
    {
      // Move the next buffer to the current buffer
      if (ResetPreviousSamples)
      {
        PreviousBufferSamples = 0;
        ResetPreviousSamples = false;
      }
      else
        PreviousBufferSamples += StreamedBuffer.Size();

      // Move the buffer's data (clears the other array)
      StreamedBuffer = Zero::MoveReference<Zero::Array<float>>(NextStreamedBuffer);

      ResetPreviousSamples = DecodingData->GetStreamingBuffer();

      DecodingData->FinishedDecoding = false;

      // Mark that we are now waiting
      WaitingForDecoder = true;
      // Send another task to the decoding thread
      gAudioSystem->AddDecodingTask(Zero::CreateFunctor(&FileDecoder::DecodeNextSamples, 
        DecodingData, DecodingData->SamplesToDecode));
    }
  }

  //************************************************************************************************
  void SamplesFromFile::ResetStreamingFile()
  {
    if (!DecodingData->Streaming)
      return;

    // Clear out any existing decoded buffers
    if (WaitingForDecoder)
    {
      Zero::Array<float>* buffer;
      while (!DecodingData->DecodedBuffers.Read(buffer))
      {

      }
    }

    // Clear the saved buffers of samples
    StreamedBuffer.Clear();
    NextStreamedBuffer.Clear();

    ResetPreviousSamples = true;

    DecodingData->ResetStreamingFile();

    // Send another task to the decoding thread
    gAudioSystem->AddDecodingTask(Zero::CreateFunctor(&FileDecoder::DecodeNextSamples, 
      DecodingData, DecodingData->SamplesToDecode));
    WaitingForDecoder = true;
  }

}