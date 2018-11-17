///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------PhysicsMesh
DefinePhysicsRuntimeClone(PhysicsMesh);

ZilchDefineType(PhysicsMesh, builder, type)
{
  ZeroBindDocumented();
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();
  ZeroBindTag(Tags::Physics);

  ZilchBindMethod(CreateRuntime);
  ZilchBindMethod(RuntimeClone);
}

void PhysicsMesh::Serialize(Serializer& stream)
{
  GenericPhysicsMesh::Serialize(stream);
  SerializeAabbTree(stream, mTree);
}

void PhysicsMesh::Initialize()
{
  GenericPhysicsMesh::Initialize();
}

void PhysicsMesh::Unload()
{
  GenericPhysicsMesh::Unload();
  mTree.DeleteTree();
}

void PhysicsMesh::OnResourceModified()
{
  PhysicsMeshManager* manager = (PhysicsMeshManager*)GetManager();
  manager->mModifiedMeshes.PushBack(PhysicsMeshManager::MeshReference(this));
}

HandleOf<PhysicsMesh> PhysicsMesh::CreateRuntime()
{
  return PhysicsMeshManager::CreateRuntime();
}

void PhysicsMesh::ForceRebuild()
{
  GenericPhysicsMesh::ForceRebuild();
}

void PhysicsMesh::RebuildMidPhase()
{
  GenerateTree();
}

void PhysicsMesh::GenerateInternalEdgeData()
{
  GenerateInternalEdgeInfo(this, &mInfoMap);
}

bool PhysicsMesh::CastRay(const Ray& localRay, ProxyResult& result, BaseCastFilter& filter)
{
  bool triangleHit = false;
  result.mTime = Math::PositiveMax();

  // Query the aabb tree for possible triangles. Test all triangles whose aabbs we hit.
  forRangeBroadphaseTree(StaticAabbTree<uint>, mTree, Ray, localRay)
  {
    uint triIndex = range.Front();
    Triangle tri = GetTriangle(triIndex);
    triangleHit |= CastRayTriangle(localRay, tri, triIndex, result, filter);
  }

  return triangleHit;
}

void PhysicsMesh::GetOverlappingTriangles(Aabb& aabb, TriangleArray& triangles, Array<uint>& triangleIds)
{
  forRangeBroadphaseTree(StaticAabbTree<uint>, mTree, Aabb, aabb)
  {
    // Get the triangle index
    uint triIndex = range.Front();
    triangles.PushBack(GetTriangle(triIndex));
    triangleIds.PushBack(triIndex);
  }
}

void PhysicsMesh::CopyTo(PhysicsMesh* destination)
{
  GenericPhysicsMesh::CopyTo(destination);
  ForceRebuild();
}

StaticAabbTree<uint>* PhysicsMesh::GetAabbTree()
{
  return &mTree;
}

void PhysicsMesh::GenerateTree()
{
  // Clear the old tree
  mTree.DeleteTree();

  // Build the Aabb-Tree
  mTree.SetPartitionMethod(PartitionMethods::MinimizeVolumeSum);

  // Dummy proxy. They will not be needed.
  BroadPhaseProxy proxy;

  size_t triangleCount = GetTriangleCount();
  for(size_t triIndex = 0; triIndex < triangleCount; ++triIndex)
  {
    Triangle tri = GetTriangle(triIndex);
    
    // Create the broad phase data.
    BaseBroadPhaseData<uint> data;
    data.mClientData = triIndex;
    data.mAabb = ToAabb(tri);

    // Insert it into the tree
    mTree.CreateProxy(proxy, data);
  }

  // Construct the tree
  mTree.Construct();
}

//-------------------------------------------------------------------PhysicsMeshManager
ImplementResourceManager(PhysicsMeshManager, PhysicsMesh);

PhysicsMeshManager::PhysicsMeshManager(BoundType* resourceType) 
  : ResourceManager(resourceType)
{
  AddLoader("PhysicsMesh", new BinaryDataFileLoader<PhysicsMeshManager>());
  mCategory = "Physics";
  mCanAddFile = true;
  AddGeometryFileFilters(this);
  DefaultResourceName = "Box";
  mExtension = "physmesh";
  mCanReload = true;
  mCanDuplicate = true;
  mCanCreateNew = true;
}

void PhysicsMeshManager::UpdateAndNotifyModifiedResources()
{
  for(size_t i = 0; i < mModifiedMeshes.Size(); ++i)
  {
    PhysicsMesh* mesh = mModifiedMeshes[i];
    if(mesh != nullptr)
      mesh->UpdateAndNotifyIfModified();
  }
  mModifiedMeshes.Clear();
}

}//namespace Zero
