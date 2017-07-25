///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{
namespace Tags
{
DeclareTag(Sound);
}

namespace Events
{
  DeclareEvent(MIDINoteOn);
  DeclareEvent(MIDINoteOff);
  DeclareEvent(MIDIPitchWheel);
  DeclareEvent(MIDIVolume);
  DeclareEvent(MIDIModWheel);
  DeclareEvent(MIDIOtherControl);
  DeclareEvent(SoundInstancePlayed);
}

//-------------------------------------------------------------------------------------- Sound Event

/// Sent for various audio-related events
class SoundEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

};

//--------------------------------------------------------------------------------------- MIDI Event

/// Sent when a MIDI message is received from a connected device.
class MidiEvent : public Event 
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MidiEvent() : 
    Channel(0),
    MIDINumber(0), 
    Value(0) 
  {}
  MidiEvent(float channel, float number, float value) : 
    Channel(channel), 
    MIDINumber(number), 
    Value(value) 
  {}

  /// The MIDI channel received from the device.
  float Channel;
  /// The MIDI note number associated with the message. 
  float MIDINumber;
  /// A value associated with the message. Will be in the range 0 - 127.
  float Value;
};

//------------------------------------------------------------------------------------- Sound System

///SoundSystem manages audio for the engine.
class SoundSystem : public System, Audio::ExternalSystemInterface
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SoundSystem() :
    mCounter(0), 
    mPreviewInstance(0) 
  {}
  ~SoundSystem();

  //System Interface
  virtual cstr GetName()override{return "Sound";}
  void Initialize(SystemInitializer& initializer) override;

//Internals
  void Update();
  void StopPreview();
  float PitchToSemitones(float pitch);
  float SemitonesToPitch(float semitones);
  float VolumeToDecibels(float volume);
  float DecibelsToVolume(float decibels);
  void SendAudioEvent(const Audio::AudioEventType eventType, void* data) override;
  NodeInfoListType::range GetNodeInfoList();
  void SendEventOnAllSpaces(StringParam eventType, Event& eventToSend);

  unsigned mCounter;
  InList<SoundSpace> mSpaces;
  Audio::AudioSystemInterface* mAudioSystem;
  HandleOf<SoundInstance> mPreviewInstance;
  String mAudioMessage;
  SoundNodeGraph NodeGraph;
};

System* CreateSoundSystem();

// Global Access
namespace Z
{
  extern SoundSystem* gSound;
}

//----------------------------------------------------------------------------------- Sound Settings

/// The possible settings for the number of channels used by the audio system when creating audio.
/// <param name="AutoDetect">The audio system will match its channels to the default output device.</param>
/// <param name="Mono">Audio will be produced using only a single channel.</param>
/// <param name="Stereo">Audio will be produced using two channels, one for the left speaker and one for the right.</param>
/// <param name="Quad">Audio will be produced using two left channels and two right channels.</param>
/// <param name="FiveOne">Audio will be produced using a typical 5.1 speaker configuration.</param>
/// <param name="SevenOne">Audio will be produced using a typical 7.1 speaker configuration.</param>
DeclareEnum6(AudioMixTypes, AutoDetect, Mono, Stereo, Quad, FiveOne, SevenOne);
/// The latency setting used by the audio system.
/// <param name="Low">The default setting, where audio will have a low amount of latency.</param>
/// <param name="High">Audio will have a higher latency. This can fix some audio problems in some cases.</param>
DeclareEnum2(AudioLatency, Low, High);

class AudioSettings : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  AudioSettings() : 
    mSystemVolume(1.0f), 
    mMixType(AudioMixTypes::AutoDetect), 
    mLatency(AudioLatency::Low) 
  {}

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// An overall volume modifier that is applied to all audio produced by Zero.
  float GetSystemVolume();
  void SetSystemVolume(float volume);
  /// Sets the number of channels the audio system uses when creating audio. See the enum descriptions.
  /// If your selection is different from the output device, it will be automatically translated 
  /// to match the number of channels needed for output.
  AudioMixTypes::Enum GetMixType();
  void SetMixType(AudioMixTypes::Enum mixType);
  /// Sets the volume threshold at which sounds will be virtualized (they will continue
  /// tracking their position and all data but will not process audio). 
  /// This is a floating point volume number, not decibels.
  float GetMinVolumeThreshold();
  void SetMinVolumeThreshold(float volume);
  /// If you are having audio problems (lots of clicks and/or static) you can 
  /// try changing this setting to High. 
  AudioLatency::Enum GetLatencySetting();
  void SetLatencySetting(AudioLatency::Enum latency);

