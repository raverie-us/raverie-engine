///////////////////////////////////////////////////////////////////////////////
///
/// \file ExtendedCollision.cpp
/// NSquared intersection functions for the shape primitives. Wraps the
/// internal intersection functions for ease of use.
///
/// Authors: Joshua Davis, Auto-Generated
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"


namespace Zero
{

void FlipManifoldInfo(Intersection::Manifold* manifold)
{
  manifold->Normal.Negate();
  for(uint i = 0; i < manifold->PointCount; ++i)
    Math::Swap(manifold->Points[i].Points[0],manifold->Points[i].Points[1]);
}

void FlipSupportShapeManifoldInfo(const Intersection::SupportShape& a,
                                  const Intersection::SupportShape& b,
                                  Intersection::Manifold* manifold)
{
  Vec3 aCenter,bCenter;
  a.GetCenter(&aCenter);
  b.GetCenter(&bCenter);
  Vec3 aToB = bCenter - aCenter;
  //flip the normal to alway point from a to b
  if(Math::Dot(aToB, manifold->Normal) < Math::real(0.0))
    manifold->Normal *= real(-1.0);
}

bool SupportShapeCollide(const Intersection::SupportShape& a,
                         const Intersection::SupportShape& b,
                         Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::Gjk gjk;
  Intersection::Type ret = gjk.Test(&a, &b, manifold);
  if(ret < (Intersection::Type)0)
    return false;

  if(manifold != NULL)
    FlipSupportShapeManifoldInfo(a,b,manifold);
  return true;
}

bool Collide(const Ray& ray, const Aabb& aabb, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RayAabb(ray.Start,ray.Direction,aabb.mMin,aabb.mMax, &point);
  if(ret < (Intersection::Type)0)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Ray& ray, const Capsule& capsule, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RayCapsule(ray.Start,ray.Direction,capsule.PointA,capsule.PointB,capsule.Radius, &point);
  if(ret < (Intersection::Type)0)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Ray& ray, const Cylinder& cylinder, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RayCylinder(ray.Start,ray.Direction,cylinder.PointA,cylinder.PointB,cylinder.Radius, &point);
  if(ret < (Intersection::Type)0)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Ray& ray, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RayEllipsoid(ray.Start,ray.Direction,ellipsoid.Center,ellipsoid.Radii,ellipsoid.Basis, &point);
  if(ret < (Intersection::Type)0)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Ray& ray, const Frustum& frustum, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Ray& ray, const Obb& obb, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RayObb(ray.Start,ray.Direction,obb.Center,obb.HalfExtents,obb.Basis, &point);
  if(ret < (Intersection::Type)0)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Ray& ray, const Plane& plane, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RayPlane(ray.Start,ray.Direction,plane.GetNormal(),plane.GetDistance(), &point);
  if(ret < (Intersection::Type)0)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Ray& ray, const Sphere& sphere, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RaySphere(ray.Start,ray.Direction,sphere.mCenter,sphere.mRadius, &point);
  if(ret < (Intersection::Type)0)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Ray& ray, const Triangle& triangle, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RayTriangle(ray.Start,ray.Direction,triangle.p0,triangle.p1,triangle.p2, &point);
  if(ret < (Intersection::Type)0)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Ray& ray, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RayTetrahedron(ray.Start,ray.Direction,tetrahedron.p0,tetrahedron.p1,tetrahedron.p2,tetrahedron.p3, &point);
  if(ret < (Intersection::Type)0)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Ray& ray, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Ray& ray, const SweptTriangle& sweptTri, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Segment& segment, const Aabb& aabb, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::SegmentAabb(segment.Start,segment.End,aabb.mMin,aabb.mMax, &point);
  if(ret < (Intersection::Type)0)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Segment& segment, const Capsule& capsule, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::SegmentCapsule(segment.Start,segment.End,capsule.PointA,capsule.PointB,capsule.Radius, &point);
  if(ret < (Intersection::Type)0)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Segment& segment, const Cylinder& cylinder, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::SegmentCylinder(segment.Start,segment.End,cylinder.PointA,cylinder.PointB,cylinder.Radius, &point);
  if(ret < (Intersection::Type)0)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Segment& segment, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Segment& segment, const Frustum& frustum, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Segment& segment, const Obb& obb, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::SegmentObb(segment.Start,segment.End,obb.Center,obb.HalfExtents,obb.Basis, &point);
  if(ret < (Intersection::Type)0)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Segment& segment, const Plane& plane, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::SegmentPlane(segment.Start,segment.End,plane.GetNormal(),plane.GetDistance(), &point);
  if(ret < (Intersection::Type)0)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Segment& segment, const Sphere& sphere, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::SegmentSphere(segment.Start,segment.End,sphere.mCenter,sphere.mRadius, &point);
  if(ret < (Intersection::Type)0)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Segment& segment, const Triangle& triangle, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::SegmentTriangle(segment.Start,segment.End,triangle.p0,triangle.p1,triangle.p2, &point);
  if(ret < (Intersection::Type)0)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Segment& segment, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Segment& segment, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Segment& segment, const SweptTriangle& sweptTri, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Aabb& aabb, const Ray& ray, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RayAabb(ray.Start,ray.Direction,aabb.mMin,aabb.mMax, &point);
  if(ret < Intersection::None)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Aabb& aabb, const Segment& segment, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::SegmentAabb(segment.Start,segment.End,aabb.mMin,aabb.mMax, &point);
  if(ret < Intersection::None)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Aabb& aabb1, const Aabb& aabb2, Intersection::Manifold* manifold)
{
  Intersection::Type ret = Intersection::AabbAabb(aabb1.mMin,aabb1.mMax,aabb2.mMin,aabb2.mMax, manifold);
  if(ret < (Intersection::Type)0)
    return false;
  return true;
}

bool Collide(const Aabb& aabb, const Capsule& capsule, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&aabb);
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Aabb& aabb, const Cylinder& cylinder, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&aabb);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Aabb& aabb, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&aabb);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Aabb& aabb, const Frustum& frustum, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&aabb);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Aabb& aabb, const Obb& obb, Intersection::Manifold* manifold)
{
  Intersection::Type ret = Intersection::AabbObb(aabb.mMin,aabb.mMax,obb.Center,obb.HalfExtents,obb.Basis, manifold);
  if(ret < (Intersection::Type)0)
    return false;
  return true;
}

