///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GRANULARSYNTH_H
#define GRANULARSYNTH_H

namespace Audio
{
  namespace GrainWindowTypes
  {
    enum Enum { Linear, Parabolic, RaisedCosine, Trapezoid };
  }

  //----------------------------------------------------------------------------------- Grain Window

  class GrainWindow
  {
  public:
    GrainWindow(unsigned length, GrainWindowTypes::Enum type) :
      mTotalLength(length),
      mCounter(0),
      mType(type)
    {}
    GrainWindow(GrainWindow& other) { /*CopySettings(&other); PLATFORMREFACTOR It is NOT legal to call a pure virtual from a constructor! */ }
    
    virtual float GetNextValue() = 0;
    virtual void Reset(unsigned length, unsigned attack, unsigned release) = 0;
    virtual void CopySettings(GrainWindow* other) = 0;

    unsigned mTotalLength;
    unsigned mCounter;
    GrainWindowTypes::Enum mType;
  };

  //---------------------------------------------------------------------------- Linear Grain Window

  class LinearGrainWindow : public GrainWindow
  {
  public:
    LinearGrainWindow(unsigned length);

    float GetNextValue() override;
    void Reset(unsigned length, unsigned attack, unsigned release) override;
    void CopySettings(GrainWindow* other) override;

    unsigned mHalfLength;
  };

  //--------------------------------------------------------------------- Raised Cosine Grain Window

  class RaisedCosineGrainWindow : public GrainWindow
  {
  public:
    RaisedCosineGrainWindow(unsigned length, unsigned attackLength, unsigned releaseLength);

    float GetNextValue() override; 
    void Reset(unsigned length, unsigned attack, unsigned release) override;
    void CopySettings(GrainWindow* other) override;

    enum State { Attack, Sustain, Release };

    unsigned mAttackLength;
    unsigned mReleaseLength;
    State mCurrentState;
    float b1;
    float y1;
    float y2;
  };

  //------------------------------------------------------------------------------- Parabolic Window

  class ParabolicGrainWindow : public GrainWindow
  {
  public:
    ParabolicGrainWindow(unsigned length);

    float GetNextValue() override;
    void Reset(unsigned length, unsigned attack, unsigned release) override;
    void CopySettings(GrainWindow* other) override;

    float mLastAmplitude;
    float mSlope;
    float mCurve;
  };

  //------------------------------------------------------------------------------- Trapezoid Window

  class TrapezoidGrainWindow : public GrainWindow
  {
  public:
    TrapezoidGrainWindow(unsigned length, unsigned attack, unsigned release);

    float GetNextValue() override;
    void Reset(unsigned length, unsigned attack, unsigned release) override;
    void CopySettings(GrainWindow* other) override;

    enum State { Attack, Sustain, Release };

    unsigned mAttackLength;
    unsigned mReleaseLength;
    float mLastAmplitude;
    float mIncrement;
    State mCurrentState;
  };

  //------------------------------------------------------------------------------------------ Grain

  class Grain
  {
  public:
    Grain();

    Grain& operator=(const Grain& otherGrain);
    void Activate(unsigned length, float volume, float panning, BufferType* sampleBuffer, 
      unsigned channels, unsigned currentIndex, float indexIncrement, GrainWindowTypes::Enum windowType, 
      unsigned windowAttack, unsigned windowRelease);
    void GetSamples(float* outputBuffer, unsigned outputFrames, unsigned outputChannels);
    
    bool mActive;
    unsigned mCounter;
    unsigned mLength;
    double mCurrentFrameIndex;
    float mIndexIncrement;
    float mLeftVolume;
    float mRightVolume;
    float mVolume;
    BufferType* mSourceSamples;
    unsigned mSourceChannels;
    GrainWindow* mWindow;
  };

  //---------------------------------------------------------------------------- Granular Synth Node

  class SoundAsset;

  class GranularSynthNode : public SimpleCollapseNode
  {
  public:
    GranularSynthNode(Zero::StringParam name, const unsigned ID, ExternalNodeInterface *extInt, 
      const bool threaded = false);

    // Starts playing new grains.
    void Play();
    // Stops playing new grains but continues to play current ones.
    void Stop();

    // Sets the asset that will be used for the grains, along with an optional start and stop time.
    // If the stopTime is 0.0, all audio from the asset will be used.
    void SetAsset(SoundAsset* asset, float startTime, float stopTime);

