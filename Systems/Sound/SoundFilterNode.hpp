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
  DeclareEvent(CustomAudioNodeSamplesNeeded);
  DeclareEvent(AudioInterpolationDone);
  DeclareEvent(SoundNodeDisconnected);
}

//-------------------------------------------------------------------------- Custom Audio Node Event

class CustomAudioNodeEvent : public Event 
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CustomAudioNodeEvent(unsigned samples) :
    SamplesNeeded(samples)
  {}

  unsigned SamplesNeeded;
};

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
  /// DEPRECATED The BypassValue property should be used instead.
  float GetBypassPercent();
  void SetBypassPercent(float percent);
  /// The percentage of output (0 to 1.0) that should skip whatever processing the node does.
  float GetBypassValue();
  void SetBypassValue(float value);

// Internals
  Audio::SoundNode* mNode;
  void SendAudioEvent(const Audio::AudioEventTypes::Enum eventType, void* data) override;
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

  /// Adds a new audio sample to the end of the buffer.
  void AddSampleToBuffer(float sample);
  /// The number of samples currently in the buffer.
  int GetSampleCount();
  /// Returns the sample at a specific index from the beginning of the buffer.
  float GetSampleAtIndex(int index);
  /// Removes all data from the buffer and resets it.
  void Reset();
  /// Takes the AudioData from a MicrophoneUncompressedFloatData event and adds all of 
  /// the audio samples to the buffer
  void AddMicUncompressedData(const HandleOf<ArrayClass<float>>& audioData);

private:
  Zero::Array<float> mBuffer;

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
  /// Takes the AudioData from a MicrophoneUncompressedFloatData event and sends all of 
  /// the audio samples to the audio engine for output
  void SendMicUncompressedData(const HandleOf<ArrayClass<float>>& audioData);
  /// Takes the AudioData from a MicrophoneCompressedByteData event, decompresses the data,
  /// and sends all of the audio samples to the audio engine for output
  void SendMicCompressedData(const HandleOf<ArrayClass<byte>>& audioData);

private:
  void SendAudioEvent(const Audio::AudioEventTypes::Enum eventType, void* data) override;
  void SendToAudioEngine(float* samples, unsigned howManySamples);
  Audio::CustomDataNode* GetNode();

  Audio::AudioStreamDecoder* AudioDecoder;

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
  /// The percentage of the square wave (from 0 to 1.0) which should be up. This will have no effect
  /// if a different wave type is chosen.
  float GetSquareWavePulseValue();
  void SetSquareWavePulseValue(float value);

private:
  Audio::GeneratedWaveSoundAsset *mAsset;
  SynthWaveType::Enum mWaveType;
  float mWaveFrequency;
  float mVolume;
  float mSquareWavePulseValue;

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

private:
  Audio::VolumeNode* GetNode();
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

private:
  Audio::PanningNode* GetNode();
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

private:
  Audio::PitchNode* GetNode();
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

private:
  Audio::LowPassNode* GetNode();
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

private:
  Audio::HighPassNode* GetNode();
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

private:
  Audio::BandPassNode* GetNode();
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

private:
  Audio::EqualizerNode* GetNode();
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
  /// DEPRECATED The WetValue property should be used instead.
  float GetWetPercent();
  void SetWetPercent(float percent);
  /// The percentage of the node's output (0 - 1.0) which has the reverb filter applied to it. 
  /// Setting this property to 0 will stop all reverb calculations. 
  float GetWetValue();
  void SetWetValue(float value);
  /// DEPRECATED The InterpolateWetValue method should be used instead.
  void InterpolateWetPercent(float percent, float time);
  /// Interpolates the WetValue property from its current value to the value passed in 
  /// as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateWetValue(float value, float time);

