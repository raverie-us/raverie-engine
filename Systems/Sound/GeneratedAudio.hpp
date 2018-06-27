///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

//------------------------------------------------------------------------------ Generated Wave Node

/// Plays audio using the specified type of generated wave
class GeneratedWaveNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  GeneratedWaveNode(StringParam name, unsigned ID);

  /// The type of sound wave used to generate the audio. 
  /// Possible types are Sine, Square, Saw, Triangle, and Noise (randomly generated white noise).
  SynthWaveType::Enum GetWaveType();
  void SetWaveType(SynthWaveType::Enum type);
  /// The frequency of the generated sound wave. This value will have no effect if the Noise type is chosen.
  float GetWaveFrequency();
  void SetWaveFrequency(float frequency);
  /// Interpolates the WaveFrequency property from its current value to the value passed in 
  /// as the first parameter, over the number of seconds passed in as the second parameter. 
  /// This method will have no effect if the Noise type is chosen.
  void InterpolateWaveFrequency(float frequency, float time);
  /// Starts playing the generated audio.
  void Play();
  /// Stops playing the generated audio.
  void Stop();
  /// The volume adjustment that will be applied to the sound when it plays. 
  /// A value of 1 does not affect the sound; 2 will double the sound's volume, 0.5 will halve it, 
  /// and 0 will make the sound inaudible. 
  float GetVolume();
  void SetVolume(float volume);
  /// Interpolates the Volume property from its current value to the value passed in 
  /// as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateVolume(float volume, float interpolationTime);
  /// The volume adjustment, in decibels, that will be applied to the sound when it plays. 
  /// A value of 0 does not affect the sound; 6 will double the sound's volume, -6 will halve it, 
  /// and -100 is effectively the same as a Volume of 0. 
  float GetDecibels();
  void SetDecibels(float decibels);
  /// Interpolates the Decibels property from its current value to the value passed in 
  /// as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateDecibels(float decibels, float interpolationTime);
  /// The percentage of the square wave (from 0 to 1.0) which should be up. This will have no effect
  /// if a different wave type is chosen.
  float GetSquareWavePulseValue();
  void SetSquareWavePulseValue(float value);

private:
  Oscillator WaveDataThreaded;
  SynthWaveType::Enum mWaveType;
  Threaded<float> mWaveFrequency;
  Threaded<float> mVolume;
  float mSquareWavePulseValue;
  InterpolatingObject FrequencyInterpolatorThreaded;
  InterpolatingObject VolumeInterpolatorThreaded;

  enum States { Starting, Stopping, Active, Off };

  Threaded<States> mState;

  bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest) override;
  void InterpolateFrequencyThreaded(float frequency, float time);
  void InterpolateVolumeThreaded(float volume, float time);
};

//------------------------------------------------------------------------------------ ADSR Envelope

/// Used to control harmonics of notes played by the AdditiveSynthNode
class AdsrEnvelope
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);

  AdsrEnvelope() :
    mDelayTime(0.0f),
    mAttackTime(0.02f),
    mDecayTime(0.0f),
    mSustainTime(1.0f),
    mSustainLevel(1.0f),
    mReleaseTime(0.5f)
  {}

  ///The time, in seconds, between when the note starts and when this harmonic starts playing.
  float mDelayTime;
  /// The time, in seconds, for this harmonic to interpolate its volume 
  /// from 0 to 1.0 when it starts playing.
  float mAttackTime;
  /// The time, in seconds, for this harmonic to interpolate logarithmically 
  /// from 1.0 to the SustainLevel after the attack.
  float mDecayTime;
  /// The time, in seconds, for this harmonic to stay at the SustainLevel after the attack and decay.
  /// A value of 0 will make the harmonic play indefinitely until NoteOff is called.
  float mSustainTime;
  /// The volume level (1.0 is full volume) to use for the sustain period.
  float mSustainLevel;
  /// The time, in seconds, for this harmonic to interpolate logarithmically from the SustainLevel to 0.
  float mReleaseTime;
};

//---------------------------------------------------------------------------------- Harmonic Data

class HarmonicData
{
public:
  HarmonicData(float multiplier, float volume, EnvelopeSettings& envelope, SynthWaveType::Enum type) :
    mFrequencyMultiplier(multiplier),
    mVolume(volume),
    mEnvelope(envelope),
    mWaveType(type)
  {}
  HarmonicData() :
    mFrequencyMultiplier(0.0f),
    mVolume(0.0f)
  {}

