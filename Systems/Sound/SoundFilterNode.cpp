///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{



//-------------------------------------------------------------------------------------- Volume Node

//**************************************************************************************************
ZilchDefineType(VolumeNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(Volume);
  ZilchBindMethod(InterpolateVolume);
  ZilchBindGetterSetter(Decibels);
  ZilchBindMethod(InterpolateDecibels);
}

//**************************************************************************************************
VolumeNode::VolumeNode()
{
  mNode = new Audio::VolumeNode("VolumeNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
float VolumeNode::GetVolume()
{
  return GetNode()->GetVolume();
}

//**************************************************************************************************
void VolumeNode::SetVolume(float volume)
{
  InterpolateVolume(volume, 0.0f);
}

//**************************************************************************************************
void VolumeNode::InterpolateVolume(float volume, float time)
{
  GetNode()->SetVolume(Math::Max(volume, 0.0f), time);
}

//**************************************************************************************************
float VolumeNode::GetDecibels()
{
  return Z::gSound->VolumeToDecibels(GetNode()->GetVolume());
}

//**************************************************************************************************
void VolumeNode::SetDecibels(float volumeDB)
{
  InterpolateDecibels(volumeDB, 0.0f);
}

//**************************************************************************************************
void VolumeNode::InterpolateDecibels(float volumeDB, float time)
{
  GetNode()->SetVolume(Z::gSound->DecibelsToVolume(volumeDB), time);
}

//**************************************************************************************************
Audio::VolumeNode* VolumeNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::VolumeNode*)mNode;
}

//------------------------------------------------------------------------------------- Panning Node

//**************************************************************************************************
ZilchDefineType(PanningNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(SumToMono);
  ZilchBindGetterSetter(LeftVolume);
  ZilchBindGetterSetter(RightVolume);
  ZilchBindMethod(InterpolateLeftVolume);
  ZilchBindMethod(InterpolateRightVolume);
  ZilchBindMethod(InterpolateVolumes);
}

//**************************************************************************************************
PanningNode::PanningNode()
{
  mNode = new Audio::PanningNode("PanningNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
bool PanningNode::GetSumToMono()
{
  return GetNode()->GetSumToMono();
}

//**************************************************************************************************
void PanningNode::SetSumToMono(bool isMono)
{
  GetNode()->SetSumToMono(isMono);
}

//**************************************************************************************************
float PanningNode::GetLeftVolume()
{
  return GetNode()->GetLeftVolume();
}

//**************************************************************************************************
void PanningNode::SetLeftVolume(float volume)
{
  InterpolateLeftVolume(volume, 0.0f);
}

//**************************************************************************************************
void PanningNode::InterpolateLeftVolume(float volume, float time)
{
  GetNode()->SetLeftVolume(Math::Max(volume, 0.0f), time);
}

//**************************************************************************************************
float PanningNode::GetRightVolume()
{
  return GetNode()->GetRightVolume();
}

//**************************************************************************************************
void PanningNode::SetRightVolume(float volume)
{
  InterpolateRightVolume(volume, 0.0f);
}

//**************************************************************************************************
void PanningNode::InterpolateRightVolume(float volume, float time)
{
  GetNode()->SetRightVolume(Math::Max(volume, 0.0f), time);
}

//**************************************************************************************************
void PanningNode::InterpolateVolumes(float leftVolume, float rightVolume, float time)
{
  GetNode()->SetLeftVolume(Math::Max(leftVolume, 0.0f), time);
  GetNode()->SetRightVolume(Math::Max(rightVolume, 0.0f), time);
}

//**************************************************************************************************
Audio::PanningNode* PanningNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::PanningNode*)mNode;
}

//--------------------------------------------------------------------------------------- Pitch Node

//**************************************************************************************************
ZilchDefineType(PitchNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(Pitch);
  ZilchBindGetterSetter(Semitones);
  ZilchBindMethod(InterpolatePitch);
  ZilchBindMethod(InterpolateSemitones);
}

//**************************************************************************************************
PitchNode::PitchNode()
{
  mNode = new Audio::PitchNode("PitchNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
float PitchNode::GetPitch()
{
  return Z::gSound->SemitonesToPitch(GetNode()->GetPitch());
}

//**************************************************************************************************
void PitchNode::SetPitch(float pitchRatio)
{
  InterpolatePitch(pitchRatio, 0.0f);
}

//**************************************************************************************************
void PitchNode::InterpolatePitch(float pitchRatio, float time)
{
  GetNode()->SetPitch(Z::gSound->PitchToSemitones(pitchRatio), time);
}

//**************************************************************************************************
float PitchNode::GetSemitones()
{
  return GetNode()->GetPitch();
}

//**************************************************************************************************
void PitchNode::SetSemitones(float pitchSemitones)
{
  InterpolateSemitones(pitchSemitones, 0.0f);
}

//**************************************************************************************************
void PitchNode::InterpolateSemitones(float pitchSemitones, float time)
{
  GetNode()->SetPitch(pitchSemitones, time);
}

//**************************************************************************************************
Audio::PitchNode* PitchNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::PitchNode*)mNode;
}

//------------------------------------------------------------------------------------ Low Pass Node

//**************************************************************************************************
ZilchDefineType(LowPassNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(CutoffFrequency);
}

