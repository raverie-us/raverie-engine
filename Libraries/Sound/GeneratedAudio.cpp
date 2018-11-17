///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

using namespace AudioConstants;

//------------------------------------------------------------------------------ Generated Wave Node

//**************************************************************************************************
ZilchDefineType(GeneratedWaveNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(WaveType);
  ZilchBindGetterSetter(WaveFrequency);
  ZilchBindGetterSetter(Volume);
  ZilchBindGetterSetter(Decibels);
  ZilchBindGetterSetter(SquareWavePulseValue);
  ZilchBindMethod(Play);
  ZilchBindMethod(Stop);
  ZilchBindMethod(InterpolateVolume);
  ZilchBindMethod(InterpolateDecibels);
  ZilchBindMethod(InterpolateWaveFrequency);
}

//**************************************************************************************************
GeneratedWaveNode::GeneratedWaveNode(StringParam name, unsigned ID) :
  SoundNode(name, ID, false, true),
  mWaveType(SynthWaveType::SineWave),
  mWaveFrequency(440.0f),
  mVolume(1.0f),
  mSquareWavePulseValue(0.5f),
  mState(Off)
{
  WaveDataThreaded.SetType(mWaveType);
  WaveDataThreaded.SetFrequency(mWaveFrequency.Get(AudioThreads::MainThread));
}

//**************************************************************************************************
SynthWaveType::Enum GeneratedWaveNode::GetWaveType()
{
  return mWaveType;
}

//**************************************************************************************************
void GeneratedWaveNode::SetWaveType(SynthWaveType::Enum newType)
{
  mWaveType = newType;

  Z::gSound->Mixer.AddTask(CreateFunctor(&Oscillator::SetType, &WaveDataThreaded, newType), this);
}

//**************************************************************************************************
float GeneratedWaveNode::GetWaveFrequency()
{
  return mWaveFrequency.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void GeneratedWaveNode::SetWaveFrequency(float frequency)
{
  frequency = Math::Clamp(frequency, 0.0f, 20000.0f);
  mWaveFrequency.Set(frequency, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&Oscillator::SetFrequency, &WaveDataThreaded, frequency), this);
}

//**************************************************************************************************
void GeneratedWaveNode::InterpolateWaveFrequency(float frequency, float time)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&GeneratedWaveNode::InterpolateFrequencyThreaded, this,
    Math::Clamp(frequency, 0.0f, 20000.0f), time), this);
}

//**************************************************************************************************
void GeneratedWaveNode::Play()
{
  mState.Set(Starting, AudioThreads::MainThread);
}

//**************************************************************************************************
void GeneratedWaveNode::Stop()
{
  mState.Set(Stopping, AudioThreads::MainThread);
}

//**************************************************************************************************
float GeneratedWaveNode::GetVolume()
{
  return mVolume.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void GeneratedWaveNode::SetVolume(float volume)
{
  InterpolateVolume(volume, 0.0f);
}

//**************************************************************************************************
void GeneratedWaveNode::InterpolateVolume(float volume, float time)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&GeneratedWaveNode::InterpolateVolumeThreaded, this,
    Math::Clamp(volume, 0.0f, cMaxVolumeValue), Math::Max(time, 0.01f)), this);
}

//**************************************************************************************************
float GeneratedWaveNode::GetDecibels()
{
  return VolumeToDecibels(mVolume.Get(AudioThreads::MainThread));
}

//**************************************************************************************************
void GeneratedWaveNode::SetDecibels(float decibels)
{
  InterpolateDecibels(decibels, 0.0f);
}

//**************************************************************************************************
void GeneratedWaveNode::InterpolateDecibels(float decibels, float time)
{
  InterpolateVolume(DecibelsToVolume(decibels), time);
}

//**************************************************************************************************
float GeneratedWaveNode::GetSquareWavePulseValue()
{
  return mSquareWavePulseValue;
}

//**************************************************************************************************
void GeneratedWaveNode::SetSquareWavePulseValue(float value)
{
  mSquareWavePulseValue = Math::Clamp(value, 0.0f, 1.0f);

  Z::gSound->Mixer.AddTask(CreateFunctor(&Oscillator::SetSquareWavePositiveFraction, &WaveDataThreaded,
    mSquareWavePulseValue), this);
}

//**************************************************************************************************
bool GeneratedWaveNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
  ListenerNode* listener, const bool firstRequest)
{
  if (mState.Get(AudioThreads::MixThread) == Off)
    return false;

  if (mState.Get(AudioThreads::MixThread) == Stopping)
  {
    VolumeInterpolatorThreaded.SetValues(mVolume.Get(AudioThreads::MixThread), 0.0f,
      (unsigned)(outputBuffer->Size() / numberOfChannels));
    mState.Set(Off, AudioThreads::MixThread);
  }
  else if (mState.Get(AudioThreads::MixThread) == Starting)
  {
    if (VolumeInterpolatorThreaded.Finished())
      VolumeInterpolatorThreaded.SetValues(0.0f, mVolume.Get(AudioThreads::MixThread),
      (unsigned)(outputBuffer->Size() / numberOfChannels));
    mState.Set(Active, AudioThreads::MixThread);
  }

  bool interpolatingVolume = !VolumeInterpolatorThreaded.Finished();
  bool interpolatingFreq = !FrequencyInterpolatorThreaded.Finished();

  float volume = mVolume.Get(AudioThreads::MixThread);
  BufferRange sampleRange = outputBuffer->All();
  while (!sampleRange.Empty())
  {
    if (!VolumeInterpolatorThreaded.Finished())
      volume = VolumeInterpolatorThreaded.NextValue();
    if (!FrequencyInterpolatorThreaded.Finished())
      WaveDataThreaded.SetFrequency(FrequencyInterpolatorThreaded.NextValue());

    float sample = WaveDataThreaded.GetNextSample() * volume;

    for (unsigned i = 0; i < numberOfChannels; ++i, sampleRange.PopFront())
      sampleRange.Front() = sample;
  }
  
  if (interpolatingVolume)
    mVolume.Set(volume, AudioThreads::MixThread);
  if (interpolatingFreq)
    mWaveFrequency.Set(FrequencyInterpolatorThreaded.GetCurrentValue(), AudioThreads::MixThread);

  if (mState.Get(AudioThreads::MixThread) == Off)
    VolumeInterpolatorThreaded.SetValues(0.0f, 0.0f, 0u);

  return true;
}

