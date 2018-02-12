///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef DEFINITIONS_H
#define DEFINITIONS_H


namespace Audio
{
  typedef intptr_t AtomicType;

  // Maximum number of channels in audio output
  static const unsigned MaxChannels = 8;

  // Volume modifier applied to all generated waves
  static const float GeneratedWaveVolume = 0.5f;

  // Types of curves used for interpolation. 
  namespace CurveTypes
  {
    enum Enum { Linear, Squared, Sine, SquareRoot, Log, Custom };
  };

  typedef Zero::Array<float> BufferType;

  struct MidiData
  {
    MidiData() : Channel(0), Value1(0), Value2(0) {}
    MidiData(int channel, float value1, float value2) : Channel(channel), Value1(value1), Value2(value2) {}

    int Channel;
    float Value1;
    float Value2;
  };

  namespace AudioEventTypes
  {
    enum Enum
    {
      // Notification type has not been set.
      NotSet,
      // Sound instance has finished playing. 
      InstanceFinished,
      // Sound instance has looped back to beginning. 
      InstanceLooped,
      // Tag has no more instances associated with it. 
      TagIsUnreferenced,
      // New sound instance was added to a tag
      TagAddedInstance,
      // Audio output is exceeding the maximum value. 
      AudioClipping,
      // Sound asset has no more instances. 
      AssetUnreferenced,
      // Sound asset has finished and been deleted. 
      AssetRemoved,
      // The custom input node needs another buffer of samples 
      NeedInputSamples,
      // An interpolation has finished 
      InterpolationDone,
      // Music notifications
      MusicBar,
      MusicBeat,
      MusicEighthNote,
      MusicQuarterNote,
      MusicHalfNote,
      MusicWholeNote,
      MusicCustomTime,
      // Sound node was disconnected
      NodeDisconnected,
      // MIDI notifications
      MidiNoteOn,
      MidiNoteOff,
      MidiPitchWheel,
      MidiVolume,
      MidiModWheel,
      MidiControl,
      // Sends a buffer of input data
      MicInputData,
      // Sends a compressed packet of input data
      CompressedMicInputData
    };
  }
}

#endif