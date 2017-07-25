///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2012-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------BuoyancyEffect
/// Applies a buoyancy force to an object in a given direction.
class BuoyancyEffect : public PhysicsEffect
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  BuoyancyEffect();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  // Physics Effect Interface
  void ApplyEffect(RigidBody* obj, real dt) override;

  uint GetDetail();
  void SetDetail(uint detail);

private:
  float ComputeOverlapPercent(Collider* collider, Vec3& volumeCenter);
  bool OverlapsRegion(Collider* object);
  bool PointInObject(Collider* object, Vec3Param point);

  Collider* mCollider;

  /// Draw the points used to compute the buoyancy during run-time.
  /// This helps in debugging, but should be left off during normal run.
  bool mDebugDrawRuntime;

  /// The amount of points to subdivide each object
  /// into for sampling (total points is Detail^3)
  uint mDetail;

  /// The density of the fluid.
  real mDensity;

  /// The direction of gravity in world-space.
  Vec3 mGravity;
};
  
}// namespace Zero