private:
  Audio::ReverbNode* GetNode();
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
  /// DEPRECATED The FeedbackValue property should be used instead.
  float GetFeedbackPercent();
  void SetFeedbackPercent(float feedback);
  /// The percentage of output (from 0 to 1.0f) which is fed back into the filter as input, 
  /// creating an echo-like effect. 
  float GetFeedbackValue();
  void SetFeedbackValue(float feedback);
  /// DEPRECATED The WetValue property should be used instead.
  float GetWetPercent();
  void SetWetPercent(float wetLevel);
  /// The percentage of the node's output (0 - 1.0) which has the delay filter applied to it. 
  float GetWetValue();
  void SetWetValue(float wetLevel);
  /// DEPRECATED The InterpolateWetValue method should be used instead.
  void InterpolateWetPercent(float wetPercent, float time);
  /// Interpolates the WetValue property from its current value to the value passed in 
  /// as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateWetValue(float wetPercent, float time);

private:
  Audio::DelayNode* GetNode();
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
  /// DEPRECATED The FeedbackValue property should be used instead.
  float GetFeedbackPercent();
  void SetFeedbackPercent(float percent);
  /// The percentage of output (0 - 1.0) which is fed back into the filter as input. 
  float GetFeedbackValue();
  void SetFeedbackValue(float value);

private:
  Audio::FlangerNode* GetNode();
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
  /// DEPRECATED The FeedbackValue property should be used instead.
  float GetFeedbackPercent();
  void SetFeedbackPercent(float percent);
  /// The percentage of output (0 - 1.0) which is fed back into the filter as input. 
  float GetFeedbackValue();
  void SetFeedbackValue(float value);
  /// The offset value of the chorus filter, in milliseconds. 
  float GetOffsetMillisec();
  void SetOffsetMillisec(float offset);

private:
  Audio::ChorusNode* GetNode();
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

private:
  Audio::DynamicsProcessorNode* GetNode();
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

private:
  Audio::DynamicsProcessorNode* GetNode();
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

private:
  Audio::RecordNode* GetNode();
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

private:
  Audio::AddNoiseNode* GetNode();
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

private:
  Audio::AdditiveSynthNode* GetNode();
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
  /// DEPRECATED The WetValue property should be used instead.
  float GetWetPercent();
  void SetWetPercent(float percent);
  /// The percentage of the input (0 - 1.0) which should have the modulation applied to it.
  float GetWetValue();
  void SetWetValue(float value);

private:
  Audio::ModulationNode* GetNode();
};

//---------------------------------------------------------------------------- Microphone Input Node

/// Receives input from a microphone and passes the audio data to its output SoundNodes
class MicrophoneInputNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MicrophoneInputNode();

  /// The volume modifier applied to all audio data received from the microphone.
  float GetVolume();
  void SetVolume(float volume);
  /// Microphone input will only be played while the Active property is set to True.
  bool GetActive();
  void SetActive(bool active);

private:
  Audio::MicrophoneInputNode* GetNode();
};

//---------------------------------------------------------------------------------- Save Audio Node

/// Types of windows (volume envelopes) that can be used for individual grains generated 
/// by the GranularSynthNode.
/// <param name="Linear">A triangle-shaped linear envelope. Does not use attack and release times.</param>
/// <param name="Parabolic">A smoothly curved envelope. Does not use attack and release times.</param>
/// <param name="RaisedCosine">Uses cosine curves to smoothly ramp up and down during attack and release times.</param>
/// <param name="Trapezoid">Uses linear ramps during attack and release times. More efficient than RaisedCosine but not as smooth.</param>
DeclareEnum4(GranularSynthWindows, Linear, Parabolic, RaisedCosine, Trapezoid);
/// Saves audio from its input SoundNodes and then plays it. All audio from inputs is passed to outputs.
class SaveAudioNode : public SoundNode 
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SaveAudioNode();

  /// When true, audio from input SoundNodes will be saved. Setting this to true will remove any
  /// existing saved audio before saving more.
  bool GetSaveAudio();
  void SetSaveAudio(bool save);
  /// Plays the saved audio.
  void PlaySavedAudio();
  /// Stops playing the saved audio.
  void StopPlaying();
  /// Removes all currently saved audio.
  void ClearSavedAudio();

private:
  Audio::SaveAudioNode* GetNode();
};

//------------------------------------------------------------------------------ Granular Synth Node

class GranularSynthNode : public SoundNode 
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  GranularSynthNode();

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
  Audio::GranularSynthNode* GetNode();
};

}
