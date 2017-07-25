///////////////////////////////////////////////////////////////////////////////
///
/// \file MeshFilterRange.cpp
/// Implementation of the MeshFilterRange and MeshPreFilteredRange class. These
/// ranges are used by any collider that wants to form a filtered range over
/// a mesh. The PreFilteredRange is for a mesh that already filters due to a
/// midphase.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Physics
{

MeshFilterRange::MeshFilterRange(const Vec3Array* vertices, const IndexArray* indices, const Aabb& aabb)
{
  mIndex = 0;
  mVertices = vertices;
  mIndices = indices;
  obj.Index = 0;
  mAabb = aabb;

  //skip over any items that we are filtering out
  SkipDead();
}

void MeshFilterRange::SkipDead()
{
  while(!Empty())
  {
    //do a quick check over the aabb we are filtering against
    Aabb aabb;
    aabb.Compute((*mVertices)[(*mIndices)[mIndex]]);
    aabb.Expand((*mVertices)[(*mIndices)[mIndex + 1]]);
    aabb.Expand((*mVertices)[(*mIndices)[mIndex + 2]]);

    if(aabb.Overlap(mAabb))
      break;

    mIndex += 3;
    ++obj.Index;
  }
}

void MeshFilterRange::PopFront()
{
  ErrorIf(Empty(), "Popping an invalid range.");

  mIndex += 3;
  ++obj.Index;

  //skip over any items that we are filtering out
  SkipDead();
}

MeshFilterRange::TriangleObject& MeshFilterRange::Front()
{
  //fill out the triangle and index
  obj.Shape.p0 = (*mVertices)[(*mIndices)[mIndex]];
  obj.Shape.p1 = (*mVertices)[(*mIndices)[mIndex + 1]];
  obj.Shape.p2 = (*mVertices)[(*mIndices)[mIndex + 2]];
  obj.Index = mIndex;
  return obj;
}

bool MeshFilterRange::Empty()
{
  return mIndex >= mIndices->Size();
}

MeshPreFilteredRange::MeshPreFilteredRange()
{
  mIndex = 0;
  obj.Index = 0;
}

void MeshPreFilteredRange::Initialize()
{
  mIndex = 0;
  //deal with getting no results
  if(!mTriangles.Empty())
  {
    obj.Shape = mTriangles[0];
    obj.Index = mIndices[0];
  }
}

void MeshPreFilteredRange::PopFront()
{
  ErrorIf(Empty(), "Popping an invalid range.");
  ++mIndex;
}

MeshPreFilteredRange::TriangleObject& MeshPreFilteredRange::Front()
{
  //fill out the triangle and index
  obj.Shape = mTriangles[mIndex];
  obj.Index = mIndices[mIndex];
  return obj;
}

bool MeshPreFilteredRange::Empty()
{
  return mIndex >= mIndices.Size();
}

}//namespace Physics

}//namespace Zero
