///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //----------------------------------------------------------------------- Threaded Volume Modifier

  //************************************************************************************************
  ThreadedVolumeModifier::ThreadedVolumeModifier() :
    VolumeStart(0),
    Volume(0),
    FramesProcessed(0),
    ReleaseFrames(0),
    Active(false),
    TotalFrames(0),
    Release(false),
    Attack(true),
    DelayFrames(0),
    Delay(false)
  {

  }

  //************************************************************************************************
  void ThreadedVolumeModifier::ApplyModification(float *sampleBuffer, const unsigned bufferSize)
  {
    if (!Active)
      return;

    for (unsigned i = 0; i < bufferSize; i += gAudioSystem->SystemChannelsThreaded)
    {
      // If delaying, don't do anything
      if (Delay)
      {
        // Done with delay, reset 
        if (FramesProcessed == DelayFrames)
        {
          FramesProcessed = 0;
          Delay = false;
        }
        else
        {
          ++FramesProcessed;
          continue;
        }
      }

      float volumeValue;
      // In attack phase
      if (Attack)
      {
        volumeValue = Interpolator.NextValue();
        
        // If attack is finished, go to sustain
        if (Interpolator.Finished())
          Attack = false;
      }
      // In release phase
      else if (Release)
      {
        volumeValue = Interpolator.NextValue();

        // If release is finished, we're done
        if (Interpolator.Finished())
        {
          Active = false;
          return;
        }
      }
      // In sustain phase
      else
      {
        volumeValue = Volume;

        // If the modification is timed and we've reached the end, go to release phase
        if (TotalFrames > 0 && FramesProcessed >= TotalFrames - ReleaseFrames)
        {
          Release = true;
          Interpolator.SetValues(Volume, VolumeStart, ReleaseFrames);
        }
      }

      // Modify volume of this frame of samples
      for (unsigned j = 0; j < gAudioSystem->SystemChannelsThreaded; ++j)
        sampleBuffer[i + j] *= volumeValue;

      // Keep track of how many frames total have been modified
      ++FramesProcessed;
    }
  }

  //************************************************************************************************
  void ThreadedVolumeModifier::Reset(const float startVolume, const float endVolume, const float attackTime, 
    const float releaseTime, const float totalTime, const float delayTime)
  {
    VolumeStart = startVolume;
    Volume = endVolume;
    unsigned attackFrames = (unsigned)(attackTime * AudioSystemInternal::SystemSampleRate);
    ReleaseFrames = (unsigned)(releaseTime * AudioSystemInternal::SystemSampleRate);
    TotalFrames = (unsigned)(totalTime * AudioSystemInternal::SystemSampleRate);
    DelayFrames = (unsigned)(delayTime * AudioSystemInternal::SystemSampleRate);
    if (DelayFrames > 0)
      Delay = true;
    else
      Delay = false;
    Release = false;
    Attack = true;
    Active = true;

    if (attackFrames == 0)
      attackFrames = 1000;

    Interpolator.SetValues(VolumeStart, Volume, attackFrames);
  }

  //************************************************************************************************
  float ThreadedVolumeModifier::GetCurrentVolume()
  {
    return Interpolator.GetCurrentValue();
  }

  //************************************************************************************************
  float ThreadedVolumeModifier::GetFutureVolume(unsigned frames)
  {
    return Interpolator.ValueAtIndex(FramesProcessed + frames);
  }

}