  float mFrequencyMultiplier;
  float mVolume;
  EnvelopeSettings mEnvelope;
  SynthWaveType::Enum mWaveType;
};

//------------------------------------------------------------------------------------ Note Harmonic

class NoteHarmonic
{
public:
  NoteHarmonic() :
    mVolume(1.0f)
  {}

  void SetValues(float frequency, float volume, EnvelopeSettings& envelope, SynthWaveType::Enum waveType);
  float operator()();
  bool IsFinished();
  void Stop();

private:
  Oscillator WaveSamples;
  ADSR Envelope;
  float mVolume;
};

//------------------------------------------------------------------------------------- AdditiveNote

// used by AdditiveSynthNode
class AdditiveNote
{
public:

  void AddHarmonic(float frequency, float volume, EnvelopeSettings& envelope, SynthWaveType::Enum waveType);
  float operator()();
  bool IsFinished();
  void Stop();

  float mVolume;

private:
  typedef Zero::Array<NoteHarmonic> HarmonicsListType;
  HarmonicsListType Harmonics;
};

//------------------------------------------------------------------------------ Additive Synth Node

/// Generates audio using additive synthesis 
class AdditiveSynthNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  AdditiveSynthNode(StringParam name, unsigned ID);
  ~AdditiveSynthNode();

  /// Adds a new harmonic to the additive synth notes. The first value is the multiplier that will
  /// be applied to the base frequency, the second is the volume of this harmonic, and the third
  /// (the AdsrEnvelope object) contains the envelope-related values.
  void AddHarmonic(float frequencyMultiplier, float volume, AdsrEnvelope envelope, SynthWaveType::Enum type);
  /// Removes all current harmonics. You must add at least one harmonic before playing a note.
  void RemoveAllHarmonics();
  /// Starts playing a new note. The first parameter is the MIDI note value (range is 0 to 127),
  /// and the second is the volume modification that should be applied to this note (a value
  /// of 1.0 does not change the volume, while 0.0 would be silence). 
  void NoteOn(float midiNote, float volume);
  /// Stops playing all current notes at the specified MIDI value.
  void NoteOff(float midiNote);
  /// Stops playing all current notes.
  void StopAllNotes();

private:
  bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest) override;
  void AddHarmonicThreaded(HarmonicData data);
  void NoteOnThreaded(int midiNote, float volume);
  void NoteOffThreaded(int midiNote);
  void StopAllNotesThreaded();

  typedef Array<HarmonicData> HarmonicDataListType;
  typedef Array<AdditiveNote*> NotesListType;
  typedef HashMap<int, NotesListType*> NotesMapType;

  // The number of notes currently playing
  int mCurrentNoteCountThreaded;
  // The harmonics to use for the notes
  HarmonicDataListType HarmonicsListThreaded;
  // A map of MIDI note values currently playing to a list of note objects
  NotesMapType CurrentNotesMapThreaded;
};

//---------------------------------------------------------------------------- Microphone Input Node

/// Receives input from a microphone and passes the audio data to its output SoundNodes
class MicrophoneInputNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MicrophoneInputNode(StringParam name, unsigned ID);

  /// The volume modifier applied to all audio data received from the microphone.
  float GetVolume();
  void SetVolume(float volume);
  /// Microphone input will only be played while the Active property is set to True.
  bool GetActive();
  void SetActive(bool active);

private:
  bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest) override;

  Threaded<bool> mActive;
  Threaded<float> mVolume;
  InterpolatingObject VolumeInterpolatorThreaded;
  ThreadedInt mStopping;
};

/// Types of windows (volume envelopes) that can be used for individual grains generated 
/// by the GranularSynthNode.
/// <param name="Linear">A triangle-shaped linear envelope. Does not use attack and release times.</param>
/// <param name="Parabolic">A smoothly curved envelope. Does not use attack and release times.</param>
/// <param name="RaisedCosine">Uses cosine curves to smoothly ramp up and down during attack and release times.</param>
/// <param name="Trapezoid">Uses linear ramps during attack and release times. More efficient than RaisedCosine but not as smooth.</param>
DeclareEnum4(GranularSynthWindows, Linear, Parabolic, RaisedCosine, Trapezoid);

//----------------------------------------------------------------------------------- Grain Window

