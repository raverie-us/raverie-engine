///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class MultiConvexMeshCollider;
struct ProxyResult;
struct BaseCastFilter;

//-------------------------------------------------------------------MultiConvexMeshRange
/// Range used to iterate through all sub convex meshes.
struct MultiConvexMeshRange
{
  struct ConvexMeshObject
  {
    void Support(Vec3Param direction, Vec3Ptr support) const;
    void GetCenter(Vec3Ref worldCenter) const;

    uint Index;
    ConvexMeshShape Shape;

    MultiConvexMeshCollider* mCollider;
  };

  MultiConvexMeshRange(MultiConvexMeshCollider* collider, const Aabb& worldAabb);

  // Range Interface
  ConvexMeshObject& Front();
  void PopFront();
  bool Empty();

  //-------------------------------------------------------------------Internal

  void SkipDead();

  /// The query aabb
  Aabb mLocalAabb;
  /// What sub-mesh we're currently looking at
  uint mIndex;
  ConvexMeshObject obj;

  MultiConvexMeshCollider* mCollider;
};

//-------------------------------------------------------------------MultiConvexMeshCollider
/// Defines a collection of sub-convex meshes. This allows a non-convex object to be broken
/// up into several convex pieces for efficient collision detection and use with rigid bodies.
class MultiConvexMeshCollider : public Collider
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MultiConvexMeshCollider();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void DebugDraw() override;

  // Collider Interface
  void ComputeWorldAabbInternal() override;
  real ComputeWorldVolumeInternal() override;
  void ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia) override;
  Vec3 GetColliderLocalCenterOfMass() const override;
  void RebuildModifiedResources() override;
  
  /// The MultiConvexMesh resource that defines collision.
  MultiConvexMesh* GetMesh();
  void SetMesh(MultiConvexMesh* mesh);

  //-------------------------------------------------------------------Internal

  void OnMeshModified(Event* e);

  /// Used to tell the collision system that this collider stores information in world space.
  typedef false_type RangeInLocalSpace;
  /// Used in the collision system. @JoshD: Maybe replace with AutoDeclare later?
  typedef MultiConvexMeshRange RangeType;
  /// Returns a range of world-space triangles that overlap the passed in local-space aabb.
  RangeType GetOverlapRange(Aabb& worldAabb);

  /// This is a specialization of Ray vs. HeightMap that goes through the internal mid-phase with an
  /// optimized ray-casting algorithm instead of the generic GetOverlapAabb function.
  /// Note: the ray here is expected to be in this cog's local space.
  bool Cast(const Ray& worldRay, ProxyResult& result, BaseCastFilter& filter);

  HandleOf<MultiConvexMesh> mMesh;
};

}//namespace Zero
