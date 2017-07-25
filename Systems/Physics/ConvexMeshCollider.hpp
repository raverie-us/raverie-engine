///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class ConvexMesh;

/// Defines the collision volume for a convex mesh (ConvexMesh resource).
/// This collider has a volume and hence is able to work with a RigidBody.
class ConvexMeshCollider : public Collider
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ConvexMeshCollider();

  // Component interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initalizer) override;
  void DebugDraw() override;

  // Collider Interface
  void ComputeWorldAabbInternal() override;
  real ComputeWorldVolumeInternal() override;
  void ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia) override;
  void RebuildModifiedResources() override;
  
  Vec3 GetColliderLocalCenterOfMass() const override;
  void Support(Vec3Param direction, Vec3Ptr support) const override;
  void GetCenter(Vec3Ref center) const override;

  /// The convex mesh resource that defines the collision volume of this collider.
  ConvexMesh* GetConvexMesh();
  void SetConvexMesh(ConvexMesh* convexMesh);
  void OnMeshModified(Event* e);

  //-------------------------------------------------------------------Internal

  /// Used to tell the collision system that this collider stores information in local space.
  /// This means that the passed in aabb for GetOverlapRange should be transformed to local space.
  typedef true_type RangeInLocalSpace;
  /// Used in the collision system. @JoshD: Maybe replace with AutoDeclare later?
  typedef Physics::MeshFilterRange RangeType;
  /// Returns a range of local-space triangles that overlap the passed in local-space aabb.
  RangeType GetOverlapRange(Aabb& localAabb);

  HandleOf<ConvexMesh> mConvexMesh;
};

}//namespace Zero
