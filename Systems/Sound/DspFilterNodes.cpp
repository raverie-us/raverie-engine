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
VolumeNode::VolumeNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mVolume(1.0f)
{

}

//**************************************************************************************************
float VolumeNode::GetVolume()
{
  return mVolume.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void VolumeNode::SetVolume(float volume)
{
  InterpolateVolume(volume, 0.0f);
}

//**************************************************************************************************
void VolumeNode::InterpolateVolume(float volume, float time)
{
  volume = Math::Clamp(volume, 0.0f, cMaxVolumeValue);
  
  if ((time == 0.0f && !GetHasOutputs()) || IsWithinLimit(volume, mVolume.Get(AudioThreads::MainThread), 0.01f))
    mVolume.Set(volume, AudioThreads::MainThread);
  else
  {
    time = Math::Max(time, 0.02f);

    Z::gSound->Mixer.AddTask(CreateFunctor(&VolumeNode::InterpolateVolumeThreaded, this, volume, time), this);
  }
}

//**************************************************************************************************
float VolumeNode::GetDecibels()
{
  return VolumeToDecibels(mVolume.Get(AudioThreads::MainThread));
}

//**************************************************************************************************
void VolumeNode::SetDecibels(float volumeDB)
{
  InterpolateDecibels(volumeDB, 0.0f);
}

//**************************************************************************************************
void VolumeNode::InterpolateDecibels(float volumeDB, float time)
{
  volumeDB = Math::Clamp(volumeDB, cMinDecibelsValue, cMaxDecibelsValue);

  InterpolateVolume(DecibelsToVolume(volumeDB), time);
}

//**************************************************************************************************
bool VolumeNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  // If first request this mix, set the previous mix data
  if (firstRequest)
    PreviousData = CurrentData;
  // If not, reset the current data to the previous
  else
    CurrentData = PreviousData;

  unsigned bufferSize = outputBuffer->Size();

  // Get input and return if there is no data
  if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
    return false;

  // Apply volume adjustment
  for (BufferRange outputRange = outputBuffer->All(), inputRange = mInputSamplesThreaded.All(); 
    !outputRange.Empty(); )
  {
    // Check if the volume is being interpolated
    if (CurrentData.mInterpolating)
    {
      // Get the current volume and increase the index
      mVolume.Set(Interpolator.ValueAtIndex(CurrentData.mIndex++), AudioThreads::MixThread);

      // Check if the interpolation is finished
      if (CurrentData.mIndex >= Interpolator.GetTotalFrames())
      {
        CurrentData.mInterpolating = false;
        if (firstRequest)
        {
          Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundNode::DispatchEventFromMixThread, (SoundNode*)this,
            Events::AudioInterpolationDone), this);
        }
      }
    }

    // Apply the volume multiplier to all samples
    for (unsigned j = 0; j < numberOfChannels; ++j, outputRange.PopFront(), inputRange.PopFront())
      outputRange.Front() = inputRange.Front() * mVolume.Get(AudioThreads::MixThread);
  }

  AddBypassThreaded(outputBuffer);

  return true;
}

//**************************************************************************************************
float VolumeNode::GetVolumeChangeFromOutputsThreaded()
{
  float outputVolume = 0.0f;

  // Get all volumes from outputs
  forRange(HandleOf<SoundNode> node, GetOutputs(AudioThreads::MixThread)->All())
    outputVolume += node->GetVolumeChangeFromOutputsThreaded();

  // Return the output volume modified by this node's volume
  return outputVolume * mVolume.Get(AudioThreads::MixThread);
}

//**************************************************************************************************
void VolumeNode::InterpolateVolumeThreaded(float volume, float time)
{
  CurrentData.mInterpolating = true;
  CurrentData.mIndex = 0;

  Interpolator.SetValues(mVolume.Get(AudioThreads::MixThread), volume, (unsigned)(time * cSystemSampleRate));
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
PanningNode::PanningNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mSumToMono(false),
  mLeftVolume(1.0f),
  mRightVolume(1.0f),
  mActive(false)
{

}

//**************************************************************************************************
bool PanningNode::GetSumToMono()
{
  return mSumToMono.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void PanningNode::SetSumToMono(bool isMono)
{
  mSumToMono.Set(isMono, AudioThreads::MainThread);
  if (isMono)
    mActive.Set(true, AudioThreads::MainThread);
}

//**************************************************************************************************
float PanningNode::GetLeftVolume()
{
  return mLeftVolume.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void PanningNode::SetLeftVolume(float volume)
{
  InterpolateLeftVolume(volume, 0.0f);
}

//**************************************************************************************************
void PanningNode::InterpolateLeftVolume(float volume, float time)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&PanningNode::SetVolumeThreaded, this, true,
    Math::Clamp(volume, 0.0f, cMaxVolumeValue), Math::Max(time, 0.0f)), this);
}

//**************************************************************************************************
float PanningNode::GetRightVolume()
{
  return mRightVolume.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void PanningNode::SetRightVolume(float volume)
{
  InterpolateRightVolume(volume, 0.0f);
}

//**************************************************************************************************
void PanningNode::InterpolateRightVolume(float volume, float time)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&PanningNode::SetVolumeThreaded, this, false,
    Math::Clamp(volume, 0.0f, cMaxVolumeValue), Math::Max(time, 0.0f)), this);
}

//**************************************************************************************************
void PanningNode::InterpolateVolumes(float leftVolume, float rightVolume, float time)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&PanningNode::SetVolumeThreaded, this, true,
    Math::Clamp(leftVolume, 0.0f, cMaxVolumeValue), Math::Max(time, 0.0f)), this);
  Z::gSound->Mixer.AddTask(CreateFunctor(&PanningNode::SetVolumeThreaded, this, false,
    Math::Clamp(rightVolume, 0.0f, cMaxVolumeValue), Math::Max(time, 0.0f)), this);
}

//**************************************************************************************************
bool PanningNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  // If first request this mix, set the previous mix data
  if (firstRequest)
    PreviousData = CurrentData;
  // If not, reset the current data to the previous
  else
    CurrentData = PreviousData;

  unsigned bufferSize = outputBuffer->Size();

  if (mActive.Get(AudioThreads::MixThread))
  {
    bool sumToMono = mSumToMono.Get(AudioThreads::MixThread);
    unsigned totalFrames = bufferSize / numberOfChannels;
    unsigned channelsToGet;
    if (sumToMono)
      channelsToGet = 1;
    else
      channelsToGet = 2;

    // Get input and return if there is no data
    if (!AccumulateInputSamples(totalFrames * channelsToGet, channelsToGet, listener))
      return false;

    float leftVolume = mLeftVolume.Get(AudioThreads::MixThread);
    float rightVolume = mRightVolume.Get(AudioThreads::MixThread);

    // Step through each frame of audio data
    for (unsigned currentFrame = 0; currentFrame < totalFrames; ++currentFrame)
    {
      float leftValue, rightValue;
      // If requested 1 channel of audio, copy this to left and right channels
      if (sumToMono)
        leftValue = rightValue = mInputSamplesThreaded[currentFrame];
      else
      {
        leftValue = mInputSamplesThreaded[currentFrame * 2];
        rightValue = mInputSamplesThreaded[(currentFrame * 2) + 1];
      }

      // Check if we are interpolating the volume
      if (CurrentData.mInterpolating)
      {
        leftVolume = LeftInterpolator.NextValue();
        rightVolume = RightInterpolator.NextValue();

        // If we finished interpolating the volume, send a notification
        if (LeftInterpolator.Finished() && RightInterpolator.Finished())
        {
          CurrentData.mInterpolating = false;
          if (firstRequest)
            Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundNode::DispatchEventFromMixThread,
              (SoundNode*)this, Events::AudioInterpolationDone), this);
        }
      }

      // Set the volume of both channels
      leftValue *= leftVolume;
      rightValue *= rightVolume;

      // Channels match, can just copy
      if (numberOfChannels == 2)
      {
        (*outputBuffer)[currentFrame * 2] = leftValue;
        (*outputBuffer)[(currentFrame * 2) + 1] = rightValue;
      }
      // Adjust channels to match output
      else
      {
        float values[2] = { leftValue, rightValue };
        AudioFrame frame(values, 2);
        memcpy(outputBuffer->Data() + (currentFrame * numberOfChannels), frame.GetSamples(numberOfChannels),
          sizeof(float) * numberOfChannels);
      }
    }

    if (PreviousData.mInterpolating)
    {
      mLeftVolume.Set(leftVolume, AudioThreads::MixThread);
      mRightVolume.Set(rightVolume, AudioThreads::MixThread);
    }

    AddBypassThreaded(outputBuffer);

    // Check for both volumes being at or near 1.0, and if true mark as not active
    if (!CurrentData.mInterpolating && !sumToMono && IsWithinLimit(1.0f - leftVolume, 0.0f, 0.01f)
      && IsWithinLimit(1.0f - rightVolume, 0.0f, 0.01f))
      mActive.Set(false, AudioThreads::MixThread);
  }
  else
  {
    // Get input and return if there is no data
    if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
      return false;

    mInputSamplesThreaded.Swap(*outputBuffer);
  }

  return true;
}

