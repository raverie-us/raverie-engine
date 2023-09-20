// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{

using namespace AudioConstants;

// Sound Entry

RaverieDefineType(SoundEntryDisplay, builder, type)
{
}

String SoundEntryDisplay::GetName(HandleParam object)
{
  SoundEntry* soundEntry = object.Get<SoundEntry*>(GetOptions::AssertOnNull);
  return BuildString("Sound: ", soundEntry->GetSound()->Name);
}

String SoundEntryDisplay::GetDebugText(HandleParam object)
{
  return GetName(object);
}

String SoundEntryDisplayText(const BoundType* type, const byte* data)
{
  SoundEntry* soundEntry = (SoundEntry*)data;
  return BuildString("Sound: ", soundEntry->GetSound()->Name);
}

RaverieDefineType(SoundEntry, builder, type)
{
  RaverieBindDocumented();
  type->ToStringFunction = &SoundEntryDisplayText;
  type->Add(new SoundEntryDisplay());
  type->HandleManager = RaverieManagerId(PointerManager);

  RaverieBindGetterSetterProperty(Sound);
  RaverieBindGetterSetterProperty(Weight);
  RaverieBindGetterSetterProperty(StartTime);
  RaverieBindGetterSetterProperty(EndTime);
  RaverieBindGetterSetterProperty(LoopStartTime);
  RaverieBindGetterSetterProperty(LoopEndTime);
  RaverieBindGetterSetterProperty(LoopTailLength);
  RaverieBindFieldProperty(mCrossFadeLoopTail);
  RaverieBindMethodProperty(Preview);
  RaverieBindMethodProperty(StopPreview);
}

SoundEntry::SoundEntry() : mLoopEndTime(0), mLoopStartTime(0), mWeight(1.0f), mStartTime(0.0f), mEndTime(0.0f), mLoopTailLength(0.0f), mSound(nullptr), mCrossFadeLoopTail(false)
{
  mSound = SoundManager::GetDefault();
  if (mSound)
  {
    mEndTime = mSound->GetLength();
    mLoopEndTime = mEndTime;
  }
}

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

Sound* SoundEntry::GetSound()
{
  return mSound;
}

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

float SoundEntry::GetWeight()
{
  return mWeight;
}

void SoundEntry::SetWeight(float weight)
{
  mWeight = Math::Clamp(weight, 0.0f, 100.0f);
}

void SoundEntry::Preview()
{
  Z::gSound->StopPreview();

  // Create a new SoundInstance with no SoundSpace
  Status status;
  SoundInstance* instance = new SoundInstance(status, nullptr, mSound->mAsset, 1.0f, 0.0f);

  if (!status.Failed())
  {
    // If instance was created successfully, play it without looping,
    // without a tag, and without an output node
    instance->Play(false, Z::gSound->mOutputNode, false);
    // Save the SoundInstance
    Z::gSound->mPreviewInstance = instance;
  }
  else
  {
    DoNotifyError("Error Previewing Sound", status.Message);

    SafeDelete(instance);
  }
}

void SoundEntry::StopPreview()
{
  Z::gSound->StopPreview();
}

float SoundEntry::GetStartTime()
{
  return mStartTime;
}

void SoundEntry::SetStartTime(float time)
{
  mStartTime = Math::Clamp(time, 0.0f, mEndTime);
}

float SoundEntry::GetEndTime()
{
  return mEndTime;
}

void SoundEntry::SetEndTime(float time)
{
  time = Math::Max(time, mStartTime);

  if (mSound && (time == 0.0f || time > mSound->GetLength()))
    mEndTime = mSound->GetLength();
  else
    mEndTime = time;
}

float SoundEntry::GetLoopStartTime()
{
  return mLoopStartTime;
}

void SoundEntry::SetLoopStartTime(float time)
{
  mLoopStartTime = Math::Clamp(time, 0.0f, mEndTime);
}

float SoundEntry::GetLoopEndTime()
{
  return mLoopEndTime;
}

void SoundEntry::SetLoopEndTime(float time)
{
  time = Math::Max(time, mLoopStartTime);

  if (mSound && (time == 0.0f || time > mSound->GetLength()))
    mLoopEndTime = mSound->GetLength();
  else
    mLoopEndTime = time;
}

float SoundEntry::GetLoopTailLength()
{
  return mLoopTailLength;
}

void SoundEntry::SetLoopTailLength(float time)
{
  mLoopTailLength = Math::Clamp(time, 0.0f, 30.0f);
}

