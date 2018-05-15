///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef AudioInputOutput_H
#define AudioInputOutput_H

struct IMMDeviceEnumerator;

namespace Audio
{
  namespace LatencyValues 
  {
    enum Enum { LowLatency, HighLatency, Count };
  }
  namespace StreamTypes
  {
    enum Enum { Output, Input, Count };
  }
  namespace StreamStatus
  {
    enum Enum { Uninitialized, Initialized, Started, Stopped, ApiProblem, DeviceProblem };
  }

  inline static void LogAudioIoError(Zero::StringParam message, Zero::String* savedMessage = nullptr)
  {
    ZPrint(Zero::String::Format(message.c_str(), "\n").c_str());
    if (savedMessage)
      *savedMessage = message;
  }

  struct StreamInfo
  {
    StreamInfo() : Status(StreamStatus::Uninitialized) {}

    StreamStatus::Enum Status;
    Zero::String ErrorMessage;
  };

  //----------------------------------------------------------------------------- Audio Input Output

  class AudioInputOutput
  {
  public:
    AudioInputOutput();
    virtual ~AudioInputOutput();

    // Waits until another mix is needed, using semaphore counter
    void WaitUntilOutputNeededThreaded();
    // Fills the buffer with the requested number of audio samples, or the max available if lower
    void GetInputDataThreaded(Zero::Array<float>& buffer, unsigned howManySamples);
    // Sets whether the system should use a low or high latency value
    void SetOutputLatency(LatencyValues::Enum latency);
    // Returns the StreamInfo data for the specified audio stream
    virtual const StreamInfo& GetStreamInfo(StreamTypes::Enum whichStream);

    // Initializes the underlying audio API
    virtual StreamStatus::Enum InitializeAPI() = 0;
    // Initializes the specified audio stream
    virtual StreamStatus::Enum InitializeStream(StreamTypes::Enum whichStream) = 0;
    // Starts the specified audio stream
    virtual StreamStatus::Enum StartStream(StreamTypes::Enum whichStream) = 0;
    // Stops the specified audio stream
    virtual StreamStatus::Enum StopStream(StreamTypes::Enum whichStream) = 0;
    // Shuts down the specified audio stream
    virtual StreamStatus::Enum ShutDownStream(StreamTypes::Enum whichStream) = 0;
    // Shuts down the underlying audio API
    virtual void ShutDownAPI() = 0;
    // Returns the number of channels in the specified audio stream
    virtual unsigned GetStreamChannels(StreamTypes::Enum whichStream) = 0;
    // Returns the sample rate of the specified audio stream
    virtual unsigned GetStreamSampleRate(StreamTypes::Enum whichStream) = 0;

    // Last error message pertaining to the audio API
    Zero::String LastErrorMessage;
    // Ring buffer used for mixed output
    RingBuffer OutputRingBuffer;

  protected:
    // Buffer used for the OutputRingBuffer
    float* MixedOutputBuffer;
    // The number of mix buffer frames for each latency setting
    unsigned OutputBufferSizePerLatency[LatencyValues::Count];
    // Current latency setting for the audio output
    LatencyValues::Enum OutputStreamLatency;
    // Size of the buffer for input data
    static const unsigned InputBufferSize = 8192;
    // Buffer of input data
    float InputBuffer[InputBufferSize];
    // Ring buffer used for receiving input data
    RingBuffer InputRingBuffer;
    // For notifying the mix thread when a new buffer is needed.
    Zero::Semaphore MixThreadSemaphore;
    // List of info objects for each stream type
    StreamInfo StreamInfoList[StreamTypes::Count];
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

}

#endif