//**************************************************************************************************
LowPassNode::LowPassNode()
{
  mNode = new Audio::LowPassNode("LowPassNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
float LowPassNode::GetCutoffFrequency()
{
  return GetNode()->GetCutoffFrequency();
}

//**************************************************************************************************
void LowPassNode::SetCutoffFrequency(float frequency)
{
  GetNode()->SetCutoffFrequency(Math::Max(frequency, 0.0f));
}

//**************************************************************************************************
Audio::LowPassNode* LowPassNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::LowPassNode*)mNode;
}

//----------------------------------------------------------------------------------- High Pass Node

//**************************************************************************************************
ZilchDefineType(HighPassNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(CutoffFrequency);
}

//**************************************************************************************************
HighPassNode::HighPassNode()
{
  mNode = new Audio::HighPassNode("HighPassNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
float HighPassNode::GetCutoffFrequency()
{
  return GetNode()->GetCutoffFrequency();
}

//**************************************************************************************************
void HighPassNode::SetCutoffFrequency(float frequency)
{
  GetNode()->SetCutoffFrequency(Math::Max(frequency, 0.0f));
}

//**************************************************************************************************
Audio::HighPassNode* HighPassNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::HighPassNode*)mNode;
}

//----------------------------------------------------------------------------------- Band Pass Node

//**************************************************************************************************
ZilchDefineType(BandPassNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(CentralFrequency);
  ZilchBindGetterSetter(QualityFactor);
}

//**************************************************************************************************
BandPassNode::BandPassNode()
{
  mNode = new Audio::BandPassNode("BandPassNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
float BandPassNode::GetCentralFrequency()
{
  return GetNode()->GetCentralFrequency();
}

//**************************************************************************************************
void BandPassNode::SetCentralFrequency(float frequency)
{
  GetNode()->SetCentralFrequency(Math::Max(frequency, 0.0f));
}

//**************************************************************************************************
float BandPassNode::GetQualityFactor()
{
  return GetNode()->GetQuality();
}

//**************************************************************************************************
void BandPassNode::SetQualityFactor(float Q)
{
  GetNode()->SetQuality(Q);
}

//**************************************************************************************************
Audio::BandPassNode* BandPassNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::BandPassNode*)mNode;
}

//----------------------------------------------------------------------------------- Equalizer Node

//**************************************************************************************************
ZilchDefineType(EqualizerNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(LowPassGain);
  ZilchBindGetterSetter(HighPassGain);
  ZilchBindGetterSetter(Band1Gain);
  ZilchBindGetterSetter(Band2Gain);
  ZilchBindGetterSetter(Band3Gain);
  ZilchBindMethod(InterpolateAllBands);
}

//**************************************************************************************************
EqualizerNode::EqualizerNode()
{
  mNode = new Audio::EqualizerNode("EqualizerNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
float EqualizerNode::GetLowPassGain()
{
  return GetNode()->GetBelow80HzGain();
}

//**************************************************************************************************
void EqualizerNode::SetLowPassGain(float gain)
{
  GetNode()->SetBelow80HzGain(Math::Max(gain, 0.0f));
}

//**************************************************************************************************
float EqualizerNode::GetHighPassGain()
{
  return GetNode()->GetAbove5000HzGain();
}

//**************************************************************************************************
void EqualizerNode::SetHighPassGain(float gain)
{
  GetNode()->SetAbove5000HzGain(Math::Max(gain, 0.0f));
}

//**************************************************************************************************
float EqualizerNode::GetBand1Gain()
{
  return GetNode()->Get150HzGain();
}

//**************************************************************************************************
void EqualizerNode::SetBand1Gain(float gain)
{
  GetNode()->Set150HzGain(Math::Max(gain, 0.0f));
}

//**************************************************************************************************
float EqualizerNode::GetBand2Gain()
{
  return GetNode()->Get600HzGain();
}

//**************************************************************************************************
void EqualizerNode::SetBand2Gain(float gain)
{
  GetNode()->Set600HzGain(Math::Max(gain, 0.0f));
}

//**************************************************************************************************
float EqualizerNode::GetBand3Gain()
{
  return GetNode()->Get2500HzGain();
}

//**************************************************************************************************
void EqualizerNode::SetBand3Gain(float gain)
{
  GetNode()->Set2500HzGain(Math::Max(gain, 0.0f));
}

//**************************************************************************************************
void EqualizerNode::InterpolateAllBands(float lowPass, float band1, float band2, float band3, 
  float highPass, float timeToInterpolate)
{
  GetNode()->InterpolateBands(Audio::EqualizerBandGains(Math::Max(lowPass, 0.0f), Math::Max(band1, 0.0f), 
    Math::Max(band2, 0.0f), Math::Max(band3, 0.0f), Math::Max(highPass, 0.0f)), timeToInterpolate);
}

//**************************************************************************************************
Audio::EqualizerNode* EqualizerNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::EqualizerNode*)mNode;
}

//-------------------------------------------------------------------------------------- Reverb Node

