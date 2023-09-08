// MIT Licensed (see LICENSE.md).
#pragma once
#include "Foundation/Common/CommonStandard.hpp"
#include "Foundation/Serialization/SerializationStandard.hpp"
#include "Foundation/Meta/MetaStandard.hpp"
#include "Foundation/Support/SupportStandard.hpp"
#include "Systems/Engine/EngineStandard.hpp"
#include "Foundation/Geometry/DebugDraw.hpp"

namespace Zero
{
// Forward declarations
class SoundSystem;
class SoundSpace;
class SoundNode;
class SoundInstance;
class SoundCue;
class SoundEntry;
class SoundTag;
class SoundTagEntry;
class Sound;

// Sound library
class ZeroNoImportExport SoundLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(SoundLibrary, "ZeroEngine");

  static void Initialize();
  static void Shutdown();
};

} // namespace Zero

#include "Definitions.hpp"
#include "RingBuffer.hpp"
#include "LockFreeQueue.hpp"
#include "Interpolator.hpp"
#include "AudioIOInterface.hpp"
#include "Filters.hpp"
#include "Resampler.hpp"
#include "VBAP.hpp"
#include "PitchChange.hpp"
#include "FileDecoder.hpp"
#include "VolumeModifier.hpp"
#include "SoundAsset.hpp"
#include "SoundNode.hpp"
#include "SoundTag.hpp"
#include "AudioMixer.hpp"
#include "AttenuatorNode.hpp"
#include "EmitterNode.hpp"
#include "ListenerNode.hpp"
#include "CustomAudioNode.hpp"
#include "GeneratedAudio.hpp"
#include "Recording.hpp"
#include "SoundListener.hpp"
#include "DspFilterNodes.hpp"
#include "SoundAttenuator.hpp"
#include "SoundEmitter.hpp"
#include "SoundSpace.hpp"
#include "SoundNodeGraph.hpp"
#include "SoundInstance.hpp"
#include "SoundSystem.hpp"
#include "Sound.hpp"
#include "SoundCue.hpp"
#include "SimpleSound.hpp"
#include "SimpleSound.hpp"
