///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{
typedef Array<HandleOf<SoundInstance>> InstanceListType;

namespace Events
{

DeclareEvent(AddedInstanceToTag);
DeclareEvent(TagHasNoInstances);

} // namespace Events

//--------------------------------------------------------------------------------------- Tag Object

class TagObject : public ReferenceCountedObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  TagObject();
  ~TagObject();

  // Add a new instance to this tag
  void AddInstanceThreaded(SoundInstance* instance);
  // Remove and instance from this tag
  void RemoveInstanceThreaded(SoundInstance* instance);
  // Returns the current volume modification
  float GetVolume();
  // Set the volume modification of this tag
  void SetVolume(float volume, float time);
  // Returns the current gain level of a frequency band
  float GetEQBandGain(EqualizerBands::Enum whichBand);
  // Sets the gain level of a frequency band
  void SetEQBandGain(EqualizerBands::Enum whichBand, float gain);
  // Sets the gain level of all bands over the specified number of seconds
  void InterpolateEQBandsThreaded(float* values, float timeToInterpolate);
  // Returns the current compressor threshold, in decibels
  float GetCompressorThresholdDb();
  // Sets the compressor threshold, in decibels
  void SetCompressorThresholdDb(float decibels);
  // Returns the current compressor attack time, in milliseconds
  float GetCompressorAttackMs();
  // Sets the compressor attack time, in milliseconds
  void SetCompressorAttackMs(float milliseconds);
  // Returns the current compressor release time, in milliseconds
  float GetCompressorReleaseMs();
  // Sets the compressor release time, in milliseconds
  void SetCompressorReleaseMs(float milliseconds);
  // Returns the current compressor ratio
  float GetCompresorRatio();
  // Sets the compressor ratio
  void SetCompressorRatio(float ratio);
  // Returns the current compressor knee width
  float GetCompresorKneeWidth();
  // Sets the compressor knee width
  void SetCompressorKneeWidth(float knee);
  // Removes this tag object from the system and all associated sound instances
  void RemoveTag();
  // Called by each tagged sound instance
  void ProcessInstanceThreaded(BufferType* instanceOutput, unsigned channels, SoundInstance* instance);
  // Accumulates audio output from all tagged sound instances into the mTotalInstanceOutput buffer
  BufferType* GetTotalInstanceOutputThreaded(unsigned howManyFrames, unsigned channels);

  // The maximum number of instances that can be played with this tag
  int mInstanceLimit;
  // If true, all associated sound instances are currently paused
  Threaded<bool> mPaused;
  // If true, the equalizer filter will be applied to tagged instances
  Threaded<bool> mUseEqualizer;
  // If true, the compressor filter will be applied
  Threaded<bool> mUseCompressor;
  // A tag whose audio will be used for the compressor input
  Threaded<TagObject*> mCompressorInputTag;

private:
  void UpdateForMixThreaded(unsigned howManyFrames, unsigned channels);
  void SetVolumeThreaded(float volume, float time);
  void SetEQBandGainThreaded(EqualizerBands::Enum whichBand, float gain);

  // Keeps track of whether this tag has updated for the current mix
  unsigned mMixVersionThreaded;
  // Used to hold the total audio output of all associated sound instances
  BufferType mTotalInstanceOutputThreaded;
  // Current volume adjustment
  Threaded<float> mVolume;
  // If true, volume adjustment should be applied to tagged instances
  bool mModifyingVolumeThreaded;
  // Used to store equalizer settings
  float mEqualizerGainValues[EqualizerBands::Count];
  float mEqualizerGainValuesThreaded[EqualizerBands::Count];
  // The compressor filter applied to tagged instances
  DynamicsProcessor CompressorObjectThreaded;
  // The volume adjustment per sample created by the compressor filter
  BufferType mCompressorVolumesThreaded;

  float mCompressorThreshold;
  float mCompressorAttackMs;
  float mCompressorReleaseMs;
  float mCompressorRatio;
  float mCompressorKnee;

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
  typedef Zero::HashMap<SoundInstance*, InstanceData*> InstanceDataMapType;
  InstanceDataMapType DataPerInstanceThreaded;

  Link<TagObject> link;
};

//-------------------------------------------------------------------------------- Sound Tag Display

class SoundTagDisplay : public MetaDisplay
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  String GetName(HandleParam object) override;
  String GetDebugText(HandleParam object) override;
};

//---------------------------------------------------------------------------------------- Sound Tag

