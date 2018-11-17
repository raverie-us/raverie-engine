///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

using namespace AudioConstants;

//-------------------------------------------------------------------------------------- Sound Space

//**************************************************************************************************
ZilchDefineType(SoundSpace, builder, type)
{
  ZeroBindComponent();
  type->AddAttribute(ObjectAttributes::cCore);
  ZeroBindDocumented();
  ZeroBindDependency(TimeSpace);

  ZilchBindFieldProperty(mPauseWithTimeSpace);
  ZilchBindFieldProperty(mPitchWithTimeSpace);

  ZilchBindGetterSetter(Paused);
  ZilchBindGetterSetter(Volume);
  ZilchBindGetterSetter(MuteAudio);
  ZilchBindGetterSetter(Decibels);
  ZilchBindGetterSetter(Pitch);
  ZilchBindGetterSetter(Semitones);
  ZilchBindGetter(InputNode)->AddAttribute(DeprecatedAttribute);
  ZilchBindGetter(OutputNode)->AddAttribute(DeprecatedAttribute);
  ZilchBindGetter(SoundNodeInput);
  ZilchBindGetter(SoundNodeOutput);
  ZilchBindMethod(InterpolatePitch);
  ZilchBindMethod(InterpolateSemitones);
  ZilchBindMethod(InterpolateVolume);
  ZilchBindMethod(InterpolateDecibels);
  ZilchBindMethod(PlayCue);
  ZilchBindMethod(PlayCuePaused);
}

//**************************************************************************************************
SoundSpace::SoundSpace() : 
  mPauseWithTimeSpace(true),
  mPitchWithTimeSpace(true),
  mPitchNode(nullptr), 
  mLevelPaused(false),
  mEditorMode(false)
{

}

//**************************************************************************************************
SoundSpace::~SoundSpace()
{
  // Remove this space from the system's list
  Z::gSound->RemoveSoundSpace(this, mEditorMode);
  // Disconnect the nodes
  mSoundNodeOutput->DisconnectThisAndAllInputs();
}

//**************************************************************************************************
void SoundSpace::Initialize(CogInitializer& config)
{
  // Are we in editor mode?
  mEditorMode = !GetGameSession() || GetGameSession()->IsEditorMode();
  // Add this space to the system's list
  Z::gSound->AddSoundSpace(this, mEditorMode);

  // Create the input node
  mSpaceNodeID = Z::gSound->mCounter++;
  String name = "Space";
  if (mEditorMode)
    name = "EditorSpace";
  mSoundNodeInput = new CombineAndPauseNode(name, mSpaceNodeID);

  // Create the volume node as the output node
  mSoundNodeOutput = new VolumeNode(name, mSpaceNodeID);
  
  mSoundNodeOutput->AddInputNode(mSoundNodeInput);

  Z::gSound->mOutputNode->AddInputNode(mSoundNodeOutput);
}

//**************************************************************************************************
void SoundSpace::Serialize(Serializer& stream)
{
  SerializeNameDefault(mPauseWithTimeSpace, true);
  SerializeNameDefault(mPitchWithTimeSpace, true);
}

//**************************************************************************************************
float SoundSpace::GetVolume()
{
  return mSoundNodeOutput->GetVolume();
}

//**************************************************************************************************
void SoundSpace::SetVolume(float value)
{
  InterpolateVolume(value, 0.0f);
}

//**************************************************************************************************
bool SoundSpace::GetMuteAudio()
{
  if (!mSoundNodeInput)
    return false;

  return mSoundNodeInput->GetMuted();
}

//**************************************************************************************************
void SoundSpace::SetMuteAudio(bool mute)
{
  if (!mSoundNodeInput)
    return;

  mSoundNodeInput->SetMuted(mute);
}

//**************************************************************************************************
void SoundSpace::InterpolateVolume(float value, float interpolationTime)
{
  mSoundNodeOutput->InterpolateVolume(Math::Max(value, 0.0f), interpolationTime);
}

//**************************************************************************************************
float SoundSpace::GetDecibels()
{
  return VolumeToDecibels(mSoundNodeOutput->GetVolume());
}

//**************************************************************************************************
void SoundSpace::SetDecibels(float decibels)
{
  InterpolateDecibels(decibels, 0.0f);
}

//**************************************************************************************************
void SoundSpace::InterpolateDecibels(float decibels, float interpolationTime)
{
  mSoundNodeOutput->InterpolateVolume(DecibelsToVolume(decibels), interpolationTime);
}

