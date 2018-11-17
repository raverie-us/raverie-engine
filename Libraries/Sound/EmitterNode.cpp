///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

using namespace AudioConstants;

//------------------------------------------------------------------------ Emitter Data Per Listener

//**************************************************************************************************
EmitterDataPerListener::EmitterDataPerListener() :
  mPreviousRelativePosition(Math::Vec3(FLT_MAX, FLT_MAX, FLT_MAX)),
  mDirectionalVolume(1.0f),
  mUseLowPass(false)
{
  memset(mPreviousGains, 0, sizeof(float) * cMaxChannels);
}

//------------------------------------------------------------------------------------- Emitter Node

//**************************************************************************************************
ZilchDefineType(EmitterNode, builder, type)
{

}

//**************************************************************************************************
EmitterNode::EmitterNode(StringParam name, const unsigned ID, Math::Vec3Param position, 
    Math::Vec3Param velocity) :
  SoundNode(name, ID, true, false),
  mPosition(position),
  mVelocity(velocity),
  mFacingDirection(1.0f, 0.0f, 0.0f),
  mPausing(false),
  mPaused(false),
  mDirectionalAngleRadians(0.0f)
{
  PanningObject.Initialize(Z::gSound->Mixer.mSystemChannels.Get(AudioThreads::MainThread));
}

//**************************************************************************************************
void EmitterNode::SetPausedThreaded(bool paused)
{
  // If we should be paused and currently are not
  if (paused && !mPaused && !mPausing)
  {
    mPausing = true;
    VolumeInterpolator.SetValues(1.0f, 0.0f, cPropertyChangeFrames);
  }
  // If we should resume playing and are currently paused
  else if (!paused && (mPaused || mPausing))
  {
    mPaused = false;
    mPausing = false;
    VolumeInterpolator.SetValues(1.0f, cPropertyChangeFrames);
  }
}

//**************************************************************************************************
void EmitterNode::SetPositionThreaded(const Math::Vec3 newPosition, const Math::Vec3 newVelocity)
{
  mVelocity = newVelocity;
  mPosition = newPosition;
}

//**************************************************************************************************
void EmitterNode::SetForwardDirectionThreaded(const Math::Vec3 forwardDirection)
{
  mFacingDirection = forwardDirection;
}

//**************************************************************************************************
void EmitterNode::SetDirectionalAngleThreaded(const float angleInDegrees, const float reducedVolume)
{
  // Store half angle, in radians
  mDirectionalAngleRadians = Math::DegToRad(angleInDegrees) * 0.5f;

  DirectionalInterpolator.SetValues(1.0f, reducedVolume, Math::cPi - mDirectionalAngleRadians);
}

//**************************************************************************************************
bool EmitterNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  // If paused, do nothing
  if (mPaused)
    return false;

  unsigned bufferSize = outputBuffer->Size();

  // Get input and return if there is no data
  if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
    return false;

  // If only one channel of output or no listener, no spatialization, so just return
  if (numberOfChannels == 1 || !listener)
  {
    // Move the input samples to the output buffer
    mInputSamplesThreaded.Swap(*outputBuffer);
    return true;
  }

  // Get the relative position between emitter and listener
  Math::Vec3 relativePosition = listener->GetRelativePositionThreaded(mPosition);

  // If necessary, add new listener data
  if (!DataPerListener.FindPointer(listener))
    DataPerListener[listener] = new EmitterDataPerListener();

  // Save reference for ease of use
  EmitterDataPerListener& listenerData = *DataPerListener[listener];

  // Check if the listener or emitter has moved (don't care about up/down changes)
  bool valuesChanged = false;
  if (!IsWithinLimit(relativePosition.x, listenerData.mPreviousRelativePosition.x, cMinimumPositionChange)
    || !IsWithinLimit(relativePosition.z, listenerData.mPreviousRelativePosition.z, cMinimumPositionChange))
  {
    CalculateData(&listenerData, relativePosition, listener, numberOfChannels);
    valuesChanged = true;
  }

  // Save the relative position
  listenerData.mPreviousRelativePosition = relativePosition;

  // Apply low pass filter to output (if turned on)
  if (listenerData.mUseLowPass)
    listenerData.LowPass.ProcessBuffer(mInputSamplesThreaded.Data(), outputBuffer->Data(), 
      numberOfChannels, bufferSize);
  // Otherwise move the input into the output buffer
  else
    outputBuffer->Swap(mInputSamplesThreaded);

  // Adjust each frame with gain values
  BufferRange outputRange1 = outputBuffer->All(), outputRange2 = outputBuffer->All();
  for (unsigned i = 0; i < bufferSize; i += numberOfChannels)
  {
    float volume = 1.0f;
    // If interpolating volume, get new volume value
    if (!VolumeInterpolator.Finished())
    {
      volume = VolumeInterpolator.NextValue();
      if (mPausing && VolumeInterpolator.Finished())
        mPaused = true;
    }

    // Adjust the volume if this is a directional emitter
    volume *= listenerData.mDirectionalVolume;

    // Combine all channels into one value
    // Leave unspatialized audio in all channels at minimum volume
    float monoValue = 0.0f;
    for (unsigned j = 0; j < numberOfChannels; ++j, outputRange1.PopFront())
    {
      monoValue += outputRange1.Front();
      outputRange1.Front() *= cMinimumVolume * volume;
    }
    monoValue /= numberOfChannels;
    monoValue *= volume;

    // Add spatialized audio using gain values
    for (unsigned j = 0; j < numberOfChannels; ++j, outputRange2.PopFront())
    {
      float multiplier = listenerData.mPreviousGains[j];

      // If the gain values changed, interpolate from old to new value
      if (valuesChanged)
        multiplier += (listenerData.mGainValues[j] - listenerData.mPreviousGains[j]) 
          * ((float)i / (float)bufferSize);

      outputRange2.Front() += monoValue * multiplier;
    }
  }

  // If gain values changed, copy new ones to previous values
  if (valuesChanged)
    memcpy(listenerData.mPreviousGains, listenerData.mGainValues, sizeof(float) * cMaxChannels);

  AddBypassThreaded(outputBuffer);

  return true;
}

