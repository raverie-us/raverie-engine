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

namespace Audio
{
  static void SetStatusAndLog(Zero::Status& status, Zero::StringParam message)
  {
    status.SetFailed(message);
    ZPrint(message.c_str());
    ZPrint("\n");
  }

  //----------------------------------------------------------------------------- Audio Input Output

  class AudioInputOutput
  {
  public:
    AudioInputOutput();
    ~AudioInputOutput();

    // Returns the current buffer to write output to
    float* GetOutputBuffer();
    // Advances the buffer index
    void FinishedMixingBuffer();
    // Waits until another mix is needed, using semaphore counter
    void WaitUntilOutputNeeded();
    // Initializes audio input and output
    void Initialize(Zero::Status& status);
    // Shuts down all audio output
    void ShutDown(Zero::Status& status);
    // Resets the audio output (should call on device change)
    void Reset(Zero::Status& status);
    // Returns the buffer sized based on current latency and the supplied sample rate
    unsigned GetBufferSize(unsigned sampleRate);
    // Fills the buffer with the requested number of audio samples, or the max available if lower
    void GetInputData(Zero::Array<float>& buffer, unsigned howManySamples);
    // Sets whether the system should use a low or high latency value
    void SetLatency(bool lowLatency);

    // Returns the number of channels in the audio output
    virtual unsigned GetOutputChannels() = 0;
    // Returns the sample rate of the audio output
    virtual unsigned GetOutputSampleRate() = 0;
    // Returns the number of channels in the audio input
    virtual unsigned GetInputChannels() = 0;
    // Returns the sample rate of the audio input
    virtual unsigned GetInputSampleRate() = 0;
    // Returns true if the audio output stream is currently running
    virtual bool IsOutputStreamOpen() = 0;
    // Returns true if the audio input stream is currently running
    virtual bool IsInputStreamOpen() = 0;
    // Starts the audio output stream
    virtual void StartOutputStream(Zero::Status& status) = 0;
    // Stops the audio output stream
    virtual void StopOutputStream(Zero::Status& status) = 0;
    // Starts the audio input stream
    virtual void StartInputStream(Zero::Status& status) = 0;
    // Stops the audio input stream
    virtual void StopInputStream(Zero::Status& status) = 0;

    // Size of the output buffer
    unsigned OutputBufferSizeThreaded;

    static const unsigned NumOutputBuffers = 3;

  protected:
    unsigned BufferBaseSize;
    unsigned BufferLargeSize;

    // Index for current output writing buffer, used for mixing output
    int WriteBufferThreaded;
    // Index for current output reading buffer, used to send samples to Port Audio.
    int ReadBufferThreaded;
    // Array of three buffers for output
    float* OutputBuffersThreaded[NumOutputBuffers];
    // If true, currently set to low latency
    bool LowLatency;
    // Size of the buffer for input data
    static const unsigned InputBufferSize = 8192;
    // Buffer of input data
    float InputBuffer[InputBufferSize];
    // Ring buffer used for receiving input data
    PaUtilRingBuffer InputRingBuffer;

    // Current position in the output buffer.
    unsigned MixedBufferIndex;
    // For notifying the mix thread when a new buffer is needed.
    Zero::Semaphore Counter;

    const float SmallBufferMultiplier = 0.01f;
    const float LargeBufferMultiplier = 0.04f;
    const unsigned BufferSizeStartValue = 128;

    // Sets variables and creates output buffers using the specified size
    void SetUpOutputBuffers(const unsigned size);
    // Sets the callback frame size and calls SetUpBuffers
    void SetLatency(const unsigned baseSize);
    // Gets the mixed buffer that is ready to output
    void GetMixedOutputSamples(float* outputBuffer, const unsigned howManySamples);
    // Saves the input buffer from the microphone
    void SaveInputSamples(const float* inputBuffer, unsigned howManySamples);

    // Initializes the audio API 
    virtual void InitializeAPI(Zero::Status& status) = 0;
    // Shuts down the audio API
    virtual void ShutDownAPI(Zero::Status& status) = 0;
  };

  //---------------------------------------------------------------- Audio Input Output using WASAPI
  
  class WasapiDeviceInfo;

  class AudioIOWindows : public AudioInputOutput
  {
  public:
    AudioIOWindows();
    ~AudioIOWindows();

    // Returns the number of channels in the audio output
    unsigned GetOutputChannels() override;
    // Returns the sample rate of the audio output
    unsigned GetOutputSampleRate() override;
    // Returns the number of channels in the audio input
    unsigned GetInputChannels() override;
    // Returns the sample rate of the audio input
    unsigned GetInputSampleRate() override;
    // Returns true if the audio output stream is currently running
    bool IsOutputStreamOpen() override;
    // Returns true if the audio input stream is currently running
    bool IsInputStreamOpen() override;
    // Starts the audio output stream
    void StartOutputStream(Zero::Status& status) override;
    // Stops the audio output stream
    void StopOutputStream(Zero::Status& status) override;
    // Starts the audio input stream
    void StartInputStream(Zero::Status& status) override;
    // Stops the audio input stream
    void StopInputStream(Zero::Status& status) override;
    // Initializes the audio API 
    void InitializeAPI(Zero::Status& status) override;
    // Shuts down the audio API
    void ShutDownAPI(Zero::Status& status) override;

    void HandleCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer);

  private:
    WasapiDeviceInfo* OutputDevice;
    WasapiDeviceInfo* InputDevice;

    static unsigned _stdcall StartOutputThread(void* param);
    static unsigned _stdcall StartInputThread(void* param);
  };

  //------------------------------------------------------------- Audio Input Output using PortAudio
  
  class AudioIOPortAudio : public AudioInputOutput
  {
  public:
    AudioIOPortAudio();
    ~AudioIOPortAudio();

    // Returns the number of channels in the audio output
    unsigned GetOutputChannels() override;
    // Returns the sample rate of the audio output
    unsigned GetOutputSampleRate() override;
    // Returns the number of channels in the audio input
    unsigned GetInputChannels() override;
    // Returns the sample rate of the audio input
    unsigned GetInputSampleRate() override;
    // Returns true if the audio output stream is currently running
    bool IsOutputStreamOpen() override;
    // Returns true if the audio input stream is currently running
    bool IsInputStreamOpen() override;
    // Starts the audio output stream
    void StartOutputStream(Zero::Status& status) override;
    // Stops the audio output stream
    void StopOutputStream(Zero::Status& status) override;
    // Starts the audio input stream
    void StartInputStream(Zero::Status& status) override;
    // Stops the audio input stream
    void StopInputStream(Zero::Status& status) override;
    // Initializes the audio API 
    void InitializeAPI(Zero::Status& status) override;
    // Shuts down the audio API
    void ShutDownAPI(Zero::Status& status) override;

    int HandleCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer);

  private:
    // Pointer to the Port Audio output stream
    void* OutputStream;
    // Port audio output stream parameters
    PaStreamParameters* OutputParameters;
    // Sample rate of the audio output
    unsigned OutputSampleRate;

    // Pointer to the Port Audio input stream
    void* InputStream;
    // Port audio input stream parameters
    PaStreamParameters* InputParameters;
    // Sample rate of the audio input
    unsigned InputSampleRate;
  };
}

#endif
