
///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Common/CommonStandard.hpp"
#include "Platform/PlatformStandard.hpp"
#include "Serialization/SerializationStandard.hpp"
#include "Meta/MetaStandard.hpp"
#include "Math/MathStandard.hpp"
#include "Support/SupportStandard.hpp"
#include "Engine/EngineStandard.hpp"
#include "Geometry/DebugDraw.hpp"

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

}//namespace Zero

#include "AudioEngine/AudioSystemInterface.h"

#include "SoundListener.hpp"
#include "SoundFilterNode.hpp"
#include "SoundAttenuator.hpp"
#include "SoundEmitter.hpp"
#include "SoundSpace.hpp"
#include "SoundNodeGraph.hpp"
#include "SoundInstance.hpp"
#include "SoundSystem.hpp"
#include "Sound.hpp"
#include "SoundCue.hpp"
#include "SimpleSound.hpp"
#include "SoundTag.hpp"
#include "SimpleSound.hpp"

