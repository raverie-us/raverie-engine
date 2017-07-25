///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// How should a capsule collider respond to non-uniform scale?
/// <param name="PreserveHeight">Scale applies to the height of the cylinder.</param>
/// <param name="PreserveScale">Scale applies to the total size of the capsule.</param>
DeclareEnum2(CapsuleScalingMode, PreserveHeight, PreserveScale);

/// Defines the collision volume for a capsule defined by a height and radius. A capsule can
/// be thought of as a cylinder with spherical end caps.
class CapsuleCollider : public Collider
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CapsuleCollider();
  
  // Component interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void DebugDraw() override;
  
  // Collider Interface
  void CacheWorldValues() override;
  void ComputeWorldAabbInternal() override;
  void ComputeWorldBoundingSphereInternal() override;
  real ComputeWorldVolumeInternal() override;
  void ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia) override;
  void Support(Vec3Param direction, Vec3Ptr support) const override;
  
  /// The local space radius of the spheres at the capsule edges.
  real GetRadius() const;
  void SetRadius(real radius);
  /// The local space distance from the center of one sphere to another.
  real GetHeight() const;
  void SetHeight(real height);
  /// The direction that the height is defined along. Allows the user to
  /// change whether the capsule's height is along the local-space x, y, or z axis.
  AxisDirection::Enum GetDirection() const;
  void SetDirection(AxisDirection::Enum direction);
  /// How should non-uniform scale affect the capsules size. Should a scale of 2
  /// on the height axis double the total capsule size or should it double the capsule height?
  CapsuleScalingMode::Enum GetScalingMode() const;
  void SetScalingMode(CapsuleScalingMode::Enum mode);

  //-------------------------------------------------------------------Internal
  /// The index of both radius axes.
  void GetRadiiIndices(uint& rIndex0, uint& rIndex1) const;
  /// The index of the height axis.
  uint GetHeightIndex() const;

  /// The radius of the sphere caps after scale is applied.
  real GetWorldRadius() const;
  /// The half height of the capsule's cylinder after scale is applied (distance from center to a sphere radius).
  real GetWorldCylinderHalfHeight() const;
  /// The full height of the capsule's cylinder after the scale is applied (world distance from one sphere to another).
  real GetWorldCylinderHeight() const;

  /// Computes the center points of both sphere caps.
  void ComputeWorldPoints(Vec3Ref pointA, Vec3Ref pointB) const;

private:
  real mRadius;
  real mHeight;

  real mWorldRadius;
  real mWorldCylinderHeight;

  AxisDirection::Enum mDirection;
  CapsuleScalingMode::Enum mScalingMode;
};

}//namespace Zero
