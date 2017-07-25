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
}

namespace Events
{
  DefineEvent(MIDINoteOn);
  DefineEvent(MIDINoteOff);
  DefineEvent(MIDIPitchWheel);
  DefineEvent(MIDIVolume);
  DefineEvent(MIDIModWheel);
  DefineEvent(MIDIOtherControl);
  DefineEvent(SoundInstancePlayed);
}

namespace Z
{
  SoundSystem* gSound;
}

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

//------------------------------------------------------------------------------------- Sound System

//**************************************************************************************************
ZilchDefineType(SoundSystem, builder, type)
{
  ZeroBindEvent(Events::MIDINoteOn, MidiEvent);
  ZeroBindEvent(Events::MIDINoteOff, MidiEvent);
  ZeroBindEvent(Events::MIDIPitchWheel, MidiEvent);
  ZeroBindEvent(Events::MIDIVolume, MidiEvent);
  ZeroBindEvent(Events::MIDIModWheel, MidiEvent);
  ZeroBindEvent(Events::MIDIOtherControl, MidiEvent);

  ZeroBindEvent(Events::SoundInstancePlayed, SoundInstanceEvent);
}

//**************************************************************************************************
SoundSystem::~SoundSystem()
{
  // If currently previewing a sound, stop
  SoundInstance* previewInstance = mPreviewInstance;
  if (previewInstance)
  {
    previewInstance->Stop();
    previewInstance->mSoundNode->mNode = nullptr;
    mPreviewInstance = nullptr;
  }

  Zero::Status status;
  mAudioSystem->StopSystem(status);
  ErrorIf(status.Failed(), status.Message.c_str());
  SafeDelete(mAudioSystem);
}

//**************************************************************************************************
void SoundSystem::Initialize(SystemInitializer& initializer)
{
  Z::gSound = this;

  //Create a System object and initialize.
  Zero::Status status;
  mAudioSystem = new Audio::AudioSystemInterface(this);
  mAudioSystem->StartSystem(status);
  if (status.Failed())
    DoNotifyWarning("Audio Error", status.Message);

  mAudioMessage = status.Message;
  
  InitializeResourceManager(SoundManager);
  SoundManager::GetInstance()->SetSystem(mAudioSystem);
  InitializeResourceManager(SoundCueManager);
  InitializeResourceManager(SoundTagManager);
  InitializeResourceManager(SoundAttenuatorManager);
}

