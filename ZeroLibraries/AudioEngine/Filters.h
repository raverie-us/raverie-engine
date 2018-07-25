///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef FILTERS_H
#define FILTERS_H

namespace Audio
{
  //---------------------------------------------------------------------------------- BiQuad Filter

  class BiQuad
  {
  public:
    BiQuad();

    void FlushDelays();
    void SetValues(const float a0, const float a1, const float a2, const float b1, const float b2);
    float DoBiQuad(const float x);
    void AddHistoryTo(BiQuad& otherFilter);

  private:
    float x_1;
    float x_2;
    float y_1;
    float y_2;
    float a0;
    float a1;
    float a2;
    float b1;
    float b2;
  };

  //----------------------------------------------------------------------------------- Delay Filter

  class Delay
  {
  public:
    Delay(float maxDelayTime, int sampleRate);
    virtual ~Delay();

    void ResetDelay();
    void SetDelayMSec(const float mSec);
    void SetOutputAttenuation(const float attenDB);
    float ReadDelay();
    float ReadDelayAt(const float mSec);
    void WriteDelayAndInc(const float delayInput);
    virtual void ProcessAudio(const float input, float *output);

  protected:
    float *mBuffer;
    float mDelayInSamples;
    float mOutputAttenuation;
    int mBufferSize;
    int mReadIndex;
    int mPrevReadIndex;
    int mWriteIndex;
    int mSampleRate;

  };

  //-------------------------------------------------------------------------- Delay All Pass Filter

  class DelayAPF : public Delay
  {
  public:
    DelayAPF(const float maxDelayTime, const int sampleRate);
    ~DelayAPF(){}

    void SetAPFg(const float g){ mAPFg = g; }
    void ProcessAudio(const float input, float *output) override;

  private:
    float mAPFg;
  };

  //------------------------------------------------------------------------------------ Comb Filter

  class Comb : public Delay
  {
  public:
    Comb(const float maxDelayTime, const int sampleRate);
    ~Comb(){}

    void SetCombG(const float g) { mCombG = g; }
    void SetCombGWithRT60(const float RT);
    void ProcessAudio(const float input, float *output) override;

  private:
    float mCombG;
  };

  //--------------------------------------------------------------------------- Low Pass Comb Filter

  class LPComb : public Delay
  {
  public:
    LPComb(const float maxDelayTime, const int sampleRate);
    ~LPComb(){}

    void SetG(const float combG, const float overallGain);
    void SetGWithRT60(const float RT, const float overallGain);
    void ProcessAudio(const float input, float *output) override;

  private:
    float mCombG;
    float mLPFg;
    float mPrevSample;
  };

  //----------------------------------------------------------------------- One Pole Low Pass Filter

  class OnePoleLP
  {
  public:
    OnePoleLP();
    ~OnePoleLP(){}

    void SetLPFg(const float g);
    void Initialize();
    void ProcessAudio(const float input, float *output);

  private:
    float mLPFg;
    float mInvG;
    float mPrevSample;
  };

  //-------------------------------------------------------------------------------- Low Pass Filter

  class LowPassFilter 
  {
  public:
    LowPassFilter();

    void ProcessFrame(const float* input, float* output, const unsigned numChannels);
    void ProcessBuffer(const float* input, float* output, const unsigned numChannels,
      const unsigned numSamples);

    float GetCutoffFrequency();
    void SetCutoffFrequency(const float value);
    void MergeWith(LowPassFilter& otherFilter);

  private:
    float CutoffFrequency;
    float SqRoot2;
    float HalfPI;

    BiQuad BiQuadsPerChannel[cMaxChannels];

    void SetCutoffValues();
  };

  //------------------------------------------------------------------------------- High Pass Filter

  class HighPassFilter
  {
  public:
    HighPassFilter();

    void ProcessFrame(const float* input, float* output, const unsigned numChannels);

    void SetCutoffFrequency(const float value);
    void MergeWith(HighPassFilter& otherFilter);

  private:
    float CutoffFrequency;
    float SqRoot2;
    float HalfPI;

    BiQuad BiQuadsPerChannel[cMaxChannels];

    void SetCutoffValues();
  };

  //------------------------------------------------------------------------------- Band Pass Filter

  class BandPassFilter
  {
  public:
    BandPassFilter();

