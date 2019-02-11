// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "IndexedHalfEdgeMesh.hpp"

namespace Zero
{

#define DefineHalfEdgeArrayType(arrayType)                                     \
  ZilchDefineType(arrayType, builder, type)                                    \
  {                                                                            \
    ZeroBindDocumented();                                                      \
                                                                               \
    ZilchBindMethod(Get);                                                      \
    ZilchBindGetter(All);                                                      \
    ZilchBindGetterProperty(Count);                                            \
  }

DefineHalfEdgeArrayType(IndexedHalfEdgeMeshVertexArray);
DefineHalfEdgeArrayType(IndexedHalfEdgeMeshEdgeArray);
DefineHalfEdgeArrayType(IndexedHalfEdgeFaceEdgeIndexArray);
DefineHalfEdgeArrayType(IndexedHalfEdgeMeshFaceArray);

ZilchDefineType(IndexedHalfEdge, builder, type)
{
  ZilchBindDefaultCopyDestructor();

  ZilchBindFieldGetter(mVertexIndex);
  ZilchBindFieldGetter(mTwinIndex);
  ZilchBindFieldGetter(mFaceIndex);
}

ZilchDefineType(IndexedHalfEdgeFace, builder, type)
{
  ZilchBindDefaultCopyDestructor();

  ZilchBindGetter(Edges);
}

IndexedHalfEdgeFace::IndexedHalfEdgeFace()
{
  mBoundEdges.mBoundArray = &mEdges;
}

IndexedHalfEdgeFace::BoundEdgeArray* IndexedHalfEdgeFace::GetEdges()
{
  return &mBoundEdges;
}

ZilchDefineType(IndexedHalfEdgeMesh, builder, type)
{
  ZilchBindDefaultCopyDestructor();

  ZilchBindGetter(Vertices);
  ZilchBindGetter(Edges);
  ZilchBindGetter(Faces);
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

} // namespace Zero