//**************************************************************************************************
ZilchDefineType(ReverbNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(Length);
  ZilchBindGetterSetter(WetPercent)->AddAttribute(DeprecatedAttribute);
  ZilchBindGetterSetter(WetValue);
  ZilchBindMethod(InterpolateWetPercent)->AddAttribute(DeprecatedAttribute);
  ZilchBindMethod(InterpolateWetValue);
}

//**************************************************************************************************
ReverbNode::ReverbNode()
{
  mNode = new Audio::ReverbNode("ReverbNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
float ReverbNode::GetLength()
{
  return GetNode()->GetTime() / 1000.0f;
}

//**************************************************************************************************
void ReverbNode::SetLength(float time)
{
  GetNode()->SetTime(Math::Max(time, 0.0f) * 1000);
}

//**************************************************************************************************
float ReverbNode::GetWetPercent()
{
  return GetNode()->GetWetLevel() * 100.0f;
}

//**************************************************************************************************
void ReverbNode::SetWetPercent(float percent)
{
  GetNode()->SetWetLevel(Math::Clamp(percent, 0.0f, 100.0f) / 100.0f);
}

//**************************************************************************************************
float ReverbNode::GetWetValue()
{
  return GetNode()->GetWetLevel();
}

//**************************************************************************************************
void ReverbNode::SetWetValue(float value)
{
  GetNode()->SetWetLevel(Math::Clamp(value, 0.0f, 1.0f));
}

//**************************************************************************************************
void ReverbNode::InterpolateWetPercent(float percent, float time)
{
  GetNode()->InterpolateWetLevel(Math::Clamp(percent, 0.0f, 100.0f) / 100.0f, time);
}

//**************************************************************************************************
void ReverbNode::InterpolateWetValue(float value, float time)
{
  GetNode()->InterpolateWetLevel(Math::Clamp(value, 0.0f, 1.0f), time);
}

//**************************************************************************************************
Audio::ReverbNode* ReverbNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::ReverbNode*)mNode;
}

//--------------------------------------------------------------------------------------- Delay Node

//**************************************************************************************************
ZilchDefineType(DelayNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(Delay);
  ZilchBindGetterSetter(FeedbackPercent)->AddAttribute(DeprecatedAttribute);
  ZilchBindGetterSetter(FeedbackValue);
  ZilchBindGetterSetter(WetPercent)->AddAttribute(DeprecatedAttribute);
  ZilchBindGetterSetter(WetValue);
  ZilchBindMethod(InterpolateWetPercent)->AddAttribute(DeprecatedAttribute);
  ZilchBindMethod(InterpolateWetValue);
}

//**************************************************************************************************
DelayNode::DelayNode()
{
  mNode = new Audio::DelayNode("DelayNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
float DelayNode::GetDelay()
{
  return GetNode()->GetDelayMSec() / 1000.0f;
}

//**************************************************************************************************
void DelayNode::SetDelay(float seconds)
{
  GetNode()->SetDelayMSec(Math::Max(seconds, 0.0f) * 1000.0f);
}

//**************************************************************************************************
float DelayNode::GetFeedbackPercent()
{
  return GetNode()->GetFeedback() * 100.0f;
}

//**************************************************************************************************
void DelayNode::SetFeedbackPercent(float feedback)
{
  GetNode()->SetFeedback(Math::Clamp(feedback, 0.0f, 100.0f) / 100.0f);
}

//**************************************************************************************************
float DelayNode::GetFeedbackValue()
{
  return GetNode()->GetFeedback();
}

//**************************************************************************************************
void DelayNode::SetFeedbackValue(float feedback)
{
  GetNode()->SetFeedback(Math::Clamp(feedback, 0.0f, 1.0f));
}

//**************************************************************************************************
float DelayNode::GetWetPercent()
{
  return GetNode()->GetWetLevel() * 100.0f;
}

//**************************************************************************************************
void DelayNode::SetWetPercent(float wetLevel)
{
  GetNode()->SetWetLevel(Math::Clamp(wetLevel, 0.0f, 100.0f) / 100.0f);
}

//**************************************************************************************************
float DelayNode::GetWetValue()
{
  return GetNode()->GetWetLevel();
}

//**************************************************************************************************
void DelayNode::SetWetValue(float wetLevel)
{
  GetNode()->SetWetLevel(Math::Clamp(wetLevel, 0.0f, 1.0f));
}

//**************************************************************************************************
void DelayNode::InterpolateWetPercent(float percent, float time)
{
  GetNode()->InterpolateWetLevel(Math::Clamp(percent, 0.0f, 100.0f) / 100.0f, time);
}

//**************************************************************************************************
void DelayNode::InterpolateWetValue(float wetLevel, float time)
{
  GetNode()->InterpolateWetLevel(Math::Clamp(wetLevel, 0.0f, 1.0f), time);
}

//**************************************************************************************************
Audio::DelayNode* DelayNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::DelayNode*)mNode;
}

//------------------------------------------------------------------------------------- Flanger Node

//**************************************************************************************************
ZilchDefineType(FlangerNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(MaxDelayMillisec);
  ZilchBindGetterSetter(ModulationFrequency);
  ZilchBindGetterSetter(FeedbackPercent)->AddAttribute(DeprecatedAttribute);
  ZilchBindGetterSetter(FeedbackValue);
}

