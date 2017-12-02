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
#include "Interpolator.h"
#include "PitchChange.h"
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
#include "MicrophoneInputNode.h"
#include "CustomDataNode.h"
#include "Emitter.h"
#include "Attenuator.h"
#include "SoundAssets.h"
#include "SoundInstances.h"
#include "Tags.h"

namespace Audio
{

  //------------------------------------------------------------------------ External System Interface

  class ExternalSystemInterface
  {
  public:
    virtual void SendAudioEvent(const AudioEventTypes::Enum eventType, void* data) = 0;
    virtual void SendAudioError(const Zero::String message) = 0;
  };

  //------------------------------------------------------------------------ External Node Interface

  class ExternalNodeInterface
  {
  public:
    virtual void SendAudioEvent(const AudioEventTypes::Enum eventType, void* data) = 0;
  };

  //------------------------------------------------------------------------- Audio System Interface

  class PacketEncoder;

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

    // Returns the sample rate used by the audio system
    unsigned GetSampleRate();

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

    // Sets whether the audio system should use a high latency value
    void UseHighLatency(const bool useHigh);

    // If true, events will be sent with microphone input data as float samples
    void SetSendUncompressedMicInput(const bool sendInput);

    // If true, events will be sent with compressed microphone input data as bytes
    void SetSendCompressedMicInput(const bool sendInput);

    // If currently sending microphone input data, returns the highest peak volume in the last input
    float GetPeakInputVolume(Zero::Status& status);

  private:
    // Pointer to the internal system object
    AudioSystemInternal* System;
    // Number of output channels used by the audio system
    unsigned SystemOutputChannels;
    // Stored microphone input samples when sending compressed input
    Zero::Array<float> PreviousInputSamples;
    // Maximum number of audio frames of microphone input to send
    static const unsigned MaxInputFrames = 5000;
    // If true, will send microphone input data to external system
    bool SendUncompressedInputData;
    // If true, will send compressed microphone input data to external system
    bool SendCompressedInputData;
    // The peak volume of the latest input data
    float PeakInputVolume;

    void SetSendMicInput(const bool turnOn);
  };

  //------------------------------------------------------------------------------------- Audio File

  class AudioFileData;

  class AudioFile
  {
  public:
    AudioFile();
    ~AudioFile();

    // Opens an audio file and reads its data into a buffer so it's ready for encoding
    void OpenFile(Zero::Status& status, Zero::StringParam fileName);
    // Encodes the audio data and writes it out to a new file
    void WriteEncodedFile(Zero::Status& status, Zero::StringParam outputFileName, bool normalize,
      float maxVolume);
    // Deletes the current audio data
    void Close();

    // Number of channels of audio data in the opened file
    unsigned Channels;
    // Length, in seconds, of the opened audio file
    float FileLength;

  private:
    ZeroDeclarePrivateData(AudioFile, 16);
  };

  //--------------------------------------------------------------------------- Audio Stream Decoder

  class PacketDecoder;

  class AudioStreamDecoder
  {
  public:
    AudioStreamDecoder();

    // Decodes the provided encoded byte data
    void DecodeCompressedPacket(const byte* encodedData, const unsigned dataSize, 
      float*& decodedSamples, unsigned& sampleCount);

  private:
    PacketDecoder* Decoder;
  };
}

#endif

