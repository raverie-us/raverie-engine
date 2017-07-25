///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{
namespace Events
{
  DefineEvent(SoundStopped);
  DefineEvent(SoundLooped);
  DefineEvent(MusicBeat);
  DefineEvent(MusicBar);
  DefineEvent(MusicEighthNote);
  DefineEvent(MusicQuarterNote);
  DefineEvent(MusicHalfNote);
  DefineEvent(MusicWholeNote);
  DefineEvent(MusicCustomTime);

  DefineEvent(SoundCuePrePlay);
  DefineEvent(SoundCuePostPlay);
}

//----------------------------------------------------------------------------- Sound Instance Event

//**************************************************************************************************
ZilchDefineType(SoundInstanceEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetter(SoundInstance);
}

//----------------------------------------------------------------------------------- Sound Instance

//**************************************************************************************************
ZilchDefineType(SoundInstance, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(Volume);
  ZilchBindGetterSetter(Decibels);
  ZilchBindGetterSetter(Pitch);
  ZilchBindGetterSetter(Semitones);
  ZilchBindGetter(IsPlaying);
  ZilchBindGetter(SoundNode);
  ZilchBindMethod(InterpolatePitch);
  ZilchBindMethod(InterpolateSemitones);
  ZilchBindMethod(InterpolateVolume);
  ZilchBindMethod(InterpolateDecibels);
  ZilchBindGetterSetter(Paused);
  ZilchBindMethod(Stop);
  ZilchBindGetterSetter(Looping);
  ZilchBindGetterSetter(Time);
  ZilchBindGetter(FileLength);
  ZilchBindGetterSetter(EndTime);
  ZilchBindGetterSetter(CustomEventTime);
  ZilchBindGetter(SoundName);

  ZeroBindEvent(Events::SoundLooped, SoundInstanceEvent);
  ZeroBindEvent(Events::SoundStopped, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicBeat, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicBar, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicEighthNote, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicQuarterNote, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicHalfNote, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicWholeNote, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicCustomTime, SoundInstanceEvent);
}

//**************************************************************************************************
SoundInstance::SoundInstance(Status& status, SoundSpace* space, Audio::SoundAssetNode* asset, 
    float volume, float pitch) : 
  mSpace(space), 
  mAssetObject(asset), 
  mIsPlaying(false), 
  mIsPaused(false)
{
  // Create the SoundInstance
  Audio::SoundInstanceNode* instance = new Audio::SoundInstanceNode(status, "SoundInstance", 
    Z::gSound->mCounter++, mAssetObject, false, true, this);
  // If it was created successfully, set the data
  if (status.Succeeded())
  {
    instance->SetVolume(volume, 0.0f);
    if (pitch != 0.0f)
      instance->SetPitch((int)(Z::gSound->PitchToSemitones(pitch) * 100.0f), 0.0f);

    SoundNode* node = new SoundNode();
    node->mNode = instance;
    node->mCanReplace = false;
    node->mCanRemove = false;
    mSoundNode = node;
  }
  else
  {
    DoNotifyWarning("Audio Error", status.Message);

    if (instance)
      instance->DeleteThisNode();
  }
}

