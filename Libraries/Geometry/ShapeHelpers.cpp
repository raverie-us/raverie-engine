///////////////////////////////////////////////////////////////////////////////
///
/// \file ShapesHelpers.hpp
/// Contains conversion functions to and from shapes.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//--------------------------------------------------------- Conversion Functions
Ray ToRay(const Segment& segment)
{
  return Ray(segment.Start, segment.End - segment.Start);
}

Segment ToSegment(const Ray& ray, real t)
{
  return Segment(ray.Start, ray.Start + ray.Direction * t);
}

Aabb ToAabb(Vec3Param point)
{
  Aabb aabb;
  aabb.SetMinAndMax(point, point);
  return aabb;
}

Aabb ToAabb(const Aabb& aabb)
{
  return aabb;
}

Aabb ToAabb(const Ray& ray, real t)
{
  Aabb aabb;
  aabb.Compute(ray.Start);
  aabb.Expand(ray.Start + ray.Direction * t);
  return aabb;
}

Aabb ToAabb(const Segment& segment)
{
  Aabb aabb;
  aabb.Compute(segment.Start);
  aabb.Expand(segment.End);
  return aabb;
}

Aabb ToAabb(const Triangle& tri)
{
  Aabb aabb;
  aabb.Compute(&tri.p0, 3);
  return aabb;
}

Aabb ToAabb(const SweptTriangle& sweptTri)
{
  Aabb aabb;
  aabb.Compute(&sweptTri.BaseTri.p0, 3);
  for(uint i = 0; i < 3; ++i)
    aabb.Expand(sweptTri.BaseTri[i] + sweptTri.ScaledDir);
  return aabb;
}

Aabb ToAabb(const Obb& obb)
{
  Vec3 e = obb.HalfExtents;
  Vec3 halfExtents;

  //some black magic from real time collision detection
  for(uint i = 0; i < 3; ++i)
  {
    halfExtents[i] = Math::Abs(obb.Basis.GetBasis(0)[i]) * e[0];
    halfExtents[i] += Math::Abs(obb.Basis.GetBasis(1)[i]) * e[1];
    halfExtents[i] += Math::Abs(obb.Basis.GetBasis(2)[i]) * e[2];
  }

  Aabb aabb;
  aabb.SetCenterAndHalfExtents(obb.Center, halfExtents);

  return aabb;
}

Aabb ToAabb(const Sphere& sphere)
{
  Aabb aabb;
  real r = sphere.mRadius;
  aabb.SetCenterAndHalfExtents(sphere.mCenter, Vec3(r, r, r));
  return aabb;
}

Aabb ToAabb(const Ellipsoid& ellipsoid)
{
  Vec3 e = ellipsoid.Radii;
  Vec3 halfExtents;

  //some black magic from real time collision detection
  for(uint i = 0; i < 3; ++i)
  {
    halfExtents[i] = Math::Abs(ellipsoid.Basis(i, 0)) * e[0];
    halfExtents[i] += Math::Abs(ellipsoid.Basis(i, 1)) * e[1];
    halfExtents[i] += Math::Abs(ellipsoid.Basis(i, 2)) * e[2];
  }

  Aabb aabb;
  aabb.SetCenterAndHalfExtents(ellipsoid.Center, halfExtents);

  return aabb;
}

Aabb ToAabb(const Capsule& capsule)
{
  Aabb aabb;
  for(uint axis = 0; axis < 3; ++axis)
  {
    real min = capsule.PointA[axis];
    real max = capsule.PointB[axis];
    if(min > max)
      Math::Swap(min, max);

    aabb.mMin[axis] = min - capsule.Radius;
    aabb.mMax[axis] = max + capsule.Radius;
  }
  return aabb;
}

Aabb ToAabb(const Cylinder& cylinder)
{
  //not quite right, this is of a capsule, but whatever...
  Aabb aabb;
  for(uint axis = 0; axis < 3; ++axis)
  {
    real min = cylinder.PointA[axis];
    real max = cylinder.PointB[axis];
    if(min > max)
      Math::Swap(min, max);

    aabb.mMin[axis] = min - cylinder.Radius;
    aabb.mMax[axis] = max + cylinder.Radius;
  }
  return aabb;
}

Aabb ToAabb(const Frustum& frustum)
{
  Vec3 points[8];
  frustum.GetPoints(points);
  Aabb aabb;
  aabb.Compute(points, 8);
  return aabb;
}

Aabb ToAabb(const ConvexMeshShape& convexMesh)
{
  return convexMesh.mWorldAabb;
}

Obb ToObb(Ellipsoid& ellipsoid)
{
  return Obb(ellipsoid.Center, ellipsoid.Radii, ellipsoid.Basis);
}

Obb ToObb(const Cylinder& cylinder)
{
  Vec3 u = (cylinder.PointA - cylinder.PointB);
  real halfHeight = u.Normalize() * real(.5);
  Obb obb;
  obb.Center = (cylinder.PointA + cylinder.PointB) * real(.5);
  obb.HalfExtents = Vec3(cylinder.Radius, halfHeight, cylinder.Radius);

  Vec3 v, w;
  Math::GenerateOrthonormalBasis(u, &v, &w);
  obb.Basis.SetBasis(0, w);
  obb.Basis.SetBasis(1, u);
  obb.Basis.SetBasis(2, v);
  return obb;
}

Obb ToObb(const Capsule& capsule)
{
  Vec3 u = (capsule.PointA - capsule.PointB);
  real halfHeight = u.Normalize() * real(.5);
  Obb obb;
  obb.Center = (capsule.PointA + capsule.PointB) * real(.5);
  obb.HalfExtents = Vec3(capsule.Radius, halfHeight, capsule.Radius);

  Vec3 v, w;
  Math::GenerateOrthonormalBasis(u, &v, &w);
  obb.Basis.SetBasis(0, w);
  obb.Basis.SetBasis(1, u);
  obb.Basis.SetBasis(2, v);
  return obb;
}

Cylinder ToCylinder(const Capsule& capsule)
{
  return Cylinder(capsule.PointA, capsule.PointB, capsule.Radius);
}

Capsule ToCapsule(const Cylinder& cylinder)
{
  return Capsule(cylinder.PointA, cylinder.PointB, cylinder.Radius);
}

}//namespace Zero
