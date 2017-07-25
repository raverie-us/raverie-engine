///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class PhysicsMesh;
struct ProxyResult;
struct BaseCastFilter;

/// Defines the collision for a generic mesh from a collection of triangles (PhysicsMesh resource).
/// This collider type is not expected to have a dynamic or kinematic RigidBody.
class MeshCollider : public Collider
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MeshCollider();

  // Component interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void DebugDraw() override;
  
  // Collider Interface
  void ComputeWorldAabbInternal() override;
  real ComputeWorldVolumeInternal() override;
  void ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia) override;
  void Support(Vec3Param direction, Vec3Ptr support) const override;
  void RebuildModifiedResources() override;

  /// The mesh resource used to define collision. This mesh is just a
  /// surface mesh of triangles (no volume is defined).
  PhysicsMesh* GetPhysicsMesh();
  void SetPhysicsMesh(PhysicsMesh* physicsMesh);
  void OnMeshModified(Event* e);

  //-------------------------------------------------------------------Internal

  /// Used to tell the collision system that this collider stores information in local space.
  /// This means that the passed in aabb for GetOverlapRange should be transformed to local space.
  typedef true_type RangeInLocalSpace;
  /// Used in the collision system. @JoshD: Maybe replace with AutoDeclare later?
  typedef Physics::MeshPreFilteredRange RangeType;
  /// Returns a range of local-space triangles that overlap the passed in local-space aabb.
  RangeType GetOverlapRange(Aabb& localAabb);

  /// This is a specialization of Ray vs. HeightMap that goes through the internal mid-phase with an
  /// optimized ray-casting algorithm instead of the generic GetOverlapAabb function.
  /// Note: the ray here is expected to be in this cog's local space.
  bool Cast(const Ray& localRay, ProxyResult& result, BaseCastFilter& filter);

private:

  /// Whether to debug draw the edges of each triangle.
  bool mDrawEdges;
  /// Whether to debug draw the faces of each triangle.
  bool mDrawFaces;
  /// Whether to debug draw the normals of each triangle.
  bool mDrawFaceNormals;

  HandleOf<PhysicsMesh> mPhysicsMesh;
};

}//namespace Zero
