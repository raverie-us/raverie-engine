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

  // The sample rate used by the audio engine for the output mix
  static const unsigned cSystemSampleRate = 48000;
  // The time increment per audio frame corresponding to the sample rate
  static const double cSystemTimeIncrement = 1.0 / 48000.0;
  // The number of frames used to interpolate instant property changes
  static const unsigned  cPropertyChangeFrames = (unsigned)(48000 * 0.02f);
  // Maximum number of channels in audio output
  static const unsigned cMaxChannels = 8;
  // Volume modifier applied to all generated waves
  static const float cGeneratedWaveVolume = 0.5f;

  // Types of curves used for interpolation. 
  namespace CurveTypes
  {
    enum Enum { Linear, Squared, Sine, SquareRoot, Log, Custom };
  };

  typedef Zero::Array<float> BufferType;

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
      // MIDI notifications (must *always* match MidiEvent enum)
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

  class EventData
  {
  public:
    EventData() : mEventType(AudioEventTypes::NotSet) {}
    EventData(AudioEventTypes::Enum type) : mEventType(type) {}
    EventData(const EventData& copy) : mEventType(copy.mEventType) {}
    virtual ~EventData() {}

    AudioEventTypes::Enum mEventType;
  };

  template <typename T>
  class EventData1 : public EventData
  {
  public:
    EventData1() {}
    EventData1(AudioEventTypes::Enum type, T data) : 
      EventData(type), 
      mData(data) 
    {}
    EventData1(const EventData1<T>& copy) : 
      EventData(copy.mEventType),
      mData(copy.mData) 
    {}

    T mData;
  };

  template <typename T1, typename T2>
  class EventData2 : public EventData
  {
  public:
    EventData2() {}
    EventData2(AudioEventTypes::Enum type, T1 data1, T2 data2) :
      EventData(type), 
      mData1(data1),
      mData2(data2)
    {}
    EventData2(const EventData2<T1, T2>& copy) :
      EventData(copy.mEventType), 
      mData1(copy.mData1),
      mData2(copy.mData2)
    {}

    T1 mData1;
    T2 mData2;
  };

  template <typename T1, typename T2, typename T3>
  class EventData3 : public EventData
  {
  public:
    EventData3() {}
    EventData3(AudioEventTypes::Enum type, T1 data1, T2 data2, T3 data3) :
      EventData(type), 
      mData1(data1),
      mData2(data2),
      mData3(data3)
    {}
    EventData3(const EventData3<T1, T2, T3>& copy) : 
      EventData(copy.mEventType), 
      mData1(copy.mData1),
      mData2(copy.mData2),
      mData3(copy.mData3)
    {}

    T1 mData1;
    T2 mData2;
    T3 mData3;
  };
}

#endif
