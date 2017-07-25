///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef AudioInputOutput_H
#define AudioInputOutput_H

namespace Audio
{
  // Will be called by Port Audio when audio data is needed.
  // May be called at interrupt level on some machines so can't do anything time consuming.
  static int PACallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);
  
  //--------------------------------------------------------------------------- Callback Data Object

  class CallbackDataObject
  {
  public:
    CallbackDataObject() : 
      Index(0), 
      ReadBufferThreaded(1),
      Channels(0),
      BufferSize(0)
    {}

    // Returns the current buffer for reading
    float* GetReadBuffer();

    // Current position in the output buffer.
    unsigned Index;
    // Index for current output reading buffer, used to send samples to Port Audio.
    int ReadBufferThreaded;
    // For notifying the mix thread when a new buffer is needed.
    Zero::Semaphore Counter;
    // Number of channels being used for output.
    unsigned Channels;
    // Size of output buffers.
    unsigned BufferSize;
  };

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
    // Initializes audio output
    void Initialize(Zero::Status& status);
    // Shuts down all audio output
    void ShutDown(Zero::Status& status);
    // Resets the audio output (should call on device change)
    void Reset();
    // Stops the output stream without shutting down
    void StopStream();
    // Re-starts the output stream after being stopped
    void StartStream();
    // Returns true if the output stream is currently open
    bool IsStreamOpen();
    // Starts the input stream
    void StartInputStream();
    // Stops the input stream
    void StopInputStream();
    // Starts or re-starts the output stream with the specified latency values
    void RestartStream(const bool lowLatency, Zero::Status& status);
    // Returns the base buffer size for the current latency
    unsigned GetBaseBufferSize();

    // Size of the buffer for input data
    static const unsigned InputBufferSize = 8192;
    // Ring buffer used for receiving input data
    PaUtilRingBuffer InputRingBuffer;
    // Buffer of input data
    float InputBuffer[InputBufferSize];
    // Parameters for input data stream
    PaStreamParameters InputParameters;
    // Sample rate of input data
    unsigned InputSampleRate;
    // Number of channels in the output
    unsigned OutputChannelsThreaded;
    // Size of the output buffer
    unsigned OutputBufferSizeThreaded;

    static const unsigned NumOutputBuffers = 3;

  private:
    static const unsigned BufferBaseSize = 512;
    static const unsigned BufferLargeSize = 2048;
    
    // Index for current output writing buffer, used for mixing output
    int WriteBufferThreaded;
    // Array of three buffers for output
    float* OutputBuffersThreaded[NumOutputBuffers];
    // Number of frames handed off to Port Audio at a time
    unsigned CallbackFrameSizeThreaded;
    // Object for communication with the Port Audio callback function
    CallbackDataObject CallbackDataThreaded;
    // Pointer to the Port Audio output stream
    PaStream *Stream;
    // Port audio output stream parameters
    PaStreamParameters OutputParameters;
    // Keeps track of whether audio output has already been initialized
    bool Initialized;
    // Pointer to the Port Audio input stream
    PaStream* InputStream;
    // Starts the Port Audio stream
    void StartPAStream(Zero::Status &status);
    // Stops the Port Audio stream
    void StopPAStream(Zero::Status &status);
    // Sets variables and creates output buffers using the specified size
    void SetUpBuffers(const unsigned size);
    // Sets the callback frame size and calls SetUpBuffers
    void SetLatency(const unsigned baseSize);
    // If true, currently set to low latency
    bool LowLatency;

    friend class CallbackDataObject;
  };
}

#endif