bool Collide(const Aabb& aabb, const Plane& plane, Intersection::Manifold* manifold)
{
  Intersection::Type ret = Intersection::AabbPlane(aabb.mMin,aabb.mMax,plane.GetNormal(),plane.GetDistance(), manifold);
  if(ret < (Intersection::Type)0)
    return false;
  return true;
}

bool Collide(const Aabb& aabb, const Sphere& sphere, Intersection::Manifold* manifold)
{
  Intersection::Type ret = Intersection::AabbSphere(aabb.mMin,aabb.mMax,sphere.mCenter,sphere.mRadius, manifold);
  if(ret < (Intersection::Type)0)
    return false;
  return true;
}

bool Collide(const Aabb& aabb, const Triangle& triangle, Intersection::Manifold* manifold)
{
  Intersection::Type ret = Intersection::AabbTriangle(aabb.mMin,aabb.mMax,triangle.p0,triangle.p1,triangle.p2, manifold);
  if(ret < (Intersection::Type)0)
    return false;
  return true;
}

bool Collide(const Aabb& aabb, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&aabb);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Aabb& aabb, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&aabb);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Aabb& aabb, const SweptTriangle& sweptTri, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&aabb);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Capsule& capsule, const Ray& ray, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RayCapsule(ray.Start,ray.Direction,capsule.PointA,capsule.PointB,capsule.Radius, &point);
  if(ret < Intersection::None)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Capsule& capsule, const Segment& segment, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::SegmentCapsule(segment.Start,segment.End,capsule.PointA,capsule.PointB,capsule.Radius, &point);
  if(ret < Intersection::None)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Capsule& capsule, const Aabb& aabb, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  Intersection::SupportShape b = Intersection::MakeSupport(&aabb);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Capsule& capsule1, const Capsule& capsule2, Intersection::Manifold* manifold)
{
  Intersection::Type ret = Intersection::CapsuleCapsule(capsule1.PointA,capsule1.PointB,capsule1.Radius,capsule2.PointA,capsule2.PointB,capsule2.Radius, manifold);
  if(ret < (Intersection::Type)0)
    return false;
  return true;
}

