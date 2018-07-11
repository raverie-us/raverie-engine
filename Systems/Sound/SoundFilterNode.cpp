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
  DefineEvent(CustomAudioNodeSamplesNeeded);
  DefineEvent(AudioInterpolationDone);
  DefineEvent(SoundNodeDisconnected);
}

//-------------------------------------------------------------------------- Custom Audio Node Event

//**************************************************************************************************
ZilchDefineType(CustomAudioNodeEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindField(SamplesNeeded);
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
  ZilchBindGetterSetter(BypassPercent)->AddAttribute(DeprecatedAttribute);
  ZilchBindGetterSetter(BypassValue);

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

  ErrorIf(!mNode || !node->mNode, "SoundNode data is null");

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
      String::Format("InsertNodeAfter method not allowed with %s and %s", mNode->Name.c_str(), node->mNode->Name.c_str()));
    return;
  }

  ErrorIf(!mNode || !node->mNode, "SoundNode data is null");

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
      String::Format("InsertNodeBefore method not allowed with %s and %s", mNode->Name.c_str(), node->mNode->Name.c_str()));
    return;
  }

  ErrorIf(!mNode || !node->mNode, "SoundNode data is null");

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
      String::Format("ReplaceWith method not allowed with %s", mNode->Name.c_str()));
    return;
  }

  ErrorIf(!mNode || !node->mNode, "SoundNode data is null");

  mNode->ReplaceWith(node->mNode);
}

//**************************************************************************************************
void SoundNode::RemoveInputNode(SoundNode* node)
{
  if (!node)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to remove a null object from sound node input");
    return;
  }

  ErrorIf(!mNode || !node->mNode, "SoundNode data is null");

  mNode->RemoveInput(node->mNode);
}

//**************************************************************************************************
void SoundNode::RemoveAllInputs()
{
  // Make sure this operation is allowed
  if (!mCanInsertBefore)
  {
    DoNotifyWarning("Incorrect SoundNode Operation",
      String::Format("RemoveAllInputs method not allowed with %s", mNode->Name.c_str()));
    return;
  }

  ErrorIf(!mNode, "SoundNode data is null");

  mNode->DisconnectInputs();
}

//**************************************************************************************************
void SoundNode::RemoveAllOutputs()
{
  // Make sure this operation is allowed
  if (!mCanInsertAfter)
  {
    DoNotifyWarning("Incorrect SoundNode Operation",
      String::Format("RemoveAllOutputs method not allowed with %s", mNode->Name.c_str()));
    return;
  }

  ErrorIf(!mNode, "SoundNode data is null");

  mNode->DisconnectOutputs();
}

//**************************************************************************************************
void SoundNode::RemoveAndAttachInputsToOutputs()
{
  // Make sure this operation is allowed
  if (!mCanRemove)
  {
    DoNotifyWarning("Incorrect SoundNode Operation",
      String::Format("RemoveAndAttachInputsToOutputs method not allowed with %s", mNode->Name.c_str()));
    return;
  }

  ErrorIf(!mNode, "SoundNode data is null");

  mNode->DisconnectOnlyThis();
}

//**************************************************************************************************
bool SoundNode::GetAutoCollapse()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return mNode->GetCollapse();
}

//**************************************************************************************************
void SoundNode::SetAutoCollapse(bool willCollapse)
{
  ErrorIf(!mNode, "SoundNode data is null");

  mNode->SetCollapse(willCollapse);
}

//**************************************************************************************************
bool SoundNode::GetHasInputs()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return mNode->HasInputs();
}

//**************************************************************************************************
bool SoundNode::GetHasOutputs()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return mNode->HasOutputs();
}

//**************************************************************************************************
int SoundNode::GetInputCount()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return mNode->GetInputs()->Size();
}

//**************************************************************************************************
int SoundNode::GetOutputCount()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return mNode->GetOutputs()->Size();
}

//**************************************************************************************************
float SoundNode::GetBypassPercent()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return mNode->GetBypassValue() * 100.0f;
}

//**************************************************************************************************
void SoundNode::SetBypassPercent(float percent)
{
  ErrorIf(!mNode, "SoundNode data is null");

  mNode->SetBypassValue(Math::Clamp(percent, 0.0f, 100.0f) / 100.0f);
}

//**************************************************************************************************
float SoundNode::GetBypassValue()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return mNode->GetBypassValue();
}

//**************************************************************************************************
void SoundNode::SetBypassValue(float value)
{
  ErrorIf(!mNode, "SoundNode data is null");

  mNode->SetBypassValue(Math::Clamp(value, 0.0f, 1.0f));
}

