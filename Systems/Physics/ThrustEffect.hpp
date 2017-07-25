///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareBitField1(ThrustFlags, LocalSpaceDirection);

/// Applies a directional force at the thrust effect's center. When applied to a
/// rigid body, this will compute a torque if the force's direction does not go through
/// the center of mass. Useful for modeling any sort of a thruster.
class ThrustEffect : public PhysicsEffect
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ThrustEffect();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void DebugDraw() override;

  // Physics Effect Interface
  void PreCalculate(real dt) override;
  void ApplyEffect(RigidBody* obj, real dt) override;

  // Properties

  /// Determines if this force is to be applied in local or world space.
  bool GetLocalSpaceDirection() const;
  void SetLocalSpaceDirection(bool state);
  /// The strength of the force being applied in the force direction.
  real GetForceStrength() const;
  void SetForceStrength(real strength);
  /// The direction that the force should be applied.
  Vec3 GetForceDirection() const;
  void SetForceDirection(Vec3Param force);
  /// The direction that the force should be applied in world space.
  Vec3 GetWorldForceDirection() const;

private:
  BitField<ThrustFlags::Enum> mThrustFlags;
  
  real mForceStrength;
  Vec3 mForceDirection;

  // Cached world-space values
  Vec3 mWorldThrustCenter;
  Vec3 mWorldThrustDirection;
};

}//namespace Zero
