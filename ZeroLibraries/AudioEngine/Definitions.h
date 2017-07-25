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
  // Maximum number of channels in audio output
  static const unsigned MaxChannels = 8;

  // Types of curves used for interpolation. 
  enum CurveTypes
  {
    LinearCurveType,
    SquaredCurveType,
    SineCurveType,
    SquareRootCurveType,
    LogCurveType,
    CustomCurveType
  };

  // Types of files
  enum AudioFileTypes
  {
    WAV_Type,
    OGG_Type,
    Other_Type
  };

  struct MidiData
  {
    MidiData() : Channel(0), Value1(0), Value2(0) {}
    MidiData(int channel, float value1, float value2) : Channel(channel), Value1(value1), Value2(value2) {}

    int Channel;
    float Value1;
    float Value2;
  };

  enum AudioEventType
  {
    // Notification type has not been set.
    Notify_NotSet,
    // Sound instance has finished playing. 
    Notify_InstanceFinished,
    // Sound instance has looped back to beginning. 
    Notify_InstanceLooped,
    // Tag has no more instances associated with it. 
    Notify_TagIsUnreferenced,
    // New sound instance was added to a tag
    Notify_TagAddedInstance,
    // Audio output is exceeding the maximum value. 
    Notify_AudioClipping,
    // Sound asset has no more instances. 
    Notify_AssetUnreferenced,
    // Sound asset has finished and been deleted. 
    Notify_AssetRemoved,
    // System error (will have error string associated with it) 
    Notify_Error,
    // The custom input node needs another buffer of samples 
    Notify_NeedInputSamples,
    // An interpolation has finished 
    Notify_InterpolationDone,
    // Music notifications
    Notify_MusicBar,
    Notify_MusicBeat,
    Notify_MusicEighthNote,
    Notify_MusicQuarterNote,
    Notify_MusicHalfNote,
    Notify_MusicWholeNote,
    Notify_MusicCustomTime,
    // Sound node was disconnected
    Notify_NodeDisconnected,
    // MIDI notifications
    Notify_MidiNoteOn,
    Notify_MidiNoteOff,
    Notify_MidiPitchWheel,
    Notify_MidiVolume,
    Notify_MidiModWheel,
    Notify_MidiControl
  };
}

#endif