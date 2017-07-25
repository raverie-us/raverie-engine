///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{
namespace
{
  Math::Random gRandom;
}

//-------------------------------------------------------------------------------------- Sound Entry 

//**************************************************************************************************
ZilchDefineType(SoundEntryDisplay, builder, type)
{
}

//**************************************************************************************************
String SoundEntryDisplay::GetName(HandleParam object)
{
  SoundEntry* soundEntry = object.Get<SoundEntry*>(GetOptions::AssertOnNull);
  return BuildString("Sound: ", soundEntry->GetSound()->Name);
}

//**************************************************************************************************
String SoundEntryDisplay::GetDebugText(HandleParam object)
{
  return GetName(object);
}

//**************************************************************************************************
String SoundEntryDisplayText(const BoundType* type, const byte* data)
{
  SoundEntry* soundEntry = (SoundEntry*)data;
  return BuildString("Sound: ", soundEntry->GetSound()->Name);
}

//**************************************************************************************************
ZilchDefineType(SoundEntry, builder, type)
{
  ZeroBindDocumented();
  type->ToStringFunction = &SoundEntryDisplayText;
  type->Add(new SoundEntryDisplay());
  type->HandleManager = ZilchManagerId(PointerManager);

  ZilchBindGetterSetterProperty(Sound);
  ZilchBindFieldProperty(mWeight);
  ZilchBindFieldProperty(mStartTime);
  ZilchBindFieldProperty(mEndTime);
  ZilchBindFieldProperty(mLoopStartTime);
  ZilchBindFieldProperty(mLoopEndTime);
  ZilchBindFieldProperty(mLoopTailLength);
  ZilchBindFieldProperty(mCrossFadeLoopTail);
  ZilchBindMethodProperty(Preview);
  ZilchBindMethodProperty(StopPreview);
}

//**************************************************************************************************
SoundEntry::SoundEntry() : 
  mLoopEndTime(0), 
  mLoopStartTime(0), 
  mWeight(1.0f), 
  mStartTime(0.0f),
  mEndTime(0.0f),
  mLoopTailLength(0.0f),
  mSound(nullptr),
  mCrossFadeLoopTail(false)
{
  mSound = SoundManager::GetDefault();
  if (mSound)
  {
    mEndTime = mSound->GetLength();
    mLoopEndTime = mEndTime;
  }
}

//**************************************************************************************************
void SoundEntry::Serialize(Serializer& stream)
{
  SerializeResourceName(mSound, SoundManager);
  SerializeNameDefault(mWeight, 1.0f);
  SerializeNameDefault(mStartTime, 0.0f);
  SerializeNameDefault(mEndTime, 0.0f);
  SerializeNameDefault(mLoopEndTime, 0.0f);
  SerializeNameDefault(mLoopStartTime, 0.0f);
  SerializeNameDefault(mLoopTailLength, 0.0f);
  SerializeNameDefault(mCrossFadeLoopTail, false);
}

//**************************************************************************************************
Sound* SoundEntry::GetSound()
{
  return mSound;
}

//**************************************************************************************************
void SoundEntry::SetSound(Sound* sound)
{
  if (sound)
  {
    mSound = sound;
    mEndTime = sound->GetLength();
    mLoopEndTime = mEndTime;
    mLoopStartTime = 0;
    mLoopTailLength = 0;
  }
}

//**************************************************************************************************
void SoundEntry::Preview()
{
  Z::gSound->StopPreview();

  // Create a new SoundInstance with no SoundSpace
  Status status;
  SoundInstance *instance = new SoundInstance(status, nullptr, mSound->mSoundAsset, 1.0f, 0.0f); 

  if (!status.Failed())
  {
    // If instance was created successfully, play it without looping, 
    // without a tag, and without an output node
    instance->Play(false, nullptr, nullptr, false);
    // Save the SoundInstance
    Z::gSound->mPreviewInstance = instance;
    // Connect it to the audio engine's output node
    Z::gSound->mAudioSystem->AddNodeToOutput(instance->mSoundNode->mNode);
  }
  else
  {
    DoNotifyError("Error Previewing Sound", status.Message);

    SafeDelete(instance);
  }
}

