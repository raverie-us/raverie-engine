///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Benjamin Strukus, Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------ ConvexMesh
DefinePhysicsRuntimeClone(ConvexMesh);

ZilchDefineType(ConvexMesh, builder, type)
{
  ZeroBindDocumented();
  ZilchBindConstructor();
  ZilchBindDestructor();

  ZeroBindTag(Tags::Physics);

  ZilchBindMethod(CreateRuntime);
  ZilchBindMethod(RuntimeClone);
}

void ConvexMesh::Serialize(Serializer& stream)
{
  GenericPhysicsMesh::Serialize(stream);
  stream.SerializeFieldDefault("Volume", mLocalVolume, real(1));
  stream.SerializeFieldDefault("CenterOfMass", mLocalCenterOfMass, Vec3::cZero);
}

void ConvexMesh::Initialize(void)
{
  GenericPhysicsMesh::Initialize();
  BuildFromPointSet(mVertices);
}

void ConvexMesh::OnResourceModified()
{
  ConvexMeshManager* manager = (ConvexMeshManager*)GetManager();
  manager->mModifiedMeshes.PushBack(ConvexMeshManager::ConvexMeshReference(this));
}

HandleOf<ConvexMesh> ConvexMesh::CreateRuntime()
{
  return ConvexMeshManager::CreateRuntime();
}

bool ConvexMesh::CastRay(const Ray& localRay, ProxyResult& result, BaseCastFilter& filter)
{
  return GenericPhysicsMesh::CastRayGeneric(localRay, result, filter);
}

void ConvexMesh::Draw(Mat4Param transform)
{
  //Debug::DefaultConfig config;
  //config.Alpha(100).Alpha(100).Border(true).OnTop(false);

  DrawFaces(transform, Color::Lime);
}

void ConvexMesh::BuildFromPointSet(const Vec3Array& points)
{
  const Vec3* p = &(points[0]);

  Geometry::Hull3D hull;
  hull.Build(p, uint(points.Size()));

  Geometry::ConvexMesh geometryMesh;
  geometryMesh.CopyInfo(hull);

  uint count = geometryMesh.GetVertexCount();
  mVertices.Resize(count);
  for(uint i = 0; i < count; ++i)
  {
    mVertices[i] = geometryMesh.GetVertex(i).Position;
  }

  count = geometryMesh.GetFaceCount();
  mIndices.Resize(count * 3);
  for(uint i = 0; i < count; ++i)
  {
    uint a = i * 3;
    uint b = (i * 3) + 1;
    uint c = (i * 3) + 2;

    mIndices[a] = geometryMesh.GetFace(i).Points[0];
    mIndices[b] = geometryMesh.GetFace(i).Points[1];
    mIndices[c] = geometryMesh.GetFace(i).Points[2];
  }
}

//---------------------------------------------------------- ConvexMeshManager
ImplementResourceManager(ConvexMeshManager, ConvexMesh);

ConvexMeshManager::ConvexMeshManager(BoundType* resourceType)
  :ResourceManager(resourceType)
{
  AddLoader("ConvexMesh", new BinaryDataFileLoader<ConvexMeshManager>());
  mCategory = "Physics";
  mCanAddFile = true;
  AddGeometryFileFilters(this);
  DefaultResourceName = "Cube";
  mExtension = "convexmesh";
  mCanCreateNew = true;
  mCanDuplicate = true;
}

void ConvexMeshManager::UpdateAndNotifyModifiedResources()
{
  for(size_t i = 0; i < mModifiedMeshes.Size(); ++i)
  {
    ConvexMesh* mesh = mModifiedMeshes[i];
    if(mesh != nullptr)
      mesh->UpdateAndNotifyIfModified();
  }
  mModifiedMeshes.Clear();
}

}//namespace Zero
