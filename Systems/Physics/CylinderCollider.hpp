///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Defines the collision volume for a cylinder defined by a height and radius.
class CylinderCollider : public Collider
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CylinderCollider();

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
  
  /// The local space radius of the cylinder.
  real GetRadius() const;
  void SetRadius(real radius);
  /// The local space distance from the top of the cylinder to the bottom.
  real GetHeight() const;
  void SetHeight(real height);
  /// The direction that the height is defined along. Allows the user to
  /// change whether the cylinder's height is along the x, y, or z axis.
  AxisDirection::Enum GetDirection() const;
  void SetDirection(AxisDirection::Enum direction);
  
  /// The index of both radius axes.
  void GetRadiiIndices(uint& rIndex0, uint& rIndex1) const;
  /// The index of the height axis.
  uint GetHeightIndex() const;

  /// The radius of the cylinder after scale is applied.
  real GetWorldRadius() const;
  /// The half height of the cylinder after scale is applied.
  real GetWorldHalfHeight() const;
  /// The full height of the cylinder after scale is applied.
  real GetWorldHeight() const;

  /// Returns the top (pointA), and the bottom (pointB) of the cylinder.
  void ComputeWorldPoints(Vec3Ref pointA, Vec3Ref pointB) const;

private:
  real mRadius;
  real mHeight;

  real mWorldRadius;
  real mWorldHeight;

  AxisDirection::Enum mDirection;
};

}//namespace Zero
