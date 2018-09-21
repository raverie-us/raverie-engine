///////////////////////////////////////////////////////////////////////////////
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

using namespace AudioConstants;

//---------------------------------------------------------------------------------- BiQuad Filter

//************************************************************************************************
BiQuad::BiQuad() :
  a0(0),
  a1(0),
  a2(0),
  b1(0),
  b2(0),
  x_1(0),
  x_2(0),
  y_1(0),
  y_2(0)
{

}

//************************************************************************************************
void BiQuad::FlushDelays()
{
  x_1 = 0.0f;
  x_2 = 0.0f;
  y_1 = 0.0f;
  y_2 = 0.0f;
}

//************************************************************************************************
void BiQuad::SetValues(const float a0_, const float a1_, const float a2_, const float b1_, const float b2_)
{
  a0 = a0_;
  a1 = a1_;
  a2 = a2_;
  b1 = b1_;
  b2 = b2_;
}

//************************************************************************************************
float BiQuad::DoBiQuad(const float x)
{
  float y = (a0 * x) + (a1 * x_1) + (a2 * x_2) - (b1 * y_1) - (b2 * y_2);

  y_2 = y_1;
  y_1 = y;
  x_2 = x_1;
  x_1 = x;

  return y;
}

//************************************************************************************************
void BiQuad::AddHistoryTo(BiQuad& otherFilter)
{
  otherFilter.x_1 += x_1;
  otherFilter.x_2 += x_2;
  otherFilter.y_1 += y_1;
  otherFilter.y_2 += y_2;
}

//----------------------------------------------------------------------------------- Delay Filter

//************************************************************************************************
Delay::Delay(float maxDelayTime, int sampleRate) :
  mBuffer(nullptr),
  mDelayInSamples(0),
  mOutputAttenuation(0),
  mBufferSize(0),
  mReadIndex(0),
  mWriteIndex(0),
  mSampleRate(sampleRate)
{
  mBufferSize = (int)(maxDelayTime * sampleRate);
  mBuffer = new float[mBufferSize];
  memset(mBuffer, 0, mBufferSize * sizeof(float));
}

//************************************************************************************************
Delay::~Delay()
{
  if (mBuffer)
    delete[] mBuffer;

}

//************************************************************************************************
void Delay::ResetDelay()
{
  if (mBuffer)
    memset(mBuffer, 0, mBufferSize * sizeof(float));

  mWriteIndex = 0;

  mReadIndex = mBufferSize - (int)mDelayInSamples;
  mPrevReadIndex = mReadIndex - 1;
}

//************************************************************************************************
void Delay::SetDelayMSec(const float mSec)
{
  mDelayInSamples = mSec * (mSampleRate / 1000.0f);

  mReadIndex = mWriteIndex - (int)mDelayInSamples;
  if (mReadIndex < 0)
    mReadIndex += mBufferSize;
  mPrevReadIndex = mReadIndex - 1;
  if (mPrevReadIndex < 0)
    mPrevReadIndex += mBufferSize;
}

//************************************************************************************************
void Delay::SetOutputAttenuation(const float attenDB)
{
  mOutputAttenuation = Math::Pow(10.0f, attenDB / 20.0f);
}

//************************************************************************************************
float Delay::ReadDelay()
{
  return mBuffer[mPrevReadIndex] + ((mBuffer[mReadIndex] - mBuffer[mPrevReadIndex])
    * (mDelayInSamples - (int)mDelayInSamples));
}

//************************************************************************************************
float Delay::ReadDelayAt(const float mSec)
{
  int readIndex = mWriteIndex - (int)(mSec * (mSampleRate / 1000.0f));
  if (readIndex < 0)
    readIndex += mBufferSize;
  // If still zero or if larger than buffer size, bad value passed in
  if (readIndex < 0 || readIndex >= mBufferSize)
    return 0;

  int prevIndex = readIndex - 1;
  if (prevIndex < 0)
    prevIndex = mBufferSize - 1;

  return mBuffer[prevIndex] + ((mBuffer[readIndex] - mBuffer[prevIndex])
    * (mDelayInSamples - (int)mDelayInSamples));
}

//************************************************************************************************
void Delay::WriteDelayAndInc(const float delayInput)
{
  mBuffer[mWriteIndex] = delayInput;

  ++mWriteIndex;
  if (mWriteIndex == mBufferSize)
    mWriteIndex = 0;

  ++mReadIndex;
  if (mReadIndex == mBufferSize)
    mReadIndex = 0;

  ++mPrevReadIndex;
  if (mPrevReadIndex == mBufferSize)
    mPrevReadIndex = 0;
}

//************************************************************************************************
void Delay::ProcessAudio(const float input, float *output)
{
  if (mDelayInSamples == 0)
    *output = input * mOutputAttenuation;
  else
    *output = ReadDelay() * mOutputAttenuation;

  WriteDelayAndInc(input);
}

//-------------------------------------------------------------------------------- DelayAPF Filter

//************************************************************************************************
DelayAPF::DelayAPF(const float maxDelayTime, const int sampleRate) :
  mAPFg(0),
  Delay(maxDelayTime, sampleRate)
{

}

//************************************************************************************************
void DelayAPF::ProcessAudio(const float input, float *output)
{
  if (mReadIndex == mWriteIndex)
  {
    WriteDelayAndInc(input);
    *output = input;
    return;
  }

  float delayedSample = ReadDelay();

  float inputWithDelay = input + (mAPFg * delayedSample);

  *output = delayedSample + (-mAPFg * inputWithDelay);

  WriteDelayAndInc(inputWithDelay);
}

//------------------------------------------------------------------------------------ Comb Filter

//************************************************************************************************
Comb::Comb(const float maxDelayTime, const int sampleRate) :
  mCombG(0),
  Delay(maxDelayTime, sampleRate)
{

}

//************************************************************************************************
void Comb::SetCombGWithRT60(const float RT)
{
  mCombG = Math::Pow(10.0f, (-3.0f * mDelayInSamples * (1.0f / mSampleRate) * 1000.0f) / RT);
}

//************************************************************************************************
void Comb::ProcessAudio(const float input, float *output)
{
  if (mReadIndex == mWriteIndex)
  {
    WriteDelayAndInc(input);
    *output = input;
    return;
  }

  *output = ReadDelay();

  WriteDelayAndInc(input + (mCombG * (*output)));
}

//--------------------------------------------------------------------------- Low Pass Comb Filter

//************************************************************************************************
LPComb::LPComb(const float maxDelayTime, const int sampleRate) :
  mCombG(0),
  mLPFg(0),
  mPrevSample(0),
  Delay(maxDelayTime, sampleRate)
{

}

//************************************************************************************************
void LPComb::SetG(const float combG, const float overallGain)
{
  mCombG = combG;
  mLPFg = overallGain * (1.0f - mCombG);
}

//************************************************************************************************
void LPComb::SetGWithRT60(const float RT, const float overallGain)
{
  mCombG = Math::Pow(10.0f, (-3.0f * mDelayInSamples * (1.0f / mSampleRate) * 1000.0f) / RT);
  mLPFg = overallGain * (1.0f - mCombG);
}

//************************************************************************************************
void LPComb::ProcessAudio(const float input, float *output)
{
  if (mReadIndex == mWriteIndex)
  {
    *output = input;
    WriteDelayAndInc(input);
    return;
  }

  mPrevSample = ReadDelay() + (mLPFg * mPrevSample);

  *output = input + (mCombG * mPrevSample);

  WriteDelayAndInc(*output);
}

//----------------------------------------------------------------------- One Pole Low Pass Filter

//************************************************************************************************
OnePoleLP::OnePoleLP() :
  mLPFg(0),
  mPrevSample(0)
{

}

//************************************************************************************************
void OnePoleLP::SetLPFg(const float g)
{
  mLPFg = g;
  mInvG = 1.0f - mLPFg;
}

//************************************************************************************************
void OnePoleLP::Initialize()
{
  mPrevSample = 0;
}

//************************************************************************************************
void OnePoleLP::ProcessAudio(const float input, float *output)
{
  *output = (input * mInvG) + (mLPFg * mPrevSample);

  mPrevSample = *output;
}

//-------------------------------------------------------------------------------- Low Pass Filter

