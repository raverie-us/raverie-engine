///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(NeedMoreSamples);
  DefineEvent(AudioInterpolationDone);
  DefineEvent(SoundNodeDisconnected);
}

//--------------------------------------------------------------------------------------- Sound Node

//**************************************************************************************************
ZilchDefineType(SoundNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindMethod(AddInputNode);
  ZilchBindMethod(InsertNodeAfter);
  ZilchBindMethod(InsertNodeBefore);
  ZilchBindMethod(ReplaceWith);
  ZilchBindMethod(RemoveInputNode);
  ZilchBindMethod(RemoveAllInputs);
  ZilchBindMethod(RemoveAllOutputs);
  ZilchBindMethod(RemoveAndAttachInputsToOutputs);
  ZilchBindGetterSetter(AutoCollapse);
  ZilchBindGetter(HasInputs);
  ZilchBindGetter(HasOutputs);
  ZilchBindGetter(InputCount);
  ZilchBindGetter(OutputCount);
  ZilchBindGetterSetter(BypassPercent);

  ZeroBindEvent(Events::AudioInterpolationDone, SoundEvent);
  ZeroBindEvent(Events::SoundNodeDisconnected, SoundEvent);
}

//**************************************************************************************************
SoundNode::SoundNode() :
  mNode(nullptr),
  mCanInsertAfter(true),
  mCanInsertBefore(true),
  mCanRemove(true),
  mCanReplace(true)
{

}

//**************************************************************************************************
SoundNode::~SoundNode()
{
  if (mNode)
    mNode->SetExternalInterface(nullptr);

}

//**************************************************************************************************
void SoundNode::AddInputNode(SoundNode* node)
{
  if (!node)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to add SoundNode to null object");
    return;
  }

  if (node == this)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to add SoundNode to itself as input");
    return;
  }

  if (mNode && node->mNode)
    mNode->AddInput(node->mNode);
}

//**************************************************************************************************
void SoundNode::InsertNodeAfter(SoundNode* node)
{
  if (!node)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to add sound node to null object");
    return;
  }

  if (node == this)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to insert SoundNode after itself");
    return;
  }

  // Make sure insertion is allowed
  if (!mCanInsertAfter || !node->mCanInsertBefore)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", 
      String::Format("InsertNodeAfter method not allowed with %s and %s", mNode->Name, node->mNode->Name));
    return;
  }

  if (mNode && node->mNode)
    mNode->InsertNodeAfter(node->mNode);
}

//**************************************************************************************************
void SoundNode::InsertNodeBefore(SoundNode* node)
{
  if (!node)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to add sound node to null object");
    return;
  }

  if (node == this)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to insert SoundNode before itself");
    return;
  }

  // Make sure insertion is allowed
  if (!mCanInsertBefore || !node->mCanInsertAfter)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", 
      String::Format("InsertNodeBefore method not allowed with %s and %s", mNode->Name, node->mNode->Name));
    return;
  }

  if (mNode && node->mNode)
    mNode->InsertNodeBefore(node->mNode);
}

//**************************************************************************************************
void SoundNode::ReplaceWith(SoundNode* node)
{
  if (!node)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to replace sound node with null object");
    return;
  }

  if (node == this)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to replace SoundNode with itself");
    return;
  }

  // Make sure this operation is allowed
  if (!mCanReplace)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", 
      String::Format("ReplaceWith method not allowed with %s", mNode->Name));
    return;
  }

  if (mNode && node->mNode)
    mNode->ReplaceWith(node->mNode);
}

//**************************************************************************************************
void SoundNode::RemoveInputNode(SoundNode* node)
{
  if (mNode && node->mNode)
    mNode->RemoveInput(node->mNode);
}

//**************************************************************************************************
void SoundNode::RemoveAllInputs()
{
  // Make sure this operation is allowed
  if (!mCanInsertBefore)
  {
    DoNotifyWarning("Incorrect SoundNode Operation",
      String::Format("RemoveAllInputs method not allowed with %s", mNode->Name));
    return;
  }

  if (mNode)
    mNode->DisconnectInputs();
}

//**************************************************************************************************
void SoundNode::RemoveAllOutputs()
{
  // Make sure this operation is allowed
  if (!mCanInsertAfter)
  {
    DoNotifyWarning("Incorrect SoundNode Operation",
      String::Format("RemoveAllOutputs method not allowed with %s", mNode->Name));
    return;
  }

  if (mNode)
    mNode->DisconnectOutputs();
}

//**************************************************************************************************
void SoundNode::RemoveAndAttachInputsToOutputs()
{
  // Make sure this operation is allowed
  if (!mCanRemove)
  {
    DoNotifyWarning("Incorrect SoundNode Operation",
      String::Format("RemoveAndAttachInputsToOutputs method not allowed with %s", mNode->Name));
    return;
  }

  if (mNode)
    mNode->DisconnectOnlyThis();
}

//**************************************************************************************************
bool SoundNode::GetAutoCollapse()
{
  if (mNode)
    return mNode->GetCollapse();
  else
    return false;
}

//**************************************************************************************************
void SoundNode::SetAutoCollapse(bool willCollapse)
{
  if (mNode)
    mNode->SetCollapse(willCollapse);
}

//**************************************************************************************************
bool SoundNode::GetHasInputs()
{
  if (mNode)
    return mNode->HasInputs();
  else
    return false;
}

//**************************************************************************************************
bool SoundNode::GetHasOutputs()
{
  if (mNode)
    return mNode->HasOutputs();
  else
    return false;
}

//**************************************************************************************************
int SoundNode::GetInputCount()
{
  if (mNode)
    return mNode->GetInputs()->Size();
  else
    return 0;
}

//**************************************************************************************************
int SoundNode::GetOutputCount()
{
  if (mNode)
    return mNode->GetOutputs()->Size();
  else
    return 0;
}