bool Collide(const Capsule& capsule, const Cylinder& cylinder, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Capsule& capsule, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Capsule& capsule, const Frustum& frustum, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Capsule& capsule, const Obb& obb, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  Intersection::SupportShape b = Intersection::MakeSupport(&obb);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Capsule& capsule, const Plane& plane, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Capsule& capsule, const Sphere& sphere, Intersection::Manifold* manifold)
{
  Intersection::Type ret = Intersection::CapsuleSphere(capsule.PointA,capsule.PointB,capsule.Radius,sphere.mCenter,sphere.mRadius, manifold);
  if(ret < (Intersection::Type)0)
    return false;
  return true;
}

bool Collide(const Capsule& capsule, const Triangle& triangle, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  Intersection::SupportShape b = Intersection::MakeSupport(&triangle);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Capsule& capsule, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Capsule& capsule, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Capsule& capsule, const SweptTriangle& sweptTri, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Cylinder& cylinder, const Ray& ray, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RayCylinder(ray.Start,ray.Direction,cylinder.PointA,cylinder.PointB,cylinder.Radius, &point);
  if(ret < Intersection::None)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Cylinder& cylinder, const Segment& segment, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::SegmentCylinder(segment.Start,segment.End,cylinder.PointA,cylinder.PointB,cylinder.Radius, &point);
  if(ret < Intersection::None)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Cylinder& cylinder, const Aabb& aabb, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&aabb);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Cylinder& cylinder, const Capsule& capsule, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Cylinder& cylinder1, const Cylinder& cylinder2, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder1);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder2);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Cylinder& cylinder, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Cylinder& cylinder, const Frustum& frustum, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Cylinder& cylinder, const Obb& obb, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&obb);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Cylinder& cylinder, const Plane& plane, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Cylinder& cylinder, const Sphere& sphere, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&sphere);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Cylinder& cylinder, const Triangle& triangle, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&triangle);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Cylinder& cylinder, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Cylinder& cylinder, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Cylinder& cylinder, const SweptTriangle& sweptTri, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Ellipsoid& ellipsoid, const Ray& ray, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RayEllipsoid(ray.Start,ray.Direction,ellipsoid.Center,ellipsoid.Radii,ellipsoid.Basis, &point);
  if(ret < Intersection::None)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Ellipsoid& ellipsoid, const Segment& segment, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Ellipsoid& ellipsoid, const Aabb& aabb, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&aabb);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Ellipsoid& ellipsoid, const Capsule& capsule, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Ellipsoid& ellipsoid, const Cylinder& cylinder, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Ellipsoid& ellipsoid1, const Ellipsoid& ellipsoid2, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid1);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid2);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Ellipsoid& ellipsoid, const Frustum& frustum, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Ellipsoid& ellipsoid, const Obb& obb, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&obb);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Ellipsoid& ellipsoid, const Plane& plane, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Ellipsoid& ellipsoid, const Sphere& sphere, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&sphere);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Ellipsoid& ellipsoid, const Triangle& triangle, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&triangle);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Ellipsoid& ellipsoid, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Ellipsoid& ellipsoid, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Ellipsoid& ellipsoid, const SweptTriangle& sweptTri, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Frustum& frustum, const Ray& ray, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Frustum& frustum, const Segment& segment, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Frustum& frustum, const Aabb& aabb, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&aabb);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Frustum& frustum, const Capsule& capsule, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Frustum& frustum, const Cylinder& cylinder, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Frustum& frustum, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Frustum& frustum1, const Frustum& frustum2, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum1);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum2);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Frustum& frustum, const Obb& obb, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&obb);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Frustum& frustum, const Plane& plane, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Frustum& frustum, const Sphere& sphere, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&sphere);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Frustum& frustum, const Triangle& triangle, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&triangle);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Frustum& frustum, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Frustum& frustum, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Frustum& frustum, const SweptTriangle& sweptTri, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Obb& obb, const Ray& ray, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RayObb(ray.Start,ray.Direction,obb.Center,obb.HalfExtents,obb.Basis, &point);
  if(ret < Intersection::None)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Obb& obb, const Segment& segment, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::SegmentObb(segment.Start,segment.End,obb.Center,obb.HalfExtents,obb.Basis, &point);
  if(ret < Intersection::None)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Obb& obb, const Aabb& aabb, Intersection::Manifold* manifold)
{
  bool ret = Collide(aabb,obb, manifold);
  if(ret == false)
    return false;
  if(manifold != NULL)
    FlipManifoldInfo(manifold);
  return true;
}

