///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

//------------------------------------------------------------------------- Attenuation Per Listener

class AttenuationPerListener
{
public:
  AttenuationPerListener() : PreviousVolume(0) {}

  // Volume used last mix for this listener
  float PreviousVolume;
  // Low pass filter for this listener
  LowPassFilter LowPass;
};

//---------------------------------------------------------------------------------- Attenuator Node

class AttenuatorNode : public SimpleCollapseNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  AttenuatorNode(StringParam name, const unsigned ID, Math::Vec3Param position, float startDistance,
    float endDistance, float minVolume, const FalloffCurveType::Enum curveType, Array<Math::Vec3> *customCurveData);
  ~AttenuatorNode();

  // Sets the current position of the attenuator
  void SetPosition(Math::Vec3Param position);
  // Sets the distance at which the volume starts attenuating
  void SetStartDistance(float distance);
  // Sets the distance at which the minimum volume is reached
  void SetEndDistance(float distance);
  // Sets the minimum volume reached at the furthest distance
  void SetMinimumVolume(float volume);
  // Sets the curve type used by the attenuator
  void SetCurveType(const FalloffCurveType::Enum type, Array<Math::Vec3> *customCurveData);
  // Sets whether the attenuator should apply a low pass filter
  void SetUsingLowPass(bool useLowPass);
  // Sets the distance to start applying the low pass filter
  void SetLowPassDistance(float distance);
  // Sets the low pass filter's cutoff frequency at max distance
  void SetLowPassCutoffFreq(float frequency);

private:
  bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest) override;
  float GetVolumeChangeFromOutputsThreaded() override;
  void RemoveListenerThreaded(SoundEvent* event) override;
  void UpdateDistanceInterpolator();
  void UpdateLowPassInterpolator();

  typedef Zero::HashMap<ListenerNode*, AttenuationPerListener*> DataPerListenerMapType;

  // Previous volume and low pass for each listener
  DataPerListenerMapType DataPerListener;

  // Current position used for attenuation calculations
  Threaded<Math::Vec3> mPosition;
  // Distance at which attenuation starts
  Threaded<float> mAttenStartDist;
  // Distance at which attenuation reaches the minimum volume
  Threaded<float> mAttenEndDist;
  // The minimum volume reached at the furthest attenuation
  Threaded<float> mMinimumVolume;
  // Used to get attenuation value based on falloff curve
  InterpolatingObject DistanceInterpolator;
  // If true, will apply a low pass filter with attenuation
  Threaded<bool> mUseLowPass;
  // The distance at which the low pass filter starts
  Threaded<float> mLowPassDistance;
  // The cutoff frequency for the low pass filter at the furthest distance
  Threaded<float> mLowPassCutoff;
  // Interpolator for low pass filter
  InterpolatingObject LowPassInterpolator;

  const float cLowPassCutoffHighValue = 15000.0f;
  const float cLowPassCutoffLowValue = 1000.0f;
};

} // namespace Zero