//**************************************************************************************************
float PanningNode::GetVolumeChangeFromOutputsThreaded()
{
  float volume = (mLeftVolume.Get(AudioThreads::MixThread) + mRightVolume.Get(AudioThreads::MixThread)) / 2.0f;

  forRange(HandleOf<SoundNode> node, GetOutputs(AudioThreads::MixThread)->All())
    volume *= node->GetVolumeChangeFromOutputsThreaded();

  return volume;
}

//**************************************************************************************************
void PanningNode::SetVolumeThreaded(bool left, float newVolume, float time)
{
  mActive.Set(true, AudioThreads::MixThread);

  Threaded<float>* volume = &mRightVolume;
  if (left)
    volume = &mLeftVolume;

  InterpolatingObject* interpolator = &RightInterpolator;
  if (left)
    interpolator = &LeftInterpolator;

  // If the new volume is very close to the current one, or we have no outputs, just set it directly
  if ((time == 0 && !GetHasOutputs()) || IsWithinLimit(newVolume, volume->Get(AudioThreads::MixThread), 0.01f))
  {
    CurrentData.mInterpolating = false;
    volume->Set(newVolume, AudioThreads::MixThread);
    interpolator->SetValues(volume->Get(AudioThreads::MixThread), volume->Get(AudioThreads::MixThread), 0u);
  }
  else
  {
    CurrentData.mInterpolating = true;

    if (time < 0.02f)
      time = 0.02f;

    interpolator->SetValues(volume->Get(AudioThreads::MixThread), newVolume, 
      (unsigned)(time * cSystemSampleRate));
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
PitchNode::PitchNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mPitchSemitones(0.0f)
{

}

//**************************************************************************************************
float PitchNode::GetPitch()
{
  return SemitonesToPitch(mPitchSemitones.Get(AudioThreads::MainThread));
}

//**************************************************************************************************
void PitchNode::SetPitch(float pitchRatio)
{
  pitchRatio = Math::Clamp(pitchRatio, cMinPitchValue, cMaxPitchValue);
  mPitchSemitones.Set(PitchToSemitones(pitchRatio), AudioThreads::MainThread);
  InterpolatePitch(pitchRatio, 0.0f);
}

//**************************************************************************************************
void PitchNode::InterpolatePitch(float pitchRatio, float time)
{
  pitchRatio = Math::Clamp(pitchRatio, cMinPitchValue, cMaxPitchValue);
  Z::gSound->Mixer.AddTask(CreateFunctor(&PitchNode::SetPitchThreaded, this, 
    PitchToSemitones(pitchRatio), time), this);
}

//**************************************************************************************************
float PitchNode::GetSemitones()
{
  return mPitchSemitones.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void PitchNode::SetSemitones(float pitchSemitones)
{
  mPitchSemitones.Set(pitchSemitones, AudioThreads::MainThread);
  InterpolateSemitones(pitchSemitones, 0.0f);
}

//**************************************************************************************************
void PitchNode::InterpolateSemitones(float pitchSemitones, float time)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&PitchNode::SetPitchThreaded, this, pitchSemitones, time), this);
}

//**************************************************************************************************
bool PitchNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  // If no inputs, return
  if (!GetHasInputs())
    return false;

  // If this is the first request, calculate the buffer size
  if (firstRequest)
    PitchObject.CalculateBufferSize(outputBuffer->Size(), numberOfChannels);
  // Otherwise reset variables to the beginning of the mix
  else
    PitchObject.ResetToStartOfMix();

  // If no valid input, set the last samples to zero and return
  if (!AccumulateInputSamples(PitchObject.GetInputSampleCount(), numberOfChannels, listener))
  {
    PitchObject.ResetLastSamples();
    return false;
  }

  PitchObject.ProcessBuffer(&mInputSamplesThreaded, outputBuffer);

  mPitchSemitones.Set(12.0f * Math::Log2(PitchObject.GetPitchFactor()), AudioThreads::MixThread);

  return true;
}

//**************************************************************************************************
void PitchNode::SetPitchThreaded(float semitones, float interpolationTime)
{
  float newFactor = Math::Pow(2.0f, semitones / 12.0f);
  PitchObject.SetPitchFactor(newFactor, interpolationTime);
}

//------------------------------------------------------------------------------------ Low Pass Node

//**************************************************************************************************
ZilchDefineType(LowPassNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(CutoffFrequency);
}

//**************************************************************************************************
LowPassNode::LowPassNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mCutoffFrequency(20001.0f)
{

}

//**************************************************************************************************
LowPassNode::~LowPassNode()
{
  forRange(LowPassFilter* filter, FiltersPerListener.Values())
    delete filter;
}

//**************************************************************************************************
float LowPassNode::GetCutoffFrequency()
{
  return mCutoffFrequency.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void LowPassNode::SetCutoffFrequency(float frequency)
{
  mCutoffFrequency.Set(frequency, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&LowPassNode::SetCutoffFrequencyThreaded, this, frequency), this);
}

//**************************************************************************************************
bool LowPassNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  unsigned bufferSize = outputBuffer->Size();

  // Get input and return if there is no data
  if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
    return false;

  // If the filter is turned off, return
  if (mCutoffFrequency.Get(AudioThreads::MixThread) >= 20000.0f)
  {
    // Move the input samples to the output buffer
    mInputSamplesThreaded.Swap(*outputBuffer);
    return true;
  }

  // Check if the listener is in the map
  LowPassFilter* filter = FiltersPerListener.FindValue(listener, nullptr);
  if (!filter)
  {
    filter = new LowPassFilter;
    filter->SetCutoffFrequency(mCutoffFrequency.Get(AudioThreads::MixThread));
    FiltersPerListener[listener] = filter;
  }

  // Apply filter
  filter->ProcessBuffer(mInputSamplesThreaded.Data(), outputBuffer->Data(), numberOfChannels, bufferSize);

  AddBypassThreaded(outputBuffer);

  return true;
}

//**************************************************************************************************
void LowPassNode::RemoveListenerThreaded(SoundEvent* event)
{
  ListenerNode* listener = (ListenerNode*)event->mPointer;

  if (FiltersPerListener.FindValue(listener, nullptr))
  {
    // If there is still another listener, we need to combine the history of the filters to avoid clicks
    if (FiltersPerListener.Size() > 1)
    {
      LowPassFilter* filter = FiltersPerListener[listener];
      FiltersPerListener.Erase(listener);
      FilterMapType::valuerange range = FiltersPerListener.All();
      filter->MergeWith(*range.Front());
    }

    delete FiltersPerListener[listener];
    FiltersPerListener.Erase(listener);
  }
}

