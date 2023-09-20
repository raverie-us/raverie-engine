// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{

namespace Tags
{

DefineTag(Sound);

} // namespace Tags

namespace Events
{

DefineEvent(MIDINoteOn);
DefineEvent(MIDINoteOff);
DefineEvent(MIDIPitchWheel);
DefineEvent(MIDIVolume);
DefineEvent(MIDIModWheel);
DefineEvent(MIDIOtherControl);
DefineEvent(SoundInstancePlayed);
DefineEvent(MicrophoneUncompressedFloatData);
DefineEvent(MicrophoneCompressedByteData);

} // namespace Events

namespace Z
{

SoundSystem* gSound;

} // namespace Z

System* CreateSoundSystem()
{
  return new SoundSystem();
}

// Sound Event

RaverieDefineType(SoundEvent, builder, type)
{
  RaverieBindDocumented();
}

// MIDI Event

RaverieDefineType(MidiEvent, builder, type)
{
  RaverieBindDocumented();

  RaverieBindField(Channel);
  RaverieBindField(MIDINumber);
  RaverieBindField(Value);
}

// Audio Float Data Event

RaverieDefineType(AudioFloatDataEvent, builder, type)
{
  RaverieBindDocumented();

  RaverieBindField(Channels);
  RaverieBindMember(AudioData);
}

// Audio Byte Data Event

RaverieDefineType(AudioByteDataEvent, builder, type)
{
  RaverieBindDocumented();

  RaverieBindMember(AudioData);
}

// Sound System

RaverieDefineType(SoundSystem, builder, type)
{
  type->HandleManager = RaverieManagerId(PointerManager);

  RaverieBindGetterSetter(SystemVolume);
  RaverieBindGetter(PeakOutputLevel);
  RaverieBindGetter(RMSOutputLevel);
  RaverieBindGetter(PeakInputLevel);
  RaverieBindMethod(GetNodeGraphInfo);
  RaverieBindGetterSetter(LatencySetting);
  RaverieBindGetterSetter(DispatchMicrophoneUncompressedFloatData);
  RaverieBindGetterSetter(DispatchMicrophoneCompressedByteData);
  RaverieBindGetter(OutputChannels);
  RaverieBindGetterSetter(MuteAllAudio);

  RaverieBindMethod(VolumeNode);
  RaverieBindMethod(PanningNode);
  RaverieBindMethod(PitchNode);
  RaverieBindMethod(LowPassNode);
  RaverieBindMethod(HighPassNode);
  RaverieBindMethod(BandPassNode);
  RaverieBindMethod(EqualizerNode);
  RaverieBindMethod(ReverbNode);
  RaverieBindMethod(DelayNode);
  RaverieBindMethod(CustomAudioNode);
  RaverieBindMethod(SoundBuffer);
  RaverieBindMethod(FlangerNode);
  RaverieBindMethod(ChorusNode);
  RaverieBindMethod(CompressorNode);
  RaverieBindMethod(ExpanderNode);
  RaverieBindMethod(GeneratedWaveNode);
  RaverieBindMethod(RecordingNode);
  RaverieBindMethod(AddNoiseNode);
  RaverieBindMethod(AdditiveSynthNode);
  RaverieBindMethod(GranularSynthNode);
  RaverieBindMethod(ModulationNode);
  RaverieBindMethod(MicrophoneInputNode);
  RaverieBindMethod(SaveAudioNode);

  RaverieBindEvent(Events::MIDINoteOn, MidiEvent);
  RaverieBindEvent(Events::MIDINoteOff, MidiEvent);
  RaverieBindEvent(Events::MIDIPitchWheel, MidiEvent);
  RaverieBindEvent(Events::MIDIVolume, MidiEvent);
  RaverieBindEvent(Events::MIDIModWheel, MidiEvent);
  RaverieBindEvent(Events::MIDIOtherControl, MidiEvent);
  RaverieBindEvent(Events::SoundInstancePlayed, SoundInstanceEvent);
  RaverieBindEvent(Events::MicrophoneUncompressedFloatData, AudioFloatDataEvent);
  RaverieBindEvent(Events::MicrophoneCompressedByteData, AudioByteDataEvent);
}

SoundSystem::SoundSystem() :
    mCounter(0),
    mPreviewInstance(0),
    mLatency(AudioLatency::Low),
    mSendMicEvents(false),
    mSendCompressedMicEvents(false),
    mSoundSpaceCounter(0),
    mUseRandomSeed(true),
    mSeed(0)
{
}

SoundSystem::~SoundSystem()
{
  // If currently previewing a sound, stop
  SoundInstance* previewInstance = mPreviewInstance;
  if (previewInstance)
  {
    previewInstance->Stop();
  }

  Mixer.ShutDown();
}

