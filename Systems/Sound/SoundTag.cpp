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
  mTagObject = new Audio::TagObject();
  mTagObject->ExternalInterface = this;
}

//**************************************************************************************************
SoundTag::~SoundTag()
{
  if (mTagObject)
  {
    mTagObject->ExternalInterface = nullptr;
    mTagObject->RemoveTag();
  }
}

//**************************************************************************************************
void SoundTag::Unload()
{
  SoundInstanceList.Clear();
  mCompressorTag = nullptr;
}

//**************************************************************************************************
void SoundTag::TagSound(SoundInstance* instance)
{
  mTagObject->AddInstance((Audio::SoundInstanceNode*)instance->GetSoundNode()->mNode);
  SoundInstanceList.PushBack(Handle(instance));
  instance->SoundTags.PushBack(this);
}

//**************************************************************************************************
void SoundTag::UnTagSound(SoundInstance* instance)
{
  mTagObject->RemoveInstance((Audio::SoundInstanceNode*)instance->GetSoundNode()->mNode);
  SoundInstanceList.EraseValue(Handle(instance));
  instance->SoundTags.EraseValue(this);
}

//**************************************************************************************************
void SoundTag::StopSounds()
{
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
  if (mTagObject)
  {
    if (pause)
      mTagObject->PauseInstances();
    else
      mTagObject->ResumeInstances();
  }
}

//**************************************************************************************************
int SoundTag::GetInstanceCount()
{
  return mTagObject->GetNumberOfInstances();
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
    mTagObject->SetVolume(value, time);
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
    mTagObject->SetBelow80HzGain(gain);
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
    mTagObject->Set150HzGain(gain);
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
    mTagObject->Set600HzGain(gain);
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
    mTagObject->Set2500HzGain(gain);
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
    mTagObject->SetAbove5000HzGain(gain);
}

//**************************************************************************************************
void SoundTag::EQSetAllBands(float below80Hz, float at150Hz, float at600Hz, 
  float at2500Hz, float above5000Hz, float timeToInterpolate)
{
  if (mTagObject)
    mTagObject->InterpolateAllBands(new Audio::TagObject::GainValues(below80Hz, at150Hz, at600Hz, 
      at2500Hz, above5000Hz), timeToInterpolate);
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
    mTagObject->SetCompressorAttackMSec(attack);
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
    mTagObject->SetCompressorReleaseMsec(release);
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
HandleOf<SoundTag> SoundTag::GetTagForDucking()
{
  return mCompressorTag;
}

//**************************************************************************************************
void SoundTag::SetTagForDucking(HandleOf<SoundTag> tag)
{
  mCompressorTag = tag;

  if (mTagObject)
  {
    if (tag)
      mTagObject->SetCompressorInputTag(tag->mTagObject);
    else
      mTagObject->SetCompressorInputTag(nullptr);
  }
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
    mTagObject->SetInstanceLimit(limit);
}

//**************************************************************************************************
void SoundTag::SendAudioEvent(const Audio::AudioEventType eventType, void * data)
{
  if (eventType == Audio::AudioEventType::Notify_TagAddedInstance)
  {
    SoundEvent event;
    DispatchEvent(Events::AddedInstanceToTag, &event);
  }
  else if (eventType == Audio::AudioEventType::Notify_TagIsUnreferenced)
  {
    SoundEvent event;
    DispatchEvent(Events::TagHasNoInstances, &event);
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
