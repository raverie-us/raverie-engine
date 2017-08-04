///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef AudioSystemInterface_H
#define AudioSystemInterface_H

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

namespace Audio
{

  //------------------------------------------------------------------------ External System Interface

  class ExternalSystemInterface
  {
  public:
    virtual void SendAudioEvent(const AudioEventType eventType, void* data) = 0;
  };

  //------------------------------------------------------------------------ External Node Interface

  class ExternalNodeInterface
  {
  public:
    virtual void SendAudioEvent(const AudioEventType eventType, void* data) = 0;
  };

  //------------------------------------------------------------------------- Audio System Interface

  // Provides interface functions for accessing API, interacts with internal system
  class AudioSystemInterface
  {
  public:
    // Creates the AudioSystemInternal object
    AudioSystemInterface(ExternalSystemInterface* extInterface);

    // Deletes the AudioSystemInternal object
    ~AudioSystemInterface();

    // Calls StartSystem on the internal system. Sets the status failed info if system does not start correctly.
    void StartSystem(Zero::Status &status);

    // Calls StopSystem on the internal system. Sets the status failed info if system does not shut down correctly. 
    void StopSystem(Zero::Status &status);

    // Update function to be called by the external engine every game update.
    void Update();

    // Gets the overall system volume
    float GetVolume();

    // Changes the overall system volume
    void SetVolume(const float volume);

    // Returns the number of channels used in the system's output
    unsigned GetOutputChannels();

    // Sets the number of channels that should be used to mix audio output. This will be translated to
    // the channels needed by the system's audio output device.
    void SetOutputChannels(const unsigned channels);

    // Adds a node to the system's final output node
    void AddNodeToOutput(SoundNode* node);

    // Removes a node from the system's final output node
    void RemoveNodeFromOutput(SoundNode* node);

    // Returns the highest volume from the last audio mix.
    float GetPeakOutputVolume();

    // Returns the RMS volume of the last audio mix.
    float GetRMSOutputVolume();

    // Sets the minimum volume at which SoundInstances will process audio.
    void SetMinimumVolumeThreshold(const float volume);

    void UseHighLatency(const bool useHigh);

  private:
    // Pointer to the internal system object
    AudioSystemInternal *System;

  };
}

#endif

