///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef TAGS_H
#define TAGS_H


namespace Audio
{
  typedef Zero::Array<SoundInstanceNode*> InstanceListType;

  //------------------------------------------------------------------------------------- Tag Object

  class TagObject 
  {
  public:
    TagObject(bool isThreaded = false);

    void UpdateForMix(unsigned howManyFrames, unsigned channels);
    void ProcessInstance(BufferType* instanceOutput, unsigned channels, SoundInstanceNode* instance);

    // Remove this tag from the system and delete it
    void RemoveTag();
    // Add a new instance to this tag
    void AddInstance(SoundInstanceNode* instance);
    // Remove and instance from this tag
    void RemoveInstance(SoundInstanceNode* instance);
    // Returns this tag's current volume
    float GetVolume();
    // Set the volume modification of this tag
    void SetVolume(const float volume, const float time);
    // Pause all tagged instances
    void PauseInstances();
    // Resume all tagged instances
    void ResumeInstances();
    // Stop all tagged instances
    void StopInstances();
    // Returns true if the tag is currently paused
    bool GetPaused();
    // Returns the number of tagged instances
    int GetNumberOfInstances();
    // Returns the maximum number of tagged instances that can play at once
    unsigned GetInstanceLimit();
    // Sets the maximum number of tagged instances that can play at once
    void SetInstanceLimit(const int limit);
    // Returns a pointer to the list of tagged instances
    const InstanceListType* GetInstances();

    // Used to pass data to equalizer functions
    struct GainValues
    {
      GainValues(float below80Hz, float at150Hz, float at600Hz, float at2500Hz, float above5000Hz) :
        mBelow80Hz(below80Hz),
        mAt150Hz(at150Hz),
        mAt600Hz(at600Hz),
        mAt2500Hz(at2500Hz),
        mAbove5000Hz(above5000Hz)
      {}

      float mBelow80Hz;
      float mAt150Hz;
      float mAt600Hz;
      float mAt2500Hz;
      float mAbove5000Hz;
    };

    // Returns true if currently applying equalizer settings
    bool GetUseEqualizer();
    // Sets whether the equalizer settings should be applied to tagged instances
    void SetUseEqualizer(const bool useEQ);
    // Returns the gain level of the frequencies below 80Hz
    float GetBelow80HzGain();
    // Sets the gain level of the frequencies below 80Hz
    void SetBelow80HzGain(const float gain);
    // Returns the gain level of the band centered at 150Hz
    float Get150HzGain();
    // Sets the gain level of the band centered at 150Hz
    void Set150HzGain(const float gain);
    // Returns the gain level of the band centered at 600Hz
    float Get600HzGain();
    // Sets the gain level of the band centered at 600Hz
    void Set600HzGain(const float gain);
    // Returns the gain level of the band centered at 2500Hz
    float Get2500HzGain();
    // Sets the gain level of the band centered at 2500Hz
    void Set2500HzGain(const float gain);
    // Returns the gain level of frequencies above 5000Hz
    float GetAbove5000HzGain();
    // Sets the gain level of frequencies above 5000Hz
    void SetAbove5000HzGain(const float gain);
    // Sets the gain level of all bands over the specified number of seconds
    void InterpolateAllBands(GainValues* values, const float timeToInterpolate);

    // Returns true if currently applying compressor settings
    bool GetUseCompressor();
    // Sets whether the compressor settings should be applied to tagged instances
    void SetUseCompressor(const bool useCompressor);
    // Returns the current compressor threshold
    float GetCompressorThreshold();
    // Sets the compressor threshold in decibels
    void SetCompressorThreshold(const float thresholdDB);
    // Returns the current compressor attack time
    float GetCompressorAttackMSec();
    // Sets the compressor attack time in milliseconds
    void SetCompressorAttackMSec(const float attack);
    // Returns the compressor release time
    float GetCompressorReleaseMSec();
    // Sets the compressor release time in milliseconds
    void SetCompressorReleaseMsec(const float release);
    // Returns the compressor ratio
    float GetCompressorRatio();
    // Sets the compressor ratio
    void SetCompressorRatio(const float ratio);
    // Returns the compressor knee width
    float GetCompressorKneeWidth();
    // Sets the compressor knee width
    void SetCompressorKneeWidth(const float kneeWidth);
    // Sets the tag used for input on the compressor filter
    void SetCompressorInputTag(TagObject* tag);

    ExternalNodeInterface* ExternalInterface;

  private:
    ~TagObject();

    // Accumulates audio output from all tagged sound instances into the mTotalInstanceOutput buffer
    BufferType* GetTotalInstanceOutput(unsigned howManyFrames, unsigned channels);
    // Removes the specified instance from the appropriate lists 
    void RemoveInstanceFromLists(SoundInstanceNode* instance);

    // If not threaded, a pointer to the threaded tag
    TagObject* mThreadedTag;
    // If true, this object is used for the mix thread
    bool mIsThreaded;
    // If true, all associated sound instances are currently paused
    bool mPaused;
    // The maximum number of instances that can be played with this tag
    int mInstanceLimit;
    // Keeps track of whether this tag has updated for the current mix
    unsigned mMixVersion;
    // Used to hold the total audio output of all associated sound instances
    BufferType mTotalInstanceOutput;
    // List of all associated sound instances
    Zero::Array<SoundInstanceNode*> mSoundInstanceList;
    // Current volume adjustment
    float mVolume;
    // If true, volume adjustment should be applied to tagged instances
    bool mModifyingVolume;
    // If true, the equalizer filter will be applied to tagged instances
    bool mUseEqualizer;
    // The compressor filter applied to tagged instances
    DynamicsProcessor CompressorObject;
    // If true, the compressor filter will be applied
    bool mUseCompressor;
    // The volume adjustment per sample created by the compressor filter
    BufferType mCompressorVolumes;
    // A tag whose audio will be used for the compressor input
    TagObject* mCompressorInputTag;
    
    // Stores data for each tagged sound instance
    struct InstanceData
    {
      InstanceData();
      ~InstanceData();

      // The volume modifier on the instance which is being used by the tag
      InstanceVolumeModifier* mVolumeModifier;
      // Equalizer filters for each tagged instance (relies on history of samples)
      Equalizer* mEqualizer;
    };

    // Map of instance pointers to data objects
    typedef Zero::HashMap<SoundInstanceNode*, InstanceData*> InstanceDataMapType;
    InstanceDataMapType DataPerInstance;
    // Used to store equalizer settings
    Equalizer EqualizerSettings;

    Zero::Link<TagObject> link;

    friend class SoundInstanceNode;
    friend class AudioSystemInterface;
    friend class AudioSystemInternal;
  };
}

#endif
