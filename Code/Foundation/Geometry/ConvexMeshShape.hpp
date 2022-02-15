// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

struct ConvexMeshShape
{
  ConvexMeshShape();
  ConvexMeshShape(const Intersection::SupportShape& support);

  void Support(Vec3Param direction, Vec3Ptr support) const;
  void GetCenter(Vec3Ref center) const;

  Obb Transform(Mat4Param transformation) const;
  /// Typedef for templated code to know what the transformed type is.
  typedef Obb TransformedShapeType;

  Intersection::SupportShape mSupport;
  Aabb mWorldAabb;
};

} // namespace Zero