    void ProcessFrame(const float* input, float* output, const unsigned numChannels);

    void SetFrequency(const float frequency);
    void SetQuality(const float Q);
    void MergeWith(BandPassFilter& otherFilter);

  private:
    float CentralFreq;
    float Quality;
    float LowPassCutoff;
    float HighPassCutoff;
    float AlphaLP;
    float AlphaHP;
    float PreviousInput[cMaxChannels];
    float PreviousOutput1[cMaxChannels];
    float PreviousOutput2[cMaxChannels];

    void ResetFrequencies();

  };

  //------------------------------------------------------------------------------------- Oscillator

  class Oscillator 
  {
  public:
    Oscillator();

    void ProcessBuffer(float *buffer, const unsigned numChannels, const unsigned bufferSize);

    float GetNextSample();

    enum Polarities
    {
      Bipolar = 0,
      Unipolar
    };

    void SetFrequency(const float frequency);
    void SetType(const OscillatorTypes::Enum type);
    void SetPolarity(const Polarities polarity) { mPolarity = polarity; }
    void SetNoteOn(const bool isOn) { mNoteOn = isOn; }
    void SetSquareWavePositiveFraction(const float positiveFraction);

  private:
    float mFrequency;
    OscillatorTypes::Enum mType;
    Polarities mPolarity;
    static const int ArraySize = 1024;
    float mWaveValues[ArraySize];
    Math::Random RandomObject;
    float mReadIndex;
    float mIncrement;
    bool mNoteOn;
    float mSquareWavePositiveFraction;
  };

  //------------------------------------------------------------------------------------- Delay Line

  class DelayLine
  {
  public:
    DelayLine();
    ~DelayLine();

    void ProcessBuffer(const float *input, float *output, const unsigned numChannels, 
      const unsigned bufferSize);

    // Sets the length of delay in milliseconds
    void SetDelayMSec(float delay);
    // Returns the current length of delay
    float GetDelayMSec();
    // Sets the fraction of output which is fed back in as input (0 - 1.0f)
    void SetFeedback(const float feedbackValue);
    // Returns the current feedback fraction (0 - 1.0f)
    float GetFeedback() { return Feedback; }
    // Sets the fraction of output which is filtered (0 - 1.0f)
    void SetWetLevel(const float wetLevelValue);
    // Returns the current wet level (0 - 1.0f)
    float GetWetLevel() { return WetLevel; }
    // Changes the wet level value over time
    void InterpolateWetLevel(const float newValue, const float time);

  private:
    // Length of delay in samples
    float DelayInSamples;
    // Feedback value (0 - 1.0f)
    float Feedback;
    // Wet level value (0 - 1.0f)
    float WetLevel;
    // Array of delay buffers per channel
    float* BuffersPerChannel[cMaxChannels];
    // Current read position in the buffers
    int ReadIndex;
    // Current write position in the buffers
    int WriteIndex;
    // Size of the buffers
    int BufferSize;

    bool InterpolatingWetLevel;
    InterpolatingObject WetLevelInterpolator;
  };

  //------------------------------------------------------------------------------ Envelope Detector

  class EnvelopeDetector
  {
  public:
    EnvelopeDetector();

    enum DetectModes { Peak, MS, RMS };

    void Initialize(const float attackMSec, const float releaseMSec, const bool analogTC, 
      const DetectModes detectMode, const bool logDetector);
    
    void SetTCModeAnalog(const bool analogTC);
    void SetAttackTime(const float attackMSec);
    void SetReleaseTime(const float releaseMSec);
    void SetDetectMode(const DetectModes mode);
    void SetLogDetect(const bool logDetect) { mLogDetector = logDetect; }

    void Reset();

    float Detect(float input);

  private:
    int mSample;
    float mAttackTime;
    float mReleaseTime;
    float mAttackTimeMSec;
    float mReleaseTimeMSec;
    float mSampleRate;
    float mEnvelope;
    DetectModes mDetectMode;
    bool mAnalogTC;
    bool mLogDetector;

    const float DIGITAL_TC = -4.60517019f; // ln(1%)
    const float ANALOG_TC = -1.00239343f; // ln(36.7%)
    //const float METER_UPDATE_INTERVAL_MSEC = 50.0;
    //const float METER_MIN_DB = -60.0;
  };