//************************************************************************************************
LowPassFilter::LowPassFilter() :
  CutoffFrequency(20001.0f),
  HalfPI(Math::cPi / 2.0f),
  SqRoot2(Math::Sqrt(2.0f))
{
  SetCutoffValues();

  for (unsigned i = 0; i < cMaxChannels; ++i)
    BiQuadsPerChannel[i].FlushDelays();
}

//************************************************************************************************
void LowPassFilter::SetCutoffValues()
{
  // LP coefficients
  float tanValue = Math::cPi * CutoffFrequency / cSystemSampleRate;
  if (tanValue >= HalfPI)
    tanValue = HalfPI - 0.001f;

  float C = 1.0f / Math::Tan(tanValue);
  float Csq = C * C;

  float alpha = 1.0f / (1.0f + (SqRoot2 * C) + Csq);
  float beta1 = 2.0f * alpha * (1.0f - Csq);
  float beta2 = alpha * (1.0f - (SqRoot2 * C) + Csq);

  for (unsigned i = 0; i < cMaxChannels; ++i)
    BiQuadsPerChannel[i].SetValues(alpha, 2.0f * alpha, alpha, beta1, beta2);
}

//************************************************************************************************
void LowPassFilter::SetCutoffFrequency(float value)
{
  CutoffFrequency = value;
  SetCutoffValues();
}

//************************************************************************************************
void LowPassFilter::MergeWith(LowPassFilter& otherFilter)
{
  for (int i = 0; i < cMaxChannels; ++i)
  {
    BiQuadsPerChannel[i].AddHistoryTo(otherFilter.BiQuadsPerChannel[i]);
  }
}

//************************************************************************************************
void LowPassFilter::ProcessFrame(const float* input, float* output, const unsigned numChannels)
{
  if (CutoffFrequency > 20000.0f)
  {
    memcpy(output, input, sizeof(float) * numChannels);
  }
  else
  {
    for (unsigned i = 0; i < numChannels; ++i)
      output[i] = BiQuadsPerChannel[i].DoBiQuad(input[i]);
  }
}

//************************************************************************************************
void LowPassFilter::ProcessBuffer(const float* input, float* output, const unsigned numChannels,
  const unsigned numSamples)
{
  if (CutoffFrequency > 20000.0f)
  {
    memcpy(output, input, sizeof(float) * numSamples);
    return;
  }

  for (unsigned i = 0; i < numSamples; i += numChannels)
    ProcessFrame(input + i, output + i, numChannels);
}

//************************************************************************************************
float LowPassFilter::GetCutoffFrequency()
{
  return CutoffFrequency;
}

//------------------------------------------------------------------------------- High Pass Filter

//************************************************************************************************
HighPassFilter::HighPassFilter() :
  CutoffFrequency(10.0f),
  HalfPI(Math::cPi / 2.0f),
  SqRoot2(Math::Sqrt(2.0f))
{
  SetCutoffValues();

  for (unsigned i = 0; i < cMaxChannels; ++i)
    BiQuadsPerChannel[i].FlushDelays();
}

//************************************************************************************************
void HighPassFilter::SetCutoffValues()
{
  float tanValue = Math::cPi * CutoffFrequency / cSystemSampleRate;
  if (tanValue >= HalfPI)
    tanValue = HalfPI - 0.001f;

  float C = Math::Tan(tanValue);
  float Csq = C * C;

  float alpha = 1.0f / (1.0f + (SqRoot2 * C) + Csq);
  float beta1 = 2.0f * alpha * (Csq - 1.0f);
  float beta2 = alpha * (1.0f - (SqRoot2 * C) + Csq);

  for (unsigned i = 0; i < cMaxChannels; ++i)
    BiQuadsPerChannel[i].SetValues(alpha, -2.0f * alpha, alpha, beta1, beta2);
}

//************************************************************************************************
void HighPassFilter::SetCutoffFrequency(const float value)
{
  CutoffFrequency = value;
  SetCutoffValues();
}

//************************************************************************************************
void HighPassFilter::MergeWith(HighPassFilter& otherFilter)
{
  for (int i = 0; i < cMaxChannels; ++i)
  {
    BiQuadsPerChannel[i].AddHistoryTo(otherFilter.BiQuadsPerChannel[i]);
  }
}

//************************************************************************************************
void HighPassFilter::ProcessFrame(const float* input, float* output, const unsigned numChannels)
{
  if (CutoffFrequency < 20.0f)
    memcpy(output, input, sizeof(float) * numChannels);
  else
  {
    for (unsigned i = 0; i < numChannels; ++i)
      output[i] = BiQuadsPerChannel[i].DoBiQuad(input[i]);
  }
}

//------------------------------------------------------------------------------- Band Pass Filter

//************************************************************************************************
BandPassFilter::BandPassFilter() :
  Quality(0.669f),
  CentralFreq(1000.0f)
{
  ResetFrequencies();

  memset(PreviousInput, 0, sizeof(float) * cMaxChannels);
  memset(PreviousOutput1, 0, sizeof(float) * cMaxChannels);
  memset(PreviousOutput2, 0, sizeof(float) * cMaxChannels);
}

//************************************************************************************************
void BandPassFilter::SetFrequency(const float freq)
{
  CentralFreq = freq;

  ResetFrequencies();
}

//************************************************************************************************
void BandPassFilter::SetQuality(const float Q)
{
  Quality = Q;

  ResetFrequencies();
}

//************************************************************************************************
void BandPassFilter::MergeWith(BandPassFilter& otherFilter)
{
  for (int i = 0; i < cMaxChannels; ++i)
  {
    otherFilter.PreviousInput[i] += PreviousInput[i];
    otherFilter.PreviousOutput1[i] += PreviousOutput1[i];
    otherFilter.PreviousOutput2[i] += PreviousOutput2[i];
  }
}

//************************************************************************************************
void BandPassFilter::ProcessFrame(const float* input, float* output, const unsigned numChannels)
{
  for (unsigned i = 0; i < numChannels; ++i)
  {
    float inputSample = input[i];
    output[i] = (AlphaHP * (1 - AlphaLP) * (inputSample - PreviousInput[i]))
      + ((AlphaHP + AlphaLP) * PreviousOutput1[i]) - (AlphaLP * AlphaHP * PreviousOutput2[i]);

    PreviousInput[i] = inputSample;
    PreviousOutput2[i] = PreviousOutput1[i];
    PreviousOutput1[i] = output[i];
  }
}

//************************************************************************************************
void BandPassFilter::ResetFrequencies()
{
  HighPassCutoff = (2.0f * CentralFreq * Quality) / (Math::Sqrt(4.0f * Quality * Quality + 1.0f) + 1.0f);

  if (HighPassCutoff != 0.0f)
    LowPassCutoff = (CentralFreq * CentralFreq) / HighPassCutoff;
  else
    LowPassCutoff = 0.0f;

  AlphaLP = cSystemSampleRate / ((LowPassCutoff * 2.0f * Math::cPi) +
    cSystemSampleRate);
  AlphaHP = cSystemSampleRate / ((HighPassCutoff * 2.0f * Math::cPi) +
    cSystemSampleRate);
}

//------------------------------------------------------------------------------------- Oscillator

//************************************************************************************************
Oscillator::Oscillator() :
  mReadIndex(0),
  mIncrement(0),
  mNoteOn(false),
  mFrequency(1.0f),
  mType(SynthWaveType::Noise),
  mPolarity(Bipolar),
  mSquareWavePositiveFraction(0.5f)
{
  mIncrement = ArraySize * mFrequency / (float)cSystemSampleRate;
  SetType(SynthWaveType::SineWave);
}

//************************************************************************************************
void Oscillator::ProcessBuffer(float *buffer, const unsigned numChannels, const unsigned bufferSize)
{
  for (unsigned i = 0; i < bufferSize; i += numChannels)
  {
    float value = GetNextSample();

    for (unsigned j = 0; j < numChannels; ++j)
    {
      buffer[i + j] = value;
    }
  }
}

