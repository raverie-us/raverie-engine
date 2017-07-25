///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef FILTERNODES_H
#define FILTERNODES_H

namespace Audio
{
  class InterpolatingObject;

  //---------------------------------------------------------------------------------- Low Pass Node
  
  class LowPassFilter;

  // Cuts out high frequencies, not altering frequencies below the cutoff
  class LowPassNode : public SimpleCollapseNode
  {
  public:
    LowPassNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface* extInt, const bool isThreaded = false);

    // Returns the current cutoff frequency
    float GetCutoffFrequency();
    // Sets the cutoff frequency for the filter
    void SetCutoffFrequency(const float freq);

  private:
    ~LowPassNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void RemoveListener(ListenerNode* listener) override;

    // The cutoff frequency for the low pass filter
    float CutoffFrequency;
    // The filter used for calculations
    typedef Zero::HashMap<ListenerNode*, LowPassFilter*> FilterMapType;
    FilterMapType FiltersPerListener;
  };

  //--------------------------------------------------------------------------------- High Pass Node

  class HighPassFilter;

  // Cuts out low frequencies, not altering frequencies above the cutoff
  class HighPassNode : public SimpleCollapseNode
  {
  public:
    HighPassNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface* extInt, const bool isThreaded = false);

    // Returns the current cutoff frequency
    float GetCutoffFrequency();
    // Sets the cutoff frequency for the filter
    void SetCutoffFrequency(const float freq);

  private:
    ~HighPassNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void RemoveListener(ListenerNode* listener) override;

    // The cutoff frequency for the high pass filter
    float CutoffFrequency;
    // The filter used for calculations
    typedef Zero::HashMap<ListenerNode*, HighPassFilter*> FilterMapType;
    FilterMapType FiltersPerListener;
  };

  //--------------------------------------------------------------------------------- Band Pass Node

  class BandPassFilter;

  // Cuts out frequencies above and below a specified band
  class BandPassNode : public SimpleCollapseNode
  {
  public:
    BandPassNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface* extInt, const bool isThreaded = false);

    // Returns the central frequency of the band
    float GetCentralFrequency();
    // Sets the central frequency of the band
    void SetCentralFrequency(const float frequency);
    // Returns the current Q factor
    float GetQuality();
    // Sets the Q factor
    void SetQuality(const float Q);

  private:
    ~BandPassNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void RemoveListener(ListenerNode* listener) override;

    // The current central frequency 
    float CentralFrequency;
    // The Q factor (controls width of the band)
    float Quality;
    // The filter used for calculations
    typedef Zero::HashMap<ListenerNode*, BandPassFilter*> FilterMapType;
    FilterMapType FiltersPerListener;
  };

  //------------------------------------------------------------------------------------- Delay Node

  class DelayLine;

  class DelayNode : public SimpleCollapseNode
  {
  public:
    DelayNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface* extInt, const bool threaded = false);

    // Returns the current length of delay
    float GetDelayMSec();
    // Sets the length of delay in milliseconds
    void SetDelayMSec(const float delay);
    // Returns the current feedback percentage
    float GetFeedbackPct();
    // Sets the percentage of output which is fed back in as input
    void SetFeedbackPct(const float feedbackPercent);
    // Returns the current wet percentage
    float GetWetLevelPct();
    // Sets the percentage of output which is filtered
    void SetWetLevelPct(const float wetLevelPercent);
    // Sets the wet level percentage over time
    void InterpolateWetLevelPct(const float percent, const float time);

  private:
    ~DelayNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void RemoveListener(ListenerNode* listener) override;

    // Length of delay 
    float DelayMSec;
    // Feedback value
    float FeedbackPct;
    // Wet level value
    float WetPercent;
    // Will be true if there was recent input
    bool HasHadInput;
    // The filter used for calculations
    typedef Zero::HashMap<ListenerNode*, DelayLine*> FilterMapType;
    FilterMapType FiltersPerListener;
  };

  //----------------------------------------------------------------------------------- Flanger Node

  class Oscillator;

  enum OscillatorTypes { Sine = 0, Saw, Triangle, Square, Noise };

  class FlangerNode : public SimpleCollapseNode
  {
  public:
    FlangerNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface *extInt, const bool threaded = false);

    // Returns the current max delay in milliseconds
    float GetMaxDelayMSec();
    // Sets the max delay in milliseconds
    void SetMaxDelayMSec(const float delay);
    // Returns the current modulation frequency
    float GetModFrequency();
    // Sets the modulation frequency
    void SetModFrequency(const float frequency);
    // Returns the current feedback percentage
    float GetFeedbackPct();
    // Sets the feedback percentage
    void SetFeedbackPct(const float percent);
    // Returns the current oscillator type
    int GetOscillatorType();
    // Sets the oscillator type to use for modulation
    void SetOscillatorType(const OscillatorTypes type);

  private:
    ~FlangerNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void RemoveListener(ListenerNode* listener) override;

    struct Data
    {
      Data(float frequency, float feedback);
      ~Data();

      // Delay filter for calculations
      DelayLine* Delay;
      // Oscillator filter for calculations
      Oscillator* LFO;
    };

    typedef Zero::HashMap<ListenerNode*, Data*> FilterMapType;
    FilterMapType FiltersPerListener;

    // Maximum delay allowed
    float MaxDelay;
    // Modulation frequency
    float ModFrequency;
    // Feedback percentage
    float FeedbackPct;
    // Oscillator type used for modulation
    OscillatorTypes OscillatorType;
  };

  //------------------------------------------------------------------------------------ Chorus Node

  class ChorusNode : public SimpleCollapseNode
  {
  public:
    ChorusNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface *extInt, const bool threaded = false);

    // Returns the current max delay in milliseconds
    float GetMaxDelayMSec();
    // Sets the max delay in milliseconds
    void SetMaxDelayMSec(const float delay);
    // Returns the current max delay in milliseconds
    float GetMinDelayMSec();
    // Sets the max delay in milliseconds
    void SetMinDelayMSec(const float delay);
    // Returns the current modulation frequency
    float GetModFrequency();
    // Sets the modulation frequency
    void SetModFrequency(const float frequency);
    // Returns the current feedback percentage
    float GetFeedbackPct();
    // Sets the feedback percentage
    void SetFeedbackPct(const float percent);
    // Returns the current oscillator type
    OscillatorTypes GetOscillatorType();
    // Sets the oscillator type to use for the modulation
    void SetOscillatorType(const OscillatorTypes type);
    // Returns the current chorus offset
    float GetOffsetMSec();
    // Sets the chorus offset
    void SetOffsetMSec(const float offset);

  private:
    ~ChorusNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void RemoveListener(ListenerNode* listener) override;

    struct Data
    {
      Data(float frequency, float minDelay, float feedback);
      ~Data();

      // Delay filter for calculations
      DelayLine* Delay;
      // Oscillator filter for calculations
      Oscillator* LFO;
    };

    typedef Zero::HashMap<ListenerNode*, Data*> FilterMapType;
    FilterMapType FiltersPerListener;

    // Minimum delay amount
    float MinDelay;
    // Maximum delay amount
    float MaxDelay;
    // Modulation frequency
    float ModFrequency;
    // Feedback percentage
    float FeedbackPct;
    // Oscillator type to use for modulation
    OscillatorTypes OscillatorType;
    // Chorus offset value
    float ChorusOffset;
  };

  //--------------------------------------------------------------------------------- Add Noise Node

  class AddNoiseNode : public SimpleCollapseNode
  {
  public:
    AddNoiseNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface *extInt, const bool threaded = false);

    // Returns the gain of the additive noise in decibels
    float GetAdditiveNoiseGainDB();
    // Sets the gain of the additive noise in decibels
    void SetAdditiveNoiseGainDB(const float decibels);
    // Returns the gain of the multiplicative noise in decibels
    float GetMultipleNoiseGainDB();
    // Sets the gain of the multiplicative noise in decibels
    void SetMultipleNoiseGainDB(const float decibels);
    // Returns the cutoff frequency of the additive noise
    float GetAdditiveCutoffHz();
    // Sets the cutoff frequency of the additive noise
    void SetAdditiveCutoffHz(const float cutoff);
    // Returns the cutoff frequency of the multiplicative noise
    float GetMultipleCutoffHz();
    // Sets the cutoff frequency of the multiplicative noise
    void SetMultipleCutoffHz(const float cutoff);

  private:
    ~AddNoiseNode() {}
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;

    // Gain of additive noise in decibels (default is -40)
    float AdditiveNoiseDB;
    // Gain of multiplicative noise in decibels (default is -10)
    float MultipleNoiseDB;
    // Cutoff frequency of additive noise in Hz (default is 2000)
    float AdditiveNoiseCutoffHz;
    // Cutoff frequency of multiplicative noise in Hz (default is 50)
    float MultipleNoiseCutoffHz;

    int AddCount;
    int MultiplyCount;
    float AddNoise;
    float MultiplyNoise;
    Math::Random RandomValues;
    float AddPeriod;
    float MultiplyPeriod;
    float AddGain;
    float MultiplyGain;
  };

  //-------------------------------------------------------------------------------- Modulation Node

  class ModulationNode : public SimpleCollapseNode
  {
  public:
    ModulationNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface *extInt, const bool threaded = false);
    
    // Returns true if using amplitude modulation, false if using ring modulation
    bool GetUsingAmplitude();
    // If true, will use amplitude modulation, if false, ring modulation
    void SetUsingAmplitude(const bool useAmplitudeMod);
    // Returns the modulator wave frequency
    float GetFrequency();
    // Sets the frequency of the modulator wave
    void SetFrequency(const float frequency);
    // Returns the percent of output that is modulated
    float GetWetPercent();
    // Sets the percent of output that should be modulated
    void SetWetPercent(const float percentage);

  private:
    ~ModulationNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void RemoveListener(ListenerNode* listener) override;

    bool Amplitude;
    float Frequency;
    float WetValue;

    typedef Zero::HashMap<ListenerNode*, Oscillator*> OscMapType;
    OscMapType OscillatorsPerListener;
  };

}

#endif