///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareBitField4(VortexFlags, LocalAxis, ClampToMax, ContinueFalloff, NoEffect);

/// Applies a force about an axis at the object's center. This will apply two
/// forces to a body: One pulls the object towards the center of the vortex and
/// the other applies a tangential force. Useful to model a vortex.
/// This only expects to be used as a Region effect.
class VortexEffect : public PhysicsEffect
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  VortexEffect();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void DebugDraw() override;

  // Physics Effect Interface
  void PreCalculate(real dt) override;
  void ApplyEffect(RigidBody* obj, real dt) override;

  // Update the cached vortex information
  void ComputeVortexInformation();
  /// Computes the strength of both forces based upon the t-value distance from the vortex center.
  void GetStrengthValues(real t, real& twistForce, real& inwardForce);

  // Properties

  /// Determines if the vortex axis is in world or local space.
  bool GetLocalAxis();
  void SetLocalAxis(bool isLocal);
  /// The first distance at which attenuation will start. If an object is
  /// under the min distance, the min strength values will be used.
  /// If an object is in between min and max, then it will attenuate.
  real GetMinDistance();
  void SetMinDistance(real distance);
  /// The max distance that attenuation will happen at. If an object is
  /// between min and max distance, the value will be attenuated. If the
  /// object is further away, the effect strength will be determined by EndCondition.
  real GetMaxDistance();
  void SetMaxDistance(real maxDistance);
  /// The perpendicular strength (twist) of the vortex at min distance.
  real GetTwistStrengthAtMinDistance();
  void SetTwistStrengthAtMinDistance(real forceStrenth);
  /// The perpendicular strength (twist) of the vortex at max distance.
  real GetTwistStrengthAtMaxDistance();
  void SetTwistStrengthAtMaxDistance(real forceStrenth);
  /// The inward strength of the vortex at the min distance.
  real GetInwardStrengthAtMinDistance();
  void SetInwardStrengthAtMinDistance(real inwardStrenth);
  /// The inward strength of the vortex at the max distance.
  real GetInwardStrengthAtMaxDistance();
  void SetInwardStrengthAtMaxDistance(real inwardStrenth);
  /// The axis the vortex spins about.
  Vec3 GetVortexAxis();
  void SetVortexAxis(Vec3 axis);
  /// The axis the vortex spins about in world space.
  Vec3 GetWorldVortexAxis();
  /// How the interpolation should be handled at MaxDistance. ClampToMax
  /// will clamp to the max strength values. NoEffect will ignore the effect.
  /// ContinueFalloff will continue the interpolation (this may go negative).
  PhysicsEffectEndCondition::Enum GetEndCondition();
  void SetEndCondition(PhysicsEffectEndCondition::Enum condition);
  /// The type of interpolation used (e.g. Linear, Quadratic) for the forces.
  PhysicsEffectInterpolationType::Enum GetInterpolationType();
  void SetInterpolationType(PhysicsEffectInterpolationType::Enum type);

private:

  real mMinDistance;
  real mMaxDistance;
  real mTwistStrengthAtMinDistance;
  real mTwistStrengthAtMaxDistance;
  real mInwardStrengthAtMinDistance;
  real mInwardStrengthAtMaxDistance;
  Vec3 mVortexAxis;
  
  BitField<VortexFlags::Enum> mVortexStates;
  PhysicsEffectInterpolationType::Enum mInterpolationType;

  Vec3 mWorldVortexAxis;
  Vec3 mWorldVortexCenter;
};

}//namespace Zero
