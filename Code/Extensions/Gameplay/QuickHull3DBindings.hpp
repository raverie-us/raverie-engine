// MIT Licensed (see LICENSE.md).
#pragma once

#include "Foundation/Geometry/QuickHull3D.hpp"
#include "IndexedHalfEdgeMesh.hpp"

namespace Raverie
{

/// Bound interface wrapper around quickhull3D. Allows adding points to build a
/// mesh and then iterating over the resultant index-based half-edge-mesh.
class QuickHull3DInterface
{
  RaverieDeclareType(QuickHull3DInterface, TypeCopyMode::ReferenceType);

  QuickHull3DInterface();
  QuickHull3DInterface(const QuickHull3DInterface& rhs);
  ~QuickHull3DInterface();

  /// Add a point to build the hull from.
  void Add(Vec3Param point);
  /// Build the a convex hull from all of the input points. Returns false if a
  // hull couldn't be built. This typically means there weren't enough points
  // (4 required) or the points don't form a volume (e.g. all lie on a plane).
  bool Build();
  /// Clear all input points and the cached mesh.
  void Clear();
  /// Debug draw the mesh at the given index. Primarily for development
  /// purposes.
  void Draw();

  // Build the half-edge mesh from the quick-hull results.
  void BuildHalfEdgeMesh();

  /// What sub-step of quick-hull should be drawn?
  int mIndex;
  /// Should debug steps be drawn? If false less work is done during
  /// the quick-hull algorithm so this is false by default.
  bool mShowDebugDraw;

  /// The input points to build quick-hull from.
  Array<Vec3> mPoints;
  QuickHull3D mQuickHull3D;
  DebugDrawStack mDebugDrawStack;

  /// The resultant half-edge-mesh from quick-hull's run.
  HandleOf<IndexedHalfEdgeMesh> mMesh;
};

} // namespace Raverie