//**************************************************************************************************
void GeneratedWaveNode::InterpolateFrequencyThreaded(float frequency, float time)
{
  FrequencyInterpolatorThreaded.SetValues(mWaveFrequency.Get(AudioThreads::MixThread),
    frequency, (unsigned)(time * cSystemSampleRate));
}

//**************************************************************************************************
void GeneratedWaveNode::InterpolateVolumeThreaded(float volume, float time)
{
  VolumeInterpolatorThreaded.SetValues(mVolume.Get(AudioThreads::MixThread), volume,
    (unsigned)(time * cSystemSampleRate));
}

//------------------------------------------------------------------------------------ ADSR Envelope

//**************************************************************************************************
ZilchDefineType(AdsrEnvelope, builder, type)
{
  ZilchBindDefaultConstructor();
  ZeroBindDocumented();
  type->CreatableInScript = true;

  ZilchBindField(mDelayTime);
  ZilchBindField(mAttackTime);
  ZilchBindField(mDecayTime);
  ZilchBindField(mSustainTime);
  ZilchBindField(mSustainLevel);
  ZilchBindField(mReleaseTime);
}

//------------------------------------------------------------------------------------ Note Harmonic

//**************************************************************************************************
void NoteHarmonic::SetValues(float frequency, float volume, EnvelopeSettings& envelope,
  SynthWaveType::Enum waveType)
{
  WaveSamples.SetFrequency(frequency);
  WaveSamples.SetType(waveType);
  Envelope.SetValues(&envelope);
  mVolume = volume;
  WaveSamples.SetNoteOn(true);
}

//**************************************************************************************************
float NoteHarmonic::operator()()
{
  return mVolume * Envelope() * WaveSamples.GetNextSample();
}

//**************************************************************************************************
bool NoteHarmonic::IsFinished()
{
  return Envelope.IsFinished();
}

//**************************************************************************************************
void NoteHarmonic::Stop()
{
  Envelope.Release();
}

//------------------------------------------------------------------------------------ Additive Note

//**************************************************************************************************
void AdditiveNote::AddHarmonic(float frequency, float volume, EnvelopeSettings& envelope,
  SynthWaveType::Enum waveType)
{
  Harmonics.PushBack().SetValues(frequency, volume, envelope, waveType);
}

//**************************************************************************************************
float AdditiveNote::operator()()
{
  float value(0.0f);
  for (HarmonicsListType::iterator it = Harmonics.Begin(); it != Harmonics.End(); ++it)
    value += (*it)();
  return value * mVolume;
}

//**************************************************************************************************
bool AdditiveNote::IsFinished()
{
  for (HarmonicsListType::iterator it = Harmonics.Begin(); it != Harmonics.End(); ++it)
  {
    if (!it->IsFinished())
      return false;
  }

  return true;
}

//**************************************************************************************************
void AdditiveNote::Stop()
{
  for (HarmonicsListType::iterator it = Harmonics.Begin(); it != Harmonics.End(); ++it)
    it->Stop();
}

//------------------------------------------------------------------------------ Additive Synth Node

//**************************************************************************************************
ZilchDefineType(AdditiveSynthNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindMethod(AddHarmonic);
  ZilchBindMethod(RemoveAllHarmonics);
  ZilchBindMethod(NoteOn);
  ZilchBindMethod(NoteOff);
  ZilchBindMethod(StopAllNotes);
}

//**************************************************************************************************
AdditiveSynthNode::AdditiveSynthNode(StringParam name, unsigned ID) :
  SoundNode(name, ID, false, true),
  mCurrentNoteCountThreaded(0)
{

}

//**************************************************************************************************
AdditiveSynthNode::~AdditiveSynthNode()
{
  // Look at each MIDI note in the map
  for (NotesMapType::valuerange allLists = CurrentNotesMapThreaded.Values(); !allLists.Empty(); allLists.PopFront())
  {
    NotesListType& list = *allLists.Front();
    // Step through each current note at this frequency
    for (unsigned i = 0; i < list.Size(); ++i)
      delete list[i];
  }
}

//**************************************************************************************************
void AdditiveSynthNode::AddHarmonic(float multiplier, float volume, AdsrEnvelope envelope,
  SynthWaveType::Enum type)
{
  EnvelopeSettings envelopeSettings(
    Math::Max(envelope.mDelayTime, 0.0f),
    Math::Max(envelope.mAttackTime, 0.0f),
    Math::Max(envelope.mDecayTime, 0.0f),
    Math::Max(envelope.mSustainTime, 0.0f),
    Math::Max(envelope.mSustainLevel, 0.0f),
    Math::Max(envelope.mReleaseTime, 0.0f));

  Z::gSound->Mixer.AddTask(CreateFunctor(&AdditiveSynthNode::AddHarmonicThreaded, this,
    HarmonicData(Math::Max(multiplier, 0.0f), Math::Max(volume, 0.0f), envelopeSettings, type)), this);
}

//**************************************************************************************************
void AdditiveSynthNode::RemoveAllHarmonics()
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&HarmonicDataListType::Clear, &HarmonicsListThreaded), this);
}

//**************************************************************************************************
void AdditiveSynthNode::NoteOn(float midiNote, float volume)
{
  if (midiNote < 0 || midiNote > 127)
    return;

  Z::gSound->Mixer.AddTask(CreateFunctor(&AdditiveSynthNode::NoteOnThreaded, this, (int)midiNote,
    volume), this);
}

//**************************************************************************************************
void AdditiveSynthNode::NoteOff(float midiNote)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&AdditiveSynthNode::NoteOffThreaded, this, (int)midiNote), this);
}

//**************************************************************************************************
void AdditiveSynthNode::StopAllNotes()
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&AdditiveSynthNode::StopAllNotesThreaded, this), this);
}

