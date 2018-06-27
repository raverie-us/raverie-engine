///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

using namespace AudioConstants;

namespace Events
{

DefineEvent(AddedInstanceToTag);
DefineEvent(TagHasNoInstances);

} // namespace Events

//--------------------------------------------------------------------------------------- Tag Object

//**************************************************************************************************
ZilchDefineType(TagObject, builder, type)
{

}

//**************************************************************************************************
TagObject::TagObject() :
  mInstanceLimit(0),
  mPaused(false),
  mUseEqualizer(false),
  mUseCompressor(false),
  mCompressorInputTag(nullptr),
  mMixVersionThreaded(Z::gSound->Mixer.mMixVersionThreaded - 1),
  mVolume(1.0f),
  mModifyingVolumeThreaded(false)
{
  for (int i = 0; i < EqualizerBands::Count; ++i)
    mEqualizerGainValues[i] = 1.0f;
  memcpy(mEqualizerGainValuesThreaded, mEqualizerGainValues, sizeof(float) * EqualizerBands::Count);

  mCompressorThreshold = CompressorObjectThreaded.GetThreshold();
  mCompressorAttackMs = CompressorObjectThreaded.GetAttackMSec();
  mCompressorReleaseMs = CompressorObjectThreaded.GetReleaseMSec();
  mCompressorRatio = CompressorObjectThreaded.GetRatio();
  mCompressorKnee = CompressorObjectThreaded.GetKneeWidth();
}

//**************************************************************************************************
TagObject::~TagObject()
{
  // TODO make sure removed from instances - call RemoveTag?
}

//**************************************************************************************************
void TagObject::AddInstanceThreaded(SoundInstance* instance)
{
  // Add a new data object to the map
  InstanceData* data = new InstanceData();
  DataPerInstanceThreaded[instance] = data;

  // If modifying volume, create the modifier
  if (mModifyingVolumeThreaded)
  {
    data->mVolumeModifier = instance->GetAvailableVolumeModThreaded();
    data->mVolumeModifier->Reset(1.0f, mVolume.Get(AudioThreads::MixThread), cPropertyChangeFrames, 0);
  }

  // If using equalizer, create it and set the settings
  if (mUseEqualizer.Get(AudioThreads::MainThread))
    data->mEqualizer = new Equalizer(mEqualizerGainValuesThreaded);
}

//**************************************************************************************************
void TagObject::RemoveInstanceThreaded(SoundInstance* instance)
{
  // Find this instance's data in the map
  InstanceData* data = DataPerInstanceThreaded.FindValue(instance, nullptr);
  if (data)
  {
    // If we are modifying the volume, interpolate back to 1.0
    if (mModifyingVolumeThreaded && data->mVolumeModifier)
    {
      data->mVolumeModifier->Reset(data->mVolumeModifier->GetCurrentVolume(), 1.0f,
        cPropertyChangeFrames, cPropertyChangeFrames);
      data->mVolumeModifier = nullptr;
    }

    // Delete the data object
    delete data;

    // Remove the instance from the map
    DataPerInstanceThreaded.Erase(instance);
  }
}

//**************************************************************************************************
float TagObject::GetVolume()
{
  return mVolume.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void TagObject::SetVolume(float volume, float time)
{
  mVolume.Set(volume, AudioThreads::MainThread);

  // TODO -- could this crash because it's not saving a handle?
  Z::gSound->Mixer.AddTask(CreateFunctor(&TagObject::SetVolumeThreaded, this, volume, time), nullptr);
}

//**************************************************************************************************
float TagObject::GetEQBandGain(EqualizerBands::Enum whichBand)
{
  return mEqualizerGainValues[whichBand];
}

//**************************************************************************************************
void TagObject::SetEQBandGain(EqualizerBands::Enum whichBand, float gain)
{
  mEqualizerGainValues[whichBand] = gain;

  // TODO -- could this crash because it's not saving a handle?
  Z::gSound->Mixer.AddTask(CreateFunctor(&TagObject::SetEQBandGainThreaded, this, whichBand, gain), nullptr);
}

//**************************************************************************************************
void TagObject::InterpolateEQBandsThreaded(float* values, float timeToInterpolate)
{
  if (!values)
    return;

  memcpy(mEqualizerGainValuesThreaded, values, sizeof(float) * EqualizerBands::Count);

  // Set the value on existing equalizers. If new ones are created they will copy settings.
  forRange(InstanceData* data, DataPerInstanceThreaded.Values())
    data->mEqualizer->InterpolateBands(values, timeToInterpolate);

  delete values;
}

//**************************************************************************************************
float TagObject::GetCompressorThresholdDb()
{
  return mCompressorThreshold;
}

//**************************************************************************************************
void TagObject::SetCompressorThresholdDb(float decibels)
{
  mCompressorThreshold = decibels;

  // TODO no handle
  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetThreshold, &CompressorObjectThreaded,
    decibels), nullptr);
}

