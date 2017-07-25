///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Applies a wind force in a given direction. The wind force is calculated from
/// the squared wind speed and is scaled by the approximate surface area of the 
/// object in the direction of the force.
class WindEffect : public PhysicsEffect
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  WindEffect();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void DebugDraw() override;

  // Physics Effect Interface
  void PreCalculate(real dt);
  void ApplyEffect(RigidBody* obj, real dt) override;
  void ApplyEffect(SpringSystem* obj, real dt) override;

  // Helper
  void ApplyEffect(Collider& collider, RigidBody* obj, real dt);

  /// The speed that the wind is blowing.
  real GetWindSpeed() const;
  void SetWindSpeed(real speed);
  /// The direction that the wind is blowing.
  Vec3 GetWindDirection() const;
  void SetWindDirection(Vec3Param direction);
  /// The direction of the wind in world space.
  Vec3 GetWorldWindDirection() const;

private:
  /// Calculates and returns the wind velocity.
  Vec3 GetWindVelocity();

  /// The speed of the wind.
  real mWindSpeed;
  
  /// The direction of the wind.
  Vec3 mWindDirection;
  Vec3 mWorldWindDirection;
  /// Determines if the wind's direction is a local or world-space vector.
  bool mLocalSpaceDirection;
};

}//namespace Zero
