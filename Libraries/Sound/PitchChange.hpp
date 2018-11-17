///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{
//----------------------------------------------------------------------------- Pitch Change Handler

class PitchChangeHandler
{
public:
  PitchChangeHandler();

  // Must be called before ProcessBuffer
  void CalculateBufferSize(unsigned outputSampleCount, unsigned numberOfChannels);
  // Interpolates the audio samples in inputSamples into outputBuffer
  void ProcessBuffer(BufferType* inputBuffer, BufferType* outputBuffer);
  // Returns the current pitch factor
  float GetPitchFactor();
  // Sets the pitch factor over the specified number of seconds
  void SetPitchFactor(float factor, float timeToInterpolate);
  // Returns how many frames should be in the input
  unsigned GetInputFrameCount() { return mInputFrameCount; }
  // Returns how many samples should be in the input
  unsigned GetInputSampleCount() { return mInputSampleCount; }
  // Resets the LastSamples buffer to zero
  void ResetLastSamples();
  // Resets back to the start of the last mix (for evaluating multiple times per mix)
  void ResetToStartOfMix();
  // Returns true if the pitch is currently being interpolated
  bool Interpolating();

private:
  int mPitchCents;
  float mPitchFactor;
  unsigned mInputFrameCount;
  unsigned mInputSampleCount;
  unsigned mChannels;
  InterpolatingObject PitchInterpolator;
  unsigned mFramesToInterpolate;

  class Data
  {
  public:
    Data();
    Data& operator=(const Data& other);

    unsigned mInterpolationFramesProcessed;
    bool mInterpolating;
    float LastSamples[AudioConstants::cMaxChannels];
    double mPitchFrameIndex;
    double mBufferSizeFraction;
  };

  Data CurrentData;
  Data PreviousData;
};

} // namespace Zero
