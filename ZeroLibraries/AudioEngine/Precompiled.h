///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef Precompiled_H
#define Precompiled_H

#include "Common/CommonStandard.hpp"
#include "Math/MathStandard.hpp"
#include "Platform/PlatformStandard.hpp"

#include "portaudio.h"
#include "pa_ringbuffer.h"

#include "Definitions.h"
#include "SoundNode.h"
#include "FilterNodes.h"
#include "ListenerNode.h"
#include "RecordNode.h"
#include "VolumeNodes.h"
#include "PitchNode.h"
#include "EqualizerNode.h"
#include "ReverbNodes.h"
#include "AdditiveSynthNode.h"
#include "DynamicsProcessorNode.h"
#include "Emitter.h"
#include "Attenuator.h"
#include "InputNode.h"
#include "SoundAssets.h"
#include "SoundInstances.h"
#include "Tags.h"

#include "LockFreeQueue.hpp"
#include "Interpolator.h"
#include "Filters.h"
#include "AudioInputOutput.h"
#include "DeviceNotifier.h"

#include "MIDI.h"
#include "Modifiers.h"
#include "FileAccess.h"
#include "FileDecoder.h"
#include "AudioSystemInterface.h"
#include "AudioSystemInternal.h"

#endif 