///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //-------------------------------------------------------------------------------- Pitch Node Data

  //************************************************************************************************
  PitchNode::Data::Data() :
    FramesProcessed(0),
    Interpolating(false),
    PitchIndex(0),
    BufferFraction(0)
  {
    memset(LastSamples, 0, sizeof(float) * MaxChannels);
  }

  //************************************************************************************************
  PitchNode::Data& PitchNode::Data::operator=(const PitchNode::Data& other)
  {
    FramesProcessed = other.FramesProcessed;
    Interpolating = other.Interpolating;
    memcpy(LastSamples, other.LastSamples, sizeof(float) * MaxChannels);
    BufferFraction = other.BufferFraction;
    PitchIndex = other.PitchIndex;
    return *this;
  }

  //------------------------------------------------------------------------------------- Pitch Node

  //************************************************************************************************
  PitchNode::PitchNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
    ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, false, isThreaded),
    PitchCents(0),
    PitchFactor(1.0f),
    FramesToInterpolate(0),
    TimeToInterpolate(0.0f),
    Interpolate(nullptr)
  {
    if (!Threaded)
      SetSiblingNodes(new PitchNode(status, name, ID, nullptr, true), status);
    else
      Interpolate = gAudioSystem->GetInterpolatorThreaded();
  }

  //************************************************************************************************
  PitchNode::~PitchNode()
  {
    if (Interpolate)
      gAudioSystem->ReleaseInterpolatorThreaded(Interpolate);
  }

  //************************************************************************************************
  void PitchNode::SetPitch(const int pitchCents, const float timeToInterpolate)
  {
    if (!Threaded)
    {
      PitchCents = pitchCents;
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&PitchNode::SetPitch, (PitchNode*)GetSiblingNode(),
          pitchCents, timeToInterpolate));
    }
    else
    {
      float newFactor = Math::Pow(2.0f, pitchCents / 1200.0f);
      if (timeToInterpolate == 0.0f)
      {
        FramesToInterpolate = 0;
        CurrentData.Interpolating = false;
        PitchFactor = newFactor;
      }
      else
      {
        // Check if currently interpolating pitch
        if (CurrentData.Interpolating)
          PitchFactor = Interpolate->ValueAtIndex(CurrentData.FramesProcessed);
        else
          CurrentData.Interpolating = true;

        FramesToInterpolate = (unsigned)(timeToInterpolate * gAudioSystem->SystemSampleRate);
        Interpolate->SetValues(PitchFactor, newFactor, (unsigned)FramesToInterpolate);
        TimeToInterpolate = timeToInterpolate;
        CurrentData.FramesProcessed = 0;
        PitchFactor = newFactor;
      }
    }
  }

  //************************************************************************************************
  int PitchNode::GetPitch()
  {
    return PitchCents;
  }

  //************************************************************************************************
  bool PitchNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
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

    // If no inputs, return
    if (!HasInputs())
      return false;

    // Save variables for the output buffer size
    int outputBufferSize = outputBuffer->Size();
    int outputBufferFrames = outputBufferSize / (int)numberOfChannels;

    // Check if we have just finished interpolating pitch
    if (CurrentData.Interpolating && CurrentData.FramesProcessed >= FramesToInterpolate)
    {
      CurrentData.Interpolating = false;

      // Notify the external data that the interpolation is finished
      if (firstRequest && GetSiblingNode())
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
          GetSiblingNode(), Notify_InterpolationDone, (void*)nullptr));
    }

    double framesToGet(0.0f);

    // Are we interpolating?
    if (CurrentData.Interpolating)
    {
      // Get the starting and ending pitch factor for interpolation
      float startingPitchFactor = Interpolate->ValueAtIndex(CurrentData.FramesProcessed);
      float endingPitchFactor = Interpolate->GetEndValue();

      // How many frames are left in the interpolation
      int interpolationFrames = FramesToInterpolate - CurrentData.FramesProcessed;
      double otherFrames(0);

      // Check if the last frame is past the end of the buffer
      if (interpolationFrames > outputBufferFrames)
      {
        // Set the number of frames to the buffer size
        interpolationFrames = outputBufferFrames;
        // Set the ending pitch factor to the end of the buffer
        endingPitchFactor = Interpolate->ValueAtIndex(CurrentData.FramesProcessed + outputBufferFrames);
      }
      // Otherwise, how many frames in the buffer after the end of the interpolation
      else
        otherFrames = (outputBufferFrames - interpolationFrames) * PitchFactor;

      // Buffer size uses the average interpolate pitch factor
      framesToGet = (startingPitchFactor + endingPitchFactor) / 2.0f * interpolationFrames;
      // Add any additional frames beyond the end of interpolation
      framesToGet += otherFrames;
    }
    // Not interpolating, buffer size is a simple multiple
    else
    {
      framesToGet = PitchFactor * outputBufferFrames;
    }

    // Account for the previous fractional part of the buffer size
    framesToGet += PreviousData.BufferFraction;
    // Get the integer portion for the actual buffer size
    int inputBufferSize = (int)framesToGet;
    // Save the fractional part of the value
    CurrentData.BufferFraction = framesToGet - inputBufferSize;

    // Store the number of frames in the source buffer
    unsigned inputBufferFrames = inputBufferSize;

    // Adjust buffer size to account for channels
    inputBufferSize *= numberOfChannels;

    bool processPitch(false);
    if (inputBufferSize != outputBufferSize)
      processPitch = true;

    // Get data from all input connections
    bool isThereInput = AccumulateInputSamples(inputBufferSize, numberOfChannels, listener);

    // If there is no input data, just return
    if (!isThereInput)
    {
      memset(CurrentData.LastSamples, 0, sizeof(float) * numberOfChannels);
      return false;
    }

    // Copy over the last frame of samples
    memcpy(CurrentData.LastSamples, InputSamples.Data() + (inputBufferSize - numberOfChannels),
      sizeof(float) * numberOfChannels);

    // If not shifting the pitch, don't need to do any more
    if (!processPitch)
    {
      InputSamples.Swap(*outputBuffer);
      if (CurrentData.Interpolating)
        CurrentData.FramesProcessed += outputBufferFrames;
      return true;
    }

    // Get the pitch factor, accounting for interpolation
    float currentPitchFactor;
    if (CurrentData.Interpolating)
      currentPitchFactor = Interpolate->ValueAtIndex(CurrentData.FramesProcessed);
    else
      currentPitchFactor = PitchFactor;

    // Set the pitch index to the previous value
    CurrentData.PitchIndex = PreviousData.PitchIndex;
    // Get the integer portion for the frame index
    int frameIndex = (int)CurrentData.PitchIndex;
    // The previous frame index starts at -1 
    int previousFrameIndex(-1);

    float firstSample, secondSample;

    // Step through all other frames in the output buffer
    for (unsigned outputFrameSample = 0; outputFrameSample + numberOfChannels <= (unsigned)outputBufferSize;
      outputFrameSample += numberOfChannels)
    {
      // Store the sample index of the current source frame
      int sourceFrameStart = frameIndex * numberOfChannels;
      // Store the sample index of the previous frame
      int previousFrameStart = previousFrameIndex * numberOfChannels;

      // Check if this frame would go past the end of the buffer
      if (sourceFrameStart + (int)numberOfChannels > (int)inputBufferSize)
      {
        // Copy the previous frame into the output buffer
        memcpy(outputBuffer->Data() + outputFrameSample, InputSamples.Data() + previousFrameStart,
          sizeof(float) * numberOfChannels);
        continue;
      }

      // Go through all samples in this frame
      for (unsigned channel = 0; channel < numberOfChannels; ++channel)
      {
        // The first sample is from the previous frame
        if (previousFrameStart < 0)
          firstSample = PreviousData.LastSamples[channel];
        else
          firstSample = InputSamples[previousFrameStart + channel];

        // The second is from this frame
        if (sourceFrameStart < 0)
          secondSample = PreviousData.LastSamples[channel];
        else
          secondSample = InputSamples[sourceFrameStart + channel];

        (*outputBuffer)[outputFrameSample + channel] = firstSample + ((secondSample
          - firstSample) * ((float)CurrentData.PitchIndex - frameIndex));

      }

      // If currently interpolating, get updated pitch factor
      if (CurrentData.FramesProcessed <= FramesToInterpolate)
        currentPitchFactor = Interpolate->ValueAtIndex(CurrentData.FramesProcessed);

      // Advance the pitch index
      CurrentData.PitchIndex += currentPitchFactor;
      // Get the integer portion for the frame index
      frameIndex = (int)CurrentData.PitchIndex;
      // Save the previous frame index value
      previousFrameIndex = frameIndex - 1;

      if (CurrentData.Interpolating)
        ++CurrentData.FramesProcessed;
    }

    // Keep only the fractional portion of the pitch index
    CurrentData.PitchIndex -= framesToGet;
    // Make sure the integer portion isn't negative
    if (CurrentData.PitchIndex < 0)
      CurrentData.PitchIndex -= (int)CurrentData.PitchIndex;

    return true;
  }

}