//**************************************************************************************************
void SoundEntry::StopPreview()
{
  Z::gSound->StopPreview();
}

//---------------------------------------------------------------------------------- Sound Tag Entry 

//**************************************************************************************************
ZilchDefineType(SoundTagEntryDisplay, builder, type)
{
}

//**************************************************************************************************
String SoundTagEntryDisplay::GetName(HandleParam object)
{
  SoundTagEntry* soundTagEntry = object.Get<SoundTagEntry*>(GetOptions::AssertOnNull);
  return BuildString("Tag: ", soundTagEntry->GetSoundTag()->Name);
}

//**************************************************************************************************
String SoundTagEntryDisplay::GetDebugText(HandleParam object)
{
  SoundTagEntry* soundTagEntry = object.Get<SoundTagEntry*>(GetOptions::AssertOnNull);
  return BuildString("Tag: ", soundTagEntry->GetSoundTag()->Name);
}

//**************************************************************************************************
String SoundTagEntryDisplayText(const BoundType* type, const byte* data)
{
  SoundTagEntry* soundEntry = (SoundTagEntry*)data;
  return BuildString("Tag: ", soundEntry->GetSoundTag()->Name);
}

//**************************************************************************************************
ZilchDefineType(SoundTagEntry, builder, type)
{
  ZeroBindDocumented();
  type->HandleManager = ZilchManagerId(PointerManager);
  type->ToStringFunction = &SoundTagEntryDisplayText;
  type->Add(new SoundTagEntryDisplay());

  ZilchBindGetterSetterProperty(SoundTag);
}

//**************************************************************************************************
SoundTagEntry::SoundTagEntry() : 
  mSoundTag(SoundTagManager::GetDefault())
{

}

//**************************************************************************************************
void SoundTagEntry::Serialize(Serializer& stream)
{
  SerializeResourceName(mSoundTag, SoundTagManager);
}

//**************************************************************************************************
void SoundTagEntry::SetSoundTag(SoundTag* tag)
{
  if (tag)
    mSoundTag = tag;
}

//**************************************************************************************************
SoundTag* SoundTagEntry::GetSoundTag()
{
  return mSoundTag;
}

//---------------------------------------------------------------------------------------- Sound Cue

//**************************************************************************************************
ZilchDefineType(SoundCueDisplay, builder, type)
{
}

//**************************************************************************************************
String SoundCueDisplay::GetName(HandleParam object)
{
  return "SoundCue";
}

//**************************************************************************************************
String SoundCueDisplay::GetDebugText(HandleParam object)
{
  return "SoundCue";
}

