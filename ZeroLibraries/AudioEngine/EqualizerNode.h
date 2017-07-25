///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef EQUALIZERNODE_H
#define EQUALIZERNODE_H

namespace Audio
{
  //--------------------------------------------------------------------------- Equalizer Band Gains

  struct EqualizerBandGains
  {
    EqualizerBandGains() :
      Below80Hz(0),
      At150Hz(0),
      At600Hz(0),
      At2500Hz(0),
      Above5000Hz(0)
    {}
    EqualizerBandGains(float below80Hz, float at150Hz, float at600Hz, float at2500Hz, float above5000Hz) :
      Below80Hz(below80Hz),
      At150Hz(at150Hz),
      At600Hz(at600Hz),
      At2500Hz(at2500Hz),
      Above5000Hz(above5000Hz)
    {}

    float Below80Hz;
    float At150Hz;
    float At600Hz;
    float At2500Hz;
    float Above5000Hz;
  };

  //--------------------------------------------------------------------------------- Equalizer Node

  class Equalizer;

  // Controls volume of frequencies in certain bands
  class EqualizerNode : public SimpleCollapseNode
  {
  public:
    EqualizerNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface* extInt, const bool isThreaded = false);

    // Returns the volume adjustment applied to sounds below 80 Hz
    float GetBelow80HzGain();
    // Sets the volume adjustment applied to sounds below 80 Hz
    void SetBelow80HzGain(const float gain);
    // Returns the volume adjustment applied to sounds above 5000 Hz
    float GetAbove5000HzGain();
    // Sets the volume adjustment applied to sounds above 5000 Hz
    void SetAbove5000HzGain(const float gain);
    // Returns the volume adjustment applied to the band centered at 150 Hz
    float Get150HzGain();
    // Sets the volume adjustment applied to the band centered at 150 Hz
    void Set150HzGain(const float gain);
    // Returns the volume adjustment applied to the band centered at 600 Hz
    float Get600HzGain();
    // Sets the volume adjustment applied to the band centered at 600 Hz
    void Set600HzGain(const float gain);
    // Returns the volume adjustment applied to the band centered at 2500 Hz
    float Get2500HzGain();
    // Sets the volume adjustment applied to the band centered at 2500 Hz
    void Set2500HzGain(const float gain);
    // Sets the volume adjustment of all five bands over the time specified, in seconds
    void InterpolateBands(const EqualizerBandGains gainValues, const float timeToInterpolate);

  private:
    ~EqualizerNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void RemoveListener(ListenerNode* listener) override;

    // volume adjustment applied to sounds below 80 Hz
    float LowPassGain;
    // volume adjustment applied to sounds above 5000 Hz
    float HighPassGain;
    // volume adjustment applied to the band centered at 150 Hz
    float Band1Gain;
    // volume adjustment applied to the band centered at 600 Hz
    float Band2Gain;
    // volume adjustment applied to the band centered at 2500 Hz
    float Band3Gain;
    // The filter used for calculations
    typedef Zero::HashMap<ListenerNode*, Equalizer*> FilterMapType;
    FilterMapType FiltersPerListener;
  };

}

#endif
