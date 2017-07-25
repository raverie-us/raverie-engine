///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(NeedMoreSamples);
  DeclareEvent(AudioInterpolationDone);
  DeclareEvent(SoundNodeDisconnected);
}

//--------------------------------------------------------------------------------------- Sound Node

class SoundNode : public ReferenceCountedEventObject, public Audio::ExternalNodeInterface
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SoundNode();
  virtual ~SoundNode();

  /// Adds the passed in node to this node's inputs.
  void AddInputNode(SoundNode* node);
  /// Inserts the passed in node after this node in the signal path, placing it 
  /// between this node and any nodes which were connected to this node's output.
  void InsertNodeAfter(SoundNode* node);
  /// Inserts the passed in node before this node in the signal path, placing it 
  /// between this node and any nodes which were connected to this node as inputs.
  void InsertNodeBefore(SoundNode* node);
  /// Replaces this node in the graph with the node passed in as a parameter. 
  /// This node will be deleted when it is no longer referenced. 
  void ReplaceWith(SoundNode* node);
  /// Removes the node passed in as a parameter from this node's inputs. 
  void RemoveInputNode(SoundNode* node);
  /// Removes the connections between this node and all of its input nodes. 
  void RemoveAllInputs();
  /// Removes the connections between this node and all of its output nodes, disconnecting this node from the graph. 
  /// If this node has no inputs it will be deleted when no longer referenced.
  void RemoveAllOutputs();
  /// Removes this node from the graph by disconnecting it from all inputs and outputs 
  /// and attaching the input nodes to the output nodes, keeping the rest of the graph intact. 
  /// This node will be deleted when it is no longer referenced.
  void RemoveAndAttachInputsToOutputs();
  /// If true, this node will automatically remove itself from the graph when its last input node is removed.
  bool GetAutoCollapse();
  void SetAutoCollapse(bool willCollapse);
  /// Will be true if this node has any input nodes. 
  bool GetHasInputs();
  /// Will be true if this node has any output nodes.
  bool GetHasOutputs();
  /// The number of input nodes that are currently attached to this node.
  int GetInputCount();
  /// The number of output nodes that are currently attached to this node.
  int GetOutputCount();
  /// The percentage of output (0 to 100) that should skip whatever processing the node does.
  float GetBypassPercent();
  void SetBypassPercent(float percent);

// Internals
  Audio::SoundNode* mNode;
  void SendAudioEvent(const Audio::AudioEventType eventType, void* data) override;
  void SetNode(Audio::SoundNode* node, Status& status);
  void ReleaseNode();
  bool mCanInsertBefore;
  bool mCanInsertAfter;
  bool mCanReplace;
  bool mCanRemove;
};

//------------------------------------------------------------------------------------- Sound Buffer

/// Used with a CustomAudioNode to play audio data directly
class SoundBuffer : public ReferenceCountedObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SoundBuffer() : mSampleRate(48000), mChannels(1) {}

  /// Adds a new audio sample to the end of the buffer.
  void AddSampleToBuffer(float sample);
  /// The sample rate of the audio in the buffer.
  int GetSampleRate();
  void SetSampleRate(int sampleRate);
  /// The number of audio channels in the buffer.
  int GetChannels();
  void SetChannels(int numChannels);
  /// The number of samples currently in the buffer.
  int GetSampleCount();
  /// Returns the sample at a specific index from the beginning of the buffer.
  float GetSampleAtIndex(int index);
  /// Removes all data from the buffer and resets it.
  void Reset();

private:
  Zero::Array<float> mBuffer;
  int mSampleRate;
  int mChannels;

  friend class CustomAudioNode;
};

//-------------------------------------------------------------------------------- Custom Audio Node

/// Uses a SoundBuffer to send audio data directly to the audio engine
class CustomAudioNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CustomAudioNode();
  ~CustomAudioNode();

  /// The minimum number of samples that should be sent when a NeedMoreSamples event is received.
  int GetMinimumBufferSize();
  /// The sample rate currently being used by the audio system.
  int GetSystemSampleRate();
  /// The number of audio channels that will be in the buffer.
  int GetChannels();
  void SetChannels(int numChannels);
  /// Sends a buffer of audio samples to the audio system for output.
  void SendBuffer(SoundBuffer* buffer);
  /// Sends a partial buffer of audio samples to the audio system for output.
  void SendPartialBuffer(SoundBuffer* buffer, int startAtIndex, int howManySamples);