//************************************************************************************************
float Oscillator::GetNextSample()
{
  // If not running, exit
  if (!mNoteOn)
    return 0.0f;

  if (mType == SynthWaveType::Noise)
    return RandomObject.FloatRange(-1.0f, 1.0f) * cGeneratedWaveVolume;

  int readIndexInt = (int)mReadIndex;

  ErrorIf(readIndexInt >= ArraySize, "Audio Engine: Oscillator read index went beyond array size");

  int readIndexNext = readIndexInt + 1;
  if (readIndexNext > ArraySize - 1)
    readIndexNext = 0;

  float returnValue = mWaveValues[readIndexInt] + (mReadIndex - readIndexInt)
    * (mWaveValues[readIndexNext] - mWaveValues[readIndexInt]);

  mReadIndex += mIncrement;
  if (mReadIndex >= ArraySize)
    mReadIndex -= ArraySize;

  if (mPolarity == Unipolar)
  {
    returnValue *= 0.5f;
    returnValue += 0.5f;

    ErrorIf(returnValue < 0 || returnValue > 1.0f, "Audio Engine: Oscillator value out of bounds");
  }

  return returnValue;
}

//************************************************************************************************
void Oscillator::SetFrequency(const float frequency)
{
  mFrequency = frequency;
  mIncrement = ArraySize * mFrequency / cSystemSampleRate;
}

//************************************************************************************************
void Oscillator::SetType(const SynthWaveType::Enum newType)
{
  if (newType == mType)
    return;

  mType = newType;

  if (mType == SynthWaveType::SineWave)
  {
    for (int i = 0; i < ArraySize; ++i)
      mWaveValues[i] = 0.99f * Math::Sin(((float)i / (float)ArraySize) * Math::cTwoPi);
  }
  else if (mType == SynthWaveType::SawWave)
  {
    int halfBuffer = ArraySize / 2;
    float sawtoothInc = 1.0f / halfBuffer;
    for (int i = 0; i < ArraySize; ++i)
    {
      if (i < halfBuffer)
        mWaveValues[i] = i * sawtoothInc;
      else
        mWaveValues[i] = ((i - halfBuffer - 1) * sawtoothInc - 1.0f);
    }
  }
  else if (mType == SynthWaveType::SquareWave)
  {
    int positiveSize = (int)(mSquareWavePositiveFraction * ArraySize);

    for (int i = 0; i < ArraySize; ++i)
    {
      if (i < positiveSize)
        mWaveValues[i] = 1.0f;
      else
        mWaveValues[i] = -1.0f;
    }
  }
  else if (mType == SynthWaveType::TriangleWave)
  {
    int halfBuffer = ArraySize / 2;
    int quarterBuffer = halfBuffer / 2;
    int threeQtrBuffer = halfBuffer + quarterBuffer;
    float triangleIncRising = 1.0f / quarterBuffer;
    float triangleIncFalling = -2.0f / halfBuffer;
    for (int i = 0; i < ArraySize; ++i)
    {
      if (i < quarterBuffer)
        // Interpolate from 0 to 1 over first quarter
        mWaveValues[i] = triangleIncRising * i;
      else if (i < threeQtrBuffer)
        // Interpolate from 1 to -1 over second and third quarters
        mWaveValues[i] = (triangleIncFalling * (i - quarterBuffer) + 1.0f);
      else
        // Interpolate from -1 to 0 over fourth quarter
        mWaveValues[i] = (triangleIncRising * (i - threeQtrBuffer) - 1.0f);
    }
  }
}

//************************************************************************************************
void Oscillator::SetSquareWavePositiveFraction(const float positiveFraction)
{
  mSquareWavePositiveFraction = positiveFraction;

  // If necessary, re-create the wave data
  if (mType == SynthWaveType::SquareWave)
  {
    int positiveSize = (int)(mSquareWavePositiveFraction * ArraySize);

    for (int i = 0; i < ArraySize; ++i)
    {
      if (i < positiveSize)
        mWaveValues[i] = 1.0f;
      else
        mWaveValues[i] = -1.0f;
    }
  }
}

//------------------------------------------------------------------------------------- Delay Line

//************************************************************************************************
DelayLine::DelayLine() :
  mDelaySamplesFractional(100.0f * cSystemSampleRate / 1000.0f),
  mFeedback(0),
  mWetLevel(0.5f),
  mReadIndex(0),
  mWriteIndex(0),
  mBufferSize(0),
  mInterpolatingWetLevel(false)
{
  mDelaySamples = (int)mDelaySamplesFractional;
  memset(mBuffersPerChannel, 0, sizeof(float) * cMaxChannels);
}

//************************************************************************************************
DelayLine::~DelayLine()
{
  DestroyBuffers();
}

//************************************************************************************************
void DelayLine::ProcessBuffer(const float* input, float* output, const unsigned numberOfChannels,
  const unsigned bufferSize)
{
  // If there is no delay, simply copy input into output
  if (mDelaySamplesFractional == 0.0f)
  {
    memcpy(output, input, sizeof(float) * bufferSize);
    return;
  }

  // Check if we need to allocate buffers
  if (!mBuffersPerChannel[numberOfChannels - 1])
  {
    CreateBuffers(mBufferSize, numberOfChannels);
  }

  for (unsigned frameIndex = 0; frameIndex < bufferSize; frameIndex += numberOfChannels)
  {
    for (unsigned channel = 0; channel < numberOfChannels; ++channel)
    {
      // Read input for this channel
      float inputSample = input[frameIndex + channel];
      float delayedSample;

      // If delay is less than 1 sample, interpolate between input(n) and input(n-1)
      if (mReadIndex == mWriteIndex && mDelaySamplesFractional < 1.0f)
        delayedSample = inputSample;
      // Otherwise, get delayed sample from buffer
      else
        delayedSample = mBuffersPerChannel[channel][mReadIndex];

      // Read previous delayed sample
      int readIndex_1 = mReadIndex - 1;
      if (readIndex_1 < 0)
        readIndex_1 += mBufferSize;
      float delayedSample_1 = mBuffersPerChannel[channel][readIndex_1];

      // Interpolate between the two samples
      delayedSample = delayedSample_1 + ((mDelaySamplesFractional - mDelaySamples) *
        (delayedSample - delayedSample_1));

      // Write input to delay buffer
      mBuffersPerChannel[channel][mWriteIndex] = inputSample + (mFeedback * delayedSample);

      // Check if we are interpolating the wet level
      if (mInterpolatingWetLevel)
      {
        mWetLevel = WetLevelInterpolator.NextValue();
        mInterpolatingWetLevel = !WetLevelInterpolator.Finished();
      }

      // Write wet/dry mix to output buffer
      output[frameIndex + channel] = (mWetLevel * delayedSample) + ((1.0f - mWetLevel) * inputSample);
    }

    // Done with this frame, increment indexes
    ++mWriteIndex;
    if (mWriteIndex >= mBufferSize)
      mWriteIndex = 0;
    ++mReadIndex;
    if (mReadIndex >= mBufferSize)
      mReadIndex = 0;
  }
}

//************************************************************************************************
void DelayLine::SetDelayMSec(float delay)
{
  mDelaySamplesFractional = delay * cSystemSampleRate / 1000.0f;

  int delayDifference = (int)mDelaySamplesFractional - mDelaySamples;

  mDelaySamples = (int)mDelaySamplesFractional;

  if (mBufferSize <= mDelaySamples && mDelaySamplesFractional > 0.0f)
  {
    ReallocateBuffers(mDelaySamples + 1);
  }
  else if (mDelaySamplesFractional > 0.0f)
  {
    // this will move the index forward if the delay is smaller, and backward if it is larger
    mReadIndex -= delayDifference;

    if (mReadIndex >= mBufferSize)
      mReadIndex -= mBufferSize;
    else if (mReadIndex < 0)
      mReadIndex += mBufferSize;

    ErrorIf(mReadIndex < 0 || mReadIndex >= mBufferSize, "Error changing read index when setting delay");
  }
}

//************************************************************************************************
float DelayLine::GetDelayMSec()
{
  return mDelaySamplesFractional / cSystemSampleRate * 1000.0f;
}

//************************************************************************************************
void DelayLine::SetFeedback(const float feedbackValue)
{
  mFeedback = feedbackValue;

}

//************************************************************************************************
void DelayLine::SetWetLevel(const float wetLevelValue)
{
  mWetLevel = wetLevelValue;

}

//************************************************************************************************
void DelayLine::InterpolateWetLevel(const float newValue, const float time)
{
  mInterpolatingWetLevel = true;
  WetLevelInterpolator.SetValues(mWetLevel, newValue, (unsigned)(time * cSystemSampleRate));
}