void SoundSystem::Initialize(SystemInitializer& initializer)
{
  Z::gSound = this;

  // Create a System object and initialize.
  Raverie::Status status;
  Mixer.StartMixing(status);
  if (status.Failed())
    DoNotifyWarning("Audio Initialization Unsuccessful", status.Message);

  mOutputNode = new CombineNode("AudioOutput", mCounter++);
  Mixer.FinalOutputNode->AddInputNode(mOutputNode);

  InitializeResourceManager(SoundManager);
  InitializeResourceManager(SoundCueManager);
  InitializeResourceManager(SoundTagManager);
  InitializeResourceManager(SoundAttenuatorManager);

  if (!mUseRandomSeed)
    mRandom.SetSeed(mSeed);
}

NodeInfoListType::range SoundSystem::GetNodeGraphInfo()
{
  return NodeGraph.GetNodeInfoList();
}

float SoundSystem::GetSystemVolume()
{
  return Mixer.GetVolume();
}

void SoundSystem::SetSystemVolume(float volume)
{
  Mixer.SetVolume(Math::Max(volume, 0.0f));
}

bool SoundSystem::GetMuteAllAudio()
{
  return Mixer.GetMuteAllAudio();
}

void SoundSystem::SetMuteAllAudio(bool muteAudio)
{
  Mixer.SetMuteAllAudio(muteAudio);
}

float SoundSystem::GetPeakOutputLevel()
{
  return Mixer.GetPeakOutputVolume();
}

float SoundSystem::GetRMSOutputLevel()
{
  return Mixer.GetRMSOutputVolume();
}

float SoundSystem::GetPeakInputLevel()
{
  return Mixer.GetPeakInputVolume();
}

AudioLatency::Enum SoundSystem::GetLatencySetting()
{
  return mLatency;
}

void SoundSystem::SetLatencySetting(AudioLatency::Enum latency)
{
  mLatency = latency;

  Mixer.SetLatency(latency);
}

bool SoundSystem::GetDispatchMicrophoneUncompressedFloatData()
{
  return mSendMicEvents;
}

void SoundSystem::SetDispatchMicrophoneUncompressedFloatData(bool dispatchData)
{
  mSendMicEvents = dispatchData;
  Mixer.SetSendUncompressedMicInput(dispatchData);
}

bool SoundSystem::GetDispatchMicrophoneCompressedByteData()
{
  return mSendCompressedMicEvents;
}

void SoundSystem::SetDispatchMicrophoneCompressedByteData(bool dispatchData)
{
  mSendCompressedMicEvents = dispatchData;
  Mixer.SetSendCompressedMicInput(dispatchData);
}

int SoundSystem::GetOutputChannels()
{
  return Mixer.GetOutputChannels();
}

VolumeNode* SoundSystem::VolumeNode()
{
  Raverie::VolumeNode* node = new Raverie::VolumeNode("VolumeNode", Z::gSound->mCounter++);
  return node;
}

PanningNode* SoundSystem::PanningNode()
{
  Raverie::PanningNode* node = new Raverie::PanningNode("PanningNode", Z::gSound->mCounter++);
  return node;
}

PitchNode* SoundSystem::PitchNode()
{
  Raverie::PitchNode* node = new Raverie::PitchNode("PitchNode", Z::gSound->mCounter++);
  return node;
}

LowPassNode* SoundSystem::LowPassNode()
{
  Raverie::LowPassNode* node = new Raverie::LowPassNode("LowPassNode", Z::gSound->mCounter++);
  return node;
}

HighPassNode* SoundSystem::HighPassNode()
{
  Raverie::HighPassNode* node = new Raverie::HighPassNode("HighPassNode", Z::gSound->mCounter++);
  return node;
}

BandPassNode* SoundSystem::BandPassNode()
{
  Raverie::BandPassNode* node = new Raverie::BandPassNode("BandPassNode", Z::gSound->mCounter++);
  return node;
}

EqualizerNode* SoundSystem::EqualizerNode()
{
  Raverie::EqualizerNode* node = new Raverie::EqualizerNode("EqualizerNode", Z::gSound->mCounter++);
  return node;
}

ReverbNode* SoundSystem::ReverbNode()
{
  Raverie::ReverbNode* node = new Raverie::ReverbNode("ReverbNode", Z::gSound->mCounter++);
  return node;
}

DelayNode* SoundSystem::DelayNode()
{
  Raverie::DelayNode* node = new Raverie::DelayNode("DelayNode", Z::gSound->mCounter++);
  return node;
}

FlangerNode* SoundSystem::FlangerNode()
{
  Raverie::FlangerNode* node = new Raverie::FlangerNode("FlangerNode", Z::gSound->mCounter++);
  return node;
}

ChorusNode* SoundSystem::ChorusNode()
{
  Raverie::ChorusNode* node = new Raverie::ChorusNode("ChorusNode", Z::gSound->mCounter++);
  return node;
}