//**************************************************************************************************
float TagObject::GetCompressorAttackMs()
{
  return mCompressorAttackMs;
}

//**************************************************************************************************
void TagObject::SetCompressorAttackMs(float milliseconds)
{
  mCompressorAttackMs = milliseconds;

  // TODO no handle
  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetAttackMSec, &CompressorObjectThreaded,
    milliseconds), nullptr);
}

//**************************************************************************************************
float TagObject::GetCompressorReleaseMs()
{
  return mCompressorReleaseMs;
}

//**************************************************************************************************
void TagObject::SetCompressorReleaseMs(float milliseconds)
{
  mCompressorReleaseMs = milliseconds;

  // TODO no handle
  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetReleaseMSec, &CompressorObjectThreaded,
    milliseconds), nullptr);
}

//**************************************************************************************************
float TagObject::GetCompresorRatio()
{
  return mCompressorRatio;
}

//**************************************************************************************************
void TagObject::SetCompressorRatio(float ratio)
{
  mCompressorRatio = ratio;

  // TODO no handle
  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetRatio, &CompressorObjectThreaded,
    ratio), nullptr);
}

//**************************************************************************************************
float TagObject::GetCompresorKneeWidth()
{
  return mCompressorKnee;
}

//**************************************************************************************************
void TagObject::SetCompressorKneeWidth(float knee)
{
  mCompressorKnee = knee;

  // TODO no handle
  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetKneeWidth, &CompressorObjectThreaded,
    knee), nullptr);
}

//**************************************************************************************************
void TagObject::RemoveTag()
{
  // TODO remove all instances
}

//**************************************************************************************************
void TagObject::ProcessInstanceThreaded(BufferType* instanceOutput, unsigned channels, SoundInstance* instance)
{
  // If this is the first instance for this mix, update
  if (mMixVersionThreaded != Z::gSound->Mixer.mMixVersionThreaded)
    UpdateForMixThreaded(instanceOutput->Size() / channels, channels);

  // Check if we are using the equalizer
  if (mUseEqualizer.Get(AudioThreads::MixThread))
  {
    InstanceData* data = DataPerInstanceThreaded.FindValue(instance, nullptr);
    ErrorIf(!data, "InstanceData was not created in tag's map");

    // If the equalizer filter does not exist for this instance, create it
    if (!data->mEqualizer)
      data->mEqualizer = new Equalizer(mEqualizerGainValuesThreaded);

    // Create a temporary buffer for the equalizer output
    BufferType processedOutput(instanceOutput->Size());

    // Apply the filter to all samples
    data->mEqualizer->ProcessBuffer(instanceOutput->Data(), processedOutput.Data(), channels,
      instanceOutput->Size());

    // Move the equalizer output into the instanceOutput buffer
    instanceOutput->Swap(processedOutput);
  }

  // Check if we are using the compressor and there are volume values
  if (mUseCompressor.Get(AudioThreads::MixThread) && !mCompressorVolumesThreaded.Empty())
  {
    // Apply the corresponding compressor volume to each sample
    int i = 0;
    forRange(float& sample, instanceOutput->All())
      sample *= mCompressorVolumesThreaded[i++];
  }
}

//**************************************************************************************************
BufferType* TagObject::GetTotalInstanceOutputThreaded(unsigned howManyFrames, unsigned channels)
{
  // Create a temporary buffer to get output from each instance
  BufferType instanceBuffer(howManyFrames * channels);
  // Resize the total output buffer
  mTotalInstanceOutputThreaded.Resize(howManyFrames * channels);
  // Set all samples to zero
  memset(mTotalInstanceOutputThreaded.Data(), 0, sizeof(float) * mTotalInstanceOutputThreaded.Size());

  forRange(InstanceDataMapType::pair mapPair, DataPerInstanceThreaded.All())
  {
    SoundInstance* instance = mapPair.first;

    // Check if the instance has valid output
    if (instance->GetOutputForThisMixThreaded(&instanceBuffer, channels))
    {
      // Get the instance's attenuated volume (SoundEmitters, VolumeNodes, etc.)
      float attenuatedVolume = instance->GetAttenuationThisMixThreaded();
      // Use the size of either the total output or instance output, whichever is smaller
      unsigned limit = Math::Min(mTotalInstanceOutputThreaded.Size(), instanceBuffer.Size());

      // Add the instance output into the total output, adjusting with tag volume
      // and attenuated instance volume
      for (unsigned i = 0; i < limit; ++i)
        mTotalInstanceOutputThreaded[i] += instanceBuffer[i] * attenuatedVolume 
          * mVolume.Get(AudioThreads::MixThread);
    }
  }

  return &mTotalInstanceOutputThreaded;
}