//**************************************************************************************************
bool AdditiveSynthNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
  ListenerNode* listener, const bool firstRequest)
{
  if (mCurrentNoteCountThreaded == 0)
    return false;

  unsigned bufferSize = outputBuffer->Size();

  memset(outputBuffer->Data(), 0, sizeof(float) * bufferSize);

  // Look at each MIDI note in the map
  forRange(NotesListType* notesList, CurrentNotesMapThreaded.Values())
  {
    NotesListType& list = *notesList;
    // Step through each current note at this frequency
    for (unsigned i = 0; i < list.Size(); ++i)
    {
      // If the note is finished, remove it from the list
      if (list[i]->IsFinished())
      {
        --mCurrentNoteCountThreaded;
        delete list[i];
        list.EraseAt(i);

        // Decrement i since we're removing this one from the list
        --i;
      }
      // If not finished, add samples from this note into the buffer
      else
      {
        BufferRange outputRange = outputBuffer->All();
        while (!outputRange.Empty())
        {
          float sample = (*list[i])();
          // Copy sample to all channels
          for (unsigned channel = 0; channel < numberOfChannels; ++channel, outputRange.PopFront())
            outputRange.Front() += sample;
        }
      }
    }
  }

  return true;
}

//**************************************************************************************************
void AdditiveSynthNode::AddHarmonicThreaded(HarmonicData data)
{
  HarmonicsListThreaded.PushBack(data);
}

//**************************************************************************************************
void AdditiveSynthNode::NoteOnThreaded(int midiNote, float volume)
{
  float frequency = Math::Pow(2.0f, (float)(midiNote - 69) / 12.0f) * 440.0f;

  if (!CurrentNotesMapThreaded.FindValue(midiNote, nullptr))
    CurrentNotesMapThreaded[midiNote] = new NotesListType;

  AdditiveNote* newNote = new AdditiveNote();
  CurrentNotesMapThreaded[midiNote]->PushBack(newNote);
  newNote->mVolume = volume;

  forRange(HarmonicData& data, HarmonicsListThreaded.All())
  {
    newNote->AddHarmonic(frequency * data.mFrequencyMultiplier, data.mVolume, data.mEnvelope,
      data.mWaveType);
  }

  ++mCurrentNoteCountThreaded;
}

//**************************************************************************************************
void AdditiveSynthNode::NoteOffThreaded(int midiNote)
{
  if (CurrentNotesMapThreaded.FindValue(midiNote, nullptr))
  {
    forRange(AdditiveNote* note, CurrentNotesMapThreaded[midiNote]->All())
      note->Stop();
  }
}

//**************************************************************************************************
void AdditiveSynthNode::StopAllNotesThreaded()
{
  for (NotesMapType::valuerange allLists = CurrentNotesMapThreaded.Values(); !allLists.Empty();
    allLists.PopFront())
  {
    forRange(AdditiveNote* note, allLists.Front()->All())
      note->Stop();
  }
}

//---------------------------------------------------------------------------- Microphone Input Node

//**************************************************************************************************
ZilchDefineType(MicrophoneInputNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(Volume);
  ZilchBindGetterSetter(Active);
}

//**************************************************************************************************
MicrophoneInputNode::MicrophoneInputNode(StringParam name, unsigned ID) :
  SoundNode(name, ID, false, true),
  mActive(false),
  mVolume(1.0f),
  mStopping(cFalse)
{

}