class GrainWindow
{
public:
  GrainWindow(unsigned length, GranularSynthWindows::Enum type) :
    mTotalLength(length),
    mCounter(0),
    mType(type)
  {}
  GrainWindow(GrainWindow& other) { CopySettings(&other); }

  virtual float GetNextValue() = 0;
  virtual void Reset(unsigned length, unsigned attack, unsigned release) = 0;
  virtual void CopySettings(GrainWindow* other) = 0;

  unsigned mTotalLength;
  unsigned mCounter;
  GranularSynthWindows::Enum mType;
};

//---------------------------------------------------------------------------- Linear Grain Window

class LinearGrainWindow : public GrainWindow
{
public:
  LinearGrainWindow(unsigned length);

  float GetNextValue() override;
  void Reset(unsigned length, unsigned attack, unsigned release) override;
  void CopySettings(GrainWindow* other) override;

  unsigned mHalfLength;
};

//--------------------------------------------------------------------- Raised Cosine Grain Window

class RaisedCosineGrainWindow : public GrainWindow
{
public:
  RaisedCosineGrainWindow(unsigned length, unsigned attackLength, unsigned releaseLength);

  float GetNextValue() override;
  void Reset(unsigned length, unsigned attack, unsigned release) override;
  void CopySettings(GrainWindow* other) override;

  enum State { Attack, Sustain, Release };

  unsigned mAttackLength;
  unsigned mReleaseLength;
  State mCurrentState;
  float b1;
  float y1;
  float y2;
};

//------------------------------------------------------------------------------- Parabolic Window

class ParabolicGrainWindow : public GrainWindow
{
public:
  ParabolicGrainWindow(unsigned length);

  float GetNextValue() override;
  void Reset(unsigned length, unsigned attack, unsigned release) override;
  void CopySettings(GrainWindow* other) override;

  float mLastAmplitude;
  float mSlope;
  float mCurve;
};

//------------------------------------------------------------------------------- Trapezoid Window

class TrapezoidGrainWindow : public GrainWindow
{
public:
  TrapezoidGrainWindow(unsigned length, unsigned attack, unsigned release);

  float GetNextValue() override;
  void Reset(unsigned length, unsigned attack, unsigned release) override;
  void CopySettings(GrainWindow* other) override;

  enum State { Attack, Sustain, Release };

  unsigned mAttackLength;
  unsigned mReleaseLength;
  float mLastAmplitude;
  float mIncrement;
  State mCurrentState;
};

//------------------------------------------------------------------------------------------ Grain

class Grain
{
public:
  Grain();

  Grain& operator=(const Grain& otherGrain);
  void Activate(unsigned length, float volume, float panning, BufferType* sampleBuffer,
    unsigned channels, unsigned currentIndex, float indexIncrement, GranularSynthWindows::Enum windowType,
    unsigned windowAttack, unsigned windowRelease);
  void GetSamples(float* outputBuffer, unsigned outputFrames, unsigned outputChannels);

  bool mActive;
  unsigned mCounter;
  unsigned mLength;
  double mCurrentFrameIndex;
  float mIndexIncrement;
  float mLeftVolume;
  float mRightVolume;
  float mVolume;
  BufferType* mSourceSamples;
  unsigned mSourceChannels;
  GrainWindow* mWindow;
};

//------------------------------------------------------------------------------ Granular Synth Node

class GranularSynthNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  GranularSynthNode(StringParam name, unsigned ID);

  /// Starts playing new grains.
  void Play();
  /// Stops playing new grains but continues to play current ones.
  void Stop();
  /// Sets the Sound resource that will be used for the grains, along with an optional start and 
  /// stop time. If the stopTime is 0.0, all audio from the Sound will be used.
  void SetSound(HandleOf<Sound> sound, float startTime, float stopTime);
  /// The volume modifier applied to the grains.
  float GetGrainVolume();
  void SetGrainVolume(float volume);
  /// The variance for randomizing the grain volume.
  float GetGrainVolumeVariance();
  void SetGrainVolumeVariance(float variance);
  /// The number of milliseconds to wait before playing another grain.
  int GetGrainDelay();
  void SetGrainDelay(int delayMS);
  /// The variance for randomizing the grain delay, in milliseconds. 
  int GetGrainDelayVariance();
  void SetGrainDelayVariance(int delayVarianceMS);
  /// The length of a grain, in milliseconds.
  int GetGrainLength();
  void SetGrainLength(int lengthMS);
  /// The variance for randomizing the grain length, in milliseconds. 
  int GetGrainLengthVariance();
  void SetGrainLengthVariance(int lengthVarianceMS);
  /// The rate at which grains resample their audio data. A value of 1.0 will play normally,
  /// 0.5 will play at half speed, and -1.0 will play at normal speed backward. Cannot be 0.
  float GetGrainResampleRate();
  void SetGrainResampleRate(float resampleRate);
  /// The variance for randomizing the grain resample rate.
  float GetGrainResampleRateVariance();
  void SetGrainResampleRateVariance(float resampleVariance);
  /// The rate at which the synthesizer scans the buffer as it creates grains. A value of 1.0
  /// will move through the audio data at the same rate as it would normally be played, 0.5 will
  /// move at half speed, and -1.0 will move at normal speed backward. A value of 0.0 will make
  /// the synthesizer repeat the same audio continuously.
  float GetBufferScanRate();
  void SetBufferScanRate(float bufferRate);
  /// The value used to pan the grains left or right. A value of 0 will be heard equally from 
  /// the left and right, 1.0 will be heard only on the right, and -1.0 will be only left.
  float GetGrainPanningValue();
  void SetGrainPanningValue(float panValue);
  /// The variance for randomizing the grain panning value.
  float GetGrainPanningVariance();
  void SetGrainPanningVariance(float panValueVariance);
  /// The value for controlling how many grains have randomized starting positions in the audio.
  /// A value of 0 will be completely sequential, while 1.0 will be completely random. 
  float GetRandomLocationValue();
  void SetRandomLocationValue(float randomLocationValue);
  /// The type of window, or volume envelope, used for each grain.
  GranularSynthWindows::Enum GetWindowType();
  void SetWindowType(GranularSynthWindows::Enum type);
  /// The window attack time, in milliseconds. Does not have an effect on some windows.
  int GetWindowAttack();
  void SetWindowAttack(int attackMS);
  /// The window release time, in milliseconds. Does not have an effect on some windows.
  int GetWindowRelease();
  void SetWindowRelease(int releaseMS);

private:
  bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest) override;
  void ValidateLengthsThreaded();
  unsigned GetValueThreaded(unsigned base, unsigned variance);
  float GetValueThreaded(float base, float variance);
  void SetSoundThreaded(HandleOf<Sound> asset, float startTime, float stopTime);
  void SetGrainLengthThreaded(int lengthMS);
  unsigned MsToFrames(int msValue);
  int FramesToMs(unsigned frames);

  // Only spawns new grains when this is true
  Threaded<bool> mActive;
  // Audio samples for grains to use
  BufferType SamplesThreaded;
  // Number of audio channels in the samples
  unsigned mSampleChannelsThreaded;
  // List of grains
  Zero::Array<Grain> GrainListThreaded;
  // Index of the first inactive grain in the list
  int mFirstInactiveGrainIndexThreaded;
  // Index of the audio sample where the grain should start playing
  int mGrainStartIndexThreaded;
  // Number of frames until the next grain should start
  int mFramesToNextGrainThreaded;
  // Base grain volume
  Threaded<float> mGrainVolume;
  // Grain volume variance
  Threaded<float> mGrainVolumeVariance;
  // Base grain delay value
  Threaded<unsigned> mGrainDelayFrames;
  // Grain delay variance
  Threaded<unsigned> mGrainDelayVariance;
  // Base grain length value
  Threaded<unsigned> mGrainLengthFrames;
  // Grain length variance
  Threaded<unsigned> mGrainLengthVariance;
  // Base grain resample rate
  Threaded<float> mGrainResampleRate;
  // Grain resample rate variance
  Threaded<float> mGrainResampleVariance;
  // Buffer scan rate
  Threaded<float> mBufferScanRate;
  // Base grain panning value
  Threaded<float> mGrainPanningValue;
  // Grain panning variance
  Threaded<float> mGrainPanningVariance;
  // Value for controlling grain location randomization
  Threaded<float> mRandomLocationValue;
  // Window type to use
  Threaded<GranularSynthWindows::Enum> mWindowType;
  // Window attack time
  Threaded<unsigned> mWindowAttackFrames;
  // Window release time
  Threaded<unsigned> mWindowReleaseFrames;

  Math::Random RandomObjectThreaded;

  const float mMaxResampleValue = 100.0f;
};

} // namespace Zero