private:
  void SendAudioEvent(const Audio::AudioEventType eventType, void* data) override;

  friend class SoundSpace;
};

//------------------------------------------------------------------------------ Generated Wave Node

/// Types of sound waves that can be used by SoundNodes.
/// <param name="SineWave">Audio generated by a Sine wave.</param>
/// <param name="SquareWave">Audio generated by a square wave (values are either -1 or 1).</param>
/// <param name="SawWave">Audio generated by a saw wave (values go linearly from -1 to 1 then jump back to -1).</param>
/// <param name="TriangleWave">Audio generated by a triangle wave (values go linearly from -1 to 1 and back again).</param>
/// <param name="Noise">White noise produced by getting random values between -1 and 1.</param>
DeclareEnum5(SynthWaveType, SineWave, SquareWave, SawWave, TriangleWave, Noise);

/// Plays audio using the specified type of generated wave
class GeneratedWaveNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  GeneratedWaveNode();
  ~GeneratedWaveNode();

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

private:
  Audio::SoundAssetNode *mAsset;
  SynthWaveType::Enum mWaveType;
  float mWaveFrequency;
  float mVolume;

  void CreateAsset();
  void ReleaseAsset();
  void CreateInstance(bool paused);
  void ReleaseInstance();
};

//-------------------------------------------------------------------------------------- Volume Node

/// Changes the volume of audio generated by its input SoundNodes
class VolumeNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  VolumeNode();

  /// The volume adjustment that will be applied to the node's input. 
  /// A value of 1 does not affect the sound; 2 will double the sound's volume, 0.5 will halve it, 
  /// and 0 will make the sound inaudible. 
  float GetVolume();
  void SetVolume(float volume);
  /// Interpolates the Volume property from its current value to the value passed in 
  /// as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateVolume(float volume, float interpolationTime);
  /// The volume adjustment, in decibels, that will be applied to the node's input. 
  /// A value of 0 does not affect the sound; 6 will double the sound's volume, -6 will halve it, 
  /// and -100 is effectively the same as a Volume of 0. 
  float GetDecibels();
  void SetDecibels(float volume);
  /// Interpolates the Decibels property from its current value to the value passed in 
  /// as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateDecibels(float volumeDB, float interpolationTime);

};

//------------------------------------------------------------------------------------- Panning Node

/// Changes the left and right channel volumes of its input SoundNode's audio separately 
class PanningNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PanningNode();

  /// If this property is true, the audio will be combined into a single channel
  /// before being split between the right and left channels. If it is false and the audio
  /// has more than two channels, it will be combined into only two channels before being processed.
  bool GetSumToMono();
  void SetSumToMono(bool isMono);
  /// The volume multiplier applied to audio in the left channel.
  float GetLeftVolume();
  void SetLeftVolume(float volume);
  /// Interpolates the LeftVolume property from its current value to the value passed in
  /// as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateLeftVolume(float volume, float time);
  /// The volume multiplier applied to audio in the right channel.
  float GetRightVolume();
  void SetRightVolume(float volume);
  /// Interpolates the RightVolume property from its current value to the value passed in
  /// as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateRightVolume(float volume, float time);
  /// Interpolates both left and right volume properties at once. The first parameter
  /// is the value to change the LeftVolume to, the second is the RightVolume,
  /// and the third is the number of seconds to use for the interpolation.
  void InterpolateVolumes(float leftVolume, float rightVolume, float time);
};

//--------------------------------------------------------------------------------------- Pitch Node

/// Changes the pitch and speed of audio generated by its input SoundNodes
class PitchNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PitchNode();

  /// The pitch adjustment applied to the node's input. 
  /// A value of 0 will not affect the sound's pitch; 1 will raise the pitch by 
  /// an octave and speed up the sound, and -1 will lower the sound by an octave and slow it down. 
  /// Large pitch changes will likely affect the quality of the sound.
  float GetPitch();
  void SetPitch(float pitchRatio);
  /// Interpolates the Pitch property from its current value to the value passed in 
  /// as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolatePitch(float pitchRatio, float interpolationTime);
  /// The pitch adjustment, in semitones (or half-steps), applied to the node's input. 
  /// A value of 0 will not affect the sound's pitch; 12 will raise the pitch by an octave 
  /// and speed up the sound, and -12 will lower the sound by an octave and slow it down. 
  /// Large pitch changes will likely affect the quality of the sound.
  float GetSemitones();
  void SetSemitones(float pitchSemitones);
  /// Interpolates the Semitones property from its current value to the value passed in 
  /// as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateSemitones(float pitchSemitones, float interpolationTime);

};