    // Returns the volume modifier applied to the grains.
    float GetGrainVolume();
    // Sets the volume modifier applied to all grains.
    void SetGrainVolume(float volume);
    // Returns the variance for randomizing the grain volume.
    float GetGrainVolumeVariance();
    // Sets the variance for randomizing the grain volume.
    void SetGrainVolumeVariance(float variance);
    // Returns the number of milliseconds between each successive grain.
    int GetGrainDelay();
    // Sets the number of milliseconds to wait before playing another grain.
    void SetGrainDelay(int delayMS);
    // Returns the variance of the grain delay, in milliseconds.
    int GetGrainDelayVariance();
    // Sets the variance for randomizing the grain delay, in milliseconds. 
    void SetGrainDelayVariance(int delayVarianceMS);
    // Returns the length of a grain, in milliseconds.
    int GetGrainLength();
    // Sets the length of the grains, in milliseconds. 
    void SetGrainLength(int lengthMS);
    // Returns the variance of the grain length, in milliseconds.
    int GetGrainLengthVariance();
    // Sets the variance for randomizing the grain length, in milliseconds. 
    void SetGrainLengthVariance(int lengthVarianceMS);
    // Returns the rate at which the grains resample the audio data.
    float GetGrainResampleRate();
    // Sets the rate at which grains resample their audio data. A value of 1.0 will play normally,
    // 0.5 will play at half speed, and -1.0 will play at normal speed backward. Cannot be 0.
    void SetGrainResampleRate(float resampleRate);
    // Returns the variance of the grain resample rate.
    float GetGrainResampleRateVariance();
    // Sets the variance for randomizing the grain resample rate.
    void SetGrainResampleRateVariance(float resampleVariance);
    // Returns the buffer scan rate.
    float GetBufferScanRate();
    // Sets the rate at which the synthesizer scans the buffer as it creates grains. A value of 1.0
    // will move through the audio data at the same rate as it would normally be played, 0.5 will
    // move at half speed, and -1.0 will move at normal speed backward. A value of 0.0 will make
    // the synthesizer repeat the same audio continuously.
    void SetBufferScanRate(float bufferRate);
    // Returns the value used for controlling panning of the grains.
    float GetGrainPanningValue();
    // Sets the value used to pan the grains left or right. A value of 0 will be heard equally from 
    // the left and right, 1.0 will be heard only on the right, and -1.0 will be only left.
    // Values above 1.0 or below -1.0 will be ignored.
    void SetGrainPanningValue(float panValue);
    // Returns the variance for randomizing the grain panning value.
    float GetGrainPanningVariance();
    // Sets the variance for randomizing the grain panning value. 
    void SetGrainPanningVariance(float panValueVariance);
    // Returns the value for controlling randomization of grain locations.
    float GetRandomLocationValue();
    // Sets the value for controlling how many grains have randomized starting positions in the audio.
    // A value of 0 will be completely sequential, while 1.0 will be completely random. 
    // Values below 0 or above 1.0 will be ignored.
    void SetRandomLocationValue(float randomLocationValue);
    // Returns the type of window used for the grains.
    GrainWindowTypes::Enum GetWindowType();
    // Sets the type of window, or volume envelope, used for each grain.
    void SetWindowType(GrainWindowTypes::Enum type);
    // Returns the window attack time, in milliseconds.
    int GetWindowAttack();
    // Sets the window attack time, in milliseconds. Does not have an effect on some windows.
    void SetWindowAttack(int attackMS);
    // Returns the window release time, in milliseconds.
    int GetWindowRelease();
    // Sets the window release time, in milliseconds. Does not have an effect on some windows.
    void SetWindowRelease(int releaseMS);

  private:
    ~GranularSynthNode() {}
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void ValidateLengths();
    unsigned GetValue(unsigned base, unsigned variance);
    float GetValue(float base, float variance);

    static unsigned MsToFrames(int msValue) { return (unsigned)(msValue * 0.001f * SystemSampleRate); }
    static int FramesToMs(unsigned frames) { return (int)(frames / SystemSampleRate * 1000.0f); }

    // Only spawns new grains when this is true
    bool mActive;
    // Audio samples for grains to use
    BufferType Samples;
    // Number of audio channels in the samples
    unsigned mSampleChannels;
    // List of grains
    Zero::Array<Grain> GrainList;
    // Index of the first inactive grain in the list
    int mFirstInactiveGrainIndex;
    // Index of the audio sample where the grain should start playing
    int mGrainStartIndex;
    // Number of frames until the next grain should start
    int mFramesToNextGrain;
    // Base grain volume
    float mGrainVolume;
    // Grain volume variance
    float mGrainVolumeVariance;
    // Base grain delay value
    unsigned mGrainDelayFrames;
    // Grain delay variance
    unsigned mGrainDelayVariance;
    // Base grain length value
    unsigned mGrainLengthFrames;
    // Grain length variance
    unsigned mGrainLengthVariance;
    // Base grain resample rate
    float mGrainResampleRate;
    // Grain resample rate variance
    float mGrainResampleVariance;
    // Buffer scan rate
    float mBufferScanRate;
    // Base grain panning value
    float mGrainPanningValue;
    // Grain panning variance
    float mGrainPanningVariance;
    // Value for controlling grain location randomization
    float mRandomLocationValue;
    // Window type to use
    GrainWindowTypes::Enum mWindowType;
    // Window attack time
    unsigned mWindowAttackFrames;
    // Window release time
    unsigned mWindowReleaseFrames;

    Math::Random RandomObject;
  };
}

#endif
