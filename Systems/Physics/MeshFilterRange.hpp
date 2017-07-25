///////////////////////////////////////////////////////////////////////////////
///
/// \file MeshFilterRange.hpp
/// Declaration of the MeshFilterRange and MeshPreFilteredRange class. These
/// ranges are used by any collider that wants to form a filtered range over
/// a mesh. The PreFilteredRange is for a mesh that already filters due to a
/// midphase.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{

/// Range used when filtering what triangles of a mesh
/// should be checked for either collision or for a cast.
struct MeshFilterRange
{
  //When performing collision against a mesh, we may need to have multiple
  //manifolds (one for each triangle). So we also need to differentiate
  //between the triangles with an id. Therefore, narrow phase needs the
  //object to test collision against as well as it's id.
  struct TriangleObject
  {
    uint Index;
    Triangle Shape;
  };

  typedef Array<uint> IndexArray;

  MeshFilterRange(const Vec3Array* vertices, const IndexArray* indices, const Aabb& aabb);

  void SkipDead();

  // Range Interface
  void PopFront();
  TriangleObject& Front();
  bool Empty();

  //since we have to return a reference in Front(), we need to store
  //the TriangleObject internally and modify that.
  TriangleObject obj;

  const Vec3Array* mVertices;
  const IndexArray* mIndices;

  ///What triangle is currently at the front of the range.
  uint mIndex;

  Aabb mAabb;
};

/// Range used when filtering against a pre-filtered range. This happens when
/// getting a set of results back from a mid-phase (aabb tree). This should
/// eventually be reconstructed to store the range from the mid-phase instead
/// the results since this range will most likely be copied.
struct MeshPreFilteredRange
{
  //When performing collision against a mesh, we may need to have multiple
  //manifolds (one for each triangle). So we also need to differentiate
  //between the triangles with an id. Therefore, narrow phase needs the
  //object to test collision against as well as it's id.
  struct TriangleObject
  {
    uint Index;
    Triangle Shape;
  };

  typedef Array<uint> IndexArray;
  typedef Array<Triangle> TriangleArray;

  MeshPreFilteredRange();

  // Used to tell the range that we have filled out it's triangles and indices.
  void Initialize();
  // Range Interface
  void PopFront();
  TriangleObject& Front();
  bool Empty();

  //since we have to return a reference in Front(), we need to store
  //the TriangleObject internally and modify that.
  TriangleObject obj;

  TriangleArray mTriangles;
  IndexArray mIndices;

  ///What triangle is currently at the front of the range.
  uint mIndex;
};

}//namespace Physics

}//namespace Zero