//**************************************************************************************************
void LowPassNode::SetCutoffFrequencyThreaded(float frequency)
{
  forRange(LowPassFilter* filter, FiltersPerListener.Values())
    filter->SetCutoffFrequency(frequency);
}

//----------------------------------------------------------------------------------- High Pass Node

//**************************************************************************************************
ZilchDefineType(HighPassNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(CutoffFrequency);
}

//**************************************************************************************************
HighPassNode::HighPassNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mCutoffFrequency(10.0f)
{

}

//**************************************************************************************************
HighPassNode::~HighPassNode()
{
  forRange(HighPassFilter* filter, FiltersPerListener.Values())
    delete filter;
}

//**************************************************************************************************
float HighPassNode::GetCutoffFrequency()
{
  return mCutoffFrequency.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void HighPassNode::SetCutoffFrequency(float frequency)
{
  mCutoffFrequency.Set(frequency, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&HighPassNode::SetCutoffFrequencyThreaded, this, frequency), this);
}

//**************************************************************************************************
bool HighPassNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  unsigned bufferSize = outputBuffer->Size();

  // Get input and return if there is no data
  if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
    return false;

  // If the filter is turned off, return
  if (mCutoffFrequency.Get(AudioThreads::MixThread) <= 20.0f)
  {
    // Move the input samples to the output buffer
    mInputSamplesThreaded.Swap(*outputBuffer);
    return true;
  }

  // Check if the listener is in the map
  HighPassFilter* filter = FiltersPerListener.FindValue(listener, nullptr);
  if (!filter)
  {
    filter = new HighPassFilter;
    filter->SetCutoffFrequency(mCutoffFrequency.Get(AudioThreads::MixThread));
    FiltersPerListener[listener] = filter;
  }

  // Apply filter
  for (unsigned i = 0; i < bufferSize; i += numberOfChannels)
    filter->ProcessFrame(mInputSamplesThreaded.Data() + i, outputBuffer->Data() + i, numberOfChannels);

  AddBypassThreaded(outputBuffer);

  return true;
}

//**************************************************************************************************
void HighPassNode::RemoveListenerThreaded(SoundEvent* event)
{
  ListenerNode* listener = (ListenerNode*)event->mPointer;

  if (FiltersPerListener.FindValue(listener, nullptr))
  {
    // If there is still another listener, we need to combine the history of the filters to avoid clicks
    if (FiltersPerListener.Size() > 1)
    {
      HighPassFilter* filter = FiltersPerListener[listener];
      FiltersPerListener.Erase(listener);
      FilterMapType::valuerange range = FiltersPerListener.All();
      filter->MergeWith(*range.Front());
    }

    delete FiltersPerListener[listener];
    FiltersPerListener.Erase(listener);
  }
}

//**************************************************************************************************
void HighPassNode::SetCutoffFrequencyThreaded(float frequency)
{
  forRange(HighPassFilter* filter, FiltersPerListener.Values())
    filter->SetCutoffFrequency(frequency);
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
BandPassNode::BandPassNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mCentralFrequency(0.0f),
  mQuality(0.669f)
{

}

//**************************************************************************************************
BandPassNode::~BandPassNode()
{
  forRange(BandPassFilter* filter, FiltersPerListener.Values())
    delete filter;
}

//**************************************************************************************************
float BandPassNode::GetCentralFrequency()
{
  return mCentralFrequency.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void BandPassNode::SetCentralFrequency(float frequency)
{
  mCentralFrequency.Set(frequency, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&BandPassNode::SetCentralFrequencyThreaded, this, frequency), this);
}

//**************************************************************************************************
float BandPassNode::GetQualityFactor()
{
  return mQuality.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void BandPassNode::SetQualityFactor(float Q)
{
  mQuality.Set(Q, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&BandPassNode::SetQualityFactorThreaded, this, Q), this);
}

//**************************************************************************************************
bool BandPassNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  unsigned bufferSize = outputBuffer->Size();

  // Get input and return if there is no data
  if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
    return false;

  // Check if the listener is in the map
  BandPassFilter* filter = FiltersPerListener.FindValue(listener, nullptr);
  if (!filter)
  {
    filter = new BandPassFilter;
    filter->SetFrequency(mCentralFrequency.Get(AudioThreads::MixThread));
    filter->SetQuality(mQuality.Get(AudioThreads::MixThread));
    FiltersPerListener[listener] = filter;
  }

  // Apply filter
  for (unsigned i = 0; i < bufferSize; i += numberOfChannels)
    filter->ProcessFrame(mInputSamplesThreaded.Data() + i, outputBuffer->Data() + i, numberOfChannels);

  AddBypassThreaded(outputBuffer);

  return true;
}

//**************************************************************************************************
void BandPassNode::RemoveListenerThreaded(SoundEvent* event)
{
  ListenerNode* listener = (ListenerNode*)event->mPointer;

  if (FiltersPerListener.FindValue(listener, nullptr))
  {
    // If there is still another listener, we need to combine the history of the filters to avoid clicks
    if (FiltersPerListener.Size() > 1)
    {
      BandPassFilter* filter = FiltersPerListener[listener];
      FiltersPerListener.Erase(listener);
      FilterMapType::valuerange range = FiltersPerListener.All();
      filter->MergeWith(*range.Front());
    }

    delete FiltersPerListener[listener];
    FiltersPerListener.Erase(listener);
  }
}

//**************************************************************************************************
void BandPassNode::SetCentralFrequencyThreaded(float frequency)
{
  forRange(BandPassFilter* filter, FiltersPerListener.Values())
    filter->SetFrequency(frequency);
}