//**************************************************************************************************
float SoundNode::GetBypassPercent()
{
  if (mNode)
    return mNode->GetBypassPercent();
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundNode::SetBypassPercent(float percent)
{
  if (mNode)
    mNode->SetBypassPercent(percent);
}

//**************************************************************************************************
void SoundNode::SendAudioEvent(const Audio::AudioEventType eventType, void* data)
{
  if (eventType == Audio::AudioEventType::Notify_InterpolationDone)
  {
    SoundEvent event;
    DispatchEvent(Events::AudioInterpolationDone, &event);
  }
  else if (eventType == Audio::AudioEventType::Notify_NodeDisconnected)
  {
    SoundEvent event;
    DispatchEvent(Events::SoundNodeDisconnected, &event);
  }
}

//**************************************************************************************************
void SoundNode::SetNode(Audio::SoundNode* node, Status& status)
{
  if (status.Succeeded())
    mNode = node;
  else
  {
    if (node)
      node->DeleteThisNode();
    DoNotifyWarning("Audio Error", status.Message);
  }
}

//------------------------------------------------------------------------------------- Sound Buffer

//**************************************************************************************************
ZilchDefineType(SoundBuffer, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetter(SampleCount);
  ZilchBindMethod(AddSampleToBuffer);
  ZilchBindMethod(GetSampleAtIndex);
  ZilchBindMethod(Reset);
}

//**************************************************************************************************
void SoundBuffer::AddSampleToBuffer(float value)
{
  mBuffer.PushBack(value);
}

//**************************************************************************************************
int SoundBuffer::GetSampleRate()
{
  return mSampleRate;
}

//**************************************************************************************************
void SoundBuffer::SetSampleRate(int sampleRate)
{
  mSampleRate = sampleRate;
}

//**************************************************************************************************
int SoundBuffer::GetChannels()
{
  return mChannels;
}

//**************************************************************************************************
void SoundBuffer::SetChannels(int channels)
{
  mChannels = channels;
}

//**************************************************************************************************
int SoundBuffer::GetSampleCount()
{
  return mBuffer.Size();
}

//**************************************************************************************************
float SoundBuffer::GetSampleAtIndex(int index)
{
  if ((unsigned)index < mBuffer.Size())
    return mBuffer[index];
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundBuffer::Reset()
{
  mBuffer.Clear();
}

//-------------------------------------------------------------------------------- Custom Audio Node

//**************************************************************************************************
ZilchDefineType(CustomAudioNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(Channels);
  ZilchBindGetter(MinimumBufferSize);
  ZilchBindGetter(SystemSampleRate);
  ZilchBindMethod(SendBuffer);
  ZilchBindMethod(SendPartialBuffer);

  ZeroBindEvent(Events::NeedMoreSamples, ObjectEvent);
}

//**************************************************************************************************
CustomAudioNode::CustomAudioNode()
{
  Zero::Status status;
  SetNode(new Audio::InputNode(status, "CustomAudioNode", Z::gSound->mCounter++, this), status);
}

//**************************************************************************************************
CustomAudioNode::~CustomAudioNode()
{
  if (mNode)
  {
    mNode->DeleteThisNode();
  }
}

//**************************************************************************************************
int CustomAudioNode::GetMinimumBufferSize()
{
  if (mNode)
    return (int)((Audio::InputNode*)mNode)->GetMinimumBufferSize();
  else
    return 0;
}

//**************************************************************************************************
int CustomAudioNode::GetSystemSampleRate()
{
  if (mNode)
    return (int)((Audio::InputNode*)mNode)->GetSystemSampleRate();
  else
    return 0;
}

//**************************************************************************************************
int CustomAudioNode::GetChannels()
{
  if (mNode)
    return ((Audio::InputNode*)mNode)->GetNumberOfChannels();
  else
    return 0;
}

//**************************************************************************************************
void CustomAudioNode::SetChannels(int channels)
{
  if (mNode)
    ((Audio::InputNode*)mNode)->SetNumberOfChannels(channels);
}

//**************************************************************************************************
void CustomAudioNode::SendBuffer(SoundBuffer* buffer)
{
  if (mNode && buffer)
  {
    // Create the array to send
    float* newBuffer = new float[buffer->mBuffer.Size()];
    // Copy the data into the array
    memcpy(newBuffer, buffer->mBuffer.Data(), sizeof(float) * buffer->mBuffer.Size());
    // Send the array (will be deleted by the audio engine)
    ((Audio::InputNode*)mNode)->AddSamples(newBuffer, buffer->mBuffer.Size());
  }
}

//**************************************************************************************************
void CustomAudioNode::SendPartialBuffer(SoundBuffer* buffer, int startAtIndex, int howManySamples)
{
  if (mNode && buffer && (startAtIndex + howManySamples) <= (int)buffer->mBuffer.Size())
  {
    // Create the array to send
    float* newBuffer = new float[howManySamples];
    // Copy the data into the array
    memcpy(newBuffer, buffer->mBuffer.Data() + startAtIndex, sizeof(float) * howManySamples);
    // Send the array (will be deleted by the audio engine)
    ((Audio::InputNode*)mNode)->AddSamples(newBuffer, howManySamples);
  }
}

//**************************************************************************************************
void CustomAudioNode::SendAudioEvent(const Audio::AudioEventType eventType, void* data)
{
  if (eventType == Audio::Notify_NeedInputSamples)
  {
    SoundEvent event;
    mDispatcher.Dispatch(Events::NeedMoreSamples, &event);
  }
}

//------------------------------------------------------------------------------ Generated Wave Node

//**************************************************************************************************
ZilchDefineType(GeneratedWaveNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(WaveType); 
  ZilchBindGetterSetter(WaveFrequency);
  ZilchBindGetterSetter(Volume);
  ZilchBindGetterSetter(Decibels);
  ZilchBindMethod(Play);
  ZilchBindMethod(Stop);
  ZilchBindMethod(InterpolateVolume);
  ZilchBindMethod(InterpolateDecibels);
  ZilchBindMethod(InterpolateWaveFrequency);
}

//**************************************************************************************************
GeneratedWaveNode::GeneratedWaveNode() :
  mWaveType(SynthWaveType::SineWave),
  mWaveFrequency(440.0f), 
  mAsset(nullptr), 
  mVolume(1.0f)
{
  CreateInstance(true);
}

//**************************************************************************************************
GeneratedWaveNode::~GeneratedWaveNode()
{
  ReleaseInstance();
  ReleaseAsset();
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

  // If there is already an asset, release it and create a new one
  if (mAsset)
  {
    ReleaseAsset();
    CreateAsset();
  }
  
  // Check if there is a SoundInstance
  if (mNode)
  {
    // If not paused, stop the SoundInstance and create one which is not paused
    if (!((Audio::SoundInstanceNode*)mNode)->GetPaused())
    {
      ((Audio::SoundInstanceNode*)mNode)->Stop();
      CreateInstance(false);
    }
    // Otherwise create one which is paused
    else
      CreateInstance(true);
  }
}

//**************************************************************************************************
float GeneratedWaveNode::GetWaveFrequency()
{
  return mWaveFrequency;
}

//**************************************************************************************************
void GeneratedWaveNode::SetWaveFrequency(float frequency)
{
  InterpolateWaveFrequency(frequency, 0.0f);
}

//**************************************************************************************************
void GeneratedWaveNode::InterpolateWaveFrequency(float frequency, float time)
{
  mWaveFrequency = frequency;

  if (mAsset)
    ((Audio::GeneratedWaveSoundAsset*)mAsset)->SetFrequency(frequency, time);
}

//**************************************************************************************************
void GeneratedWaveNode::Play()
{
  // If there is a SoundInstance and it's paused, resume it
  if (mNode && ((Audio::SoundInstanceNode*)mNode)->GetPaused())
    ((Audio::SoundInstanceNode*)mNode)->Resume();
  // Otherwise create a new SoundInstance
  else
    CreateInstance(false);

}

//**************************************************************************************************
void GeneratedWaveNode::Stop()
{
  if (mNode)
  {
    ((Audio::SoundInstanceNode*)mNode)->Stop();
  }
}

//**************************************************************************************************
float GeneratedWaveNode::GetVolume()
{
  return mVolume;
}

//**************************************************************************************************
void GeneratedWaveNode::SetVolume(float volume)
{
  InterpolateVolume(volume, 0.0f);
}

//**************************************************************************************************
void GeneratedWaveNode::InterpolateVolume(float volume, float time)
{
  mVolume = volume;

  if (mNode)
    ((Audio::SoundInstanceNode*)mNode)->SetVolume(volume, time);
}

//**************************************************************************************************
float GeneratedWaveNode::GetDecibels()
{
  return Z::gSound->VolumeToDecibels(mVolume);
}

//**************************************************************************************************
void GeneratedWaveNode::SetDecibels(float decibels)
{
  InterpolateDecibels(decibels, 0.0f);
}

//**************************************************************************************************
void GeneratedWaveNode::InterpolateDecibels(float decibels, float time)
{
  mVolume = Z::gSound->DecibelsToVolume(decibels);

  if (mNode)
    ((Audio::SoundInstanceNode*)mNode)->SetVolume(mVolume, time);
}

//**************************************************************************************************
void GeneratedWaveNode::CreateAsset()
{
  if (mAsset)
    ReleaseAsset();

  Audio::OscillatorTypes waveType;

  switch (mWaveType)
  {
  case SynthWaveType::SineWave:
    waveType = Audio::Sine;
    break;
  case SynthWaveType::SawWave:
    waveType = Audio::Saw;
    break;
  case SynthWaveType::SquareWave:
    waveType = Audio::Square;
    break;
  case SynthWaveType::TriangleWave:
    waveType = Audio::Triangle;
    break;
  case SynthWaveType::Noise:
    waveType = Audio::Noise;
    break;
  }

  mAsset = new Audio::GeneratedWaveSoundAsset(waveType, mWaveFrequency, this);
}

//**************************************************************************************************
void GeneratedWaveNode::ReleaseAsset()
{
  if (mAsset)
  {
    mAsset->SetExternalInterface(nullptr);
    mAsset = nullptr;
  }
}

//**************************************************************************************************
void GeneratedWaveNode::CreateInstance(bool paused)
{
  // If there currently is a node, stop it 
  if (mNode)
    Stop();

  // If there is no asset, create it
  if (!mAsset)
    CreateAsset();

  // Create the new sound instance
  Status status;
  Audio::SoundInstanceNode* newNode = new Audio::SoundInstanceNode(status, "GeneratedWaveInstance", 
    Z::gSound->mCounter++, mAsset, true, true, this);
  // If it wasn't successful delete the new node
  if (status.Failed())
  {
    if (newNode)
      newNode->DeleteThisNode();

    DoNotifyWarning("Audio Error", "GeneratedWaveNode could not be created with current settings");
  }
  else
  {
    // If there currently is a node, swap it with the new one in the graph
    if (mNode)
    {
      mNode->ReplaceWith(newNode);
      ReleaseInstance();
    }

    mNode = newNode;

    // Set the volume on the new node
    ((Audio::SoundInstanceNode*)mNode)->SetVolume(mVolume, 0.0f);
    // If it shouldn't be paused, resume it
    if (!paused)
      ((Audio::SoundInstanceNode*)mNode)->Resume();
  }
}

//**************************************************************************************************
void GeneratedWaveNode::ReleaseInstance()
{
  if (mNode)
  {
    ((Audio::SoundInstanceNode*)mNode)->Stop();
    mNode->DeleteThisNode();
    mNode = nullptr;
  }
}

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
  Zero::Status status;
  SetNode(new Audio::VolumeNode(status, "VolumeNode", Z::gSound->mCounter++, this), status);
}

//**************************************************************************************************
float VolumeNode::GetVolume()
{
  if (mNode)
    return ((Audio::VolumeNode*)mNode)->GetVolume();
  else
    return 0.0f;
}

//**************************************************************************************************
void VolumeNode::SetVolume(float volume)
{
  InterpolateVolume(volume, 0.0f);
}

//**************************************************************************************************
void VolumeNode::InterpolateVolume(float volume, float time)
{
  if (mNode)
    ((Audio::VolumeNode*)mNode)->SetVolume(volume, time);
}

//**************************************************************************************************
float VolumeNode::GetDecibels()
{
  if (mNode)
    return Z::gSound->VolumeToDecibels(((Audio::VolumeNode*)mNode)->GetVolume());
  else
    return 0.0f;
}

//**************************************************************************************************
void VolumeNode::SetDecibels(float volumeDB)
{
  InterpolateDecibels(volumeDB, 0.0f);
}

//**************************************************************************************************
void VolumeNode::InterpolateDecibels(float volumeDB, float time)
{
  if (mNode)
    ((Audio::VolumeNode*)mNode)->SetVolume(Z::gSound->DecibelsToVolume(volumeDB), time);
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
  Zero::Status status;
  SetNode(new Audio::PanningNode(status, "PanningNode", Z::gSound->mCounter++, this), status);
}

//**************************************************************************************************
bool PanningNode::GetSumToMono()
{
  if (mNode)
    return ((Audio::PanningNode*)mNode)->GetSumToMono();
  else
    return false;
}

//**************************************************************************************************
void PanningNode::SetSumToMono(bool isMono)
{
  if (mNode)
    ((Audio::PanningNode*)mNode)->SetSumToMono(isMono);
}

//**************************************************************************************************
float PanningNode::GetLeftVolume()
{
  if (mNode)
    return ((Audio::PanningNode*)mNode)->GetLeftVolume();
  else
    return 0.0f;
}

//**************************************************************************************************
void PanningNode::SetLeftVolume(float volume)
{
  InterpolateLeftVolume(volume, 0.0f);
}

//**************************************************************************************************
void PanningNode::InterpolateLeftVolume(float volume, float time)
{
  if (mNode)
    ((Audio::PanningNode*)mNode)->SetLeftVolume(volume, time);
}

//**************************************************************************************************
float PanningNode::GetRightVolume()
{
  if (mNode)
    return ((Audio::PanningNode*)mNode)->GetRightVolume();
  else
    return 0.0f;
}

//**************************************************************************************************
void PanningNode::SetRightVolume(float volume)
{
  InterpolateRightVolume(volume, 0.0f);
}

//**************************************************************************************************
void PanningNode::InterpolateRightVolume(float volume, float time)
{
  if (mNode)
    ((Audio::PanningNode*)mNode)->SetRightVolume(volume, time);
}

//**************************************************************************************************
void PanningNode::InterpolateVolumes(float leftVolume, float rightVolume, float time)
{
  if (mNode)
  {
    ((Audio::PanningNode*)mNode)->SetLeftVolume(leftVolume, time);
    ((Audio::PanningNode*)mNode)->SetRightVolume(rightVolume, time);
  }
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
  Zero::Status status;
  SetNode(new Audio::PitchNode(status, "PitchNode", Z::gSound->mCounter++, this), status);
}

//**************************************************************************************************
float PitchNode::GetPitch()
{
  if (mNode)
    return Z::gSound->SemitonesToPitch(((Audio::PitchNode*)mNode)->GetPitch() / 100.0f);
  else
    return 0.0f;
}

//**************************************************************************************************
void PitchNode::SetPitch(float pitchRatio)
{
  InterpolatePitch(pitchRatio, 0.0f);
}

//**************************************************************************************************
void PitchNode::InterpolatePitch(float pitchRatio, float time)
{
  if (mNode)
    ((Audio::PitchNode*)mNode)->SetPitch((int)(Z::gSound->PitchToSemitones(pitchRatio) * 100.0f), time);
}

//**************************************************************************************************
float PitchNode::GetSemitones()
{
  if (mNode)
    return ((Audio::PitchNode*)mNode)->GetPitch() / 100.0f;
  else
    return 0.0f;
}

//**************************************************************************************************
void PitchNode::SetSemitones(float pitchSemitones)
{
  InterpolateSemitones(pitchSemitones, 0.0f);
}

//**************************************************************************************************
void PitchNode::InterpolateSemitones(float pitchSemitones, float time)
{
  if (mNode)
    ((Audio::PitchNode*)mNode)->SetPitch((int)(pitchSemitones * 100), time);
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
  Zero::Status status;
  SetNode(new Audio::LowPassNode(status, "LowPassNode", Z::gSound->mCounter++, this), status);
}

//**************************************************************************************************
float LowPassNode::GetCutoffFrequency()
{
  if (mNode)
    return ((Audio::LowPassNode*)mNode)->GetCutoffFrequency();
  else
    return 0.0f;
}

//**************************************************************************************************
void LowPassNode::SetCutoffFrequency(float frequency)
{
  if (mNode)
    ((Audio::LowPassNode*)mNode)->SetCutoffFrequency(frequency);
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
  Status status;
  SetNode(new Audio::HighPassNode(status, "HighPassNode", Z::gSound->mCounter++, this), status);
}

//**************************************************************************************************
float HighPassNode::GetCutoffFrequency()
{
  if (mNode)
    return ((Audio::HighPassNode*)mNode)->GetCutoffFrequency();
  else
    return 0.0f;
}

//**************************************************************************************************
void HighPassNode::SetCutoffFrequency(float frequency)
{
  if (mNode)
    ((Audio::HighPassNode*)mNode)->SetCutoffFrequency(frequency);
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
  Status status;
  SetNode(new Audio::BandPassNode(status, "BandPassNode", Z::gSound->mCounter++, this), status);
}

//**************************************************************************************************
float BandPassNode::GetCentralFrequency()
{
  if (mNode)
    return ((Audio::BandPassNode*)mNode)->GetCentralFrequency();
  else
    return 0.0f;
}

//**************************************************************************************************
void BandPassNode::SetCentralFrequency(float frequency)
{
  if (mNode)
    ((Audio::BandPassNode*)mNode)->SetCentralFrequency(frequency);
}

//**************************************************************************************************
float BandPassNode::GetQualityFactor()
{
  if (mNode)
    return ((Audio::BandPassNode*)mNode)->GetQuality();
  else
    return 0.0f;
}

//**************************************************************************************************
void BandPassNode::SetQualityFactor(float Q)
{
  if (mNode)
    ((Audio::BandPassNode*)mNode)->SetQuality(Q);
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
  Status status;
  SetNode(new Audio::EqualizerNode(status, "EqualizerNode", Z::gSound->mCounter++, this), status);
}

//**************************************************************************************************
float EqualizerNode::GetLowPassGain()
{
  if (mNode)
    return ((Audio::EqualizerNode*)mNode)->GetBelow80HzGain();
  else
    return 0.0f;
}

//**************************************************************************************************
void EqualizerNode::SetLowPassGain(float gain)
{
  if (mNode)
    ((Audio::EqualizerNode*)mNode)->SetBelow80HzGain(gain);
}

//**************************************************************************************************
float EqualizerNode::GetHighPassGain()
{
  if (mNode)
    return ((Audio::EqualizerNode*)mNode)->GetAbove5000HzGain();
  else
    return 0.0f;
}

//**************************************************************************************************
void EqualizerNode::SetHighPassGain(float gain)
{
  if (mNode)
    ((Audio::EqualizerNode*)mNode)->SetAbove5000HzGain(gain);
}

//**************************************************************************************************
float EqualizerNode::GetBand1Gain()
{
  if (mNode)
    return ((Audio::EqualizerNode*)mNode)->Get150HzGain();
  else
    return 0.0f;
}

//**************************************************************************************************
void EqualizerNode::SetBand1Gain(float gain)
{
  if (mNode)
    ((Audio::EqualizerNode*)mNode)->Set150HzGain(gain);
}

//**************************************************************************************************
float EqualizerNode::GetBand2Gain()
{
  if (mNode)
    return ((Audio::EqualizerNode*)mNode)->Get600HzGain();
  else
    return 0.0f;
}

//**************************************************************************************************
void EqualizerNode::SetBand2Gain(float gain)
{
  if (mNode)
    ((Audio::EqualizerNode*)mNode)->Set600HzGain(gain);
}

//**************************************************************************************************
float EqualizerNode::GetBand3Gain()
{
  if (mNode)
    return ((Audio::EqualizerNode*)mNode)->Get2500HzGain();
  else
    return 0.0f;
}

//**************************************************************************************************
void EqualizerNode::SetBand3Gain(float gain)
{
  if (mNode)
    ((Audio::EqualizerNode*)mNode)->Set2500HzGain(gain);
}

//**************************************************************************************************
void EqualizerNode::InterpolateAllBands(float lowPass, float band1, float band2, float band3, 
  float highPass, float timeToInterpolate)
{
  if (mNode)
    ((Audio::EqualizerNode*)mNode)->InterpolateBands(Audio::EqualizerBandGains(lowPass, band1,
      band2, band3, highPass), timeToInterpolate);
}

//-------------------------------------------------------------------------------------- Reverb Node

//**************************************************************************************************
ZilchDefineType(ReverbNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(Length);
  ZilchBindGetterSetter(WetPercent);
  ZilchBindMethod(InterpolateWetPercent);
}

//**************************************************************************************************
ReverbNode::ReverbNode()
{
  Status status;
  SetNode(new Audio::ReverbNode(status, "ReverbNode", Z::gSound->mCounter++, this), status);
}

//**************************************************************************************************
float ReverbNode::GetLength()
{
  if (mNode)
    return ((Audio::ReverbNode*)mNode)->GetTime() / 1000.0f;
  else
    return 0.0f;
}

//**************************************************************************************************
void ReverbNode::SetLength(float time)
{
  if (mNode)
    ((Audio::ReverbNode*)mNode)->SetTime(time * 1000);
}

//**************************************************************************************************
float ReverbNode::GetWetPercent()
{
  if (mNode)
    return ((Audio::ReverbNode*)mNode)->GetWetPercent();
  else
    return 0;
}

//**************************************************************************************************
void ReverbNode::SetWetPercent(float percent)
{
  if (mNode)
    ((Audio::ReverbNode*)mNode)->SetWetPercent(percent);
}

//**************************************************************************************************
void ReverbNode::InterpolateWetPercent(float percent, float time)
{
  if (mNode)
    ((Audio::ReverbNode*)mNode)->InterpolateWetPercent(percent, time);
}

//--------------------------------------------------------------------------------------- Delay Node

//**************************************************************************************************
ZilchDefineType(DelayNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(Delay);
  ZilchBindGetterSetter(FeedbackPercent);
  ZilchBindGetterSetter(WetPercent);
  ZilchBindMethod(InterpolateWetPercent);
}

//**************************************************************************************************
DelayNode::DelayNode()
{
  Status status;
  SetNode(new Audio::DelayNode(status, "DelayNode", Z::gSound->mCounter++, this), status);
}

//**************************************************************************************************
float DelayNode::GetDelay()
{
  if (mNode)
    return ((Audio::DelayNode*)mNode)->GetDelayMSec() / 1000.0f;
  else
    return 0.0f;
}

//**************************************************************************************************
void DelayNode::SetDelay(float seconds)
{
  if (mNode)
    ((Audio::DelayNode*)mNode)->SetDelayMSec(seconds * 1000.0f);
}

//**************************************************************************************************
float DelayNode::GetFeedbackPercent()
{
  if (mNode)
    return ((Audio::DelayNode*)mNode)->GetFeedbackPct();
  else
    return 0.0f;
}

//**************************************************************************************************
void DelayNode::SetFeedbackPercent(float feedback)
{
  if (mNode)
    ((Audio::DelayNode*)mNode)->SetFeedbackPct(feedback);
}

//**************************************************************************************************
float DelayNode::GetWetPercent()
{
  if (mNode)
    return ((Audio::DelayNode*)mNode)->GetWetLevelPct();
  else
    return 0.0f;
}

//**************************************************************************************************
void DelayNode::SetWetPercent(float wetLevel)
{
  if (mNode)
    ((Audio::DelayNode*)mNode)->SetWetLevelPct(wetLevel);
}

//**************************************************************************************************
void DelayNode::InterpolateWetPercent(float percent, float time)
{
  if (mNode)
    ((Audio::DelayNode*)mNode)->InterpolateWetLevelPct(percent, time);
}

//------------------------------------------------------------------------------------- Flanger Node

//**************************************************************************************************
ZilchDefineType(FlangerNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(MaxDelayMillisec);
  ZilchBindGetterSetter(ModulationFrequency);
  ZilchBindGetterSetter(FeedbackPercent);
}

//**************************************************************************************************
FlangerNode::FlangerNode()
{
  Status status;
  SetNode(new Audio::FlangerNode(status, "FlangerNode", Z::gSound->mCounter++, this), status);
}

//**************************************************************************************************
float FlangerNode::GetMaxDelayMillisec()
{
  if (mNode)
    return ((Audio::FlangerNode*)mNode)->GetMaxDelayMSec();
  else
    return 0.0f;
}

//**************************************************************************************************
void FlangerNode::SetMaxDelayMillisec(float delay)
{
  if (mNode)
    ((Audio::FlangerNode*)mNode)->SetMaxDelayMSec(delay);
}

//**************************************************************************************************
float FlangerNode::GetModulationFrequency()
{
  if (mNode)
    return ((Audio::FlangerNode*)mNode)->GetModFrequency();
  else
    return 0.0f;
}

//**************************************************************************************************
void FlangerNode::SetModulationFrequency(float frequency)
{
  if (mNode)
    ((Audio::FlangerNode*)mNode)->SetModFrequency(frequency);
}

//**************************************************************************************************
float FlangerNode::GetFeedbackPercent()
{
  if (mNode)
    return ((Audio::FlangerNode*)mNode)->GetFeedbackPct();
  else
    return 0.0f;
}

//**************************************************************************************************
void FlangerNode::SetFeedbackPercent(float percent)
{
  if (mNode)
    ((Audio::FlangerNode*)mNode)->SetFeedbackPct(percent);
}

//-------------------------------------------------------------------------------------- Chorus Node

//**************************************************************************************************
ZilchDefineType(ChorusNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(MaxDelayMillisec);
  ZilchBindGetterSetter(MinDelayMillisec);
  ZilchBindGetterSetter(ModulationFrequency);
  ZilchBindGetterSetter(FeedbackPercent);
  ZilchBindGetterSetter(OffsetMillisec);
}

//**************************************************************************************************
ChorusNode::ChorusNode()
{
  Status status;
  SetNode(new Audio::ChorusNode(status, "ChorusNode", Z::gSound->mCounter++, this), status);
}

//**************************************************************************************************
float ChorusNode::GetMaxDelayMillisec()
{
  if (mNode)
    return ((Audio::ChorusNode*)mNode)->GetMaxDelayMSec();
  else
    return 0.0f;
}

//**************************************************************************************************
void ChorusNode::SetMaxDelayMillisec(float delay)
{
  if (mNode)
    ((Audio::ChorusNode*)mNode)->SetMaxDelayMSec(delay);
}

//**************************************************************************************************
float ChorusNode::GetMinDelayMillisec()
{
  if (mNode)
    return ((Audio::ChorusNode*)mNode)->GetMinDelayMSec();
  else
    return 0.0f;
}

//**************************************************************************************************
void ChorusNode::SetMinDelayMillisec(float delay)
{
  if (mNode)
    ((Audio::ChorusNode*)mNode)->SetMinDelayMSec(delay);
}

//**************************************************************************************************
float ChorusNode::GetModulationFrequency()
{
  if (mNode)
    return ((Audio::ChorusNode*)mNode)->GetModFrequency();
  else
    return 0.0f;
}

//**************************************************************************************************
void ChorusNode::SetModulationFrequency(float frequency)
{
  if (mNode)
    ((Audio::ChorusNode*)mNode)->SetModFrequency(frequency);
}

//**************************************************************************************************
float ChorusNode::GetFeedbackPercent()
{
  if (mNode)
    return ((Audio::ChorusNode*)mNode)->GetFeedbackPct();
  else
    return 0.0f;
}

//**************************************************************************************************
void ChorusNode::SetFeedbackPercent(float percent)
{
  if (mNode)
    ((Audio::ChorusNode*)mNode)->SetFeedbackPct(percent);
}

//**************************************************************************************************
float ChorusNode::GetOffsetMillisec()
{
  if (mNode)
    return ((Audio::ChorusNode*)mNode)->GetOffsetMSec();
  else
    return 0.0f;
}

//**************************************************************************************************
void ChorusNode::SetOffsetMillisec(float offset)
{
  if (mNode)
    ((Audio::ChorusNode*)mNode)->SetOffsetMSec(offset);
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
  Status status;

  Audio::DynamicsProcessorNode* node = new Audio::DynamicsProcessorNode(status, 
    "DynamicsCompressionNode", Z::gSound->mCounter++, this);
  node->SetType(Audio::ProcessorTypes::Compressor);

  SetNode(node, status);
}

//**************************************************************************************************
float CompressorNode::GetInputGainDecibels()
{
  if (mNode)
    return ((Audio::DynamicsProcessorNode*)mNode)->GetInputGain();
  else
    return 0.0f;
}

//**************************************************************************************************
void CompressorNode::SetInputGainDecibels(float gain)
{
  if (mNode)
    ((Audio::DynamicsProcessorNode*)mNode)->SetInputGain(gain);
}

//**************************************************************************************************
float CompressorNode::GetThresholdDecibels()
{
  if (mNode)
    return ((Audio::DynamicsProcessorNode*)mNode)->GetThreshold();
  else
    return 0.0f;
}

//**************************************************************************************************
void CompressorNode::SetThresholdDecibels(float dB)
{
  if (mNode)
    ((Audio::DynamicsProcessorNode*)mNode)->SetThreshold(dB);
}

//**************************************************************************************************
float CompressorNode::GetAttackMillisec()
{
  if (mNode)
    return ((Audio::DynamicsProcessorNode*)mNode)->GetAttackMSec();
  else
    return 0.0f;
}

//**************************************************************************************************
void CompressorNode::SetAttackMillisec(float attack)
{
  if (mNode)
    ((Audio::DynamicsProcessorNode*)mNode)->SetAttackMSec(attack);
}

//**************************************************************************************************
float CompressorNode::GetReleaseMillisec()
{
  if (mNode)
    return ((Audio::DynamicsProcessorNode*)mNode)->GetReleaseMSec();
  else
    return 0.0f;
}

//**************************************************************************************************
void CompressorNode::SetReleaseMillisec(float release)
{
  if (mNode)
    ((Audio::DynamicsProcessorNode*)mNode)->SetReleaseMsec(release);
}

//**************************************************************************************************
float CompressorNode::GetRatio()
{
  if (mNode)
    return ((Audio::DynamicsProcessorNode*)mNode)->GetRatio();
  else
    return 0.0f;
}

//**************************************************************************************************
void CompressorNode::SetRatio(float ratio)
{
  if (mNode)
    ((Audio::DynamicsProcessorNode*)mNode)->SetRatio(ratio);
}

//**************************************************************************************************
float CompressorNode::GetOutputGainDecibels()
{
  if (mNode)
    return ((Audio::DynamicsProcessorNode*)mNode)->GetOutputGain();
  else
    return 0.0f;
}

//**************************************************************************************************
void CompressorNode::SetOutputGainDecibels(float gain)
{
  if (mNode)
    ((Audio::DynamicsProcessorNode*)mNode)->SetOutputGain(gain);
}

//**************************************************************************************************
float CompressorNode::GetKneeWidth()
{
  if (mNode)
    return ((Audio::DynamicsProcessorNode*)mNode)->GetKneeWidth();
  else
    return 0.0f;
}

//**************************************************************************************************
void CompressorNode::SetKneeWidth(float knee)
{
  if (mNode)
    ((Audio::DynamicsProcessorNode*)mNode)->SetKneeWidth(knee);
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
  Status status;

  Audio::DynamicsProcessorNode* node = new Audio::DynamicsProcessorNode(status,
    "DynamicsCompressionNode", Z::gSound->mCounter++, this);
  node->SetType(Audio::ProcessorTypes::Expander);

  SetNode(node, status);
}

//**************************************************************************************************
float ExpanderNode::GetInputGainDecibels()
{
  if (mNode)
    return ((Audio::DynamicsProcessorNode*)mNode)->GetInputGain();
  else
    return 0.0f;
}

//**************************************************************************************************
void ExpanderNode::SetInputGainDecibels(float gain)
{
  if (mNode)
    ((Audio::DynamicsProcessorNode*)mNode)->SetInputGain(gain);
}

//**************************************************************************************************
float ExpanderNode::GetThresholdDecibels()
{
  if (mNode)
    return ((Audio::DynamicsProcessorNode*)mNode)->GetThreshold();
  else
    return 0.0f;
}

//**************************************************************************************************
void ExpanderNode::SetThresholdDecibels(float dB)
{
  if (mNode)
    ((Audio::DynamicsProcessorNode*)mNode)->SetThreshold(dB);
}

//**************************************************************************************************
float ExpanderNode::GetAttackMillisec()
{
  if (mNode)
    return ((Audio::DynamicsProcessorNode*)mNode)->GetAttackMSec();
  else
    return 0.0f;
}

//**************************************************************************************************
void ExpanderNode::SetAttackMillisec(float attack)
{
  if (mNode)
    ((Audio::DynamicsProcessorNode*)mNode)->SetAttackMSec(attack);
}

//**************************************************************************************************
float ExpanderNode::GetReleaseMillisec()
{
  if (mNode)
    return ((Audio::DynamicsProcessorNode*)mNode)->GetReleaseMSec();
  else
    return 0.0f;
}

//**************************************************************************************************
void ExpanderNode::SetReleaseMillisec(float release)
{
  if (mNode)
    ((Audio::DynamicsProcessorNode*)mNode)->SetReleaseMsec(release);
}

//**************************************************************************************************
float ExpanderNode::GetRatio()
{
  if (mNode)
    return ((Audio::DynamicsProcessorNode*)mNode)->GetRatio();
  else
    return 0.0f;
}

//**************************************************************************************************
void ExpanderNode::SetRatio(float ratio)
{
  if (mNode)
    ((Audio::DynamicsProcessorNode*)mNode)->SetRatio(ratio);
}

//**************************************************************************************************
float ExpanderNode::GetOutputGainDecibels()
{
  if (mNode)
    return ((Audio::DynamicsProcessorNode*)mNode)->GetOutputGain();
  else
    return 0.0f;
}

//**************************************************************************************************
void ExpanderNode::SetOutputGainDecibels(float gain)
{
  if (mNode)
    ((Audio::DynamicsProcessorNode*)mNode)->SetOutputGain(gain);
}

//**************************************************************************************************
float ExpanderNode::GetKneeWidth()
{
  if (mNode)
    return ((Audio::DynamicsProcessorNode*)mNode)->GetKneeWidth();
  else
    return 0.0f;
}

//**************************************************************************************************
void ExpanderNode::SetKneeWidth(float knee)
{
  if (mNode)
    ((Audio::DynamicsProcessorNode*)mNode)->SetKneeWidth(knee);
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
  Status status;
  SetNode(new Audio::RecordNode(status, "RecordingNode", Z::gSound->mCounter++, this), status);
}

//**************************************************************************************************
RecordingNode::~RecordingNode()
{
  StopRecording();
}

//**************************************************************************************************
String RecordingNode::GetFileName()
{
  if (mNode)
    return ((Audio::RecordNode*)mNode)->GetFileName();
  else
    return String();
}

//**************************************************************************************************
void RecordingNode::SetFileName(String& fileName)
{
  if (mNode)
    ((Audio::RecordNode*)mNode)->SetFileName(fileName);
}

//**************************************************************************************************
void RecordingNode::StartRecording()
{
  if (mNode)
    ((Audio::RecordNode*)mNode)->StartRecording();
}

//**************************************************************************************************
void RecordingNode::StopRecording()
{
  if (mNode)
    ((Audio::RecordNode*)mNode)->StopRecording();
}

//**************************************************************************************************
bool RecordingNode::GetPaused()
{
  if (mNode)
    return ((Audio::RecordNode*)mNode)->GetPaused();
  else
    return false;
}

//**************************************************************************************************
void RecordingNode::SetPaused(bool paused)
{
  if (mNode)
    ((Audio::RecordNode*)mNode)->SetPaused(paused);
}

//**************************************************************************************************
bool RecordingNode::GetStreamToDisk()
{
  if (mNode)
    return ((Audio::RecordNode*)mNode)->GetStreamToDisk();
  else
    return false;
}

//**************************************************************************************************
void RecordingNode::SetStreamToDisk(bool stream)
{
  if (mNode)
    ((Audio::RecordNode*)mNode)->SetStreamToDisk(stream);
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
  Status status;
  SetNode(new Audio::AddNoiseNode(status, "AddNoiseNode", Z::gSound->mCounter++, this), status);
}

//**************************************************************************************************
float AddNoiseNode::GetAdditiveGain()
{
  if (mNode)
    return ((Audio::AddNoiseNode*)mNode)->GetAdditiveNoiseGainDB();
  else
    return 0.0f;
}

//**************************************************************************************************
void AddNoiseNode::SetAdditiveGain(float decibels)
{
  if (mNode)
    ((Audio::AddNoiseNode*)mNode)->SetAdditiveNoiseGainDB(decibels);
}

//**************************************************************************************************
float AddNoiseNode::GetMultiplicativeGain()
{
  if (mNode)
    return ((Audio::AddNoiseNode*)mNode)->GetMultipleNoiseGainDB();
  else
    return 0.0f;
}

//**************************************************************************************************
void AddNoiseNode::SetMultiplicativeGain(float decibels)
{
  if (mNode)
    ((Audio::AddNoiseNode*)mNode)->SetMultipleNoiseGainDB(decibels);
}

//**************************************************************************************************
float AddNoiseNode::GetAdditiveCutoff()
{
  if (mNode)
    return ((Audio::AddNoiseNode*)mNode)->GetAdditiveCutoffHz();
  else
    return 0.0f;
}

//**************************************************************************************************
void AddNoiseNode::SetAdditiveCutoff(float frequency)
{
  if (mNode)
    ((Audio::AddNoiseNode*)mNode)->SetAdditiveCutoffHz(frequency);
}

//**************************************************************************************************
float AddNoiseNode::GetMultiplicativeCutoff()
{
  if (mNode)
    return ((Audio::AddNoiseNode*)mNode)->GetMultipleCutoffHz();
  else
    return 0.0;
}

//**************************************************************************************************
void AddNoiseNode::SetMultiplicativeCutoff(float frequency)
{
  if (mNode)
    ((Audio::AddNoiseNode*)mNode)->SetMultipleCutoffHz(frequency);
}

//------------------------------------------------------------------------------------ ADSR Envelope

//**************************************************************************************************
ZilchDefineType(AdsrEnvelope, builder, type)
{
  ZilchBindDefaultConstructor();
  ZeroBindDocumented();

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
  Status status;
  SetNode(new Audio::AdditiveSynthNode(status, "AdditiveSynthNode", Z::gSound->mCounter++, this), status);
}

//**************************************************************************************************
void AdditiveSynthNode::AddHarmonic(float multiplier, float volume, AdsrEnvelope envelope, 
  SynthWaveType::Enum type)
{
  if (mNode)
  {
    Audio::EnvelopeSettings envelope(envelope.DelayTime, envelope.AttackTime,
      envelope.DecayTime, envelope.SustainTime, envelope.SustainLevel, envelope.ReleaseTime);
    Audio::OscillatorTypes oscType;
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

    ((Audio::AdditiveSynthNode*)mNode)->AddHarmonic(Audio::HarmonicData(multiplier, 
      volume, envelope, oscType));
  }
}

//**************************************************************************************************
void AdditiveSynthNode::RemoveAllHarmonics()
{
  if (mNode)
    ((Audio::AdditiveSynthNode*)mNode)->RemoveAllHarmonics();
}

//**************************************************************************************************
void AdditiveSynthNode::NoteOn(float midiNote, float volume)
{
  if (mNode)
    ((Audio::AdditiveSynthNode*)mNode)->NoteOn((int)midiNote, volume);
}

//**************************************************************************************************
void AdditiveSynthNode::NoteOff(float midiNote)
{
  if (mNode)
    ((Audio::AdditiveSynthNode*)mNode)->NoteOff((int)midiNote);
}

//**************************************************************************************************
void AdditiveSynthNode::StopAllNotes()
{
  if (mNode)
    ((Audio::AdditiveSynthNode*)mNode)->StopAll();
}

//---------------------------------------------------------------------------------- Modulation Node

//**************************************************************************************************
ZilchDefineType(ModulationNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(UseAmplitudeModulation);
  ZilchBindGetterSetter(Frequency);
  ZilchBindGetterSetter(WetPercent);
}

//**************************************************************************************************
ModulationNode::ModulationNode()
{
  Status status;
  SetNode(new Audio::ModulationNode(status, "ModulationNode", Z::gSound->mCounter++, this), status);
}

//**************************************************************************************************
bool ModulationNode::GetUseAmplitudeModulation()
{
  if (mNode)
    return ((Audio::ModulationNode*)mNode)->GetUsingAmplitude();
  else
    return false;
}

//**************************************************************************************************
void ModulationNode::SetUseAmplitudeModulation(bool useAmplitude)
{
  if (mNode)
    ((Audio::ModulationNode*)mNode)->SetUsingAmplitude(useAmplitude);
}

//**************************************************************************************************
float ModulationNode::GetFrequency()
{
  if (mNode)
    return ((Audio::ModulationNode*)mNode)->GetFrequency();
  else
    return 0.0f;
}

//**************************************************************************************************
void ModulationNode::SetFrequency(float frequency)
{
  if (mNode)
    ((Audio::ModulationNode*)mNode)->SetFrequency(frequency);
}

//**************************************************************************************************
float ModulationNode::GetWetPercent()
{
  if (mNode)
    return ((Audio::ModulationNode*)mNode)->GetWetPercent();
  else
    return 0.0f;
}

//**************************************************************************************************
void ModulationNode::SetWetPercent(float percent)
{
  if (mNode)
    ((Audio::ModulationNode*)mNode)->SetWetPercent(percent);
}

}
