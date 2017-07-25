///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(ColliderInspector, builder, type)
{
  ZeroBindComponent();
  ZeroBindDependency(Collider);
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindMethod(GetColliderLocalCenterOfMass);
  ZilchBindMethod(ComputeMass);
  ZilchBindMethod(GetLocalInverseInertiaTensorRow0);
  ZilchBindMethod(GetLocalInverseInertiaTensorRow1);
  ZilchBindMethod(GetLocalInverseInertiaTensorRow2);
}

Vec3 ColliderInspector::GetColliderLocalCenterOfMass() const
{
  return GetOwner()->has(Collider)->GetColliderLocalCenterOfMass();
}

real ColliderInspector::ComputeMass() const
{
  return GetOwner()->has(Collider)->ComputeMass();
}

Vec3 ColliderInspector::GetLocalInverseInertiaTensorRow0() const
{
  return GetInertia()[0];
}

Vec3 ColliderInspector::GetLocalInverseInertiaTensorRow1() const
{
  return GetInertia()[1];
}

Vec3 ColliderInspector::GetLocalInverseInertiaTensorRow2() const
{
  return GetInertia()[2];
}

Mat3 ColliderInspector::GetInertia() const
{
  Collider* collider = GetOwner()->has(Collider);
  real mass = ComputeMass();
  Mat3 inertia;
  collider->ComputeLocalInverseInertiaTensor(mass, inertia);
  return inertia;
}

} // namespace Zero