//**************************************************************************************************
void BandPassNode::SetQualityFactorThreaded(float Q)
{
  forRange(BandPassFilter* filter, FiltersPerListener.Values())
    filter->SetQuality(Q);
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
EqualizerNode::EqualizerNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mLowPassGain(1.0f),
  mHighPassGain(1.0f),
  mBand1Gain(1.0f),
  mBand2Gain(1.0f),
  mBand3Gain(1.0f)
{

}

//**************************************************************************************************
EqualizerNode::~EqualizerNode()
{
  forRange(Equalizer* filter, FiltersPerListener.Values())
    delete filter;
}

//**************************************************************************************************
float EqualizerNode::GetLowPassGain()
{
  return mLowPassGain.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void EqualizerNode::SetLowPassGain(float gain)
{
  mLowPassGain.Set(Math::Clamp(gain, 0.0f, cMaxVolumeValue), AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&EqualizerNode::SetBandGainThreaded, this,
    EqualizerBands::Below80, mLowPassGain.Get(AudioThreads::MainThread)), this);
}

//**************************************************************************************************
float EqualizerNode::GetHighPassGain()
{
  return mHighPassGain.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void EqualizerNode::SetHighPassGain(float gain)
{
  mHighPassGain.Set(Math::Clamp(gain, 0.0f, cMaxVolumeValue), AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&EqualizerNode::SetBandGainThreaded, this,
    EqualizerBands::Above5000, mHighPassGain.Get(AudioThreads::MainThread)), this);
}

//**************************************************************************************************
float EqualizerNode::GetBand1Gain()
{
  return mBand1Gain.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void EqualizerNode::SetBand1Gain(float gain)
{
  mBand1Gain.Set(Math::Clamp(gain, 0.0f, cMaxVolumeValue), AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&EqualizerNode::SetBandGainThreaded, this,
    EqualizerBands::At150, mBand1Gain.Get(AudioThreads::MainThread)), this);
}

//**************************************************************************************************
float EqualizerNode::GetBand2Gain()
{
  return mBand2Gain.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void EqualizerNode::SetBand2Gain(float gain)
{
  mBand2Gain.Set(Math::Clamp(gain, 0.0f, cMaxVolumeValue), AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&EqualizerNode::SetBandGainThreaded, this,
    EqualizerBands::At600, mBand2Gain.Get(AudioThreads::MainThread)), this);
}

//**************************************************************************************************
float EqualizerNode::GetBand3Gain()
{
  return mBand3Gain.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void EqualizerNode::SetBand3Gain(float gain)
{
  mBand3Gain.Set(Math::Clamp(gain, 0.0f, cMaxVolumeValue), AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&EqualizerNode::SetBandGainThreaded, this,
    EqualizerBands::At2500, mBand3Gain.Get(AudioThreads::MainThread)), this);
}

//**************************************************************************************************
void EqualizerNode::InterpolateAllBands(float lowPass, float band1, float band2, float band3, 
  float highPass, float timeToInterpolate)
{
  mLowPassGain.Set(Math::Clamp(lowPass, 0.0f, cMaxVolumeValue), AudioThreads::MainThread);
  mBand1Gain.Set(Math::Clamp(band1, 0.0f, cMaxVolumeValue), AudioThreads::MainThread);
  mBand2Gain.Set(Math::Clamp(band2, 0.0f, cMaxVolumeValue), AudioThreads::MainThread);
  mBand3Gain.Set(Math::Clamp(band3, 0.0f, cMaxVolumeValue), AudioThreads::MainThread);
  mHighPassGain.Set(Math::Clamp(highPass, 0.0f, cMaxVolumeValue), AudioThreads::MainThread);

  float* values = new float[EqualizerBands::Count];
  values[EqualizerBands::Below80] = mLowPassGain.Get(AudioThreads::MainThread);
  values[EqualizerBands::At150] = mBand1Gain.Get(AudioThreads::MainThread);
  values[EqualizerBands::At600] = mBand2Gain.Get(AudioThreads::MainThread);
  values[EqualizerBands::At2500] = mBand3Gain.Get(AudioThreads::MainThread);
  values[EqualizerBands::Above5000] = mHighPassGain.Get(AudioThreads::MainThread);

  Z::gSound->Mixer.AddTask(CreateFunctor(&EqualizerNode::InterpolateAllThreaded, this, values,
    timeToInterpolate), this);
}

//**************************************************************************************************
bool EqualizerNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  unsigned bufferSize = outputBuffer->Size();

  // Get input and return if there is no data
  if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
    return false;

  // Check if the listener is in the map
  Equalizer* filter = FiltersPerListener.FindValue(listener, nullptr);
  if (!filter)
  {
    filter = new Equalizer(
      mLowPassGain.Get(AudioThreads::MixThread),
      mHighPassGain.Get(AudioThreads::MixThread), 
      mBand1Gain.Get(AudioThreads::MixThread),
      mBand2Gain.Get(AudioThreads::MixThread),
      mBand3Gain.Get(AudioThreads::MixThread));
    FiltersPerListener[listener] = filter;
  }

  // Apply filter
  filter->ProcessBuffer(mInputSamplesThreaded.Data(), outputBuffer->Data(), numberOfChannels, bufferSize);

  AddBypassThreaded(outputBuffer);

  return true;
}

//**************************************************************************************************
void EqualizerNode::RemoveListenerThreaded(SoundEvent* event)
{
  ListenerNode* listener = (ListenerNode*)event->mPointer;

  if (FiltersPerListener.FindValue(listener, nullptr))
  {
    // If there is still another listener, we need to combine the history of the filters to avoid clicks
    if (FiltersPerListener.Size() > 1)
    {
      Equalizer* filter = FiltersPerListener[listener];
      FiltersPerListener.Erase(listener);
      FilterMapType::valuerange range = FiltersPerListener.All();
      filter->MergeWith(*range.Front());
    }

    delete FiltersPerListener[listener];
    FiltersPerListener.Erase(listener);
  }
}

//**************************************************************************************************
void EqualizerNode::SetBandGainThreaded(EqualizerBands::Enum whichBand, float gain)
{
  forRange(Equalizer* filter, FiltersPerListener.Values())
    filter->SetBandGain(whichBand, gain);
}

//**************************************************************************************************
void EqualizerNode::InterpolateAllThreaded(float* values, float timeToInterpolate)
{
  forRange(Equalizer* filter, FiltersPerListener.Values())
    filter->InterpolateBands(values, timeToInterpolate);
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
ReverbNode::ReverbNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mTimeSec(1.0f),
  mWetLevelValue(0.5f),
  mOutputFinishedThreaded(true)
{

}

//**************************************************************************************************
ReverbNode::~ReverbNode()
{
  forRange(Reverb* filter, FiltersPerListener.Values())
    delete filter;
}

//**************************************************************************************************
float ReverbNode::GetLength()
{
  return mTimeSec.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void ReverbNode::SetLength(float time)
{
  time = Math::Clamp(time, 0.0f, 100.0f);
  mTimeSec.Set(time, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&ReverbNode::SetLengthMsThreaded, this, time * 1000.0f), this);
}

//**************************************************************************************************
float ReverbNode::GetWetPercent()
{
  return mWetLevelValue.Get(AudioThreads::MainThread) * 100.0f;
}

//**************************************************************************************************
void ReverbNode::SetWetPercent(float percent)
{
  percent = Math::Clamp(percent, 0.0f, 100.0f) / 100.0f;
  mWetLevelValue.Set(percent, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&ReverbNode::SetWetValueThreaded, this, percent), this);
}

//**************************************************************************************************
float ReverbNode::GetWetValue()
{
  return mWetLevelValue.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void ReverbNode::SetWetValue(float value)
{
  value = Math::Clamp(value, 0.0f, 1.0f);
  mWetLevelValue.Set(value, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&ReverbNode::SetWetValueThreaded, this, value), this);
}

//**************************************************************************************************
void ReverbNode::InterpolateWetPercent(float percent, float time)
{
  percent = Math::Clamp(percent, 0.0f, 100.0f) / 100.0f;
  mWetLevelValue.Set(percent, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&ReverbNode::InterpolateWetValueThreaded, this, percent, time), this);
}

//**************************************************************************************************
void ReverbNode::InterpolateWetValue(float value, float time)
{
  value = Math::Clamp(value, 0.0f, 1.0f);
  mWetLevelValue.Set(value, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&ReverbNode::InterpolateWetValueThreaded, this, value, time), this);
}

//**************************************************************************************************
bool ReverbNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  unsigned bufferSize = outputBuffer->Size();

  // Get input
  bool isThereInput = AccumulateInputSamples(bufferSize, numberOfChannels, listener);

  // No input and the filter has no output
  if (!isThereInput && mOutputFinishedThreaded)
    return false;

  // Check if the listener is in the map
  Reverb* filter = FiltersPerListener.FindValue(listener, nullptr);
  if (!filter)
  {
    filter = new Reverb;
    filter->SetTime(mTimeSec.Get(AudioThreads::MixThread) * 1000.0f);
    filter->SetWetLevel(mWetLevelValue.Get(AudioThreads::MixThread));
    FiltersPerListener[listener] = filter;
  }

  bool hasOutput = filter->ProcessBuffer(mInputSamplesThreaded.Data(), outputBuffer->Data(),
    numberOfChannels, bufferSize);

  if (!isThereInput && !hasOutput)
    mOutputFinishedThreaded = true;
  else
    mOutputFinishedThreaded = false;

  AddBypassThreaded(outputBuffer);

  return true;
}

