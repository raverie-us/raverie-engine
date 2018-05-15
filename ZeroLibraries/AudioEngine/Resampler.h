///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef Resampler_H
#define Resampler_H

namespace Audio
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
    float PreviousFrame[cMaxChannels];
    double ResampleFactor;
    double ResampleFrameIndex;
    double BufferFraction;
    const float* InputSamples;
    unsigned InputFrames;
    unsigned InputChannels;
  };
}

#endif
