///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------- Attenuator Node

//**************************************************************************************************
ZilchDefineType(AttenuatorNode, builder, type)
{

}

//**************************************************************************************************
AttenuatorNode::AttenuatorNode(StringParam name, const unsigned ID, Math::Vec3Param position, 
    float startDistance, float endDistance, float minVolume, const FalloffCurveType::Enum curveType,
    Array<Math::Vec3>* customCurveData) :
  SimpleCollapseNode(name, ID, true, false),
  mAttenStartDist(startDistance),
  mUseLowPass(false),
  mLowPassDistance(endDistance * 0.5f),
  mAttenEndDist(endDistance),
  mMinimumVolume(minVolume),
  mPosition(position),
  mLowPassCutoff(cLowPassCutoffHighValue)
{
  // Set values on interpolator
  DistanceInterpolator.SetValues(1.0f, minVolume, endDistance - startDistance);

  // Set the falloff curve on the interpolator
  if (curveType == FalloffCurveType::Custom)
  {
    if (customCurveData)
      DistanceInterpolator.SetCustomCurve(customCurveData);
  }
  else
    DistanceInterpolator.SetCurve(curveType);

  // Set the low pass interpolator 
  LowPassInterpolator.SetValues(cLowPassCutoffHighValue, cLowPassCutoffLowValue, endDistance * 0.5f);
}

//**************************************************************************************************
AttenuatorNode::~AttenuatorNode()
{
  forRange(AttenuationPerListener* data, DataPerListener.Values())
    delete data;
}

//**************************************************************************************************
void AttenuatorNode::SetPosition(Math::Vec3Param position)
{
  mPosition.Set(position, AudioThreads::MainThread);
}

//**************************************************************************************************
void AttenuatorNode::SetStartDistance(float distance)
{
  mAttenStartDist.Set(distance, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&AttenuatorNode::UpdateDistanceInterpolator, this), this);
}

//**************************************************************************************************
void AttenuatorNode::SetEndDistance(float distance)
{
  mAttenEndDist.Set(distance, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&AttenuatorNode::UpdateDistanceInterpolator, this), this);
}

//**************************************************************************************************
void AttenuatorNode::SetMinimumVolume(float volume)
{
  mMinimumVolume.Set(volume, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&AttenuatorNode::UpdateDistanceInterpolator, this), this);
}

//**************************************************************************************************
void AttenuatorNode::SetCurveType(const FalloffCurveType::Enum type, Array<Math::Vec3> *customCurveData)
{
  if (customCurveData)
    // Interpolator will delete curve on destruction or when replaced with another curve
    Z::gSound->Mixer.AddTask(CreateFunctor(&InterpolatingObject::SetCustomCurve, &DistanceInterpolator,
      new Array<Math::Vec3>(*customCurveData)), this);
  else
    Z::gSound->Mixer.AddTask(CreateFunctor(&InterpolatingObject::SetCurve, &DistanceInterpolator, type), this);
}

//**************************************************************************************************
void AttenuatorNode::SetUsingLowPass(const bool useLowPass)
{
  mUseLowPass.Set(useLowPass, AudioThreads::MainThread);
}

//**************************************************************************************************
void AttenuatorNode::SetLowPassDistance(const float distance)
{
  mLowPassDistance.Set(distance, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&AttenuatorNode::UpdateLowPassInterpolator, this), this);
}

//**************************************************************************************************
void AttenuatorNode::SetLowPassCutoffFreq(const float frequency)
{
  mLowPassCutoff.Set(frequency, AudioThreads::MainThread);
  Z::gSound->Mixer.AddTask(CreateFunctor(&AttenuatorNode::UpdateLowPassInterpolator, this), this);
}

