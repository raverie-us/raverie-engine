///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"
#include "Math/Reals.hpp"

namespace Audio
{
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

    for (unsigned i = 0; i < MaxChannels; ++i)
      BiQuadsPerChannel[i].FlushDelays();
  }

  //************************************************************************************************
  void LowPassFilter::SetCutoffValues()
  {
    // LP coefficients
    float tanValue = Math::cPi * CutoffFrequency / gAudioSystem->SystemSampleRate;
    if (tanValue >= HalfPI)
      tanValue = HalfPI - 0.001f;

    float C = 1.0f / Math::Tan(tanValue);
    float Csq = C * C;

    float alpha = 1.0f / (1.0f + (SqRoot2 * C) + Csq);
    float beta1 = 2.0f * alpha * (1.0f - Csq);
    float beta2 = alpha * (1.0f - (SqRoot2 * C) + Csq);

    for (unsigned i = 0; i < MaxChannels; ++i)
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
    for (int i = 0; i < MaxChannels; ++i)
    {
      BiQuadsPerChannel[i].AddHistoryTo(otherFilter.BiQuadsPerChannel[i]);
    }
  }

  //************************************************************************************************
  void LowPassFilter::ProcessSample(const float* input, float* output, const unsigned numChannels)
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

  //------------------------------------------------------------------------------- High Pass Filter

  //************************************************************************************************
  HighPassFilter::HighPassFilter() : 
    CutoffFrequency(10.0f), 
    HalfPI(Math::cPi / 2.0f), 
    SqRoot2(Math::Sqrt(2.0f))
  {
    SetCutoffValues();

    for (unsigned i = 0; i < MaxChannels; ++i)
      BiQuadsPerChannel[i].FlushDelays();
  }

  //************************************************************************************************
  void HighPassFilter::SetCutoffValues()
  {
    float tanValue = Math::cPi * CutoffFrequency / gAudioSystem->SystemSampleRate;
    if (tanValue >= HalfPI)
      tanValue = HalfPI - 0.001f;

    float C = Math::Tan(tanValue);
    float Csq = C * C;

    float alpha = 1.0f / (1.0f + (SqRoot2 * C) + Csq);
    float beta1 = 2.0f * alpha * (Csq - 1.0f);
    float beta2 = alpha * (1.0f - (SqRoot2 * C) + Csq);

    for (unsigned i = 0; i < MaxChannels; ++i)
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
    for (int i = 0; i < MaxChannels; ++i)
    {
      BiQuadsPerChannel[i].AddHistoryTo(otherFilter.BiQuadsPerChannel[i]);
    }
  }

  //************************************************************************************************
  void HighPassFilter::ProcessSample(const float* input, float* output, const unsigned numChannels)
  {
    if (CutoffFrequency < 20.0f)
      memcpy(output, input, sizeof(float) * numChannels);

    for (unsigned i = 0; i < numChannels; ++i)
      output[i] = BiQuadsPerChannel[i].DoBiQuad(input[i]);
  }

  //------------------------------------------------------------------------------- Band Pass Filter

  //************************************************************************************************
  BandPassFilter::BandPassFilter() :
    Quality(0.669f), 
    CentralFreq(1000.0f)
  {
    ResetFrequencies();

    memset(PreviousInput, 0, sizeof(float) * MaxChannels);
    memset(PreviousOutput1, 0, sizeof(float) * MaxChannels);
    memset(PreviousOutput2, 0, sizeof(float) * MaxChannels);
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
    for (int i = 0; i < MaxChannels; ++i)
    {
      otherFilter.PreviousInput[i] += PreviousInput[i];
      otherFilter.PreviousOutput1[i] += PreviousOutput1[i];
      otherFilter.PreviousOutput2[i] += PreviousOutput2[i];
    }
  }

  //************************************************************************************************
  void BandPassFilter::ProcessSample(const float* input, float* output, const unsigned numChannels)
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

    AlphaLP = gAudioSystem->SystemSampleRate / ((LowPassCutoff * 2.0f * Math::cPi) + 
      gAudioSystem->SystemSampleRate);
    AlphaHP = gAudioSystem->SystemSampleRate / ((HighPassCutoff * 2.0f * Math::cPi) + 
      gAudioSystem->SystemSampleRate);
  }

  //------------------------------------------------------------------------------------- Oscillator

  //************************************************************************************************
  Oscillator::Oscillator() :
    mReadIndex(0),
    mIncrement(0), 
    mNoteOn(false), 
    mSampleRate(gAudioSystem->SystemSampleRate),
    mFrequency(1.0f),
    mType(Noise), 
    mPolarity(Bipolar)
  {
    mIncrement = 1024.0f * mFrequency / mSampleRate;
    SetType(Sine);
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

    if (mType == Noise)
      return RandomObject.FloatRange(-1.0f, 1.0f) * WAVE_VOLUME;

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
    mIncrement = ArraySize * mFrequency / mSampleRate;
  }

  //************************************************************************************************
  void Oscillator::SetType(const Types newType)
  {
    if (newType == mType)
      return;

    mType = newType;

    if (mType == Sine)
    {
      for (int i = 0; i < ArraySize; ++i)
        mWaveValues[i] = 0.99f * Math::Sin(((float)i / (float)ArraySize) * 2.0f * Math::cPi);
    }
    else if (mType == Saw)
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
    else if (mType == Square)
    {
      int halfBuffer = ArraySize / 2;
      for (int i = 0; i < ArraySize; ++i)
      {
        if (i < halfBuffer)
          mWaveValues[i] = 1.0f;
        else
          mWaveValues[i] = -1.0f;
      }
    }
    else if (mType == Triangle)
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

  //------------------------------------------------------------------------------------- Delay Line

  //************************************************************************************************
  DelayLine::DelayLine() : 
    DelayInSamples(100.0f * gAudioSystem->SystemSampleRate / 1000.0f),
    Feedback(0),
    WetLevel(0.5f),
    ReadIndex(0), 
    WriteIndex(0), 
    BufferSize(0),
    MaxDelaySec(2.0f),
    InterpolatingWetLevel(false)
  {
    BufferSize = (int)(MaxDelaySec * gAudioSystem->SystemSampleRate);
    for (unsigned i = 0; i < MaxChannels; ++i)
    {
      BuffersPerChannel[i] = new float[BufferSize];
      memset(BuffersPerChannel[i], 0, sizeof(float) * BufferSize);
    }

    ReadIndex = (int)(BufferSize - DelayInSamples);
  }

  //************************************************************************************************
  DelayLine::~DelayLine()
  {
    for (unsigned i = 0; i < MaxChannels; ++i)
      delete[] BuffersPerChannel[i];
  }

  //************************************************************************************************
  void DelayLine::ProcessBuffer(const float* input, float* output, const unsigned numberOfChannels, 
    const unsigned bufferSize)
  {
    for (unsigned frameIndex = 0; frameIndex < bufferSize; frameIndex += numberOfChannels)
    {
      for (unsigned channel = 0; channel < numberOfChannels && channel < 
        gAudioSystem->SystemChannelsThreaded; ++channel)
      {
        // Read input for this channel
        float inputSample = input[frameIndex + channel];
        float delayedSample;
        // If no delay, delayed sample is just input
        if (DelayInSamples == 0)
          delayedSample = inputSample;
        // Otherwise, get interpolated delayed sample 
        else
        {
          // If delay is less than 1 sample, interpolate between input(n) and input(n-1)
          if (ReadIndex == WriteIndex && DelayInSamples < 1.0f)
            delayedSample = inputSample;
          // Otherwise, get delayed sample from buffer
          else
            delayedSample = BuffersPerChannel[channel][ReadIndex];

          // Read previous delayed sample
          int readIndex_1 = ReadIndex - 1;
          if (readIndex_1 < 0)
            readIndex_1 = BufferSize - 1;
          float delayedSample_1 = BuffersPerChannel[channel][readIndex_1];

           delayedSample = delayedSample_1 + ((DelayInSamples - (int)DelayInSamples) * 
             (delayedSample - delayedSample_1));
        }

        // Write input to delay buffer
        BuffersPerChannel[channel][WriteIndex] = inputSample + (Feedback * delayedSample);

        // Check if we are interpolating the wet percent
        if (InterpolatingWetLevel)
        {
          WetLevel = WetLevelInterpolator.NextValue();
          InterpolatingWetLevel = !WetLevelInterpolator.Finished();
        }

        // Write wet/dry mix to output buffer
        output[frameIndex + channel] = (WetLevel * delayedSample) + ((1.0f - WetLevel) * inputSample);
      }

      // Done with this frame, increment indexes
      ++WriteIndex;
      if (WriteIndex == BufferSize)
        WriteIndex = 0;
      ++ReadIndex;
      if (ReadIndex == BufferSize)
        ReadIndex = 0;
    }
  }

  //************************************************************************************************
  void DelayLine::SetDelayMSec(float delay)
  {
    if (delay > MaxDelaySec * 1000)
      delay = MaxDelaySec * 1000;

    DelayInSamples = delay * gAudioSystem->SystemSampleRate / 1000.0f;

    ReadIndex = WriteIndex - (int)DelayInSamples;
    while (ReadIndex < 0)
      ReadIndex += BufferSize;
  }

  //************************************************************************************************
  float DelayLine::GetDelayMSec()
  {
    return DelayInSamples / gAudioSystem->SystemSampleRate * 1000.0f;
  }

  //************************************************************************************************
  void DelayLine::SetFeedbackPct(const float feedbackPct)
  {
    Feedback = feedbackPct / 100.0f;

  }

  //************************************************************************************************
  void DelayLine::SetWetLevelPct(const float wetPct)
  {
    WetLevel = wetPct / 100.0f;

  }

  //************************************************************************************************
  void DelayLine::InterpolateWetLevelPct(const float percent, const float time)
  {
    InterpolatingWetLevel = true;
    WetLevelInterpolator.SetValues(WetLevel, percent / 100.0f, (unsigned)(time * gAudioSystem->SystemSampleRate));
  }

  //------------------------------------------------------------------------------ Envelope Detector

  //************************************************************************************************
  EnvelopeDetector::EnvelopeDetector() :
    mAttackTimeMSec(0.0f),
    mReleaseTimeMSec(0.0f), 
    mAttackTime(0.0f),
    mReleaseTime(0.0f),
    mSampleRate((float)gAudioSystem->SystemSampleRate), 
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

    for (unsigned i = 0; i < MaxChannels; ++i)
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

    for (unsigned i = 0; i < MaxChannels; ++i)
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
  float DynamicsProcessor::CompressorGain(const float value)
  {
    float slope;

    if (mProcessorType == Limiter)
      slope = 1.0f;
    else
      slope = mCompressorRatio;

    // Soft-knee with detection value in range?
    if (mKneeWidth > 0 && value > (mThresholdDB - mHalfKnee)
      && value < (mThresholdDB + mHalfKnee))
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

      slope = (float)LagrangeInterpolation(x, y, 2, value);
    }

    float gain = slope * (mThresholdDB - value);

    if (gain > 0)
      gain = 0;

    return Math::Pow(10.0f, gain / 20.0f);
  }

  //************************************************************************************************
  float DynamicsProcessor::ExpanderGain(const float value)
  {
    float slope;

    if (mProcessorType == Gate)
      slope = -1.0f;
    else
      slope = mExpanderRatio;

    // Soft-knee with detection value in range?
    if (mKneeWidth > 0 && value > (mThresholdDB - mHalfKnee)
      && value < (mThresholdDB + mHalfKnee))
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

      slope = (float)LagrangeInterpolation(x, y, 2, value);
    }

    float gain = slope * (mThresholdDB - value);

    if (gain > 0)
      gain = 0;

    return Math::Pow(10.0f, gain / 20.0f);
  }

  //************************************************************************************************
  void DynamicsProcessor::SetAttackMSec(const float attack)
  {
    mAttackMSec = attack;

    for (unsigned i = 0; i < MaxChannels; ++i)
      Detectors[i].SetAttackTime(attack);
  }

  //************************************************************************************************
  void DynamicsProcessor::SetReleaseMsec(const float release)
  {
    mReleaseMSec = release;

    for (unsigned i = 0; i < MaxChannels; ++i)
      Detectors[i].SetReleaseTime(release);
  }

  //************************************************************************************************
  void DynamicsProcessor::SetRatio(const float ratio)
  {
    mRatio = ratio;
    mCompressorRatio = 1.0f / ratio;
    mExpanderRatio = mCompressorRatio - 1.0f;
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
  Equalizer::Equalizer() :
    mLowPassGain(1.0f),
    mHighPassGain(1.0f),
    mBand1Gain(1.0f),
    mBand2Gain(1.0f),
    mBand3Gain(1.0f)
  {
    SetFilterData();
  }

  //************************************************************************************************
  Equalizer::Equalizer(const float below80Hz, const float at150Hz, const float at600Hz, 
      const float at2500Hz, const float above5000Hz) :
    mLowPassGain(below80Hz),
    mHighPassGain(at150Hz),
    mBand1Gain(at600Hz),
    mBand2Gain(at2500Hz),
    mBand3Gain(above5000Hz)
  {
    SetFilterData();
  }

  //************************************************************************************************
  void Equalizer::ProcessBuffer(const float* input, float* output, const unsigned numChannels, 
    const unsigned bufferSize)
  {
    Zero::Array<float> resultSamples(numChannels);

    for (unsigned i = 0; i < bufferSize; i += numChannels)
    {
      LowPass.ProcessSample(input + i, resultSamples.Data(), numChannels);
      if (!LowPassInterpolator.Finished())
        mLowPassGain = LowPassInterpolator.NextValue();
      for (unsigned j = 0; j < numChannels; ++j)
        output[i + j] = resultSamples[j] * mLowPassGain;

      Band1.ProcessSample(input + i, resultSamples.Data(), numChannels);
      if (!Band1Interpolator.Finished())
        mBand1Gain = Band1Interpolator.NextValue();
      for (unsigned j = 0; j < numChannels; ++j)
        output[i + j] += resultSamples[j] * mBand1Gain;

      Band2.ProcessSample(input + i, resultSamples.Data(), numChannels);
      if (!Band2Interpolator.Finished())
        mBand2Gain = Band2Interpolator.NextValue();
      for (unsigned j = 0; j < numChannels; ++j)
        output[i + j] += resultSamples[j] * mBand2Gain;

      Band3.ProcessSample(input + i, resultSamples.Data(), numChannels);
      if (!Band3Interpolator.Finished())
        mBand3Gain = Band3Interpolator.NextValue();
      for (unsigned j = 0; j < numChannels; ++j)
        output[i + j] += resultSamples[j] * mBand3Gain;

      HighPass.ProcessSample(input + i, resultSamples.Data(), numChannels);
      if (!HighPassInterpolator.Finished())
        mHighPassGain = HighPassInterpolator.NextValue();
      for (unsigned j = 0; j < numChannels; ++j)
        output[i + j] += resultSamples[j] * mHighPassGain;
    }
  }

  //************************************************************************************************
  float Equalizer::GetBelow80HzGain()
  {
    return mLowPassGain;
  }

  //************************************************************************************************
  void Equalizer::SetBelow80HzGain(const float gain)
  {
    mLowPassGain = gain;
  }

  //************************************************************************************************
  float Equalizer::Get150HzGain()
  {
    return mBand1Gain;
  }

  //************************************************************************************************
  void Equalizer::Set150HzGain(const float gain)
  {
    mBand1Gain = gain;
  }

  //************************************************************************************************
  float Equalizer::Get600HzGain()
  {
    return mBand2Gain;
  }

  //************************************************************************************************
  void Equalizer::Set600HzGain(const float gain)
  {
    mBand2Gain = gain;
  }

  //************************************************************************************************
  float Equalizer::Get2500HzGain()
  {
    return mBand3Gain;
  }

  //************************************************************************************************
  void Equalizer::Set2500HzGain(const float gain)
  {
    mBand3Gain = gain;
  }

  //************************************************************************************************
  float Equalizer::GetAbove5000HzGain()
  {
    return mHighPassGain;
  }

  //************************************************************************************************
  void Equalizer::SetAbove5000HzGain(const float gain)
  {
    mHighPassGain = gain;
  }

  //************************************************************************************************
  void Equalizer::InterpolateBands(const float below80Hz, const float at150Hz, const float at600Hz, 
    const float at2500Hz, const float above5000Hz, const float timeToInterpolate)
  {
    LowPassInterpolator.SetValues(mLowPassGain, below80Hz, (unsigned)(timeToInterpolate * 
      gAudioSystem->SystemSampleRate));
    Band1Interpolator.SetValues(mBand1Gain, at150Hz, (unsigned)(timeToInterpolate * 
      gAudioSystem->SystemSampleRate));
    Band2Interpolator.SetValues(mBand2Gain, at600Hz, (unsigned)(timeToInterpolate *
      gAudioSystem->SystemSampleRate));
    Band3Interpolator.SetValues(mBand3Gain, at2500Hz, (unsigned)(timeToInterpolate * 
      gAudioSystem->SystemSampleRate));
    HighPassInterpolator.SetValues(mHighPassGain, above5000Hz, (unsigned)(timeToInterpolate * 
      gAudioSystem->SystemSampleRate));
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
    PreDelay(0.5f, gAudioSystem->SystemSampleRate),
    InputAP_1(0.5f, gAudioSystem->SystemSampleRate),
    InputAP_2(0.5f, gAudioSystem->SystemSampleRate),
    Comb_1(0.5f, gAudioSystem->SystemSampleRate),
    Comb_2(0.5f, gAudioSystem->SystemSampleRate),
    LPComb_1(0.5f, gAudioSystem->SystemSampleRate),
    LPComb_2(0.5f, gAudioSystem->SystemSampleRate),
    OutputAP(0.5f, gAudioSystem->SystemSampleRate)
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
  void Reverb::SetWetPercent(const float newPercent)
  {
    WetValue = newPercent / 100.0f;
  }

  //************************************************************************************************
  void Reverb::InterpolateWetPercent(const float newPercent, const float time)
  {
    WetValueInterpolator.SetValues(WetValue, newPercent / 100.0f, (unsigned)(time * 
      gAudioSystem->SystemSampleRate));
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
    mTimeDelta(1.0f / gAudioSystem->SystemSampleRate),
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
    mTimeDelta(copy.mTimeDelta),
    mCurrentState(copy.mCurrentState),
    mLastAmplitude(copy.mLastAmplitude)
  {

  }

  //************************************************************************************************
  void ADSR::SetValues(const EnvelopeSettings* settings)
  {
    mDelayTime = settings->DelayTime;
    mAttackTime = settings->AttackTime;
    mDecayTime = settings->DecayTime;
    mSustainTime = settings->SustainTime;
    mSustainLevel = settings->SustainLevel;
    if (mSustainLevel > 1.0f)
      mSustainLevel = 1.0f;
    if (mSustainLevel < 0.99f && mDecayTime < 0.02f)
      mDecayTime = 0.02f;
    mReleaseTime = settings->ReleaseTime;
    mCurrentTime = 0.0f;
    mCurrentState = DelayState;
  }

  //************************************************************************************************
  void ADSR::SetSampleRate(const unsigned rate)
  {
    mTimeDelta = 1.0f / rate;
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

    mCurrentTime += mTimeDelta;
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
    mTime(0.0f),
    mTimeDelta(1.0f / gAudioSystem->SystemSampleRate)
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
    float value = (mVolume/* * Envelope()*/) * Math::Sin((Math::cTwoPi * mFrequency * (float)mTime) + phi);

    if (mPitchOffset != 0.0f)
      mTime += mTimeDelta * mPitchOffset;
    else
      mTime += mTimeDelta;

    return value;
  }

}