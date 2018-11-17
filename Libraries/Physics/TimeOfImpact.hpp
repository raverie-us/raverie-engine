///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Nathan Carlson
/// Copyright 2015-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct TimeOfImpactData
{
  TimeOfImpactData(Collider* colliderA, Collider* colliderB, real dt, Vec3 velocity = Vec3::cZero, bool linearSweep = false)
    : ColliderA(colliderA)
    , ColliderB(colliderB)
    , Dt(dt)
    , Velocity(velocity)
    , LinearSweep(linearSweep)
    , Steps(0)
  {
  }

  Collider* ColliderA;
  Collider* ColliderB;
  real Dt;
  Vec3 Velocity;
  /// Only sweep the first collider's linear velocity (no angular).
  /// This sweep currently comes from ContinuousCollider().
  bool LinearSweep;
  Array<real> ImpactTimes;
  Array<Physics::Manifold> Manifolds;

  int Steps;
};

void TimeOfImpact(TimeOfImpactData* data);

} // namespace Zero
