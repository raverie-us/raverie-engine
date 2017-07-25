///////////////////////////////////////////////////////////////////////////////
///
/// \file Epa.cpp
/// Expanding polytope algorithm for finding contact data on a CSO.
///
/// Authors: Nathan Carlson
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Intersection
{

void Epa::Init(const Simplex &simplex)
{
  mVertices.Clear();
  mEdges.Clear();
  mFaces.Clear();

  // Cannot init Epa without a valid tetrahedron
  // Order is dependent on how the simplex is built
  mVertices.PushBack(simplex.mPoints[0]);
  mVertices.PushBack(simplex.mPoints[2]);
  mVertices.PushBack(simplex.mPoints[1]);
  mVertices.PushBack(simplex.mPoints[3]);

  mFaces.PushBack(Face(0, 1, 2));
  mFaces.PushBack(Face(0, 3, 1));
  mFaces.PushBack(Face(1, 3, 2));
  mFaces.PushBack(Face(2, 3, 0));

  // Compute normals of initial faces
  for (unsigned i = 0; i < mFaces.Size(); ++i)
  {
    Face &face = mFaces[i];
    face.normal = (mVertices[face.p1].cso - mVertices[face.p0].cso).Cross(mVertices[face.p2].cso - mVertices[face.p0].cso);
    face.normal.Normalize();
  }

  mDistClosest = FLT_MAX;
}

Vec3 Epa::GetClosestFaceNormal(void)
{
  mIndexClosest = (uint)-1;
  mDistClosest = -FLT_MAX;

  for (unsigned i = 0; i < mFaces.Size(); ++i)
  {
    Face &face = mFaces[i];

    Vec3 closest = Vec3::cZero;
    ClosestPointOnTriangleToPoint(mVertices[face.p0].cso, mVertices[face.p1].cso, mVertices[face.p2].cso, &closest);

    float dist = -closest.Length();
    if (dist > mDistClosest)
    {
      mDistClosest = dist;
      mIndexClosest = i;
    }
  }

  // Not having any faces is not considered a valid use case
  return mFaces[mIndexClosest].normal;
}

float Epa::GetClosestDistance(void)
{
  return mDistClosest;
}

void Epa::GetClosestFace(CSOVertex *retFace)
{
  retFace[0] = mVertices[mFaces[mIndexClosest].p0];
  retFace[1] = mVertices[mFaces[mIndexClosest].p1];
  retFace[2] = mVertices[mFaces[mIndexClosest].p2];
}

bool Epa::Expand(CSOVertex newPoint)
{
  // Primary terminating condition
  // When new support point is not far enough from the closest face
  if (mFaces[mIndexClosest].normal.Dot(newPoint.cso - mVertices[mFaces[mIndexClosest].p0].cso) < 0.001f)
    return false;

  // If point is already in the CSO, fail to expand
  for (unsigned i = 0; i < mVertices.Size(); ++i)
  {
    if (Math::Equal(newPoint.cso, mVertices[i].cso, 0.001f))
      return false;
  }

  // Find every face that the new point is in front of
  Zero::Array<unsigned> visibleFaces;
  // float maxDot = 0.0f;
  for (unsigned i = 0; i < mFaces.Size(); ++i)
  {
    Face &face = mFaces[i];
    float dot = face.normal.Dot(newPoint.cso - mVertices[face.p0].cso);
    if (dot > 0.00001f)
    // {
      visibleFaces.PushBack(i);
      // maxDot = dot > maxDot ? dot : maxDot;
    // }
  }

  // The edge case of not having any visible faces that are a reasonable distance from the support point
  // should no longer happen due to the primary terminating condition above
  // bool clearFaces = maxDot < 0.0001f;
  // if (clearFaces)
  //   visibleFaces.Clear();

  // Degenerate point, can't expand
  if (!visibleFaces.Size())
    return false;

  // DebugPrint("%f, %d\n", maxDot, clearFaces);

  // Evaluate faces back to front for easy array clean up
  for (unsigned i = visibleFaces.Size() - 1; i < visibleFaces.Size(); --i)
  {
    Face &face = mFaces[visibleFaces[i]];

    // Add every edge of the triangular face
    // If there is a duplicate edge, then two adjacent faces are being removed
    // and we do not want to create an interior face from that edge,
    // so we remove the original from the list instead of adding a duplicate
    AddEdge(face.p0, face.p1);
    AddEdge(face.p1, face.p2);
    AddEdge(face.p2, face.p0);

    // We have the edges, now get rid of the face
    mFaces[visibleFaces[i]] = mFaces.Back();
    mFaces.PopBack();
  }

  // Create new faces to the support point
  mVertices.PushBack(newPoint);
  unsigned index = mVertices.Size() - 1;
  for (unsigned i = 0; i < mEdges.Size(); ++i)
  {
    Edge &edge = mEdges[i];
    Face newFace(edge.p0, edge.p1, index);
    newFace.normal = (mVertices[newFace.p1].cso - mVertices[newFace.p0].cso).Cross(mVertices[newFace.p2].cso - mVertices[newFace.p0].cso);
    newFace.normal.Normalize();
    mFaces.PushBack(newFace);
  }
  mEdges.Clear();

  return true;
}

void Epa::DebugPoint(CSOVertex debugPoint)
{
  mDebugPoint = debugPoint;
  mStep = 0;
}

bool Epa::DebugStep(void)
{
  // Find every face that the new point is in front of
  mVisibleFaces.Clear();
  for (unsigned i = 0; i < mFaces.Size(); ++i)
  {
    Face &face = mFaces[i];
    float dot = face.normal.Dot(mDebugPoint.cso - mVertices[face.p0].cso);
    if (dot > 0.0000001f)
      mVisibleFaces.PushBack(i);
  }

  // Degenerate point, can't expand
  if (!mVisibleFaces.Size() && !mEdges.Size())
    return false;

  if (mStep < 1)
  {
    ++mStep;
    return true;
  }

  // Evaluate faces back to front for easy array clean up
  for (unsigned i = mVisibleFaces.Size() - 1; i < mVisibleFaces.Size(); --i)
  {
    Face &face = mFaces[mVisibleFaces[i]];

    // Add every edge of the triangular face
    // If there is a duplicate edge, then two adjacent faces are being removed
    // and we do not want to create an interior face from that edge,
    // so we remove the original from the list instead of adding a duplicate
    AddEdge(face.p0, face.p1);
    AddEdge(face.p1, face.p2);
    AddEdge(face.p2, face.p0);

    // We have the edges, now get rid of the face
    mFaces[mVisibleFaces[i]] = mFaces.Back();
    mFaces.PopBack();
  }
  mVisibleFaces.Clear();

  if (mStep < 2)
  {
    ++mStep;
    return true;
  }

  // Create new faces to the support point
  mVertices.PushBack(mDebugPoint);
  unsigned index = mVertices.Size() - 1;
  for (unsigned i = 0; i < mEdges.Size(); ++i)
  {
    Edge &edge = mEdges[i];
    Face newFace(edge.p0, edge.p1, index);
    newFace.normal = (mVertices[newFace.p1].cso - mVertices[newFace.p0].cso).Cross(mVertices[newFace.p2].cso - mVertices[newFace.p0].cso);
    newFace.normal.Normalize();
    mFaces.PushBack(newFace);
  }
  mEdges.Clear();

  mStep = 0;

  return true;
}

void Epa::DrawDebug(void)
{
  Zero::gDebugDraw->Add(Zero::Debug::Sphere(mDebugPoint.cso, 0.01f).Color(Color::Red));

  for (unsigned i = 0; i < mEdges.Size(); ++i)
  {
    Edge &edge = mEdges[i];
    Zero::gDebugDraw->Add(Zero::Debug::Line(mVertices[edge.p0].cso, mVertices[edge.p1].cso).Color(Color::Orange));
  }

  for (unsigned i = 0; i < mFaces.Size(); ++i)
  {
    Face &face = mFaces[i];
    ByteColor color;
    if (mVisibleFaces.FindIndex(i) != Zero::Array<unsigned>::InvalidIndex)
      color = Color::Orange;
    else if (i == mIndexClosest)
      color = Color::Red;
    else
      color = Color::Blue;
      Zero::gDebugDraw->Add(Zero::Debug::Triangle(mVertices[face.p0].cso, mVertices[face.p1].cso, mVertices[face.p2].cso).Color(color).Border(true).Alpha(50));
  }
}

void Epa::AddEdge(unsigned p0, unsigned p1)
{
  Edge newEdge = Edge(p0, p1);
  unsigned index = mEdges.FindIndex(newEdge);
  if (index != InvalidIndex)
  {
    mEdges[index] = mEdges.Back();
    mEdges.PopBack();
  }
  else
  {
    mEdges.PushBack(newEdge);
  }
}

} // namespace Intersection