//************************************************************************************************
bool DelayLine::IsDataInBuffer()
{
  for (unsigned i = 0; i < cMaxChannels; ++i)
  {
    // Check if this buffer exists
    if (!mBuffersPerChannel[i])
      continue;

    // If there is any value in this buffer, return true
    for (int j = mReadIndex; j != mWriteIndex; ++j)
    {
      if (j >= mBufferSize)
        j = 0;

      if (mBuffersPerChannel[i][j] != 0)
        return true;
    }
  }

  // No values other than zero were found, so return false
  return false;
}

//************************************************************************************************
void DelayLine::SetMaxDelayMSec(float maxDelay, int numChannels)
{
  int delaySamples = (int)(maxDelay * cSystemSampleRate / 1000.0f);

  if (delaySamples >= mBufferSize)
  {
    DestroyBuffers();

    mBufferSize = delaySamples + 1;

    CreateBuffers(mBufferSize, numChannels);
  }
}

//************************************************************************************************
void DelayLine::CreateBuffers(int length, int numChannels)
{
  for (int i = 0; i < numChannels; ++i)
  {
    if (mBuffersPerChannel[i] != nullptr)
      continue;

    mBuffersPerChannel[i] = new float[length];
    memset(mBuffersPerChannel[i], 0, sizeof(float) * length);
  }
}

//************************************************************************************************
void DelayLine::DestroyBuffers()
{
  for (unsigned i = 0; i < cMaxChannels; ++i)
  {
    if (mBuffersPerChannel[i])
    {
      delete[] mBuffersPerChannel[i];
      mBuffersPerChannel[i] = nullptr;
    }
  }
}

//************************************************************************************************
void DelayLine::ReallocateBuffers(int newLength)
{
  if (newLength <= mBufferSize)
    return;

  int dataStart = newLength - mBufferSize;
  int firstChunk = mBufferSize - mReadIndex;

  for (unsigned i = 0; i < cMaxChannels; ++i)
  {
    if (!mBuffersPerChannel[i])
      continue;

    float* newBuffer = new float[newLength];
    memset(newBuffer, 0, sizeof(float) * newLength);

    memcpy(newBuffer + dataStart, mBuffersPerChannel[i] + mReadIndex, sizeof(float) * mBufferSize - mReadIndex);
    memcpy(newBuffer + dataStart + mBufferSize - mReadIndex, mBuffersPerChannel[i] + mWriteIndex,
      sizeof(float) * mWriteIndex);

    delete[] mBuffersPerChannel[i];
    mBuffersPerChannel[i] = newBuffer;
  }

  mWriteIndex = 0;
  mReadIndex = newLength - mDelaySamples;
  ErrorIf(mReadIndex < 1, "Read index bad position when reallocating buffers for delay line");

  mBufferSize = newLength;
}

//------------------------------------------------------------------------------ Envelope Detector

//************************************************************************************************
EnvelopeDetector::EnvelopeDetector() :
  mAttackTimeMSec(0.0f),
  mReleaseTimeMSec(0.0f),
  mAttackTime(0.0f),
  mReleaseTime(0.0f),
  mSampleRate((float)cSystemSampleRate),
  mEnvelope(0.0f),
  mDetectMode(DetectModes::Peak),
  mSample(0),
  mAnalogTC(false),
  mLogDetector(false)
{

}

//************************************************************************************************
void EnvelopeDetector::Initialize(const float attackMSec, const float releaseMSec, const bool analogTC,
  const DetectModes mode, const bool logDetector)
{
  mEnvelope = 0.0f;
  mAnalogTC = analogTC;
  mAttackTimeMSec = attackMSec;
  mReleaseTimeMSec = releaseMSec;
  mDetectMode = mode;
  mLogDetector = logDetector;

  SetAttackTime(attackMSec);
  SetReleaseTime(releaseMSec);
}

//************************************************************************************************
void EnvelopeDetector::SetTCModeAnalog(const bool analogTC)
{
  mAnalogTC = analogTC;
  SetAttackTime(mAttackTimeMSec);
  SetReleaseTime(mReleaseTimeMSec);
}

//************************************************************************************************
void EnvelopeDetector::SetAttackTime(const float attackMSec)
{
  mAttackTimeMSec = attackMSec;

  if (mAnalogTC)
    mAttackTime = Math::Exp(ANALOG_TC / (attackMSec * mSampleRate * 0.001f));
  else
    mAttackTime = Math::Exp(DIGITAL_TC / (attackMSec * mSampleRate * 0.001f));
}

//************************************************************************************************
void EnvelopeDetector::SetReleaseTime(const float releaseMSec)
{
  mReleaseTimeMSec = releaseMSec;

  if (mAnalogTC)
    mReleaseTime = Math::Exp(ANALOG_TC / (releaseMSec * mSampleRate * 0.001f));
  else
    mReleaseTime = Math::Exp(DIGITAL_TC / (releaseMSec * mSampleRate * 0.001f));
}

//************************************************************************************************
void EnvelopeDetector::SetDetectMode(const DetectModes mode)
{
  mAnalogTC = mode;

  SetAttackTime(mAttackTimeMSec);
  SetReleaseTime(mReleaseTimeMSec);
}

//************************************************************************************************
void EnvelopeDetector::Reset()
{
  mEnvelope = 0.0f;
  mSample = 0;
}

//************************************************************************************************
float EnvelopeDetector::Detect(float input)
{
  input = Math::Abs(input);

  switch (mDetectMode)
  {
  case Peak:
    break;
  case MS:
    input *= input;
    break;
  case RMS:
    input = Math::Pow(input * input, 0.5f);
    break;
  default:
    break;
  }

  if (input > mEnvelope)
    mEnvelope = (mAttackTime * (mEnvelope - input)) + input;
  else
    mEnvelope = (mReleaseTime * (mEnvelope - input)) + input;

  if (mEnvelope > 0.0f && mEnvelope < FLT_MIN)
    mEnvelope = 0.0f;
  else if (mEnvelope < 0.0f && mEnvelope > -FLT_MIN)
    mEnvelope = 0.0f;

  mEnvelope = Math::Min(mEnvelope, 1.0f);
  mEnvelope = Math::Max(mEnvelope, 0.0f);

  // 16-bit scaling
  if (mLogDetector)
  {
    if (mEnvelope <= 0.0f)
      return -96.0f; // 16 bit noise floor

    return 20.0f * Math::Log10(mEnvelope);
  }

  return mEnvelope;
}

//---------------------------------------------------------------------- Dynamics Processor Filter

//************************************************************************************************
DynamicsProcessor::DynamicsProcessor() :
  mInputGainDB(0),
  mThresholdDB(0),
  mAttackMSec(20),
  mReleaseMSec(1000),
  mRatio(1),
  mOutputGainDB(0),
  mKneeWidth(0),
  mHalfKnee(0),
  mProcessorType(Compressor),
  mAnalog(true)
{
  mHalfKnee = mKneeWidth * 0.5f;

  mOutputGain = Math::Pow(10.0f, mOutputGainDB / 20.0f);
  mCompressorRatio = 1.0f / mRatio;
  mExpanderRatio = mCompressorRatio - 1.0f;

  for (unsigned i = 0; i < cMaxChannels; ++i)
    Detectors[i].Initialize(mAttackMSec, mReleaseMSec, mAnalog, EnvelopeDetector::RMS, true);
}

//************************************************************************************************
DynamicsProcessor::DynamicsProcessor(const float inputGain, const float threshold, const float attack,
  const float release, const float ratio, const float outputGain, const float knee, const ProcessorTypes type) :
  mInputGainDB(inputGain),
  mThresholdDB(threshold),
  mAttackMSec(attack),
  mReleaseMSec(release),
  mRatio(ratio),
  mOutputGainDB(outputGain),
  mKneeWidth(knee),
  mHalfKnee(0),
  mProcessorType(type),
  mAnalog(true)
{
  mOutputGain = Math::Pow(10.0f, mOutputGainDB / 20.0f);
  mCompressorRatio = 1.0f / mRatio;
  mExpanderRatio = mCompressorRatio - 1.0f;

  for (unsigned i = 0; i < cMaxChannels; ++i)
    Detectors[i].Initialize(mAttackMSec, mReleaseMSec, mAnalog, EnvelopeDetector::RMS, true);
}

