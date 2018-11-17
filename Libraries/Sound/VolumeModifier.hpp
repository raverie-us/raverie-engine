///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

//----------------------------------------------------------------------- Threaded Volume Modifier

// Object to modify the volume of sound instances. Multiplies with the instance volume. 
class InstanceVolumeModifier
{
public:
  InstanceVolumeModifier();

  // Applies this modification to a buffer of samples.
  void ApplyVolume(float *sampleBuffer, const unsigned bufferSize, const unsigned channels);
  // Resets the modifier with new volume and time data. 
  void Reset(const float startVolume, const float endVolume, const float changeTime,
    const float lifetime);
  void Reset(const float startVolume, const float endVolume, const unsigned changeFrames,
    const unsigned lifetimeFrames);
  // Gets the current volume 
  float GetCurrentVolume();
  // Gets the volume at a specified number of frames ahead
  float GetFutureVolume(unsigned frames);
  // Keeps track of whether this modifier is currently active
  bool Active;

private:
  // Current volume modification
  float mCurrentVolume;
  // Used to interpolate between start and end volumes. 
  InterpolatingObject Interpolator;
  // Number of frames that this modifier should stay active. If zero, will be active indefinitely.
  unsigned mLifetimeFrames;
  // Keeps track of the number of frames this modifier has been active.
  unsigned mLifetimeFrameCounter;
};

} // namespace Zero