//**************************************************************************************************
float SoundSpace::GetPitch()
{
  if (mPitchNode)
    return mPitchNode->GetPitch();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundSpace::SetPitch(float pitch)
{
  InterpolatePitch(pitch, 0.0f);
}

//**************************************************************************************************
void SoundSpace::InterpolatePitch(float pitch, float time)
{
  if (!mPitchNode)
  {
    mPitchNode = new PitchNode("Space", mSpaceNodeID);
    mSoundNodeInput->InsertNodeAfter(mPitchNode);
  }

  mPitchNode->InterpolatePitch(pitch, time);
}

//**************************************************************************************************
float SoundSpace::GetSemitones()
{
  if (mPitchNode)
    return mPitchNode->GetSemitones();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundSpace::SetSemitones(float pitch)
{
  InterpolateSemitones(pitch, 0.0f);
}

//**************************************************************************************************
void SoundSpace::InterpolateSemitones(float semitones, float time)
{
  semitones = Math::Clamp(semitones, cMinSemitonesValue, cMaxSemitonesValue);

  if (!mPitchNode)
  {
    mPitchNode = new PitchNode("Space", mSpaceNodeID);
    mSoundNodeInput->InsertNodeAfter(mPitchNode);
  }

  mPitchNode->InterpolateSemitones(semitones, time);
}

//**************************************************************************************************
bool SoundSpace::GetPaused()
{
  return mSoundNodeInput->GetPaused();
}

//**************************************************************************************************
void SoundSpace::SetPaused(bool pause)
{
  mSoundNodeInput->SetPaused(pause);
}

//**************************************************************************************************
HandleOf<SoundInstance> SoundSpace::PlayCue(SoundCue* cue)
{
  if (!cue)
    return nullptr;

  HandleOf<SoundInstance> instance = cue->PlayCue(this, mSoundNodeInput, false);

  if (instance)
  {
    SoundInstanceEvent eventToSend(instance);
    DispatchEvent(Events::SoundInstancePlayed, &eventToSend);
  }

  return instance;
}

//**************************************************************************************************
HandleOf<SoundInstance> SoundSpace::PlayCuePaused(SoundCue* cue)
{
  if (!cue)
    return nullptr;

  HandleOf<SoundInstance> instance = cue->PlayCue(this, mSoundNodeInput, true);

  if (instance)
  {
    SoundInstanceEvent eventToSend(instance);
    DispatchEvent(Events::SoundInstancePlayed, &eventToSend);
  }

  return instance;
}

//**************************************************************************************************
Zilch::HandleOf<Zero::SoundNode> SoundSpace::GetSoundNodeInput()
{
  return mSoundNodeInput;
}

//**************************************************************************************************
HandleOf<SoundNode> SoundSpace::GetInputNode()
{
  return mSoundNodeInput;
}

//**************************************************************************************************
Zilch::HandleOf<Zero::SoundNode> SoundSpace::GetSoundNodeOutput()
{
  return mSoundNodeOutput;
}

//**************************************************************************************************
HandleOf<SoundNode> SoundSpace::GetOutputNode()
{
  return mSoundNodeOutput;
}

//**************************************************************************************************
void SoundSpace::Update()
{
  // If this sound space should pause when the level is paused, check for handling that
  if (mPauseWithTimeSpace)
  {
    bool spacePaused = GetOwner()->has(TimeSpace)->GetGloballyPaused();

    // If the level was just paused
    if (spacePaused && !mLevelPaused)
      SetPaused(true);
    // If the level was just un-paused
    else if (!spacePaused && mLevelPaused)
      SetPaused(false);

    mLevelPaused = spacePaused;
  }

  float dt = GetOwner()->has(TimeSpace)->GetDtOrZero();
  float invDt = dt!= 0.0f ? (1.0f / dt) : 0.0f;

  // Check if this sound space should change pitch with time scale
  if (mPitchWithTimeSpace)
  {
    float scale = GetOwner()->has(TimeSpace)->mTimeScale;
    if (scale != 1.0f)
      InterpolatePitch(Math::Log2(scale), dt);
  }

  // Update emitters
  for(InList<SoundEmitter>::range r = mEmitters.All(); !r.Empty(); r.PopFront())
    r.Front().Update(dt);

  // Update listeners
  InList<SoundListener>::range r = mListeners.All();
  for(; !r.Empty(); r.PopFront())
    r.Front().Update(invDt);
}

//**************************************************************************************************
InList<SoundListener>* Zero::SoundSpace::GetListeners()
{
  return &mListeners;
}

} // namespace Zero
