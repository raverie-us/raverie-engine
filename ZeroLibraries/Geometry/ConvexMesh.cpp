///////////////////////////////////////////////////////////////////////////////
///
/// \file ConvexMesh.cpp
/// Implementation of the ConvexMesh class.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#define DrawGaussMap
#ifdef DrawGaussMap

#define DrawPoint(point, color)\
  Zero::gDebugDraw->Add(Zero::Debug::LineCross((point), real(0.025)).Color((color)))

#define DrawArc(pointA, pointB, color)\
  Zero::gDebugDraw->Add(Zero::Debug::Arc((pointA), (pointB)).Color((color)))

#endif

namespace Geometry
{

ConvexMesh::Vertex::Vertex(void)
{
  Id = uint(-1);
}

ConvexMesh::Vertex::~Vertex(void)
{
  //
}

ConvexMesh::Edge::Edge(void)
{
  Id = uint(-1);
  Points[0] = Id;
  Points[1] = Id;
  Faces[0] = Id;
  Faces[1] = Id;  
}

void ConvexMesh::Edge::SetFace(uint face)
{
  ErrorIf(Faces[1] != uint(-1), "Geometry::ConvexMesh - This edge is being "
                                "given a third face.");
  if(Faces[0] == uint(-1))
  {
    Faces[0] = face;
  }
  else
  {
    Faces[1] = face;
  }
}

Vec3 ConvexMesh::Edge::GetPlaneNormal(MeshParam convexMesh) const
{
  Vec3 normalA = convexMesh.GetFaceNormal(Faces[0]);
  Vec3 normalB = convexMesh.GetFaceNormal(Faces[1]);
  return Math::Normalized(Cross(normalA, normalB));
}

ConvexMesh::Face::Face(void)
{
  Id = uint(-1);
  Points[0] = Id;
  Points[1] = Id;
  Points[2] = Id;
  Edges[0] = Id;
  Edges[1] = Id;
  Edges[2] = Id;
}

ConvexMesh::ConvexMesh(void)
{
  //
}

ConvexMesh::~ConvexMesh(void)
{
  //
}

void ConvexMesh::CopyInfo(Hull3D& hull)
{
  //Go through all the hull vertices and copy their positions over to the new buffer
  //while giving all of the mesh's vertices an Id. Give the hull vertices the same
  //Id in order to save the position of each mesh vertex in the buffer
  mVertices.Resize(hull.GetHullVertexCount());
  uint vertexCount = uint(mVertices.Size());
  Hull3D::VertexList::range vertices = hull.GetHullVertices();
  for(uint i = 0; i < vertexCount; ++i)
  {
    Hull3D::Vertex* currentVertex = &vertices.Front();
    vertices.PopFront();

    ConvexMesh::Vertex& vertex = mVertices[i];
    vertex.Id = i;
    vertex.Position = currentVertex->Position;
    currentVertex->Id = i;
  }

  //Go through all the hull edges and copy their endpoints over to the new buffer 
  //while giving all of the mesh's edges an Id. Give the hull edges the same Id in
  //order to save the position of each mesh edge in the buffer
  mEdges.Resize(hull.GetEdgeCount());
  uint edgeCount = uint(mEdges.Size());
  Hull3D::EdgeList::range edges = hull.GetEdges();
  for(uint i = 0; i < edgeCount; ++i)
  {
    Hull3D::Edge* currentEdge = &edges.Front();
    edges.PopFront();

    Edge& edge = mEdges[i];
    edge.Id = i;

    //Link the edges to the points
    edge.Points[0] = currentEdge->EndPoints[0]->Id;
    edge.Points[1] = currentEdge->EndPoints[1]->Id;

    //Assign this Id to the hull edge for use in linking edges to faces
    currentEdge->Id = i;
  }

  UintArray vertexFaceCount(vertexCount, 0);

  mFaces.Resize(hull.GetFaceCount());
  uint faceCount = uint(mFaces.Size());
  Hull3D::FaceList::range faces = hull.GetFaces();
  for(uint i = 0; i < faceCount; ++i)
  {
    Hull3D::Face* currentFace = &faces.Front();
    faces.PopFront();

    Face& face = mFaces[i];

    face.Id = i;

    //Link the faces to the points and update how many faces the points are 
    //connected to (used later for array size)
    face.Points[0] = currentFace->Vertices[0]->Id;
    ++(vertexFaceCount[face.Points[0]]);

    face.Points[1] = currentFace->Vertices[1]->Id;
    ++(vertexFaceCount[face.Points[1]]);

    face.Points[2] = currentFace->Vertices[2]->Id;
    ++(vertexFaceCount[face.Points[2]]);

    //Link the faces to the edges and link the edges to the faces
    face.Edges[0] = currentFace->Edges[0]->Id;
    mEdges[face.Edges[0]].SetFace(i);

    face.Edges[1] = currentFace->Edges[1]->Id;
    mEdges[face.Edges[1]].SetFace(i);

    face.Edges[2] = currentFace->Edges[2]->Id;
    mEdges[face.Edges[2]].SetFace(i);
  }

  //Go through all of the mesh's vertices to allocate the arrays needed to store
  //the indices of the faces that they are linked to. Then go through all of the
  //faces looking to see which faces are linked to the mesh vertex, and link the
  //vertex to the face
  for(uint i = 0; i < vertexCount; ++i)
  {
    Vertex& vertex = mVertices[i];
    vertex.Faces.Resize(vertexFaceCount[i]);
    uint faceIndex = 0;

    uint id = vertex.Id;
    for(uint j = 0; j < faceCount; ++j)
    {
      Face& face = mFaces[j];
      if(face.Points[0] == id || face.Points[1] == id || face.Points[2] == id)
      {
        vertex.Faces[faceIndex] = face.Id;
        ++faceIndex;
      }
    }
  }
}

uint ConvexMesh::GetVertexCount(void) const
{
  return uint(mVertices.Size());
}

uint ConvexMesh::GetEdgeCount(void) const
{
  return uint(mEdges.Size());
}

uint ConvexMesh::GetFaceCount(void) const
{
  return uint(mFaces.Size());
}

const ConvexMesh::Vertex* ConvexMesh::GetVertices(void) const
{
  return &(mVertices[0]);
}

const ConvexMesh::Edge* ConvexMesh::GetEdges(void) const
{
  return &(mEdges[0]);
}

const ConvexMesh::Face* ConvexMesh::GetFaces(void) const
{
  return &(mFaces[0]);
}

const ConvexMesh::Vertex& ConvexMesh::GetVertex(uint index) const
{
  return mVertices[index];
}

Vec3 ConvexMesh::GetEdgePlaneNormal(uint index) const
{
  return mEdges[index].GetPlaneNormal(*this);
}

const ConvexMesh::Face& ConvexMesh::GetFace(uint index) const
{
  return mFaces[index];
}

uint ConvexMesh::GetFacesSharedByVertex(uint vertexIndex, 
                                        const uint* faces) const
{
  faces = &(GetVertex(vertexIndex).Faces[0]);
  return uint(mVertices[vertexIndex].Faces.Size());
}

void ConvexMesh::GetFacesSharedByEdge(uint edgeIndex, const uint*& faces) const
{
  faces = mEdges[edgeIndex].Faces;
}

Vec3 ConvexMesh::GetFaceNormal(uint faceIndex) const
{
  const Face& face = GetFace(faceIndex);
  const Vec3& a = mVertices[face.Points[0]].Position;
  const Vec3& b = mVertices[face.Points[1]].Position;
  const Vec3& c = mVertices[face.Points[2]].Position;
  return GenerateNormal(a, b, c);
}

///Generates vertex and index buffer, returns the mesh's triangle count
uint ConvexMesh::GenerateRenderData(Vec3** vertexBuffer, uint** indexBuffer)
{
  ErrorIf(vertexBuffer == nullptr, "Geometry - Null pointer passed, this "\
                                "function needs a valid pointer.");
  ErrorIf(indexBuffer == nullptr, "Geometry - Null pointer passed, this "\
                               "function needs a valid pointer.");

  //Copy all of the vertex positions
  uint vertexCount = uint(mVertices.Size());
  *vertexBuffer = new Vec3[vertexCount];
  for(uint i = 0; i < vertexCount; ++i)
  {
    (*vertexBuffer)[i] = mVertices[i].Position;
  }

  //Copy all of the triangle indices
  uint faceCount = uint(mFaces.Size());
  *indexBuffer = new uint[faceCount * 3];
  for(uint i = 0; i < faceCount; ++i)
  {
    uint index = i * 3;
    (*indexBuffer)[index] = mFaces[i].Points[0];
    index = (i * 3) + 1;
    (*indexBuffer)[index] = mFaces[i].Points[1];
    index = (i * 3) + 2;
    (*indexBuffer)[index] = mFaces[i].Points[2];
  }

  //Faces are triangles
  return faceCount;
}

void ConvexMesh::Draw(void) const
{
  //
}

void ConvexMesh::RenderGaussMap(void)
{
#ifdef DrawGaussMap
  //Draw all new edges as arcs between face normals
  uint arraySize = uint(mEdges.Size());
  for(uint i = 0; i < arraySize; ++i)
  {
    const Edge& edge = mEdges[i];
    const Face& faceA = mFaces[edge.Faces[0]];
    const Face& faceB = mFaces[edge.Faces[1]];
    Vec3 normalA = GetFaceNormal(faceA.Id);
    Vec3 normalB = GetFaceNormal(faceB.Id);
    //DrawArc(normalA, normalB, Color::Red);
  }

//   //Draw all face normals as points
//   arraySize = uint(mFaces.Size());
//   for(uint i = 0; i < arraySize; ++i)
//   {
//     const Face& face = mFaces[i];
//     Vec3 faceNormal = GetFaceNormal(face.Id);
//     DrawPoint(faceNormal, Color::Green);
//   }
#endif
}

}// namespace Geometry
