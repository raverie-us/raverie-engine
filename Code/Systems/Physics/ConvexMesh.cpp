// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Foundation/Geometry/QuickHull3D.hpp"

namespace Raverie
{

DefinePhysicsRuntimeClone(ConvexMesh);

RaverieDefineType(ConvexMesh, builder, type)
{
  RaverieBindDocumented();
  RaverieBindConstructor();
  RaverieBindDestructor();

  RaverieBindTag(Tags::Physics);

  RaverieBindMethod(CreateRuntime);
  RaverieBindMethod(RuntimeClone);
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
  // Debug::DefaultConfig config;
  // config.Alpha(100).Alpha(100).Border(true).OnTop(false);

  DrawFaces(transform, Color::Lime);
}

void ConvexMesh::BuildFromPointSet(const Vec3Array& points)
{
  typedef QuickHull3D::QuickHullVertex Vertex;
  typedef QuickHull3D::QuickHullEdge Edge;
  typedef QuickHull3D::QuickHullFace Face;
  typedef QuickHull3D::EdgeList EdgeList;
  typedef QuickHull3D::FaceList FaceList;

  QuickHull3D hull3D;
  bool success = hull3D.Build(points);
  if (!success)
    return;

  mVertices.Clear();
  mIndices.Clear();

  size_t vertexCount = hull3D.ComputeVertexCount();
  mVertices.Resize(vertexCount);

  int currentVertexId = 0;
  HashMap<Vertex*, int> vertexIdMap;
  Array<int> faceVertexIndices;
  for (FaceList::range faces = hull3D.GetFaces(); !faces.Empty(); faces.PopFront())
  {
    // Clear the list of vertices for this face
    faceVertexIndices.Clear();

    Face* face = &faces.Front();
    for (EdgeList::range edges = face->mEdges.All(); !edges.Empty(); edges.PopFront())
    {
      Vertex* vertex = edges.Front().mTail;
      int vertexId = 0;
      // If we haven't seen this vertex then add it to the final list of
      // vertices and give it a new id
      if (!vertexIdMap.ContainsKey(vertex))
      {
        vertexIdMap[vertex] = currentVertexId;
        mVertices[currentVertexId] = vertex->mPosition;
        vertexId = currentVertexId;
        ++currentVertexId;
      }
      // Otherwise grab the id it's already been assigned
      else
        vertexId = vertexIdMap[vertex];

      faceVertexIndices.PushBack(vertexId);
    }
    // Create a triangle fan for the vertices in this face
    for (size_t i = 2; i < faceVertexIndices.Size(); ++i)
    {
      mIndices.PushBack(faceVertexIndices[0]);
      mIndices.PushBack(faceVertexIndices[i - 1]);
      mIndices.PushBack(faceVertexIndices[i]);
    }
  }
}

ImplementResourceManager(ConvexMeshManager, ConvexMesh);

ConvexMeshManager::ConvexMeshManager(BoundType* resourceType) : ResourceManager(resourceType)
{
  AddLoader("ConvexMesh", new BinaryDataFileLoader<ConvexMeshManager>());
  mCategory = "Physics";
  mCanAddFile = true;
  AddGeometryFileFilters(this);
  DefaultResourceName = "Cube";
  mExtension = "convexmesh";
  mCanReload = true;
  mCanCreateNew = true;
  mCanDuplicate = true;
}

void ConvexMeshManager::UpdateAndNotifyModifiedResources()
{
  for (size_t i = 0; i < mModifiedMeshes.Size(); ++i)
  {
    ConvexMesh* mesh = mModifiedMeshes[i];
    if (mesh != nullptr)
      mesh->UpdateAndNotifyIfModified();
  }
  mModifiedMeshes.Clear();
}

} // namespace Raverie
