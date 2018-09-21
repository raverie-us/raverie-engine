///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

typedef Array<float> BufferType;
typedef Array<float>::range BufferRange;

namespace AudioThreads
{

enum Enum { MainThread = 0, MixThread };

} // namespace AudioThreads

void LogAudioIoError(StringParam message, String* savedMessage = nullptr);

namespace AudioConstants
{

// The sample rate used by the audio engine for the output mix
const unsigned cSystemSampleRate = 48000;
// The time increment per audio frame corresponding to the sample rate
const double cSystemTimeIncrement = 1.0 / 48000.0;
// The number of frames used to interpolate instant property changes
const unsigned  cPropertyChangeFrames = (unsigned)(48000 * 0.01f);
// Maximum number of channels in audio output
const unsigned cMaxChannels = 8;
// Volume modifier applied to all generated waves
const float cGeneratedWaveVolume = 0.5f;

const float cMaxVolumeValue = 50.0f;
const float cMaxDecibelsValue = 20.0f;
const float cMinDecibelsValue = -100.0f;
const float cMinPitchValue = -5.0f;
const float cMaxPitchValue = 5.0f;
const float cMinSemitonesValue = -100.0f;
const float cMaxSemitonesValue = 100.0f;

const int cTrue = 1;
const int cFalse = 0;

bool IsWithinLimit(float valueToCheck, float centralValue, float limit);

void AppendToBuffer(BufferType* destinationBuffer, const BufferType& sourceBuffer,
  unsigned sourceStartIndex, unsigned numberOfSamples);

float PitchToSemitones(float pitch);

float SemitonesToPitch(float semitone);

float VolumeToDecibels(float volume);

float DecibelsToVolume(float decibels);

} // namespace AudioConstants

//**************************************************************************************************
template <typename T>
class Threaded
{
public:
  Threaded() {}
  Threaded(T value) { mValues[0] = value; mValues[1] = value; }

  T Get(AudioThreads::Enum whichThread) { return mValues[whichThread]; }

  void Set(T value, AudioThreads::Enum threadCalledOn)
  {
    mValues[threadCalledOn] = value;

    if (threadCalledOn == AudioThreads::MainThread)
      Z::gSound->Mixer.AddTask(CreateFunctor(&mValues[AudioThreads::MixThread], value), nullptr);
    else
      Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&mValues[AudioThreads::MainThread], value), nullptr);
  }

  void SetDirectly(T value) { mValues[0] = value; mValues[1] = value; }

private:
  T mValues[2];

  T& operator=(const T& lhs) {}
};

//**************************************************************************************************
class ThreadedInt
{
public:
  ThreadedInt() {}
  ThreadedInt(const int& value) : mValue(value) {}

  int Get() { return mValue; }

  void Set(int value) { AtomicExchange(&mValue, value); }

private:
  volatile int mValue;
};

}