//************************************************************************************************
void DynamicsProcessor::ProcessBuffer(const float *input, const float *envelopeInput, float *output,
  const unsigned numChannels, const unsigned bufferSize)
{
  float gain;

  for (unsigned i = 0; i < bufferSize; i += numChannels)
  {
    for (unsigned channel = 0; channel < numChannels; ++channel)
    {
      if (mProcessorType == Compressor || mProcessorType == Limiter)
        gain = CompressorGain(Detectors[channel].Detect(envelopeInput[i + channel]));
      else
        gain = ExpanderGain(Detectors[channel].Detect(envelopeInput[i + channel]));

      output[i + channel] = gain * input[i + channel] * mOutputGain;
    }
  }
}

//************************************************************************************************
float DynamicsProcessor::CompressorGain(const float detectorValue)
{
  float slope;

  if (mProcessorType == Limiter)
    slope = 1.0f;
  else
    slope = mCompressorRatio;

  // Soft-knee with detection value in range?
  if (mKneeWidth > 0 && detectorValue > (mThresholdDB - mHalfKnee)
    && detectorValue < (mThresholdDB + mHalfKnee))
  {
    // Set up for Lagrange interpolation
    double x[2], y[2];

    x[0] = mThresholdDB - mHalfKnee;
    x[1] = mThresholdDB + mHalfKnee;
    // (top limit is 0dB)
    if (x[1] > 0)
      x[1] = 0;

    y[0] = 0;
    y[1] = slope;

    slope = (float)LagrangeInterpolation(x, y, 2, detectorValue);
  }

  float gain = slope * (mThresholdDB - detectorValue);

  if (gain > 0)
    gain = 0;

  return Math::Pow(10.0f, gain / 20.0f);
}

//************************************************************************************************
float DynamicsProcessor::ExpanderGain(const float detectorValue)
{
  float slope;

  if (mProcessorType == Gate)
    slope = -1.0f;
  else
    slope = mExpanderRatio;

  // Soft-knee with detection value in range?
  if (mKneeWidth > 0 && detectorValue > (mThresholdDB - mHalfKnee)
    && detectorValue < (mThresholdDB + mHalfKnee))
  {
    // Set up for Lagrange interpolation
    double x[2], y[2];

    x[0] = mThresholdDB - mHalfKnee;
    x[1] = mThresholdDB + mHalfKnee;
    // (top limit is 0dB)
    if (x[1] > 0)
      x[1] = 0;

    y[0] = slope;
    y[1] = 0;

    slope = (float)LagrangeInterpolation(x, y, 2, detectorValue);
  }

  float gain = slope * (mThresholdDB - detectorValue);

  if (gain > 0)
    gain = 0;

  return Math::Pow(10.0f, gain / 20.0f);
}

//************************************************************************************************
void DynamicsProcessor::SetAttackMSec(const float attack)
{
  mAttackMSec = attack;

  for (unsigned i = 0; i < cMaxChannels; ++i)
    Detectors[i].SetAttackTime(attack);
}

//************************************************************************************************
void DynamicsProcessor::SetReleaseMSec(const float release)
{
  mReleaseMSec = release;

  for (unsigned i = 0; i < cMaxChannels; ++i)
    Detectors[i].SetReleaseTime(release);
}

//************************************************************************************************
void DynamicsProcessor::SetRatio(const float ratio)
{
  mRatio = ratio;
  if (ratio == 0)
  {
    mCompressorRatio = 1.0f;
    mExpanderRatio = -1.0f;
  }
  else
  {
    mCompressorRatio = 1.0f - (1.0f / ratio);
    mExpanderRatio = (1.0f / ratio) - 1.0f;
  }
}

//************************************************************************************************
void DynamicsProcessor::SetOutputGain(const float gainDB)
{
  mOutputGainDB = gainDB;
  mOutputGain = Math::Pow(10.0f, mOutputGainDB / 20.0f);
}

//************************************************************************************************
void DynamicsProcessor::SetKneeWidth(const float kneeWidth)
{
  mKneeWidth = kneeWidth;
  mHalfKnee = kneeWidth * 0.5f;
}

//************************************************************************************************
double DynamicsProcessor::LagrangeInterpolation(double *x, double *y, int howMany, double xBar)
{
  double interpolatedValue(0.0f);
  double tempValue;

  for (int i = 0; i < howMany; ++i)
  {
    tempValue = 1.0f;

    for (int j = 0; j < howMany; ++j)
    {
      if (j != i)
        tempValue *= (xBar - x[j]) / (x[i] - x[j]);
    }

    interpolatedValue += tempValue * y[i];
  }

  return interpolatedValue;
}

//------------------------------------------------------------------------------- Equalizer Filter

//************************************************************************************************
Equalizer::Equalizer()
{
  for (int i = 0; i < EqualizerBands::Count; ++i)
    mBandGains[i] = 1.0f;

  SetFilterData();
}

//************************************************************************************************
Equalizer::Equalizer(const float below80Hz, const float at150Hz, const float at600Hz,
  const float at2500Hz, const float above5000Hz)
{
  mBandGains[EqualizerBands::Below80] = below80Hz;
  mBandGains[EqualizerBands::At150] = at150Hz;
  mBandGains[EqualizerBands::At600] = at600Hz;
  mBandGains[EqualizerBands::At2500] = at2500Hz;
  mBandGains[EqualizerBands::Above5000] = above5000Hz;

  SetFilterData();
}

//************************************************************************************************
Equalizer::Equalizer(const Equalizer& copy) 
{
  memcpy(mBandGains, copy.mBandGains, sizeof(float) * EqualizerBands::Count);

  SetFilterData();

  Equalizer& other = const_cast<Equalizer&>(copy);
  if (!other.LowPassInterpolator.Finished())
  {
    LowPassInterpolator.SetValues(other.LowPassInterpolator.GetStartValue(),
      other.LowPassInterpolator.GetEndValue(), other.LowPassInterpolator.GetTotalFrames());
    LowPassInterpolator.JumpForward(other.LowPassInterpolator.GetCurrentFrame());
    HighPassInterpolator.SetValues(other.HighPassInterpolator.GetStartValue(),
      other.HighPassInterpolator.GetEndValue(), other.HighPassInterpolator.GetTotalFrames());
    HighPassInterpolator.JumpForward(other.HighPassInterpolator.GetCurrentFrame());
    Band1Interpolator.SetValues(other.Band1Interpolator.GetStartValue(),
      other.Band1Interpolator.GetEndValue(), other.Band1Interpolator.GetTotalFrames());
    Band1Interpolator.JumpForward(other.Band1Interpolator.GetCurrentFrame());
    Band2Interpolator.SetValues(other.Band2Interpolator.GetStartValue(),
      other.Band2Interpolator.GetEndValue(), other.Band2Interpolator.GetTotalFrames());
    Band2Interpolator.JumpForward(other.Band2Interpolator.GetCurrentFrame());
    Band3Interpolator.SetValues(other.Band3Interpolator.GetStartValue(),
      other.Band3Interpolator.GetEndValue(), other.Band3Interpolator.GetTotalFrames());
    Band3Interpolator.JumpForward(other.Band3Interpolator.GetCurrentFrame());
  }
}

//************************************************************************************************
Equalizer::Equalizer(float* values)
{
  memcpy(mBandGains, values, sizeof(float) * EqualizerBands::Count);
  SetFilterData();
}