private:
  float mSystemVolume;
  float mMinVolumeThreshold;
  AudioMixTypes::Enum mMixType;
  AudioLatency::Enum mLatency;
};

//------------------------------------------------------------------------------------ Audio Statics

class AudioStatics
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Used by the SoundNode Graph window to display the current state of connected SoundNodes.
  static NodeInfoListType::range GetNodeGraphInfo();
  /// The volume modifier applied to all audio generated by Zero.
  static float GetSystemVolume();
  /// The volume modifier applied to all audio generated by Zero.
  static void SetSystemVolume(float volume);
  /// The current peak volume level of all audio output
  static float GetPeakOutputLevel();
  /// The current RMS volume level of all audio output
  static float GetRMSOutputLevel();
  /// This can be set to True to fix some audio problems
  static void SetUseHighLatency(bool useHigh);
  /// Creates a new VolumeNode object
  static VolumeNode* VolumeNode() { return new Zero::VolumeNode(); }
  /// Creates a new PanningNode object
  static PanningNode* PanningNode() { return new Zero::PanningNode(); }
  /// Creates a new PitchNode object
  static PitchNode* PitchNode() { return new Zero::PitchNode(); }
  /// Creates a new LowPassNode object
  static LowPassNode* LowPassNode() { return new Zero::LowPassNode(); }
  /// Creates a new HighPassNode object
  static HighPassNode* HighPassNode() { return new Zero::HighPassNode(); }
  /// Creates a new BandPassNode object
  static BandPassNode* BandPassNode() { return new Zero::BandPassNode(); }
  /// Creates a new EqualizerNode object
  static EqualizerNode* EqualizerNode() { return new Zero::EqualizerNode(); }
  /// Creates a new ReverbNode object
  static ReverbNode* ReverbNode() { return new Zero::ReverbNode(); }
  /// Creates a new DelayNode object
  static DelayNode* DelayNode() { return new Zero::DelayNode(); }
  /// Creates a new FlangerNode object
  static FlangerNode* FlangerNode() { return new Zero::FlangerNode(); }
  /// Creates a new ChorusNode object
  static ChorusNode* ChorusNode() { return new Zero::ChorusNode(); }
  /// Creates a new CompressorNode object
  static CompressorNode* CompressorNode() { return new Zero::CompressorNode(); }
  /// Creates a new ExpanderNode object
  static ExpanderNode* ExpanderNode() { return new Zero::ExpanderNode(); }
  /// Creates a new CustomAudioNode object
  static CustomAudioNode* CustomAudioNode() { return new Zero::CustomAudioNode(); }
  /// Creates a new SoundBuffer object
  static SoundBuffer* SoundBuffer() { return new Zero::SoundBuffer(); }
  /// Creates a new GeneratedWaveNode object
  static GeneratedWaveNode* GeneratedWaveNode() { return new Zero::GeneratedWaveNode(); }
  /// Creates a new RecordingNode object
  static RecordingNode* RecordingNode() { return new Zero::RecordingNode(); }
  /// Creates a new AddNoiseNode object
  static AddNoiseNode* AddNoiseNode() { return new Zero::AddNoiseNode(); }
  /// Creates a new AdditiveSynthNode object
  static AdditiveSynthNode* AdditiveSynthNode() { return new Zero::AdditiveSynthNode(); }
  /// Creates a new ModulationNode object
  static ModulationNode* ModulationNode() { return new Zero::ModulationNode(); }
  
  static void PrintAudioStartupMessage();
};

}//namespace Zero
