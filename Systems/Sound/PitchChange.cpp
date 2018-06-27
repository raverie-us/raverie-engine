///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

using namespace AudioConstants;

//--------------------------------------------------------------------------- Pitch Change Handler

//************************************************************************************************
PitchChangeHandler::PitchChangeHandler() :
  mPitchCents(0),
  mPitchFactor(1.0f),
  mFramesToInterpolate(0),
  mInputFrameCount(0),
  mInputSampleCount(0),
  mChannels(0)
{
  ResetLastSamples();
}

//************************************************************************************************
void PitchChangeHandler::CalculateBufferSize(unsigned outputSampleCount, unsigned numberOfChannels)
{
  PreviousData = CurrentData;

  mChannels = numberOfChannels;
  unsigned outputFrames = outputSampleCount / numberOfChannels;
  double framesToGet(0.0);

  if (!CurrentData.mInterpolating)
  {
    framesToGet = mPitchFactor * outputFrames;
  }
  else
  {
    // Starting and ending pitch factors for interpolation
    float startingPitchFactor = PitchInterpolator.ValueAtIndex(CurrentData.mInterpolationFramesProcessed);
    float endingPitchFactor = PitchInterpolator.GetEndValue();

    // How many frames are left in the interpolation
    unsigned interpolationFrames = mFramesToInterpolate - CurrentData.mInterpolationFramesProcessed;

    double framesWithoutInterpolation(0.0);

    // Check if the last frame is past the end of the buffer
    if (interpolationFrames > outputFrames)
    {
      // Set the interpolation frames to the buffer size
      interpolationFrames = outputFrames;
      // Set the ending pitch factor using the buffer size
      endingPitchFactor = PitchInterpolator.ValueAtIndex(CurrentData.mInterpolationFramesProcessed
        + outputFrames);
    }
    // Otherwise, find out how many frames won't be interpolated
    else
      framesWithoutInterpolation = (outputFrames - interpolationFrames) * endingPitchFactor;

    // Buffer size uses the average pitch factor
    framesToGet = (startingPitchFactor + endingPitchFactor) / 2.0f * interpolationFrames;
    // Add any additional frames beyond the end of interpolation
    framesToGet += framesWithoutInterpolation;

    // Set the pitch factor to be the factor at the end of the buffer
    mPitchFactor = endingPitchFactor;
  }

  // Account for the previous fractional part of the buffer size
  framesToGet += CurrentData.mBufferSizeFraction;
  // Get the integer portion for the actual number of frames to get
  mInputFrameCount = (int)framesToGet;
  // Also save the number of samples
  mInputSampleCount = mInputFrameCount * mChannels;
  // Save the fractional part of the frames value
  CurrentData.mBufferSizeFraction = framesToGet - mInputFrameCount;
}

