///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

//Components
namespace Zero
{

namespace Events
{
  DefineEvent(AddedInstanceToTag);
  DefineEvent(TagHasNoInstances);
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
void SoundTag::TagSound(HandleOf<SoundInstance>& instance)
{
  // Check if this sound is already tagged
  if (SoundInstanceList.Contains(instance))
    return;

  // If we have an instance limit and we've reached it, stop the sound and return
  if (GetInstanceLimit() > 0 && SoundInstanceList.Size() >= GetInstanceLimit())
  {
    instance->Stop();
    return;
  }

  if (mTagObject)
    mTagObject->AddInstance((Audio::SoundInstanceNode*)instance->GetSoundNode()->mNode);
  SoundInstanceList.PushBack(instance);
  instance->SoundTags.PushBack(this);
}

//**************************************************************************************************
void SoundTag::UnTagSound(HandleOf<SoundInstance>& instance)
{
  if (mTagObject)
    mTagObject->RemoveInstance((Audio::SoundInstanceNode*)instance->GetSoundNode()->mNode);
  SoundInstanceList.EraseValue(instance);
  instance->SoundTags.EraseValue(this);
}

//**************************************************************************************************
void SoundTag::StopSounds()
{
  if (mTagObject)
    mTagObject->StopInstances();
}

//**************************************************************************************************
bool SoundTag::GetPaused()
{
  if (mTagObject)
    return mTagObject->GetPaused();
  else
    return false;
}

//**************************************************************************************************
void SoundTag::SetPaused(bool pause)
{
  if (!mTagObject)
    return;

  if (pause)
    mTagObject->PauseInstances();
  else
    mTagObject->ResumeInstances();
}

//**************************************************************************************************
int SoundTag::GetInstanceCount()
{
  if (mTagObject)
    return mTagObject->GetNumberOfInstances();
  else
    return 0;
}

//**************************************************************************************************
InstanceList::range SoundTag::GetInstances()
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
    mTagObject->SetVolume(Math::Max(value, 0.0f), time);
}

//**************************************************************************************************
float SoundTag::GetDecibels()
{
  if (mTagObject)
    return Z::gSound->VolumeToDecibels(mTagObject->GetVolume());
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
  if (mTagObject)
    mTagObject->SetVolume(Z::gSound->DecibelsToVolume(decibels), interpolationTime);
}

//**************************************************************************************************
float SoundTag::GetEQLowPassGain()
{
  if (mTagObject)
    return mTagObject->GetBelow80HzGain();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetEQLowPassGain(float gain)
{
  if (mTagObject)
    mTagObject->SetBelow80HzGain(Math::Max(gain, 0.0f));
}

//**************************************************************************************************
float SoundTag::GetEQBand1Gain()
{
  if (mTagObject)
    return mTagObject->Get150HzGain();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetEQBand1Gain(float gain)
{
  if (mTagObject)
    mTagObject->Set150HzGain(Math::Max(gain, 0.0f));
}

//**************************************************************************************************
float SoundTag::GetEQBand2Gain()
{
  if (mTagObject)
    return mTagObject->Get600HzGain();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetEQBand2Gain(float gain)
{
  if (mTagObject)
    mTagObject->Set600HzGain(Math::Max(gain, 0.0f));
}

//**************************************************************************************************
float SoundTag::GetEQBand3Gain()
{
  if (mTagObject)
    return mTagObject->Get2500HzGain();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetEQBand3Gain(float gain)
{
  if (mTagObject)
    mTagObject->Set2500HzGain(Math::Max(gain, 0.0f));
}

//**************************************************************************************************
float SoundTag::GetEQHighPassGain()
{
  if (mTagObject)
    return mTagObject->GetAbove5000HzGain();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetEQHighPassGain(float gain)
{
  if (mTagObject)
    mTagObject->SetAbove5000HzGain(Math::Max(gain, 0.0f));
}

//**************************************************************************************************
void SoundTag::EQSetAllBands(float below80Hz, float at150Hz, float at600Hz, 
  float at2500Hz, float above5000Hz, float timeToInterpolate)
{
  if (mTagObject)
    mTagObject->InterpolateAllBands(new Audio::TagObject::GainValues(Math::Max(below80Hz, 0.0f), 
      Math::Max(at150Hz, 0.0f), Math::Max(at600Hz, 0.0f), Math::Max(at2500Hz, 0.0f), 
      Math::Max(above5000Hz, 0.0f)), timeToInterpolate);
}

//**************************************************************************************************
float SoundTag::GetCompressorThreshold()
{
  if (mTagObject)
    return mTagObject->GetCompressorThreshold();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetCompressorThreshold(float decibels)
{
  if (mTagObject)
    mTagObject->SetCompressorThreshold(decibels);
}

//**************************************************************************************************
float SoundTag::GetCompressorAttack()
{
  if (mTagObject)
    return mTagObject->GetCompressorAttackMSec();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetCompressorAttack(float attack)
{
  if (mTagObject)
    mTagObject->SetCompressorAttackMSec(Math::Max(attack, 0.0f));
}

//**************************************************************************************************
float SoundTag::GetCompressorRelease()
{
  if (mTagObject)
    return mTagObject->GetCompressorReleaseMSec();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetCompressorRelease(float release)
{
  if (mTagObject)
    mTagObject->SetCompressorReleaseMsec(Math::Max(release, 0.0f));
}

//**************************************************************************************************
float SoundTag::GetCompressorRatio()
{
  if (mTagObject)
    return mTagObject->GetCompressorRatio();
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
    return mTagObject->GetCompressorKneeWidth();
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

  if (!mTagObject)
    return;

  if (tag)
    mTagObject->SetCompressorInputTag(tag->mTagObject);
  else
    mTagObject->SetCompressorInputTag(nullptr);
}

//**************************************************************************************************
bool SoundTag::GetUseEqualizer()
{
  if (mTagObject)
    return mTagObject->GetUseEqualizer();
  else
    return false;
}

//**************************************************************************************************
void SoundTag::SetUseEqualizer(bool useEQ)
{
  if (mTagObject)
    mTagObject->SetUseEqualizer(useEQ);
}

//**************************************************************************************************
bool SoundTag::GetUseCompressor()
{
  if (mTagObject)
    return mTagObject->GetUseCompressor();
  else
    return false;
}

//**************************************************************************************************
void SoundTag::SetUseCompressor(bool useCompressor)
{
  if (mTagObject)
    mTagObject->SetUseCompressor(useCompressor);
}

//**************************************************************************************************
float SoundTag::GetInstanceLimit()
{
  if (mTagObject)
    return (float)mTagObject->GetInstanceLimit();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundTag::SetInstanceLimit(float limit)
{
  if (mTagObject)
    mTagObject->SetInstanceLimit((int)limit);
}

//**************************************************************************************************
void SoundTag::SendAudioEvent(Audio::AudioEventTypes::Enum eventType)
{
  if (eventType == Audio::AudioEventTypes::TagAddedInstance)
  {
    SoundEvent event;
    DispatchEvent(Events::AddedInstanceToTag, &event);
  }
  else if (eventType == Audio::AudioEventTypes::TagIsUnreferenced)
  {
    SoundEvent event;
    DispatchEvent(Events::TagHasNoInstances, &event);
  }
}

//**************************************************************************************************
void SoundTag::CreateTag()
{
  if (!mTagObject)
  {
    mTagObject = new Audio::TagObject();
    mTagObject->ExternalInterface = this;
  }
}

//**************************************************************************************************
void SoundTag::ReleaseTag()
{
  if (mTagObject)
  {
    mTagObject->ExternalInterface = nullptr;
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

}