bool Collide(const Obb& obb, const Capsule& capsule, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&obb);
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Obb& obb, const Cylinder& cylinder, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&obb);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Obb& obb, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&obb);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Obb& obb, const Frustum& frustum, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&obb);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Obb& obb1, const Obb& obb2, Intersection::Manifold* manifold)
{
  Intersection::Type ret = Intersection::ObbObb(obb1.Center,obb1.HalfExtents,obb1.Basis,obb2.Center,obb2.HalfExtents,obb2.Basis, manifold);
  if(ret < (Intersection::Type)0)
    return false;
  return true;
}

bool Collide(const Obb& obb, const Plane& plane, Intersection::Manifold* manifold)
{
  Intersection::Type ret = Intersection::ObbPlane(obb.Center,obb.HalfExtents,obb.Basis,plane.GetNormal(),plane.GetDistance(), manifold);
  if(ret < (Intersection::Type)0)
    return false;
  return true;
}

bool Collide(const Obb& obb, const Sphere& sphere, Intersection::Manifold* manifold)
{
  Intersection::Type ret = Intersection::ObbSphere(obb.Center,obb.HalfExtents,obb.Basis,sphere.mCenter,sphere.mRadius, manifold);
  if(ret < (Intersection::Type)0)
    return false;
  return true;
}

bool Collide(const Obb& obb, const Triangle& triangle, Intersection::Manifold* manifold)
{
  Intersection::Type ret = Intersection::ObbTriangle(obb.Center,obb.HalfExtents,obb.Basis,triangle.p0,triangle.p1,triangle.p2, manifold);
  if(ret < (Intersection::Type)0)
    return false;
  return true;
}

bool Collide(const Obb& obb, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&obb);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Obb& obb, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&obb);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Obb& obb, const SweptTriangle& sweptTri, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&obb);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Plane& plane, const Ray& ray, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RayPlane(ray.Start,ray.Direction,plane.GetNormal(),plane.GetDistance(), &point);
  if(ret < Intersection::None)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Plane& plane, const Segment& segment, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::SegmentPlane(segment.Start,segment.End,plane.GetNormal(),plane.GetDistance(), &point);
  if(ret < Intersection::None)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Plane& plane, const Aabb& aabb, Intersection::Manifold* manifold)
{
  bool ret = Collide(aabb,plane, manifold);
  if(ret == false)
    return false;
  if(manifold != NULL)
    FlipManifoldInfo(manifold);
  return true;
}

bool Collide(const Plane& plane, const Capsule& capsule, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Plane& plane, const Cylinder& cylinder, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Plane& plane, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Plane& plane, const Frustum& frustum, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Plane& plane, const Obb& obb, Intersection::Manifold* manifold)
{
  bool ret = Collide(obb,plane, manifold);
  if(ret == false)
    return false;
  if(manifold != NULL)
    FlipManifoldInfo(manifold);
  return true;
}

bool Collide(const Plane& plane1, const Plane& plane2, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Plane& plane, const Sphere& sphere, Intersection::Manifold* manifold)
{
  Intersection::Type ret = Intersection::PlaneSphere(plane.GetNormal(),plane.GetDistance(),sphere.mCenter,sphere.mRadius, manifold);
  if(ret < (Intersection::Type)0)
    return false;
  return true;
}

bool Collide(const Plane& plane, const Triangle& triangle, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Plane& plane, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Plane& plane, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Plane& plane, const SweptTriangle& sweptTri, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Sphere& sphere, const Ray& ray, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RaySphere(ray.Start,ray.Direction,sphere.mCenter,sphere.mRadius, &point);
  if(ret < Intersection::None)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Sphere& sphere, const Segment& segment, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::SegmentSphere(segment.Start,segment.End,sphere.mCenter,sphere.mRadius, &point);
  if(ret < Intersection::None)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Sphere& sphere, const Aabb& aabb, Intersection::Manifold* manifold)
{
  bool ret = Collide(aabb,sphere, manifold);
  if(ret == false)
    return false;
  if(manifold != NULL)
    FlipManifoldInfo(manifold);
  return true;
}

