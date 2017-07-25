///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2013-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Mostly internal states used by point effects.
DeclareBitField6(PointFlags, UseMaxDistance, LinearInterpolation, QuadraticInterpolation, ClampToMax, ContinueFalloff, NoEffect);

//-------------------------------------------------------------------BasicPointEffect
/// Common interface for all point effects. Used to attract or repel objects towards 
/// a central point. The strength of the effect is calculated as an interpolation between 
/// two strength values at two radial distances.
class BasicPointEffect : public PhysicsEffect
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  BasicPointEffect();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void DebugDraw() override;

  // PhysicsEffect Interface
  void PreCalculate(real dt) override;

  /// Update the cached world space position of the origin of this point effect.
  void ComputeApplicationPoint();
  /// Returns the force strength that would be applied at a given radial distance.
  /// A positive value points away from the effect center.
  float GetForceStrenthAtDistance(float distance);
  /// Helper that returns the force that would be applied at the given world position.
  Vec3 GetForceAppliedAtPoint(Vec3Param point);
  /// Calculates the point force for a rigid body. The resultant force direction
  /// will point towards the effect center assuming the strength values and mInwardDirectionScalar
  /// are positive. To be used by any inheriting classes so all the core logic is in one place.
  Vec3 CalculateInwardForce(RigidBody* obj);

  /// Used to get the strength of the force from the t value that
  /// represents the interpolant between the min and max distance.
  real GetStrengthValue(real t);

  // Properties

  /// The first distance at which attenuation will start. If an object is
  /// under the min distance, StrengthAtMin will be used.
  /// If an object is in between min and max, then it will attenuate.
  real GetMinDistance();
  void SetMinDistance(real distance);
  /// The max distance that attenuation will happen at. If an object is
  /// between min and max distance, the value will be attenuated. If the
  /// object is further away, the effect strength will be determined by EndCondition.
  real GetMaxDistance();
  void SetMaxDistance(real distance);
  /// The strength that this effect applies at the min distance.
  real GetStrengthAtMin();
  void SetStrengthAtMin(real strength);
  /// The strength that this effect applies at the max distance.
  real GetStrengthAtMax();
  void SetStrengthAtMax(real strength);
  /// The offset from the transform position (in local space)
  /// that the point effect will be applied at.
  Vec3 GetLocalPositionOffset();
  void SetLocalPositionOffset(Vec3Param offset);
  /// How the interpolation should be handled after max distance. ClampToMax
  /// will clamp to StrengthAtMax. NoEffect will ignore the effect.
  /// ContinueFalloff will continue the interpolation (this may go negative).
  PhysicsEffectEndCondition::Enum GetEndCondition();
  void SetEndCondition(PhysicsEffectEndCondition::Enum condition);
  /// The type of interpolation used (e.g. Linear, Quadratic) for the effect.
  PhysicsEffectInterpolationType::Enum GetInterpolationType();
  void SetInterpolationType(PhysicsEffectInterpolationType::Enum type);

protected:

  real mMinDistance;
  real mMaxDistance;

  real mMinStrength;
  real mMaxStrength;

  // What direction this force points (positive points away from the effect center)
  real mOutwardDirectionScalar;
  Vec3 mLocalPositionOffset;
  Vec3 mWorldPoint;
  BitField<PointFlags::Enum> mPointFlags;
};

//-------------------------------------------------------------------PointForceEffect
/// A force effect with a direction and strength based upon the distance from
/// a central point. The direction of the force will always point away from the
/// effect center, but the strength will vary depending on the min/max distance
/// and strength values. Positive strength values point away from the effect center.
/// Useful to make planetary force field like effects.
class PointForceEffect : public BasicPointEffect
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PointForceEffect();

  // PhysicsEffect interface
  void ApplyEffect(RigidBody* obj, real dt) override;
};

//-------------------------------------------------------------------PointGravityEffect
/// A force effect that pulls an object towards a central point. This effect is
/// nearly identical to PointForceEffect with two exceptions. 1. An acceleration is
/// applied instead of a force (mass is ignored). 2. A positive strength value will
/// pull objects toward the center of the effect. This is useful to make planetary
/// gravity or other similar effects.
class PointGravityEffect : public BasicPointEffect
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PointGravityEffect();

  // PhysicsEffect interface
  void ApplyEffect(RigidBody* obj, real dt) override;
};

}//namespace Zero