//**************************************************************************************************
bool AttenuatorNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  unsigned bufferSize = outputBuffer->Size();

  // Get input and return if there is no data
  if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
    return false;

  // If no listener then no attenuation
  if (!listener)
  {
    mInputSamplesThreaded.Swap(*outputBuffer);
    return true;
  }

  // Get the relative position with the listener
  Math::Vec3 relativePosition = listener->GetRelativePositionThreaded(mPosition.Get(AudioThreads::MixThread));
  // Save the distance value
  float distance = relativePosition.Length();

  // Account for the listener's attenuation scale (this is supposed to act like a multiplier on the start and end distances)
  if (listener->GetAttenuationScale() <= 0.0f)
    distance = mAttenEndDist.Get(AudioThreads::MixThread);
  else
    distance /= listener->GetAttenuationScale();

  // If we are outside the max distance and the minimum volume is zero, there is no audio
  if (distance >= mAttenEndDist.Get(AudioThreads::MixThread) && mMinimumVolume.Get(AudioThreads::MixThread) == 0.0f)
    return false;

  float attenuatedVolume;
  // If the distance is further than the attenuation end distance, the volume is the end volume
  if (distance >= mAttenEndDist.Get(AudioThreads::MixThread))
    attenuatedVolume = mMinimumVolume.Get(AudioThreads::MixThread);
  // If the distance is less than the attenuation start distance, the volume is not attenuated
  else if (distance <= mAttenStartDist.Get(AudioThreads::MixThread))
    attenuatedVolume = 1.0f;
  // If the attenuation start and end are too close together than just use end volume
  else if (mAttenEndDist.Get(AudioThreads::MixThread) - mAttenStartDist.Get(AudioThreads::MixThread) <= 0.1f)
    attenuatedVolume = mMinimumVolume.Get(AudioThreads::MixThread);
  // Otherwise, get the value using the falloff curve on the interpolator
  else
    attenuatedVolume = DistanceInterpolator.ValueAtDistance(distance - mAttenStartDist.Get(AudioThreads::MixThread));

  // Check if the listener needs to be added to the map
  if (!DataPerListener.FindValue(listener, nullptr))
    DataPerListener[listener] = new AttenuationPerListener();

  AttenuationPerListener& listenerData = *DataPerListener[listener];

  // Apply volume adjustment to each frame of samples
  InterpolatingObject volume;
  volume.SetValues(listenerData.PreviousVolume, attenuatedVolume, bufferSize / numberOfChannels);
  for (BufferRange outputRange = outputBuffer->All(), inputRange = mInputSamplesThreaded.All(); 
    !outputRange.Empty(); )
  {
    float frameVolume = volume.NextValue();

    // Copy samples from input buffer and apply volume adjustment
    for (unsigned j = 0; j < numberOfChannels; ++j, outputRange.PopFront(), inputRange.PopFront())
      outputRange.Front() = inputRange.Front() * frameVolume;
  }

  // Store the previous volume
  listenerData.PreviousVolume = attenuatedVolume;

  // Check if using low pass filter
  if (mUseLowPass.Get(AudioThreads::MixThread) && distance > mLowPassDistance.Get(AudioThreads::MixThread))
  {
    // Find cutoff frequency for this distance (will return end value if distance is past AttenEndDist)
    float cutoffFreq = LowPassInterpolator.ValueAtDistance(distance - mLowPassDistance.Get(AudioThreads::MixThread));

    // Set the cutoff frequency on the filter
    listenerData.LowPass.SetCutoffFrequency(cutoffFreq);

    // Apply the filter to each frame of audio samples
    listenerData.LowPass.ProcessBuffer(outputBuffer->Data(), outputBuffer->Data(), numberOfChannels,
      outputBuffer->Size());
  }

  AddBypassThreaded(outputBuffer);

  return true;
}

//**************************************************************************************************
float AttenuatorNode::GetVolumeChangeFromOutputsThreaded()
{
  float volume = 0.0f;

  // Get all volumes from outputs
  forRange(HandleOf<SoundNode> node, GetOutputs(AudioThreads::MixThread)->All())
    volume += node->GetVolumeChangeFromOutputsThreaded();

  // If there are multiple listeners, the sounds they hear are added together
  float attenuatorVolume = 0.0f;
  forRange(AttenuationPerListener* data, DataPerListener.Values())
    attenuatorVolume += data->PreviousVolume;

  // Return the output volume modified by this node's volume
  return volume * attenuatorVolume;
}

//**************************************************************************************************
void AttenuatorNode::RemoveListenerThreaded(SoundEvent* event)
{
  ListenerNode* listener = (ListenerNode*)event->mPointer;

  // Check if the listener is in the map
  if (listener && DataPerListener.FindValue(listener, nullptr))
  {
    // Delete the listener's data and remove it from the map
    delete DataPerListener[listener];
    DataPerListener.Erase(listener);
  }
}

//**************************************************************************************************
void AttenuatorNode::UpdateDistanceInterpolator()
{
  DistanceInterpolator.SetValues(1.0f, mMinimumVolume.Get(AudioThreads::MixThread), 
    mAttenEndDist.Get(AudioThreads::MixThread) - mAttenStartDist.Get(AudioThreads::MixThread));
}

//**************************************************************************************************
void AttenuatorNode::UpdateLowPassInterpolator()
{
  LowPassInterpolator.SetValues(cLowPassCutoffHighValue, mLowPassCutoff.Get(AudioThreads::MixThread), 
    mAttenEndDist.Get(AudioThreads::MixThread) - mAttenStartDist.Get(AudioThreads::MixThread));
}

} // namespace Zero
