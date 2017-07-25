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

  //------------------------------------------------------------------------- Audio Channels Manager

  class AudioChannelsManager
  {
  public:
    AudioChannelsManager();
    ~AudioChannelsManager();

    void GetClosestSpeakerValues(Math::Vec2 sourceVec, unsigned numberOfChannels, float& gain1, 
      float& gain2, int& channel1, int& channel2);

  private:
    struct SpeakerInfo
    {
      SpeakerInfo() : Channel1(-1), Channel2(-1) {}

      Math::Mat2 SpeakerMatrix;
      int Channel1;
      int Channel2;

      void GetGainValues(const Math::Vec2& sourceVec, float& gain1, float& gain2);
    };

    SpeakerInfo* SpeakerMatrixArrays[9];

    void CreateSpeakerMatrix();

  };

  //-------------------------------------------------------------------------- Audio System Internal

  // Main audio system. 
  class AudioSystemInternal
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
    // Stops a current task block from the main thread
    void StopTaskBlock();
    // Loop to execute decoding tasks
    void DecodeLoopThreaded();
    // Adds a new decoding task to the list
    void AddDecodingTask(Zero::Functor* function);
    // Adds a new node to the threaded or non-threaded list
    bool AddSoundNode(SoundNode* node, const bool threaded);
    // Removes a node from the threaded or non-threaded list
    void RemoveSoundNode(SoundNode* node, const bool threaded);
    // Returns a pointer to an available interpolator object
    InterpolatingObject* GetInterpolatorThreaded();
    // Releases an interpolator object that was in use
    void ReleaseInterpolatorThreaded(InterpolatingObject* object);

    // Number of channels to use for calculating output. 
    unsigned SystemChannelsThreaded;
    // Samples per second in audio output.
    unsigned SystemSampleRate;
    // Size of the system mix buffer.
    unsigned MixBufferSizeThreaded;
    // Used to lock for swapping pointers to buffers.
    Zero::ThreadLock LockObject;
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
    // Sets the threaded variable for the minimum volume threshold.
    void SetMinVolumeThresholdThreaded(const float volume);
    // Adds a tag to the system
    void AddTag(TagObject* tag, bool threaded);
    // Removes a tag from the system
    void RemoveTag(TagObject* tag, bool threaded);
    // Adds a tag to the list to delete
    void DelayDeleteTag(TagObject* tag, bool threaded);
    // Adds a non-threaded sound asset to the system
    void AddAsset(SoundAssetNode* asset);
    // Removes a non-threaded sound asset from the system
    void RemoveAsset(SoundAssetNode* asset);
    
    AudioChannelsManager ChannelsManager;
    AudioInputOutput AudioIO;
    
  private:
    typedef Zero::Array<TagObject*> TagsToDeleteListType;
    typedef Zero::InList<TagObject> TagListType;
    typedef Zero::InList<SoundNode> NodeListType;
    typedef Zero::InList<SoundAssetNode> AssetListType;

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
    // Thread for decoding tasks
    Zero::Thread DecodeThread;
    // Used to add tasks to decoding list
    Zero::ThreadLock DecodeLock;
    // True when all decoding tasks have finished
    bool DecodingFinished;
    // List of decoding tasks
    Zero::Array<Zero::Functor*> DecodingList;
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
    // Receives audio device change notifications (headphones plugged in etc.) 
    DeviceNotificationObject DeviceInfo;
    // For low frequency channel on 5.1 or 7.1 mix 
    // Must be pointer because relies on audio system in constructor
    LowPassFilter* LowPass;
    // Maximum number of tasks to execute on one update
    static const unsigned MaxTasksToRead = 300;
    static const unsigned MaxTasksToReadThreaded = 100;
    // Tasks for the mix thread from the main thread
    LockFreeQueue<Zero::Functor*> TasksForMixThread;
    // Tasks for the main thread from the mix thread
    LockFreeQueue<Zero::Functor*> TasksForGameThread;
    // The highest volume value from the last mix.
    float PeakVolumeLastMix;
    // The RMS volume value from the last mix.
    float RmsVolumeLastMix;
    // Object to get MIDI data from the operating system.
    MidiIn MidiObject;
    // List of all current threaded sound nodes
    NodeListType NodeListThreaded;
    // List of all current sound nodes
    NodeListType NodeList;
    // Number of currently alive sound nodes
    unsigned NodeCount;
    // List of interpolator objects
    Zero::Array<InterpolatorContainer> InterpolatorArray;
    // The index of the next available interpolator
    int NextInterpolator;

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
    void SetUseHighLatency(const bool useHighLatency);

    class NodeInterface : public ExternalNodeInterface
    {
    public:
      void SendAudioEvent(const AudioEventType eventType, void* data) override {}
    };

    NodeInterface NodeInt;

    friend class AudioSystemInterface;
    friend class AudioInputOutput;
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

  static const float ChannelMatrix1[MaxChannels] =
  {
    0.707f, 0.707f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f
  };

  static const float ChannelMatrix2[MaxChannels * 2] =
  {
    1.0f, 0.0f, 0.707f, 0.0f, 0.707f, 0.0f, 0.707f, 0.0f,
    0.0f, 1.0f, 0.707f, 0.0f, 0.0f, 0.707f, 0.0f, 0.707f
  };

  static const float ChannelMatrix3[MaxChannels * 3] =
  {
    1.0f, 0.0f, 0.0f, 0.0f, 0.707f, 0.0f, 0.707f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.707f, 0.0f, 0.707f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
  };

  static const float ChannelMatrix4[MaxChannels * 4] =
  {
    1.0f, 0.0f, 0.707f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.707f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
  };

  static const float ChannelMatrix5[MaxChannels * 5] =
  {
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
  };

  static const float ChannelMatrix6[MaxChannels * 6] =
  {
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
  };

  static const float ChannelMatrix7[MaxChannels * 7] =
  {
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
  };

  static const float ChannelMatrix8[MaxChannels * 8] =
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

  class AudioFrame
  {
  public:
    AudioFrame(float* samples, unsigned channels);

    void TranslateChannels(const unsigned channels);

    float Samples[8];

  private:
    enum Channels { FrontLeft, FrontRight, Center, LowFreq, SideLeft, SideRight, BackLeft, BackRight };
    unsigned HowManyChannels;
    const float* Matrices[MaxChannels];
  };

}



#endif