// Sound Tag Entry

RaverieDefineType(SoundTagEntryDisplay, builder, type)
{
}

String SoundTagEntryDisplay::GetName(HandleParam object)
{
  SoundTagEntry* soundTagEntry = object.Get<SoundTagEntry*>(GetOptions::AssertOnNull);
  return BuildString("Tag: ", soundTagEntry->GetSoundTag()->Name);
}

String SoundTagEntryDisplay::GetDebugText(HandleParam object)
{
  SoundTagEntry* soundTagEntry = object.Get<SoundTagEntry*>(GetOptions::AssertOnNull);
  return BuildString("Tag: ", soundTagEntry->GetSoundTag()->Name);
}

String SoundTagEntryDisplayText(const BoundType* type, const byte* data)
{
  SoundTagEntry* soundEntry = (SoundTagEntry*)data;
  return BuildString("Tag: ", soundEntry->GetSoundTag()->Name);
}

RaverieDefineType(SoundTagEntry, builder, type)
{
  RaverieBindDocumented();
  type->HandleManager = RaverieManagerId(PointerManager);
  type->ToStringFunction = &SoundTagEntryDisplayText;
  type->Add(new SoundTagEntryDisplay());

  RaverieBindGetterSetterProperty(SoundTag);
}

SoundTagEntry::SoundTagEntry() : mSoundTag(SoundTagManager::GetDefault())
{
}

void SoundTagEntry::Serialize(Serializer& stream)
{
  SerializeResourceName(mSoundTag, SoundTagManager);
}

void SoundTagEntry::SetSoundTag(SoundTag* tag)
{
  if (tag)
    mSoundTag = tag;
}

SoundTag* SoundTagEntry::GetSoundTag()
{
  return mSoundTag;
}

// Sound Cue

RaverieDefineType(SoundCueDisplay, builder, type)
{
}

String SoundCueDisplay::GetName(HandleParam object)
{
  return "SoundCue";
}

String SoundCueDisplay::GetDebugText(HandleParam object)
{
  return "SoundCue";
}

RaverieDefineType(SoundCue, builder, type)
{
  RaverieBindDocumented();

  RaverieBindGetterSetterProperty(PlayMode);
  RaverieBindGetterSetterProperty(SelectMode);
  RaverieBindGetterSetterProperty(Volume)->Add(new EditorSlider(0.0f, 2.0f, 0.01f));
  RaverieBindGetterSetterProperty(Decibels)->Add(new EditorSlider(-32.0f, 6.0f, 0.1f));
  RaverieBindFieldProperty(mUseDecibelVariation)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  RaverieBindGetterSetterProperty(VolumeVariation)->Add(new EditorSlider(0.0f, 1.0f, 0.01f))->RaverieFilterNotBool(mUseDecibelVariation);
  RaverieBindGetterSetterProperty(DecibelVariation)->Add(new EditorSlider(0.0f, 6.0f, 0.1f))->RaverieFilterBool(mUseDecibelVariation);
  RaverieBindGetterSetterProperty(Pitch)->Add(new EditorSlider(-2.0f, 2.0f, 0.1f));
  RaverieBindGetterSetterProperty(Semitones)->Add(new EditorSlider(-24.0f, 24.0f, 0.1f));
  RaverieBindFieldProperty(mUseSemitoneVariation)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  RaverieBindGetterSetterProperty(PitchVariation)->Add(new EditorSlider(0.0f, 1.0f, 0.1f))->RaverieFilterNotBool(mUseSemitoneVariation);
  RaverieBindGetterSetterProperty(SemitoneVariation)->Add(new EditorSlider(0.0f, 12.0f, 0.1f))->RaverieFilterBool(mUseSemitoneVariation);
  RaverieBindGetterSetterProperty(Attenuator);
  RaverieBindFieldProperty(mShowMusicOptions)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  RaverieBindGetterSetterProperty(BeatsPerMinute)->RaverieFilterBool(mShowMusicOptions);
  RaverieBindGetterSetterProperty(TimeSigBeats)->RaverieFilterBool(mShowMusicOptions);
  RaverieBindGetterSetterProperty(TimeSigValue)->RaverieFilterBool(mShowMusicOptions);

  RaverieBindMethodProperty(Preview);
  RaverieBindMethodProperty(StopPreview);

  RaverieBindFieldProperty(Sounds);
  RaverieBindFieldProperty(SoundTags);

  RaverieBindMethod(AddSoundTagEntry);
  RaverieBindMethod(AddSoundEntry);
  RaverieBindMethod(PlayCueOnNode);

  RaverieBindEvent(Events::SoundCuePostPlay, SoundInstanceEvent);
  RaverieBindEvent(Events::SoundCuePrePlay, SoundInstanceEvent);

  type->Add(new SoundCueDisplay());
}

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

