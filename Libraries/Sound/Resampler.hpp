///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

class Resampler
{
public:
  Resampler();

  void SetFactor(double factor);
  unsigned GetOutputFrameCount(unsigned inputFrames);
  void SetInputBuffer(const float* inputSamples, unsigned frameCount, unsigned channels);
  bool GetNextFrame(float* output);

private:
  float PreviousFrame[AudioConstants::cMaxChannels];
  double ResampleFactor;
  double ResampleFrameIndex;
  double BufferFraction;
  const float* InputSamples;
  unsigned InputFrames;
  unsigned InputChannels;
};

} // namespace Zero
