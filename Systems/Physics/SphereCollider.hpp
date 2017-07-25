///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Defines the collision volume for a sphere defined by a radius.
class SphereCollider : public Collider
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SphereCollider();

  // Component interface
  void Serialize(Serializer& stream) override;
  void DebugDraw() override;

  // Collider Interface
  void CacheWorldValues() override;
  void ComputeWorldAabbInternal() override;
  void ComputeWorldBoundingSphereInternal() override;
  real ComputeWorldVolumeInternal() override;
  void ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia) override;
  void Support(Vec3Param direction, Vec3Ptr support) const override;

  /// The radius of the sphere in local space (before transform is applied).
  real GetRadius() const;
  void SetRadius(real radius);

  /// The radius of the sphere after transform is applied (scale).
  real GetWorldRadius() const;

private:

  static const real mMinAllowedRadius;
  static const real mMaxAllowedRadius;

  /// The local-space radius of the sphere
  real mRadius;
  /// The world-space radius of the sphere
  real mWorldRadius;
};

}//namespace Zero