  //---------------------------------------------------------------------- Dynamics Processor Filter

  class DynamicsProcessor
  {
  public:
    enum ProcessorTypes { Compressor, Limiter, Expand, Gate };

    DynamicsProcessor();
    DynamicsProcessor(const float inputGain, const float threshold, const float attack, const float release, 
      const float ratio, const float outputGain, const float knee, const ProcessorTypes type);

    void ProcessBuffer(const float *input, const float* envelopeInput, float *output, 
      const unsigned numChannels, const unsigned bufferSize);
    
    float GetInputGain() { return mInputGainDB; }
    void SetInputGain(const float gainDB) { mInputGainDB = gainDB; }
    float GetThreshold() { return mThresholdDB; }
    void SetThreshold(const float thresholdDB) { mThresholdDB = thresholdDB; }
    float GetAttackMSec() { return mAttackMSec; }
    void SetAttackMSec(const float attack);
    float GetReleaseMSec() { return mReleaseMSec; }
    void SetReleaseMSec(const float release);
    float GetRatio() { return mRatio; }
    void SetRatio(const float ratio);
    float GetOutputGain() { return mOutputGainDB; }
    void SetOutputGain(const float gainDB);
    float GetKneeWidth() { return mKneeWidth; }
    void SetKneeWidth(const float kneeWidth);
    ProcessorTypes GetType() { return mProcessorType; }
    void SetType(const ProcessorTypes type) { mProcessorType = type; }

  private:
    float CompressorGain(const float detectorValue);
    float ExpanderGain(const float detectorValue);

    float mInputGainDB;
    float mThresholdDB;
    float mAttackMSec;
    float mReleaseMSec;
    float mRatio;
    float mOutputGainDB;
    float mKneeWidth;
    ProcessorTypes mProcessorType;
    bool mAnalog;

    float mOutputGain;
    float mCompressorRatio;
    float mExpanderRatio;
    float mHalfKnee;

    EnvelopeDetector Detectors[cMaxChannels];

    static double LagrangeInterpolation(double *x, double *y, int howMany, double xBar);
  };

  //------------------------------------------------------------------------------- Equalizer Filter

  class Equalizer
  {
  public:
    Equalizer();
    Equalizer(const float below80Hz, const float at150Hz, const float at600Hz, const float at2500Hz, 
      const float above5000Hz);
    Equalizer(const Equalizer& copy);

    void ProcessBuffer(const float *input, float *output, const unsigned numChannels, 
      const unsigned bufferSize);

    float GetBelow80HzGain();
    void SetBelow80HzGain(const float gain);
    float Get150HzGain();
    void Set150HzGain(const float gain);
    float Get600HzGain();
    void Set600HzGain(const float gain);
    float Get2500HzGain();
    void Set2500HzGain(const float gain);
    float GetAbove5000HzGain();
    void SetAbove5000HzGain(const float gain);
    void InterpolateBands(const float below80Hz, const float at150Hz, const float at600Hz, 
      const float at2500Hz, const float above5000Hz, const float timeToInterpolate);

    void MergeWith(Equalizer& otherFilter);

  private:
    float mLowPassGain;
    float mHighPassGain;
    float mBand1Gain;
    float mBand2Gain;
    float mBand3Gain;

    LowPassFilter LowPass;
    HighPassFilter HighPass;
    BandPassFilter Band1;
    BandPassFilter Band2;
    BandPassFilter Band3;

    InterpolatingObject LowPassInterpolator;
    InterpolatingObject HighPassInterpolator;
    InterpolatingObject Band1Interpolator;
    InterpolatingObject Band2Interpolator;
    InterpolatingObject Band3Interpolator;

    void SetFilterData();
  };

  //---------------------------------------------------------------------------------- Reverb Filter

  class ReverbData
  {
  public:
    ReverbData();

    void Initialize(const float lpGain);
    float ProcessSample(const float input);

    Delay PreDelay;

    // Input diffusion
    DelayAPF InputAP_1;
    DelayAPF InputAP_2;
    OnePoleLP InputLP;

    // Parallel comb filters
    Comb Comb_1;
    Comb Comb_2;
    LPComb LPComb_1;
    LPComb LPComb_2;

    // Damping
    OnePoleLP DampingLP;