CompressorNode* SoundSystem::CompressorNode()
{
  Raverie::CompressorNode* node = new Raverie::CompressorNode("CompressorNode", Z::gSound->mCounter++);
  return node;
}

ExpanderNode* SoundSystem::ExpanderNode()
{
  Raverie::ExpanderNode* node = new Raverie::ExpanderNode("ExpanderNode", Z::gSound->mCounter++);
  return node;
}

CustomAudioNode* SoundSystem::CustomAudioNode()
{
  Raverie::CustomAudioNode* node = new Raverie::CustomAudioNode("CustomAudioNode", Z::gSound->mCounter++);
  return node;
}

SoundBuffer* SoundSystem::SoundBuffer()
{
  Raverie::SoundBuffer* buffer = new Raverie::SoundBuffer();
  return buffer;
}

GeneratedWaveNode* SoundSystem::GeneratedWaveNode()
{
  Raverie::GeneratedWaveNode* node = new Raverie::GeneratedWaveNode("GeneratedWaveNode", Z::gSound->mCounter++);
  return node;
}

RecordingNode* SoundSystem::RecordingNode()
{
  Raverie::RecordingNode* node = new Raverie::RecordingNode("RecordingNode", Z::gSound->mCounter++);
  return node;
}

AddNoiseNode* SoundSystem::AddNoiseNode()
{
  Raverie::AddNoiseNode* node = new Raverie::AddNoiseNode("AddNoiseNode", Z::gSound->mCounter++);
  return node;
}

AdditiveSynthNode* SoundSystem::AdditiveSynthNode()
{
  Raverie::AdditiveSynthNode* node = new Raverie::AdditiveSynthNode("AdditiveSynthNode", Z::gSound->mCounter++);
  return node;
}

ModulationNode* SoundSystem::ModulationNode()
{
  Raverie::ModulationNode* node = new Raverie::ModulationNode("ModulationNode", Z::gSound->mCounter++);
  return node;
}

MicrophoneInputNode* SoundSystem::MicrophoneInputNode()
{
  Raverie::MicrophoneInputNode* node = new Raverie::MicrophoneInputNode("MicrophoneInputNode", Z::gSound->mCounter++);
  return node;
}

SaveAudioNode* SoundSystem::SaveAudioNode()
{
  Raverie::SaveAudioNode* node = new Raverie::SaveAudioNode("SaveAudioNode", Z::gSound->mCounter++);
  return node;
}

GranularSynthNode* SoundSystem::GranularSynthNode()
{
  Raverie::GranularSynthNode* node = new Raverie::GranularSynthNode("GranularSynthNode", Z::gSound->mCounter++);
  return node;
}

void SoundSystem::Update(bool debugger)
{
  if (debugger)
    return;

  // Update spaces (also updates emitters)
  forRange (SoundSpace& space, mSpaces.All())
    space.Update();

  // Update audio system
  Mixer.Update();
}

void SoundSystem::StopPreview()
{
  SoundInstance* sound = mPreviewInstance;
  if (sound)
  {
    sound->Stop();
    mPreviewInstance = nullptr;
  }
}

void SoundSystem::AddSoundSpace(SoundSpace* space, bool isEditor)
{
  mSpaces.PushBack(space);

  // If not an editor space, increase the counter and notify tags if necessary
  if (!isEditor)
  {
    ++mSoundSpaceCounter;

    if (mSoundSpaceCounter == 1)
    {
      forRange (SoundTag& tag, mSoundTags.All())
        tag.CreateTag();
    }
  }
}

void SoundSystem::RemoveSoundSpace(SoundSpace* space, bool isEditor)
{
  mSpaces.Erase(space);

  // If not an editor space, decrease the counter and notify tags if necessary
  if (!isEditor)
  {
    --mSoundSpaceCounter;

    ErrorIf(mSoundSpaceCounter < 0, "SoundSystem's space tracking has become negative");

    if (mSoundSpaceCounter == 0)
    {
      forRange (SoundTag& tag, mSoundTags.All())
        tag.ReleaseTag();
    }
  }
}

// Audio Settings

RaverieDefineType(AudioSettings, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindTag(Tags::Sound);
  RaverieBindDocumented();

  RaverieBindGetterSetterProperty(SystemVolume)->Add(new EditorSlider(0.0f, 2.0f, 0.01f));
  RaverieBindGetterSetterProperty(MuteAllAudio);
  RaverieBindGetterSetterProperty(UseRandomSeed)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  RaverieBindGetterSetterProperty(Seed)->RaverieFilterEquality(mUseRandomSeed, bool, false);
  RaverieBindGetterSetterProperty(MixType);
  RaverieBindGetterSetterProperty(MinVolumeThreshold)->Add(new EditorSlider(0.0f, 0.2f, 0.001f));
  RaverieBindGetterSetterProperty(LatencySetting);
}

