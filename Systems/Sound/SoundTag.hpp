///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{
class SoundInstance;
typedef Array<HandleOf<SoundInstance>> InstanceList;

namespace Events
{
  DeclareEvent(AddedInstanceToTag);
  DeclareEvent(TagHasNoInstances);
}

//---------------------------------------------------------------------------------------- Sound Tag

class SoundTagDisplay : public MetaDisplay
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  String GetName(HandleParam object) override;
  String GetDebugText(HandleParam object) override;
};

/// Controls settings on all tagged SoundInstances
class SoundTag : public DataResource, public Audio::ExternalNodeInterface
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  SoundTag();
  ~SoundTag();

  void Serialize(Zero::Serializer &) override {}
  void Unload() override;

  /// Adds a new SoundInstance to this SoundTag.
  void TagSound(SoundInstance* instance);
  /// Removes a SoundInstance from this SoundTag.
  void UnTagSound(SoundInstance* instance);
  /// Stops all currently tagged SoundInstances.
  void StopSounds();
  /// Setting this property to true will pause all tagged instances. Setting it to false will resume playback.
  bool GetPaused();
  void SetPaused(bool pause);
  /// This allows you to get all currently tagged SoundInstances. Using a foreach loop, 
  /// you can access any SoundInstance functionality on each of the tagged instances.
  InstanceList::range GetInstances();
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
  HandleOf<SoundTag> GetTagForDucking();
  void SetTagForDucking(HandleOf<SoundTag> tag);
  /// If this value is greater than zero, SoundCues with this SoundTag will only play if
  /// the number of tagged SoundInstances is less than this number.
  float GetInstanceLimit();
  void SetInstanceLimit(float limit);
    
// Internals
  Audio::TagObject* mTagObject;
  InstanceList SoundInstanceList;
  void SendAudioEvent(const Audio::AudioEventType eventType, void* data) override;

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

}