//**************************************************************************************************
float SoundInstance::GetVolume()
{
  if (mSoundNode->mNode)
    return ((Audio::SoundInstanceNode*)mSoundNode->mNode)->GetVolume();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundInstance::SetVolume(float newVolume)
{
  InterpolateVolume(newVolume, 0.0f);
}

//**************************************************************************************************
void SoundInstance::InterpolateVolume(float newVolume, float interpolationTime)
{
  if (mSoundNode->mNode)
    ((Audio::SoundInstanceNode*)mSoundNode->mNode)->SetVolume(newVolume, interpolationTime);
}

//**************************************************************************************************
float SoundInstance::GetDecibels()
{
  if (mSoundNode->mNode)
    return Z::gSound->VolumeToDecibels(((Audio::SoundInstanceNode*)mSoundNode->mNode)->GetVolume());
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundInstance::SetDecibels(float decibels)
{
  InterpolateDecibels(decibels, 0.0f);
}

//**************************************************************************************************
void SoundInstance::InterpolateDecibels(float decibels, float interpolationTime)
{
  if (mSoundNode->mNode)
    ((Audio::SoundInstanceNode*)mSoundNode->mNode)->SetVolume(Z::gSound->DecibelsToVolume(decibels), 
      interpolationTime);
}

//**************************************************************************************************
float SoundInstance::GetPitch()
{
  if (mSoundNode->mNode)
    return Z::gSound->SemitonesToPitch(((Audio::SoundInstanceNode*)mSoundNode->mNode)->GetPitch() / 100.0f);
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundInstance::SetPitch(float newPitch)
{
  InterpolatePitch(newPitch, 0.0f);
}

//**************************************************************************************************
void SoundInstance::InterpolatePitch(float newPitch, float interpolationTime)
{
  if (mSoundNode->mNode)
    ((Audio::SoundInstanceNode*)mSoundNode->mNode)->SetPitch((int)(Z::gSound->PitchToSemitones(newPitch) 
      * 100.0f), interpolationTime);
}

//**************************************************************************************************
float SoundInstance::GetSemitones()
{
  if (mSoundNode->mNode)
    return ((Audio::SoundInstanceNode*)mSoundNode->mNode)->GetPitch() / 100.0f;
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundInstance::SetSemitones(float newSemitones)
{
  InterpolateSemitones(newSemitones, 0.0f);
}

//**************************************************************************************************
void SoundInstance::InterpolateSemitones(float newSemitones, float interpolationTime)
{
  if (mSoundNode->mNode)
    ((Audio::SoundInstanceNode*)mSoundNode->mNode)->SetPitch((int)(newSemitones * 100), interpolationTime);
}

//**************************************************************************************************
bool SoundInstance::GetPaused()
{
  return mIsPaused;
}

//**************************************************************************************************
void SoundInstance::SetPaused(bool pause)
{
  if (!mSoundNode->mNode)
    return;

  // Should be set to pause and is not currently paused
  if (pause && !mIsPaused)
    ((Audio::SoundInstanceNode*)mSoundNode->mNode)->Pause();
  // Should be set to un-paused and is currently paused
  else if (!pause && mIsPaused)
    ((Audio::SoundInstanceNode*)mSoundNode->mNode)->Resume();
}

//**************************************************************************************************
void SoundInstance::Stop()
{
  if (mSoundNode->mNode)
    ((Audio::SoundInstanceNode*)mSoundNode->mNode)->Stop();
}

//**************************************************************************************************
bool SoundInstance::GetIsPlaying()
{
  return mIsPlaying;
}

//**************************************************************************************************
SoundNode* SoundInstance::GetSoundNode()
{
  return mSoundNode;
}

//**************************************************************************************************
bool SoundInstance::GetLooping()
{
  if (mSoundNode->mNode)
    return ((Audio::SoundInstanceNode*)mSoundNode->mNode)->GetLooping();
  else
    return false;
}

//**************************************************************************************************
void SoundInstance::SetLooping(bool loop)
{
  if (mSoundNode->mNode)
    ((Audio::SoundInstanceNode*)mSoundNode->mNode)->SetLooping(loop);
}

//**************************************************************************************************
float SoundInstance::GetTime()
{
  if (mSoundNode->mNode)
    return ((Audio::SoundInstanceNode*)mSoundNode->mNode)->GetTime();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundInstance::SetTime(float seconds)
{
  if (mSoundNode->mNode)
    ((Audio::SoundInstanceNode*)mSoundNode->mNode)->JumpTo(seconds);
}

//**************************************************************************************************
float SoundInstance::GetFileLength()
{
  return ((Audio::SoundAssetFromFile*)mAssetObject)->GetLengthOfFile();
}

//**************************************************************************************************
float SoundInstance::GetEndTime()
{
  if (mSoundNode->mNode)
    return ((Audio::SoundInstanceNode*)mSoundNode->mNode)->GetEndTime();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundInstance::SetEndTime(float seconds)
{
  if (mSoundNode->mNode)
    ((Audio::SoundInstanceNode*)mSoundNode->mNode)->SetEndTime(seconds);
}

//**************************************************************************************************
float SoundInstance::GetBeatsPerMinute()
{
  if (mSoundNode->mNode)
    return ((Audio::SoundInstanceNode*)mSoundNode->mNode)->GetBeatsPerMinute();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundInstance::SetBeatsPerMinute(float beats)
{
  if (mSoundNode->mNode)
    ((Audio::SoundInstanceNode*)mSoundNode->mNode)->SetBeatsPerMinute(beats);
}

//**************************************************************************************************
void SoundInstance::SetTimeSignature(float beats, float noteType)
{
  if (mSoundNode->mNode)
    ((Audio::SoundInstanceNode*)mSoundNode->mNode)->SetTimeSignature((int)beats, (int)noteType);
}

//**************************************************************************************************
float SoundInstance::GetCustomEventTime()
{
  if (mSoundNode->mNode)
    return ((Audio::SoundInstanceNode*)mSoundNode->mNode)->GetCustomNotifyTime();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundInstance::SetCustomEventTime(float seconds)
{
  if (mSoundNode->mNode)
    ((Audio::SoundInstanceNode*)mSoundNode->mNode)->SetCustomNotifyTime(seconds);
}

//**************************************************************************************************
Zero::StringParam SoundInstance::GetSoundName()
{
  return mAssetObject->Name;
}

//**************************************************************************************************
void SoundInstance::Play(bool loop, SoundTag* tag, Audio::SoundNode* outputNode, bool startPaused)
{
  // Save a pointer to the SoundInstance
  Audio::SoundInstanceNode* instance = (Audio::SoundInstanceNode*)mSoundNode->mNode;

  if (tag)
    tag->TagSound(this);
  
  instance->SetLooping(loop);

  // If there is an output node, add the instance as input
  if (outputNode)
    outputNode->AddInput(instance);
  // If there is no output node but there is a SoundSpace, add to direct output of space
  else if (mSpace)
    mSpace->GetInputNode()->AddInputNode(mSoundNode);

  mIsPlaying = true;

  if (!startPaused)
    instance->Resume();
  else
    mIsPaused = true;
}

//**************************************************************************************************
void SoundInstance::SendAudioEvent(const Audio::AudioEventType eventType, void* data)
{
  if (eventType == Audio::Notify_InstanceFinished)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::SoundStopped, &event);
    mIsPlaying = false;

    // Remove from any SoundTags
    for (unsigned i = 0; i < SoundTags.Size(); ++i)
      SoundTags[i]->SoundInstanceList.EraseValue(Handle(this));

    // Remove the SoundNode
    if (mSoundNode->mNode)
    {
      mSoundNode->mNode->DeleteThisNode();
      mSoundNode->mNode = nullptr;
    }
  }
  else if (eventType == Audio::Notify_InstanceLooped)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::SoundLooped, &event);
  }
  else if (eventType == Audio::Notify_MusicBeat)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::MusicBeat, &event);
  }
  else if (eventType == Audio::Notify_MusicBar)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::MusicBar, &event);
  }
  else if (eventType == Audio::Notify_MusicEighthNote)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::MusicEighthNote, &event);
  }
  else if (eventType == Audio::Notify_MusicQuarterNote)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::MusicQuarterNote, &event);
  }
  else if (eventType == Audio::Notify_MusicHalfNote)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::MusicHalfNote, &event);
  }
  else if (eventType == Audio::Notify_MusicWholeNote)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::MusicWholeNote, &event);
  }
  else if (eventType == Audio::Notify_MusicCustomTime)
  {
    SoundInstanceEvent event(this);
    DispatchEvent(Events::MusicCustomTime, &event);
  }
  else if (eventType == Audio::AudioEventType::Notify_InterpolationDone)
  {
    SoundEvent event;
    DispatchEvent(Events::AudioInterpolationDone, &event);
  }
  else
    mSoundNode->SendAudioEvent(eventType, data);
}

}//namespace Zero