//**************************************************************************************************
void TagObject::UpdateForMixThreaded(unsigned howManyFrames, unsigned channels)
{
  if (mMixVersionThreaded == Z::gSound->Mixer.mMixVersionThreaded)
    return;

  // Check if we are using the compressor
  if (mUseCompressor.Get(AudioThreads::MixThread))
  {
    BufferType* compressorInput;

    // If we are not using another tag for the compressor input, get the output from this tag
    if (!mCompressorInputTag.Get(AudioThreads::MixThread))
      compressorInput = GetTotalInstanceOutputThreaded(howManyFrames, channels);
    // Otherwise get the output from the other tag
    else
      compressorInput = mCompressorInputTag.Get(AudioThreads::MixThread)->
        GetTotalInstanceOutputThreaded(howManyFrames, channels);

    // Reset the buffer of volume modifiers
    mCompressorVolumesThreaded.Clear();

    // Check if there was output from the tag
    if (!compressorInput->Empty())
    {
      // Reset the volume buffer to all 1.0 (this will become volume multipliers when 
      // processed by the compressor filter)
      mCompressorVolumesThreaded.Resize(compressorInput->Size(), 1.0f);

      // Run the compressor filter, using the tag output as the envelope input
      CompressorObjectThreaded.ProcessBuffer(mCompressorVolumesThreaded.Data(), compressorInput->Data(),
        mCompressorVolumesThreaded.Data(), channels, mCompressorVolumesThreaded.Size());
    }
  }

  mMixVersionThreaded = Z::gSound->Mixer.mMixVersionThreaded;
}

//**************************************************************************************************
void TagObject::SetVolumeThreaded(float volume, float time)
{
  mModifyingVolumeThreaded = true;

  // Determine how many frames to change the volume over, keeping a minimum value
  unsigned frames = Math::Max((unsigned)(time * cSystemSampleRate), cPropertyChangeFrames);

  // Set the volume modifier for each tagged instance
  forRange(InstanceDataMapType::pair mapPair, DataPerInstanceThreaded.All())
  {
    SoundInstance* instance = mapPair.first;
    InstanceData* data = mapPair.second;

    if (!data->mVolumeModifier)
      data->mVolumeModifier = instance->GetAvailableVolumeModThreaded();

    data->mVolumeModifier->Reset(data->mVolumeModifier->GetCurrentVolume(), volume, frames, 0);
  }
}

//**************************************************************************************************
void TagObject::SetEQBandGainThreaded(EqualizerBands::Enum whichBand, float gain)
{
  mEqualizerGainValuesThreaded[whichBand] = gain;

  // Set the value on existing equalizers. If new ones are created they will copy settings.
  forRange(InstanceData* data, DataPerInstanceThreaded.Values())
    data->mEqualizer->SetBandGain(whichBand, gain);
}

//**************************************************************************************************
TagObject::InstanceData::InstanceData() :
  mVolumeModifier(nullptr),
  mEqualizer(nullptr)
{

}

//**************************************************************************************************
TagObject::InstanceData::~InstanceData()
{
  if (mEqualizer)
    delete mEqualizer;
  if (mVolumeModifier)
    mVolumeModifier->Active = false;
}

//---------------------------------------------------------------------------------------- Sound Tag

//**************************************************************************************************
ZilchDefineType(SoundTagDisplay, builder, type)
{
}

//**************************************************************************************************
String SoundTagDisplay::GetName(HandleParam object)
{
  SoundTag* soundTag = object.Get<SoundTag*>(GetOptions::AssertOnNull);
  return BuildString("SoundTag: ", soundTag->Name);
}

//**************************************************************************************************
String SoundTagDisplay::GetDebugText(HandleParam object)
{
  return GetName(object);
}

