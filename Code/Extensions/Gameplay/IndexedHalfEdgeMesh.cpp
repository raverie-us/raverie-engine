// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "IndexedHalfEdgeMesh.hpp"

namespace Raverie
{

#define DefineHalfEdgeArrayType(arrayType)                                                                             \
  RaverieDefineType(arrayType, builder, type)                                                                            \
  {                                                                                                                    \
    RaverieBindDocumented();                                                                                              \
                                                                                                                       \
    RaverieBindMethod(Get);                                                                                              \
    RaverieBindGetter(All);                                                                                              \
    RaverieBindGetterProperty(Count);                                                                                    \
  }

DefineHalfEdgeArrayType(IndexedHalfEdgeMeshVertexArray);
DefineHalfEdgeArrayType(IndexedHalfEdgeMeshEdgeArray);
DefineHalfEdgeArrayType(IndexedHalfEdgeFaceEdgeIndexArray);
DefineHalfEdgeArrayType(IndexedHalfEdgeMeshFaceArray);

RaverieDefineType(IndexedHalfEdge, builder, type)
{
  RaverieBindDefaultCopyDestructor();

  RaverieBindFieldGetter(mVertexIndex);
  RaverieBindFieldGetter(mTwinIndex);
  RaverieBindFieldGetter(mFaceIndex);
}

RaverieDefineType(IndexedHalfEdgeFace, builder, type)
{
  RaverieBindDefaultCopyDestructor();

  RaverieBindGetter(Edges);
}

IndexedHalfEdgeFace::IndexedHalfEdgeFace()
{
  mBoundEdges.mBoundArray = &mEdges;
}

IndexedHalfEdgeFace::BoundEdgeArray* IndexedHalfEdgeFace::GetEdges()
{
  return &mBoundEdges;
}

RaverieDefineType(IndexedHalfEdgeMesh, builder, type)
{
  RaverieBindDefaultCopyDestructor();

  RaverieBindGetter(Vertices);
  RaverieBindGetter(Edges);
  RaverieBindGetter(Faces);
}

IndexedHalfEdgeMesh::IndexedHalfEdgeMesh()
{
  mBoundVertices.mBoundArray = &mVertices;
  mBoundEdges.mBoundArray = &mEdges;
  mBoundFaces.mBoundArray = &mFaces;
}

void IndexedHalfEdgeMesh::Create(int vertexCount, int edgeCount, int faceCount)
{
  Clear();
  mVertices.Resize(vertexCount);
  mEdges.Resize(edgeCount);
  mFaces.Resize(faceCount);
  for (int i = 0; i < edgeCount; ++i)
    mEdges[i] = new IndexedHalfEdge();
  for (int i = 0; i < faceCount; ++i)
    mFaces[i] = new IndexedHalfEdgeFace();
}

void IndexedHalfEdgeMesh::Clear()
{
  mVertices.Clear();
  DeleteObjectsIn(mEdges);
  DeleteObjectsIn(mFaces);
  mEdges.Clear();
  mFaces.Clear();
}

IndexedHalfEdgeMesh::BoundVertexArray* IndexedHalfEdgeMesh::GetVertices()
{
  return &mBoundVertices;
}

IndexedHalfEdgeMesh::BoundEdgeArray* IndexedHalfEdgeMesh::GetEdges()
{
  return &mBoundEdges;
}

IndexedHalfEdgeMesh::BoundFaceArray* IndexedHalfEdgeMesh::GetFaces()
{
  return &mBoundFaces;
}

} // namespace Raverie