//**************************************************************************************************
void ReverbNode::RemoveListenerThreaded(SoundEvent* event)
{
  ListenerNode* listener = (ListenerNode*)event->mPointer;

  if (FiltersPerListener.FindValue(listener, nullptr))
  {
    delete FiltersPerListener[listener];
    FiltersPerListener.Erase(listener);
  }
}

//**************************************************************************************************
void ReverbNode::SetLengthMsThreaded(float time)
{
  forRange(Reverb* filter, FiltersPerListener.Values())
    filter->SetTime(time);
}

//**************************************************************************************************
void ReverbNode::SetWetValueThreaded(float value)
{
  forRange(Reverb* filter, FiltersPerListener.Values())
    filter->SetWetLevel(value);
}

//**************************************************************************************************
void ReverbNode::InterpolateWetValueThreaded(float value, float time)
{
  forRange(Reverb* filter, FiltersPerListener.Values())
    filter->InterpolateWetLevel(value, time);
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
DelayNode::DelayNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mDelaySec(0.1f),
  mFeedbackValue(0.0f),
  mWetValue(0.5f)
{

}

//**************************************************************************************************
DelayNode::~DelayNode()
{
  forRange(DelayLine* filter, FiltersPerListener.Values())
    delete filter;
}

//**************************************************************************************************
float DelayNode::GetDelay()
{
  return mDelaySec.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void DelayNode::SetDelay(float seconds)
{
  seconds = Math::Max(seconds, 0.0f);
  mDelaySec.Set(seconds, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&DelayNode::SetDelayMsThreaded, this, seconds * 1000.0f), this);
}

//**************************************************************************************************
float DelayNode::GetFeedbackPercent()
{
  return mFeedbackValue.Get(AudioThreads::MainThread) * 100.0f;
}

//**************************************************************************************************
void DelayNode::SetFeedbackPercent(float feedback)
{
  feedback = Math::Clamp(feedback, 0.0f, 100.0f) / 100.0f;
  mFeedbackValue.Set(feedback, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&DelayNode::SetFeedbackThreaded, this, feedback), this);
}

//**************************************************************************************************
float DelayNode::GetFeedbackValue()
{
  return mFeedbackValue.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void DelayNode::SetFeedbackValue(float feedback)
{
  feedback = Math::Clamp(feedback, 0.0f, 1.0f);
  mFeedbackValue.Set(feedback, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&DelayNode::SetFeedbackThreaded, this, feedback), this);
}

//**************************************************************************************************
float DelayNode::GetWetPercent()
{
  return mWetValue.Get(AudioThreads::MainThread) * 100.0f;
}

//**************************************************************************************************
void DelayNode::SetWetPercent(float wetLevel)
{
  wetLevel = Math::Clamp(wetLevel, 0.0f, 100.0f) / 100.0f;
  mWetValue.Set(wetLevel, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&DelayNode::SetWetValueThreaded, this, wetLevel), this);
}

//**************************************************************************************************
float DelayNode::GetWetValue()
{
  return mWetValue.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void DelayNode::SetWetValue(float wetLevel)
{
  wetLevel = Math::Clamp(wetLevel, 0.0f, 1.0f);
  mWetValue.Set(wetLevel, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&DelayNode::SetWetValueThreaded, this, wetLevel), this);
}

//**************************************************************************************************
void DelayNode::InterpolateWetPercent(float percent, float time)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&DelayNode::InterpolateWetValueThreaded, this, 
    Math::Clamp(percent, 0.0f, 100.0f) / 100.0f, time), this);
}

//**************************************************************************************************
void DelayNode::InterpolateWetValue(float wetLevel, float time)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&DelayNode::InterpolateWetValueThreaded, this,
    Math::Clamp(wetLevel, 0.0f, 1.0f), time), this);
}

//**************************************************************************************************
bool DelayNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  unsigned outputBufferSize = outputBuffer->Size();

  // Get input
  bool isThereInput = AccumulateInputSamples(outputBufferSize, numberOfChannels, listener);

  // Check if the listener is in the map
  DelayLine* filter = FiltersPerListener.FindValue(listener, nullptr);
  if (!filter)
  {
    filter = new DelayLine;
    filter->SetDelayMSec(mDelaySec.Get(AudioThreads::MixThread) * 1000.0f);
    filter->SetFeedback(mFeedbackValue.Get(AudioThreads::MixThread));
    filter->SetWetLevel(mWetValue.Get(AudioThreads::MixThread));
    FiltersPerListener[listener] = filter;
  }

  // If no input and delayed output is done, return
  if (!isThereInput && !filter->IsDataInBuffer())
    return false;

  // If there is no input data make sure the array is zeroed out
  if (!isThereInput)
    memset(mInputSamplesThreaded.Data(), 0, sizeof(float) * mInputSamplesThreaded.Size());

  // Apply filter
  filter->ProcessBuffer(mInputSamplesThreaded.Data(), outputBuffer->Data(), numberOfChannels, outputBufferSize);

  AddBypassThreaded(outputBuffer);

  return true;
}

//**************************************************************************************************
void DelayNode::RemoveListenerThreaded(SoundEvent* event)
{
  ListenerNode* listener = (ListenerNode*)event->mPointer;

  if (FiltersPerListener.FindValue(listener, nullptr))
  {
    delete FiltersPerListener[listener];
    FiltersPerListener.Erase(listener);
  }
}

//**************************************************************************************************
void DelayNode::SetDelayMsThreaded(float time)
{
  forRange(DelayLine* filter, FiltersPerListener.Values())
    filter->SetDelayMSec(time);
}

//**************************************************************************************************
void DelayNode::SetFeedbackThreaded(float value)
{
  forRange(DelayLine* filter, FiltersPerListener.Values())
    filter->SetFeedback(value);
}

//**************************************************************************************************
void DelayNode::SetWetValueThreaded(float value)
{
  forRange(DelayLine* filter, FiltersPerListener.Values())
    filter->SetWetLevel(value);
}

//**************************************************************************************************
void DelayNode::InterpolateWetValueThreaded(float value, float time)
{
  forRange(DelayLine* filter, FiltersPerListener.Values())
    filter->InterpolateWetLevel(value, time);
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
FlangerNode::FlangerNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mMaxDelayMS(5.0f),
  mModFrequency(0.18f),
  mFeedback(0.0f)
{

}

//**************************************************************************************************
FlangerNode::~FlangerNode()
{
  forRange(Data* data, FiltersPerListener.Values())
    delete data;
}

//**************************************************************************************************
float FlangerNode::GetMaxDelayMillisec()
{
  return mMaxDelayMS.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void FlangerNode::SetMaxDelayMillisec(float delay)
{
  mMaxDelayMS.Set(Math::Max(delay, 0.0f), AudioThreads::MainThread);
}

//**************************************************************************************************
float FlangerNode::GetModulationFrequency()
{
  return mModFrequency.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void FlangerNode::SetModulationFrequency(float frequency)
{
  frequency = Math::Clamp(frequency, 0.0f, 20000.0f);
  mModFrequency.Set(frequency, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&FlangerNode::SetModFreqThreaded, this, frequency), this);
}

//**************************************************************************************************
float FlangerNode::GetFeedbackPercent()
{
  return mFeedback.Get(AudioThreads::MainThread) * 100.0f;
}

//**************************************************************************************************
void FlangerNode::SetFeedbackPercent(float percent)
{
  percent = Math::Clamp(percent, 0.0f, 100.0f) / 100.0f;
  mFeedback.Set(percent, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&FlangerNode::SetFeedbackThreaded, this, percent), this);
}

//**************************************************************************************************
float FlangerNode::GetFeedbackValue()
{
  return mFeedback.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void FlangerNode::SetFeedbackValue(float value)
{
  value = Math::Clamp(value, 0.0f, 1.0f);
  mFeedback.Set(value, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&FlangerNode::SetFeedbackThreaded, this, value), this);
}

//**************************************************************************************************
bool FlangerNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  unsigned bufferSize = outputBuffer->Size();

  // Get input and return if there is no data
  if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
    return false;

  // Check if the listener is in the map
  Data* filter = FiltersPerListener.FindValue(listener, nullptr);
  if (!filter)
  {
    filter = new Data(mModFrequency.Get(AudioThreads::MixThread), mFeedback.Get(AudioThreads::MixThread),
      mMaxDelayMS.Get(AudioThreads::MixThread));
    FiltersPerListener[listener] = filter;
  }

  // Apply filter
  for (unsigned frame = 0; frame < bufferSize; frame += numberOfChannels)
  {
    filter->Delay.SetDelayMSec(filter->LFO.GetNextSample() * mMaxDelayMS.Get(AudioThreads::MixThread));

    filter->Delay.ProcessBuffer(mInputSamplesThreaded.Data() + frame, outputBuffer->Data() + frame,
      numberOfChannels, numberOfChannels);
  }

  AddBypassThreaded(outputBuffer);

  return true;
}

