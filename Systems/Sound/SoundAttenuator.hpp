///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

//---------------------------------------------------------------------------- Sound Attenuator Node

class SoundAttenuatorNode
{
public:
  SoundAttenuatorNode(AttenuatorNode* node) : mNode(node) {}

  HandleOf<AttenuatorNode> mNode;

  Link<SoundAttenuatorNode> link;
};

//--------------------------------------------------------------------------------- Sound Attenuator 

/// Decreases a positional sound's volume as the SoundEmitter gets further away from a SoundListener.
class SoundAttenuator : public DataResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SoundAttenuator();
  ~SoundAttenuator();

  void Serialize(Serializer& serializer) override;
  void Initialize() override;
  void Unload() override;

  /// The distance from a SoundListener at which the sound's volume begins attenuating. 
  /// At shorter distances the volume will not be changed. Cannot be larger than the StopDistance.
  float GetStartDistance();
  void SetStartDistance(float value);
  /// The distance at which the attenuation reaches the minimum volume.
  /// No volume changes will happen past this distance. Cannot be smaller than the StartDistance.
  float GetStopDistance();
  void SetStopDistance(float value);
  /// The lowest volume that the attenuation will reach. 
  /// If set above 0, the sound will continue to be heard at all distances.
  float GetMinAttenuatedVolume();
  void SetMinAttenuatedVolume(float value);
  /// The type of curve used to reduce the sound's volume over distance. The default is a logarithmic 
  /// curve which mimics the real world.
  FalloffCurveType::Enum GetFalloffCurveType();
  void SetFalloffCurveType(FalloffCurveType::Enum curveType);
  /// The SampleCurve resource to use as the attenuation's falloff curve. 
  /// It will be normalized and stretched to fit between the StartDistance and StopDistance values.
  SampleCurve* GetFalloffCurve();
  void SetFalloffCurve(SampleCurve* curve);
  /// If true, a low pass filter will be applied to the sound after reaching a specified distance,
  /// mimicking the way sound is muffled with distance in real life. The filter begins at the
  /// LowPassStartDistance and interpolates its cutoff frequency logarithmically until the StopDistance.
  /// The filter will not change past the StopDistance.
  bool GetUseLowPassFilter();
  void SetUseLowPassFilter(bool useFilter);
  /// The distance at which the low pass filter begins to take effect. 
  float GetLowPassStartDistance();
  void SetLowPassStartDistance(float distance);
  /// The lowest cutoff frequency of the low pass filter, reached at the StopDistance. 
  /// The cutoff frequency will be interpolated logarithmically from 15000.00 (a value with 
  /// very little effect on the sound) to the LowPassCutoffFreq between the 
  /// LowPassStartDistance and the StopDistance.
  float GetLowPassCutoffFreq();
  void SetLowPassCutoffFreq(float frequency);

// Internals
  // Returns an attenuation node to use for sound cues and emitters
  SoundAttenuatorNode* GetAttenuationNode(StringParam name, unsigned ID);

  void RemoveAttenuationNode(SoundAttenuatorNode* node);

  bool HasInput();

private:
  void UpdateCurve(Event* event);

  typedef InList<SoundAttenuatorNode> AttenuatorListType;
  AttenuatorListType mNodeList;

  float mStartDistance;
  float mStopDistance;
  float mMinAttenuatedVolume;
  FalloffCurveType::Enum mFalloffCurveType;
  HandleOf<SampleCurve> mCustomFalloffCurve;
  bool mUseLowPassFilter;
  float mLowPassStartDistance;
  float mLowPassCutoffFreq;
};

class SoundAttenuatorDisplay : public MetaDisplay
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  String GetName(HandleParam object) override;
  String GetDebugText(HandleParam object) override;
};

//------------------------------------------------------------------------- Sound Attenuator Manager

class SoundAttenuatorManager : public ResourceManager
{
public:
  DeclareResourceManager(SoundAttenuatorManager, SoundAttenuator);
  SoundAttenuatorManager(BoundType* resourceType);
};

}//namespace Zero
