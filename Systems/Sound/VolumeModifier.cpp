///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{
//----------------------------------------------------------------------- Threaded Volume Modifier

//************************************************************************************************
InstanceVolumeModifier::InstanceVolumeModifier() :
  mCurrentVolume(1.0f),
  mLifetimeFrames(0),
  mLifetimeFrameCounter(0)
{

}

//************************************************************************************************
void InstanceVolumeModifier::ApplyVolume(float *sampleBuffer, const unsigned bufferSize,
  const unsigned numberOfChannels)
{
  if (!Active)
    return;

  // If this modifier has an end time, check if we should deactivate
  ++mLifetimeFrameCounter;
  if (mLifetimeFrames > 0 && mLifetimeFrameCounter > mLifetimeFrames)
  {
    Active = false;
    return;
  }

  // If not interpolating and volume is close to 1.0, don't do anything
  if (Interpolator.Finished() && AudioConstants::IsWithinLimit(mCurrentVolume, 1.0f, 0.01f))
    return;

  // If we are not interpolating, apply the same volume to all samples
  if (Interpolator.Finished())
  {
    for (unsigned i = 0; i < bufferSize; ++i)
      sampleBuffer[i] *= mCurrentVolume;
  }
  // If we are interpolating, get the volume for each frame and apply to samples
  else
  {
    for (unsigned sample = 0; sample < bufferSize; sample += numberOfChannels)
    {
      mCurrentVolume = Interpolator.NextValue();

      for (unsigned channel = 0; channel < numberOfChannels; ++channel)
        sampleBuffer[sample + channel] *= mCurrentVolume;
    }
  }
}

//************************************************************************************************
void InstanceVolumeModifier::Reset(const float startVolume, const float endVolume,
  const float time, const float lifetime)
{
  Reset(startVolume, endVolume, (unsigned)(time * AudioConstants::cSystemSampleRate),
    (unsigned)(lifetime * AudioConstants::cSystemSampleRate));
}

//************************************************************************************************
void InstanceVolumeModifier::Reset(const float startVolume, const float endVolume,
  const unsigned frames, const unsigned lifetimeFrames)
{
  mCurrentVolume = startVolume;
  Interpolator.SetValues(startVolume, endVolume, frames);
  mLifetimeFrames = lifetimeFrames;
  mLifetimeFrameCounter = 0;
  Active = true;
}

//************************************************************************************************
float InstanceVolumeModifier::GetCurrentVolume()
{
  return mCurrentVolume;
}

//************************************************************************************************
float InstanceVolumeModifier::GetFutureVolume(unsigned frames)
{
  if (Interpolator.Finished())
    return mCurrentVolume;
  else
    return Interpolator.ValueAtIndex(Interpolator.GetCurrentFrame() + frames);
}


} // namespace Zero
