///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef FILEDECODE_H
#define FILEDECODE_H

struct stb_vorbis;

namespace Audio
{
  //----------------------------------------------------------------------------------- File Decoder

  struct FrameSamples
  {
    FrameSamples& operator=(const FrameSamples& other) 
    { 
      memcpy(Samples, other.Samples, sizeof(float) * MaxChannels); 
      return *this;
    }

    float Samples[MaxChannels];
  };

  // Base class for threaded decoding of audio files
  class FileDecoder
  {
  public:
    FileDecoder(AudioFileTypes type, const unsigned channels, const unsigned rate, 
      const unsigned sampleBytes, const unsigned sampleCount, const unsigned dataBytes);
    virtual ~FileDecoder();

    // Decodes the specified number of samples (should be on a separate thread)
    void DecodeNextSamples(unsigned howManySamples);
    // Sets the decoder up for streaming
    void SetStreaming(const Zero::String& fileName, const Zero::FilePosition beginPosition);
    // Sets the file pointer for streaming
    virtual void SetFile(void* file) = 0;

    // The size of the buffer that should be read in for streaming
    unsigned StreamedBufferSizeInBytes;
    // Total number of samples
    const unsigned TotalSamples;
    // Number of channels
    const unsigned Channels;
    // Sample rate of the file
    const unsigned SampleRate;
    // Number of frames in the file
    const unsigned FrameCount;

  protected:
    // Un-decoded data read from the file
    byte* RawSamples;
    // Number of bytes in the RawSamples buffer
    unsigned BytesInBuffer;
    // Number of bytes per sample
    const unsigned BytesPerSample;
    // Total size of the audio data in bytes
    const unsigned DataSizeBytes;
    // Number of decoded samples to read at a time
    const unsigned SamplesToDecode;
    // The beginning of audio data in the file
    Zero::FilePosition FileDataBeginPosition;
    // Whether this is a streaming file
    bool Streaming;
    // Total bytes read in from the streaming file
    unsigned TotalBytesRead;
    // The name of the streaming file
    Zero::String FileName;

  private:
    virtual void DecodeFrame(float* samples) = 0;
    virtual bool IsFinished() = 0;
    virtual bool GetStreamingBuffer() = 0;
    virtual void ResetStreamingFile() = 0;
    virtual void CloseStreamingFile() = 0;
    virtual void ReopenStreamingFile() = 0;
    virtual bool StreamingFileIsOpen() = 0;

    LockFreeQueue<Zero::Array<float>*> DecodedBuffers;

    // Checked by the SamplesFromFile object when it is deleted
    bool FinishedDecoding;
    // Used for the FinishedDecoding variable
    Zero::ThreadLock LockObject;
    // If true, okay to delete when finished processing
    bool DeleteOnFinishing;
    // Array to hold decoded samples
    Zero::Array<float> DecodedSamples;
    // The type of this file
    AudioFileTypes Type;

    friend class SamplesFromFile;
  };

  //------------------------------------------------------------------------------------ Wav Decoder

  const static float Normalize16Bit = (1 << 15) - 1;
  const static float Normalize24Bit = (1 << 23) - 1;

  // Decodes WAV files
  class WavDecoder : public FileDecoder
  {
  public:
    WavDecoder(const unsigned channels, const unsigned rate, const unsigned sampleBytes,
      const unsigned sampleCount, const unsigned dataBytes) :
      FileDecoder(WAV_Type, channels, rate, sampleBytes, sampleCount, dataBytes),
      DecodingIndex(0)
    {}

    void SetFile(void* file) override {}
    void DecodeFrame(float* samples) override;
    bool IsFinished() override;
    bool GetStreamingBuffer() override;
    void ResetStreamingFile() override;
    void CloseStreamingFile() override;
    void ReopenStreamingFile() override;
    bool StreamingFileIsOpen() override;

  private:
    unsigned DecodingIndex;
    Zero::File StreamingFile;
  };

  //------------------------------------------------------------------------------------ Ogg Decoder

  // Decodes Ogg files
  class OggDecoder : public FileDecoder
  {
  public:
    OggDecoder(const unsigned channels, const unsigned rate, const unsigned sampleBytes,
      const unsigned sampleCount, const unsigned dataBytes) :
      FileDecoder(OGG_Type, channels, rate, sampleBytes, sampleCount, dataBytes),
      OggData(nullptr),
      SampleArray(nullptr),
      SamplesRead(0),
      TotalSamplesRead(0),
      SampleIndex(0)
    {}
    ~OggDecoder();

    void SetFile(void* file) override;
    void DecodeFrame(float* samples) override;
    bool IsFinished() override;
    bool GetStreamingBuffer() override;
    void ResetStreamingFile() override;
    void CloseStreamingFile() override;
    void ReopenStreamingFile() override;
    bool StreamingFileIsOpen() override;

  private:
    // Pointer to vorbis file data
    stb_vorbis* OggData;
    // Array of samples read in by stb_vorbis
    float **SampleArray;
    // Number of samples read this time
    int SamplesRead;
    // Total number of samples read in
    unsigned TotalSamplesRead;
    // Current index into SampleArray
    int SampleIndex;

  };

  //------------------------------------------------------------------------------ Samples From File

  class SamplesFromFile
  {
  public:
    SamplesFromFile(FileDecoder* decoder);
    ~SamplesFromFile();

    // Returns the audio sample at the specified index
    float operator[](const unsigned index);
    // Adds a buffer of raw audio 
    void SetBuffer(byte* rawSampleBuffer, const unsigned bufferSizeInBytes);
    // Returns a pointer to the decoded sample buffer
    float* GetSampleBuffer() { return DecodedSamples; }
    // Returns the type of the audio file
    AudioFileTypes GetFileType();
    // Closes the streaming file. No samples will be provided after this.
    void CloseStreamingFile();
    // Re-opens a streaming file
    void ReopenStreamingFile();
    // Resets the streaming file to the beginning
    void ResetStreamingFile();
    // Pointer to the object used for decoding raw audio data
    FileDecoder* DecodingData;

  private:
    // Buffer of decoded audio samples 
    float* DecodedSamples;
    // Last decoded sample index 
    unsigned LastAvailableIndex;
    // If true, waiting for decoder to finish a batch of samples
    bool WaitingForDecoder;
    // Samples in previous buffers
    unsigned PreviousBufferSamples;
    // If true, will reset PreviousBufferSamples to 0 next time
    bool ResetPreviousSamples;
    // Buffer of streamed samples currently being used
    Zero::Array<float> StreamedBuffer;
    // Next buffer of streamed samples 
    Zero::Array<float> NextStreamedBuffer;

    void CheckForDecodedSamples();
    void CheckForNeedingStreamedBuffer(const unsigned index);
  };
}

#endif
