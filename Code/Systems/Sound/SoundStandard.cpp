// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Ranges
RaverieDefineRange(InstanceListType::range);
RaverieDefineRange(NodeInfoListType::range);

// Enums
RaverieDefineEnum(FalloffCurveType);
RaverieDefineEnum(SoundPlayMode);
RaverieDefineEnum(SoundSelectMode);
RaverieDefineEnum(SynthWaveType);
RaverieDefineEnum(AudioMixTypes);
RaverieDefineEnum(AudioLatency);
RaverieDefineEnum(GranularSynthWindows);

// Arrays
RaverieDefineArrayType(Array<SoundEntry>);
RaverieDefineArrayType(Array<SoundTagEntry>);

RaverieDefineStaticLibrary(SoundLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  RaverieInitializeRangeAs(InstanceListType::range, "SoundInstanceRange");
  RaverieInitializeRangeAs(NodeInfoListType::range, "NodeInfoListRange");

  // Enums
  RaverieInitializeEnum(FalloffCurveType);
  RaverieInitializeEnum(SoundPlayMode);
  RaverieInitializeEnum(SoundSelectMode);
  RaverieInitializeEnum(SynthWaveType);
  RaverieInitializeEnum(AudioMixTypes);
  RaverieInitializeEnum(AudioLatency);
  RaverieInitializeEnum(GranularSynthWindows);

  // Arrays
  RaverieInitializeArrayTypeAs(Array<SoundEntry>, "Sounds");
  RaverieInitializeArrayTypeAs(Array<SoundTagEntry>, "SoundTags");

  // Events
  RaverieInitializeType(SoundInstanceEvent);
  RaverieInitializeType(SoundEvent);
  RaverieInitializeType(MidiEvent);
  RaverieInitializeType(AudioFloatDataEvent);
  RaverieInitializeType(CustomAudioNodeEvent);
  RaverieInitializeType(AudioByteDataEvent);

  RaverieInitializeTypeAs(SoundSystem, "Audio");
  RaverieInitializeType(SoundNode);
  RaverieInitializeType(SimpleCollapseNode);
  RaverieInitializeType(SoundAsset);
  RaverieInitializeType(SoundListener);
  RaverieInitializeType(ListenerNode);
  RaverieInitializeType(AudioSettings);
  RaverieInitializeType(SoundSpace);
  RaverieInitializeType(SoundAttenuatorDisplay);
  RaverieInitializeType(SoundAttenuator);
  RaverieInitializeType(AttenuatorNode);
  RaverieInitializeType(SoundEmitterDisplay);
  RaverieInitializeType(SoundEmitter);
  RaverieInitializeType(EmitterNode);
  RaverieInitializeType(SoundInstance);
  RaverieInitializeType(SoundEntryDisplay);
  RaverieInitializeType(SoundEntry);
  RaverieInitializeType(SoundTagEntryDisplay);
  RaverieInitializeType(SoundTagEntry);
  RaverieInitializeType(SoundCueDisplay);
  RaverieInitializeType(SoundCue);
  RaverieInitializeType(SoundDisplay);
  RaverieInitializeType(Sound);
  RaverieInitializeType(SimpleSound);
  RaverieInitializeType(SoundBuffer);
  RaverieInitializeType(CustomAudioNode);
  RaverieInitializeType(GeneratedWaveNode);
  RaverieInitializeType(VolumeNode);
  RaverieInitializeType(PitchNode);
  RaverieInitializeType(LowPassNode);
  RaverieInitializeType(HighPassNode);
  RaverieInitializeType(BandPassNode);
  RaverieInitializeType(EqualizerNode);
  RaverieInitializeType(ReverbNode);
  RaverieInitializeType(DelayNode);
  RaverieInitializeType(FlangerNode);
  RaverieInitializeType(ChorusNode);
  RaverieInitializeType(RecordingNode);
  RaverieInitializeType(CompressorNode);
  RaverieInitializeType(ExpanderNode);
  RaverieInitializeType(SoundTag);
  RaverieInitializeType(PanningNode);
  RaverieInitializeType(AddNoiseNode);
  RaverieInitializeType(AdsrEnvelope);
  RaverieInitializeType(AdditiveSynthNode);
  RaverieInitializeType(GranularSynthNode);
  RaverieInitializeType(ModulationNode);
  RaverieInitializeType(MicrophoneInputNode);
  RaverieInitializeType(SaveAudioNode);
  RaverieInitializeType(SoundTagDisplay);
  RaverieInitializeType(SoundTag);
  RaverieInitializeType(TagObject);
  RaverieInitializeType(NodePrintInfo);
  RaverieInitializeType(SimpleCollapseNode);
  RaverieInitializeType(OutputNode);
  RaverieInitializeType(CombineNode);
  RaverieInitializeType(CombineAndPauseNode);
  RaverieInitializeType(SoundAsset);
  RaverieInitializeType(DecompressedSoundAsset);
  RaverieInitializeType(StreamingSoundAsset);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

void SoundLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

void SoundLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Raverie
