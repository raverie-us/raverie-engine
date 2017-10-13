///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef AudioInputOutput_H
#define AudioInputOutput_H

struct PaStreamParameters;
struct PaHostApiInfo;
struct IMMDeviceEnumerator;

namespace Audio
{
  enum LatencyValues { LowLatency, HighLatency, NumLatencyValues };
  enum StreamType { OutputStream, InputStream, NumStreamTypes };
  enum StreamStatus { Uninitialized, Initialized, Started, Stopped, ApiProblem, DeviceProblem };

  inline static void LogAudioIoError(Zero::StringParam message, Zero::String* savedMessage = nullptr)
  {
    ZPrint(Zero::String::Format(message.c_str(), "\n").c_str());
    if (savedMessage)
      *savedMessage = message;
  }

  struct StreamInfo
  {
    StreamInfo() : Status(Uninitialized) {}

    StreamStatus Status;
    Zero::String ErrorMessage;
  };

  //----------------------------------------------------------------------------- Audio Input Output

  class AudioInputOutput
  {
  public:
    AudioInputOutput();
    ~AudioInputOutput();

    // Waits until another mix is needed, using semaphore counter
    void WaitUntilOutputNeededThreaded();
    // Fills the buffer with the requested number of audio samples, or the max available if lower
    void GetInputDataThreaded(Zero::Array<float>& buffer, unsigned howManySamples);
    // Sets whether the system should use a low or high latency value
    void SetOutputLatency(LatencyValues latency);
    // Returns the StreamInfo data for the specified audio stream
    virtual const StreamInfo& GetStreamInfo(StreamType whichStream);

    // Initializes the underlying audio API
    virtual StreamStatus InitializeAPI() = 0;
    // Initializes the specified audio stream
    virtual StreamStatus InitializeStream(StreamType whichStream) = 0;
    // Starts the specified audio stream
    virtual StreamStatus StartStream(StreamType whichStream) = 0;
    // Stops the specified audio stream
    virtual StreamStatus StopStream(StreamType whichStream) = 0;
    // Shuts down the specified audio stream
    virtual StreamStatus ShutDownStream(StreamType whichStream) = 0;
    // Shuts down the underlying audio API
    virtual void ShutDownAPI() = 0;
    // Returns the number of channels in the specified audio stream
    virtual unsigned GetStreamChannels(StreamType whichStream) = 0;
    // Returns the sample rate of the specified audio stream
    virtual unsigned GetStreamSampleRate(StreamType whichStream) = 0;
    // Returns true if the specified audio stream has been started
    virtual bool IsStreamStarted(StreamType whichStream) = 0;

    // Last error message pertaining to the audio API
    Zero::String LastErrorMessage;
    // Ring buffer used for mixed output
    RingBuffer OutputRingBuffer;

  protected:
    // Buffer used for the OutputRingBuffer
    float* MixedOutputBuffer;
    // The number of mix buffer frames for each latency setting
    unsigned OutputBufferSizePerLatency[NumLatencyValues];
    // Current latency setting for the audio output
    LatencyValues OutputStreamLatency;
    // Size of the buffer for input data
    static const unsigned InputBufferSize = 8192;
    // Buffer of input data
    float InputBuffer[InputBufferSize];
    // Ring buffer used for receiving input data
    RingBuffer InputRingBuffer;
    // For notifying the mix thread when a new buffer is needed.
    Zero::Semaphore MixThreadSemaphore;
    // List of info objects for each stream type
    StreamInfo StreamInfoList[NumStreamTypes];
    // The multiplier used to find the mix frames for a certain sample rate
    const float BufferSizeMultiplier = 0.04f;
    // The value used to start calculating the mix frames
    const unsigned BufferSizeStartValue = 512;

    // Sets variables and initializes output buffers at the appropriate size
    void InitializeOutputBuffers();
    // Creates output buffers using the specified size
    void SetUpOutputBuffers();
    // Gets the mixed buffer that is ready to output
    void GetMixedOutputSamples(float* outputBuffer, const unsigned howManySamples);
    // Saves the input buffer from the microphone
    void SaveInputSamples(const float* inputBuffer, unsigned howManySamples);
  };

  //------------------------------------------------------------- Audio Input Output using PortAudio
  
  class AudioIOPortAudio : public AudioInputOutput
  {
  public:
    AudioIOPortAudio();
    ~AudioIOPortAudio();

    // Initializes the underlying audio API
    StreamStatus InitializeAPI() override;
    // Initializes the specified audio stream
    StreamStatus InitializeStream(StreamType whichStream) override;
    // Starts the specified audio stream
    StreamStatus StartStream(StreamType whichStream) override;
    // Stops the specified audio stream
    StreamStatus StopStream(StreamType whichStream) override;
    // Shuts down the specified audio stream
    StreamStatus ShutDownStream(StreamType whichStream) override;
    // Shuts down the underlying audio API
    void ShutDownAPI() override;
    // Returns the number of channels in the specified audio stream
    unsigned GetStreamChannels(StreamType whichStream) override;
    // Returns the sample rate of the specified audio stream
    unsigned GetStreamSampleRate(StreamType whichStream) override;
    // Returns true if the specified audio stream has been started
    bool IsStreamStarted(StreamType whichStream) override;

    int HandleCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer);

  private:
    const PaHostApiInfo* ApiInfo;

    // Pointer to the Port Audio output stream
    void* PaOutputStream;
    // Port audio output stream parameters
    PaStreamParameters* OutputParameters;
    // Sample rate of the audio output
    unsigned OutputSampleRate;

    // Pointer to the Port Audio input stream
    void* PaInputStream;
    // Port audio input stream parameters
    PaStreamParameters* InputParameters;
    // Sample rate of the audio input
    unsigned InputSampleRate;

    void InitializeInput();
    void InitializeOutput();
  };
}

#endif