//************************************************************************************************
void Equalizer::ProcessBuffer(const float* input, float* output, const unsigned numChannels,
  const unsigned bufferSize)
{
  Zero::Array<float> resultSamples(numChannels);

  for (unsigned i = 0; i < bufferSize; i += numChannels)
  {
    LowPass.ProcessFrame(input + i, resultSamples.Data(), numChannels);
    if (!LowPassInterpolator.Finished())
      mBandGains[EqualizerBands::Below80] = LowPassInterpolator.NextValue();
    for (unsigned j = 0; j < numChannels; ++j)
      output[i + j] = resultSamples[j] * mBandGains[EqualizerBands::Below80];

    Band1.ProcessFrame(input + i, resultSamples.Data(), numChannels);
    if (!Band1Interpolator.Finished())
      mBandGains[EqualizerBands::At150] = Band1Interpolator.NextValue();
    for (unsigned j = 0; j < numChannels; ++j)
      output[i + j] += resultSamples[j] * mBandGains[EqualizerBands::At150];

    Band2.ProcessFrame(input + i, resultSamples.Data(), numChannels);
    if (!Band2Interpolator.Finished())
      mBandGains[EqualizerBands::At600] = Band2Interpolator.NextValue();
    for (unsigned j = 0; j < numChannels; ++j)
      output[i + j] += resultSamples[j] * mBandGains[EqualizerBands::At600];

    Band3.ProcessFrame(input + i, resultSamples.Data(), numChannels);
    if (!Band3Interpolator.Finished())
      mBandGains[EqualizerBands::At2500] = Band3Interpolator.NextValue();
    for (unsigned j = 0; j < numChannels; ++j)
      output[i + j] += resultSamples[j] * mBandGains[EqualizerBands::At2500];

    HighPass.ProcessFrame(input + i, resultSamples.Data(), numChannels);
    if (!HighPassInterpolator.Finished())
      mBandGains[EqualizerBands::Above5000] = HighPassInterpolator.NextValue();
    for (unsigned j = 0; j < numChannels; ++j)
      output[i + j] += resultSamples[j] * mBandGains[EqualizerBands::Above5000];
  }
}

//************************************************************************************************
float Equalizer::GetBandGain(EqualizerBands::Enum whichBand)
{
  return mBandGains[whichBand];
}

//************************************************************************************************
void Equalizer::SetBandGain(EqualizerBands::Enum whichBand, float gain)
{
  mBandGains[whichBand] = gain;
}

//************************************************************************************************
void Equalizer::InterpolateBands(float* gainValues, float timeToInterpolate)
{
  unsigned frames = (unsigned)(timeToInterpolate * cSystemSampleRate);

  LowPassInterpolator.SetValues(mBandGains[EqualizerBands::Below80], 
    gainValues[EqualizerBands::Below80], frames);
  Band1Interpolator.SetValues(mBandGains[EqualizerBands::At150], 
    gainValues[EqualizerBands::At150], frames);
  Band2Interpolator.SetValues(mBandGains[EqualizerBands::At600], 
    gainValues[EqualizerBands::At600], frames);
  Band3Interpolator.SetValues(mBandGains[EqualizerBands::At2500], 
    gainValues[EqualizerBands::At2500], frames);
  HighPassInterpolator.SetValues(mBandGains[EqualizerBands::Above5000], 
    gainValues[EqualizerBands::Above5000], frames);
}

//************************************************************************************************
void Equalizer::MergeWith(Equalizer& otherFilter)
{
  LowPass.MergeWith(otherFilter.LowPass);
  HighPass.MergeWith(otherFilter.HighPass);
  Band1.MergeWith(otherFilter.Band1);
  Band2.MergeWith(otherFilter.Band2);
  Band3.MergeWith(otherFilter.Band3);
}

//************************************************************************************************
void Equalizer::SetFilterData()
{
  LowPass.SetCutoffFrequency(79.62f);
  HighPass.SetCutoffFrequency(5023.77f);
  Band1.SetFrequency(158.87f);
  Band1.SetQuality(0.669f);
  Band2.SetFrequency(632.46f);
  Band2.SetQuality(0.669f);
  Band3.SetFrequency(2517.85f);
  Band3.SetQuality(0.669f);
}

//------------------------------------------------------------------------------------ Reverb Data

//************************************************************************************************
ReverbData::ReverbData() :
  PreDelay(0.5f, cSystemSampleRate),
  InputAP_1(0.5f, cSystemSampleRate),
  InputAP_2(0.5f, cSystemSampleRate),
  Comb_1(0.5f, cSystemSampleRate),
  Comb_2(0.5f, cSystemSampleRate),
  LPComb_1(0.5f, cSystemSampleRate),
  LPComb_2(0.5f, cSystemSampleRate),
  OutputAP(0.5f, cSystemSampleRate)
{

}

//************************************************************************************************
void ReverbData::Initialize(const float lpGain)
{
  InputLP.Initialize();
  DampingLP.Initialize();

  Comb_1.ResetDelay();
  Comb_2.ResetDelay();
  LPComb_1.ResetDelay();
  LPComb_2.ResetDelay();

  PreDelay.SetDelayMSec(40.0f);
  PreDelay.SetOutputAttenuation(0.0f);

  InputAP_1.SetDelayMSec(13.28f);
  InputAP_1.SetAPFg(0.7f);
  InputAP_2.SetDelayMSec(28.13f);
  InputAP_2.SetAPFg(-0.7f);

  DampingLP.SetLPFg(lpGain);

  InputLP.SetLPFg(0.45f);

  PreDelay.ResetDelay();
  InputAP_1.ResetDelay();
  InputAP_2.ResetDelay();
  OutputAP.ResetDelay();
}

//************************************************************************************************
float ReverbData::ProcessSample(const float input)
{
  float result;
  PreDelay.ProcessAudio(input, &result);

  InputAP_1.ProcessAudio(result, &result);

  InputAP_2.ProcessAudio(result, &result);

  InputLP.ProcessAudio(result, &result);

  float comb1result, comb2result, lpcomb1result, lpcomb2result;

  Comb_1.ProcessAudio(result, &comb1result);
  Comb_2.ProcessAudio(result, &comb2result);
  LPComb_1.ProcessAudio(result, &lpcomb1result);
  LPComb_2.ProcessAudio(result, &lpcomb2result);

  DampingLP.ProcessAudio(0.15f * (comb1result - comb2result + lpcomb1result - lpcomb2result), &result);

  OutputAP.ProcessAudio(result, &result);

  return result;
}

//----------------------------------------------------------------------------------------- Reverb

//************************************************************************************************
Reverb::Reverb() :
  TimeMSec(1000.0f),
  LPgain(0.5f),
  WetValue(0.5f)
{
  Initialize();
  SetTime(TimeMSec);
}

//************************************************************************************************
bool Reverb::ProcessBuffer(const float *input, float *output, const unsigned numChannels,
  const unsigned bufferSize)
{
  bool hasOutput(false);
  for (unsigned i = 0; i < bufferSize; i += numChannels)
  {
    if (!WetValueInterpolator.Finished())
      WetValue = WetValueInterpolator.NextValue();

    // Only process if reverb is turned on
    if (WetValue > 0)
    {
      for (unsigned channel = 0; channel < numChannels && channel < ChannelCount; ++channel)
      {
        output[i + channel] = ((1.0f - WetValue) * input[i + channel])
          + (WetValue * Data[channel].ProcessSample(input[i + channel]));

        if (Math::Abs(output[i + channel]) > 0.001f)
          hasOutput = true;
      }
    }
    else
    {
      // If not interpolating, copy the rest of the buffer 
      if (WetValueInterpolator.Finished())
      {
        memcpy(output, input, sizeof(float) * (bufferSize - i));
        break;
      }
      // Otherwise copy just this frame
      else
        memcpy(output, input, sizeof(float) * numChannels);
    }
  }

  return hasOutput;
}

//************************************************************************************************
void Reverb::SetTime(const float timeInMSec)
{
  TimeMSec = timeInMSec;
  for (unsigned i = 0; i < ChannelCount; ++i)
  {
    Data[i].Comb_1.SetCombGWithRT60(TimeMSec);
    Data[i].Comb_2.SetCombGWithRT60(TimeMSec);

    Data[i].LPComb_1.SetGWithRT60(TimeMSec, LPgain);
    Data[i].LPComb_2.SetGWithRT60(TimeMSec, LPgain);
  }
}

//************************************************************************************************
void Reverb::SetWetLevel(const float wetLevel)
{
  WetValue = wetLevel;
}

//************************************************************************************************
void Reverb::InterpolateWetLevel(const float newWetLevel, const float time)
{
  WetValueInterpolator.SetValues(WetValue, newWetLevel, (unsigned)(time * cSystemSampleRate));
}