//************************************************************************************************
void PitchChangeHandler::ProcessBuffer(BufferType* inputBuffer, BufferType* outputBuffer)
{
  unsigned outputBufferSize = outputBuffer->Size();

  // Not shifting pitch, don't need to do any more
  if (outputBufferSize == mInputSampleCount)
  {
    inputBuffer->Swap(*outputBuffer);
    if (CurrentData.mInterpolating)
    {
      CurrentData.mInterpolationFramesProcessed += mInputFrameCount;
      mPitchFactor = PitchInterpolator.ValueAtIndex(CurrentData.mInterpolationFramesProcessed);
      if (CurrentData.mInterpolationFramesProcessed >= mFramesToInterpolate)
        CurrentData.mInterpolating = false;
    }
    return;
  }

  // Get the integer portion for the frame index
  int frameIndex = (int)CurrentData.mPitchFrameIndex;
  // Previous frame index starts at -1
  int previousFrameIndex(-1);

  float firstSample, secondSample;

  // Step through all frames in the output buffer
  BufferRange outputRange = outputBuffer->All();
  for (unsigned outputFrameIndex = 0; outputFrameIndex + mChannels <= outputBufferSize;
    outputFrameIndex += mChannels)
  {
    // Sample index of current source frame
    int sourceFrameStart = frameIndex * mChannels;
    // Sample index of previous frame
    int previousFrameStart = previousFrameIndex * mChannels;

    // Check if this frame would go past the end of the buffer
    if (sourceFrameStart + mChannels > mInputSampleCount)
    {
      // Copy the previous frame into the output buffer
      memcpy(outputBuffer->Data() + outputFrameIndex, inputBuffer->Data() + previousFrameStart,
        sizeof(float) * mChannels);
      continue;
    }

    // Go through all samples in this frame
    for (unsigned channel = 0; channel < mChannels; ++channel, outputRange.PopFront())
    {
      // First sample is from the previous frame
      if (previousFrameStart < 0)
        firstSample = CurrentData.LastSamples[channel];
      else
        firstSample = (*inputBuffer)[previousFrameStart + channel];

      // Second sample is from this frame
      if (sourceFrameStart < 0)
        secondSample = CurrentData.LastSamples[channel];
      else
        secondSample = (*inputBuffer)[sourceFrameStart + channel];

      // Interpolate between the two samples for the output sample
      outputRange.Front() = firstSample + ((secondSample - firstSample)
        * ((float)CurrentData.mPitchFrameIndex - frameIndex));
    }

    // If currently interpolating, get updated pitch factor
    if (CurrentData.mInterpolating)
    {
      ++CurrentData.mInterpolationFramesProcessed;
      mPitchFactor = PitchInterpolator.ValueAtIndex(CurrentData.mInterpolationFramesProcessed);

      // Check if the interpolation is finished
      if (CurrentData.mInterpolationFramesProcessed >= mFramesToInterpolate)
        CurrentData.mInterpolating = false;
    }

    // Advance the pitch index
    CurrentData.mPitchFrameIndex += (double)mPitchFactor;
    // Update the frame indexes
    frameIndex = (int)CurrentData.mPitchFrameIndex;
    previousFrameIndex = frameIndex - 1;
  }

  // Keep only the fractional portion of the pitch index
  CurrentData.mPitchFrameIndex -= mInputFrameCount;
  // Make sure the integer portion isn't negative
  if (CurrentData.mPitchFrameIndex < 0)
    CurrentData.mPitchFrameIndex -= (int)CurrentData.mPitchFrameIndex;

  // Save last frame of samples
  memcpy(CurrentData.LastSamples, inputBuffer->Data() + (mInputSampleCount - mChannels),
    sizeof(float) * mChannels);
}

//************************************************************************************************
float PitchChangeHandler::GetPitchFactor()
{
  return mPitchFactor;
}

//************************************************************************************************
void PitchChangeHandler::SetPitchFactor(float factor, float timeToInterpolate)
{
  // If the time is zero, set the pitch directly
  if (timeToInterpolate == 0.0f)
  {
    mFramesToInterpolate = 0;
    CurrentData.mInterpolating = false;
    mPitchFactor = factor;
  }
  // Otherwise start interpolating
  else
  {
    // If we are already interpolating, make sure the pitch factor is set correctly
    if (CurrentData.mInterpolating)
      mPitchFactor = PitchInterpolator.ValueAtIndex(CurrentData.mInterpolationFramesProcessed);
    else
      CurrentData.mInterpolating = true;

    mFramesToInterpolate = (unsigned)(timeToInterpolate * cSystemSampleRate);
    PitchInterpolator.SetValues(mPitchFactor, factor, mFramesToInterpolate);
    CurrentData.mInterpolationFramesProcessed = 0;
  }
}

//************************************************************************************************
void PitchChangeHandler::ResetLastSamples()
{
  memset(CurrentData.LastSamples, 0, sizeof(float) * cMaxChannels);
}

//************************************************************************************************
void PitchChangeHandler::ResetToStartOfMix()
{
  CurrentData = PreviousData;
}

//************************************************************************************************
bool PitchChangeHandler::Interpolating()
{
  return CurrentData.mInterpolating;
}

//************************************************************************************************
PitchChangeHandler::Data::Data() :
  mInterpolationFramesProcessed(0),
  mInterpolating(false),
  mPitchFrameIndex(0.0),
  mBufferSizeFraction(0.0)
{
  memset(LastSamples, 0, sizeof(float) * cMaxChannels);
}

//************************************************************************************************
PitchChangeHandler::Data& PitchChangeHandler::Data::operator=(const Data& other)
{
  mInterpolationFramesProcessed = other.mInterpolationFramesProcessed;
  mInterpolating = other.mInterpolating;
  mPitchFrameIndex = other.mPitchFrameIndex;
  mBufferSizeFraction = other.mBufferSizeFraction;
  memcpy(LastSamples, other.LastSamples, sizeof(float) * cMaxChannels);

  return *this;
}

} // namespace Zero