AudioSettings::AudioSettings() :
    mSystemVolume(1.0f),
    mMinVolumeThreshold(0.015f),
    mMixType(AudioMixTypes::AutoDetect),
    mLatency(AudioLatency::Low),
    mUseRandomSeed(true),
    mSeed(0)
{
}

void AudioSettings::Serialize(Serializer& stream)
{
  SerializeNameDefault(mSystemVolume, 1.0f);
  SerializeEnumNameDefault(AudioMixTypes, mMixType, AudioMixTypes::AutoDetect);
  SerializeNameDefault(mMinVolumeThreshold, 0.015f);
  SerializeEnumNameDefault(AudioLatency, mLatency, AudioLatency::Low);
  SerializeNameDefault(mUseRandomSeed, true);
  SerializeNameDefault(mSeed, 0u);
}

void AudioSettings::Initialize(CogInitializer& initializer)
{
  Z::gSound->Mixer.SetVolume(mSystemVolume);
  SetMixType(mMixType);
  Z::gSound->Mixer.SetMinimumVolumeThreshold(mMinVolumeThreshold);
  Z::gSound->SetLatencySetting(mLatency);
  Z::gSound->mUseRandomSeed = mUseRandomSeed;
  Z::gSound->mSeed = mSeed;
  if (mUseRandomSeed)
    Z::gSound->mRandom.SetSeed(mSeed);
}

float AudioSettings::GetSystemVolume()
{
  mSystemVolume = Z::gSound->Mixer.GetVolume();
  return mSystemVolume;
}

void AudioSettings::SetSystemVolume(float volume)
{
  mSystemVolume = Math::Clamp(volume, 0.0f, AudioConstants::cMaxVolumeValue);

  Z::gSound->Mixer.SetVolume(mSystemVolume);
}

bool AudioSettings::GetMuteAllAudio()
{
  return Z::gSound->GetMuteAllAudio();
}

void AudioSettings::SetMuteAllAudio(bool muteAudio)
{
  Z::gSound->SetMuteAllAudio(muteAudio);
}

AudioMixTypes::Enum AudioSettings::GetMixType()
{
  return mMixType;
}

void AudioSettings::SetMixType(AudioMixTypes::Enum mixType)
{
  mMixType = mixType;

  switch (mixType)
  {
  case AudioMixTypes::AutoDetect:
    Z::gSound->Mixer.SetOutputChannels(0);
    break;
  case AudioMixTypes::Mono:
    Z::gSound->Mixer.SetOutputChannels(1);
    break;
  case AudioMixTypes::Stereo:
    Z::gSound->Mixer.SetOutputChannels(2);
    break;
  case AudioMixTypes::Quad:
    Z::gSound->Mixer.SetOutputChannels(4);
    break;
  case AudioMixTypes::FiveOne:
    Z::gSound->Mixer.SetOutputChannels(6);
    break;
  case AudioMixTypes::SevenOne:
    Z::gSound->Mixer.SetOutputChannels(8);
    break;
  default:
    Z::gSound->Mixer.SetOutputChannels(2);
    break;
  }
}

float AudioSettings::GetMinVolumeThreshold()
{
  return mMinVolumeThreshold;
}

void AudioSettings::SetMinVolumeThreshold(float volume)
{
  mMinVolumeThreshold = Math::Clamp(volume, 0.0f, 0.5f);
  Z::gSound->Mixer.SetMinimumVolumeThreshold(mMinVolumeThreshold);
}

Raverie::AudioLatency::Enum AudioSettings::GetLatencySetting()
{
  return mLatency;
}

void AudioSettings::SetLatencySetting(AudioLatency::Enum latency)
{
  mLatency = latency;
  Z::gSound->SetLatencySetting(latency);
}

bool AudioSettings::GetUseRandomSeed()
{
  return mUseRandomSeed;
}

void AudioSettings::SetUseRandomSeed(bool useRandom)
{
  if (useRandom && !mUseRandomSeed)
  {
    Z::gSound->mRandom.SetSeed(Z::gSound->mRandom.mGlobalSeed);
  }
  else if (!useRandom && mUseRandomSeed)
  {
    Z::gSound->mRandom.SetSeed(mSeed);
  }

  mUseRandomSeed = useRandom;
  Z::gSound->mUseRandomSeed = useRandom;
}

uint AudioSettings::GetSeed()
{
  return mSeed;
}

void AudioSettings::SetSeed(uint seed)
{
  if (!mUseRandomSeed)
    Z::gSound->mRandom.SetSeed(seed);

  mSeed = seed;
  Z::gSound->mSeed = seed;
}

} // namespace Raverie