//**************************************************************************************************
String SoundTagToString(const BoundType* type, const byte* instance)
{
  SoundTag* soundTag = (SoundTag*)instance;
  return BuildString("SoundTag: ", soundTag->Name);
}

//**************************************************************************************************
ZilchDefineType(SoundTag, builder, type)
{
  ZeroBindDocumented();
  type->ToStringFunction = &SoundTagToString;

  ZilchBindGetterSetter(Volume);
  ZilchBindGetterSetter(Decibels);
  ZilchBindGetterSetter(UseEqualizer);
  ZilchBindGetterSetter(EQLowPassGain);
  ZilchBindGetterSetter(EQBand1Gain);
  ZilchBindGetterSetter(EQBand2Gain);
  ZilchBindGetterSetter(EQBand3Gain);
  ZilchBindGetterSetter(EQHighPassGain);
  ZilchBindGetterSetter(UseCompressor);
  ZilchBindGetterSetter(CompressorThreshold);
  ZilchBindGetterSetter(CompressorAttack);
  ZilchBindGetterSetter(CompressorRelease);
  ZilchBindGetterSetter(CompressorRatio);
  ZilchBindGetterSetter(CompressorKneeWidth);
  ZilchBindGetterSetter(InstanceLimit);
  ZilchBindGetter(InstanceCount);
  ZilchBindGetterSetter(Paused);
  ZilchBindGetter(Instances);
  ZilchBindMethod(TagSound);
  ZilchBindMethod(UnTagSound);
  ZilchBindMethod(StopSounds);
  ZilchBindMethod(InterpolateVolume);
  ZilchBindMethod(InterpolateDecibels);
  ZilchBindMethod(EQSetAllBands);
  ZilchBindGetterSetter(TagForDucking);

  ZeroBindEvent(Events::AddedInstanceToTag, SoundEvent);
  ZeroBindEvent(Events::TagHasNoInstances, SoundEvent);
}

//**************************************************************************************************
SoundTag::SoundTag() :
  mCompressorTag(nullptr),
  mTagObject(nullptr)
{
  Z::gSound->mSoundTags.PushBack(this);

  if (Z::gSound->mSoundSpaceCounter > 0)
    CreateTag();
}

//**************************************************************************************************
SoundTag::~SoundTag()
{
  Z::gSound->mSoundTags.Erase(this);

  if (mTagObject)
    ReleaseTag();
}

//**************************************************************************************************
void SoundTag::Unload()
{
  SoundInstanceList.Clear();
  mCompressorTag = nullptr;
}

//**************************************************************************************************
void SoundTag::TagSound(HandleOf<SoundInstance>& instanceHandle)
{
  // Check if this sound is already tagged
  if (SoundInstanceList.Contains(instanceHandle))
    return;

  SoundInstance* instance = instanceHandle;

  // If we have an instance limit and we've reached it, stop the sound and return
  if (GetInstanceLimit() > 0 && SoundInstanceList.Size() >= GetInstanceLimit())
  {
    instance->Stop();
    return;
  }

  TagObject* tag = mTagObject;

  // If the tag is currently paused, pause the instance
  if (tag && tag->mPaused.Get(AudioThreads::MainThread))
    instance->SetPaused(true);

  // Add the instance to the list
  SoundInstanceList.PushBack(instanceHandle);
  instance->SoundTags.PushBack(this);

  // Add the instance to the tag object
  if (tag)
    Z::gSound->Mixer.AddTask(CreateFunctor(&TagObject::AddInstanceThreaded, tag, instance), nullptr);

  SoundEvent event;
  DispatchEvent(Events::AddedInstanceToTag, &event);
}

//**************************************************************************************************
void SoundTag::UnTagSound(HandleOf<SoundInstance>& instanceHandle)
{
  // First check if the instance has been added to this tag
  if (!SoundInstanceList.Contains(instanceHandle))
    return;

  SoundInstance* instance = instanceHandle;

  // Remove the instance from this tag's list
  SoundInstanceList.EraseValue(instanceHandle);
  instance->SoundTags.EraseValue(this);

  // Remove it from the tag object
  if (mTagObject)
    Z::gSound->Mixer.AddTask(CreateFunctor(&TagObject::RemoveInstanceThreaded, *mTagObject, instance), nullptr);

  if (SoundInstanceList.Empty())
  {
    SoundEvent event;
    DispatchEvent(Events::TagHasNoInstances, &event);
  }
}

//**************************************************************************************************
void SoundTag::StopSounds()
{
  forRange(SoundInstance* instance, SoundInstanceList.All())
    instance->Stop();
}