//**************************************************************************************************
FlangerNode::FlangerNode()
{
  mNode = new Audio::FlangerNode("FlangerNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
float FlangerNode::GetMaxDelayMillisec()
{
  return GetNode()->GetMaxDelayMSec();
}

//**************************************************************************************************
void FlangerNode::SetMaxDelayMillisec(float delay)
{
  GetNode()->SetMaxDelayMSec(Math::Max(delay, 0.0f));
}

//**************************************************************************************************
float FlangerNode::GetModulationFrequency()
{
  return GetNode()->GetModFrequency();
}

//**************************************************************************************************
void FlangerNode::SetModulationFrequency(float frequency)
{
  GetNode()->SetModFrequency(Math::Max(frequency, 0.0f));
}

//**************************************************************************************************
float FlangerNode::GetFeedbackPercent()
{
  return GetNode()->GetFeedback() * 100.0f;
}

//**************************************************************************************************
void FlangerNode::SetFeedbackPercent(float percent)
{
  GetNode()->SetFeedback(Math::Clamp(percent, 0.0f, 100.0f) / 100.0f);
}

//**************************************************************************************************
float FlangerNode::GetFeedbackValue()
{
  return GetNode()->GetFeedback();
}

//**************************************************************************************************
void FlangerNode::SetFeedbackValue(float value)
{
  GetNode()->SetFeedback(Math::Clamp(value, 0.0f, 1.0f));
}

//**************************************************************************************************
Audio::FlangerNode* FlangerNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::FlangerNode*)mNode;
}

//-------------------------------------------------------------------------------------- Chorus Node

//**************************************************************************************************
ZilchDefineType(ChorusNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(MaxDelayMillisec);
  ZilchBindGetterSetter(MinDelayMillisec);
  ZilchBindGetterSetter(ModulationFrequency);
  ZilchBindGetterSetter(FeedbackPercent)->AddAttribute(DeprecatedAttribute);
  ZilchBindGetterSetter(FeedbackValue);
  ZilchBindGetterSetter(OffsetMillisec);
}

//**************************************************************************************************
ChorusNode::ChorusNode()
{
  mNode = new Audio::ChorusNode("ChorusNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
float ChorusNode::GetMaxDelayMillisec()
{
  return GetNode()->GetMaxDelayMSec();
}

//**************************************************************************************************
void ChorusNode::SetMaxDelayMillisec(float delay)
{
  GetNode()->SetMaxDelayMSec(Math::Max(delay, 0.0f));
}

//**************************************************************************************************
float ChorusNode::GetMinDelayMillisec()
{
  return GetNode()->GetMinDelayMSec();
}

//**************************************************************************************************
void ChorusNode::SetMinDelayMillisec(float delay)
{
  GetNode()->SetMinDelayMSec(Math::Max(delay, 0.0f));
}

//**************************************************************************************************
float ChorusNode::GetModulationFrequency()
{
  return GetNode()->GetModFrequency();
}

//**************************************************************************************************
void ChorusNode::SetModulationFrequency(float frequency)
{
  GetNode()->SetModFrequency(Math::Max(frequency, 0.0f));
}

//**************************************************************************************************
float ChorusNode::GetFeedbackPercent()
{
  return GetNode()->GetFeedback() * 100.0f;
}

//**************************************************************************************************
void ChorusNode::SetFeedbackPercent(float percent)
{
  GetNode()->SetFeedback(Math::Clamp(percent, 0.0f, 100.0f) / 100.0f);
}

//**************************************************************************************************
float ChorusNode::GetFeedbackValue()
{
  return GetNode()->GetFeedback();
}

//**************************************************************************************************
void ChorusNode::SetFeedbackValue(float value)
{
  GetNode()->SetFeedback(Math::Clamp(value, 0.0f, 1.0f));
}

//**************************************************************************************************
float ChorusNode::GetOffsetMillisec()
{
  return GetNode()->GetOffsetMSec();
}

//**************************************************************************************************
void ChorusNode::SetOffsetMillisec(float offset)
{
  GetNode()->SetOffsetMSec(Math::Max(offset, 0.0f));
}

//**************************************************************************************************
Audio::ChorusNode* ChorusNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::ChorusNode*)mNode;
}

//---------------------------------------------------------------------------------- Compressor Node

//**************************************************************************************************
ZilchDefineType(CompressorNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(InputGainDecibels);
  ZilchBindGetterSetter(ThresholdDecibels);
  ZilchBindGetterSetter(AttackMillisec);
  ZilchBindGetterSetter(ReleaseMillisec);
  ZilchBindGetterSetter(Ratio);
  ZilchBindGetterSetter(OutputGainDecibels);
  ZilchBindGetterSetter(KneeWidth);
}

//**************************************************************************************************
CompressorNode::CompressorNode()
{
  Audio::DynamicsProcessorNode* node = new Audio::DynamicsProcessorNode("DynamicsCompressionNode", 
    Z::gSound->mCounter++, this);
  node->SetType(Audio::DynamicsProcessorTypes::Compressor);
  mNode = node;
}

//**************************************************************************************************
float CompressorNode::GetInputGainDecibels()
{
  return GetNode()->GetInputGain();
}

//**************************************************************************************************
void CompressorNode::SetInputGainDecibels(float dB)
{
  GetNode()->SetInputGain(dB);
}

