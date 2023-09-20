// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(ColliderInspector, builder, type)
{
  RaverieBindComponent();
  RaverieBindDependency(Collider);
  RaverieBindSetup(SetupMode::DefaultSerialization);

  RaverieBindMethod(GetColliderLocalCenterOfMass);
  RaverieBindMethod(ComputeMass);
  RaverieBindMethod(GetLocalInverseInertiaTensorRow0);
  RaverieBindMethod(GetLocalInverseInertiaTensorRow1);
  RaverieBindMethod(GetLocalInverseInertiaTensorRow2);
  type->AddAttribute(::Raverie::ObjectAttributes::cDoNotDocument);
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

} // namespace Raverie