/// Controls settings on all tagged SoundInstances
class SoundTag : public DataResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  SoundTag();
  ~SoundTag();

  void Serialize(Serializer &) override {}
  void Unload() override;

  /// Adds a new SoundInstance to this SoundTag.
  void TagSound(HandleOf<SoundInstance>& instance);
  /// Removes a SoundInstance from this SoundTag.
  void UnTagSound(HandleOf<SoundInstance>& instance);
  /// Stops all currently tagged SoundInstances.
  void StopSounds();
  /// Setting this property to true will pause all tagged instances. Setting it to false will resume playback.
  bool GetPaused();
  void SetPaused(bool pause);
  /// This allows you to get all currently tagged SoundInstances. Using a foreach loop, 
  /// you can access any SoundInstance functionality on each of the tagged instances.
  InstanceListType::range GetInstances();
  /// The number of SoundInstances currently associated with this SoundTag. 
  int GetInstanceCount();
  /// The volume adjustment applied to all tagged instances. 
  float GetVolume();
  void SetVolume(float value);
  /// Interpolates the SoundTag's Volume property from its current value to the value 
  /// passed in as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateVolume(float value, float interpolationTime);
  /// The volume adjustment, in decibels, applied to all tagged instances. 
  float GetDecibels();
  void SetDecibels(float decibels);
  /// Interpolates the SoundTag's Decibels property from its current value to the value 
  /// passed in as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateDecibels(float decibels, float interpolationTime);
  /// If true, the SoundTag's equalizer settings will be applied to the tagged SoundInstances.
  bool GetUseEqualizer();
  void SetUseEqualizer(bool useEQ);
  /// The volume adjustment applied to frequencies below 80 Hz.
  /// Positive values will boost these frequencies while negative values will reduce them.
  float GetEQLowPassGain();
  void SetEQLowPassGain(float gain);
  /// The volume adjustment applied to frequencies within the band centered at 150 Hz.
  /// Positive values will boost these frequencies while negative values will reduce them.
  float GetEQBand1Gain();
  void SetEQBand1Gain(float gain);
  /// The volume adjustment applied to frequencies within the band centered at 600 Hz.
  /// Positive values will boost these frequencies while negative values will reduce them.
  float GetEQBand2Gain();
  void SetEQBand2Gain(float gain);
  /// The volume adjustment applied to frequencies within the band centered at 2500 Hz.
  /// Positive values will boost these frequencies while negative values will reduce them.
  float GetEQBand3Gain();
  void SetEQBand3Gain(float gain);
  /// The volume adjustment applied to frequencies above 5000 Hz.
  /// Positive values will boost these frequencies while negative values will reduce them.
  float GetEQHighPassGain();
  void SetEQHighPassGain(float gain);
  /// Sets all equalizer band gain values at once. The parameters are in order from the lowest band
  /// to the highest. The last parameter is the number of seconds to interpolate the values over.
  void EQSetAllBands(float lowPass, float band1, float band2, float band3, float highPass, 
    float timeToInterpolate);
  /// If true, the SoundTag's compressor settings will be applied to the tagged SoundInstances.
  bool GetUseCompressor();
  void SetUseCompressor(bool useCompressor);
  /// The threshold, in decibels, at which the volume is affected by the filter. 
  float GetCompressorThreshold();
  void SetCompressorThreshold(float thresholdDB);
  /// The time, in milliseconds, for the filter to ramp to full effect after the input reaches the threshold.
  float GetCompressorAttack();
  void SetCompressorAttack(float attack);
  /// The time, in milliseconds, for the filter to ramp from full effect to off 
  /// after the input drops below the threshold.
  float GetCompressorRelease();
  void SetCompressorRelease(float release);
  /// The ratio of the compression applied by the filter. 
  float GetCompressorRatio();
  void SetCompressorRatio(float ratio);
  /// The knee width of the filter, in decibels.
  float GetCompressorKneeWidth();
  void SetCompressorKneeWidth(float kneeWidth);
  /// If this property is not null, the selected SoundTag will be used to trigger this SoundTag's compressor.
  SoundTag* GetTagForDucking();
  void SetTagForDucking(SoundTag*);
  /// If this value is greater than zero, SoundCues with this SoundTag will only play if
  /// the number of tagged SoundInstances is less than this number.
  float GetInstanceLimit();
  void SetInstanceLimit(float limit);
    
// Internals
  HandleOf<TagObject> mTagObject;
  InstanceListType SoundInstanceList;
  Link<SoundTag> link;

  void CreateTag();
  void ReleaseTag();

private:
  HandleOf<SoundTag> mCompressorTag;

};

//-------------------------------------------------------------------------------- Sound Tag Manager

class SoundTagManager : public ResourceManager
{
public:
  DeclareResourceManager(SoundTagManager, SoundTag);
  SoundTagManager(BoundType* resourceType);
};

} // namespace Zero