//**************************************************************************************************
float CompressorNode::GetThresholdDecibels()
{
  return GetNode()->GetThreshold();
}

//**************************************************************************************************
void CompressorNode::SetThresholdDecibels(float dB)
{
  GetNode()->SetThreshold(dB);
}

//**************************************************************************************************
float CompressorNode::GetAttackMillisec()
{
  return GetNode()->GetAttackMSec();
}

//**************************************************************************************************
void CompressorNode::SetAttackMillisec(float attack)
{
  GetNode()->SetAttackMSec(Math::Max(attack, 0.0f));
}

//**************************************************************************************************
float CompressorNode::GetReleaseMillisec()
{
  return GetNode()->GetReleaseMSec();
}

//**************************************************************************************************
void CompressorNode::SetReleaseMillisec(float release)
{
  GetNode()->SetReleaseMsec(Math::Max(release, 0.0f));
}

//**************************************************************************************************
float CompressorNode::GetRatio()
{
  return GetNode()->GetRatio();
}

//**************************************************************************************************
void CompressorNode::SetRatio(float ratio)
{
  GetNode()->SetRatio(ratio);
}

//**************************************************************************************************
float CompressorNode::GetOutputGainDecibels()
{
  return GetNode()->GetOutputGain();
}

//**************************************************************************************************
void CompressorNode::SetOutputGainDecibels(float dB)
{
  GetNode()->SetOutputGain(dB);
}

//**************************************************************************************************
float CompressorNode::GetKneeWidth()
{
  return GetNode()->GetKneeWidth();
}

//**************************************************************************************************
void CompressorNode::SetKneeWidth(float knee)
{
  GetNode()->SetKneeWidth(knee);
}

//**************************************************************************************************
Audio::DynamicsProcessorNode* CompressorNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::DynamicsProcessorNode*)mNode;
}

//------------------------------------------------------------------------------------ Expander Node

//**************************************************************************************************
ZilchDefineType(ExpanderNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(InputGainDecibels);
  ZilchBindGetterSetter(ThresholdDecibels);
  ZilchBindGetterSetter(AttackMillisec);
  ZilchBindGetterSetter(ReleaseMillisec);
  ZilchBindGetterSetter(Ratio);
  ZilchBindGetterSetter(OutputGainDecibels);
  ZilchBindGetterSetter(KneeWidth);
}

//**************************************************************************************************
ExpanderNode::ExpanderNode()
{
  Audio::DynamicsProcessorNode* node = new Audio::DynamicsProcessorNode("DynamicsCompressionNode", 
    Z::gSound->mCounter++, this);
  node->SetType(Audio::DynamicsProcessorTypes::Expander);
  mNode = node;
}

//**************************************************************************************************
float ExpanderNode::GetInputGainDecibels()
{
  return GetNode()->GetInputGain();
}

//**************************************************************************************************
void ExpanderNode::SetInputGainDecibels(float dB)
{
  GetNode()->SetInputGain(dB);
}

//**************************************************************************************************
float ExpanderNode::GetThresholdDecibels()
{
  return GetNode()->GetThreshold();
}

//**************************************************************************************************
void ExpanderNode::SetThresholdDecibels(float dB)
{
  GetNode()->SetThreshold(dB);
}

//**************************************************************************************************
float ExpanderNode::GetAttackMillisec()
{
  return GetNode()->GetAttackMSec();
}

//**************************************************************************************************
void ExpanderNode::SetAttackMillisec(float attack)
{
  GetNode()->SetAttackMSec(Math::Max(attack, 0.0f));
}

//**************************************************************************************************
float ExpanderNode::GetReleaseMillisec()
{
  return GetNode()->GetReleaseMSec();
}

//**************************************************************************************************
void ExpanderNode::SetReleaseMillisec(float release)
{
  GetNode()->SetReleaseMsec(Math::Max(release, 0.0f));
}

//**************************************************************************************************
float ExpanderNode::GetRatio()
{
  return GetNode()->GetRatio();
}

//**************************************************************************************************
void ExpanderNode::SetRatio(float ratio)
{
  GetNode()->SetRatio(ratio);
}

//**************************************************************************************************
float ExpanderNode::GetOutputGainDecibels()
{
  return GetNode()->GetOutputGain();
}

//**************************************************************************************************
void ExpanderNode::SetOutputGainDecibels(float dB)
{
  GetNode()->SetOutputGain(dB);
}

//**************************************************************************************************
float ExpanderNode::GetKneeWidth()
{
  return GetNode()->GetKneeWidth();
}

//**************************************************************************************************
void ExpanderNode::SetKneeWidth(float knee)
{
  GetNode()->SetKneeWidth(knee);
}

//**************************************************************************************************
Audio::DynamicsProcessorNode* ExpanderNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::DynamicsProcessorNode*)mNode;
}

//----------------------------------------------------------------------------------- Recording Node

//**************************************************************************************************
ZilchDefineType(RecordingNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(FileName);
  ZilchBindMethod(StartRecording);
  ZilchBindMethod(StopRecording);
  ZilchBindGetterSetter(Paused);
  ZilchBindGetterSetter(StreamToDisk);
}

