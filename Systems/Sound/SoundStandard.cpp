
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
ZilchDefineRange(InstanceListType::range);
ZilchDefineRange(NodeInfoListType::range);

// Enums
ZilchDefineEnum(FalloffCurveType);
ZilchDefineEnum(SoundPlayMode);
ZilchDefineEnum(SoundSelectMode);
ZilchDefineEnum(SynthWaveType);
ZilchDefineEnum(AudioMixTypes);
ZilchDefineEnum(AudioLatency);
ZilchDefineEnum(GranularSynthWindows);

// Arrays
ZeroDefineArrayType(Array<SoundEntry>);
ZeroDefineArrayType(Array<SoundTagEntry>);

//**************************************************************************************************
ZilchDefineStaticLibrary(SoundLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  ZilchInitializeRangeAs(InstanceListType::range, "SoundInstanceRange");
  ZilchInitializeRangeAs(NodeInfoListType::range, "NodeInfoListRange");

  // Enums
  ZilchInitializeEnum(FalloffCurveType);
  ZilchInitializeEnum(SoundPlayMode);
  ZilchInitializeEnum(SoundSelectMode);
  ZilchInitializeEnum(SynthWaveType);
  ZilchInitializeEnum(AudioMixTypes);
  ZilchInitializeEnum(AudioLatency);
  ZilchInitializeEnum(GranularSynthWindows);
  
  // Arrays
  ZeroInitializeArrayTypeAs(Array<SoundEntry>, "Sounds");
  ZeroInitializeArrayTypeAs(Array<SoundTagEntry>, "SoundTags");

  // Events
  ZilchInitializeType(SoundInstanceEvent);
  ZilchInitializeType(SoundEvent);
  ZilchInitializeType(MidiEvent);
  ZilchInitializeType(AudioFloatDataEvent);
  ZilchInitializeType(CustomAudioNodeEvent);
  ZilchInitializeType(AudioByteDataEvent);

  ZilchInitializeTypeAs(SoundSystem, "Audio");
  ZilchInitializeType(SoundNode);
  ZilchInitializeType(SimpleCollapseNode);
  ZilchInitializeType(SoundAsset);
  ZilchInitializeType(SoundListener);
  ZilchInitializeType(ListenerNode);
  ZilchInitializeType(AudioSettings);
  ZilchInitializeType(SoundSpace);
  ZilchInitializeType(SoundAttenuatorDisplay);
  ZilchInitializeType(SoundAttenuator);
  ZilchInitializeType(AttenuatorNode);
  ZilchInitializeType(SoundEmitterDisplay);
  ZilchInitializeType(SoundEmitter);
  ZilchInitializeType(EmitterNode);
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
  ZilchInitializeType(SoundTag);
  ZilchInitializeType(PanningNode);
  ZilchInitializeType(AddNoiseNode);
  ZilchInitializeType(AdsrEnvelope);
  ZilchInitializeType(AdditiveSynthNode);
  ZilchInitializeType(GranularSynthNode);
  ZilchInitializeType(ModulationNode);
  ZilchInitializeType(MicrophoneInputNode);
  ZilchInitializeType(SaveAudioNode);
  ZilchInitializeType(SoundTagDisplay);
  ZilchInitializeType(SoundTag);
  ZilchInitializeType(TagObject);
  ZilchInitializeType(NodePrintInfo);
  ZilchInitializeType(SimpleCollapseNode);
  ZilchInitializeType(OutputNode);
  ZilchInitializeType(CombineNode);
  ZilchInitializeType(CombineAndPauseNode);
  ZilchInitializeType(SoundAsset);
  ZilchInitializeType(DecompressedSoundAsset);
  ZilchInitializeType(StreamingSoundAsset);

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