//**************************************************************************************************
void FlangerNode::RemoveListenerThreaded(SoundEvent* event)
{
  ListenerNode* listener = (ListenerNode*)event->mPointer;

  if (FiltersPerListener.FindValue(listener, nullptr))
  {
    delete FiltersPerListener[listener];
    FiltersPerListener.Erase(listener);
  }
}

//**************************************************************************************************
void FlangerNode::SetModFreqThreaded(float frequency)
{
  forRange(Data* data, FiltersPerListener.Values())
    data->LFO.SetFrequency(frequency);
}

//**************************************************************************************************
void FlangerNode::SetFeedbackThreaded(float value)
{
  forRange(Data* data, FiltersPerListener.Values())
    data->Delay.SetFeedback(value);
}

//**************************************************************************************************
void FlangerNode::SetMaxDelayMSecThreaded(float delay)
{
  forRange(Data* data, FiltersPerListener.Values())
    data->Delay.SetMaxDelayMSec(delay, cMaxChannels);
}

//**************************************************************************************************
FlangerNode::Data::Data(float frequency, float feedback, float maxDelayMs)
{
  Delay.SetMaxDelayMSec(maxDelayMs, Z::gSound->Mixer.mSystemChannels.Get(AudioThreads::MixThread));
  Delay.SetDelayMSec(0);
  Delay.SetFeedback(feedback);
  Delay.SetWetLevel(0.5f);
  LFO.SetFrequency(frequency);
  LFO.SetPolarity(Oscillator::Unipolar);
  LFO.SetNoteOn(true);
}

//-------------------------------------------------------------------------------------- Chorus Node

//************************************************************************************************
ChorusNode::Data::Data(float frequency, float minDelay, float feedback)
{
  Delay.SetDelayMSec(minDelay);
  Delay.SetFeedback(feedback);
  Delay.SetWetLevel(0.5f);
  LFO.SetFrequency(frequency);
  LFO.SetPolarity(Oscillator::Unipolar);
  LFO.SetNoteOn(true);
}

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
ChorusNode::ChorusNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mMinDelayMS(5.0f),
  mMaxDelayMS(20.0f),
  mModFrequency(0.1f),
  mFeedback(0.0f),
  mChorusOffsetMS(40.0f)
{

}

//**************************************************************************************************
ChorusNode::~ChorusNode()
{
  forRange(Data* data, FiltersPerListener.Values())
    delete data;
}

//**************************************************************************************************
float ChorusNode::GetMaxDelayMillisec()
{
  return mMaxDelayMS.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void ChorusNode::SetMaxDelayMillisec(float delay)
{
  mMaxDelayMS.Set(Math::Max(delay, 0.0f), AudioThreads::MainThread);
}

//**************************************************************************************************
float ChorusNode::GetMinDelayMillisec()
{
  return mMinDelayMS.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void ChorusNode::SetMinDelayMillisec(float delay)
{
  mMinDelayMS.Set(Math::Max(delay, 0.0f), AudioThreads::MainThread);
}

//**************************************************************************************************
float ChorusNode::GetModulationFrequency()
{
  return mModFrequency.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void ChorusNode::SetModulationFrequency(float frequency)
{
  frequency = Math::Max(frequency, 0.0f);
  mModFrequency.Set(frequency, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&ChorusNode::SetModFreqThreaded, this, frequency), this);
}

//**************************************************************************************************
float ChorusNode::GetFeedbackPercent()
{
  return mFeedback.Get(AudioThreads::MainThread) * 100.0f;
}

//**************************************************************************************************
void ChorusNode::SetFeedbackPercent(float percent)
{
  percent = Math::Clamp(percent, 0.0f, 100.0f) / 100.0f;
  mFeedback.Set(percent, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&ChorusNode::SetFeedbackThreaded, this, percent), this);
}

//**************************************************************************************************
float ChorusNode::GetFeedbackValue()
{
  return mFeedback.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void ChorusNode::SetFeedbackValue(float value)
{
  value = Math::Clamp(value, 0.0f, 1.0f);
  mFeedback.Set(value, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&ChorusNode::SetFeedbackThreaded, this, value), this);
}

//**************************************************************************************************
float ChorusNode::GetOffsetMillisec()
{
  return mChorusOffsetMS.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void ChorusNode::SetOffsetMillisec(float offset)
{
  mChorusOffsetMS.Set(Math::Max(offset, 0.0f), AudioThreads::MainThread);
}

//**************************************************************************************************
bool ChorusNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  unsigned bufferSize = outputBuffer->Size();

  // Get input and return if there is no data
  if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
    return false;

  // Check if the listener is in the map
  Data* filter = FiltersPerListener.FindValue(listener, nullptr);
  if (!filter)
  {
    filter = new Data(mModFrequency.Get(AudioThreads::MixThread), mMinDelayMS.Get(AudioThreads::MixThread), 
      mFeedback.Get(AudioThreads::MixThread));
    FiltersPerListener[listener] = filter;
  }

  // Apply filter
  for (unsigned frame = 0; frame < bufferSize; frame += numberOfChannels)
  {
    filter->Delay.SetDelayMSec((filter->LFO.GetNextSample() 
      * (mMaxDelayMS.Get(AudioThreads::MixThread) - mMinDelayMS.Get(AudioThreads::MixThread))) 
      + (mMinDelayMS.Get(AudioThreads::MixThread) + mChorusOffsetMS.Get(AudioThreads::MixThread)));

    filter->Delay.ProcessBuffer(mInputSamplesThreaded.Data() + frame, outputBuffer->Data() + frame,
      numberOfChannels, numberOfChannels);
  }

  AddBypassThreaded(outputBuffer);

  return true;
}

//**************************************************************************************************
void ChorusNode::RemoveListenerThreaded(SoundEvent* event)
{
  ListenerNode* listener = (ListenerNode*)event->mPointer;

  if (FiltersPerListener.FindValue(listener, nullptr))
  {
    delete FiltersPerListener[listener];
    FiltersPerListener.Erase(listener);
  }
}

//**************************************************************************************************
void ChorusNode::SetModFreqThreaded(float frequency)
{
  forRange(Data* data, FiltersPerListener.Values())
    data->LFO.SetFrequency(frequency);
}

//**************************************************************************************************
void ChorusNode::SetFeedbackThreaded(float value)
{
  forRange(Data* data, FiltersPerListener.Values())
    data->Delay.SetFeedback(value);
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
CompressorNode::CompressorNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mInputGainDB(0.0f),
  mThresholdDB(0.0f),
  mAttackMSec(20.0f),
  mReleaseMSec(1000.0f),
  mRatio(1.0f),
  mOutputGainDB(0.0f),
  mKneeWidth(0.0f)
{
  Filter.SetType(DynamicsProcessor::Compressor);
}

//**************************************************************************************************
float CompressorNode::GetInputGainDecibels()
{
  return mInputGainDB;
}

//**************************************************************************************************
void CompressorNode::SetInputGainDecibels(float dB)
{
  mInputGainDB = Math::Clamp(dB, cMinDecibelsValue, cMaxDecibelsValue);

  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetInputGain, &Filter, mInputGainDB), this);
}

