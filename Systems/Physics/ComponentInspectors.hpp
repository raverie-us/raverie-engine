///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Component to inspect various properties on a collider that the user
// shouldn't have access to. Used to test things with developer config.
class ColliderInspector : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Vec3 GetColliderLocalCenterOfMass() const;
  real ComputeMass() const;
  Vec3 GetLocalInverseInertiaTensorRow0() const;
  Vec3 GetLocalInverseInertiaTensorRow1() const;
  Vec3 GetLocalInverseInertiaTensorRow2() const;

  //-------------------------------------------------------------------Internal
  Mat3 GetInertia() const;
};

} // namespace Zero
