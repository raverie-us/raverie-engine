///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

/// The latency setting used by the audio system.
/// <param name="Low">The default setting, where audio will have a low amount of latency.</param>
/// <param name="High">Audio will have a higher latency. This can fix some audio problems in some cases.</param>
DeclareEnum2(AudioLatency, Low, High);

class StreamInfo
{
public:
  StreamInfo() :
    mStatus(StreamStatus::Uninitialized),
    mChannels(0),
    mSampleRate(0)
  {}

  StreamStatus::Enum mStatus;
  String mErrorMessage;
  unsigned mChannels;
  unsigned mSampleRate;
};

void IOCallback(float* outputBuffer, float* inputBuffer, unsigned framesPerBuffer, void* data);

//--------------------------------------------------------------------- Audio Input Output Interface

class AudioIOInterface
{
public:
  AudioIOInterface();
  ~AudioIOInterface();

  // Initializes the audio API 
  bool InitializeAPI();
  // Initializes the input and/or output streams, depending on parameters
  bool Initialize(bool initOutput, bool initInput);
  // Shuts down the audio API. Streams should be already stopped.
  void ShutDown();
  // Starts the output and/or input streams, depending on parameters
  bool StartStreams(bool startOutput, bool startInput);
  // Stops the output and/or input streams, depending on parameters
  bool StopStreams(bool stopOutput, bool stopInput);
  // Returns the current status of the specified audio stream
  StreamStatus::Enum GetStreamStatus(StreamTypes::Enum whichStream);
  // Returns the last error message associated with the specified audio stream
  const String& GetStreamErrorMessage(StreamTypes::Enum whichStream);
  // Returns the last error message associated with the audio API
  const String& GetSystemErrorMessage();
  // Returns the number of channels in the specified audio stream
  unsigned GetStreamChannels(StreamTypes::Enum whichStream);
  // Returns the sample rate of the specified audio stream
  unsigned GetStreamSampleRate(StreamTypes::Enum whichStream);
  // Waits until another mix is needed, using semaphore counter
  void WaitUntilOutputNeededThreaded();
  // Fills the buffer with the requested number of audio samples, or the max available if lower
  void GetInputDataThreaded(Array<float>& buffer, unsigned howManySamples);
  // Sets whether the system should use a low or high latency value
  void SetOutputLatencyThreaded(AudioLatency::Enum latency);

  // Ring buffer used for mixed output
  RingBuffer OutputRingBuffer;

  // The following two functions are called from the audio IO thread using the callback

  // Gets the mixed buffer that is ready to output
  void GetMixedOutputSamples(float* outputBuffer, const unsigned frames);
  // Saves the input buffer from the microphone
  void SaveInputSamples(const float* inputBuffer, unsigned frames);

private:
  // Object to handle OS-specific audio input and output
  AudioInputOutput AudioIO;
  // Buffer used for the OutputRingBuffer
  float* MixedOutputBuffer;
  // The number of mix buffer frames for each latency setting
  unsigned OutputBufferSizePerLatency[AudioLatency::Size];
  // Current latency setting for the audio output
  AudioLatency::Enum mOutputStreamLatency;
  // Buffer used for the InputRingBuffer
  float* InputBuffer;
  // Ring buffer used for receiving input data
  RingBuffer InputRingBuffer;
  // For notifying the mix thread when a new buffer is needed.
  Semaphore MixThreadSemaphore;
  // List of info objects for each stream type
  StreamInfo StreamInfoList[StreamTypes::Size];
  // The value used to start calculating the mix frames
  const unsigned BufferSizeStartValue = 512;
  // Last error message pertaining to the audio API
  String mApiErrorMessage;

  // Sets variables and initializes output buffers at the appropriate size
  void InitializeOutputBuffers();
  // Initializes the input buffer at the appropriate size
  void InitializeInputBuffers();
  // Determines a power of two size for buffers depending on the provided sample rate
  unsigned GetBufferSize(unsigned sampleRate, unsigned channels);
  // Initializes the specified RingBuffer at the specified size
  void InitializeRingBuffer(RingBuffer& ringBuffer, float* buffer, unsigned size);
};

} // namespace Zero
