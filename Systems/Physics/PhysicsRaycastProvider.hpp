///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Provides the interface for casting through PhysicsSpaces.
class PhysicsRaycastProvider : public RaycastProvider
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PhysicsRaycastProvider();

  void RayCast(Ray& ray, CastInfo& castInfo, RaycastResultList& results) override;
  void FrustumCast(Frustum& frustum, CastInfo& castInfo, RaycastResultList& results) override;

  /// Should dynamic colliders (those with rigid bodies) be selected?
  bool mDynamicColliders;
  /// Should ghost colliders be selected?
  bool mSelectGhosts;
  /// Should static colliders (those without rigid bodies) be selected?
  bool mStaticColliders;
  /// Should multi-selection work with static objects?
  bool mMultiSelectStatic;
  /// Should multi-selection work with kinematic objects?
  bool mMultiSelectKinematic;
};

}//namespace Zero
