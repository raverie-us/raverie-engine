///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

//**************************************************************************************************
Resampler::Resampler() :
  ResampleFactor(0),
  ResampleFrameIndex(0),
  BufferFraction(0),
  InputSamples(nullptr),
  InputFrames(0),
  InputChannels(0)
{
  memset(PreviousFrame, 0, sizeof(float) * AudioConstants::cMaxChannels);
}

//**************************************************************************************************
void Resampler::SetFactor(double factor)
{
  ResampleFactor = factor;
}

//**************************************************************************************************
unsigned Resampler::GetOutputFrameCount(unsigned inputFrames)
{
  // Get initial frame count
  double frames = ResampleFactor * inputFrames;
  // Add the leftover fraction from the last buffer
  frames += BufferFraction;
  // Save the current fraction
  BufferFraction = frames - (int)frames;
  // Return the integer buffer size
  return (unsigned)frames;
}

//**************************************************************************************************
void Resampler::SetInputBuffer(const float* inputSamples, unsigned frameCount, unsigned channels)
{
  InputSamples = inputSamples;
  InputFrames = frameCount;
  InputChannels = channels;
  ResampleFrameIndex = ResampleFrameIndex - (int)ResampleFrameIndex;
}

//**************************************************************************************************
bool Resampler::GetNextFrame(float* output)
{
  // Save the integer frame index
  unsigned frameIndex = (unsigned)ResampleFrameIndex;

  // Check if this would be past the end of the buffer
  if (frameIndex >= InputFrames)
  {
    memcpy(output, PreviousFrame, sizeof(float) * InputChannels);
    return false;
  }

  // Translate to the sample index
  unsigned sampleIndex = frameIndex * InputChannels;

  // Get the pointer to the first frame. If the index is at 0, use the PreviousFrame values.
  const float* firstFrame(PreviousFrame);
  if (ResampleFrameIndex > 1.0)
    firstFrame = InputSamples + (sampleIndex - InputChannels);

  // Get the pointer to the second frame
  const float* secondFrame(InputSamples + sampleIndex);

  // Interpolate between the two frames for each channel
  for (unsigned i = 0; i < InputChannels; ++i)
    output[i] = firstFrame[i] + ((secondFrame[i] - firstFrame[i])
      * (float)(ResampleFrameIndex - frameIndex));

  // Advance the frame index
  ResampleFrameIndex += ResampleFactor;

  // If we are now past the end of the buffer, save the PreviousFrame values
  if ((unsigned)ResampleFrameIndex >= InputFrames)
  {
    // Get the last frame of samples in the buffer
    memcpy(PreviousFrame, InputSamples + ((InputFrames - 1) * InputChannels),
      sizeof(float) * InputChannels);
    return false;
  }
  else
    return true;
}

} // namespace Zero