bool Collide(const Sphere& sphere, const Capsule& capsule, Intersection::Manifold* manifold)
{
  bool ret = Collide(capsule,sphere, manifold);
  if(ret == false)
    return false;
  if(manifold != NULL)
    FlipManifoldInfo(manifold);
  return true;
}

bool Collide(const Sphere& sphere, const Cylinder& cylinder, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sphere);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Sphere& sphere, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sphere);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Sphere& sphere, const Frustum& frustum, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sphere);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Sphere& sphere, const Obb& obb, Intersection::Manifold* manifold)
{
  bool ret = Collide(obb,sphere, manifold);
  if(ret == false)
    return false;
  if(manifold != NULL)
    FlipManifoldInfo(manifold);
  return true;
}

bool Collide(const Sphere& sphere, const Plane& plane, Intersection::Manifold* manifold)
{
  bool ret = Collide(plane,sphere, manifold);
  if(ret == false)
    return false;
  if(manifold != NULL)
    FlipManifoldInfo(manifold);
  return true;
}

bool Collide(const Sphere& sphere1, const Sphere& sphere2, Intersection::Manifold* manifold)
{
  Intersection::Type ret = Intersection::SphereSphere(sphere1.mCenter,sphere1.mRadius,sphere2.mCenter,sphere2.mRadius, manifold);
  if(ret < (Intersection::Type)0)
    return false;
  return true;
}

bool Collide(const Sphere& sphere, const Triangle& triangle, Intersection::Manifold* manifold)
{
  Intersection::Type ret = Intersection::SphereTriangle(sphere.mCenter,sphere.mRadius,triangle.p0,triangle.p1,triangle.p2, manifold);
  if(ret < (Intersection::Type)0)
    return false;
  return true;
}

bool Collide(const Sphere& sphere, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sphere);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Sphere& sphere, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sphere);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Sphere& sphere, const SweptTriangle& sweptTri, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sphere);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Triangle& triangle, const Ray& ray, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RayTriangle(ray.Start,ray.Direction,triangle.p0,triangle.p1,triangle.p2, &point);
  if(ret < Intersection::None)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Triangle& triangle, const Segment& segment, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::SegmentTriangle(segment.Start,segment.End,triangle.p0,triangle.p1,triangle.p2, &point);
  if(ret < Intersection::None)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Triangle& triangle, const Aabb& aabb, Intersection::Manifold* manifold)
{
  bool ret = Collide(aabb,triangle, manifold);
  if(ret == false)
    return false;
  if(manifold != NULL)
    FlipManifoldInfo(manifold);
  return true;
}

bool Collide(const Triangle& triangle, const Capsule& capsule, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&triangle);
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Triangle& triangle, const Cylinder& cylinder, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&triangle);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Triangle& triangle, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&triangle);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Triangle& triangle, const Frustum& frustum, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&triangle);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Triangle& triangle, const Obb& obb, Intersection::Manifold* manifold)
{
  bool ret = Collide(obb,triangle, manifold);
  if(ret == false)
    return false;
  if(manifold != NULL)
    FlipManifoldInfo(manifold);
  return true;
}

bool Collide(const Triangle& triangle, const Plane& plane, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Triangle& triangle, const Sphere& sphere, Intersection::Manifold* manifold)
{
  bool ret = Collide(sphere,triangle, manifold);
  if(ret == false)
    return false;
  if(manifold != NULL)
    FlipManifoldInfo(manifold);
  return true;
}

bool Collide(const Triangle& triangle1, const Triangle& triangle2, Intersection::Manifold* manifold)
{
  Intersection::Type ret = Intersection::TriangleTriangle(triangle1.p0,triangle1.p1,triangle1.p2,triangle2.p0,triangle2.p1,triangle2.p2, manifold);
  if(ret < (Intersection::Type)0)
    return false;
  return true;
}