//**************************************************************************************************
float CompressorNode::GetThresholdDecibels()
{
  return mThresholdDB;
}

//**************************************************************************************************
void CompressorNode::SetThresholdDecibels(float dB)
{
  mThresholdDB = Math::Clamp(dB, cMinDecibelsValue, cMaxDecibelsValue);

  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetThreshold, &Filter, mThresholdDB), this);
}

//**************************************************************************************************
float CompressorNode::GetAttackMillisec()
{
  return mAttackMSec;
}

//**************************************************************************************************
void CompressorNode::SetAttackMillisec(float attack)
{
  mAttackMSec = Math::Max(attack, 0.0f);

  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetAttackMSec, &Filter, mAttackMSec), this);
}

//**************************************************************************************************
float CompressorNode::GetReleaseMillisec()
{
  return mReleaseMSec;
}

//**************************************************************************************************
void CompressorNode::SetReleaseMillisec(float release)
{
  mReleaseMSec = Math::Max(release, mReleaseMSec);

  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetReleaseMSec, &Filter, mReleaseMSec), this);
}

//**************************************************************************************************
float CompressorNode::GetRatio()
{
  return mRatio;
}

//**************************************************************************************************
void CompressorNode::SetRatio(float ratio)
{
  mRatio = ratio;

  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetRatio, &Filter, mRatio), this);
}

//**************************************************************************************************
float CompressorNode::GetOutputGainDecibels()
{
  return mOutputGainDB;
}

//**************************************************************************************************
void CompressorNode::SetOutputGainDecibels(float dB)
{
  mOutputGainDB = Math::Clamp(dB, cMinDecibelsValue, cMaxDecibelsValue);

  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetOutputGain, &Filter, mOutputGainDB), this);
}

//**************************************************************************************************
float CompressorNode::GetKneeWidth()
{
  return mKneeWidth;
}

//**************************************************************************************************
void CompressorNode::SetKneeWidth(float knee)
{
  mKneeWidth = knee;

  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetKneeWidth, &Filter, mKneeWidth), this);
}

//**************************************************************************************************
bool CompressorNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  unsigned bufferSize = outputBuffer->Size();

  // Get input and return if there is no data
  if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
    return false;

  // Apply filter
  Filter.ProcessBuffer(mInputSamplesThreaded.Data(), mInputSamplesThreaded.Data(), outputBuffer->Data(),
    numberOfChannels, bufferSize);

  AddBypassThreaded(outputBuffer);

  return true;
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
ExpanderNode::ExpanderNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mInputGainDB(0.0f),
  mThresholdDB(0.0f),
  mAttackMSec(20.0f),
  mReleaseMSec(1000.0f),
  mRatio(1.0f),
  mOutputGainDB(0.0f),
  mKneeWidth(0.0f)
{
  Filter.SetType(DynamicsProcessor::Expand);
}

//**************************************************************************************************
float ExpanderNode::GetInputGainDecibels()
{
  return mInputGainDB;
}

//**************************************************************************************************
void ExpanderNode::SetInputGainDecibels(float dB)
{
  mInputGainDB = Math::Clamp(dB, cMinDecibelsValue, cMaxDecibelsValue);

  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetInputGain, &Filter, mInputGainDB), this);
}

//**************************************************************************************************
float ExpanderNode::GetThresholdDecibels()
{
  return mThresholdDB;
}

//**************************************************************************************************
void ExpanderNode::SetThresholdDecibels(float dB)
{
  mThresholdDB = Math::Clamp(dB, cMinDecibelsValue, cMaxDecibelsValue);

  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetThreshold, &Filter, mThresholdDB), this);
}

//**************************************************************************************************
float ExpanderNode::GetAttackMillisec()
{
  return mAttackMSec;
}

//**************************************************************************************************
void ExpanderNode::SetAttackMillisec(float attack)
{
  mAttackMSec = Math::Max(attack, 0.0f);

  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetAttackMSec, &Filter, mAttackMSec), this);
}

//**************************************************************************************************
float ExpanderNode::GetReleaseMillisec()
{
  return mReleaseMSec;
}

//**************************************************************************************************
void ExpanderNode::SetReleaseMillisec(float release)
{
  mReleaseMSec = Math::Max(release, 0.0f);

  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetReleaseMSec, &Filter, mReleaseMSec), this);
}

//**************************************************************************************************
float ExpanderNode::GetRatio()
{
  return mRatio;
}

//**************************************************************************************************
void ExpanderNode::SetRatio(float ratio)
{
  mRatio = ratio;

  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetRatio, &Filter, mRatio), this);
}

//**************************************************************************************************
float ExpanderNode::GetOutputGainDecibels()
{
  return mOutputGainDB;
}

//**************************************************************************************************
void ExpanderNode::SetOutputGainDecibels(float dB)
{
  mOutputGainDB = Math::Clamp(dB, cMinDecibelsValue, cMaxDecibelsValue);

  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetOutputGain, &Filter, mOutputGainDB), this);
}

//**************************************************************************************************
float ExpanderNode::GetKneeWidth()
{
  return mKneeWidth;
}

//**************************************************************************************************
void ExpanderNode::SetKneeWidth(float knee)
{
  mKneeWidth = knee;

  Z::gSound->Mixer.AddTask(CreateFunctor(&DynamicsProcessor::SetKneeWidth, &Filter, mKneeWidth), this);
}

//**************************************************************************************************
bool ExpanderNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  unsigned bufferSize = outputBuffer->Size();

  // Get input and return if there is no data
  if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
    return false;

  // Apply filter
  Filter.ProcessBuffer(mInputSamplesThreaded.Data(), mInputSamplesThreaded.Data(), outputBuffer->Data(),
    numberOfChannels, bufferSize);

  AddBypassThreaded(outputBuffer);

  return true;
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
float ValueFromDecibels(float decibels)
{
  return Math::Pow(10.0f, 0.05f * decibels);
}

//**************************************************************************************************
float ValueFromFrequency(float frequency)
{
  return cSystemSampleRate * 0.5f / frequency;
}

//**************************************************************************************************
AddNoiseNode::AddNoiseNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mAdditiveNoiseDB(-40.0f),
  mMultipleNoiseDB(-10.0f),
  mAdditiveNoiseCutoffHz(2000.0f),
  mMultipleNoiseCutoffHz(50.05),
  mAddCountThreaded(0),
  mMultiplyCountThreaded(0)
{
  mAddPeriodThreaded = ValueFromFrequency(mAdditiveNoiseCutoffHz);
  mMultiplyPeriodThreaded = ValueFromFrequency(mMultipleNoiseCutoffHz);
  mAddGainThreaded = ValueFromDecibels(mAdditiveNoiseDB);
  mMultiplyGainThreaded = ValueFromDecibels(mMultipleNoiseDB);
}

//**************************************************************************************************
float AddNoiseNode::GetAdditiveGain()
{
  return mAdditiveNoiseDB;
}