//------------------------------------------------------------------------------------ Low Pass Node

/// Applies a low pass filter to audio generated by its input SoundNodes (removes high frequencies)
class LowPassNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  LowPassNode();

  /// Frequencies above this number in the node's input will be attenuated. 
  /// Setting this value to 20,000.00 or higher will skip all filter calculations. 
  float GetCutoffFrequency();
  void SetCutoffFrequency(float frequency);

};

//----------------------------------------------------------------------------------- High Pass Node

/// Applies a high pass filter to audio generated by its input SoundNodes (removes low frequencies)
class HighPassNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  HighPassNode();

  /// Frequencies below this number in the node's input will be attenuated. 
  /// Setting this value to 20.0 or lower will skip all filter calculations.
  float GetCutoffFrequency();
  void SetCutoffFrequency(float frequency);

};

//----------------------------------------------------------------------------------- Band Pass Node

/// Applies a band pass filter to audio generated by its input SoundNodes (removes low and high frequencies)
class BandPassNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  BandPassNode();

  /// The center frequency of the band. Frequencies above and below this band will be attenuated.
  float GetCentralFrequency();
  void SetCentralFrequency(float frequency);
  /// The Q number of the band pass filter: higher numbers make the band smaller, 
  /// while smaller numbers make it wider. The default value is 0.669.
  float GetQualityFactor();
  void SetQualityFactor(float Q);
};

//----------------------------------------------------------------------------------- Equalizer Node

/// Applied volume changes to specific frequency bands in the audio generated by its input SoundNodes
class EqualizerNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  EqualizerNode();

  /// The volume adjustment applied to frequencies below 80 Hz in the node's input. 
  /// Values above 1.0 will boost these frequencies while values less than 1.0 will reduce them.
  float GetLowPassGain();
  void SetLowPassGain(float gain);
  /// The volume adjustment applied to frequencies above 5000 Hz in the node's input. 
  /// Values above 1.0 will boost these frequencies while values less than 1.0 will reduce them.
  float GetHighPassGain();
  void SetHighPassGain(float gain);
  /// The volume adjustment applied to frequencies within the band centered at 150 Hz in the node's input. 
  /// Values above 1.0 will boost these frequencies while values less than 1.0 will reduce them.
  float GetBand1Gain();
  void SetBand1Gain(float gain);
  /// The volume adjustment applied to frequencies within the band centered at 600 Hz in the node's input. 
  /// Values above 1.0 will boost these frequencies while values less than 1.0 will reduce them.
  float GetBand2Gain();
  void SetBand2Gain(float gain);
  /// The volume adjustment applied to frequencies within the band centered at 2500 Hz in the node's input. 
  /// Values above 1.0 will boost these frequencies while values less than 1.0 will reduce them.
  float GetBand3Gain();
  void SetBand3Gain(float gain);
  /// Sets the volume adjustment of all bands (the parameters in order are low pass, band 1, band 2, 
  /// band 3, high pass) over the number of seconds passed in as the final parameter.
  void InterpolateAllBands(float lowPass, float band1, float band2, float band3, float highPass, 
    float timeToInterpolate);
};

//-------------------------------------------------------------------------------------- Reverb Node

/// Applies a simple reverb filter to audio generated by its input SoundNodes
class ReverbNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ReverbNode();

  /// The length of the reverb tail, in seconds. The default value is 0.1.
  float GetLength();
  void SetLength(float time);
  /// The percentage of the node's output which has the reverb filter applied to it. 
  /// Setting this property to 0 will stop all reverb calculations. The default value is 50.0.
  float GetWetPercent();
  void SetWetPercent(float percent);
  /// Interpolates the WetPercent property from its current value to the value passed in 
  /// as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateWetPercent(float percent, float time);
};