bool Collide(const Triangle& triangle, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&triangle);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Triangle& triangle, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&triangle);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Triangle& triangle, const SweptTriangle& sweptTri, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&triangle);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Tetrahedron& tetrahedron, const Ray& ray, Intersection::Manifold* manifold)
{
  Intersection::IntersectionPoint point;
  Intersection::Type ret = Intersection::RayTetrahedron(ray.Start,ray.Direction,tetrahedron.p0,tetrahedron.p1,tetrahedron.p2,tetrahedron.p3, &point);
  if(ret < Intersection::None)
    return false;
  if(manifold != NULL)
  {
    manifold->Normal.Set(1,0,0);
    manifold->Points[0] = point;
    if(ret == Intersection::Point)
      manifold->PointCount = 1;
    else
      manifold->PointCount = 2;
  }
  return true;
}

bool Collide(const Tetrahedron& tetrahedron, const Segment& segment, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Tetrahedron& tetrahedron, const Aabb& aabb, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&aabb);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Tetrahedron& tetrahedron, const Capsule& capsule, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Tetrahedron& tetrahedron, const Cylinder& cylinder, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Tetrahedron& tetrahedron, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Tetrahedron& tetrahedron, const Frustum& frustum, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Tetrahedron& tetrahedron, const Obb& obb, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&obb);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Tetrahedron& tetrahedron, const Plane& plane, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const Tetrahedron& tetrahedron, const Sphere& sphere, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&sphere);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Tetrahedron& tetrahedron, const Triangle& triangle, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&triangle);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Tetrahedron& tetrahedron1, const Tetrahedron& tetrahedron2, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron1);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron2);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Tetrahedron& tetrahedron, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const Tetrahedron& tetrahedron, const SweptTriangle& sweptTri, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const ConvexMeshShape& supportShape, const Ray& ray, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const ConvexMeshShape& supportShape, const Segment& segment, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const ConvexMeshShape& supportShape, const Aabb& aabb, Intersection::Manifold* manifold)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&aabb);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const ConvexMeshShape& supportShape, const Capsule& capsule, Intersection::Manifold* manifold)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const ConvexMeshShape& supportShape, const Cylinder& cylinder, Intersection::Manifold* manifold)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const ConvexMeshShape& supportShape, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const ConvexMeshShape& supportShape, const Frustum& frustum, Intersection::Manifold* manifold)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const ConvexMeshShape& supportShape, const Obb& obb, Intersection::Manifold* manifold)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&obb);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const ConvexMeshShape& supportShape, const Plane& plane, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const ConvexMeshShape& supportShape, const Sphere& sphere, Intersection::Manifold* manifold)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&sphere);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const ConvexMeshShape& supportShape, const Triangle& triangle, Intersection::Manifold* manifold)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&triangle);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const ConvexMeshShape& supportShape, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const ConvexMeshShape& supportShape1, const ConvexMeshShape& supportShape2, Intersection::Manifold* manifold)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape1.mSupport;
  const Intersection::SupportShape& b = supportShape2.mSupport;
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const ConvexMeshShape& supportShape, const SweptTriangle& sweptTri, Intersection::Manifold* manifold)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const SweptTriangle& sweptTri, const Ray& ray, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const SweptTriangle& sweptTri, const Segment& segment, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const SweptTriangle& sweptTri, const Aabb& aabb, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&aabb);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const SweptTriangle& sweptTri, const Capsule& capsule, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const SweptTriangle& sweptTri, const Cylinder& cylinder, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const SweptTriangle& sweptTri, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const SweptTriangle& sweptTri, const Frustum& frustum, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const SweptTriangle& sweptTri, const Obb& obb, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&obb);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const SweptTriangle& sweptTri, const Plane& plane, Intersection::Manifold* manifold)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Collide(const SweptTriangle& sweptTri, const Sphere& sphere, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&sphere);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const SweptTriangle& sweptTri, const Triangle& triangle, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&triangle);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const SweptTriangle& sweptTri, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const SweptTriangle& sweptTri, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeCollide(a,b,manifold);
}

bool Collide(const SweptTriangle& sweptTri1, const SweptTriangle& sweptTri2, Intersection::Manifold* manifold)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri1);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri2);
  return SupportShapeCollide(a,b,manifold);
}

}//namespace Zero