//**************************************************************************************************
void AddNoiseNode::SetAdditiveGain(float decibels)
{
  mAdditiveNoiseDB = Math::Clamp(decibels, cMinDecibelsValue, cMaxDecibelsValue);

  Z::gSound->Mixer.AddTask(CreateFunctor(&AddNoiseNode::mAddGainThreaded, this,
    ValueFromDecibels(mAdditiveNoiseDB)), this);
}

//**************************************************************************************************
float AddNoiseNode::GetMultiplicativeGain()
{
  return mMultipleNoiseDB;
}

//**************************************************************************************************
void AddNoiseNode::SetMultiplicativeGain(float decibels)
{
  mMultipleNoiseDB = Math::Clamp(decibels, cMinDecibelsValue, cMaxDecibelsValue);

  Z::gSound->Mixer.AddTask(CreateFunctor(&AddNoiseNode::mMultiplyGainThreaded, this,
    ValueFromDecibels(mMultipleNoiseDB)), this);
}

//**************************************************************************************************
float AddNoiseNode::GetAdditiveCutoff()
{
  return mAdditiveNoiseCutoffHz;
}

//**************************************************************************************************
void AddNoiseNode::SetAdditiveCutoff(float frequency)
{
  mAdditiveNoiseCutoffHz = Math::Max(frequency, 0.0f);

  Z::gSound->Mixer.AddTask(CreateFunctor(&AddNoiseNode::mAddPeriodThreaded, this,
    ValueFromFrequency(mAdditiveNoiseCutoffHz)), this);
}

//**************************************************************************************************
float AddNoiseNode::GetMultiplicativeCutoff()
{
  return mMultipleNoiseCutoffHz;
}

//**************************************************************************************************
void AddNoiseNode::SetMultiplicativeCutoff(float frequency)
{
  mMultipleNoiseCutoffHz = Math::Max(frequency, 0.0f);

  Z::gSound->Mixer.AddTask(CreateFunctor(&AddNoiseNode::mMultiplyPeriodThreaded, this,
    ValueFromFrequency(mMultipleNoiseCutoffHz)), this);
}

//**************************************************************************************************
bool AddNoiseNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  unsigned bufferSize = outputBuffer->Size();

  // Get input and return if there is no data
  if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
    return false;

  BufferRange outputRange = outputBuffer->All(), inputRange = mInputSamplesThreaded.All();
  while (!outputRange.Empty())
  {
    for (unsigned j = 0; j < numberOfChannels; ++j, outputRange.PopFront(), inputRange.PopFront())
    {
      outputRange.Front() = (inputRange.Front() * (1.0f - mMultiplyGainThreaded +
        (mMultiplyGainThreaded * mMultiplyNoiseThreaded))) + (mAddGainThreaded * mAddNoiseThreaded);
    }

    ++mAddCountThreaded;
    if (mAddCountThreaded >= mAddPeriodThreaded)
    {
      mAddCountThreaded -= (int)mAddPeriodThreaded;
      mAddNoiseThreaded = mRandomValuesThreaded.FloatRange(-1.0f, 1.0f);
    }

    ++mMultiplyCountThreaded;
    if (mMultiplyCountThreaded >= mMultiplyPeriodThreaded)
    {
      mMultiplyCountThreaded -= (int)mMultiplyPeriodThreaded;
      mMultiplyNoiseThreaded = mRandomValuesThreaded.FloatRange(0.0f, 1.0f);
    }
  }

  AddBypassThreaded(outputBuffer);

  return true;
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
ModulationNode::ModulationNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mAmplitude(false),
  mFrequency(10.0f),
  mWetLevelValue(1.0f)
{

}

//**************************************************************************************************
ModulationNode::~ModulationNode()
{
  forRange(Oscillator* osc, OscillatorsPerListenerThreaded.Values())
    delete osc;
}

//**************************************************************************************************
bool ModulationNode::GetUseAmplitudeModulation()
{
  return mAmplitude.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void ModulationNode::SetUseAmplitudeModulation(bool useAmplitude)
{
  mAmplitude.Set(useAmplitude, AudioThreads::MainThread);

  Z::gSound->Mixer.AddTask(CreateFunctor(&ModulationNode::SetUseAmplitudeThreaded, this, useAmplitude),
    this);
}

//**************************************************************************************************
float ModulationNode::GetFrequency()
{
  return mFrequency.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void ModulationNode::SetFrequency(float frequency)
{
  mFrequency.Set(Math::Max(frequency, 0.0f), AudioThreads::MainThread);

  Z::gSound->Mixer.AddTask(CreateFunctor(&ModulationNode::SetFrequencyThreaded, this,
    mFrequency.Get(AudioThreads::MainThread)), this);
}

//**************************************************************************************************
float ModulationNode::GetWetPercent()
{
  return mWetLevelValue.Get(AudioThreads::MainThread) * 100.0f;
}

//**************************************************************************************************
void ModulationNode::SetWetPercent(float percent)
{
  mWetLevelValue.Set(Math::Clamp(percent, 0.0f, 100.0f) / 100.0f, AudioThreads::MainThread);
}

//**************************************************************************************************
float ModulationNode::GetWetValue()
{
  return mWetLevelValue.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void ModulationNode::SetWetValue(float value)
{
  mWetLevelValue.Set(Math::Clamp(value, 0.0f, 1.0f), AudioThreads::MainThread);
}

//**************************************************************************************************
bool ModulationNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  unsigned bufferSize = outputBuffer->Size();

  // Get input and return if there is no data
  if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
    return false;

  // Check if the listener is in the map
  Oscillator* sineWave = OscillatorsPerListenerThreaded.FindValue(listener, nullptr);
  if (!sineWave)
  {
    sineWave = new Oscillator;
    sineWave->SetNoteOn(true);
    sineWave->SetFrequency(mFrequency.Get(AudioThreads::MixThread));
    if (mAmplitude.Get(AudioThreads::MixThread))
      sineWave->SetPolarity(Oscillator::Unipolar);
    else
      sineWave->SetPolarity(Oscillator::Bipolar);
    OscillatorsPerListenerThreaded[listener] = sineWave;
  }

  // Go through all frames
  BufferRange outputRange = outputBuffer->All(), inputRange = mInputSamplesThreaded.All();
  while (!outputRange.Empty())
  {
    float waveValue = sineWave->GetNextSample();

    // Multiply signal with modulator wave, taking into account gain and wet percent
    for (unsigned channel = 0; channel < numberOfChannels; ++channel, outputRange.PopFront(),
      inputRange.PopFront())
    {
      outputRange.Front() = (inputRange.Front() * waveValue * mWetLevelValue.Get(AudioThreads::MixThread)) 
        + (inputRange.Front() * (1.0f - mWetLevelValue.Get(AudioThreads::MixThread)));
    }
  }

  AddBypassThreaded(outputBuffer);

  return true;
}

//**************************************************************************************************
void ModulationNode::RemoveListenerThreaded(SoundEvent* event)
{
  ListenerNode* listener = (ListenerNode*)event->mPointer;

  if (OscillatorsPerListenerThreaded.FindValue(listener, nullptr))
  {
    delete OscillatorsPerListenerThreaded[listener];
    OscillatorsPerListenerThreaded.Erase(listener);
  }
}

//**************************************************************************************************
void ModulationNode::SetUseAmplitudeThreaded(bool useAmplitude)
{
  forRange(Oscillator* sineWave, OscillatorsPerListenerThreaded.Values())
  {
    if (useAmplitude)
      sineWave->SetPolarity(Oscillator::Unipolar);
    else
      sineWave->SetPolarity(Oscillator::Bipolar);
  }
}

//**************************************************************************************************
void ModulationNode::SetFrequencyThreaded(float frequency)
{
  forRange(Oscillator* sineWave, OscillatorsPerListenerThreaded.Values())
    sineWave->SetFrequency(frequency);
}

} // namespace Zero