//************************************************************************************************
void Reverb::Initialize()
{
  for (unsigned i = 0; i < ChannelCount; ++i)
    Data[i].Initialize(LPgain);

  Data[0].OutputAP.SetDelayMSec(9.38f);
  Data[0].OutputAP.SetAPFg(-0.6f);
  Data[1].OutputAP.SetDelayMSec(11.0f);
  Data[1].OutputAP.SetAPFg(0.6f);
  Data[2].OutputAP.SetDelayMSec(9.38f);
  Data[2].OutputAP.SetAPFg(-0.6f);
  Data[3].OutputAP.SetDelayMSec(9.38f);
  Data[3].OutputAP.SetAPFg(-0.6f);
  Data[4].OutputAP.SetDelayMSec(11.0f);
  Data[4].OutputAP.SetAPFg(0.6f);
  Data[5].OutputAP.SetDelayMSec(9.38f);
  Data[5].OutputAP.SetAPFg(-0.6f);
  Data[6].OutputAP.SetDelayMSec(11.0f);
  Data[6].OutputAP.SetAPFg(0.6f);

  Data[0].Comb_1.SetDelayMSec(31.71f);
  Data[0].Comb_2.SetDelayMSec(37.11f);
  Data[1].Comb_1.SetDelayMSec(30.47f);
  Data[1].Comb_2.SetDelayMSec(33.98f);
  Data[2].Comb_1.SetDelayMSec(31.71f);
  Data[2].Comb_2.SetDelayMSec(37.11f);
  Data[3].Comb_1.SetDelayMSec(31.71f);
  Data[3].Comb_2.SetDelayMSec(37.11f);
  Data[4].Comb_1.SetDelayMSec(30.47f);
  Data[4].Comb_2.SetDelayMSec(33.98f);
  Data[5].Comb_1.SetDelayMSec(31.71f);
  Data[5].Comb_2.SetDelayMSec(37.11f);
  Data[6].Comb_1.SetDelayMSec(30.47f);
  Data[6].Comb_2.SetDelayMSec(33.98f);

  Data[0].LPComb_1.SetDelayMSec(40.23f);
  Data[0].LPComb_2.SetDelayMSec(44.14f);
  Data[1].LPComb_1.SetDelayMSec(41.41f);
  Data[1].LPComb_2.SetDelayMSec(42.58f);
  Data[2].LPComb_1.SetDelayMSec(40.23f);
  Data[2].LPComb_2.SetDelayMSec(44.14f);
  Data[3].LPComb_1.SetDelayMSec(40.23f);
  Data[3].LPComb_2.SetDelayMSec(44.14f);
  Data[4].LPComb_1.SetDelayMSec(41.41f);
  Data[4].LPComb_2.SetDelayMSec(42.58f);
  Data[5].LPComb_1.SetDelayMSec(40.23f);
  Data[5].LPComb_2.SetDelayMSec(44.14f);
  Data[6].LPComb_1.SetDelayMSec(41.41f);
  Data[6].LPComb_2.SetDelayMSec(42.58f);
}

//--------------------------------------------------------------------------------- Complex Number

//************************************************************************************************
ComplexNumber ComplexNumber::operator*(const float constant) const
{
  return ComplexNumber(mReal * constant, mImaginary * constant);
}

//************************************************************************************************
ComplexNumber ComplexNumber::operator*(const ComplexNumber& number) const
{
  return ComplexNumber(mReal * number.mReal - mImaginary * number.mImaginary,
    mReal * number.mImaginary + mImaginary * number.mReal);
}

//************************************************************************************************
ComplexNumber ComplexNumber::operator+(const ComplexNumber& number) const
{
  return ComplexNumber(mReal + number.mReal, mImaginary + number.mImaginary);
}

//************************************************************************************************
ComplexNumber ComplexNumber::operator-(const ComplexNumber& number) const
{
  return ComplexNumber(mReal - number.mReal, mImaginary - number.mImaginary);
}

//************************************************************************************************
ComplexNumber& ComplexNumber::operator+=(const ComplexNumber& number)
{
  mReal += number.mReal;
  mImaginary += number.mImaginary;
  return *this;
}

//************************************************************************************************
void ComplexNumber::Set(float real, float imaginary)
{
  mReal = real;
  mImaginary = imaginary;
}

//************************************************************************************************
float ComplexNumber::Magnitude() const
{
  if (mReal > 0 || mImaginary > 0)
    return Math::Sqrt(MagnitudeSquared());
  else
    return 0.0f;
}

//************************************************************************************************
float ComplexNumber::MagnitudeSquared() const
{
  return mReal * mReal + mImaginary * mImaginary;
}

//------------------------------------------------------------------------- Fast Fourier Transform

//************************************************************************************************
void FFT::Forward(ComplexNumber* samples, const int numberOfSamples)
{
  DoFFT(samples, numberOfSamples, true);
}

//************************************************************************************************
void FFT::Forward(const float* input, ComplexNumber* result, const int numberOfSamples)
{
  for (int i = 0; i < numberOfSamples; ++i)
    result[i].Set(input[i], 0.0f);

  Forward(result, numberOfSamples);
}

//************************************************************************************************
void FFT::Backward(ComplexNumber* samples, const int numberOfSamples)
{
  DoFFT(samples, numberOfSamples, false);

  float scale = 1.0f / numberOfSamples;
  for (int i = 0; i < numberOfSamples; ++i)
  {
    samples[i].mReal *= scale;
    samples[i].mImaginary *= -scale;
  }
}

//************************************************************************************************
void SwapFloats(float& first, float& second)
{
  float temp = first;
  first = second;
  second = temp;
}

//************************************************************************************************
void FFT::DoFFT(ComplexNumber* samples, const int numberOfSamples, const bool forward)
{
  int count(1), numBits(0);
  while (count < numberOfSamples)
  {
    count += count;
    ++numBits;
  }

  static int* reverseTable[32] = { nullptr };
  int* table = reverseTable[numBits];
  if (!table)
  {
    table = new int[numberOfSamples];
    for (int n = 0; n < numberOfSamples; ++n)
    {
      unsigned j = 1;
      unsigned k = 0;
      unsigned m = numberOfSamples >> 1;
      while (m > 0)
      {
        if (n & m)
          k |= j;

        j += j;
        m >>= 1;
      }

      table[n] = k;
    }

#ifdef ENABLE_FFT_TEST
    for (int n = 0; n < numberOfSamples; ++n)
      ErrorIf(table[table[n]] != n);
#endif

    reverseTable[numBits] = table;
  }

  for (int i = 0; i < numberOfSamples; ++i)
  {
    int j = table[i];
    if (i < j)
    {
      SwapFloats(samples[i].mReal, samples[j].mReal);
      SwapFloats(samples[i].mImaginary, samples[j].mImaginary);
    }
  }

  float w0;
  if (forward)
    w0 = -Math::cPi;
  else
    w0 = Math::cPi;
  for (int j = 1; j < numberOfSamples; j += j)
  {
    ComplexNumber wr(Math::Cos(w0), Math::Sin(w0));
    ComplexNumber wd(1.0f, 0.0f);
    int step = j + j;
    for (int m = 0; m < j; ++m)
    {
      for (int i = m; i < numberOfSamples; i += step)
      {
        ComplexNumber temp = wd * samples[i + j];
        samples[i + j] = samples[i] - temp;
        samples[i] = samples[i] + temp;
      }

      wd = wd * wr;
    }

    w0 *= 0.5f;
  }
}

//--------------------------------------------------------------- Fast Fourier Transform Convolver

//************************************************************************************************
int NextPowerOf2(const int& value)
{
  int nextPowerOf2 = 1;
  while (nextPowerOf2 < value)
    nextPowerOf2 *= 2;

  return nextPowerOf2;
}

//************************************************************************************************
FFTConvolver::FFTConvolver() :
  mBlockSize(0),
  mSegmentCount(0),
  mBufferPosition(0),
  mCurrentSegment(0)
{

}

//************************************************************************************************
FFTConvolver::~FFTConvolver()
{
  Reset();
}