SoundCue::~SoundCue()
{
}

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

void SoundCue::Unload()
{
  Sounds.Clear();
  SoundTags.Clear();
  mAttenuator = nullptr;
}

SoundPlayMode::Enum SoundCue::GetPlayMode()
{
  return mPlayMode;
}

void SoundCue::SetPlayMode(SoundPlayMode::Enum mode)
{
  mPlayMode = mode;
}

SoundSelectMode::Enum SoundCue::GetSelectMode()
{
  return mSelectMode;
}

void SoundCue::SetSelectMode(SoundSelectMode::Enum selectMode)
{
  mSelectMode = selectMode;
}

float SoundCue::GetVolume()
{
  return mVolume;
}

void SoundCue::SetVolume(float newVolume)
{
  mVolume = Math::Clamp(newVolume, 0.0f, cMaxVolumeValue);
}

float SoundCue::GetDecibels()
{
  return VolumeToDecibels(mVolume);
}

void SoundCue::SetDecibels(float newDecibels)
{
  newDecibels = Math::Clamp(newDecibels, cMinDecibelsValue, cMaxDecibelsValue);
}

float SoundCue::GetVolumeVariation()
{
  return mVolumeVariation;
}

void SoundCue::SetVolumeVariation(float variation)
{
  mVolumeVariation = Math::Clamp(variation, 0.0f, cMaxVolumeValue);
}

float SoundCue::GetDecibelVariation()
{
  return mDecibelVariation;
}

void SoundCue::SetDecibelVariation(float variation)
{
  mDecibelVariation = Math::Clamp(variation, cMinDecibelsValue, cMaxDecibelsValue);
}

float SoundCue::GetPitch()
{
  return mPitch;
}

void SoundCue::SetPitch(float newPitch)
{
  mPitch = Math::Clamp(newPitch, cMinPitchValue, cMaxPitchValue);
}

float SoundCue::GetSemitones()
{
  return PitchToSemitones(mPitch);
}

void SoundCue::SetSemitones(float newSemitones)
{
  mPitch = Math::Clamp(SemitonesToPitch(newSemitones), cMinPitchValue, cMaxPitchValue);
}

float SoundCue::GetPitchVariation()
{
  return mPitchVariation;
}

void SoundCue::SetPitchVariation(float variation)
{
  mPitchVariation = Math::Clamp(variation, 0.0f, cMaxPitchValue);
}

float SoundCue::GetSemitoneVariation()
{
  return mSemitoneVariation;
}

void SoundCue::SetSemitoneVariation(float variation)
{
  mSemitoneVariation = Math::Clamp(variation, 0.0f, cMaxSemitonesValue);
}

float SoundCue::GetBeatsPerMinute()
{
  return mBeatsPerMinute;
}

void SoundCue::SetBeatsPerMinute(float bpm)
{
  mBeatsPerMinute = Math::Clamp(bpm, 0.0f, 500.0f);
}

float SoundCue::GetTimeSigBeats()
{
  return mTimeSigBeats;
}

void SoundCue::SetTimeSigBeats(float beats)
{
  mTimeSigBeats = Math::Clamp(beats, 0.0f, 100.0f);
}

float SoundCue::GetTimeSigValue()
{
  return mTimeSigValue;
}

void SoundCue::SetTimeSigValue(float value)
{
  mTimeSigValue = Math::Clamp(value, 0.0f, 64.0f);
}

SoundAttenuator* SoundCue::GetAttenuator()
{
  return mAttenuator;
}

void SoundCue::SetAttenuator(SoundAttenuator* attenuation)
{
  mAttenuator = attenuation;
}

void SoundCue::AddSoundEntry(Sound* sound, float weight)
{
  SoundEntry& soundEntry = Sounds.PushBack();
  soundEntry.SetSound(sound);
  soundEntry.SetWeight(weight);
}

void SoundCue::AddSoundTagEntry(SoundTag* tag)
{
  SoundTagEntry& tagEntry = SoundTags.PushBack();
  tagEntry.SetSoundTag(tag);
}