    // Output diffusion
    DelayAPF OutputAP;
  };

  class Reverb
  {
  public:
    Reverb();

    bool ProcessBuffer(const float *input, float *output, const unsigned numChannels, 
      const unsigned bufferSize);

    // Sets the length of the reverb tail in milliseconds
    void SetTime(const float timeInMSec);
    // Sets the fraction of output that is filtered (0 - 1.0)
    void SetWetLevel(const float wetLevel);
    // Sets the fraction of filtered output over the specified number of seconds
    void InterpolateWetLevel(const float newWetLevel, const float time);

  private:
    void Initialize();

    // The current length of reverb in milliseconds
    float TimeMSec;
    // Maximum reverb data channels
    static const int ChannelCount = cMaxChannels - 1;
    // Data per channel
    ReverbData Data[ChannelCount];
    // The low pass gain value
    float LPgain;
    // The value of the wet level
    float WetValue;
    // Used to interpolate the wet level
    InterpolatingObject WetValueInterpolator;
  };

  //--------------------------------------------------------------------------------- Complex Number

  class ComplexNumber
  {
  public:
    ComplexNumber() :
      mReal(0.0f),
      mImaginary(0.0f)
    {}
    ComplexNumber(const float real, const float imaginary) :
      mReal(real),
      mImaginary(imaginary)
    {}
    ComplexNumber(const ComplexNumber& copy) :
      mReal(copy.mReal),
      mImaginary(copy.mImaginary)
    {}

    ComplexNumber operator*(const float constant) const;
    ComplexNumber operator*(const ComplexNumber& number) const;
    ComplexNumber operator+(const ComplexNumber& number) const;
    ComplexNumber operator-(const ComplexNumber& number) const;
    ComplexNumber& operator+=(const ComplexNumber& number);
    void Set(float real, float imaginary);
    float Magnitude() const;
    float MagnitudeSquared() const;

    float mReal;
    float mImaginary;
  };

  //------------------------------------------------------------------------- Fast Fourier Transform

  class FFT
  {
  public:

    static void Forward(ComplexNumber* samples, const int numberOfSamples);
    static void Backward(ComplexNumber* samples, const int numberOfSamples);
    static void Forward(const float* input, ComplexNumber* result, const int numberOfSamples);
    
  private:
    static void DoFFT(ComplexNumber* samples, const int numberOfSamples, const bool forward);
  };

  //--------------------------------------------------------------- Fast Fourier Transform Convolver

  class FFTConvolver
  {
  public:
    FFTConvolver();
    ~FFTConvolver();

    bool Initialize(int blockSize, const float* impulseResponse, int irLength);
    void ProcessBuffer(const float* input, float* output, int length);
    void Reset();

  private:
    int mBlockSize;
    int mSegmentCount;
    Zero::Array<float> mStoredInputBuffer;
    int mBufferPosition;
    int mCurrentSegment;
    typedef Zero::Array<ComplexNumber> ComplexListType;
    Zero::Array<ComplexListType> mSegments;
    Zero::Array<ComplexListType> mSegmentsIR;
    ComplexListType mConvolvedSamples;
    ComplexListType mOverlap;
  };


  static int NextPowerOf2(const int& value);

  //---------------------------------------------------------------------------------- ADSR envelope
  class EnvelopeSettings;

  class ADSR
  {
  public:
    ADSR();
    ADSR(const ADSR& copy);

    void SetValues(const EnvelopeSettings* settings);
    float operator()();
    void Release();
    bool IsFinished();

  private:
    float mDelayTime;
    float mAttackTime;
    float mDecayTime;
    float mSustainTime;
    float mSustainLevel;
    float mReleaseTime;
    float mCurrentTime;
    float mLastAmplitude;

    enum States { DelayState, AttackState, DecayState, SustainState, ReleaseState, OffState };

    States mCurrentState;
  };

  //------------------------------------------------------------------ Frequency Modulation Operator

  class FMOperator
  {
  public:
    FMOperator();

    //void SetValues(float frequency, float volume, EnvelopeSettings& envelope);
    void SetOffsetPitch(const float cents);
    float operator()(const float phi);

    float mFrequency;
    float mVolume;

  private:
    ADSR Envelope;
    float mPitchOffset;
    double mTime;
  };
}

#endif