//--------------------------------------------------------------------------------------- Delay Node

/// Applies a delay filter to audio generated by its input SoundNodes
class DelayNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  DelayNode();

  /// The length of the delay, in seconds. 
  float GetDelay();
  void SetDelay(float seconds);
  /// The percentage of output which is fed back into the filter as input, 
  /// creating an echo-like effect. 
  float GetFeedbackPercent();
  void SetFeedbackPercent(float feedback);
  /// The percentage of the node's output which has the delay filter applied to it. 
  float GetWetPercent();
  void SetWetPercent(float wetLevel);
  /// Interpolates the WetPercent property from its current value to the value passed in 
  /// as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateWetPercent(float wetPercent, float time);
};

//------------------------------------------------------------------------------------- Flanger Node

/// Applies a flanger filter to audio generated by its input SoundNodes
class FlangerNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  FlangerNode();

  /// The maximum delay reached by the modulation. It will oscillate between 0 and this value
  /// at the frequency set by the ModulationFrequency property.
  float GetMaxDelayMillisec();
  void SetMaxDelayMillisec(float delay);
  /// The frequency of the oscillator which varies the modulation. 
  float GetModulationFrequency();
  void SetModulationFrequency(float frequency);
  /// The percentage of output which is fed back into the filter as input. 
  float GetFeedbackPercent();
  void SetFeedbackPercent(float percent);
};

//-------------------------------------------------------------------------------------- Chorus Node

/// Applies a chorus filter to audio generated by its input SoundNodes
class ChorusNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ChorusNode();

  /// The maximum delay reached by the modulation. It will oscillate between the MinDelayMillisec
  /// value and this value at the frequency set by the ModulationFrequency property.
  float GetMaxDelayMillisec();
  void SetMaxDelayMillisec(float delay);
  /// The minimum delay reached by the modulation. It will oscillate between the this value and the
  /// MaxDelayMillisec value at the frequency set by the ModulationFrequency property.
  float GetMinDelayMillisec();
  void SetMinDelayMillisec(float delay);
  /// The frequency of the oscillator which varies the modulation. 
  float GetModulationFrequency();
  void SetModulationFrequency(float frequency);
  /// The percentage of output which is fed back into the filter as input. 
  float GetFeedbackPercent();
  void SetFeedbackPercent(float percent);
  /// The offset value of the chorus filter, in milliseconds. 
  float GetOffsetMillisec();
  void SetOffsetMillisec(float offset);
};

//---------------------------------------------------------------------------------- Compressor Node

class CompressorNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CompressorNode();

  /// The volume adjustment applied to the audio input, in decibels. 
  float GetInputGainDecibels();
  void SetInputGainDecibels(float gain);
  /// The threshold, in decibels, at which the volume of the input is affected by the compressor. 
  float GetThresholdDecibels();
  void SetThresholdDecibels(float dB);
  /// The time for the compressor to ramp to full effect after the input reaches the threshold.
  float GetAttackMillisec();
  void SetAttackMillisec(float attack);
  /// The time for the compressor to ramp from full effect to off after the input drops below the threshold.
  float GetReleaseMillisec();
  void SetReleaseMillisec(float release);
  /// The ratio of the volume reduction applied by the compressor. 
  float GetRatio();
  void SetRatio(float ratio);
  /// The volume adjustment applied to the compressor output, in decibels. 
  float GetOutputGainDecibels();
  void SetOutputGainDecibels(float gain);
  /// The knee width of the compressor, in decibels.
  float GetKneeWidth();
  void SetKneeWidth(float knee);
};

//------------------------------------------------------------------------------------ Expander Node

class ExpanderNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ExpanderNode();

  /// The volume adjustment applied to the audio input, in decibels. 
  float GetInputGainDecibels();
  void SetInputGainDecibels(float gain);
  /// The threshold, in decibels, at which the volume of the input is affected by the expander. 
  float GetThresholdDecibels();
  void SetThresholdDecibels(float dB);
  /// The time for the expander to ramp to full effect after the input reaches the threshold.
  float GetAttackMillisec();
  void SetAttackMillisec(float attack);
  /// The time for the expander to ramp from full effect to off after the input goes above the threshold.
  float GetReleaseMillisec();
  void SetReleaseMillisec(float release);
  /// The ratio of the volume reduction applied by the expander. 
  float GetRatio();
  void SetRatio(float ratio);
  /// The volume adjustment applied to the expander output, in decibels. 
  float GetOutputGainDecibels();
  void SetOutputGainDecibels(float gain);
  /// The knee width of the expander, in decibels.
  float GetKneeWidth();
  void SetKneeWidth(float knee);
};

