///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
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

//-------------------------------------------------------------------------------------- Sound Event

ZilchDefineType(SoundEvent, builder, type)
{
  ZeroBindDocumented();
}

//--------------------------------------------------------------------------------------- MIDI Event

ZilchDefineType(MidiEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindField(Channel);
  ZilchBindField(MIDINumber);
  ZilchBindField(Value);
}

//--------------------------------------------------------------------------- Audio Float Data Event

ZilchDefineType(AudioFloatDataEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindField(Channels);
  ZilchBindMember(AudioData);
}

//---------------------------------------------------------------------------- Audio Byte Data Event

ZilchDefineType(AudioByteDataEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindMember(AudioData);
}

//------------------------------------------------------------------------------------- Sound System

//**************************************************************************************************
ZilchDefineType(SoundSystem, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);

  ZilchBindGetterSetter(SystemVolume);
  ZilchBindGetter(PeakOutputLevel);
  ZilchBindGetter(RMSOutputLevel);
  ZilchBindGetter(PeakInputLevel);
  ZilchBindMethod(GetNodeGraphInfo);
  ZilchBindGetterSetter(LatencySetting);
  ZilchBindGetterSetter(DispatchMicrophoneUncompressedFloatData);
  ZilchBindGetterSetter(DispatchMicrophoneCompressedByteData);
  ZilchBindGetter(OutputChannels);
  ZilchBindGetterSetter(MuteAllAudio);

  ZilchBindMethod(VolumeNode);
  ZilchBindMethod(PanningNode);
  ZilchBindMethod(PitchNode);
  ZilchBindMethod(LowPassNode);
  ZilchBindMethod(HighPassNode);
  ZilchBindMethod(BandPassNode);
  ZilchBindMethod(EqualizerNode);
  ZilchBindMethod(ReverbNode);
  ZilchBindMethod(DelayNode);
  ZilchBindMethod(CustomAudioNode);
  ZilchBindMethod(SoundBuffer);
  ZilchBindMethod(FlangerNode);
  ZilchBindMethod(ChorusNode);
  ZilchBindMethod(CompressorNode);
  ZilchBindMethod(ExpanderNode);
  ZilchBindMethod(GeneratedWaveNode);
  ZilchBindMethod(RecordingNode);
  ZilchBindMethod(AddNoiseNode);
  ZilchBindMethod(AdditiveSynthNode);
  ZilchBindMethod(GranularSynthNode);
  ZilchBindMethod(ModulationNode);
  ZilchBindMethod(MicrophoneInputNode);
  ZilchBindMethod(SaveAudioNode);

  ZeroBindEvent(Events::MIDINoteOn, MidiEvent);
  ZeroBindEvent(Events::MIDINoteOff, MidiEvent);
  ZeroBindEvent(Events::MIDIPitchWheel, MidiEvent);
  ZeroBindEvent(Events::MIDIVolume, MidiEvent);
  ZeroBindEvent(Events::MIDIModWheel, MidiEvent);
  ZeroBindEvent(Events::MIDIOtherControl, MidiEvent);
  ZeroBindEvent(Events::SoundInstancePlayed, SoundInstanceEvent);
  ZeroBindEvent(Events::MicrophoneUncompressedFloatData, AudioFloatDataEvent);
  ZeroBindEvent(Events::MicrophoneCompressedByteData, AudioByteDataEvent);
}

//**************************************************************************************************
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

//**************************************************************************************************
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

