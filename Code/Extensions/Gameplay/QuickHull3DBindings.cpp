// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "IndexedHalfEdgeMesh.hpp"
#include "QuickHull3DBindings.hpp"

namespace Raverie
{

RaverieDefineType(QuickHull3DInterface, builder, type)
{
  RaverieBindDefaultCopyDestructor();
  type->CreatableInScript = true;

  RaverieBindMethod(Add);
  RaverieBindMethod(Build);
  RaverieBindMethod(Clear);
  RaverieBindMethod(Draw);
  RaverieBindField(mIndex);
  RaverieBindField(mShowDebugDraw);
  RaverieBindField(mMesh);
}

QuickHull3DInterface::QuickHull3DInterface()
{
  mMesh = new IndexedHalfEdgeMesh();
}

QuickHull3DInterface::QuickHull3DInterface(const QuickHull3DInterface& rhs)
{
  mIndex = rhs.mIndex;
  mShowDebugDraw = rhs.mShowDebugDraw;
  mPoints = rhs.mPoints;
  // Don't build the mesh. There was no guarantee they had
  // already built so for this on the user for now?
}

QuickHull3DInterface::~QuickHull3DInterface()
{
  mQuickHull3D.Clear();
  mMesh = nullptr;
}

void QuickHull3DInterface::Add(Vec3Param point)
{
  mPoints.PushBack(point);
}

bool QuickHull3DInterface::Build()
{
  // Clear the old debug/mesh information
  mDebugDrawStack.Clear();
  mMesh->Clear();

  // If we don't debug draw then pass null to quick-hull
  DebugDrawStack* debugStack = &mDebugDrawStack;
  if (!mShowDebugDraw)
    debugStack = nullptr;

  bool result = mQuickHull3D.Build(mPoints, debugStack);
  // If quick-hull succeeded then build the half-edge mesh
  if (result)
    BuildHalfEdgeMesh();
  return result;
}

void QuickHull3DInterface::Clear()
{
  mPoints.Clear();
}

void QuickHull3DInterface::Draw()
{
  mDebugDrawStack.Draw(mIndex);
}

void QuickHull3DInterface::BuildHalfEdgeMesh()
{
  typedef QuickHull3D::QuickHullVertex Vertex;
  typedef QuickHull3D::QuickHullEdge Edge;
  typedef QuickHull3D::QuickHullFace Face;
  typedef QuickHull3D::VertexList VertexList;
  typedef QuickHull3D::EdgeList EdgeList;
  typedef QuickHull3D::FaceList FaceList;

  // Compute how much memory is required
  int vertexCount = mQuickHull3D.ComputeVertexCount();
  int halfEdgeCount = mQuickHull3D.ComputeHalfEdgeCount();
  int faceCount = mQuickHull3D.ComputeFaceCount();

  // Cache the arrays we're going to fill out
  IndexedHalfEdgeMesh* mesh = mMesh;
  Array<Vec3>& outMeshVertices = mesh->mVertices;
  Array<IndexedHalfEdge*>& outMeshEdges = mesh->mEdges;
  Array<IndexedHalfEdgeFace*>& outMeshFaces = mesh->mFaces;

  // Allocate all the memory we need
  mMesh->Create(vertexCount, halfEdgeCount, faceCount);

  // Build up point to id mappings. Use an ordered hash-map for determinism.
  typedef OrderedHashMap<Vertex*, int> VertexMap;
  typedef OrderedHashMap<Edge*, int> EdgeMap;
  typedef OrderedHashMap<Face*, int> FaceMap;
  VertexMap mVertexIds;
  EdgeMap mEdgeIds;
  FaceMap mFaceIds;
  int vertexId = 0;
  int edgeId = 0;
  int faceId = 0;

  // Compute the ids of all faces, edges, and vertices
  for (FaceList::range faces = mQuickHull3D.GetFaces(); !faces.Empty(); faces.PopFront())
  {
    Face* face = &faces.Front();
    mFaceIds[face] = faceId;
    ++faceId;

    for (EdgeList::range edges = face->mEdges.All(); !edges.Empty(); edges.PopFront())
    {
      Edge* edge = &edges.Front();
      mEdgeIds[edge] = edgeId;
      ++edgeId;

      // Check to see if this vertex has already been mapped
      if (!mVertexIds.ContainsKey(edge->mTail))
      {
        mVertexIds[edge->mTail] = vertexId;
        ++vertexId;
      }
    }
  }

  // Copy all vertices
  forRange (VertexMap::PairType& pair, mVertexIds.All())
  {
    outMeshVertices[pair.second] = pair.first->mPosition;
  }
  // Copy all edges, making sure to map the vertex, twin, and faces
  forRange (EdgeMap::PairType& pair, mEdgeIds.All())
  {
    IndexedHalfEdge* outEdge = outMeshEdges[pair.second];
    Edge* inEdge = pair.first;
    outEdge->mFaceIndex = mFaceIds[inEdge->mFace];
    outEdge->mVertexIndex = mVertexIds[inEdge->mTail];
    outEdge->mTwinIndex = mEdgeIds[inEdge->mTwin];
  }
  // Copy all faces, making sure to map all edges
  forRange (auto& pair, mFaceIds.All())
  {
    IndexedHalfEdgeFace* outFace = outMeshFaces[pair.second];
    Face* inFace = pair.first;

    for (EdgeList::range edges = inFace->mEdges.All(); !edges.Empty(); edges.PopFront())
    {
      Edge* edge = &edges.Front();
      outFace->mEdges.PushBack(mEdgeIds[edge]);
    }
  }
}

} // namespace Raverie