//----------------------------------------------------------------------------------- Recording Node

/// Records audio generated by its input SoundNodes into a WAV file
class RecordingNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  RecordingNode();
  ~RecordingNode();

  /// The name of the output file that will be created, including the full path.
  /// Do not include the file extension. 
  String GetFileName();
  void SetFileName(String& fileName);
  /// Starts writing all audio input to a file.
  void StartRecording();
  /// Stops writing data and closes the file.
  void StopRecording();
  /// When true, recording is paused, and can be resumed by setting to false
  bool GetPaused();
  void SetPaused(bool paused);
  /// When false, audio data will be saved in a buffer and written to the file when
  /// StopRecording is called. When true, data will be written to the file 
  /// constantly during every update frame, and nothing will be saved.
  bool GetStreamToDisk();
  void SetStreamToDisk(bool stream);
};

//----------------------------------------------------------------------------------- Add Noise Node

/// Adds random noise (static) to audio generated by its input SoundNodes
class AddNoiseNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  AddNoiseNode();

  /// The gain of the additive noise component, in decibels.
  float GetAdditiveGain();
  void SetAdditiveGain(float decibels);
  /// The gain of the multiplicative noise component, in decibels.
  float GetMultiplicativeGain();
  void SetMultiplicativeGain(float decibels);
  /// The cutoff frequency used for the additive noise component, in Hz.
  float GetAdditiveCutoff();
  void SetAdditiveCutoff(float frequency);
  /// The cutoff frequency used for the multiplicative noise component, in Hz.
  float GetMultiplicativeCutoff();
  void SetMultiplicativeCutoff(float frequency);
};

//------------------------------------------------------------------------------------ ADSR Envelope

/// Used to control harmonics of notes played by the AdditiveSynthNode
class AdsrEnvelope
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);

  AdsrEnvelope() :
    DelayTime(0.0f),
    AttackTime(0.02f),
    DecayTime(0.0f),
    SustainTime(1.0f),
    SustainLevel(1.0f),
    ReleaseTime(0.5f)
  {}

  ///The time, in seconds, between when the note starts and when this harmonic starts playing.
  float DelayTime;
  /// The time, in seconds, for this harmonic to interpolate its volume 
  /// from 0 to 1.0 when it starts playing.
  float AttackTime;
  /// The time, in seconds, for this harmonic to interpolate logarithmically 
  /// from 1.0 to the SustainLevel after the attack.
  float DecayTime;
  /// The time, in seconds, for this harmonic to stay at the SustainLevel after the attack and decay.
  /// A value of 0 will make the harmonic play indefinitely until NoteOff is called.
  float SustainTime;
  /// The volume level (1.0 is full volume) to use for the sustain period.
  float SustainLevel;
  /// The time, in seconds, for this harmonic to interpolate logarithmically from the SustainLevel to 0.
  float ReleaseTime;
};

//------------------------------------------------------------------------------ Additive Synth Node

/// Generates audio using additive synthesis 
class AdditiveSynthNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  AdditiveSynthNode();

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
}; 

//---------------------------------------------------------------------------------- Modulation Node

/// Applies either ring or amplitude modulation to audio generated by its input SoundNodes
class ModulationNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ModulationNode();

  /// If this property is true, the node will apply amplitude modulation (multiply the audio input 
  /// with a unipolar sine wave with values from 0.0 to 1.0). If false, the node will apply
  /// ring modulation (multiply the input with a bipolar sine wave with values from -1.0 to 1.0).
  bool GetUseAmplitudeModulation();
  void SetUseAmplitudeModulation(bool useAmplitude);
  /// The frequency of the sine wave used for the modulation.
  float GetFrequency();
  void SetFrequency(float frequency);
  /// The percentage of the input which should have the modulation applied to it.
  float GetWetPercent();
  void SetWetPercent(float percent);
};

}