//**************************************************************************************************
void EmitterNode::RemoveListenerThreaded(SoundEvent* event)
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
void EmitterNode::CalculateData(EmitterDataPerListener* data, const Math::Vec3& relativePosition, 
  ListenerNode* listener, const unsigned numberOfChannels)
{
  // Save reference for ease of use
  EmitterDataPerListener& listenerData = *data;

  // Don't need to use the height difference for positional calculations
  Math::Vec2 relativePosition2D(relativePosition.x, relativePosition.z);

  // Get the squared length
  float distanceSq = relativePosition2D.LengthSq();

  // Check if the emitter should be limited by direction 
  if (mDirectionalAngleRadians > 0.0f)
  {
    // Get the emitter's facing direction relative to the listener
    Math::Vec3 relativeFacing = listener->GetRelativeFacingThreaded(mFacingDirection);
    // Get the relative angle (facing should always be normalized)
    float angle = Math::ArcCos(Math::Dot(relativePosition.Normalized(), relativeFacing));

    // If the angle to the listener is greater than the emitter's angle, reduce volume
    if (angle > mDirectionalAngleRadians)
      listenerData.mDirectionalVolume = DirectionalInterpolator.ValueAtDistance(angle
        - mDirectionalAngleRadians);
  }
  else
    listenerData.mDirectionalVolume = 1.0f;

  // Check for sounds behind listener
  if (relativePosition2D.x < 0)
  {
    // Get the angle to the listener
    float angle = Math::ArcTan2(relativePosition.z, -relativePosition.x);
    // Translate this to a percentage of a quarter circle (this value should be 1.0 when the 
    // emitter is directly behind the listener and 0.0 when off to the side)
    float percent = Math::Abs(angle) / (Math::cPi / 2.0f);

    // The low pass cutoff frequency ranges from min to max depending on the angle
    // percentage, using a squared curve
    float frequency = cLowPassCutoffBase + (cLowPassCutoffAdditional * percent * percent);

    // Check if the difference between this frequency and the last frequency used is large
    if (!IsWithinLimit(frequency, listenerData.LowPass.GetCutoffFrequency(), cMaxLowPassDifference))
    {
      // Set frequency to be only maxDifferenceAllowed away from the last frequency used
      if (frequency > listenerData.LowPass.GetCutoffFrequency())
        frequency = listenerData.LowPass.GetCutoffFrequency() + cMaxLowPassDifference;
      else
        frequency = listenerData.LowPass.GetCutoffFrequency() - cMaxLowPassDifference;
    }

    // Set the cutoff frequency on the low pass filter
    listenerData.LowPass.SetCutoffFrequency(frequency);

    listenerData.mUseLowPass = true;
  }
  else
  {
    // Make sure the cutoff frequency is set to the maximum value
    listenerData.LowPass.SetCutoffFrequency(cLowPassCutoffBase + cLowPassCutoffAdditional);

    listenerData.mUseLowPass = false;
  }

  // Check if we need to re-initialize for a different number of channels
  if (numberOfChannels != PanningObject.GetNumberOfChannels())
    PanningObject.Initialize(numberOfChannels);

  PanningObject.ComputeGains(relativePosition2D, 0.0f, listenerData.mGainValues);
}

} // namespace Zero