//**************************************************************************************************
bool SoundTag::GetPaused()
{
  if (mTagObject)
    return mTagObject->mPaused.Get(AudioThreads::MainThread);
  else
    return false;
}

//**************************************************************************************************
void SoundTag::SetPaused(bool pause)
{
  if (!mTagObject)
    return;

  mTagObject->mPaused.Set(pause, AudioThreads::MainThread);

  forRange(SoundInstance* instance, SoundInstanceList.All())
    instance->SetPaused(pause);
}

//**************************************************************************************************
int SoundTag::GetInstanceCount()
{
  return SoundInstanceList.Size();
}

//**************************************************************************************************
InstanceListType::range SoundTag::GetInstances()
{
  return SoundInstanceList.All();
}

//**************************************************************************************************
float SoundTag::GetVolume()
{
  if (mTagObject)
    return mTagObject->GetVolume();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetVolume(float value)
{
  InterpolateVolume(value, 0.0f);
}

//**************************************************************************************************
void SoundTag::InterpolateVolume(float value, float time)
{
  if (mTagObject)
  {
    value = Math::Max(value, 0.0f);
    time = Math::Max(time, 0.0f);
    mTagObject->SetVolume(value, time);
  }
}

//**************************************************************************************************
float SoundTag::GetDecibels()
{
  if (mTagObject)
    return VolumeToDecibels(mTagObject->GetVolume());
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetDecibels(float decibels)
{
  InterpolateDecibels(decibels, 0.0f);
}

//**************************************************************************************************
void SoundTag::InterpolateDecibels(float decibels, float interpolationTime)
{
  InterpolateVolume(DecibelsToVolume(decibels), interpolationTime);
}

//**************************************************************************************************
float SoundTag::GetEQLowPassGain()
{
  if (mTagObject)
    return mTagObject->GetEQBandGain(EqualizerBands::Below80);
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetEQLowPassGain(float gain)
{
  if (mTagObject)
    mTagObject->SetEQBandGain(EqualizerBands::Below80, Math::Clamp(gain, 0.0f, cMaxVolumeValue));
}

//**************************************************************************************************
float SoundTag::GetEQBand1Gain()
{
  if (mTagObject)
    return mTagObject->GetEQBandGain(EqualizerBands::At150);
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetEQBand1Gain(float gain)
{
  if (mTagObject)
    mTagObject->SetEQBandGain(EqualizerBands::At150, Math::Clamp(gain, 0.0f, cMaxVolumeValue));
}

//**************************************************************************************************
float SoundTag::GetEQBand2Gain()
{
  if (mTagObject)
    return mTagObject->GetEQBandGain(EqualizerBands::At600);
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetEQBand2Gain(float gain)
{
  if (mTagObject)
    mTagObject->SetEQBandGain(EqualizerBands::At600, Math::Clamp(gain, 0.0f, cMaxVolumeValue));
}

//**************************************************************************************************
float SoundTag::GetEQBand3Gain()
{
  if (mTagObject)
    return mTagObject->GetEQBandGain(EqualizerBands::At2500);
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetEQBand3Gain(float gain)
{
  if (mTagObject)
    mTagObject->SetEQBandGain(EqualizerBands::At2500, Math::Clamp(gain, 0.0f, cMaxVolumeValue));
}

//**************************************************************************************************
float SoundTag::GetEQHighPassGain()
{
  if (mTagObject)
    return mTagObject->GetEQBandGain(EqualizerBands::Above5000);
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetEQHighPassGain(float gain)
{
  if (mTagObject)
    mTagObject->SetEQBandGain(EqualizerBands::Above5000, Math::Clamp(gain, 0.0f, cMaxVolumeValue));
}

//**************************************************************************************************
void SoundTag::EQSetAllBands(float below80Hz, float at150Hz, float at600Hz, 
  float at2500Hz, float above5000Hz, float timeToInterpolate)
{
  if (mTagObject)
  {
    float* values = new float[EqualizerBands::Count];
    values[EqualizerBands::Below80] = Math::Clamp(below80Hz, 0.0f, cMaxVolumeValue);
    values[EqualizerBands::At150] = Math::Clamp(at150Hz, 0.0f, cMaxVolumeValue);
    values[EqualizerBands::At600] = Math::Clamp(at600Hz, 0.0f, cMaxVolumeValue);
    values[EqualizerBands::At2500] = Math::Clamp(at2500Hz, 0.0f, cMaxVolumeValue);
    values[EqualizerBands::Above5000] = Math::Clamp(above5000Hz, 0.0f, cMaxVolumeValue);

    Z::gSound->Mixer.AddTask(CreateFunctor(&TagObject::InterpolateEQBandsThreaded, *mTagObject,
      values, timeToInterpolate), nullptr);
  }
}

//**************************************************************************************************
float SoundTag::GetCompressorThreshold()
{
  if (mTagObject)
    return mTagObject->GetCompressorThresholdDb();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetCompressorThreshold(float decibels)
{
  if (mTagObject)
    mTagObject->SetCompressorThresholdDb(Math::Clamp(decibels, cMinDecibelsValue, cMaxDecibelsValue));
}

//**************************************************************************************************
float SoundTag::GetCompressorAttack()
{
  if (mTagObject)
    return mTagObject->GetCompressorAttackMs();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetCompressorAttack(float attack)
{
  if (mTagObject)
    mTagObject->SetCompressorAttackMs(Math::Max(0.0f, attack));
}

//**************************************************************************************************
float SoundTag::GetCompressorRelease()
{
  if (mTagObject)
    return mTagObject->GetCompressorReleaseMs();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetCompressorRelease(float release)
{
  if (mTagObject)
    mTagObject->SetCompressorReleaseMs(Math::Max(release, 0.0f));
}

//**************************************************************************************************
float SoundTag::GetCompressorRatio()
{
  if (mTagObject)
    return mTagObject->GetCompresorRatio();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetCompressorRatio(float ratio)
{
  if (mTagObject)
    mTagObject->SetCompressorRatio(ratio);
}

//**************************************************************************************************
float SoundTag::GetCompressorKneeWidth()
{
  if (mTagObject)
    return mTagObject->GetCompresorKneeWidth();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetCompressorKneeWidth(float knee)
{
  if (mTagObject)
    mTagObject->SetCompressorKneeWidth(knee);
}

//**************************************************************************************************
SoundTag* SoundTag::GetTagForDucking()
{
  return mCompressorTag;
}

//**************************************************************************************************
void SoundTag::SetTagForDucking(SoundTag* tag)
{
  mCompressorTag = tag;

  if (mTagObject)
  {
    if (tag)
      mTagObject->mCompressorInputTag.Set(tag->mTagObject, AudioThreads::MainThread);
    else
      mTagObject->mCompressorInputTag.Set(nullptr, AudioThreads::MainThread);
  }
}

//**************************************************************************************************
bool SoundTag::GetUseEqualizer()
{
  if (mTagObject)
    return mTagObject->mUseEqualizer.Get(AudioThreads::MainThread);
  else
    return false;
}

//**************************************************************************************************
void SoundTag::SetUseEqualizer(bool useEQ)
{
  if (mTagObject)
    mTagObject->mUseEqualizer.Set(useEQ, AudioThreads::MainThread);
}

//**************************************************************************************************
bool SoundTag::GetUseCompressor()
{
  if (mTagObject)
    return mTagObject->mUseCompressor.Get(AudioThreads::MainThread);
  else
    return false;
}

//**************************************************************************************************
void SoundTag::SetUseCompressor(bool useCompressor)
{
  if (mTagObject)
    mTagObject->mUseCompressor.Set(useCompressor, AudioThreads::MainThread);
}

//**************************************************************************************************
float SoundTag::GetInstanceLimit()
{
  if (mTagObject)
    return (float)mTagObject->mInstanceLimit;
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetInstanceLimit(float limit)
{
  if (mTagObject)
    mTagObject->mInstanceLimit = (int)limit;
}

//**************************************************************************************************
void SoundTag::CreateTag()
{
  if (!mTagObject)
    mTagObject = new TagObject();
}

//**************************************************************************************************
void SoundTag::ReleaseTag()
{
  if (mTagObject)
  {
    mTagObject->RemoveTag();
    mTagObject = nullptr;
  }
}

//-------------------------------------------------------------------------------- Sound Tag Manager

ImplementResourceManager(SoundTagManager, SoundTag);

//**************************************************************************************************
SoundTagManager::SoundTagManager(BoundType* resourceType) : ResourceManager(resourceType)
{
  AddLoader("SoundTag", new TextDataFileLoader<SoundTagManager>());
  DefaultResourceName = "DefaultSoundTag";
  mCategory = "Sound";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.SoundTag.data"));
  mCanCreateNew = true;
  mCanDuplicate = true;
  mExtension = DataResourceExtension;
}

} // namespace Zero