//**************************************************************************************************
RecordingNode::RecordingNode()
{
  mNode = new Audio::RecordNode("RecordingNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
RecordingNode::~RecordingNode()
{
  StopRecording();
}

//**************************************************************************************************
String RecordingNode::GetFileName()
{
  return GetNode()->GetFileName();
}

//**************************************************************************************************
void RecordingNode::SetFileName(String& fileName)
{
  GetNode()->SetFileName(fileName);
}

//**************************************************************************************************
void RecordingNode::StartRecording()
{
  GetNode()->StartRecording();
}

//**************************************************************************************************
void RecordingNode::StopRecording()
{
  GetNode()->StopRecording();
}

//**************************************************************************************************
bool RecordingNode::GetPaused()
{
  return GetNode()->GetPaused();
}

//**************************************************************************************************
void RecordingNode::SetPaused(bool paused)
{
  GetNode()->SetPaused(paused);
}

//**************************************************************************************************
bool RecordingNode::GetStreamToDisk()
{
  return GetNode()->GetStreamToDisk();
}

//**************************************************************************************************
void RecordingNode::SetStreamToDisk(bool stream)
{
  GetNode()->SetStreamToDisk(stream);
}

//**************************************************************************************************
Audio::RecordNode* RecordingNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::RecordNode*)mNode;
}

//----------------------------------------------------------------------------------- Add Noise Node

//**************************************************************************************************
ZilchDefineType(AddNoiseNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(AdditiveGain);
  ZilchBindGetterSetter(MultiplicativeGain);
  ZilchBindGetterSetter(AdditiveCutoff);
  ZilchBindGetterSetter(MultiplicativeCutoff);
}

//**************************************************************************************************
AddNoiseNode::AddNoiseNode()
{
  mNode = new Audio::AddNoiseNode("AddNoiseNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
float AddNoiseNode::GetAdditiveGain()
{
  return GetNode()->GetAdditiveNoiseGainDB();
}

//**************************************************************************************************
void AddNoiseNode::SetAdditiveGain(float decibels)
{
  GetNode()->SetAdditiveNoiseGainDB(decibels);
}

//**************************************************************************************************
float AddNoiseNode::GetMultiplicativeGain()
{
  return GetNode()->GetMultipleNoiseGainDB();
}

//**************************************************************************************************
void AddNoiseNode::SetMultiplicativeGain(float decibels)
{
  GetNode()->SetMultipleNoiseGainDB(decibels);
}

//**************************************************************************************************
float AddNoiseNode::GetAdditiveCutoff()
{
  return GetNode()->GetAdditiveCutoffHz();
}

//**************************************************************************************************
void AddNoiseNode::SetAdditiveCutoff(float frequency)
{
  GetNode()->SetAdditiveCutoffHz(Math::Max(frequency, 0.0f));
}

//**************************************************************************************************
float AddNoiseNode::GetMultiplicativeCutoff()
{
  return ((Audio::AddNoiseNode*)mNode)->GetMultipleCutoffHz();
}

//**************************************************************************************************
void AddNoiseNode::SetMultiplicativeCutoff(float frequency)
{
  GetNode()->SetMultipleCutoffHz(Math::Max(frequency, 0.0f));
}

//**************************************************************************************************
Audio::AddNoiseNode* AddNoiseNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::AddNoiseNode*)mNode;
}

//------------------------------------------------------------------------------------ ADSR Envelope

