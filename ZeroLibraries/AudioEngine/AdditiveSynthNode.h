///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef ADDITIVESYNTH_H
#define ADDITIVESYNTH_H

namespace Audio
{
  //------------------------------------------------------------------------------ Envelope Settings

  class EnvelopeSettings
  {
  public:
    EnvelopeSettings() :
      DelayTime(0.0f),
      AttackTime(0.02f),
      DecayTime(0.0f),
      SustainTime(0.0f),
      SustainLevel(1.0f),
      ReleaseTime(0.02f)
    {}
    EnvelopeSettings(float delayTime, float attackTime, float decayTime, float sustainTime,
      float sustainLevel, float releaseTime) :
      DelayTime(delayTime),
      AttackTime(attackTime),
      DecayTime(decayTime),
      SustainTime(sustainTime),
      SustainLevel(sustainLevel),
      ReleaseTime(releaseTime)
    {}

    float DelayTime;
    float AttackTime;
    float DecayTime;
    float SustainTime;
    float SustainLevel;
    float ReleaseTime;
  };

  //---------------------------------------------------------------------------------- Harmonic Data

  struct HarmonicData
  {
    HarmonicData(float multiplier, float volume, EnvelopeSettings& envelope, OscillatorTypes type) :
      FrequencyMultiplier(multiplier),
      Volume(volume),
      Envelope(envelope),
      WaveType(type)
    {}
    HarmonicData() :
      FrequencyMultiplier(0.0f),
      Volume(0.0f)
    {}

    float FrequencyMultiplier;
    float Volume;
    EnvelopeSettings Envelope;
    OscillatorTypes WaveType;
  };

  //---------------------------------------------------------------------------- Additive Synth Node

  class AdditiveNote;

  class AdditiveSynthNode : public SimpleCollapseNode
  {
  public:
    AdditiveSynthNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface *extInt, const bool threaded = false);

    // Adds a new note harmonic
    void AddHarmonic(HarmonicData harmonic);
    // Removes all harmonics
    void RemoveAllHarmonics();
    // Starts a new note
    void NoteOn(const int note, const float volume);
    // Stops a specified note (will stop all current notes at that pitch)
    void NoteOff(const int note);
    // Stops all currently playing notes
    void StopAll();

  private:
    ~AdditiveSynthNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;

    typedef Zero::Array<HarmonicData> HarmonicDataListType;
    typedef Zero::Array<AdditiveNote*> NotesListType;
    typedef Zero::HashMap<int, NotesListType*> NotesMapType;

    // The number of notes currently playing
    int CurrentNoteCount;
    // The harmonics to use for the notes
    HarmonicDataListType Harmonics;
    // A map of MIDI note values currently playing to a list of note objects
    NotesMapType CurrentNotes;
  };

}

#endif
