///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2013-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareBitField1(BasicForceFlags, LocalSpaceDirection);

//-------------------------------------------------------------------BasicDirectionEffect
/// Common interface for all directional force effects. Used to group
/// together all common logic/variables for the force/gravity variants.
class BasicDirectionEffect : public PhysicsEffect
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  BasicDirectionEffect();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void DebugDraw() override;

  // PhysicsEffect Interface
  void PreCalculate(real dt) override;

  // Properties

  /// Determines if the direction that the effect is applied is in local or
  /// world space. This vector is normalized when calculating forces.
  bool GetLocalSpaceDirection() const;
  void SetLocalSpaceDirection(bool state);
  /// The magnitude of the force to apply.
  real GetStrength() const;
  void SetStrength(real strength);
  /// The direction that the effect will be applied in (may be in local
  /// or world space depending on the LocalSpaceDirection property).
  Vec3 GetDirection() const;
  void SetDirection(Vec3Param force);

  /// The world direction of the effect. If the effect is not in local
  /// space then this is the same as Direction.
  Vec3 GetWorldDirection() const;

protected:
  /// Currently only used to store the local/world direction state.
  BitField<BasicForceFlags::Enum> mForceStates;
  
  real mStrength;
  Vec3 mDirection;
  Vec3 mWorldForce;
};

//-------------------------------------------------------------------ForceEffect
/// A force effect that is applied in a given direction (local or world space).
/// This is used to create force regions that will push objects in a given direction.
/// This can also be used on a rigid body to push an object in it's forward direction
/// (e.g a missile) if applied locally. Note: this is always applied at
/// the center of mass of the object. If a more rocket like effect is desired see ThrustEffect.
class ForceEffect : public BasicDirectionEffect
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ForceEffect();

  // PhysicsEffect Interface
  void ApplyEffect(RigidBody* obj, real dt) override;
};

//-------------------------------------------------------------------GravityEffect
/// A constant acceleration that is applied in the given direction (mass is ignored).
/// This is useful for creating gravity (either on the entire world or in a region)
/// that will push/pull objects in a given direction at a constant acceleration.
class GravityEffect : public BasicDirectionEffect
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  GravityEffect();

  // PhysicsEffect Interface
  void ApplyEffect(RigidBody* obj, real dt) override;
  void ApplyEffect(SpringSystem* obj, real dt) override;
};

}//namespace Zero
