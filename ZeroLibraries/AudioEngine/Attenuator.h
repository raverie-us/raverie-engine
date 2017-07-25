///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef ATTENUATOR_H
#define ATTENUATOR_H


namespace Audio
{
  //------------------------------------------------------------------------------- Attenuation Data

  struct AttenuationData
  {
    AttenuationData() : 
      StartDistance(0.0f), 
      EndDistance(0.0f), 
      MinimumVolume(0.0f) 
    {}
    AttenuationData(float startDist, float endDist, float minVolume) : 
      StartDistance(startDist),
      EndDistance(endDist),
      MinimumVolume(minVolume) 
    {}

    // Distance at which the volume starts attenuating. 
    float StartDistance;
    // Distance at which the minimum volume is reached. 
    float EndDistance;
    // The minimum volume the sound reaches at the furthest distance. 
    float MinimumVolume;
  };

  //-------------------------------------------------------------------------------- Attenuator Node

  class AttenuationPerListener;

  class AttenuatorNode : public SimpleCollapseNode
  {
  public:
    AttenuatorNode(Zero::Status& status, Zero::StringParam name, const unsigned ID, Math::Vec3Param position, 
      const AttenuationData& data, const CurveTypes curveType, Zero::Array<Math::Vec3> *customCurveData,
      ExternalNodeInterface* extInt, const bool isThreaded = false);
    
    // Sets the current position of the attenuator
    void SetPosition(Math::Vec3Param position);
    // Sets the attenuation data
    void SetAttenuationData(const AttenuationData data);
    // Gets a copy of the attenuation data
    AttenuationData GetAttenuationData();
    // Sets the curve type used by the attenuator
    void SetCurveType(const CurveTypes type, Zero::Array<Math::Vec3> *customCurveData);
    // Sets whether the attenuator should apply a low pass filter
    void SetUsingLowPass(const bool useLowPass);
    // Sets the distance to start applying the low pass filter
    void SetLowPassDistance(const float distance);
    // Sets the low pass filter's cutoff frequency at max distance
    void SetLowPassCutoffFreq(const float frequency);

  private:
    ~AttenuatorNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    float GetAttenuatedVolume() override;
    void RemoveListener(ListenerNode* listener) override;

    typedef Zero::HashMap<ListenerNode*, AttenuationPerListener*> DataPerListenerMapType;

    // Previous volume and low pass for each listener
    DataPerListenerMapType DataPerListener;

    // Current position used for attenuation calculations
    Math::Vec3 Position;
    // Distance at which attenuation starts
    float AttenStartDist;
    // Distance at which attenuation reaches the minimum volume
    float AttenEndDist;
    // The minimum volume reached at the furthest attenuation
    float MinimumVolume;
    // Used to get attenuation value based on falloff curve
    InterpolatingObject* DistanceInterpolator;
    // If true, will apply a low pass filter with attenuation
    bool UseLowPass;
    // The distance at which the low pass filter starts
    float LowPassDistance;
    // Interpolator for low pass filter
    InterpolatingObject* LowPassInterpolator;
  };


}

#endif 
