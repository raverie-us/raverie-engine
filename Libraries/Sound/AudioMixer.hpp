///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

//--------------------------------------------------------------------------------------- Audio Task

class AudioTask
{
public:
  AudioTask();
  AudioTask(Functor* function, HandleOf<SoundNode> node);
  AudioTask(const AudioTask& other);
  ~AudioTask();

  Functor* mFunction;
  HandleOf<SoundNode> mObject;
};

//-------------------------------------------------------------------------------------- Audio Mixer

class AudioMixer : public EventObject
{
public:
  AudioMixer();

  // Sets up variables, initializes audio input and output, starts mix thread
  void StartMixing(Status& status);
  // Shuts down the mix thread, shuts down audio output
  void ShutDown();
  // Update function on the main thread, should be called every game update
  void Update();
  // Looping function on the mix thread to handle tasks and mix output
  void MixLoopThreaded();
  // Adds a task from the main thread for the mix thread to handle
  void AddTask(Functor* task, HandleOf<SoundNode> node);
  // Adds a task from the mix thread for the main thread to handle
  void AddTaskThreaded(Functor* task, HandleOf<SoundNode> node);
  // Sets whether to use the high or low latency values
  void SetLatency(AudioLatency::Enum latency);
  // Starts the input stream if it is not already started. Returns false if stream could not be started.
  bool StartInput();
  // Gets the overall system volume
  float GetVolume();
  // Changes the overall system volume
  void SetVolume(const float volume);
  // Returns true if the audio is currently muted
  bool GetMuteAllAudio();
  // If set to true, all audio will be processed as normal but will be silent
  void SetMuteAllAudio(const bool muteAudio);
  // Returns the number of channels used in the system's output
  unsigned GetOutputChannels();
  // Sets the number of channels that should be used to mix audio output. This will be translated to
  // the channels needed by the system's audio output device.
  void SetOutputChannels(const unsigned channels);
  // Returns the highest volume from the last audio mix.
  float GetPeakOutputVolume();
  // Returns the RMS volume of the last audio mix.
  float GetRMSOutputVolume();
  // Sets the minimum volume at which SoundInstances will process audio.
  void SetMinimumVolumeThreshold(const float volume);
  // If true, events will be sent with microphone input data as float samples
  void SetSendUncompressedMicInput(const bool sendInput);
  // If true, events will be sent with compressed microphone input data as bytes
  void SetSendCompressedMicInput(const bool sendInput);
  // If currently sending microphone input data, returns the highest peak volume in the last input
  float GetPeakInputVolume();
  // Sends an event when a listener is removed so SoundNodes can remove stored information
  void SendListenerRemovedEvent(ListenerNode* listener);

  // Number of channels used for the mixed output
  Threaded<int> mSystemChannels;
  // Current mix version number
  unsigned mMixVersionThreaded;
  // If a SoundInstance is below this threshold it will keep its place but not process any audio.
  float mMinimumVolumeThresholdThreaded;
  // Audio input data for the current mix, matching the current output sample rate and channels
  Array<float> InputBuffer;
  // If true, will send microphone input data to external system
  ThreadedInt mSendMicrophoneInputData;
  // List of decoding tasks used if the system is not threaded
  Array<AudioFileDecoder*> DecodingTasks;
  // The maximum number of decoding tasks that will be processed on one update
  // (this number is arbitrary and can be changed)
  static const unsigned MaxDecodingTasksToRun = 10;
  // The node that all audio is attached to
  HandleOf<OutputNode> FinalOutputNode;
  // The interface for audio input and output
  AudioIOInterface AudioIO;

private:
  // Adds current sounds into the output buffer. Will return false when the system can shut down. 
  bool MixCurrentInstancesThreaded();
  // Switches buffer pointers and executes all current tasks for the mix thread. 
  void HandleTasksThreaded();
  // Switches buffer pointers and executes all tasks for the main thread.
  void HandleTasks();
  // Checks for resampling and resets variables if applicable
  void CheckForResamplingThreaded();
  // Gets the current input data from the AudioIO and adjusts if necessary to match output settings
  void GetAudioInputDataThreaded(unsigned howManySamples);
  // Sets whether or not all audio should be muted
  void SetMutedThreaded(bool muteAudio);
  // Gets microphone input data from the mix thread and sends it out via an event
  void DispatchMicrophoneInput();
  // Turns on and off sending microphone input
  void SetSendMicInput(bool turnOn);