//**************************************************************************************************
ZilchDefineType(SoundCue, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(PlayMode);
  ZilchBindGetterSetterProperty(SelectMode);
  ZilchBindGetterSetterProperty(Volume)->Add(new EditorRange(0.0f, 2.0f, 0.01f));
  ZilchBindGetterSetterProperty(Decibels)->Add(new EditorRange(-32.0f, 6.0f, 0.1f));
  ZilchBindFieldProperty(mUseDecibelVariation)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindFieldProperty(mVolumeVariation)->Add(new EditorRange(0.0f, 1.0f, 0.01f))->
    ZeroFilterNotBool(mUseDecibelVariation);
  ZilchBindFieldProperty(mDecibelVariation)->Add(new EditorRange(0.0f, 6.0f, 0.1f))->
    ZeroFilterBool(mUseDecibelVariation);
  ZilchBindGetterSetterProperty(Pitch)->Add(new EditorRange(-2.0f, 2.0f, 0.1f));
  ZilchBindGetterSetterProperty(Semitones)->Add(new EditorRange(-24.0f, 24.0f, 0.1f));
  ZilchBindFieldProperty(mUseSemitoneVariation)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindFieldProperty(mPitchVariation)->Add(new EditorRange(0.0f, 1.0f, 0.1f))->
    ZeroFilterNotBool(mUseSemitoneVariation);
  ZilchBindFieldProperty(mSemitoneVariation)->Add(new EditorRange(0.0f, 12.0f, 0.1f))->
    ZeroFilterBool(mUseSemitoneVariation);
  ZilchBindGetterSetterProperty(Attenuator);
  ZilchBindFieldProperty(mShowMusicOptions)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindFieldProperty(mBeatsPerMinute)->ZeroFilterBool(mShowMusicOptions);
  ZilchBindFieldProperty(mTimeSigBeats)->ZeroFilterBool(mShowMusicOptions);
  ZilchBindFieldProperty(mTimeSigValue)->ZeroFilterBool(mShowMusicOptions);

  ZilchBindMethodProperty(Preview);
  ZilchBindMethodProperty(StopPreview);

  ZilchBindFieldProperty(Sounds);
  ZilchBindFieldProperty(SoundTags);

  ZilchBindMethod(AddSoundTagEntry);
  ZilchBindMethod(AddSoundEntry);
  ZilchBindMethod(PlayCueOnNode);

  ZeroBindEvent(Events::SoundCuePostPlay, SoundInstanceEvent);
  ZeroBindEvent(Events::SoundCuePrePlay, SoundInstanceEvent);

  type->Add(new SoundCueDisplay());
}

//**************************************************************************************************
SoundCue::SoundCue() :
  mPlayMode(SoundPlayMode::Single),
  mSelectMode(SoundSelectMode::Random),
  mVolume(1.0f),
  mVolumeVariation(0.0f),
  mPitch(0.0f),
  mPitchVariation(0.0f),
  mSemitoneVariation(0),
  mDecibelVariation(0),
  mShowMusicOptions(false),
  mBeatsPerMinute(0),
  mTimeSigBeats(0),
  mTimeSigValue(0),
  mUseSemitoneVariation(false),
  mUseDecibelVariation(false),
  mSoundIndex(0)
{
  SoundTags.PushBack();

  mAttenuator = SoundAttenuatorManager::GetDefault();
}

//**************************************************************************************************
SoundCue::~SoundCue()
{
  Unload();
}

//**************************************************************************************************
void SoundCue::Serialize(Serializer& stream)
{
  SerializeEnumNameDefault(SoundPlayMode, mPlayMode, SoundPlayMode::Single);
  SerializeNameDefault(mVolume, 1.0f);
  SerializeNameDefault(mUseDecibelVariation, false);
  SerializeNameDefault(mVolumeVariation, 0.0f);
  SerializeNameDefault(mDecibelVariation, 0.0f);
  SerializeNameDefault(mPitch, 0.0f);
  SerializeNameDefault(mUseSemitoneVariation, false);
  SerializeNameDefault(mPitchVariation, 0.0f);
  SerializeNameDefault(mSemitoneVariation, 0.0f);
  SerializeNameDefault(mShowMusicOptions, false);
  SerializeNameDefault(mBeatsPerMinute, 0.0f);
  SerializeNameDefault(mTimeSigBeats, 0.0f);
  SerializeNameDefault(mTimeSigValue, 0.0f);

  SerializeName(Sounds);
  SerializeNameDefault(SoundTags, Array<SoundTagEntry>());

  SerializeResourceName(mAttenuator, SoundAttenuatorManager);
}

//**************************************************************************************************
void SoundCue::Unload()
{
  Sounds.Clear();
  SoundTags.Clear();
  mAttenuator = nullptr;
}

//**************************************************************************************************
SoundPlayMode::Enum SoundCue::GetPlayMode()
{
  return mPlayMode;
}

//**************************************************************************************************
void SoundCue::SetPlayMode(SoundPlayMode::Enum mode)
{
  mPlayMode = mode;
}