//**************************************************************************************************
float MicrophoneInputNode::GetVolume()
{
  return mVolume.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void MicrophoneInputNode::SetVolume(float volume)
{
  // Don't do anything if setting to same value
  if (volume == mVolume.Get(AudioThreads::MainThread))
    return;

  mVolume.Set(Math::Clamp(volume, 0.0f, cMaxVolumeValue), AudioThreads::MainThread);

  // Don't change volume if we are currently not active or deactivating
  if (mActive.Get(AudioThreads::MainThread) && mStopping.Get() == cFalse)
    Z::gSound->Mixer.AddTask(CreateFunctor(&InterpolatingObject::SetValues, &VolumeInterpolatorThreaded,
      volume, cPropertyChangeFrames), this);
}

//**************************************************************************************************
bool MicrophoneInputNode::GetActive()
{
  return mActive.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void MicrophoneInputNode::SetActive(bool active)
{
  if (active == mActive.Get(AudioThreads::MainThread))
    return;

  if (active && !Z::gSound->Mixer.StartInput())
  {
    DoNotifyWarning("No Microphone Available", "No microphone input is available for the MicrophoneInputNode");
    return;
  }

  // Check if we are deactivating
  if (!active && mStopping.Get() == cFalse)
  {
    mStopping.Set(cTrue);
    Z::gSound->Mixer.AddTask(CreateFunctor(&InterpolatingObject::SetValues, &VolumeInterpolatorThreaded,
      0.0f, cPropertyChangeFrames), this);
  }
  // Otherwise we are activating
  {
    // Make sure the stopping flag is reset
    mStopping.Set(cFalse);
    // Interpolate volume to its previous setting
    Z::gSound->Mixer.AddTask(CreateFunctor(&InterpolatingObject::SetValues, &VolumeInterpolatorThreaded,
      mVolume.Get(AudioThreads::MainThread), cPropertyChangeFrames), this);
    // Mark that we are active
    mActive.Set(true, AudioThreads::MainThread);
  }
}

//**************************************************************************************************
bool MicrophoneInputNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
  ListenerNode* listener, const bool firstRequest)
{
  if (!mActive.Get(AudioThreads::MixThread) || Z::gSound->Mixer.InputBuffer.Empty())
    return false;

  unsigned bufferSize = outputBuffer->Size();

  // Start with the number of samples in the system's input buffer
  unsigned samplesToCopy = Z::gSound->Mixer.InputBuffer.Size();
  // If this is larger than the output buffer, adjust the amount
  if (samplesToCopy > bufferSize)
    samplesToCopy = bufferSize;

  // Copy the samples from the input buffer to the output buffer
  memcpy(outputBuffer->Data(), Z::gSound->Mixer.InputBuffer.Data(), sizeof(float) * samplesToCopy);

  // If we copied less samples than the output buffer size, fill the rest with zeros
  if (samplesToCopy < bufferSize)
    memset(outputBuffer->Data() + samplesToCopy, 0, sizeof(float) * (bufferSize - samplesToCopy));

  // If interpolating volume or volume is not 1.0, apply volume change
  float volume = mVolume.Get(AudioThreads::MixThread);
  if (!VolumeInterpolatorThreaded.Finished() || !IsWithinLimit(volume, 1.0f, 0.01f))
  {
    // Step through output buffer
    BufferRange outputRange = outputBuffer->All();
    while (!outputRange.Empty())
    {
      // Check if volume is interpolating
      if (!VolumeInterpolatorThreaded.Finished())
      {
        volume = VolumeInterpolatorThreaded.NextValue();

        // If interpolation is now finished and we are stopping, mark as not active
        if (VolumeInterpolatorThreaded.Finished() && mStopping.Get() == cTrue)
          mActive.Set(false, AudioThreads::MixThread);
      }

      // Apply volume to all channels in this frame
      for (unsigned channel = 0; channel < numberOfChannels; ++channel, outputRange.PopFront())
        outputRange.Front() *= volume;
    }

    if (volume != mVolume.Get(AudioThreads::MixThread))
      mVolume.Set(volume, AudioThreads::MixThread);
  }

  return true;
}

//------------------------------------------------------------------------------ Linear Grain Window

//**************************************************************************************************
LinearGrainWindow::LinearGrainWindow(unsigned length) :
  GrainWindow(length, GranularSynthWindows::Linear),
  mHalfLength(length / 2)
{

}

//**************************************************************************************************
float LinearGrainWindow::GetNextValue()
{
  if (mCounter < mHalfLength)
    return (float)mCounter / (float)mHalfLength;
  else
    return 2.0f * (1.0f - ((float)mCounter / (float)mTotalLength));
}

//**************************************************************************************************
void LinearGrainWindow::Reset(unsigned length, unsigned attack, unsigned release)
{
  mCounter = 0;
  mTotalLength = length;
  mHalfLength = length / 2;
}

//**************************************************************************************************
void LinearGrainWindow::CopySettings(GrainWindow* other)
{
  mCounter = other->mCounter;
  mTotalLength = other->mTotalLength;
  mHalfLength = ((LinearGrainWindow*)other)->mHalfLength;
}

//----------------------------------------------------------------------- Raised Cosine Grain Window

//**************************************************************************************************
RaisedCosineGrainWindow::RaisedCosineGrainWindow(unsigned length, unsigned attackLength,
  unsigned releaseLength) :
  GrainWindow(length, GranularSynthWindows::RaisedCosine)
{
  Reset(length, attackLength, releaseLength);
}

//**************************************************************************************************
float RaisedCosineGrainWindow::GetNextValue()
{
  ++mCounter;
  float result;

  switch (mCurrentState)
  {
  case Attack:
    result = b1 * y1 - y2;
    y2 = y1;
    y1 = result;

    result += 1.0f;
    result *= 0.5f;

    if (mCounter == mAttackLength)
      mCurrentState = Sustain;

    break;
  case Sustain:
    result = 1.0f;

    if (mCounter == mTotalLength - mReleaseLength)
    {
      mCurrentState = Release;

      float w = Math::cPi / mReleaseLength;
      float ip = Math::cPi * 0.5f;
      b1 = 2.0f * Math::Cos(w);
      y1 = Math::Sin(ip - w);
      y2 = Math::Sin(ip - 2.0f * w);
    }

    break;
  case Release:
    if (mCounter >= mTotalLength)
      return 0.0f;

    result = b1 * y1 - y2;
    y2 = y1;
    y1 = result;

    result += 1.0f;
    result *= 0.5f;

    break;
  }

  return result;
}

//**************************************************************************************************
void RaisedCosineGrainWindow::Reset(unsigned length, unsigned attack, unsigned release)
{
  mCounter = 0;
  mCurrentState = Attack;
  mTotalLength = length;
  mAttackLength = attack;
  mReleaseLength = release;

  if (mAttackLength + mReleaseLength > mTotalLength)
  {
    float adjustment = (float)mTotalLength / (float)(mAttackLength + mReleaseLength);
    mAttackLength = (unsigned)(mAttackLength * adjustment);
    mReleaseLength = (unsigned)(mReleaseLength * adjustment);
  }

  float w = Math::cPi / mAttackLength;
  float ip = -Math::cPi * 0.5f;
  b1 = 2.0f * Math::Cos(w);
  y1 = Math::Sin(ip - w);
  y2 = Math::Sin(ip - 2.0f * w);
}

//**************************************************************************************************
void RaisedCosineGrainWindow::CopySettings(GrainWindow* other)
{
  RaisedCosineGrainWindow* otherWindow = (RaisedCosineGrainWindow*)other;

  mCounter = otherWindow->mCounter;
  mTotalLength = otherWindow->mTotalLength;
  mCurrentState = otherWindow->mCurrentState;
  mAttackLength = otherWindow->mAttackLength;
  mReleaseLength = otherWindow->mReleaseLength;
  b1 = otherWindow->b1;
  y1 = otherWindow->y1;
  y2 = otherWindow->y2;
}

//--------------------------------------------------------------------------------- Parabolic Window

//**************************************************************************************************
ParabolicGrainWindow::ParabolicGrainWindow(unsigned length) :
  GrainWindow(length, GranularSynthWindows::Parabolic),
  mLastAmplitude(0.0f),
  mSlope(0.0f),
  mCurve(0.0f)
{
  Reset(length, 0, 0);
}

//**************************************************************************************************
float ParabolicGrainWindow::GetNextValue()
{
  ++mCounter;

  mLastAmplitude = mLastAmplitude + mSlope;
  mSlope = mSlope + mCurve;

  float returnValue = Math::Min(mLastAmplitude, 1.0f);
  returnValue = Math::Max(returnValue, 0.0f);

  return mLastAmplitude;
}

//**************************************************************************************************
void ParabolicGrainWindow::Reset(unsigned length, unsigned attack, unsigned release)
{
  mLastAmplitude = 0.0f;
  float rdur = 1.0f / length;
  float rdurSq = rdur * rdur;
  mSlope = 4.0f * (rdur - rdurSq);
  mCurve = -8.0f * rdurSq;
}

//**************************************************************************************************
void ParabolicGrainWindow::CopySettings(GrainWindow* other)
{
  ParabolicGrainWindow* otherWindow = (ParabolicGrainWindow*)other;

  mCounter = otherWindow->mCounter;
  mTotalLength = otherWindow->mTotalLength;
  mLastAmplitude = otherWindow->mLastAmplitude;
  mSlope = otherWindow->mSlope;
  mCurve = otherWindow->mCurve;
}

//--------------------------------------------------------------------------------- Trapezoid Window

//**************************************************************************************************
TrapezoidGrainWindow::TrapezoidGrainWindow(unsigned length, unsigned attack, unsigned release) :
  GrainWindow(length, GranularSynthWindows::Trapezoid)
{
  Reset(length, attack, release);
}

//**************************************************************************************************
float TrapezoidGrainWindow::GetNextValue()
{
  ++mCounter;

  switch (mCurrentState)
  {
  case Attack:
    if (mCounter >= mAttackLength)
    {
      mCurrentState = Sustain;
      mLastAmplitude = 1.0f;
      mIncrement = 0.0f;
    }
    break;
  case Sustain:
    if (mCounter >= mTotalLength - mReleaseLength)
    {
      mCurrentState = Release;
      mIncrement = -1.0f / mReleaseLength;
    }
    break;
  case Release:
    break;
  }

  mLastAmplitude += mIncrement;
  return mLastAmplitude;
}

//**************************************************************************************************
void TrapezoidGrainWindow::Reset(unsigned length, unsigned attack, unsigned release)
{
  mCounter = 0;
  mTotalLength = length;
  mLastAmplitude = 0.0f;

  mAttackLength = attack;
  mReleaseLength = release;

  if (mAttackLength + mReleaseLength > mTotalLength)
  {
    float adjustment = (float)mTotalLength / (float)(mAttackLength + mReleaseLength);
    mAttackLength = (unsigned)(mAttackLength * adjustment);
    mReleaseLength = (unsigned)(mReleaseLength * adjustment);
  }

  mIncrement = 1.0f / mAttackLength;
  mCurrentState = Attack;
}

//**************************************************************************************************
void TrapezoidGrainWindow::CopySettings(GrainWindow* other)
{
  TrapezoidGrainWindow* otherWindow = (TrapezoidGrainWindow*)other;

  mCounter = otherWindow->mCounter;
  mTotalLength = otherWindow->mTotalLength;
  mAttackLength = otherWindow->mAttackLength;
  mReleaseLength = otherWindow->mReleaseLength;
  mLastAmplitude = otherWindow->mLastAmplitude;
  mIncrement = otherWindow->mIncrement;
  mCurrentState = otherWindow->mCurrentState;
}

//-------------------------------------------------------------------------------------------- Grain

//**************************************************************************************************
Grain::Grain() :
  mActive(false),
  mCounter(0),
  mLength(0),
  mCurrentFrameIndex(0.0),
  mIndexIncrement(1.0f),
  mLeftVolume(1.0f),
  mRightVolume(1.0f),
  mVolume(1.0f),
  mSourceSamples(nullptr),
  mSourceChannels(0),
  mWindow(nullptr)
{

}

//**************************************************************************************************
void Grain::Activate(unsigned length, float volume, float panning, BufferType* sampleBuffer,
  unsigned channels, unsigned currentIndex, float indexIncrement, GranularSynthWindows::Enum windowType,
  unsigned windowAttack, unsigned windowRelease)
{
  mActive = true;
  mCounter = 0;
  mLength = length;
  mCurrentFrameIndex = currentIndex / channels;
  mIndexIncrement = indexIncrement;
  if (mIndexIncrement == 0.0f)
    mIndexIncrement = 0.001f;
  mSourceSamples = sampleBuffer;
  mSourceChannels = channels;
  mVolume = volume;

  if (panning == 0.0f)
  {
    mLeftVolume = 0.5f;
    mRightVolume = 0.5f;
  }
  else if (panning < 0.0f)
  {
    panning = Math::Max(panning, -1.0f);

    mLeftVolume = 0.5f * -panning + 0.5f;
    mRightVolume = 1.0f - mLeftVolume;
  }
  else
  {
    panning = Math::Min(panning, 1.0f);

    mRightVolume = 0.5f * panning + 0.5f;
    mLeftVolume = 1.0f - mRightVolume;
  }

  if (mWindow)
  {
    if (mWindow->mType == windowType)
      mWindow->Reset(length, windowAttack, windowRelease);
    else
    {
      delete mWindow;
      mWindow = nullptr;
    }
  }

  if (!mWindow)
  {
    if (windowType == GranularSynthWindows::RaisedCosine)
      mWindow = new RaisedCosineGrainWindow(length, windowAttack, windowRelease);
    else if (windowType == GranularSynthWindows::Parabolic)
      mWindow = new ParabolicGrainWindow(length);
    else if (windowType == GranularSynthWindows::Trapezoid)
      mWindow = new TrapezoidGrainWindow(length, windowAttack, windowRelease);
    else
      mWindow = new LinearGrainWindow(length);
  }
}

//**************************************************************************************************
void Grain::GetSamples(float* outputBuffer, unsigned outputFrames, unsigned outputChannels)
{
  if (!mActive)
    return;

  for (unsigned frame = 0; frame < outputFrames && mCounter < mLength; ++frame, ++mCounter)
  {
    float sample = 0.0f;
    unsigned sampleIndex = (int)mCurrentFrameIndex * mSourceChannels;
    for (unsigned i = 0; i < mSourceChannels; ++i)
    {
      if (mIndexIncrement == 1.0f)
        sample += (*mSourceSamples)[sampleIndex];
      else
      {
        float firstSample = (*mSourceSamples)[sampleIndex + i];
        float secondSample = (*mSourceSamples)[sampleIndex + i + mSourceChannels];

        sample += firstSample + ((secondSample - firstSample) * (float)(mCurrentFrameIndex -
          (int)mCurrentFrameIndex));
      }
    }

    sample /= (float)mSourceChannels;

    mCurrentFrameIndex += mIndexIncrement;
    if (mCurrentFrameIndex >= mSourceSamples->Size() - mSourceChannels)
      mCurrentFrameIndex = 0;
    else if (mCurrentFrameIndex < 0)
      mCurrentFrameIndex = mSourceSamples->Size();

    sample *= mWindow->GetNextValue() * mVolume;

    if (outputChannels == 1)
      outputBuffer[frame * outputChannels] += sample;
    else if (outputChannels == 2)
    {
      outputBuffer[frame * outputChannels] += sample * mLeftVolume;
      outputBuffer[frame * outputChannels + 1] += sample * mRightVolume;
    }
    else
    {
      float samples[2] = { sample * mLeftVolume, sample * mRightVolume };
      AudioFrame frameSamples(samples, 2);
      memcpy(outputBuffer + (frame * outputChannels), frameSamples.GetSamples(outputChannels),
        sizeof(float) * outputChannels);
    }
  }

  if (mCounter >= mLength)
    mActive = false;
}

//**************************************************************************************************
Grain& Grain::operator=(const Grain& otherGrain)
{
  if (&otherGrain == this)
    return *this;

  mActive = otherGrain.mActive;
  mCounter = otherGrain.mCounter;
  mLength = otherGrain.mLength;
  mCurrentFrameIndex = otherGrain.mCurrentFrameIndex;
  mIndexIncrement = otherGrain.mIndexIncrement;
  mLeftVolume = otherGrain.mLeftVolume;
  mRightVolume = otherGrain.mRightVolume;
  mSourceSamples = otherGrain.mSourceSamples;
  mSourceChannels = otherGrain.mSourceChannels;
  mWindow->CopySettings(otherGrain.mWindow);

  return *this;
}

//------------------------------------------------------------------------------ Granular Synth Node

//**************************************************************************************************
ZilchDefineType(GranularSynthNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindMethod(SetSound);
  ZilchBindMethod(Play);
  ZilchBindMethod(Stop);
  ZilchBindGetterSetterProperty(GrainVolume);
  ZilchBindGetterSetterProperty(GrainVolumeVariance);
  ZilchBindGetterSetterProperty(GrainDelay);
  ZilchBindGetterSetterProperty(GrainDelayVariance);
  ZilchBindGetterSetterProperty(GrainLength);
  ZilchBindGetterSetterProperty(GrainLengthVariance);
  ZilchBindGetterSetterProperty(GrainResampleRate);
  ZilchBindGetterSetterProperty(GrainResampleRateVariance);
  ZilchBindGetterSetterProperty(BufferScanRate);
  ZilchBindGetterSetterProperty(GrainPanningValue);
  ZilchBindGetterSetterProperty(GrainPanningVariance);
  ZilchBindGetterSetterProperty(RandomLocationValue);
  ZilchBindGetterSetterProperty(WindowType);
  ZilchBindGetterSetterProperty(WindowAttack);
  ZilchBindGetterSetterProperty(WindowRelease);
}

//**************************************************************************************************
GranularSynthNode::GranularSynthNode(StringParam name, unsigned ID) :
  SoundNode(name, ID, false, true),
  mActive(false),
  mSampleChannelsThreaded(0),
  mFirstInactiveGrainIndexThreaded(0),
  mGrainStartIndexThreaded(0),
  mFramesToNextGrainThreaded(0),
  mGrainVolume(1.0f),
  mGrainVolumeVariance(0.0f),
  mGrainDelayFrames(4800),
  mGrainDelayVariance(0),
  mGrainLengthFrames(14400),
  mGrainLengthVariance(0),
  mGrainResampleRate(1.0f),
  mGrainResampleVariance(0.0f),
  mBufferScanRate(1.0f),
  mGrainPanningValue(0.0f),
  mGrainPanningVariance(0.0f),
  mRandomLocationValue(0.0f),
  mWindowType(GranularSynthWindows::Parabolic),
  mWindowAttackFrames(200),
  mWindowReleaseFrames(200)
{
  GrainListThreaded.Resize(16);
}

//**************************************************************************************************
void GranularSynthNode::Play()
{
  mActive.Set(true, AudioThreads::MainThread);
}

//**************************************************************************************************
void GranularSynthNode::Stop()
{
  mActive.Set(false, AudioThreads::MainThread);
}

//**************************************************************************************************
void GranularSynthNode::SetSound(HandleOf<Sound> sound, float startTime, float stopTime)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&GranularSynthNode::SetSoundThreaded, this, sound,
    startTime, stopTime), this);
}

