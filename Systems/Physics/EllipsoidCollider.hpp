///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Defines the collision volume for an ellipsoid (3 dimensional ellipse) defined by three radius values.
class EllipsoidCollider : public Collider
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  EllipsoidCollider();

  // Component interface
  void Serialize(Serializer& stream) override;
  void DebugDraw() override;

  // Collider Interface
  void ComputeWorldAabbInternal() override;
  void ComputeWorldBoundingSphereInternal() override;
  real ComputeWorldVolumeInternal() override;
  void ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia) override;
  
  void Support(Vec3Param direction, Vec3Ptr support) const override;

  /// The x, y, and z radius of the ellipsoid.
  Vec3 GetRadii() const;
  void SetRadii(Vec3Param radii);

  /// The radii of the ellipsoid after transform is applied (scale and rotation).
  Vec3 GetWorldRadii() const;

private:
  Vec3 mRadii;
};

}//namespace Zero
