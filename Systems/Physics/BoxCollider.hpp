//////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Defines the collision volume of a box defined by a size on each axis.
class BoxCollider : public Collider
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  BoxCollider();

  // Component interface
  void Serialize(Serializer& stream) override;
  void DebugDraw() override;

  // Collider Interface
  void CacheWorldValues() override;
  void ComputeWorldAabbInternal() override;
  real ComputeWorldVolumeInternal() override;
  void ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia) override;
  void Support(Vec3Param direction, Vec3Ptr support) const override;
  
  /// The half size (from the center to the upper-right corner) on each axis of the box in local space.
  /// Used to make the box's size match a model or some other volume without
  /// needing to scale the transform (also avoids non-uniform scale issues).
  Vec3 GetHalfSize();
  void SetHalfSize(Vec3Param localHalfSize);
  /// The size (from min to max) on each axis of the box in local space.
  /// Used to make the box's size match a model or some other volume without
  /// needing to scale the transform (also avoids non-uniform scale issues).
  Vec3 GetSize() const;
  void SetSize(Vec3Param localSize);

  /// The size of the box after the transform is applied (scale and rotation).
  Vec3 GetWorldSize() const;

private:
  static const Vec3 mMinAllowedSize;
  static const Vec3 mMaxAllowedSize;

  /// The local size of the box (half extents).
  Vec3 mLocalHalfSize;
  Vec3 mWorldHalfSize;
};

}//namespace Zero