//**************************************************************************************************
SoundSelectMode::Enum SoundCue::GetSelectMode()
{
  return mSelectMode;
}

//**************************************************************************************************
void SoundCue::SetSelectMode(SoundSelectMode::Enum selectMode)
{
  mSelectMode = selectMode;
}

//**************************************************************************************************
float SoundCue::GetVolume()
{
  return mVolume;
}

//**************************************************************************************************
void SoundCue::SetVolume(float newVolume)
{
  mVolume = newVolume;
}

//**************************************************************************************************
float SoundCue::GetDecibels()
{
  return Z::gSound->VolumeToDecibels(mVolume);
}

//**************************************************************************************************
void SoundCue::SetDecibels(float newDecibels)
{
  mVolume = Z::gSound->DecibelsToVolume(newDecibels);
}

//**************************************************************************************************
float SoundCue::GetPitch()
{
  return mPitch;
}

//**************************************************************************************************
void SoundCue::SetPitch(float newPitch)
{
  mPitch = newPitch;
}

//**************************************************************************************************
float SoundCue::GetSemitones()
{
  return Z::gSound->PitchToSemitones(mPitch);
}

//**************************************************************************************************
void SoundCue::SetSemitones(float newSemitones)
{
  mPitch = Z::gSound->SemitonesToPitch(newSemitones);
}

//**************************************************************************************************
HandleOf<SoundAttenuator> SoundCue::GetAttenuator()
{
  return mAttenuator;
}

//**************************************************************************************************
void SoundCue::SetAttenuator(SoundAttenuator* attenuation)
{
  mAttenuator = attenuation;
}

//**************************************************************************************************
void SoundCue::AddSoundEntry(Sound* sound, float weight)
{
  SoundEntry& soundEntry = Sounds.PushBack();
  soundEntry.SetSound(sound);
  soundEntry.mWeight = weight;

}

//**************************************************************************************************
void SoundCue::AddSoundTagEntry(SoundTag* tag)
{
  SoundTagEntry& tagEntry = SoundTags.PushBack();
  tagEntry.SetSoundTag(tag);
}

//**************************************************************************************************
HandleOf<SoundInstance> SoundCue::PlayCueOnNode(SoundNode* outputNode, bool startPaused)
{
  if (outputNode && outputNode->mNode)
    return PlayCue(nullptr, outputNode->mNode, startPaused);
  else
    return nullptr;
}

//**************************************************************************************************
void SoundCue::Preview()
{
  Z::gSound->StopPreview();

  // Play this SoundCue with no SoundSpace and no output node
  Z::gSound->mPreviewInstance = PlayCue(nullptr, nullptr, false);
  // Check if it successfully played
  SoundInstance* instance = Z::gSound->mPreviewInstance;
  if (instance)
  {
    // Connect it to the audio system's output 
    Z::gSound->mAudioSystem->AddNodeToOutput(instance->mSoundNode->mNode);
  }
}

//**************************************************************************************************
void SoundCue::StopPreview()
{
  Z::gSound->StopPreview();
}

