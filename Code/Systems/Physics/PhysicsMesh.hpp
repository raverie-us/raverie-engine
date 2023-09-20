// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// A mesh used to represent static world geometry. All geometry is
/// stored and tested as a collection of triangles.
class PhysicsMesh : public GenericPhysicsMesh
{
public:
  RaverieDeclareType(PhysicsMesh, TypeCopyMode::ReferenceType);
  typedef StaticAabbTree<uint> AabbTree;

  // Interface
  void Serialize(Serializer& stream) override;
  void Initialize();
  void Unload() override;
  void OnResourceModified() override;

  HandleOf<Resource> Clone() override;
  /// Creates a clone of this mesh for run-time modifications.
  HandleOf<PhysicsMesh> RuntimeClone();
  /// Creates a PhysicsMesh for run-time modifications.
  static HandleOf<PhysicsMesh> CreateRuntime();

  // GenericPhysicsMesh Interface
  void ForceRebuild() override;
  void RebuildMidPhase() override;
  void GenerateInternalEdgeData() override;

  /// Finds the first triangle hit by the local-space ray.
  bool CastRay(const Ray& localRay, ProxyResult& result, BaseCastFilter& filter);
  /// Fills out the given array with all overlapping triangles.
  void GetOverlappingTriangles(Aabb& aabb, TriangleArray& triangles, Array<uint>& triangleIds);

  /// Copy all relevant info for runtime clone.
  void CopyTo(PhysicsMesh* destination);
  /// Returns the mesh's Aabb tree.
  StaticAabbTree<uint>* GetAabbTree();

private:
  void GenerateTree();

  /// Aabb Tree used for fast ray casts and triangle lookups.
  StaticAabbTree<uint> mTree;
};

class PhysicsMeshManager : public ResourceManager
{
public:
  DeclareResourceManager(PhysicsMeshManager, PhysicsMesh);
  PhysicsMeshManager(BoundType* resourceType);

  void UpdateAndNotifyModifiedResources();

  typedef HandleOf<PhysicsMesh> MeshReference;
  Array<MeshReference> mModifiedMeshes;
};

} // namespace Raverie