//************************************************************************************************
bool FFTConvolver::Initialize(int blockSize, const float* impulseResponse, int irLength)
{
  Reset();

  //if (blockSize == 0)
  //  return false;

  //// Remove zeros at end of impulse response
  //while (irLength > 0 && Math::Abs(impulseResponse[irLength - 1]) < 0.000001f)
  //  --irLength;

  //if (irLength == 0)
  //  return true;

  mBlockSize = NextPowerOf2(blockSize);

  mStoredInputBuffer.Resize(mBlockSize);
  mBufferPosition = 0;

  mSegmentCount = (int)((float)irLength / (float)mBlockSize);
  mCurrentSegment = 0;
  for (int i = 0; i < mSegmentCount; ++i)
    mSegments.PushBack(ComplexListType(mBlockSize));

  for (int i = 0; i < mSegmentCount; ++i)
  {
    int remaining = irLength - (i * mBlockSize);
    int sizeToProcess;
    if (remaining >= mBlockSize)
      sizeToProcess = mBlockSize;
    else
      sizeToProcess = remaining;
    mSegmentsIR.PushBack(ComplexListType(sizeToProcess));
    FFT::Forward(impulseResponse + (i * mBlockSize), mSegmentsIR.Back().Data(), sizeToProcess);
  }

  mConvolvedSamples.Resize(mBlockSize);
  mOverlap.Resize(mBlockSize);

  return true;
}

//************************************************************************************************
void FFTConvolver::ProcessBuffer(const float* input, float* output, int length)
{
  if (mSegmentCount == 0)
  {
    memset(output, 0, sizeof(float) * length);
    return;
  }

  int samplesProcessed = 0;
  while (samplesProcessed < length)
  {
    // Process either the rest of the input or the amount that will fit in the stored input buffer
    int processing = Math::Min(length - samplesProcessed, mBlockSize - mBufferPosition);
    // Copy input samples into the stored buffer
    memcpy(mStoredInputBuffer.Data() + mBufferPosition, input + samplesProcessed,
      sizeof(float) * processing);

    // Forward FFT
    FFT::Forward(mStoredInputBuffer.Data(), mSegments[mCurrentSegment].Data(), mBlockSize);

    // Complex multiplication
    if (mBufferPosition == 0)
    {
      for (int i = 1; i < mSegmentCount; ++i)
      {
        int index = (mCurrentSegment + i) % mSegmentCount;
        for (int j = 0; j < mBlockSize; ++j)
          mConvolvedSamples[j] = mSegmentsIR[i][j] * mSegments[index][j];
      }
    }
    for (int i = 0; i < mBlockSize; ++i)
      mConvolvedSamples[i] += mSegments[mCurrentSegment][i] * mSegmentsIR[0][i];

    // Backward FFT
    FFT::Backward(mConvolvedSamples.Data(), mBlockSize);

    // Add overlap
    for (int i = 0; i < processing; ++i)
      output[samplesProcessed + i] = mConvolvedSamples[mBufferPosition + i].mReal +
      mOverlap[mBufferPosition + i].mReal;

    // If input buffer full, go to next block
    mBufferPosition += processing;
    if (mBufferPosition >= mBlockSize)
    {
      mBufferPosition = 0;

      // Save the overlap
      memcpy(mOverlap.Data(), mConvolvedSamples.Data() + mBlockSize,
        sizeof(ComplexNumber) * mBlockSize);

      // Update current segment
      --mCurrentSegment;
      if (mCurrentSegment < 0)
        mCurrentSegment = mSegmentCount - 1;
    }

    samplesProcessed += processing;
  }
}

//************************************************************************************************
void FFTConvolver::Reset()
{
  mBlockSize = 0;
  mSegmentCount = 0;
  mBufferPosition = 0;
  mCurrentSegment = 0;

  mStoredInputBuffer.Clear();
  mSegments.Clear();
  mSegmentsIR.Clear();
  mConvolvedSamples.Clear();
  mOverlap.Clear();
}

//---------------------------------------------------------------------------------- ADSR envelope

//************************************************************************************************
ADSR::ADSR() :
  mDelayTime(0.0f),
  mAttackTime(0.0f),
  mDecayTime(0.0f),
  mSustainTime(0.0f),
  mSustainLevel(0.0f),
  mReleaseTime(0.0f),
  mCurrentTime(0.0f),
  mCurrentState(DelayState),
  mLastAmplitude(0.0f)
{

}

//************************************************************************************************
ADSR::ADSR(const ADSR& copy) :
  mDelayTime(copy.mDelayTime),
  mAttackTime(copy.mAttackTime),
  mDecayTime(copy.mDecayTime),
  mSustainTime(copy.mSustainTime),
  mSustainLevel(copy.mSustainLevel),
  mReleaseTime(copy.mReleaseTime),
  mCurrentTime(copy.mCurrentTime),
  mCurrentState(copy.mCurrentState),
  mLastAmplitude(copy.mLastAmplitude)
{

}

//************************************************************************************************
void ADSR::SetValues(const EnvelopeSettings* settings)
{
  mDelayTime = settings->mDelayTime;
  mAttackTime = settings->mAttackTime;
  mDecayTime = settings->mDecayTime;
  mSustainTime = settings->mSustainTime;
  mSustainLevel = settings->mSustainLevel;
  if (mSustainLevel > 1.0f)
    mSustainLevel = 1.0f;
  if (mSustainLevel < 0.99f && mDecayTime < 0.02f)
    mDecayTime = 0.02f;
  mReleaseTime = settings->mReleaseTime;
  mCurrentTime = 0.0f;
  mCurrentState = DelayState;
}

//************************************************************************************************
float ADSR::operator()()
{
  float amplitude(0.0f);

  switch (mCurrentState)
  {
  case DelayState:
    amplitude = 0.0f;
    if (mCurrentTime >= mDelayTime)
    {
      mCurrentTime = 0.0f;
      mCurrentState = AttackState;
    }
    break;
  case AttackState:
    amplitude = mCurrentTime / mAttackTime;
    if (amplitude > 1.0f)
      amplitude = 1.0f;
    if (mCurrentTime >= mAttackTime)
    {
      mCurrentTime = 0.0f;
      if (mDecayTime > 0.0f)
        mCurrentState = DecayState;
      else
        mCurrentState = SustainState;
    }
    break;
  case DecayState:
    amplitude = 1.0f - ((1.0f - mSustainLevel) * (mCurrentTime / mDecayTime));
    if (mCurrentTime >= mDecayTime)
    {
      mCurrentTime = 0.0f;
      mCurrentState = SustainState;
    }
    break;
  case SustainState:
    amplitude = mSustainLevel;
    if (mSustainTime > 0.0f && mCurrentTime >= mSustainTime)
    {
      mCurrentTime = 0.0f;
      mCurrentState = ReleaseState;
    }
    break;
  case ReleaseState:
    amplitude = mSustainLevel - (mSustainLevel * (mCurrentTime / mReleaseTime));
    if (mCurrentTime >= mReleaseTime)
      mCurrentState = OffState;
    break;
  case OffState:
    amplitude = 0.0f;
    break;
  default:
    amplitude = 0.0f;
    break;
  }

  mCurrentTime += cSystemTimeIncrement;
  mLastAmplitude = amplitude;

  return amplitude;
}

//************************************************************************************************
void ADSR::Release()
{
  // If currently releasing, do nothing
  if (mCurrentState == ReleaseState)
    return;

  // If current amplitude is not at the sustain level, reset the sustain level so release is smooth
  if (mCurrentState != SustainState)
    mSustainLevel = mLastAmplitude;

  mCurrentTime = 0.0f;
  mCurrentState = ReleaseState;
}

//************************************************************************************************
bool ADSR::IsFinished()
{
  return mCurrentState == OffState;
}

//------------------------------------------------------------------ Frequency Modulation Operator

//************************************************************************************************
FMOperator::FMOperator() :
  mFrequency(0.0f),
  mVolume(0.0f),
  mPitchOffset(0.0f),
  mTime(0.0f)
{

}

//************************************************************************************************
//void FMOperator::SetValues(float frequency, float volume, EnvelopeSettings& envelope)
//{
//  mFrequency = frequency;
//  mVolume = volume;
//  Envelope.SetValues(envelope);
//}

//************************************************************************************************
void FMOperator::SetOffsetPitch(const float cents)
{
  if (cents == 0)
    mPitchOffset = 0.0f;
  else
    mPitchOffset = Math::Pow(2.0f, cents / 1200.0f);
}

//************************************************************************************************
float FMOperator::operator()(const float phi)
{
  float value = (mVolume/* * Envelope()*/)* Math::Sin((Math::cTwoPi * mFrequency * (float)mTime) + phi);

  if (mPitchOffset != 0.0f)
    mTime += cSystemTimeIncrement * mPitchOffset;
  else
    mTime += cSystemTimeIncrement;

  return value;
}

} // namespace Zero
