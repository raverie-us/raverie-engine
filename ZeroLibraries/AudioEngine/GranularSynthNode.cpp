///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //---------------------------------------------------------------------------- Linear Grain Window

  //************************************************************************************************
  LinearGrainWindow::LinearGrainWindow(unsigned length) : 
    GrainWindow(length, GrainWindowTypes::Linear), 
    mHalfLength(length / 2) 
  {
  
  }

  //************************************************************************************************
  float LinearGrainWindow::GetNextValue()
  {
    if (mCounter < mHalfLength)
      return (float)mCounter / (float)mHalfLength;
    else
      return 2.0f * (1.0f - ((float)mCounter / (float)mTotalLength));
  }

  //************************************************************************************************
  void LinearGrainWindow::Reset(unsigned length, unsigned attack, unsigned release)
  {
    mCounter = 0;
    mTotalLength = length;
    mHalfLength = length / 2;
  }

  //************************************************************************************************
  void LinearGrainWindow::CopySettings(GrainWindow* other)
  {
    mCounter = other->mCounter;
    mTotalLength = other->mTotalLength;
    mHalfLength = ((LinearGrainWindow*)other)->mHalfLength;
  }

  //--------------------------------------------------------------------- Raised Cosine Grain Window

  //************************************************************************************************
  RaisedCosineGrainWindow::RaisedCosineGrainWindow(unsigned length, unsigned attackLength, 
      unsigned releaseLength) :
    GrainWindow(length, GrainWindowTypes::RaisedCosine)
  {
    Reset(length, attackLength, releaseLength);
  }

  //************************************************************************************************
  float RaisedCosineGrainWindow::GetNextValue()
  {
    ++mCounter;
    float result;

    switch (mCurrentState)
    {
    case Attack:
      result = b1 * y1 - y2;
      y2 = y1;
      y1 = result;

      result += 1.0f;
      result *= 0.5f;

      if (mCounter == mAttackLength)
        mCurrentState = Sustain;

      break;
    case Sustain:
      result = 1.0f;

      if (mCounter == mTotalLength - mReleaseLength)
      {
        mCurrentState = Release;

        float w = Math::cPi / mReleaseLength;
        float ip = Math::cPi * 0.5f;
        b1 = 2.0f * Math::Cos(w);
        y1 = Math::Sin(ip - w);
        y2 = Math::Sin(ip - 2.0f * w);
      }

      break;
    case Release:
      if (mCounter >= mTotalLength)
        return 0.0f;

      result = b1 * y1 - y2;
      y2 = y1;
      y1 = result;

      result += 1.0f;
      result *= 0.5f;

      break;
    }

    return result;
  }

  //************************************************************************************************
  void RaisedCosineGrainWindow::Reset(unsigned length, unsigned attack, unsigned release)
  {
    mCounter = 0;
    mCurrentState = Attack;
    mTotalLength = length;
    mAttackLength = attack;
    mReleaseLength = release;

    if (mAttackLength + mReleaseLength > mTotalLength)
    {
      float adjustment = (float)mTotalLength / (float)(mAttackLength + mReleaseLength);
      mAttackLength = (unsigned)(mAttackLength * adjustment);
      mReleaseLength  = (unsigned)(mReleaseLength * adjustment);
    }

    float w = Math::cPi / mAttackLength;
    float ip = -Math::cPi * 0.5f;
    b1 = 2.0f * Math::Cos(w);
    y1 = Math::Sin(ip - w);
    y2 = Math::Sin(ip - 2.0f * w);
  }

  //************************************************************************************************
  void RaisedCosineGrainWindow::CopySettings(GrainWindow* other)
  {
    RaisedCosineGrainWindow* otherWindow = (RaisedCosineGrainWindow*)other;

    mCounter = otherWindow->mCounter;
    mTotalLength = otherWindow->mTotalLength;
    mCurrentState = otherWindow->mCurrentState;
    mAttackLength = otherWindow->mAttackLength;
    mReleaseLength = otherWindow->mReleaseLength;
    b1 = otherWindow->b1;
    y1 = otherWindow->y1;
    y2 = otherWindow->y2;
  }

  //------------------------------------------------------------------------------- Parabolic Window

  //************************************************************************************************
  ParabolicGrainWindow::ParabolicGrainWindow(unsigned length) :
    GrainWindow(length, GrainWindowTypes::Parabolic),
    mLastAmplitude(0.0f),
    mSlope(0.0f),
    mCurve(0.0f)
  {
    Reset(length, 0, 0);
  }

  //************************************************************************************************
  float ParabolicGrainWindow::GetNextValue()
  {
    ++mCounter;

    mLastAmplitude = mLastAmplitude + mSlope;
    mSlope = mSlope + mCurve;

    float returnValue = Math::Min(mLastAmplitude, 1.0f);
    returnValue = Math::Max(returnValue, 0.0f);

    return mLastAmplitude;
  }

  //************************************************************************************************
  void ParabolicGrainWindow::Reset(unsigned length, unsigned attack, unsigned release)
  {
    mLastAmplitude = 0.0f;
    float rdur = 1.0f / length;
    float rdurSq = rdur * rdur;
    mSlope = 4.0f * (rdur - rdurSq);
    mCurve = -8.0f * rdurSq;
  }

  //************************************************************************************************
  void ParabolicGrainWindow::CopySettings(GrainWindow* other)
  {
    ParabolicGrainWindow* otherWindow = (ParabolicGrainWindow*)other;

    mCounter = otherWindow->mCounter;
    mTotalLength = otherWindow->mTotalLength;
    mLastAmplitude = otherWindow->mLastAmplitude;
    mSlope = otherWindow->mSlope;
    mCurve = otherWindow->mCurve;
  }

  //------------------------------------------------------------------------------- Trapezoid Window

  //************************************************************************************************
  TrapezoidGrainWindow::TrapezoidGrainWindow(unsigned length, unsigned attack, unsigned release) :
    GrainWindow(length, GrainWindowTypes::Trapezoid)
  {
    Reset(length, attack, release);
  }

  //************************************************************************************************
  float TrapezoidGrainWindow::GetNextValue()
  {
    ++mCounter;

    switch (mCurrentState)
    {
    case Attack:
      if (mCounter >= mAttackLength)
      {
        mCurrentState = Sustain;
        mLastAmplitude = 1.0f;
        mIncrement = 0.0f;
      }
      break;
    case Sustain:
      if (mCounter >= mTotalLength - mReleaseLength)
      {
        mCurrentState = Release;
        mIncrement = -1.0f / mReleaseLength;
      }
      break;
    case Release:
      break;
    }

    mLastAmplitude += mIncrement;
    return mLastAmplitude;
  }

  //************************************************************************************************
  void TrapezoidGrainWindow::Reset(unsigned length, unsigned attack, unsigned release)
  {
    mCounter = 0;
    mTotalLength = length;
    mLastAmplitude = 0.0f;

    mAttackLength = attack;
    mReleaseLength = release;

    if (mAttackLength + mReleaseLength > mTotalLength)
    {
      float adjustment = (float)mTotalLength / (float)(mAttackLength + mReleaseLength);
      mAttackLength = (unsigned)(mAttackLength * adjustment);
      mReleaseLength = (unsigned)(mReleaseLength * adjustment);
    }

    mIncrement = 1.0f / mAttackLength;
    mCurrentState = Attack;
  }

  //************************************************************************************************
  void TrapezoidGrainWindow::CopySettings(GrainWindow* other)
  {
    TrapezoidGrainWindow* otherWindow = (TrapezoidGrainWindow*)other;

    mCounter = otherWindow->mCounter;
    mTotalLength = otherWindow->mTotalLength;
    mAttackLength = otherWindow->mAttackLength;
    mReleaseLength = otherWindow->mReleaseLength;
    mLastAmplitude = otherWindow->mLastAmplitude;
    mIncrement = otherWindow->mIncrement;
    mCurrentState = otherWindow->mCurrentState;
  }

  //------------------------------------------------------------------------------------------ Grain

  //************************************************************************************************
  Grain::Grain() :
    mActive(false),
    mCounter(0),
    mLength(0),
    mCurrentFrameIndex(0.0),
    mIndexIncrement(1.0f),
    mLeftVolume(1.0f),
    mRightVolume(1.0f),
    mVolume(1.0f),
    mSourceSamples(nullptr),
    mSourceChannels(0),
    mWindow(nullptr)
  {

  }

  //************************************************************************************************
  void Grain::Activate(unsigned length, float volume, float panning, BufferType* sampleBuffer, 
    unsigned channels, unsigned currentIndex, float indexIncrement, GrainWindowTypes::Enum windowType, 
    unsigned windowAttack, unsigned windowRelease)
  {
    mActive = true;
    mCounter = 0;
    mLength = length;
    mCurrentFrameIndex = currentIndex / channels;
    mIndexIncrement = indexIncrement;
    if (mIndexIncrement == 0.0f)
      mIndexIncrement = 0.001f;
    mSourceSamples = sampleBuffer;
    mSourceChannels = channels;
    mVolume = volume;

    if (panning == 0.0f)
    {
      mLeftVolume = 0.5f;
      mRightVolume = 0.5f;
    }
    else if (panning < 0.0f)
    {
      panning = Math::Max(panning, -1.0f);

      mLeftVolume = 0.5f * -panning + 0.5f;
      mRightVolume = 1.0f - mLeftVolume;
    }
    else
    {
      panning = Math::Min(panning, 1.0f);

      mRightVolume = 0.5f * panning + 0.5f;
      mLeftVolume = 1.0f - mRightVolume;
    }

    if (mWindow)
    {
      if (mWindow->mType == windowType)
        mWindow->Reset(length, windowAttack, windowRelease);
      else
      {
        delete mWindow;
        mWindow = nullptr;
      }
    }

    if (!mWindow)
    {
      if (windowType == GrainWindowTypes::RaisedCosine)
        mWindow = new RaisedCosineGrainWindow(length, windowAttack, windowRelease);
      else if (windowType == GrainWindowTypes::Parabolic)
        mWindow = new ParabolicGrainWindow(length);
      else if (windowType == GrainWindowTypes::Trapezoid)
        mWindow = new TrapezoidGrainWindow(length, windowAttack, windowRelease);
      else
        mWindow = new LinearGrainWindow(length);
    }
  }

  //************************************************************************************************
  void Grain::GetSamples(float* outputBuffer, unsigned outputFrames, unsigned outputChannels)
  {
    if (!mActive)
      return;

    for (unsigned frame = 0; frame < outputFrames && mCounter < mLength; ++frame, ++mCounter)
    {
      float sample = 0.0f;
      unsigned sampleIndex = (int)mCurrentFrameIndex * mSourceChannels;
      for (unsigned i = 0; i < mSourceChannels; ++i)
      {
        if (mIndexIncrement == 1.0f)
          sample += (*mSourceSamples)[sampleIndex];
        else
        {
          float firstSample = (*mSourceSamples)[sampleIndex + i];
          float secondSample = (*mSourceSamples)[sampleIndex + i + mSourceChannels];

          sample += firstSample + ((secondSample - firstSample) * (float)(mCurrentFrameIndex -
            (int)mCurrentFrameIndex));
        }
      }

      sample /= (float)mSourceChannels;

      mCurrentFrameIndex += mIndexIncrement;
      if (mCurrentFrameIndex >= mSourceSamples->Size() - mSourceChannels)
        mCurrentFrameIndex = 0;
      else if (mCurrentFrameIndex < 0)
        mCurrentFrameIndex = mSourceSamples->Size();

      sample *= mWindow->GetNextValue() * mVolume;

      if (outputChannels == 1)
        outputBuffer[frame * outputChannels] += sample;
      else if (outputChannels == 2)
      {
        outputBuffer[frame * outputChannels] += sample * mLeftVolume;
        outputBuffer[frame * outputChannels + 1] += sample * mRightVolume;
      }
      else
      {
        float samples[2] = { sample * mLeftVolume, sample * mRightVolume };
        AudioFrame frameSamples(samples, 2);
        memcpy(outputBuffer + (frame * outputChannels), frameSamples.GetSamples(outputChannels), 
          sizeof(float) * outputChannels);
      }
    }

    if (mCounter >= mLength)
      mActive = false;
  }

  //************************************************************************************************
  Grain& Grain::operator=(const Grain& otherGrain)
  {
    if (&otherGrain == this)
      return *this;

    mActive = otherGrain.mActive;
    mCounter = otherGrain.mCounter;
    mLength = otherGrain.mLength;
    mCurrentFrameIndex = otherGrain.mCurrentFrameIndex;
    mIndexIncrement = otherGrain.mIndexIncrement;
    mLeftVolume = otherGrain.mLeftVolume;
    mRightVolume = otherGrain.mRightVolume;
    mSourceSamples = otherGrain.mSourceSamples;
    mSourceChannels = otherGrain.mSourceChannels;
    mWindow->CopySettings(otherGrain.mWindow);

    return *this;
  }

  //---------------------------------------------------------------------------- Granular Synth Node

  //************************************************************************************************
  GranularSynthNode::GranularSynthNode(Zero::StringParam name, const unsigned ID, 
      ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(name, ID, extInt, false, true, isThreaded),
    mActive(false),
    mSampleChannels(0),
    mFirstInactiveGrainIndex(0),
    mGrainStartIndex(0),
    mFramesToNextGrain(0),
    mGrainVolume(1.0f),
    mGrainVolumeVariance(0.0f),
    mGrainDelayFrames(4800),
    mGrainDelayVariance(0),
    mGrainLengthFrames(14400),
    mGrainLengthVariance(0),
    mGrainResampleRate(1.0f),
    mGrainResampleVariance(0.0f),
    mBufferScanRate(1.0f),
    mGrainPanningValue(0.0f),
    mGrainPanningVariance(0.0f),
    mRandomLocationValue(0.0f),
    mWindowType(GrainWindowTypes::Parabolic),
    mWindowAttackFrames(200),
    mWindowReleaseFrames(200)
  {
    if (!Threaded)
      SetSiblingNodes(new GranularSynthNode(name, ID, nullptr, true));
    else
      GrainList.Resize(16);
  }

  //************************************************************************************************
  void GranularSynthNode::Play()
  {
    if (!Threaded)
      AddTaskForSibling(&GranularSynthNode::Play);

    mActive = true;
  }

  //************************************************************************************************
  void GranularSynthNode::Stop()
  {
    if (!Threaded)
      AddTaskForSibling(&GranularSynthNode::Stop);

    mActive = false;
  }

  //************************************************************************************************
  void GranularSynthNode::SetAsset(SoundAsset* asset, float startTime, float stopTime)
  {
    if (!Threaded)
    {
      AddTaskForSibling(&GranularSynthNode::SetAsset, asset->ThreadedAsset, startTime, stopTime);
    }
    else
    {
      // Get the number of channels from the asset
      mSampleChannels = asset->GetChannels();

      // Translate the start and stop times to frame numbers
      int start = Math::Max((int)(startTime * SystemSampleRate), 0);
      int stop = Math::Min((int)(stopTime * SystemSampleRate),
        (int)asset->GetNumberOfFrames());
      if (stop == 0)
        stop = (int)asset->GetNumberOfFrames();

      // Get the audio samples from the asset
      Samples.Clear();
      asset->AppendSamples(&Samples, start, (stop - start) * mSampleChannels);

      ValidateLengths();
    }
  }

  //************************************************************************************************
  float GranularSynthNode::GetGrainVolume()
  {
    return mGrainVolume;
  }

  //************************************************************************************************
  void GranularSynthNode::SetGrainVolume(float volume)
  {
    if (!Threaded)
    {
      volume = Math::Max(volume, 0.0f);

      AddTaskForSibling(&GranularSynthNode::SetGrainVolume, volume);
    }

    mGrainVolume = volume;
  }

  //************************************************************************************************
  float GranularSynthNode::GetGrainVolumeVariance()
  {
    return mGrainVolumeVariance;
  }

  //************************************************************************************************
  void GranularSynthNode::SetGrainVolumeVariance(float variance)
  {
    if (!Threaded)
      AddTaskForSibling(&GranularSynthNode::SetGrainVolumeVariance, variance);

    mGrainVolumeVariance = variance;
  }

  //************************************************************************************************
  int GranularSynthNode::GetGrainDelay()
  {
    return FramesToMs(mGrainDelayFrames);
  }

  //************************************************************************************************
  void GranularSynthNode::SetGrainDelay(int delayMS)
  {
    if (!Threaded)
    {
      delayMS = Math::Max(delayMS, 0);

      AddTaskForSibling(&GranularSynthNode::SetGrainDelay, delayMS);
    }

    mGrainDelayFrames = MsToFrames(delayMS);
  }

  //************************************************************************************************
  int GranularSynthNode::GetGrainDelayVariance()
  {
    return FramesToMs(mGrainDelayVariance);
  }

  //************************************************************************************************
  void GranularSynthNode::SetGrainDelayVariance(int delayVarianceMS)
  {
    if (!Threaded)
      AddTaskForSibling(&GranularSynthNode::SetGrainDelayVariance, delayVarianceMS);

    mGrainDelayVariance = MsToFrames(delayVarianceMS);
  }

  //************************************************************************************************
  int GranularSynthNode::GetGrainLength()
  {
    return FramesToMs(mGrainLengthFrames);
  }

  //************************************************************************************************
  void GranularSynthNode::SetGrainLength(int lengthMS)
  {
    if (!Threaded)
    {
      lengthMS = Math::Max(lengthMS, 0);

      AddTaskForSibling(&GranularSynthNode::SetGrainLength, lengthMS);
    }

    mGrainLengthFrames = MsToFrames(lengthMS);
    ValidateLengths();
  }

  //************************************************************************************************
  int GranularSynthNode::GetGrainLengthVariance()
  {
    return FramesToMs(mGrainLengthVariance);
  }

  //************************************************************************************************
  void GranularSynthNode::SetGrainLengthVariance(int lengthVarianceMS)
  {
    if (!Threaded)
      AddTaskForSibling(&GranularSynthNode::SetGrainLengthVariance, lengthVarianceMS);

    mGrainLengthVariance = MsToFrames(lengthVarianceMS);
  }

  //************************************************************************************************
  float GranularSynthNode::GetGrainResampleRate()
  {
    return mGrainResampleRate;
  }

  //************************************************************************************************
  void GranularSynthNode::SetGrainResampleRate(float resampleRate)
  {
    if (!Threaded)
      AddTaskForSibling(&GranularSynthNode::SetGrainResampleRate, resampleRate);

    mGrainResampleRate = resampleRate;
  }

  //************************************************************************************************
  float GranularSynthNode::GetGrainResampleRateVariance()
  {
    return mGrainResampleVariance;
  }

  //************************************************************************************************
  void GranularSynthNode::SetGrainResampleRateVariance(float resampleVariance)
  {
    if (!Threaded)
      AddTaskForSibling(&GranularSynthNode::SetGrainResampleRateVariance, resampleVariance);

    mGrainResampleVariance = resampleVariance;
  }

  //************************************************************************************************
  float GranularSynthNode::GetBufferScanRate()
  {
    return mBufferScanRate;
  }

  //************************************************************************************************
  void GranularSynthNode::SetBufferScanRate(float bufferRate)
  {
    if (!Threaded)
      AddTaskForSibling(&GranularSynthNode::SetBufferScanRate, bufferRate);

    mBufferScanRate = bufferRate;
  }

  //************************************************************************************************
  float GranularSynthNode::GetGrainPanningValue()
  {
    return mGrainPanningValue;
  }

  //************************************************************************************************
  void GranularSynthNode::SetGrainPanningValue(float panValue)
  {
    if (!Threaded)
      AddTaskForSibling(&GranularSynthNode::SetGrainPanningValue, panValue);

    mGrainPanningValue = panValue;
  }

  //************************************************************************************************
  float GranularSynthNode::GetGrainPanningVariance()
  {
    return mGrainPanningVariance;
  }

  //************************************************************************************************
  void GranularSynthNode::SetGrainPanningVariance(float panValueVariance)
  {
    if (!Threaded)
      AddTaskForSibling(&GranularSynthNode::SetGrainPanningVariance, panValueVariance);

    mGrainPanningVariance = panValueVariance;
  }

  //************************************************************************************************
  float GranularSynthNode::GetRandomLocationValue()
  {
    return mRandomLocationValue;
  }

  //************************************************************************************************
  void GranularSynthNode::SetRandomLocationValue(float randomLocationValue)
  {
    if (!Threaded)
      AddTaskForSibling(&GranularSynthNode::SetRandomLocationValue, randomLocationValue);
    
    mRandomLocationValue = randomLocationValue;
  }

  //************************************************************************************************
  Audio::GrainWindowTypes::Enum GranularSynthNode::GetWindowType()
  {
    return mWindowType;
  }

  //************************************************************************************************
  void GranularSynthNode::SetWindowType(GrainWindowTypes::Enum type)
  {
    if (!Threaded)
      AddTaskForSibling(&GranularSynthNode::SetWindowType, type);

    mWindowType = type;
  }

  //************************************************************************************************
  int GranularSynthNode::GetWindowAttack()
  {
    return FramesToMs(mWindowAttackFrames);
  }

  //************************************************************************************************
  void GranularSynthNode::SetWindowAttack(int attackMS)
  {
    if (!Threaded)
    {
      attackMS = Math::Max(attackMS, 0);

      AddTaskForSibling(&GranularSynthNode::SetWindowAttack, attackMS);
    }

    mWindowAttackFrames = MsToFrames(attackMS);
  }

  //************************************************************************************************
  int GranularSynthNode::GetWindowRelease()
  {
    return FramesToMs(mWindowReleaseFrames);
  }

  //************************************************************************************************
  void GranularSynthNode::SetWindowRelease(int releaseMS)
  {
    if (!Threaded)
    {
      releaseMS = Math::Max(releaseMS, 0);

      AddTaskForSibling(&GranularSynthNode::SetWindowRelease, releaseMS);
    }

    mWindowReleaseFrames = MsToFrames(releaseMS);
  }

  //************************************************************************************************
  bool GranularSynthNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
    ListenerNode* listener, const bool firstRequest)
  {
    if (Samples.Empty())
      return false;

    if (!mActive && mFirstInactiveGrainIndex == 0)
      return false;

    memset(outputBuffer->Data(), 0, outputBuffer->Size() * sizeof(float));
    unsigned frames = outputBuffer->Size() / numberOfChannels;

    // Process existing grains
    for (int i = 0; i < mFirstInactiveGrainIndex; ++i)
    {
      GrainList[i].GetSamples(outputBuffer->Data(), frames, numberOfChannels);

      if (!GrainList[i].mActive)
      {
        --mFirstInactiveGrainIndex;
        GrainList[i] = GrainList[mFirstInactiveGrainIndex];
        GrainList[mFirstInactiveGrainIndex].mActive = false;
        --i;
      }
    }

    if (!mActive)
      return true;

    // Add new grains
    unsigned currentIndex = mFramesToNextGrain * mSampleChannels;
    while (currentIndex < outputBuffer->Size())
    {
      // Add another grain to the list if necessary
      if (mFirstInactiveGrainIndex == GrainList.Size())
        GrainList.PushBack();

      Grain& newGrain = GrainList[mFirstInactiveGrainIndex++];

      int grainIndex;
      // If randomizing grain position, get a random starting index
      if (mRandomLocationValue > 0.0f && RandomObject.Float() < mRandomLocationValue)
      {
        grainIndex = RandomObject.IntRangeInEx(0, Samples.Size());
        grainIndex -= grainIndex % mSampleChannels;
      }
      // Otherwise use the sequential grain start index
      else
      {
        grainIndex = mGrainStartIndex + currentIndex;

        if (grainIndex < 0)
          grainIndex += Samples.Size();
        else if (grainIndex >= (int)Samples.Size())
          grainIndex -= Samples.Size();
      }

      // Activate the new grain
      newGrain.Activate(
        GetValue(mGrainLengthFrames, mGrainLengthVariance),
        GetValue(mGrainVolume, mGrainVolumeVariance),
        GetValue(mGrainPanningValue, mGrainPanningVariance), 
        &Samples, 
        mSampleChannels, 
        grainIndex,
        GetValue(mGrainResampleRate, mGrainResampleVariance), 
        mWindowType, 
        mWindowAttackFrames,
        mWindowReleaseFrames);

      int nextDelayFrames = mGrainDelayFrames;
      if (mGrainDelayVariance > 0)
        nextDelayFrames = RandomObject.IntVariance(mGrainDelayFrames, mGrainDelayVariance);
      if (mBufferScanRate != 0.0f)
        nextDelayFrames = (unsigned)(nextDelayFrames * mBufferScanRate);

      mFramesToNextGrain += nextDelayFrames;
      currentIndex += nextDelayFrames * mSampleChannels;
    }

    if (mBufferScanRate != 0.0f)
      mFramesToNextGrain -= (int)(frames * mBufferScanRate);
    else
      mFramesToNextGrain -= frames;

    mGrainStartIndex += (unsigned)(frames * mBufferScanRate) * mSampleChannels;
    if (mGrainStartIndex >= (int)Samples.Size())
      mGrainStartIndex -= Samples.Size();
    else if (mGrainStartIndex < 0)
      mGrainStartIndex += Samples.Size();
    
    return true;
  }

  //************************************************************************************************
  void GranularSynthNode::ValidateLengths()
  {
    if (mGrainLengthFrames * mSampleChannels >= Samples.Size())
    {
      mGrainLengthFrames = Samples.Size() - 1;
      mGrainLengthVariance = 0;
    }
    else if ((mGrainLengthFrames + mGrainLengthVariance) * mSampleChannels >= Samples.Size())
    {
      mGrainLengthVariance = (Samples.Size() / mSampleChannels) - mGrainLengthFrames;
    }
  }

  //************************************************************************************************
  unsigned GranularSynthNode::GetValue(unsigned base, unsigned variance)
  {
    if (variance != 0)
      return RandomObject.IntVariance(base, variance);
    else
      return base;
  }

  //************************************************************************************************
  float GranularSynthNode::GetValue(float base, float variance)
  {
    if (variance != 0.0f)
      return RandomObject.FloatVariance(base, variance);
    else
      return base;
  }

}