//**************************************************************************************************
float GranularSynthNode::GetGrainVolume()
{
  return mGrainVolume.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void GranularSynthNode::SetGrainVolume(float volume)
{
  mGrainVolume.Set(Math::Clamp(volume, 0.0f, cMaxVolumeValue), AudioThreads::MainThread);

  // TODO this should probably be interpolated
}

//**************************************************************************************************
float GranularSynthNode::GetGrainVolumeVariance()
{
  return mGrainVolumeVariance.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void GranularSynthNode::SetGrainVolumeVariance(float variance)
{
  mGrainVolumeVariance.Set(Math::Clamp(variance, 0.0f, cMaxVolumeValue), AudioThreads::MainThread);
}

//**************************************************************************************************
int GranularSynthNode::GetGrainDelay()
{
  return FramesToMs(mGrainDelayFrames.Get(AudioThreads::MainThread));
}

//**************************************************************************************************
void GranularSynthNode::SetGrainDelay(int delayMS)
{
  mGrainDelayFrames.Set(MsToFrames(Math::Max(delayMS, 0)), AudioThreads::MainThread);
}

//**************************************************************************************************
int GranularSynthNode::GetGrainDelayVariance()
{
  return FramesToMs(mGrainDelayVariance.Get(AudioThreads::MainThread));
}

//**************************************************************************************************
void GranularSynthNode::SetGrainDelayVariance(int delayVarianceMS)
{
  mGrainDelayVariance.Set(MsToFrames(Math::Max(delayVarianceMS, 0)), AudioThreads::MainThread);
}

//**************************************************************************************************
int GranularSynthNode::GetGrainLength()
{
  return FramesToMs(mGrainLengthFrames.Get(AudioThreads::MainThread));
}

//**************************************************************************************************
void GranularSynthNode::SetGrainLength(int lengthMS)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&GranularSynthNode::SetGrainLengthThreaded, this,
    lengthMS), this);
}