//**************************************************************************************************
void SoundSystem::Update()
{
  // Update audio system 
  mAudioSystem->Update();

  // Update spaces (also updates emitters)
  forRange(SoundSpace& space, mSpaces.All())
    space.Update();
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
float SoundSystem::PitchToSemitones(float pitch)
{
  if (pitch == 0)
    return 0;
  else
    return 3986.0f * Math::Log10(Math::Exp2(pitch)) / 100.0f;
}

//**************************************************************************************************
float SoundSystem::SemitonesToPitch(float semitone)
{
  return Math::Log2(Math::Exp2(semitone / 12.0f));
}

//**************************************************************************************************
float SoundSystem::VolumeToDecibels(float volume)
{
  if (volume == 0.0f)
    return -100.0f;
  else
  {
    float decibels = 20.0f * Math::Log10(volume);
    if (decibels < -100.0f)
      decibels = -100.0f;
    return decibels;
  }
}

//**************************************************************************************************
float SoundSystem::DecibelsToVolume(float decibels)
{
  return Math::Pow(10.0f, decibels / 20.0f);
}

//**************************************************************************************************
void SoundSystem::SendAudioEvent(const Audio::AudioEventType eventType, void * data)
{
  if (eventType == Audio::Notify_AudioClipping)
    DoNotifyWarning("Audio Error", "Audio is too loud and is being clipped. Reduce volume or number of sounds to avoid audio problems.");
  else if (eventType == Audio::Notify_Error)
  {
    DoNotifyWarning("Audio Error", ((String*)data)->c_str());
    delete (String*)data;
  }
  else if (eventType == Audio::Notify_MidiNoteOn)
  {
    Audio::MidiData* midiData = (Audio::MidiData*)data;
    MidiEvent event((float)midiData->Channel, midiData->Value1, midiData->Value2);
    SendEventOnAllSpaces(Events::MIDINoteOn, event);
    delete (Audio::MidiData*)data;
  }
  else if (eventType == Audio::Notify_MidiNoteOff)
  {
    Audio::MidiData* midiData = (Audio::MidiData*)data;
    MidiEvent event((float)midiData->Channel, midiData->Value1, 0);
    SendEventOnAllSpaces(Events::MIDINoteOff, event);
    delete (Audio::MidiData*)data;
  }
  else if (eventType == Audio::Notify_MidiPitchWheel)
  {
    Audio::MidiData* midiData = (Audio::MidiData*)data;
    MidiEvent event((float)midiData->Channel, 0, midiData->Value1);
    SendEventOnAllSpaces(Events::MIDIPitchWheel, event);
    delete (Audio::MidiData*)data;
  }
  else if (eventType == Audio::Notify_MidiVolume)
  {
    Audio::MidiData* midiData = (Audio::MidiData*)data;
    MidiEvent event((float)midiData->Channel, 0, midiData->Value1);
    SendEventOnAllSpaces(Events::MIDIVolume, event);
    delete (Audio::MidiData*)data;
  }
  else if (eventType == Audio::Notify_MidiModWheel)
  {
    Audio::MidiData* midiData = (Audio::MidiData*)data;
    MidiEvent event((float)midiData->Channel, 0, midiData->Value1);
    SendEventOnAllSpaces(Events::MIDIModWheel, event);
    delete (Audio::MidiData*)data;
  }
  else if (eventType == Audio::Notify_MidiControl)
  {
    Audio::MidiData* midiData = (Audio::MidiData*)data;
    MidiEvent event((float)midiData->Channel, midiData->Value1, midiData->Value2);
    SendEventOnAllSpaces(Events::MIDIOtherControl, event);
    delete (Audio::MidiData*)data;
  }
}

//**************************************************************************************************
NodeInfoListType::range SoundSystem::GetNodeInfoList()
{
  return NodeGraph.GetNodeInfoList();
}

//**************************************************************************************************
void SoundSystem::SendEventOnAllSpaces(StringParam eventType, Event& eventToSend)
{
  forRange(SoundSpace& space, mSpaces.All())
  {
    space.DispatchEvent(eventType, &eventToSend);
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

  ZilchBindGetterSetterProperty(SystemVolume)->Add(new EditorRange(0.0f, 2.0f, 0.01f));
  ZilchBindGetterSetterProperty(MixType); 
  ZilchBindGetterSetterProperty(MinVolumeThreshold)->Add(new EditorRange(0.0f, 0.2f, 0.001f));
  ZilchBindGetterSetterProperty(LatencySetting);
}

//**************************************************************************************************
void AudioSettings::Serialize(Serializer& stream)
{
  SerializeNameDefault(mSystemVolume, 1.0f);
  SerializeEnumNameDefault(AudioMixTypes, mMixType, AudioMixTypes::AutoDetect);
  SerializeNameDefault(mMinVolumeThreshold, 0.015f);
  SerializeEnumNameDefault(AudioLatency, mLatency, AudioLatency::Low);
}

//**************************************************************************************************
void AudioSettings::Initialize(CogInitializer& initializer)
{
  Z::gSound->mAudioSystem->SetVolume(mSystemVolume);
  SetMixType(mMixType);
  Z::gSound->mAudioSystem->SetMinimumVolumeThreshold(mMinVolumeThreshold);
  if (mLatency == AudioLatency::High)
    Z::gSound->mAudioSystem->UseHighLatency(true);
}

//**************************************************************************************************
float AudioSettings::GetSystemVolume()
{
  return mSystemVolume;
}

//**************************************************************************************************
void AudioSettings::SetSystemVolume(float volume)
{
  mSystemVolume = volume;

  Z::gSound->mAudioSystem->SetVolume(volume);
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
    Z::gSound->mAudioSystem->SetOutputChannels(0);
    break;
  case AudioMixTypes::Mono:
    Z::gSound->mAudioSystem->SetOutputChannels(1);
    break;
  case AudioMixTypes::Stereo:
    Z::gSound->mAudioSystem->SetOutputChannels(2);
    break;
  case AudioMixTypes::Quad:
    Z::gSound->mAudioSystem->SetOutputChannels(4);
    break;
  case AudioMixTypes::FiveOne:
    Z::gSound->mAudioSystem->SetOutputChannels(6);
    break;
  case AudioMixTypes::SevenOne:
    Z::gSound->mAudioSystem->SetOutputChannels(8);
    break;
  default:
    Z::gSound->mAudioSystem->SetOutputChannels(2);
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
  mMinVolumeThreshold = volume;
  Z::gSound->mAudioSystem->SetMinimumVolumeThreshold(volume);
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

  if (latency == AudioLatency::High)
    Z::gSound->mAudioSystem->UseHighLatency(true);
  else
    Z::gSound->mAudioSystem->UseHighLatency(false);
}

//------------------------------------------------------------------------------------ Audio Statics

//**************************************************************************************************
ZilchDefineType(AudioStatics, builder, type)
{
  ZeroBindDocumented();

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
  ZilchBindMethod(ModulationNode);

  ZilchBindGetterSetter(SystemVolume);
  ZilchBindGetter(PeakOutputLevel);
  ZilchBindGetter(RMSOutputLevel);

  ZilchBindMethod(GetNodeGraphInfo);
  ZilchBindMethod(SetUseHighLatency);
}

//**************************************************************************************************
NodeInfoListType::range AudioStatics::GetNodeGraphInfo()
{
  return Z::gSound->GetNodeInfoList();
}

//**************************************************************************************************
void AudioStatics::PrintAudioStartupMessage()
{
  StringBuilder message;
  message << "Audio Startup Data: " << Z::gSound->mAudioMessage << "\n";
  ZPrint(message.ToString().c_str());
}

//**************************************************************************************************
float AudioStatics::GetSystemVolume()
{
  return Z::gSound->mAudioSystem->GetVolume();
}

//**************************************************************************************************
void AudioStatics::SetSystemVolume(float volume)
{
  Z::gSound->mAudioSystem->SetVolume(volume);
}

//**************************************************************************************************
float AudioStatics::GetPeakOutputLevel()
{
  return Z::gSound->mAudioSystem->GetPeakOutputVolume();
}

//**************************************************************************************************
float AudioStatics::GetRMSOutputLevel()
{
  return Z::gSound->mAudioSystem->GetRMSOutputVolume();
}

//**************************************************************************************************
void AudioStatics::SetUseHighLatency(bool useHigh)
{
  Z::gSound->mAudioSystem->UseHighLatency(useHigh);
}

}//namespace Zero
