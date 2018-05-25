///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef AudioSystemInternal_H
#define AudioSystemInternal_H


namespace Audio
{
  static bool IsWithinLimit(float valueToCheck, float centralValue, float limit)
  {
    if (valueToCheck > centralValue + limit || valueToCheck < centralValue - limit)
      return false;
    else
      return true;
  }

  static void CopyIntoBuffer(BufferType* destinationBuffer, const BufferType& sourceBuffer,
    unsigned sourceStartIndex, unsigned numberOfSamples)
  {
    unsigned destIndex = destinationBuffer->Size();
    destinationBuffer->Resize(destinationBuffer->Size() + numberOfSamples);
    memcpy(destinationBuffer->Data() + destIndex, sourceBuffer.Data() + sourceStartIndex,
      sizeof(float) * numberOfSamples);
  }

  //------------------------------------------------------------------------------------ Audio Frame
  
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
    const float* Matrices[cMaxChannels + 1];
    float mSamples[cMaxChannels];
    float mCopiedSamples[cMaxChannels];

    static void CopySamples(const float* source, float* destination, const unsigned channels);
  };
  
  //-------------------------------------------------------------------------- Audio System Internal

  // Main audio system. 
  class AudioSystemInternal : public ExternalNodeInterface
  {
  public:
    AudioSystemInternal(ExternalSystemInterface* extInterface);

    // Sets up variables, initializes audio output, starts mix thread.
    void StartSystem(Zero::Status &status);
    // Shuts down and deletes the mix thread, shuts down audio output
    void StopSystem(Zero::Status &status);
    // Looping function on the mix thread to handle tasks and mix output.
    // Waits for signal from PACallback to mix another buffer.
    void MixLoopThreaded();
    // Adds a task from the main thread for the mix thread to handle
    void AddTask(Zero::Functor* function);
    // Adds a task from the mix thread for the main thread to handle
    void AddTaskThreaded(Zero::Functor* function);
    // Adds a tag to the system
    void AddTag(TagObject* tag, bool threaded);
    // Removes a tag from the system
    void RemoveTag(TagObject* tag, bool threaded);
    // Adds a tag to the list to delete
    void DelayDeleteTag(TagObject* tag, bool threaded);
    // Adds a non-threaded sound asset to the system
    void AddAsset(SoundAsset* asset);
    // Removes a non-threaded sound asset from the system
    void RemoveAsset(SoundAsset* asset);
    // Adds a new node to the threaded or non-threaded list
    bool AddSoundNode(SoundNode* node, const bool threaded);
    // Removes a node from the threaded or non-threaded list
    void RemoveSoundNode(SoundNode* node, const bool threaded);
    // Sets the threaded variable for the minimum volume threshold.
    void SetMinVolumeThresholdThreaded(const float volume);
    // Sets whether or not all audio should be muted
    void SetMutedThreaded(bool muteAudio);
    
    // Number of channels to use for calculating output. 
    unsigned SystemChannelsThreaded;
    // Notifies the system to reset Port Audio after a device change.
    bool ResetPA;
    // Current mix version number
    unsigned MixVersionNumber;
    // Non-threaded counterpart of the output node
    OutputNode* FinalOutputNode;
    // Threaded counterpart of the output node
    OutputNode* FinalOutputNodeThreaded;
    // Pointer to the external interface attached to the system
    ExternalSystemInterface* ExternalInterface;
    // If a SoundInstance is below this threshold it will keep its place but not process any audio.
    float MinimumVolumeThresholdThreaded;
    // Audio input data for the current mix, matching the current output sample rate and channels
    Zero::Array<float> InputBuffer;
    // If true, will send microphone input data to external system
    bool SendMicrophoneInputData;
    
    AudioIOInterface InputOutputInterface;
    
  private:
    typedef Zero::Array<TagObject*> TagsToDeleteListType;
    typedef Zero::InList<TagObject> TagListType;
    typedef Zero::InList<SoundNode> NodeListType;
    typedef Zero::InList<SoundAsset> AssetListType;

    // List of objects to be deleted on update.
    TagsToDeleteListType TagsToDelete;
    // List of objects to be deleted after a mix.
    TagsToDeleteListType TagsToDeleteThreaded;
    // List of all current tags
    TagListType TagList;
    // List of all current threaded tags
    TagListType TagListThreaded;
    // List of all current sound assets
    AssetListType AssetList;
    // Array used to accumulate samples for output
    BufferType BufferForOutput;
    // Array for finished mixed output
    BufferType MixedOutput;
    // Thread for mix loop
    Zero::Thread MixThread;
    // To tell the system to shut down once everything stops. 
    bool ShuttingDownThreaded;
    // Threaded copy of overall system volume.
    float SystemVolumeThreaded;
    // Non-threaded copy of overall system volume. 
    float Volume;
    // For interpolating the overall system volume on the mix thread. 
    InterpolatingObject VolumeInterpolatorThreaded;
    // For low frequency channel on 5.1 or 7.1 mix 
    // Must be pointer because relies on audio system in constructor
    LowPassFilter* LowPass;
    // Maximum number of tasks to execute on one update (to prevent the possibility
    // of infinite loops, even though that would be rare)
    static const unsigned MaxTasksToRead = 10000;
    // Tasks for the mix thread from the main thread
    LockFreeQueue<Zero::Functor*> TasksForMixThread;
    // Tasks for the main thread from the mix thread
    LockFreeQueue<Zero::Functor*> TasksForGameThread;
    // The highest volume value from the last mix.
    float PeakVolumeLastMix;
    // The RMS volume value from the last mix.
    float RmsVolumeLastMix;
    // Object to get MIDI data from the operating system.
    MidiInput MidiObject;
    // List of all current threaded sound nodes
    NodeListType NodeListThreaded;
    // List of all current sound nodes
    NodeListType NodeList;
    // Number of currently alive sound nodes
    unsigned NodeCount;
    // The peak volume from the last mix, used to check whether to create a task
    float PreviousPeakVolumeThreaded;
    // The RMS volume from the last mix, used to check whether to create a task
    unsigned PreviousRMSVolumeThreaded;
    // If true the output is being resampled to match the sample rate of the device
    bool Resampling;
    // Resampler object used to resample mixed output
    Resampler OutputResampling;
    // Encoder to use for compressed microphone input
    PacketEncoder Encoder;
    // Queue for passing microphone input between threads
    LockFreeQueue<Zero::Array<float>*> InputDataQueue;
    // Resampler object used to resample microphone input
    Resampler InputResampling;

    // Adds current sounds into the output buffer. Will return false when the system can shut down. 
    bool MixCurrentInstancesThreaded();
    // Switches buffer pointers and executes all current tasks for the mix thread. 
    void HandleTasksThreaded();
    // Switches buffer pointers and executes all tasks from the mix thread
    void HandleTasks();
    // Tells the system to shut down
    void ShutDownThreaded();
    // Sets the threaded system channels variables
    void SetSystemChannelsThreaded(const unsigned channels);
    // Sets the peak and RMS volume values.
    void SetVolumes(const float peak, const unsigned rms);
    // Sets whether to use the high or low latency values
    void SetLatencyThreaded(const bool useHighLatency);
    // Checks for resampling and resets variables if applicable
    void CheckForResampling();
    // Gets the current input data from the AudioIO and adjusts if necessary to match output settings
    void GetAudioInputDataThreaded(unsigned howManySamples);
    // If true, audio will be processed normally but will not be sent to the output device
    bool Muted;
    bool MutedThreaded;
    // Used to know when to set the Muted variable
    bool MutingThreaded;

    friend class AudioSystemInterface;
    friend class ListenerNode;
  };

  /** Global pointer to the audio system. */
  extern AudioSystemInternal *gAudioSystem;
  

  //------------------------------------------------------------------------------------ Audio Frame

  /*
  Format for matrices:
  When down-sampling columns are source channels, rows are destination channels
  When up-sampling it's reversed
  Order is FrontLeft, FrontRight, Center, LowFreq, SideLeft, SideRight, BackLeft, BackRight
  */
  
  static const float Sqrt2Inv = 1.0f / Math::Sqrt(2.0f);

  static const float ChannelMatrix1[cMaxChannels] =
  {
    Sqrt2Inv, Sqrt2Inv, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f
  };

  static const float ChannelMatrix2[cMaxChannels * 2] =
  {
    1.0f, 0.0f, Sqrt2Inv, 0.0f, Sqrt2Inv, 0.0f, Sqrt2Inv, 0.0f,
    0.0f, 1.0f, Sqrt2Inv, 0.0f, 0.0f, Sqrt2Inv, 0.0f, Sqrt2Inv
  };

  static const float ChannelMatrix3[cMaxChannels * 3] =
  {
    1.0f, 0.0f, 0.0f, 0.0f, Sqrt2Inv, 0.0f, Sqrt2Inv, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f, Sqrt2Inv, 0.0f, Sqrt2Inv,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
  };

  static const float ChannelMatrix4[cMaxChannels * 4] =
  {
    1.0f, 0.0f, Sqrt2Inv, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, Sqrt2Inv, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
  };

  static const float ChannelMatrix5[cMaxChannels * 5] =
  {
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
  };

  static const float ChannelMatrix6[cMaxChannels * 6] =
  {
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
  };

  static const float ChannelMatrix7[cMaxChannels * 7] =
  {
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
  };

  static const float ChannelMatrix8[cMaxChannels * 8] =
  {
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
  };

}



#endif