//**************************************************************************************************
void SoundNode::SendAudioEvent(const Audio::AudioEventTypes::Enum eventType)
{
  if (eventType == Audio::AudioEventTypes::InterpolationDone)
  {
    SoundEvent event;
    DispatchEvent(Events::AudioInterpolationDone, &event);
  }
  else if (eventType == Audio::AudioEventTypes::NodeDisconnected)
  {
    SoundEvent event;
    DispatchEvent(Events::SoundNodeDisconnected, &event);
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
  ZilchBindMethod(AddMicUncompressedData);
}

//**************************************************************************************************
void SoundBuffer::AddSampleToBuffer(float value)
{
  mBuffer.PushBack(Math::Clamp(value, -1.0f, 1.0f));
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

//**************************************************************************************************
void SoundBuffer::AddMicUncompressedData(const HandleOf<ArrayClass<float>>& buffer)
{
  mBuffer.Append(buffer->NativeArray.All());
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
  ZilchBindMethod(SendMicUncompressedData);
  ZilchBindMethod(SendMicCompressedData);

  ZeroBindEvent(Events::CustomAudioNodeSamplesNeeded, CustomAudioNodeEvent);
}

//**************************************************************************************************
CustomAudioNode::CustomAudioNode() :
  AudioDecoder(nullptr)
{
  mNode = new Audio::CustomDataNode("CustomAudioNode", Z::gSound->mCounter++, this);
}

//**************************************************************************************************
CustomAudioNode::~CustomAudioNode()
{
  GetNode()->DeleteThisNode();
  mNode = nullptr;
}

//**************************************************************************************************
int CustomAudioNode::GetMinimumBufferSize()
{
  return (int)GetNode()->GetMinimumBufferSize();
}

//**************************************************************************************************
int CustomAudioNode::GetSystemSampleRate()
{
  return (int)GetNode()->GetSystemSampleRate();
}

//**************************************************************************************************
int CustomAudioNode::GetChannels()
{
  return GetNode()->GetNumberOfChannels();
}

//**************************************************************************************************
void CustomAudioNode::SetChannels(int channels)
{
  GetNode()->SetNumberOfChannels(Math::Clamp(channels, 0, (int)Audio::MaxChannels));
}

//**************************************************************************************************
void CustomAudioNode::SendBuffer(SoundBuffer* buffer)
{
  if (!buffer)
    DoNotifyException("Audio Error", "Called SendBuffer on CustomAudioNode with a null SoundBuffer");
  
  SendToAudioEngine(buffer->mBuffer.Data(), buffer->mBuffer.Size());
}

//**************************************************************************************************
void CustomAudioNode::SendPartialBuffer(SoundBuffer* buffer, int startAtIndex, int howManySamples)
{
  if (!buffer)
    DoNotifyException("Audio Error", "Called SendPartialBuffer on CustomAudioNode with a null SoundBuffer");
  else if (startAtIndex < 0 || ((startAtIndex + howManySamples) > (int)buffer->mBuffer.Size()))
    DoNotifyException("Audio Error", "SendPartialBuffer parameters exceed size of the SoundBuffer");
  
  SendToAudioEngine(buffer->mBuffer.Data() + startAtIndex, howManySamples);
}

//**************************************************************************************************
void CustomAudioNode::SendMicUncompressedData(const HandleOf<ArrayClass<float>>& audioData)
{
  SendToAudioEngine(audioData->NativeArray.Data(), audioData->NativeArray.Size());
}

//**************************************************************************************************
void CustomAudioNode::SendMicCompressedData(const HandleOf<ArrayClass<byte>>& audioData)
{
  // If we haven't created the decoder yet, create it
  if (!AudioDecoder)
    AudioDecoder = new Audio::AudioStreamDecoder();

  // Decode the compressed data
  float* decodedSamples;
  unsigned sampleCount;
  AudioDecoder->DecodeCompressedPacket(audioData->NativeArray.Data(), audioData->NativeArray.Size(), 
    decodedSamples, sampleCount);

  // Send the array (will be deleted by the audio engine)
  GetNode()->AddSamples(decodedSamples, sampleCount);
}

//**************************************************************************************************
void CustomAudioNode::SendAudioEventData(Audio::EventData* data)
{
  if (data->mEventType == Audio::AudioEventTypes::NeedInputSamples)
  {
    CustomAudioNodeEvent event(((Audio::EventData1<unsigned>*)data)->mData);
    mDispatcher.Dispatch(Events::CustomAudioNodeSamplesNeeded, &event);
  }

  delete data;
}

//**************************************************************************************************
void CustomAudioNode::SendToAudioEngine(float* samples, unsigned howManySamples)
{
  // Create the array to send
  float* newBuffer = new float[howManySamples];
  // Copy the data into the array
  memcpy(newBuffer, samples, sizeof(float) * howManySamples);
  // Send the array (will be deleted by the audio engine)
  GetNode()->AddSamples(newBuffer, howManySamples);
}

//**************************************************************************************************
Audio::CustomDataNode* CustomAudioNode::GetNode()
{
  ErrorIf(!mNode, "SoundNode data is null");

  return (Audio::CustomDataNode*)mNode;
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
  ZilchBindGetterSetter(SquareWavePulseValue);
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
  mVolume(1.0f),
  mSquareWavePulseValue(0.5f)
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
  mWaveFrequency = Math::Max(frequency, 0.0f);

  if (mAsset)
    mAsset->SetFrequency(mWaveFrequency, time);
}

//**************************************************************************************************
void GeneratedWaveNode::Play()
{
  // If there is a SoundInstance and it's paused, resume it
  if (mNode && ((Audio::SoundInstanceNode*)mNode)->GetPaused())
    ((Audio::SoundInstanceNode*)mNode)->SetPaused(false);
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
  mVolume = Math::Clamp(volume, 0.0f, Audio::MaxVolumeValue);

  if (mNode)
    ((Audio::SoundInstanceNode*)mNode)->SetVolume(mVolume, time);
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
  mVolume = Math::Clamp(Z::gSound->DecibelsToVolume(decibels), 0.0f, Audio::MaxVolumeValue);

  if (mNode)
    ((Audio::SoundInstanceNode*)mNode)->SetVolume(mVolume, time);
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

  if (mAsset)
    mAsset->SetSquareWavePositiveFraction(mSquareWavePulseValue);
}

//**************************************************************************************************
void GeneratedWaveNode::CreateAsset()
{
  if (mAsset)
    ReleaseAsset();

  Audio::OscillatorTypes::Enum waveType;

  switch (mWaveType)
  {
  case SynthWaveType::SineWave:
    waveType = Audio::OscillatorTypes::Sine;
    break;
  case SynthWaveType::SawWave:
    waveType = Audio::OscillatorTypes::Saw;
    break;
  case SynthWaveType::SquareWave:
    waveType = Audio::OscillatorTypes::Square;
    break;
  case SynthWaveType::TriangleWave:
    waveType = Audio::OscillatorTypes::Triangle;
    break;
  case SynthWaveType::Noise:
    waveType = Audio::OscillatorTypes::Noise;
    break;
  }

  mAsset = new Audio::GeneratedWaveSoundAsset(waveType, mWaveFrequency, this);
  if (mWaveType == SynthWaveType::SquareWave)
    mAsset->SetSquareWavePositiveFraction(mSquareWavePulseValue);
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
      ((Audio::SoundInstanceNode*)mNode)->SetPaused(false);
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
  GetNode()->SetVolume(Math::Clamp(volume, 0.0f, Audio::MaxVolumeValue), time);
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
  GetNode()->SetVolume(Math::Clamp(Z::gSound->DecibelsToVolume(volumeDB), 0.0f, Audio::MaxVolumeValue), time);
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
  GetNode()->SetLeftVolume(Math::Clamp(volume, 0.0f, Audio::MaxVolumeValue), time);
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
  GetNode()->SetRightVolume(Math::Clamp(volume, 0.0f, Audio::MaxVolumeValue), time);
}

//**************************************************************************************************
void PanningNode::InterpolateVolumes(float leftVolume, float rightVolume, float time)
{
  GetNode()->SetLeftVolume(Math::Clamp(leftVolume, 0.0f, Audio::MaxVolumeValue), time);
  GetNode()->SetRightVolume(Math::Clamp(rightVolume, 0.0f, Audio::MaxVolumeValue), time);
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
  GetNode()->SetPitch(Math::Clamp(Z::gSound->PitchToSemitones(pitchRatio), Audio::MinSemitonesValue,
    Audio::MaxSemitonesValue), time);
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
  GetNode()->SetPitch(Math::Clamp(pitchSemitones, Audio::MinSemitonesValue, Audio::MaxSemitonesValue), time);
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
  GetNode()->SetBelow80HzGain(Math::Clamp(gain, 0.0f, Audio::MaxVolumeValue));
}

//**************************************************************************************************
float EqualizerNode::GetHighPassGain()
{
  return GetNode()->GetAbove5000HzGain();
}

//**************************************************************************************************
void EqualizerNode::SetHighPassGain(float gain)
{
  GetNode()->SetAbove5000HzGain(Math::Clamp(gain, 0.0f, Audio::MaxVolumeValue));
}

//**************************************************************************************************
float EqualizerNode::GetBand1Gain()
{
  return GetNode()->Get150HzGain();
}

//**************************************************************************************************
void EqualizerNode::SetBand1Gain(float gain)
{
  GetNode()->Set150HzGain(Math::Clamp(gain, 0.0f, Audio::MaxVolumeValue));
}

//**************************************************************************************************
float EqualizerNode::GetBand2Gain()
{
  return GetNode()->Get600HzGain();
}

//**************************************************************************************************
void EqualizerNode::SetBand2Gain(float gain)
{
  GetNode()->Set600HzGain(Math::Clamp(gain, 0.0f, Audio::MaxVolumeValue));
}

//**************************************************************************************************
float EqualizerNode::GetBand3Gain()
{
  return GetNode()->Get2500HzGain();
}

//**************************************************************************************************
void EqualizerNode::SetBand3Gain(float gain)
{
  GetNode()->Set2500HzGain(Math::Clamp(gain, 0.0f, Audio::MaxVolumeValue));
}

//**************************************************************************************************
void EqualizerNode::InterpolateAllBands(float lowPass, float band1, float band2, float band3, 
  float highPass, float timeToInterpolate)
{
  GetNode()->InterpolateBands(Audio::EqualizerBandGains(Math::Clamp(lowPass, 0.0f, Audio::MaxVolumeValue), 
    Math::Clamp(band1, 0.0f, Audio::MaxVolumeValue), Math::Clamp(band2, 0.0f, Audio::MaxVolumeValue), 
    Math::Clamp(band3, 0.0f, Audio::MaxVolumeValue), Math::Clamp(highPass, 0.0f, Audio::MaxVolumeValue)), 
    timeToInterpolate);
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
  GetNode()->SetTime(Math::Clamp(time, 0.0f, 100.0f) * 1000);
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
  GetNode()->SetInputGain(Math::Clamp(dB, Audio::MinDecibelsValue, Audio::MaxDecibelsValue));
}

//**************************************************************************************************
float ExpanderNode::GetThresholdDecibels()
{
  return GetNode()->GetThreshold();
}

//**************************************************************************************************
void ExpanderNode::SetThresholdDecibels(float dB)
{
  GetNode()->SetThreshold(Math::Clamp(dB, Audio::MinDecibelsValue, Audio::MaxDecibelsValue));
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
  GetNode()->SetRatio(Math::Clamp(ratio, -10.0f, 10.0f));
}

//**************************************************************************************************
float ExpanderNode::GetOutputGainDecibels()
{
  return GetNode()->GetOutputGain();
}

//**************************************************************************************************
void ExpanderNode::SetOutputGainDecibels(float dB)
{
  GetNode()->SetOutputGain(Math::Clamp(dB, Audio::MinDecibelsValue, Audio::MaxDecibelsValue));
}

//**************************************************************************************************
float ExpanderNode::GetKneeWidth()
{
  return GetNode()->GetKneeWidth();
}

//**************************************************************************************************
void ExpanderNode::SetKneeWidth(float knee)
{
  GetNode()->SetKneeWidth(Math::Clamp(knee, 0.0f, 100.0f));
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
  GetNode()->SetAdditiveNoiseGainDB(Math::Clamp(decibels, Audio::MinDecibelsValue, Audio::MaxDecibelsValue));
}

//**************************************************************************************************
float AddNoiseNode::GetMultiplicativeGain()
{
  return GetNode()->GetMultipleNoiseGainDB();
}

//**************************************************************************************************
void AddNoiseNode::SetMultiplicativeGain(float decibels)
{
  GetNode()->SetMultipleNoiseGainDB(Math::Clamp(decibels, Audio::MinDecibelsValue, Audio::MaxDecibelsValue));
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
  GetNode()->NoteOn((int)midiNote, Math::Clamp(volume, 0.0f, Audio::MaxVolumeValue));
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

  if (!GetNode()->GetActive())
    DoNotifyWarning("No Microphone Available", "No microphone input is available for the MicrophoneInputNode");
}

//**************************************************************************************************
float MicrophoneInputNode::GetVolume()
{
  return GetNode()->GetVolume();
}

//**************************************************************************************************
void MicrophoneInputNode::SetVolume(float volume)
{
  GetNode()->SetVolume(Math::Clamp(volume, 0.0f, Audio::MaxVolumeValue));
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

  if (active && !GetNode()->GetActive())
    DoNotifyWarning("No Microphone Available", "No microphone input is available for the MicrophoneInputNode");
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
  GetNode()->SetAsset(sound->mSoundAsset, Math::Max(startTime, 0.0f), Math::Max(stopTime, 0.0f));
}

//**************************************************************************************************
float GranularSynthNode::GetGrainVolume()
{
  return GetNode()->GetGrainVolume();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainVolume(float volume)
{
  GetNode()->SetGrainVolume(Math::Clamp(volume, 0.0f, Audio::MaxVolumeValue));
}

//**************************************************************************************************
float GranularSynthNode::GetGrainVolumeVariance()
{
  return GetNode()->GetGrainVolumeVariance();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainVolumeVariance(float variance)
{
  GetNode()->SetGrainVolumeVariance(Math::Clamp(variance, 0.0f, Audio::MaxVolumeValue));
}

//**************************************************************************************************
int GranularSynthNode::GetGrainDelay()
{
  return GetNode()->GetGrainDelay();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainDelay(int delayMS)
{
  GetNode()->SetGrainDelay(Math::Max(delayMS, 0));
}

//**************************************************************************************************
int GranularSynthNode::GetGrainDelayVariance()
{
  return GetNode()->GetGrainDelayVariance();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainDelayVariance(int delayVarianceMS)
{
  GetNode()->SetGrainDelayVariance(Math::Max(delayVarianceMS, 0));
}

//**************************************************************************************************
int GranularSynthNode::GetGrainLength()
{
  return GetNode()->GetGrainLength();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainLength(int lengthMS)
{
  GetNode()->SetGrainLength(Math::Max(lengthMS, 0));
}

//**************************************************************************************************
int GranularSynthNode::GetGrainLengthVariance()
{
  return GetNode()->GetGrainLengthVariance();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainLengthVariance(int lengthVarianceMS)
{
  GetNode()->SetGrainLengthVariance(Math::Max(lengthVarianceMS, 0));
}

//**************************************************************************************************
float GranularSynthNode::GetGrainResampleRate()
{
  return GetNode()->GetGrainResampleRate();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainResampleRate(float resampleRate)
{
  GetNode()->SetGrainResampleRate(Math::Clamp(resampleRate, -mMaxResampleValue, mMaxResampleValue));
}

//**************************************************************************************************
float GranularSynthNode::GetGrainResampleRateVariance()
{
  return GetNode()->GetGrainResampleRateVariance();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainResampleRateVariance(float resampleVariance)
{
  GetNode()->SetGrainResampleRateVariance(Math::Clamp(resampleVariance, 0.0f, mMaxResampleValue));
}

//**************************************************************************************************
float GranularSynthNode::GetBufferScanRate()
{
  return GetNode()->GetBufferScanRate();
}

//**************************************************************************************************
void GranularSynthNode::SetBufferScanRate(float bufferRate)
{
  GetNode()->SetBufferScanRate(Math::Clamp(bufferRate, -mMaxResampleValue, mMaxResampleValue));
}

//**************************************************************************************************
float GranularSynthNode::GetGrainPanningValue()
{
  return GetNode()->GetGrainPanningValue();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainPanningValue(float panValue)
{
  GetNode()->SetGrainPanningValue(Math::Clamp(panValue, -1.0f, 1.0f));
}

//**************************************************************************************************
float GranularSynthNode::GetGrainPanningVariance()
{
  return GetNode()->GetGrainPanningVariance();
}

//**************************************************************************************************
void GranularSynthNode::SetGrainPanningVariance(float panValueVariance)
{
  GetNode()->SetGrainPanningVariance(Math::Clamp(panValueVariance, 0.0f, 1.0f));
}

//**************************************************************************************************
float GranularSynthNode::GetRandomLocationValue()
{
  return GetNode()->GetRandomLocationValue();
}

//**************************************************************************************************
void GranularSynthNode::SetRandomLocationValue(float randomLocationValue)
{
  GetNode()->SetRandomLocationValue(Math::Clamp(randomLocationValue, 0.0f, 1.0f));
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
  GetNode()->SetWindowAttack(Math::Max(attackMS, 0));
}

//**************************************************************************************************
int GranularSynthNode::GetWindowRelease()
{
  return GetNode()->GetWindowRelease();
}

//**************************************************************************************************
void GranularSynthNode::SetWindowRelease(int releaseMS)
{
  GetNode()->SetWindowRelease(Math::Max(releaseMS, 0));
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