HandleOf<SoundInstance> SoundCue::PlayCueOnNode(HandleOf<SoundNode> outputNode, bool startPaused)
{
  if (outputNode)
    return PlayCue(nullptr, outputNode, startPaused);
  else
    return nullptr;
}

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
    Z::gSound->mOutputNode->AddInputNode(instance);
  }
}

void SoundCue::StopPreview()
{
  Z::gSound->StopPreview();
}

HandleOf<SoundInstance> SoundCue::PlayCue(SoundSpace* space, HandleOf<SoundNode> outputNode, bool startPaused)
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
    volume = DecibelsToVolume(Z::gSound->mRandom.FloatVariance(GetDecibels(), mDecibelVariation));
  else if (!mUseDecibelVariation && mVolumeVariation > 0)
    volume = Z::gSound->mRandom.FloatVariance(mVolume, mVolumeVariation);

  // Get pitch value
  float pitch(mPitch);
  if (mUseSemitoneVariation && mSemitoneVariation > 0)
    pitch = SemitonesToPitch(Z::gSound->mRandom.FloatVariance(GetSemitones(), mSemitoneVariation));
  else if (!mUseSemitoneVariation && mPitchVariation > 0)
    pitch = Z::gSound->mRandom.FloatVariance(mPitch, mPitchVariation);

  // Get the sound to play
  SoundEntry* entry;
  // If only one sound, choose that one
  if (Sounds.Size() == 1)
    entry = &Sounds.Front();
  // If selecting randomly, use the weighted table
  else if (mSelectMode == SoundSelectMode::Random)
  {
    // Created weighted table to get a random sound
    // (doing this every time because there isn't an easy way right now to
    // update as sounds are added and removed)
    Math::WeightedProbabilityTable<SoundEntry*> weightedtable;
    for (unsigned i = 0; i < Sounds.Size(); ++i)
      weightedtable.AddItem(&Sounds[i], Sounds[i].GetWeight());
    weightedtable.BuildTable();
    entry = weightedtable.Sample(Z::gSound->mRandom);
  }
  // If sequential, advance the index and check for wrapping
  else
  {
    entry = &Sounds[mSoundIndex++];
    if (mSoundIndex == Sounds.Size())
      mSoundIndex = 0;
  }

  // Get asset
  SoundAsset* asset = entry->GetSound()->mAsset;
  ErrorIf(!asset, "No sound asset when playing SoundCue");
  if (!asset)
  {
    DoNotifyError("Audio Error", String::Format("No audio asset for Sound %s", entry->GetSound()->Name.c_str()));
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
  if (entry->GetStartTime() > 0.0f)
    instance->SetTime(entry->GetStartTime());
  instance->SetEndTime(entry->GetEndTime());
  if (mPlayMode == SoundPlayMode::Looping)
  {
    instance->SetLoopStartTime(entry->GetLoopStartTime());
    instance->SetLoopEndTime(entry->GetLoopEndTime());
    instance->SetLoopTailTime(entry->GetLoopTailLength());
    instance->SetCrossFadeLoopTail(entry->mCrossFadeLoopTail);
  }

  // Create the handle to avoid deleting the instance object
  HandleOf<SoundInstance> instanceHandle = instance;

  // Add any applicable tags
  for (unsigned i = 0; i < SoundTags.Size(); ++i)
    SoundTags[i].GetSoundTag()->TagSound(instanceHandle);

  // If the music options have data, set the data on the sound instance
  if (mBeatsPerMinute > 0)
    instance->SetBeatsPerMinute(mBeatsPerMinute);
  if (mTimeSigBeats > 0 && mTimeSigValue > 0)
    instance->SetTimeSignature(mTimeSigBeats, mTimeSigValue);

  // Send the pre-play event
  SoundInstanceEvent event(instance);
  DispatchEvent(Events::SoundCuePrePlay, &event);

  // Tell the sound instance to start playing
  instance->Play(mPlayMode == SoundPlayMode::Looping, outputNode, startPaused);

  // Send the post-play event
  DispatchEvent(Events::SoundCuePostPlay, &event);

  return instanceHandle;
}

// Sound Cue Manager

ImplementResourceManager(SoundCueManager, SoundCue);

SoundCueManager::SoundCueManager(BoundType* resourceType) : ResourceManager(resourceType)
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

} // namespace Raverie