//**************************************************************************************************
ZilchDefineType(AdsrEnvelope, builder, type)
{
  ZilchBindDefaultConstructor();
  ZeroBindDocumented();
  type->CreatableInScript = true;

  ZilchBindField(DelayTime);
  ZilchBindField(AttackTime);
  ZilchBindField(DecayTime);
  ZilchBindField(SustainTime);
  ZilchBindField(SustainLevel);
  ZilchBindField(ReleaseTime);
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
AdditiveSynthNode::AdditiveSynthNode()
{
  mNode = new Audio::AdditiveSynthNode("AdditiveSynthNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
void AdditiveSynthNode::AddHarmonic(float multiplier, float volume, AdsrEnvelope envelope, 
  SynthWaveType::Enum type)
{
  Audio::EnvelopeSettings envelopeSettings(
    Math::Max(envelope.DelayTime, 0.0f), 
    Math::Max(envelope.AttackTime, 0.0f),
    Math::Max(envelope.DecayTime, 0.0f), 
    Math::Max(envelope.SustainTime, 0.0f), 
    Math::Max(envelope.SustainLevel, 0.0f), 
    Math::Max(envelope.ReleaseTime, 0.0f));

  Audio::OscillatorTypes::Enum oscType;

  switch (type)
  {
  case SynthWaveType::SineWave:
    oscType = Audio::OscillatorTypes::Sine;
    break;
  case SynthWaveType::SawWave:
    oscType = Audio::OscillatorTypes::Saw;
    break;
  case SynthWaveType::SquareWave:
    oscType = Audio::OscillatorTypes::Square;
    break;
  case SynthWaveType::TriangleWave:
    oscType = Audio::OscillatorTypes::Triangle;
    break;
  case SynthWaveType::Noise:
    oscType = Audio::OscillatorTypes::Noise;
    break;
  }

  GetNode()->AddHarmonic(Audio::HarmonicData(Math::Max(multiplier, 0.0f), Math::Max(volume, 0.0f), 
    envelopeSettings, oscType));
}

//**************************************************************************************************
void AdditiveSynthNode::RemoveAllHarmonics()
{
  GetNode()->RemoveAllHarmonics();
}

//**************************************************************************************************
void AdditiveSynthNode::NoteOn(float midiNote, float volume)
{
  GetNode()->NoteOn((int)midiNote, Math::Max(volume, 0.0f));
}

//**************************************************************************************************
void AdditiveSynthNode::NoteOff(float midiNote)
{
  GetNode()->NoteOff((int)midiNote);
}

//**************************************************************************************************
void AdditiveSynthNode::StopAllNotes()
{
  GetNode()->StopAll();
}

//**************************************************************************************************
Audio::AdditiveSynthNode* AdditiveSynthNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::AdditiveSynthNode*)mNode;
}

//---------------------------------------------------------------------------------- Modulation Node

//**************************************************************************************************
ZilchDefineType(ModulationNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(UseAmplitudeModulation);
  ZilchBindGetterSetter(Frequency);
  ZilchBindGetterSetter(WetPercent)->AddAttribute(DeprecatedAttribute);
  ZilchBindGetterSetter(WetValue);
}

//**************************************************************************************************
ModulationNode::ModulationNode()
{
  mNode = new Audio::ModulationNode("ModulationNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
bool ModulationNode::GetUseAmplitudeModulation()
{
  return GetNode()->GetUsingAmplitude();
}

//**************************************************************************************************
void ModulationNode::SetUseAmplitudeModulation(bool useAmplitude)
{
  GetNode()->SetUsingAmplitude(useAmplitude);
}

//**************************************************************************************************
float ModulationNode::GetFrequency()
{
  return GetNode()->GetFrequency();
}

//**************************************************************************************************
void ModulationNode::SetFrequency(float frequency)
{
  GetNode()->SetFrequency(Math::Max(frequency, 0.0f));
}

//**************************************************************************************************
float ModulationNode::GetWetPercent()
{
  return GetNode()->GetWetLevel() * 100.0f;
}

//**************************************************************************************************
void ModulationNode::SetWetPercent(float percent)
{
  GetNode()->SetWetLevel(Math::Clamp(percent, 0.0f, 100.0f) / 100.0f);
}

//**************************************************************************************************
float ModulationNode::GetWetValue()
{
  return GetNode()->GetWetLevel();
}

//**************************************************************************************************
void ModulationNode::SetWetValue(float value)
{
  GetNode()->SetWetLevel(Math::Clamp(value, 0.0f, 1.0f));
}

//**************************************************************************************************
Audio::ModulationNode* ModulationNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::ModulationNode*)mNode;
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
MicrophoneInputNode::MicrophoneInputNode()
{
  mNode = new Audio::MicrophoneInputNode("MicrophoneInputNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
float MicrophoneInputNode::GetVolume()
{
  return GetNode()->GetVolume();
}

//**************************************************************************************************
void MicrophoneInputNode::SetVolume(float volume)
{
  GetNode()->SetVolume(Math::Max(volume, 0.0f));
}

//**************************************************************************************************
bool MicrophoneInputNode::GetActive()
{
  return GetNode()->GetActive();
}

//**************************************************************************************************
void MicrophoneInputNode::SetActive(bool active)
{
  GetNode()->SetActive(active);
}

//**************************************************************************************************
Audio::MicrophoneInputNode* MicrophoneInputNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::MicrophoneInputNode*)mNode;
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
GranularSynthNode::GranularSynthNode()
{
  mNode = new Audio::GranularSynthNode("GranularSynthNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
void GranularSynthNode::Play()
{
  GetNode()->Play();
}

//**************************************************************************************************
void GranularSynthNode::Stop()
{
  GetNode()->Stop();
}

//**************************************************************************************************
void GranularSynthNode::SetSound(HandleOf<Sound> sound, float startTime, float stopTime)
{
  GetNode()->SetAsset(sound->mSoundAsset, startTime, stopTime);
}

//**************************************************************************************************
float GranularSynthNode::GetGrainVolume()
{
  return GetNode()->GetGrainVolume();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainVolume(float volume)
{
  GetNode()->SetGrainVolume(volume);
}

//**************************************************************************************************
float GranularSynthNode::GetGrainVolumeVariance()
{
  return GetNode()->GetGrainVolumeVariance();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainVolumeVariance(float variance)
{
  GetNode()->SetGrainVolumeVariance(variance);
}

//**************************************************************************************************
int GranularSynthNode::GetGrainDelay()
{
  return GetNode()->GetGrainDelay();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainDelay(int delayMS)
{
  GetNode()->SetGrainDelay(delayMS);
}

//**************************************************************************************************
int GranularSynthNode::GetGrainDelayVariance()
{
  return GetNode()->GetGrainDelayVariance();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainDelayVariance(int delayVarianceMS)
{
  GetNode()->SetGrainDelayVariance(delayVarianceMS);
}

//**************************************************************************************************
int GranularSynthNode::GetGrainLength()
{
  return GetNode()->GetGrainLength();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainLength(int lengthMS)
{
  GetNode()->SetGrainLength(lengthMS);
}

//**************************************************************************************************
int GranularSynthNode::GetGrainLengthVariance()
{
  return GetNode()->GetGrainLengthVariance();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainLengthVariance(int lengthVarianceMS)
{
  GetNode()->SetGrainLengthVariance(lengthVarianceMS);
}

//**************************************************************************************************
float GranularSynthNode::GetGrainResampleRate()
{
  return GetNode()->GetGrainResampleRate();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainResampleRate(float resampleRate)
{
  GetNode()->SetGrainResampleRate(resampleRate);
}

//**************************************************************************************************
float GranularSynthNode::GetGrainResampleRateVariance()
{
  return GetNode()->GetGrainResampleRateVariance();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainResampleRateVariance(float resampleVariance)
{
  GetNode()->SetGrainResampleRateVariance(resampleVariance);
}

//**************************************************************************************************
float GranularSynthNode::GetBufferScanRate()
{
  return GetNode()->GetBufferScanRate();
}

//**************************************************************************************************
void GranularSynthNode::SetBufferScanRate(float bufferRate)
{
  GetNode()->SetBufferScanRate(bufferRate);
}

//**************************************************************************************************
float GranularSynthNode::GetGrainPanningValue()
{
  return GetNode()->GetGrainPanningValue();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainPanningValue(float panValue)
{
  GetNode()->SetGrainPanningValue(panValue);
}

//**************************************************************************************************
float GranularSynthNode::GetGrainPanningVariance()
{
  return GetNode()->GetGrainPanningVariance();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainPanningVariance(float panValueVariance)
{
  GetNode()->SetGrainPanningVariance(panValueVariance);
}

//**************************************************************************************************
float GranularSynthNode::GetRandomLocationValue()
{
  return GetNode()->GetRandomLocationValue();
}

//**************************************************************************************************
void GranularSynthNode::SetRandomLocationValue(float randomLocationValue)
{
  GetNode()->SetRandomLocationValue(randomLocationValue);
}

//**************************************************************************************************
GranularSynthWindows::Enum GranularSynthNode::GetWindowType()
{
  Audio::GrainWindowTypes::Enum type = GetNode()->GetWindowType();
  if (type == Audio::GrainWindowTypes::Parabolic)
    return GranularSynthWindows::Parabolic;
  else if (type == Audio::GrainWindowTypes::RaisedCosine)
    return GranularSynthWindows::RaisedCosine;
  else if (type == Audio::GrainWindowTypes::Trapezoid)
    return GranularSynthWindows::Trapezoid;
  else
    return GranularSynthWindows::Linear;
}

//**************************************************************************************************
void GranularSynthNode::SetWindowType(GranularSynthWindows::Enum type)
{
  Audio::GrainWindowTypes::Enum windowType;
  if (type == GranularSynthWindows::Parabolic)
    windowType = Audio::GrainWindowTypes::Parabolic;
  else if (type == GranularSynthWindows::RaisedCosine)
    windowType = Audio::GrainWindowTypes::RaisedCosine;
  else if (type == GranularSynthWindows::Trapezoid)
    windowType = Audio::GrainWindowTypes::Trapezoid;
  else windowType = Audio::GrainWindowTypes::Linear;

  GetNode()->SetWindowType(windowType);
}

//**************************************************************************************************
int GranularSynthNode::GetWindowAttack()
{
  return GetNode()->GetWindowAttack();
}

//**************************************************************************************************
void GranularSynthNode::SetWindowAttack(int attackMS)
{
  GetNode()->SetWindowAttack(attackMS);
}

//**************************************************************************************************
int GranularSynthNode::GetWindowRelease()
{
  return GetNode()->GetWindowRelease();
}

//**************************************************************************************************
void GranularSynthNode::SetWindowRelease(int releaseMS)
{
  GetNode()->SetWindowRelease(releaseMS);
}

//**************************************************************************************************
Audio::GranularSynthNode* GranularSynthNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::GranularSynthNode*)mNode;
}

//---------------------------------------------------------------------------------- Save Audio Node

//**************************************************************************************************
ZilchDefineType(SaveAudioNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(SaveAudio);
  ZilchBindMethod(PlaySavedAudio);
  ZilchBindMethod(StopPlaying);
  ZilchBindMethod(ClearSavedAudio);
}

//**************************************************************************************************
SaveAudioNode::SaveAudioNode()
{
  mNode = new Audio::SaveAudioNode("SaveAudioNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
bool SaveAudioNode::GetSaveAudio()
{
  return GetNode()->GetSaveAudio();
}

//**************************************************************************************************
void SaveAudioNode::SetSaveAudio(bool save)
{
  GetNode()->SetSaveAudio(save);
}

//**************************************************************************************************
void SaveAudioNode::PlaySavedAudio()
{
  GetNode()->PlaySavedAudio();
}

//**************************************************************************************************
void SaveAudioNode::StopPlaying()
{
  GetNode()->StopPlaying();
}

//**************************************************************************************************
void SaveAudioNode::ClearSavedAudio()
{
  GetNode()->ClearSavedAudio();
}

//**************************************************************************************************
Audio::SaveAudioNode* SaveAudioNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::SaveAudioNode*)mNode;
}

}
