///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //---------------------------------------------------------------------------------- Note Harmonic

  // used by AdditiveNote
  class NoteHarmonic
  {
  public:
    NoteHarmonic() :
      mVolume(1.0f)
    {}

    void SetValues(float frequency, float volume, EnvelopeSettings& envelope, Oscillator::Types waveType);
    float operator()();
    bool IsFinished();
    void Stop();

  private:
    Oscillator WaveSamples;
    ADSR Envelope;
    float mVolume;
  };

  //************************************************************************************************
  void NoteHarmonic::SetValues(float frequency, float volume, EnvelopeSettings& envelope,
    Oscillator::Types waveType)
  {
    WaveSamples.SetFrequency(frequency);
    WaveSamples.SetType(waveType);
    Envelope.SetValues(&envelope);
    mVolume = volume;
    WaveSamples.SetNoteOn(true);
  }

  //************************************************************************************************
  float NoteHarmonic::operator()()
  {
    return mVolume * Envelope() * WaveSamples.GetNextSample();
  }

  //************************************************************************************************
  bool NoteHarmonic::IsFinished()
  {
    return Envelope.IsFinished();
  }

  //************************************************************************************************
  void NoteHarmonic::Stop()
  {
    Envelope.Release();
  }

  //----------------------------------------------------------------------------------- AdditiveNote

  // used by AdditiveSynthNode
  class AdditiveNote
  {
  public:

    void AddHarmonic(float frequency, float volume, EnvelopeSettings& envelope,
      Oscillator::Types waveType);
    float operator()();
    bool IsFinished();
    void Stop();

    float Volume;

  private:
    typedef Zero::Array<NoteHarmonic> HarmonicsListType;
    HarmonicsListType Harmonics;
  };

  //************************************************************************************************
  void AdditiveNote::AddHarmonic(float frequency, float volume, EnvelopeSettings& envelope,
    Oscillator::Types waveType)
  {
    Harmonics.PushBack().SetValues(frequency, volume, envelope, waveType);
  }

  //************************************************************************************************
  float AdditiveNote::operator()()
  {
    float value(0.0f);
    for (HarmonicsListType::iterator it = Harmonics.Begin(); it != Harmonics.End(); ++it)
      value += (*it)();
    return value * Volume;
  }

  //************************************************************************************************
  bool AdditiveNote::IsFinished()
  {
    for (HarmonicsListType::iterator it = Harmonics.Begin(); it != Harmonics.End(); ++it)
    {
      if (!it->IsFinished())
        return false;
    }

    return true;
  }

  //************************************************************************************************
  void AdditiveNote::Stop()
  {
    for (HarmonicsListType::iterator it = Harmonics.Begin(); it != Harmonics.End(); ++it)
      it->Stop();
  }

  //---------------------------------------------------------------------------- Additive Synth Node

  //************************************************************************************************
  AdditiveSynthNode::AdditiveSynthNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
    ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, true, isThreaded),
    CurrentNoteCount(0)
  {
    if (!Threaded)
      SetSiblingNodes(new AdditiveSynthNode(status, name, ID, nullptr, true), status);
  }

  //************************************************************************************************
  AdditiveSynthNode::~AdditiveSynthNode()
  {
    // Look at each MIDI note in the map
    for (NotesMapType::valuerange allLists = CurrentNotes.Values(); !allLists.Empty(); allLists.PopFront())
    {
      NotesListType& list = *allLists.Front();
      // Step through each current note at this frequency
      for (unsigned i = 0; i < list.Size(); ++i)
        delete list[i];
    }
  }

  //************************************************************************************************
  void AdditiveSynthNode::AddHarmonic(HarmonicData harmonic)
  {
    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&AdditiveSynthNode::AddHarmonic,
        (AdditiveSynthNode*)GetSiblingNode(), harmonic));
    }
    else
    {
      Harmonics.PushBack(harmonic);
    }
  }

  //************************************************************************************************
  void AdditiveSynthNode::RemoveAllHarmonics()
  {
    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&AdditiveSynthNode::RemoveAllHarmonics,
        (AdditiveSynthNode*)GetSiblingNode()));
    }
    else
    {
      Harmonics.Clear();
    }
  }

  //************************************************************************************************
  void AdditiveSynthNode::NoteOn(const int note, const float volume)
  {
    if (!Threaded)
    {
      if (note < 0 || note > 127)
        return;

      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&AdditiveSynthNode::NoteOn,
        (AdditiveSynthNode*)GetSiblingNode(), note, volume));
    }
    else
    {
      float frequency = Math::Pow(2.0f, (float)(note - 69) / 12.0f) * 440.0f;

      if (!CurrentNotes.FindValue(note, nullptr))
        CurrentNotes[note] = new NotesListType;

      AdditiveNote* newNote = new AdditiveNote();
      CurrentNotes[note]->PushBack(newNote);
      newNote->Volume = volume;

      for (unsigned i = 0; i < Harmonics.Size(); ++i)
      {
        HarmonicData& data = Harmonics[i];
        newNote->AddHarmonic(frequency * data.FrequencyMultiplier, data.Volume, data.Envelope,
          (Oscillator::Types)data.WaveType);
      }

      ++CurrentNoteCount;
    }
  }

  //************************************************************************************************
  void AdditiveSynthNode::NoteOff(const int note)
  {
    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&AdditiveSynthNode::NoteOff,
        (AdditiveSynthNode*)GetSiblingNode(), note));
    }
    else
    {
      if (CurrentNotes.FindValue(note, nullptr))
      {
        for (unsigned i = 0; i < CurrentNotes[note]->Size(); ++i)
          (*CurrentNotes[note])[i]->Stop();
      }
    }
  }

  //************************************************************************************************
  void AdditiveSynthNode::StopAll()
  {
    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&AdditiveSynthNode::StopAll,
        (AdditiveSynthNode*)GetSiblingNode()));
    }
    else
    {
      for (NotesMapType::valuerange allLists = CurrentNotes.Values(); !allLists.Empty(); allLists.PopFront())
      {
        NotesListType& list = *allLists.Front();
        for (unsigned i = 0; i < list.Size(); ++i)
          list[i]->Stop();
      }
    }
  }

  //************************************************************************************************
  bool AdditiveSynthNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    if (CurrentNoteCount == 0)
      return false;

    unsigned bufferSize = outputBuffer->Size();

    memset(outputBuffer->Data(), 0, sizeof(float) * bufferSize);

    // Look at each MIDI note in the map
    for (NotesMapType::valuerange allLists = CurrentNotes.Values(); !allLists.Empty(); allLists.PopFront())
    {
      NotesListType& list = *allLists.Front();
      // Step through each current note at this frequency
      for (unsigned i = 0; i < list.Size(); ++i)
      {
        // If the note is finished, remove it from the list
        if (list[i]->IsFinished())
        {
          --CurrentNoteCount;
          delete list[i];
          list.EraseAt(i);

          // Decrement i since we're removing this one from the list
          --i;
        }
        // If not finished, add samples from this note into the buffer
        else
        {
          for (unsigned frame = 0; frame < bufferSize; frame += numberOfChannels)
          {
            float sample = (*list[i])();
            // Copy sample to all channels
            for (unsigned channel = 0; channel < numberOfChannels; ++channel)
              (*outputBuffer)[frame + channel] += sample;
          }
        }
      }
    }

    return true;
  }
}