//**************************************************************************************************
void SoundSystem::Initialize(SystemInitializer& initializer)
{
  Z::gSound = this;

  //Create a System object and initialize.
  Zero::Status status;
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

//**************************************************************************************************
NodeInfoListType::range SoundSystem::GetNodeGraphInfo()
{
  return NodeGraph.GetNodeInfoList();
}

//**************************************************************************************************
float SoundSystem::GetSystemVolume()
{
  return Mixer.GetVolume();
}

//**************************************************************************************************
void SoundSystem::SetSystemVolume(float volume)
{
  Mixer.SetVolume(Math::Max(volume, 0.0f));
}

//**************************************************************************************************
bool SoundSystem::GetMuteAllAudio()
{
  return Mixer.GetMuteAllAudio();
}

//**************************************************************************************************
void SoundSystem::SetMuteAllAudio(bool muteAudio)
{
  Mixer.SetMuteAllAudio(muteAudio);
}

//**************************************************************************************************
float SoundSystem::GetPeakOutputLevel()
{
  return Mixer.GetPeakOutputVolume();
}

//**************************************************************************************************
float SoundSystem::GetRMSOutputLevel()
{
  return Mixer.GetRMSOutputVolume();
}

//**************************************************************************************************
float SoundSystem::GetPeakInputLevel()
{
  return Mixer.GetPeakInputVolume();
}

//**************************************************************************************************
AudioLatency::Enum SoundSystem::GetLatencySetting()
{
  return mLatency;
}

//**************************************************************************************************
void SoundSystem::SetLatencySetting(AudioLatency::Enum latency)
{
  mLatency = latency;

  Mixer.SetLatency(latency);
}

//**************************************************************************************************
bool SoundSystem::GetDispatchMicrophoneUncompressedFloatData()
{
  return mSendMicEvents;
}

//**************************************************************************************************
void SoundSystem::SetDispatchMicrophoneUncompressedFloatData(bool dispatchData)
{
  mSendMicEvents = dispatchData;
  Mixer.SetSendUncompressedMicInput(dispatchData);
}

//**************************************************************************************************
bool SoundSystem::GetDispatchMicrophoneCompressedByteData()
{
  return mSendCompressedMicEvents;
}

//**************************************************************************************************
void SoundSystem::SetDispatchMicrophoneCompressedByteData(bool dispatchData)
{
  mSendCompressedMicEvents = dispatchData;
  Mixer.SetSendCompressedMicInput(dispatchData);
}

//**************************************************************************************************
int SoundSystem::GetOutputChannels()
{
  return Mixer.GetOutputChannels();
}

//**************************************************************************************************
VolumeNode* SoundSystem::VolumeNode()
{
  Zero::VolumeNode* node = new Zero::VolumeNode("VolumeNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
PanningNode* SoundSystem::PanningNode()
{
  Zero::PanningNode* node = new Zero::PanningNode("PanningNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
PitchNode* SoundSystem::PitchNode()
{
  Zero::PitchNode* node = new Zero::PitchNode("PitchNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
LowPassNode* SoundSystem::LowPassNode()
{
  Zero::LowPassNode* node = new Zero::LowPassNode("LowPassNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
HighPassNode* SoundSystem::HighPassNode()
{
  Zero::HighPassNode* node = new Zero::HighPassNode("HighPassNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
BandPassNode* SoundSystem::BandPassNode()
{
  Zero::BandPassNode* node = new Zero::BandPassNode("BandPassNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
EqualizerNode* SoundSystem::EqualizerNode()
{
  Zero::EqualizerNode* node = new Zero::EqualizerNode("EqualizerNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
ReverbNode* SoundSystem::ReverbNode()
{
  Zero::ReverbNode* node = new Zero::ReverbNode("ReverbNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
DelayNode* SoundSystem::DelayNode()
{
  Zero::DelayNode* node = new Zero::DelayNode("DelayNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
FlangerNode* SoundSystem::FlangerNode()
{
  Zero::FlangerNode* node = new Zero::FlangerNode("FlangerNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
ChorusNode* SoundSystem::ChorusNode()
{
  Zero::ChorusNode* node = new Zero::ChorusNode("ChorusNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
CompressorNode* SoundSystem::CompressorNode()
{
  Zero::CompressorNode* node = new Zero::CompressorNode("CompressorNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
ExpanderNode* SoundSystem::ExpanderNode()
{
  Zero::ExpanderNode* node = new Zero::ExpanderNode("ExpanderNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
CustomAudioNode* SoundSystem::CustomAudioNode()
{
  Zero::CustomAudioNode* node = new Zero::CustomAudioNode("CustomAudioNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
SoundBuffer* SoundSystem::SoundBuffer()
{
  Zero::SoundBuffer* buffer = new Zero::SoundBuffer();
  return buffer;
}

//**************************************************************************************************
GeneratedWaveNode* SoundSystem::GeneratedWaveNode()
{
  Zero::GeneratedWaveNode* node = new Zero::GeneratedWaveNode("GeneratedWaveNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
RecordingNode* SoundSystem::RecordingNode()
{
  Zero::RecordingNode* node = new Zero::RecordingNode("RecordingNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
AddNoiseNode* SoundSystem::AddNoiseNode()
{
  Zero::AddNoiseNode* node = new Zero::AddNoiseNode("AddNoiseNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
AdditiveSynthNode* SoundSystem::AdditiveSynthNode()
{
  Zero::AdditiveSynthNode* node = new Zero::AdditiveSynthNode("AdditiveSynthNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
ModulationNode* SoundSystem::ModulationNode()
{
  Zero::ModulationNode* node = new Zero::ModulationNode("ModulationNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
MicrophoneInputNode* SoundSystem::MicrophoneInputNode()
{
  Zero::MicrophoneInputNode* node = new Zero::MicrophoneInputNode("MicrophoneInputNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
SaveAudioNode* SoundSystem::SaveAudioNode()
{
  Zero::SaveAudioNode* node = new Zero::SaveAudioNode("SaveAudioNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
GranularSynthNode* SoundSystem::GranularSynthNode()
{
  Zero::GranularSynthNode* node = new Zero::GranularSynthNode("GranularSynthNode", Z::gSound->mCounter++);
  return node;
}

//**************************************************************************************************
void SoundSystem::Update()
{
  // Update spaces (also updates emitters)
  forRange(SoundSpace& space, mSpaces.All())
    space.Update();

  // Update audio system 
  Mixer.Update();
}

//**************************************************************************************************
void SoundSystem::StopPreview()
{
  SoundInstance *sound = mPreviewInstance;
  if (sound)
  {
    sound->Stop();
    mPreviewInstance = nullptr;
  }
}

//**************************************************************************************************
void SoundSystem::AddSoundSpace(SoundSpace* space, bool isEditor)
{
  mSpaces.PushBack(space);

  // If not an editor space, increase the counter and notify tags if necessary
  if (!isEditor)
  {
    ++mSoundSpaceCounter;

    if (mSoundSpaceCounter == 1)
    {
      forRange(SoundTag& tag, mSoundTags.All())
        tag.CreateTag();
    }
  }
}

//**************************************************************************************************
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
      forRange(SoundTag& tag, mSoundTags.All())
        tag.ReleaseTag();
    }
  }
}

//----------------------------------------------------------------------------------- Audio Settings

//**************************************************************************************************
ZilchDefineType(AudioSettings, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindTag(Tags::Sound);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(SystemVolume)->Add(new EditorSlider(0.0f, 2.0f, 0.01f));
  ZilchBindGetterSetterProperty(MuteAllAudio);
  ZilchBindGetterSetterProperty(UseRandomSeed)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindGetterSetterProperty(Seed)->ZeroFilterEquality(mUseRandomSeed, bool, false);
  ZilchBindGetterSetterProperty(MixType); 
  ZilchBindGetterSetterProperty(MinVolumeThreshold)->Add(new EditorSlider(0.0f, 0.2f, 0.001f));
  ZilchBindGetterSetterProperty(LatencySetting);
}

//**************************************************************************************************
AudioSettings::AudioSettings() :
  mSystemVolume(1.0f),
  mMinVolumeThreshold(0.015f),
  mMixType(AudioMixTypes::AutoDetect),
  mLatency(AudioLatency::Low),
  mUseRandomSeed(true),
  mSeed(0)
{

}

//**************************************************************************************************
void AudioSettings::Serialize(Serializer& stream)
{
  SerializeNameDefault(mSystemVolume, 1.0f);
  SerializeEnumNameDefault(AudioMixTypes, mMixType, AudioMixTypes::AutoDetect);
  SerializeNameDefault(mMinVolumeThreshold, 0.015f);
  SerializeEnumNameDefault(AudioLatency, mLatency, AudioLatency::Low);
  SerializeNameDefault(mUseRandomSeed, true);
  SerializeNameDefault(mSeed, 0u);
}

//**************************************************************************************************
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

//**************************************************************************************************
float AudioSettings::GetSystemVolume()
{
  mSystemVolume = Z::gSound->Mixer.GetVolume();
  return mSystemVolume;
}

//**************************************************************************************************
void AudioSettings::SetSystemVolume(float volume)
{
  mSystemVolume = Math::Clamp(volume, 0.0f, AudioConstants::cMaxVolumeValue);

  Z::gSound->Mixer.SetVolume(mSystemVolume);
}

//**************************************************************************************************
bool AudioSettings::GetMuteAllAudio()
{
  return Z::gSound->GetMuteAllAudio();
}

//**************************************************************************************************
void AudioSettings::SetMuteAllAudio(bool muteAudio)
{
  Z::gSound->SetMuteAllAudio(muteAudio);
}

//**************************************************************************************************
AudioMixTypes::Enum AudioSettings::GetMixType()
{
  return mMixType;
}

//**************************************************************************************************
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

//**************************************************************************************************
float AudioSettings::GetMinVolumeThreshold()
{
  return mMinVolumeThreshold;
}

//**************************************************************************************************
void AudioSettings::SetMinVolumeThreshold(float volume)
{
  mMinVolumeThreshold = Math::Clamp(volume, 0.0f, 0.5f);
  Z::gSound->Mixer.SetMinimumVolumeThreshold(mMinVolumeThreshold);
}

//**************************************************************************************************
Zero::AudioLatency::Enum AudioSettings::GetLatencySetting()
{
  return mLatency;
}

//**************************************************************************************************
void AudioSettings::SetLatencySetting(AudioLatency::Enum latency)
{
  mLatency = latency;
  Z::gSound->SetLatencySetting(latency);
}

//**************************************************************************************************
bool AudioSettings::GetUseRandomSeed()
{
  return mUseRandomSeed;
}

//**************************************************************************************************
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

//**************************************************************************************************
uint AudioSettings::GetSeed()
{
  return mSeed;
}

//**************************************************************************************************
void AudioSettings::SetSeed(uint seed)
{
  if (!mUseRandomSeed)
    Z::gSound->mRandom.SetSeed(seed);

  mSeed = seed;
  Z::gSound->mSeed = seed;
}

}//namespace Zero