//**************************************************************************************************
HandleOf<SoundInstance> SoundCue::PlayCue(SoundSpace* space, Audio::SoundNode* outputNode, bool startPaused)
{
  // No sounds to choose from
  if (Sounds.Empty())
  {
    DoNotifyWarning("Audio Warning", "Tried to play a SoundCue with no Sounds.");
    return nullptr;
  }

  // Get volume value
  float volume(mVolume); 
  if (mUseDecibelVariation && mDecibelVariation > 0)
    volume = Z::gSound->DecibelsToVolume(gRandom.FloatVariance(GetDecibels(), mDecibelVariation));
  else if (!mUseDecibelVariation && mVolumeVariation > 0)
    volume = gRandom.FloatVariance(mVolume, mVolumeVariation);

  // Get pitch value
  float pitch(mPitch);
  if (mUseSemitoneVariation && mSemitoneVariation > 0)
    pitch = Z::gSound->SemitonesToPitch(gRandom.FloatVariance(GetSemitones(), mSemitoneVariation));
  else if (!mUseSemitoneVariation && mPitchVariation > 0)
    pitch = gRandom.FloatVariance(mPitch, mPitchVariation);

  // Get the sound to play
  SoundEntry* entry;
  // If only one sound, choose that one
  if (Sounds.Size() == 1)
    entry = &Sounds.Front();
  // If selecting randomly, use the weighted table
  else if (mSelectMode == SoundSelectMode::Random)
  {
    // Created weighted table to get a random sound
    // (doing this every time because there isn't an easy way right now to update as sounds are added and removed)
    Math::WeightedProbabilityTable<SoundEntry*> weightedtable;
    for (unsigned i = 0; i < Sounds.Size(); ++i)
      weightedtable.AddItem(&Sounds[i], Sounds[i].mWeight);
    weightedtable.BuildTable();
    entry = weightedtable.Sample(gRandom);
  }
  // If sequential, advance the index and check for wrapping
  else
  {
    entry = &Sounds[mSoundIndex++];
    if (mSoundIndex == Sounds.Size())
      mSoundIndex = 0;
  }

  // Get asset
  Audio::SoundAssetNode* asset = entry->GetSound()->mSoundAsset;
  ErrorIf(!asset, "No sound asset when playing SoundCue");
  if (!asset)
  {
    DoNotifyError("Audio Error", String::Format("No audio asset for Sound %s", entry->GetSound()->Name));
    return nullptr;
  }

  // Create sound instance
  Status status;
  SoundInstance* instance(new SoundInstance(status, space, asset, volume, pitch));
  if (status.Failed())
  {
    DoNotifyError("Audio Error", status.Message);
    delete instance;
    return nullptr;
  }

  // Set the time settings on the instance
  Audio::SoundInstanceNode* instanceNode = (Audio::SoundInstanceNode*)instance->mSoundNode->mNode;
  instanceNode->SetStartTime(entry->mStartTime);
  instanceNode->SetEndTime(entry->mEndTime);
  if (mPlayMode == SoundPlayMode::Looping)
  {
    instanceNode->SetLoopStartTime(entry->mLoopStartTime);
    instanceNode->SetLoopEndTime(entry->mLoopEndTime);
    instanceNode->SetLoopTailTime(entry->mLoopTailLength);
    instanceNode->SetCrossFadeTail(entry->mCrossFadeLoopTail);
  }

  // Add any applicable tags
  for (unsigned i = 0; i < SoundTags.Size(); ++i)
    SoundTags[i].GetSoundTag()->TagSound(instance);

  // If the music options have data, set the data on the sound instance
  if (mBeatsPerMinute > 0)
    instance->SetBeatsPerMinute(mBeatsPerMinute);
  if (mTimeSigBeats > 0 && mTimeSigValue > 0)
    instance->SetTimeSignature(mTimeSigBeats, mTimeSigValue);

  // Send the pre-play event
  SoundInstanceEvent event(instance);
  DispatchEvent(Events::SoundCuePrePlay, &event);

  // Tell the sound instance to start playing
  instance->Play(mPlayMode == SoundPlayMode::Looping, nullptr, outputNode, startPaused);

  // Send the post-play event
  DispatchEvent(Events::SoundCuePostPlay, &event);

  return instance;
}

//-------------------------------------------------------------------------------- Sound Cue Manager

ImplementResourceManager(SoundCueManager, SoundCue);

//**************************************************************************************************
SoundCueManager::SoundCueManager(BoundType* resourceType)
  :ResourceManager(resourceType)
{
  AddLoader("SoundCue", new TextDataFileLoader<SoundCueManager>());
  mCategory = "Sound";
  DefaultResourceName = "DefaultSoundCue";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.SoundCue.data"));
  mCanCreateNew = true;
  mCanDuplicate = true;
  mExtension = DataResourceExtension;
}

}//namespace Zero