  typedef Array<AudioTask> TaskListType;

  // Array used to accumulate samples for output
  BufferType BufferForOutput;
  // Array for finished mixed output
  BufferType MixedOutput;
  // Thread for mix loop
  Thread MixThread;
  // For interpolating the overall system volume on the mix thread. 
  InterpolatingObject VolumeInterpolatorThreaded;
  // For low frequency channel on 5.1 or 7.1 mix 
  // Must be pointer because relies on audio system in constructor
  LowPassFilter* LowPass;
  // Read and write task buffers for the mix thread
  TaskListType TasksForMixThread[2];
  // Lock used for switching mix thread task buffers
  ThreadLock MixThreadTasksLock;
  // Read and write task buffers for the game thread
  TaskListType TasksForGameThread[2];
  // Lock used for switching game thread task buffers
  ThreadLock GameThreadTasksLock;
  // Object to get MIDI data from the operating system.
  MidiInput MidiObject;
  // Resampler object used to resample mixed output
  Resampler OutputResampler;
  // Encoder to use for compressed microphone input
  PacketEncoder Encoder;
  // Resampler object used to resample microphone input
  Resampler InputResampler;
  // Buffer for passing microphone input from the mix thread to the game thread
  RingBuffer InputDataBuffer;
  // Stored microphone input samples when sending compressed input
  Array<float> PreviousInputSamples;

  // Index of the mix thread task buffer to write to
  int mMixThreadTaskWriteIndex;
  // Index of the game thread task buffer to write to
  int mGameThreadTaskWriteIndex;
  // To tell the system to shut down once everything stops. 
  ThreadedInt mShuttingDown;
  // Overall system volume. 
  Threaded<float> mVolume;
  // The highest volume value from the last mix.
  Threaded<float> mPeakVolumeLastMix;
  // The RMS volume value from the last mix.
  Threaded<float> mRmsVolumeLastMix;
  // The peak volume from the last mix, used to check whether to create a task
  float mPreviousPeakVolumeThreaded;
  // The RMS volume from the last mix, used to check whether to create a task
  unsigned mPreviousRMSVolumeThreaded;
  // If true the output is being resampled to match the sample rate of the device
  bool mResamplingThreaded;
  // If true, audio will be processed normally but will not be sent to the output device
  ThreadedInt mMuted;
  // Used to know when to set the Muted variable
  bool mMutingThreaded;
  // Peak volume of the last microphone input data
  float mPeakInputVolume;
  // If true, will send the microphone input data event in a compressed format
  bool mSendMicrophoneInputCompressed;
  // If true, will send the microphone input data event in an uncompressed format
  bool mSendMicrophoneInputUncompressed;
};

//-------------------------------------------------------------------------------------- Audio Frame

class AudioFrame
{
public:
  AudioFrame(float* samples, unsigned channels);
  AudioFrame();
  AudioFrame(const AudioFrame& copy);

  float* GetSamples(const unsigned channels);
  void SetSamples(const float* samples, unsigned channels);
  void Clamp();
  float GetMaxValue();
  float GetMonoValue();
  void operator*=(float multiplier);
  void operator=(const AudioFrame& copy);

private:
  enum Channels { FrontLeft, FrontRight, Center, LowFreq, SideLeft, SideRight, BackLeft, BackRight };
  unsigned mStoredChannels;
  const float* Matrices[AudioConstants::cMaxChannels + 1];
  float mSamples[AudioConstants::cMaxChannels];
  float mCopiedSamples[AudioConstants::cMaxChannels];

  static void CopySamples(const float* source, float* destination, const unsigned channels);
};

} // namespace Zero
