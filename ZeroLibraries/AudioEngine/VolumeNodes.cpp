///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //------------------------------------------------------------------------------------ Volume Node

  //************************************************************************************************
  VolumeNode::VolumeNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
    ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, false, isThreaded),
    Volume(1.0f),
    Interpolate(nullptr)
  {
    if (!Threaded)
      SetSiblingNodes(new VolumeNode(status, name, ID, nullptr, true), status);
    else
      Interpolate = gAudioSystem->GetInterpolatorThreaded();
  }

  //************************************************************************************************
  VolumeNode::~VolumeNode()
  {
    if (Interpolate)
      gAudioSystem->ReleaseInterpolatorThreaded(Interpolate);
  }

  //************************************************************************************************
  float VolumeNode::GetVolume()
  {
    return Volume;
  }

  //************************************************************************************************
  bool VolumeNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

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
    for (unsigned i = 0; i < bufferSize; i += numberOfChannels)
    {
      // Check if the volume is being interpolated
      if (CurrentData.Interpolating)
      {
        Volume = Interpolate->ValueAtIndex(CurrentData.Index++);

        CurrentData.Interpolating = !Interpolate->Finished(GetSiblingNode());

        if (!CurrentData.Interpolating && firstRequest && GetSiblingNode())
          gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&VolumeNode::Volume,
          (VolumeNode*)GetSiblingNode(), Volume));
      }

      // Apply the volume multiplier to all samples
      for (unsigned j = 0; j < numberOfChannels; ++j)
        (*outputBuffer)[i + j] = InputSamples[i + j] * Volume;
    }

    AddBypass(outputBuffer);

    if (firstRequest && CurrentData.Interpolating)
      gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&VolumeNode::Volume, 
        (VolumeNode*)GetSiblingNode(), Volume));

    return true;
  }

  //************************************************************************************************
  void VolumeNode::SetVolume(const float newVolume, float timeToInterpolate)
  {
    if (!Threaded)
    {
      if (timeToInterpolate == 0 || !HasOutputs() || IsWithinLimit(newVolume, Volume, 0.01f))
        Volume = newVolume;

      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&VolumeNode::SetVolume, (VolumeNode*)GetSiblingNode(),
          newVolume, timeToInterpolate));
    }
    else
    {
      // If the new volume is very close to the current one, or we have no outputs, just set it directly
      if ((timeToInterpolate == 0 && !HasOutputs()) || IsWithinLimit(newVolume, Volume, 0.01f))
      {
        CurrentData.Interpolating = false;
        Volume = newVolume;
      }
      else
      {
        CurrentData.Interpolating = true;
        CurrentData.Index = 0;

        if (timeToInterpolate < 0.02f)
          timeToInterpolate = 0.02f;

        Interpolate->SetValues(Volume, newVolume, (unsigned)(timeToInterpolate
          * gAudioSystem->SystemSampleRate));
      }
    }
  }

  //----------------------------------------------------------------------------------- Panning Node

  //************************************************************************************************
  PanningNode::PanningNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
    ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, false, isThreaded),
    SumToMono(false),
    LeftVolume(1.0f),
    RightVolume(1.0f),
    Active(false),
    LeftInterpolator(nullptr),
    RightInterpolator(nullptr)
  {
    if (!isThreaded)
      SetSiblingNodes(new PanningNode(status, name, ID, nullptr, true), status);
    else
    {
      LeftInterpolator = gAudioSystem->GetInterpolatorThreaded();
      LeftInterpolator->SetValues(1.0f, 1.0f, (unsigned)0);
      RightInterpolator = gAudioSystem->GetInterpolatorThreaded();
      RightInterpolator->SetValues(1.0f, 1.0f, (unsigned)0);
    }
  }

  //************************************************************************************************
  PanningNode::~PanningNode()
  {
    if (LeftInterpolator)
    {
      gAudioSystem->ReleaseInterpolatorThreaded(LeftInterpolator);
      gAudioSystem->ReleaseInterpolatorThreaded(RightInterpolator);
    }
  }

  //************************************************************************************************
  bool PanningNode::GetSumToMono()
  {
    return SumToMono;
  }

  //************************************************************************************************
  void PanningNode::SetSumToMono(const bool isMono)
  {
    SumToMono = isMono;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&PanningNode::SumToMono,
        (PanningNode*)GetSiblingNode(), isMono));
  }

  //************************************************************************************************
  float PanningNode::GetLeftVolume()
  {
    return LeftVolume;
  }

  //************************************************************************************************
  void PanningNode::SetLeftVolume(const float volume, float time)
  {
    if (!Threaded)
    {
      LeftVolume = volume;

      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&PanningNode::SetLeftVolume,
        (PanningNode*)GetSiblingNode(), volume, time));
    }
    else
    {
      Active = true;

      // If the new volume is very close to the current one, or we have no outputs, just set it directly
      if ((time == 0 && !HasOutputs()) || IsWithinLimit(volume, LeftVolume, 0.01f))
      {
        CurrentData.Interpolating = false;
        LeftVolume = volume;
      }
      else
      {
        CurrentData.Interpolating = true;

        if (time < 0.02f)
          time = 0.02f;

        LeftInterpolator->SetValues(LeftVolume, volume, (unsigned)(time
          * gAudioSystem->SystemSampleRate));
      }
    }
  }

  //************************************************************************************************
  float PanningNode::GetRightVolume()
  {
    return RightVolume;
  }

  //************************************************************************************************
  void PanningNode::SetRightVolume(const float volume, float time)
  {
    if (!Threaded)
    {
      RightVolume = volume;

      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&PanningNode::SetRightVolume,
        (PanningNode*)GetSiblingNode(), volume, time));
    }
    else
    {
      Active = true;

      // If the new volume is very close to the current one, or we have no outputs, just set it directly
      if ((time == 0 && !HasOutputs()) || IsWithinLimit(volume, RightVolume, 0.01f))
      {
        CurrentData.Interpolating = false;
        RightVolume = volume;
      }
      else
      {
        CurrentData.Interpolating = true;

        if (time < 0.02f)
          time = 0.02f;

        RightInterpolator->SetValues(RightVolume, volume, (unsigned)(time
          * gAudioSystem->SystemSampleRate));
      }
    }
  }

  //************************************************************************************************
  bool PanningNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    // If first request this mix, set the previous mix data
    if (firstRequest)
      PreviousData = CurrentData;
    // If not, reset the current data to the previous
    else
      CurrentData = PreviousData;

    unsigned bufferSize = outputBuffer->Size();

    if (Active)
    {
      unsigned channelsToGet;
      if (SumToMono)
        channelsToGet = 1;
      else
        channelsToGet = 2;

      // Get input and return if there is no data
      if (!AccumulateInputSamples(bufferSize, channelsToGet, listener))
        return false;

      unsigned totalFrames = bufferSize / numberOfChannels;
      // Step through each frame of audio data
      for (unsigned currentFrame = 0; currentFrame < totalFrames; ++currentFrame)
      {
        float leftValue, rightValue;
        // If requested 1 channel of audio, copy this to left and right channels
        if (SumToMono)
          leftValue = rightValue = InputSamples[currentFrame] * 0.5f;
        else
        {
          leftValue = InputSamples[currentFrame * 2];
          rightValue = InputSamples[(currentFrame * 2) + 1];
        }

        // Check if we are interpolating the volume
        if (CurrentData.Interpolating)
        {
          LeftVolume = LeftInterpolator->NextValue();
          RightVolume = RightInterpolator->NextValue();

          // If we finished interpolating the volume, send a notification
          if (LeftInterpolator->Finished() && RightInterpolator->Finished())
          {
            CurrentData.Interpolating = false;
            if (firstRequest && GetSiblingNode())
            {
              gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
                GetSiblingNode(), Notify_InterpolationDone, (void*)nullptr));

              // Set the final volume values on the non-threaded object
              gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&PanningNode::LeftVolume,
                (PanningNode*)GetSiblingNode(), LeftVolume));
              gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&PanningNode::RightVolume,
                (PanningNode*)GetSiblingNode(), RightVolume));
            }
          }
        }

        // Set the volume of both channels
        leftValue *= LeftVolume;
        rightValue *= RightVolume;

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
          frame.TranslateChannels(numberOfChannels);
          memcpy(outputBuffer->Data() + (currentFrame * numberOfChannels), frame.Samples,
            sizeof(float) * numberOfChannels);
        }
      }

      AddBypass(outputBuffer);

      // Check for both volumes being at or near 1.0, and if true mark as not active
      if (!CurrentData.Interpolating && IsWithinLimit(1.0f - LeftVolume, 0.0f, 0.01f)
        && IsWithinLimit(1.0f - RightVolume, 0.0f, 0.01f))
        Active = false;

      // Set the current volume values on the non-threaded object
      if (CurrentData.Interpolating && GetSiblingNode())
      {
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&PanningNode::LeftVolume,
          (PanningNode*)GetSiblingNode(), LeftVolume));
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&PanningNode::RightVolume,
          (PanningNode*)GetSiblingNode(), RightVolume));
      }
    }
    else
    {
      // Get input and return if there is no data
      if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
        return false;

      InputSamples.Swap(*outputBuffer);
    }

    return true;
  }
}
