///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef MODIFIERS
#define MODIFIERS

namespace Audio
{
  //----------------------------------------------------------------------- Threaded Volume Modifier

  // Object to modify the volume of sound instances. Multiplies with the instance volume. 
  class ThreadedVolumeModifier
  {
  public:
    ThreadedVolumeModifier();
    ~ThreadedVolumeModifier();

    // Applies this modification to a buffer of samples.
    void ApplyModification(float *sampleBuffer, const unsigned bufferSize);
    // Resets the modifier with new volume and time data. Keeps the same tag or instance.
    void Reset(const float startVolume, const float endVolume, const float attackTime, 
      const float releaseTime, const float totalTime, const float delayTime);
    // Gets the current volume from the last Update call.
    float GetCurrentVolume();
    // Gets the volume at a specified number of frames ahead
    float GetFutureVolume(unsigned frames);
    // Keeps track of whether this modifier is currently active
    bool Active;

  private:
    // The starting volume. 
    float VolumeStart;
    // The volume of the modification. 
    float Volume;
    // If true, currently in release phase. 
    bool Release;
    // If true, currently in attack phase. 
    bool Attack;
    // Number of frames to modify for. If 0, will keep going indefinitely. 
    unsigned TotalFrames;
    // Number of frames that have been processed so far. 
    unsigned FramesProcessed;
    // Number of frames to interpolate from final back to start volume. 
    unsigned ReleaseFrames;
    // Number of frames to wait before applying modification. 
    unsigned DelayFrames;
    // If true, currently delaying.
    bool Delay;
    // Used to interpolate between start and end volumes. 
    InterpolatingObject* Interpolate;
  };
}

#endif