//**************************************************************************************************
int GranularSynthNode::GetGrainLengthVariance()
{
  return FramesToMs(mGrainLengthVariance.Get(AudioThreads::MainThread));
}

//**************************************************************************************************
void GranularSynthNode::SetGrainLengthVariance(int lengthVarianceMS)
{
  mGrainLengthVariance.Set(MsToFrames(Math::Max(lengthVarianceMS, 0)), AudioThreads::MainThread);
}

//**************************************************************************************************
float GranularSynthNode::GetGrainResampleRate()
{
  return mGrainResampleRate.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void GranularSynthNode::SetGrainResampleRate(float resampleRate)
{
  mGrainResampleRate.Set(Math::Clamp(resampleRate, -mMaxResampleValue, mMaxResampleValue),
    AudioThreads::MainThread);
}

//**************************************************************************************************
float GranularSynthNode::GetGrainResampleRateVariance()
{
  return mGrainResampleVariance.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void GranularSynthNode::SetGrainResampleRateVariance(float resampleVariance)
{
  mGrainResampleVariance.Set(Math::Clamp(resampleVariance, 0.0f, mMaxResampleValue),
    AudioThreads::MainThread);
}

//**************************************************************************************************
float GranularSynthNode::GetBufferScanRate()
{
  return mBufferScanRate.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void GranularSynthNode::SetBufferScanRate(float bufferRate)
{
  mBufferScanRate.Set(Math::Clamp(bufferRate, -mMaxResampleValue, mMaxResampleValue),
    AudioThreads::MainThread);
}

//**************************************************************************************************
float GranularSynthNode::GetGrainPanningValue()
{
  return mGrainPanningValue.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void GranularSynthNode::SetGrainPanningValue(float panValue)
{
  mGrainPanningValue.Set(Math::Clamp(panValue, -1.0f, 1.0f), AudioThreads::MainThread);
}

//**************************************************************************************************
float GranularSynthNode::GetGrainPanningVariance()
{
  return mGrainPanningVariance.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void GranularSynthNode::SetGrainPanningVariance(float panValueVariance)
{
  mGrainPanningVariance.Set(Math::Clamp(panValueVariance, 0.0f, 1.0f), AudioThreads::MainThread);
}

//**************************************************************************************************
float GranularSynthNode::GetRandomLocationValue()
{
  return mRandomLocationValue.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void GranularSynthNode::SetRandomLocationValue(float randomLocationValue)
{
  mRandomLocationValue.Set(Math::Clamp(randomLocationValue, 0.0f, 1.0f), AudioThreads::MainThread);
}

//**************************************************************************************************
GranularSynthWindows::Enum GranularSynthNode::GetWindowType()
{
  return mWindowType.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void GranularSynthNode::SetWindowType(GranularSynthWindows::Enum type)
{
  mWindowType.Set(type, AudioThreads::MainThread);
}

//**************************************************************************************************
int GranularSynthNode::GetWindowAttack()
{
  return FramesToMs(mWindowAttackFrames.Get(AudioThreads::MainThread));
}

//**************************************************************************************************
void GranularSynthNode::SetWindowAttack(int attackMS)
{
  mWindowAttackFrames.Set(MsToFrames(Math::Max(attackMS, 0)), AudioThreads::MainThread);
}

//**************************************************************************************************
int GranularSynthNode::GetWindowRelease()
{
  return FramesToMs(mWindowReleaseFrames.Get(AudioThreads::MainThread));
}

//**************************************************************************************************
void GranularSynthNode::SetWindowRelease(int releaseMS)
{
  mWindowReleaseFrames.Set(MsToFrames(Math::Max(releaseMS, 0)), AudioThreads::MainThread);
}

//**************************************************************************************************
bool GranularSynthNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
  ListenerNode* listener, const bool firstRequest)
{
  if (SamplesThreaded.Empty())
    return false;

  if (!mActive.Get(AudioThreads::MixThread) && mFirstInactiveGrainIndexThreaded == 0)
    return false;

  memset(outputBuffer->Data(), 0, outputBuffer->Size() * sizeof(float));
  unsigned frames = outputBuffer->Size() / numberOfChannels;

  // Process existing grains
  for (int i = 0; i < mFirstInactiveGrainIndexThreaded; ++i)
  {
    GrainListThreaded[i].GetSamples(outputBuffer->Data(), frames, numberOfChannels);

    if (!GrainListThreaded[i].mActive)
    {
      --mFirstInactiveGrainIndexThreaded;
      GrainListThreaded[i] = GrainListThreaded[mFirstInactiveGrainIndexThreaded];
      GrainListThreaded[mFirstInactiveGrainIndexThreaded].mActive = false;
      --i;
    }
  }

  if (!mActive.Get(AudioThreads::MixThread))
    return true;

  // Add new grains
  unsigned currentIndex = mFramesToNextGrainThreaded * mSampleChannelsThreaded;
  while (currentIndex < outputBuffer->Size())
  {
    // Add another grain to the list if necessary
    if (mFirstInactiveGrainIndexThreaded == GrainListThreaded.Size())
      GrainListThreaded.PushBack();

    Grain& newGrain = GrainListThreaded[mFirstInactiveGrainIndexThreaded++];

    int grainIndex;
    // If randomizing grain position, get a random starting index
    if (mRandomLocationValue.Get(AudioThreads::MixThread) > 0.0f
      && RandomObjectThreaded.Float() < mRandomLocationValue.Get(AudioThreads::MixThread))
    {
      grainIndex = RandomObjectThreaded.IntRangeInEx(0, SamplesThreaded.Size());
      grainIndex -= grainIndex % mSampleChannelsThreaded;
    }
    // Otherwise use the sequential grain start index
    else
    {
      grainIndex = mGrainStartIndexThreaded + currentIndex;

      if (grainIndex < 0)
        grainIndex += SamplesThreaded.Size();
      else if (grainIndex >= (int)SamplesThreaded.Size())
        grainIndex -= SamplesThreaded.Size();
    }

    // Activate the new grain
    newGrain.Activate(
      GetValueThreaded(mGrainLengthFrames.Get(AudioThreads::MixThread), mGrainLengthVariance.Get(AudioThreads::MixThread)),
      GetValueThreaded(mGrainVolume.Get(AudioThreads::MixThread), mGrainVolumeVariance.Get(AudioThreads::MixThread)),
      GetValueThreaded(mGrainPanningValue.Get(AudioThreads::MixThread), mGrainPanningVariance.Get(AudioThreads::MixThread)),
      &SamplesThreaded,
      mSampleChannelsThreaded,
      grainIndex,
      GetValueThreaded(mGrainResampleRate.Get(AudioThreads::MixThread), mGrainResampleVariance.Get(AudioThreads::MixThread)),
      mWindowType.Get(AudioThreads::MixThread),
      mWindowAttackFrames.Get(AudioThreads::MixThread),
      mWindowReleaseFrames.Get(AudioThreads::MixThread));

    int nextDelayFrames = mGrainDelayFrames.Get(AudioThreads::MixThread);
    if (mGrainDelayVariance.Get(AudioThreads::MixThread) > 0)
      nextDelayFrames = RandomObjectThreaded.IntVariance(mGrainDelayFrames.Get(AudioThreads::MixThread),
        mGrainDelayVariance.Get(AudioThreads::MixThread));
    if (mBufferScanRate.Get(AudioThreads::MixThread) != 0.0f)
      nextDelayFrames = (unsigned)(nextDelayFrames * mBufferScanRate.Get(AudioThreads::MixThread));

    mFramesToNextGrainThreaded += nextDelayFrames;
    currentIndex += nextDelayFrames * mSampleChannelsThreaded;
  }

  if (mBufferScanRate.Get(AudioThreads::MixThread) != 0.0f)
    mFramesToNextGrainThreaded -= (int)(frames * mBufferScanRate.Get(AudioThreads::MixThread));
  else
    mFramesToNextGrainThreaded -= frames;

  mGrainStartIndexThreaded += (unsigned)(frames * mBufferScanRate.Get(AudioThreads::MixThread))
    * mSampleChannelsThreaded;
  if (mGrainStartIndexThreaded >= (int)SamplesThreaded.Size())
    mGrainStartIndexThreaded -= SamplesThreaded.Size();
  else if (mGrainStartIndexThreaded < 0)
    mGrainStartIndexThreaded += SamplesThreaded.Size();

  return true;
}

//**************************************************************************************************
void GranularSynthNode::ValidateLengthsThreaded()
{
  if (mGrainLengthFrames.Get(AudioThreads::MixThread) * mSampleChannelsThreaded >= SamplesThreaded.Size())
  {
    mGrainLengthFrames.Set(SamplesThreaded.Size() - 1, AudioThreads::MixThread);
    mGrainLengthVariance.Set(0, AudioThreads::MixThread);
  }
  else if ((mGrainLengthFrames.Get(AudioThreads::MixThread) + mGrainLengthVariance.Get(AudioThreads::MixThread))
    * mSampleChannelsThreaded >= SamplesThreaded.Size())
  {
    mGrainLengthVariance.Set((SamplesThreaded.Size() / mSampleChannelsThreaded)
      - mGrainLengthFrames.Get(AudioThreads::MixThread), AudioThreads::MixThread);
  }
}

//**************************************************************************************************
unsigned GranularSynthNode::GetValueThreaded(unsigned base, unsigned variance)
{
  if (variance != 0)
    return RandomObjectThreaded.IntVariance(base, variance);
  else
    return base;
}

//**************************************************************************************************
float GranularSynthNode::GetValueThreaded(float base, float variance)
{
  if (variance != 0.0f)
    return RandomObjectThreaded.FloatVariance(base, variance);
  else
    return base;
}

//**************************************************************************************************
void GranularSynthNode::SetSoundThreaded(HandleOf<Sound> sound, float startTime, float stopTime)
{
  SoundAsset* asset = sound->mAsset;
  if (!asset)
    return;

  // Get the number of channels from the asset
  mSampleChannelsThreaded = asset->mChannels;

  // Translate the start and stop times to frame numbers
  int start = Math::Max((int)(startTime * cSystemSampleRate), 0);
  int stop = Math::Clamp((int)(stopTime * cSystemSampleRate), 0, (int)asset->mFrameCount);
  if (stop == 0)
    stop = (int)asset->mFrameCount;

  // Get the audio samples from the asset
  SamplesThreaded.Clear();
  asset->AppendSamplesThreaded(&SamplesThreaded, start, (stop - start) * mSampleChannelsThreaded, cNodeID);

  ValidateLengthsThreaded();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainLengthThreaded(int lengthMS)
{
  mGrainLengthFrames.Set(MsToFrames(Math::Max(lengthMS, 0)), AudioThreads::MixThread);
  ValidateLengthsThreaded();
}

//**************************************************************************************************
unsigned GranularSynthNode::MsToFrames(int msValue)
{
  return (unsigned)(msValue * 0.001f * cSystemSampleRate);
}

//**************************************************************************************************
int GranularSynthNode::FramesToMs(unsigned frames)
{
  return (int)(frames / cSystemSampleRate * 1000.0f);
}

} // namespace Zero
