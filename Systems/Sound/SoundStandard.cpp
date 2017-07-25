
///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Ranges
ZilchDefineRange(InstanceList::range);
ZilchDefineRange(NodeInfoListType::range);

// Enums
ZilchDefineEnum(FalloffCurveType);
ZilchDefineEnum(SoundPlayMode);
ZilchDefineEnum(SoundSelectMode);
ZilchDefineEnum(SynthWaveType);
ZilchDefineEnum(AudioMixTypes);
ZilchDefineEnum(AudioLatency);

// Arrays
ZeroDefineArrayType(Array<SoundEntry>);
ZeroDefineArrayType(Array<SoundTagEntry>);

//**************************************************************************************************
ZilchDefineStaticLibrary(SoundLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  ZilchInitializeRangeAs(InstanceList::range, "SoundInstanceRange");
  ZilchInitializeRangeAs(NodeInfoListType::range, "NodeInfoListRange");

  // Enums
  ZilchInitializeEnum(FalloffCurveType);
  ZilchInitializeEnum(SoundPlayMode);
  ZilchInitializeEnum(SoundSelectMode);
  ZilchInitializeEnum(SynthWaveType);
  ZilchInitializeEnum(AudioMixTypes);
  ZilchInitializeEnum(AudioLatency);
  
  // Arrays
  ZeroInitializeArrayTypeAs(Array<SoundEntry>, "Sounds");
  ZeroInitializeArrayTypeAs(Array<SoundTagEntry>, "SoundTags");

  // Events
  ZilchInitializeType(SoundInstanceEvent);
  ZilchInitializeType(SoundEvent);
  ZilchInitializeType(MidiEvent);

  ZilchInitializeType(SoundListener);
  ZilchInitializeType(SoundSystem);
  ZilchInitializeType(AudioSettings);
  ZilchInitializeType(SoundSpace);
  ZilchInitializeType(SoundAttenuatorDisplay);
  ZilchInitializeType(SoundAttenuator);
  ZilchInitializeType(SoundEmitterDisplay);
  ZilchInitializeType(SoundEmitter);
  ZilchInitializeType(SoundInstance);
  ZilchInitializeType(SoundEntryDisplay);
  ZilchInitializeType(SoundEntry);
  ZilchInitializeType(SoundTagEntryDisplay);
  ZilchInitializeType(SoundTagEntry);
  ZilchInitializeType(SoundCueDisplay);
  ZilchInitializeType(SoundCue);
  ZilchInitializeType(SoundDisplay);
  ZilchInitializeType(Sound);
  ZilchInitializeType(SimpleSound);
  ZilchInitializeType(SoundNode);
  ZilchInitializeType(SoundBuffer);
  ZilchInitializeType(CustomAudioNode);
  ZilchInitializeType(GeneratedWaveNode);
  ZilchInitializeType(VolumeNode);
  ZilchInitializeType(PitchNode);
  ZilchInitializeType(LowPassNode);
  ZilchInitializeType(HighPassNode);
  ZilchInitializeType(BandPassNode);
  ZilchInitializeType(EqualizerNode);
  ZilchInitializeType(ReverbNode);
  ZilchInitializeType(DelayNode);
  ZilchInitializeType(FlangerNode);
  ZilchInitializeType(ChorusNode);
  ZilchInitializeType(RecordingNode);
  ZilchInitializeType(CompressorNode);
  ZilchInitializeType(ExpanderNode);
  ZilchInitializeTypeAs(AudioStatics, "Audio");
  ZilchInitializeType(SoundTag);
  ZilchInitializeType(PanningNode);
  ZilchInitializeType(AddNoiseNode);
  ZilchInitializeType(AdsrEnvelope);
  ZilchInitializeType(AdditiveSynthNode);
  ZilchInitializeType(ModulationNode);
  ZilchInitializeType(SoundTagDisplay);
  ZilchInitializeType(SoundTag);
  ZilchInitializeType(NodePrintInfo);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

//**************************************************************************************************
void SoundLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

//**************************************************************************************************
void SoundLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

}//namespace Zero
