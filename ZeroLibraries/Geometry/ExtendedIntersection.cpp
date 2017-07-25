///////////////////////////////////////////////////////////////////////////////
///
/// \file ExtendedIntersection.cpp
/// NSquared intersection functions for the shape primitives. Wraps the
/// internal intersection functions for ease of use.
///
/// Authors: Joshua Davis, Auto-Generated
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

#include "Geometry/ExtendedIntersection.hpp"

#include "Geometry/Shapes.hpp"
#include "Geometry/Intersection.hpp"
#include "Geometry/Mpr.hpp"

namespace Zero
{

bool SupportShapeOverlap(const Intersection::SupportShape& a, const Intersection::SupportShape& b)
{
  //Test for collision.
  Intersection::Mpr mpr;
  Intersection::Type ret = mpr.Test(&a, &b, NULL);
  if(ret < (Intersection::Type)0)
    return false;
  return true;
}

bool Overlap(Vec3Param point1, Vec3Param point2)
{
  return Intersection::PointPoint(point1,point2) >= (Intersection::Type)0;
}

bool Overlap(Vec3Param point, const Ray& ray)
{
  return Intersection::PointRay(point,ray.Start,ray.Direction) >= (Intersection::Type)0;
}

bool Overlap(Vec3Param point, const Segment& segment)
{
  return Intersection::PointSegment(point,segment.Start,segment.End) >= (Intersection::Type)0;
}

bool Overlap(Vec3Param point, const Aabb& aabb)
{
  return Intersection::PointAabb(point,aabb.mMin,aabb.mMax) >= (Intersection::Type)0;
}

bool Overlap(Vec3Param point, const Capsule& capsule)
{
  return Intersection::PointCapsule(point,capsule.PointA,capsule.PointB,capsule.Radius) >= (Intersection::Type)0;
}

bool Overlap(Vec3Param point, const Cylinder& cylinder)
{
  return Intersection::PointCylinder(point,cylinder.PointA,cylinder.PointB,cylinder.Radius) >= (Intersection::Type)0;
}

bool Overlap(Vec3Param point, const Ellipsoid& ellipsoid)
{
  return Intersection::PointEllipsoid(point,ellipsoid.Center,ellipsoid.Radii,ellipsoid.Basis) >= (Intersection::Type)0;
}

bool Overlap(Vec3Param point, const Frustum& frustum)
{
  return Intersection::PointFrustum(point,frustum.GetIntersectionData()) >= (Intersection::Type)0;
}

bool Overlap(Vec3Param point, const Obb& obb)
{
  return Intersection::PointObb(point,obb.Center,obb.HalfExtents,obb.Basis) >= (Intersection::Type)0;
}

bool Overlap(Vec3Param point, const Plane& plane)
{
  return Intersection::PointPlane(point,plane.GetNormal(),plane.GetDistance()) >= (Intersection::Type)0;
}

bool Overlap(Vec3Param point, const Sphere& sphere)
{
  return Intersection::PointSphere(point,sphere.mCenter,sphere.mRadius) >= (Intersection::Type)0;
}

bool Overlap(Vec3Param point, const Triangle& triangle)
{
  return Intersection::PointTriangle(point,triangle.p0,triangle.p1,triangle.p2) >= (Intersection::Type)0;
}

bool Overlap(Vec3Param point, const Tetrahedron& tetrahedron)
{
  return Intersection::PointTetrahedron(point,tetrahedron.p0,tetrahedron.p1,tetrahedron.p2,tetrahedron.p3) >= (Intersection::Type)0;
}

bool Overlap(Vec3Param point, const ConvexMeshShape& supportShape)
{
  return Intersection::PointConvexShape(point,supportShape.mSupport) >= (Intersection::Type)0;
}

bool Overlap(Vec3Param point, const SweptTriangle& sweptTri)
{
  Intersection::SupportShape supportShape = Intersection::MakeSupport(&sweptTri);
  return Intersection::PointConvexShape(point, supportShape) >= (Intersection::Type)0;
}

bool Overlap(const Ray& ray, Vec3Param point)
{
  return Intersection::PointRay(point,ray.Start,ray.Direction) >= (Intersection::Type)0;
}

bool Overlap(const Ray& ray1, const Ray& ray2)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Ray& ray, const Segment& segment)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Ray& ray, const Aabb& aabb)
{
  return Intersection::RayAabb(ray.Start,ray.Direction,aabb.mMin,aabb.mMax) >= (Intersection::Type)0;
}

bool Overlap(const Ray& ray, const Capsule& capsule)
{
  return Intersection::RayCapsule(ray.Start,ray.Direction,capsule.PointA,capsule.PointB,capsule.Radius) >= (Intersection::Type)0;
}

bool Overlap(const Ray& ray, const Cylinder& cylinder)
{
  return Intersection::RayCylinder(ray.Start,ray.Direction,cylinder.PointA,cylinder.PointB,cylinder.Radius) >= (Intersection::Type)0;
}

bool Overlap(const Ray& ray, const Ellipsoid& ellipsoid)
{
  return Intersection::RayEllipsoid(ray.Start,ray.Direction,ellipsoid.Center,ellipsoid.Radii,ellipsoid.Basis) >= (Intersection::Type)0;
}

bool Overlap(const Ray& ray, const Frustum& frustum)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Ray& ray, const Obb& obb)
{
  return Intersection::RayObb(ray.Start,ray.Direction,obb.Center,obb.HalfExtents,obb.Basis) >= (Intersection::Type)0;
}

bool Overlap(const Ray& ray, const Plane& plane)
{
  return Intersection::RayPlane(ray.Start,ray.Direction,plane.GetNormal(),plane.GetDistance()) >= (Intersection::Type)0;
}

bool Overlap(const Ray& ray, const Sphere& sphere)
{
  return Intersection::RaySphere(ray.Start,ray.Direction,sphere.mCenter,sphere.mRadius) >= (Intersection::Type)0;
}

bool Overlap(const Ray& ray, const Triangle& triangle)
{
  return Intersection::RayTriangle(ray.Start,ray.Direction,triangle.p0,triangle.p1,triangle.p2) >= (Intersection::Type)0;
}

bool Overlap(const Ray& ray, const Tetrahedron& tetrahedron)
{
  return Intersection::RayTetrahedron(ray.Start,ray.Direction,tetrahedron.p0,tetrahedron.p1,tetrahedron.p2,tetrahedron.p3) >= (Intersection::Type)0;
}

bool Overlap(const Ray& ray, const ConvexMeshShape& supportShape)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Ray& ray, const SweptTriangle& sweptTri)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Segment& segment, Vec3Param point)
{
  return Intersection::PointSegment(point,segment.Start,segment.End) >= (Intersection::Type)0;
}

bool Overlap(const Segment& segment, const Ray& ray)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Segment& segment1, const Segment& segment2)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Segment& segment, const Aabb& aabb)
{
  return Intersection::SegmentAabb(segment.Start,segment.End,aabb.mMin,aabb.mMax) >= (Intersection::Type)0;
}

bool Overlap(const Segment& segment, const Capsule& capsule)
{
  return Intersection::SegmentCapsule(segment.Start,segment.End,capsule.PointA,capsule.PointB,capsule.Radius) >= (Intersection::Type)0;
}

bool Overlap(const Segment& segment, const Cylinder& cylinder)
{
  return Intersection::SegmentCylinder(segment.Start,segment.End,cylinder.PointA,cylinder.PointB,cylinder.Radius) >= (Intersection::Type)0;
}

bool Overlap(const Segment& segment, const Ellipsoid& ellipsoid)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Segment& segment, const Frustum& frustum)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Segment& segment, const Obb& obb)
{
  return Intersection::SegmentObb(segment.Start,segment.End,obb.Center,obb.HalfExtents,obb.Basis) >= (Intersection::Type)0;
}

bool Overlap(const Segment& segment, const Plane& plane)
{
  return Intersection::SegmentPlane(segment.Start,segment.End,plane.GetNormal(),plane.GetDistance()) >= (Intersection::Type)0;
}

bool Overlap(const Segment& segment, const Sphere& sphere)
{
  return Intersection::SegmentSphere(segment.Start,segment.End,sphere.mCenter,sphere.mRadius) >= (Intersection::Type)0;
}

bool Overlap(const Segment& segment, const Triangle& triangle)
{
  return Intersection::SegmentTriangle(segment.Start,segment.End,triangle.p0,triangle.p1,triangle.p2) >= (Intersection::Type)0;
}

bool Overlap(const Segment& segment, const Tetrahedron& tetrahedron)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Segment& segment, const ConvexMeshShape& supportShape)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Segment& segment, const SweptTriangle& sweptTri)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Aabb& aabb, Vec3Param point)
{
  return Intersection::PointAabb(point,aabb.mMin,aabb.mMax) >= (Intersection::Type)0;
}

bool Overlap(const Aabb& aabb, const Ray& ray)
{
  return Intersection::RayAabb(ray.Start,ray.Direction,aabb.mMin,aabb.mMax) >= (Intersection::Type)0;
}

bool Overlap(const Aabb& aabb, const Segment& segment)
{
  return Intersection::SegmentAabb(segment.Start,segment.End,aabb.mMin,aabb.mMax) >= (Intersection::Type)0;
}

bool Overlap(const Aabb& aabb1, const Aabb& aabb2)
{
  return Intersection::AabbAabb(aabb1.mMin,aabb1.mMax,aabb2.mMin,aabb2.mMax) >= (Intersection::Type)0;
}

bool Overlap(const Aabb& aabb, const Capsule& capsule)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&aabb);
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Aabb& aabb, const Cylinder& cylinder)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&aabb);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Aabb& aabb, const Ellipsoid& ellipsoid)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&aabb);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Aabb& aabb, const Frustum& frustum)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&aabb);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Aabb& aabb, const Obb& obb)
{
  return Intersection::AabbObb(aabb.mMin,aabb.mMax,obb.Center,obb.HalfExtents,obb.Basis) >= (Intersection::Type)0;
}

bool Overlap(const Aabb& aabb, const Plane& plane)
{
  return Intersection::AabbPlane(aabb.mMin,aabb.mMax,plane.GetNormal(),plane.GetDistance()) >= (Intersection::Type)0;
}

bool Overlap(const Aabb& aabb, const Sphere& sphere)
{
  return Intersection::AabbSphere(aabb.mMin,aabb.mMax,sphere.mCenter,sphere.mRadius) >= (Intersection::Type)0;
}

bool Overlap(const Aabb& aabb, const Triangle& triangle)
{
  return Intersection::AabbTriangle(aabb.mMin,aabb.mMax,triangle.p0,triangle.p1,triangle.p2) >= (Intersection::Type)0;
}

bool Overlap(const Aabb& aabb, const Tetrahedron& tetrahedron)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&aabb);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Aabb& aabb, const ConvexMeshShape& supportShape)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&aabb);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Aabb& aabb, const SweptTriangle& sweptTri)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&aabb);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Capsule& capsule, Vec3Param point)
{
  return Intersection::PointCapsule(point,capsule.PointA,capsule.PointB,capsule.Radius) >= (Intersection::Type)0;
}

bool Overlap(const Capsule& capsule, const Ray& ray)
{
  return Intersection::RayCapsule(ray.Start,ray.Direction,capsule.PointA,capsule.PointB,capsule.Radius) >= (Intersection::Type)0;
}

bool Overlap(const Capsule& capsule, const Segment& segment)
{
  return Intersection::SegmentCapsule(segment.Start,segment.End,capsule.PointA,capsule.PointB,capsule.Radius) >= (Intersection::Type)0;
}

bool Overlap(const Capsule& capsule, const Aabb& aabb)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  Intersection::SupportShape b = Intersection::MakeSupport(&aabb);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Capsule& capsule1, const Capsule& capsule2)
{
  return Intersection::CapsuleCapsule(capsule1.PointA,capsule1.PointB,capsule1.Radius,capsule2.PointA,capsule2.PointB,capsule2.Radius) >= (Intersection::Type)0;
}

bool Overlap(const Capsule& capsule, const Cylinder& cylinder)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Capsule& capsule, const Ellipsoid& ellipsoid)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Capsule& capsule, const Frustum& frustum)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Capsule& capsule, const Obb& obb)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  Intersection::SupportShape b = Intersection::MakeSupport(&obb);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Capsule& capsule, const Plane& plane)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Capsule& capsule, const Sphere& sphere)
{
  return Intersection::CapsuleSphere(capsule.PointA,capsule.PointB,capsule.Radius,sphere.mCenter,sphere.mRadius) >= (Intersection::Type)0;
}

bool Overlap(const Capsule& capsule, const Triangle& triangle)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  Intersection::SupportShape b = Intersection::MakeSupport(&triangle);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Capsule& capsule, const Tetrahedron& tetrahedron)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Capsule& capsule, const ConvexMeshShape& supportShape)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Capsule& capsule, const SweptTriangle& sweptTri)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&capsule);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Cylinder& cylinder, Vec3Param point)
{
  return Intersection::PointCylinder(point,cylinder.PointA,cylinder.PointB,cylinder.Radius) >= (Intersection::Type)0;
}

bool Overlap(const Cylinder& cylinder, const Ray& ray)
{
  return Intersection::RayCylinder(ray.Start,ray.Direction,cylinder.PointA,cylinder.PointB,cylinder.Radius) >= (Intersection::Type)0;
}

bool Overlap(const Cylinder& cylinder, const Segment& segment)
{
  return Intersection::SegmentCylinder(segment.Start,segment.End,cylinder.PointA,cylinder.PointB,cylinder.Radius) >= (Intersection::Type)0;
}

bool Overlap(const Cylinder& cylinder, const Aabb& aabb)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&aabb);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Cylinder& cylinder, const Capsule& capsule)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Cylinder& cylinder1, const Cylinder& cylinder2)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder1);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder2);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Cylinder& cylinder, const Ellipsoid& ellipsoid)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Cylinder& cylinder, const Frustum& frustum)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Cylinder& cylinder, const Obb& obb)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&obb);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Cylinder& cylinder, const Plane& plane)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Cylinder& cylinder, const Sphere& sphere)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&sphere);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Cylinder& cylinder, const Triangle& triangle)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&triangle);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Cylinder& cylinder, const Tetrahedron& tetrahedron)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Cylinder& cylinder, const ConvexMeshShape& supportShape)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Cylinder& cylinder, const SweptTriangle& sweptTri)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&cylinder);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Ellipsoid& ellipsoid, Vec3Param point)
{
  return Intersection::PointEllipsoid(point,ellipsoid.Center,ellipsoid.Radii,ellipsoid.Basis) >= (Intersection::Type)0;
}

bool Overlap(const Ellipsoid& ellipsoid, const Ray& ray)
{
  return Intersection::RayEllipsoid(ray.Start,ray.Direction,ellipsoid.Center,ellipsoid.Radii,ellipsoid.Basis) >= (Intersection::Type)0;
}

bool Overlap(const Ellipsoid& ellipsoid, const Segment& segment)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Ellipsoid& ellipsoid, const Aabb& aabb)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&aabb);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Ellipsoid& ellipsoid, const Capsule& capsule)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Ellipsoid& ellipsoid, const Cylinder& cylinder)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Ellipsoid& ellipsoid1, const Ellipsoid& ellipsoid2)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid1);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid2);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Ellipsoid& ellipsoid, const Frustum& frustum)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Ellipsoid& ellipsoid, const Obb& obb)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&obb);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Ellipsoid& ellipsoid, const Plane& plane)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Ellipsoid& ellipsoid, const Sphere& sphere)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&sphere);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Ellipsoid& ellipsoid, const Triangle& triangle)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&triangle);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Ellipsoid& ellipsoid, const Tetrahedron& tetrahedron)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Ellipsoid& ellipsoid, const ConvexMeshShape& supportShape)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Ellipsoid& ellipsoid, const SweptTriangle& sweptTri)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&ellipsoid);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Frustum& frustum, Vec3Param point)
{
  return Intersection::PointFrustum(point,frustum.GetIntersectionData()) >= (Intersection::Type)0;
}

bool Overlap(const Frustum& frustum, const Ray& ray)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Frustum& frustum, const Segment& segment)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Frustum& frustum, const Aabb& aabb)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&aabb);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Frustum& frustum, const Capsule& capsule)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Frustum& frustum, const Cylinder& cylinder)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Frustum& frustum, const Ellipsoid& ellipsoid)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Frustum& frustum1, const Frustum& frustum2)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum1);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum2);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Frustum& frustum, const Obb& obb)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&obb);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Frustum& frustum, const Plane& plane)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Frustum& frustum, const Sphere& sphere)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&sphere);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Frustum& frustum, const Triangle& triangle)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&triangle);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Frustum& frustum, const Tetrahedron& tetrahedron)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Frustum& frustum, const ConvexMeshShape& supportShape)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Frustum& frustum, const SweptTriangle& sweptTri)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&frustum);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Obb& obb, Vec3Param point)
{
  return Intersection::PointObb(point,obb.Center,obb.HalfExtents,obb.Basis) >= (Intersection::Type)0;
}

bool Overlap(const Obb& obb, const Ray& ray)
{
  return Intersection::RayObb(ray.Start,ray.Direction,obb.Center,obb.HalfExtents,obb.Basis) >= (Intersection::Type)0;
}

bool Overlap(const Obb& obb, const Segment& segment)
{
  return Intersection::SegmentObb(segment.Start,segment.End,obb.Center,obb.HalfExtents,obb.Basis) >= (Intersection::Type)0;
}

bool Overlap(const Obb& obb, const Aabb& aabb)
{
  return Intersection::AabbObb(aabb.mMin,aabb.mMax,obb.Center,obb.HalfExtents,obb.Basis) >= (Intersection::Type)0;
}

bool Overlap(const Obb& obb, const Capsule& capsule)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&obb);
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Obb& obb, const Cylinder& cylinder)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&obb);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Obb& obb, const Ellipsoid& ellipsoid)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&obb);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Obb& obb, const Frustum& frustum)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&obb);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Obb& obb1, const Obb& obb2)
{
  return Intersection::ObbObb(obb1.Center,obb1.HalfExtents,obb1.Basis,obb2.Center,obb2.HalfExtents,obb2.Basis) >= (Intersection::Type)0;
}

bool Overlap(const Obb& obb, const Plane& plane)
{
  return Intersection::ObbPlane(obb.Center,obb.HalfExtents,obb.Basis,plane.GetNormal(),plane.GetDistance()) >= (Intersection::Type)0;
}

bool Overlap(const Obb& obb, const Sphere& sphere)
{
  return Intersection::ObbSphere(obb.Center,obb.HalfExtents,obb.Basis,sphere.mCenter,sphere.mRadius) >= (Intersection::Type)0;
}

bool Overlap(const Obb& obb, const Triangle& triangle)
{
  return Intersection::ObbTriangle(obb.Center,obb.HalfExtents,obb.Basis,triangle.p0,triangle.p1,triangle.p2) >= (Intersection::Type)0;
}

bool Overlap(const Obb& obb, const Tetrahedron& tetrahedron)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&obb);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Obb& obb, const ConvexMeshShape& supportShape)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&obb);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Obb& obb, const SweptTriangle& sweptTri)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&obb);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Plane& plane, Vec3Param point)
{
  return Intersection::PointPlane(point,plane.GetNormal(),plane.GetDistance()) >= (Intersection::Type)0;
}

bool Overlap(const Plane& plane, const Ray& ray)
{
  return Intersection::RayPlane(ray.Start,ray.Direction,plane.GetNormal(),plane.GetDistance()) >= (Intersection::Type)0;
}

bool Overlap(const Plane& plane, const Segment& segment)
{
  return Intersection::SegmentPlane(segment.Start,segment.End,plane.GetNormal(),plane.GetDistance()) >= (Intersection::Type)0;
}

bool Overlap(const Plane& plane, const Aabb& aabb)
{
  return Intersection::AabbPlane(aabb.mMin,aabb.mMax,plane.GetNormal(),plane.GetDistance()) >= (Intersection::Type)0;
}

bool Overlap(const Plane& plane, const Capsule& capsule)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Plane& plane, const Cylinder& cylinder)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Plane& plane, const Ellipsoid& ellipsoid)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Plane& plane, const Frustum& frustum)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Plane& plane, const Obb& obb)
{
  return Intersection::ObbPlane(obb.Center,obb.HalfExtents,obb.Basis,plane.GetNormal(),plane.GetDistance()) >= (Intersection::Type)0;
}

bool Overlap(const Plane& plane1, const Plane& plane2)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Plane& plane, const Sphere& sphere)
{
  return Intersection::PlaneSphere(plane.GetNormal(),plane.GetDistance(),sphere.mCenter,sphere.mRadius) >= (Intersection::Type)0;
}

bool Overlap(const Plane& plane, const Triangle& triangle)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Plane& plane, const Tetrahedron& tetrahedron)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Plane& plane, const ConvexMeshShape& supportShape)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Plane& plane, const SweptTriangle& sweptTri)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Sphere& sphere, Vec3Param point)
{
  return Intersection::PointSphere(point,sphere.mCenter,sphere.mRadius) >= (Intersection::Type)0;
}

bool Overlap(const Sphere& sphere, const Ray& ray)
{
  return Intersection::RaySphere(ray.Start,ray.Direction,sphere.mCenter,sphere.mRadius) >= (Intersection::Type)0;
}

bool Overlap(const Sphere& sphere, const Segment& segment)
{
  return Intersection::SegmentSphere(segment.Start,segment.End,sphere.mCenter,sphere.mRadius) >= (Intersection::Type)0;
}

bool Overlap(const Sphere& sphere, const Aabb& aabb)
{
  return Intersection::AabbSphere(aabb.mMin,aabb.mMax,sphere.mCenter,sphere.mRadius) >= (Intersection::Type)0;
}

bool Overlap(const Sphere& sphere, const Capsule& capsule)
{
  return Intersection::CapsuleSphere(capsule.PointA,capsule.PointB,capsule.Radius,sphere.mCenter,sphere.mRadius) >= (Intersection::Type)0;
}

bool Overlap(const Sphere& sphere, const Cylinder& cylinder)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sphere);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Sphere& sphere, const Ellipsoid& ellipsoid)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sphere);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Sphere& sphere, const Frustum& frustum)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sphere);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Sphere& sphere, const Obb& obb)
{
  return Intersection::ObbSphere(obb.Center,obb.HalfExtents,obb.Basis,sphere.mCenter,sphere.mRadius) >= (Intersection::Type)0;
}

bool Overlap(const Sphere& sphere, const Plane& plane)
{
  return Intersection::PlaneSphere(plane.GetNormal(),plane.GetDistance(),sphere.mCenter,sphere.mRadius) >= (Intersection::Type)0;
}

bool Overlap(const Sphere& sphere1, const Sphere& sphere2)
{
  return Intersection::SphereSphere(sphere1.mCenter,sphere1.mRadius,sphere2.mCenter,sphere2.mRadius) >= (Intersection::Type)0;
}

bool Overlap(const Sphere& sphere, const Triangle& triangle)
{
  return Intersection::SphereTriangle(sphere.mCenter,sphere.mRadius,triangle.p0,triangle.p1,triangle.p2) >= (Intersection::Type)0;
}

bool Overlap(const Sphere& sphere, const Tetrahedron& tetrahedron)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sphere);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Sphere& sphere, const ConvexMeshShape& supportShape)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sphere);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Sphere& sphere, const SweptTriangle& sweptTri)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sphere);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Triangle& triangle, Vec3Param point)
{
  return Intersection::PointTriangle(point,triangle.p0,triangle.p1,triangle.p2) >= (Intersection::Type)0;
}

bool Overlap(const Triangle& triangle, const Ray& ray)
{
  return Intersection::RayTriangle(ray.Start,ray.Direction,triangle.p0,triangle.p1,triangle.p2) >= (Intersection::Type)0;
}

bool Overlap(const Triangle& triangle, const Segment& segment)
{
  return Intersection::SegmentTriangle(segment.Start,segment.End,triangle.p0,triangle.p1,triangle.p2) >= (Intersection::Type)0;
}

bool Overlap(const Triangle& triangle, const Aabb& aabb)
{
  return Intersection::AabbTriangle(aabb.mMin,aabb.mMax,triangle.p0,triangle.p1,triangle.p2) >= (Intersection::Type)0;
}

bool Overlap(const Triangle& triangle, const Capsule& capsule)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&triangle);
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Triangle& triangle, const Cylinder& cylinder)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&triangle);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Triangle& triangle, const Ellipsoid& ellipsoid)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&triangle);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Triangle& triangle, const Frustum& frustum)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&triangle);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Triangle& triangle, const Obb& obb)
{
  return Intersection::ObbTriangle(obb.Center,obb.HalfExtents,obb.Basis,triangle.p0,triangle.p1,triangle.p2) >= (Intersection::Type)0;
}

bool Overlap(const Triangle& triangle, const Plane& plane)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Triangle& triangle, const Sphere& sphere)
{
  return Intersection::SphereTriangle(sphere.mCenter,sphere.mRadius,triangle.p0,triangle.p1,triangle.p2) >= (Intersection::Type)0;
}

bool Overlap(const Triangle& triangle1, const Triangle& triangle2)
{
  return Intersection::TriangleTriangle(triangle1.p0,triangle1.p1,triangle1.p2,triangle2.p0,triangle2.p1,triangle2.p2) >= (Intersection::Type)0;
}

bool Overlap(const Triangle& triangle, const Tetrahedron& tetrahedron)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&triangle);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Triangle& triangle, const ConvexMeshShape& supportShape)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&triangle);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Triangle& triangle, const SweptTriangle& sweptTri)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&triangle);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Tetrahedron& tetrahedron, Vec3Param point)
{
  return Intersection::PointTetrahedron(point,tetrahedron.p0,tetrahedron.p1,tetrahedron.p2,tetrahedron.p3) >= (Intersection::Type)0;
}

bool Overlap(const Tetrahedron& tetrahedron, const Ray& ray)
{
  return Intersection::RayTetrahedron(ray.Start,ray.Direction,tetrahedron.p0,tetrahedron.p1,tetrahedron.p2,tetrahedron.p3) >= (Intersection::Type)0;
}

bool Overlap(const Tetrahedron& tetrahedron, const Segment& segment)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Tetrahedron& tetrahedron, const Aabb& aabb)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&aabb);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Tetrahedron& tetrahedron, const Capsule& capsule)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Tetrahedron& tetrahedron, const Cylinder& cylinder)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Tetrahedron& tetrahedron, const Ellipsoid& ellipsoid)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Tetrahedron& tetrahedron, const Frustum& frustum)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Tetrahedron& tetrahedron, const Obb& obb)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&obb);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Tetrahedron& tetrahedron, const Plane& plane)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const Tetrahedron& tetrahedron, const Sphere& sphere)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&sphere);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Tetrahedron& tetrahedron, const Triangle& triangle)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&triangle);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Tetrahedron& tetrahedron1, const Tetrahedron& tetrahedron2)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron1);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron2);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Tetrahedron& tetrahedron, const ConvexMeshShape& supportShape)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeOverlap(a,b);
}

bool Overlap(const Tetrahedron& tetrahedron, const SweptTriangle& sweptTri)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&tetrahedron);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const ConvexMeshShape& supportShape, Vec3Param point)
{
  return Intersection::PointConvexShape(point,supportShape.mSupport) >= (Intersection::Type)0;
}

bool Overlap(const ConvexMeshShape& supportShape, const Ray& ray)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const ConvexMeshShape& supportShape, const Segment& segment)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const ConvexMeshShape& supportShape, const Aabb& aabb)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&aabb);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const ConvexMeshShape& supportShape, const Capsule& capsule)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const ConvexMeshShape& supportShape, const Cylinder& cylinder)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const ConvexMeshShape& supportShape, const Ellipsoid& ellipsoid)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const ConvexMeshShape& supportShape, const Frustum& frustum)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const ConvexMeshShape& supportShape, const Obb& obb)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&obb);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const ConvexMeshShape& supportShape, const Plane& plane)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const ConvexMeshShape& supportShape, const Sphere& sphere)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&sphere);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const ConvexMeshShape& supportShape, const Triangle& triangle)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&triangle);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const ConvexMeshShape& supportShape, const Tetrahedron& tetrahedron)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const ConvexMeshShape& supportShape1, const ConvexMeshShape& supportShape2)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape1.mSupport;
  const Intersection::SupportShape& b = supportShape2.mSupport;
  return SupportShapeOverlap(a,b);
}

bool Overlap(const ConvexMeshShape& supportShape, const SweptTriangle& sweptTri)
{
  //Test for collision.
  const Intersection::SupportShape& a = supportShape.mSupport;
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const SweptTriangle& sweptTri, Vec3Param point)
{
  Intersection::SupportShape supportShape = Intersection::MakeSupport(&sweptTri);
  return Intersection::PointConvexShape(point, supportShape) >= (Intersection::Type)0;
}

bool Overlap(const SweptTriangle& sweptTri, const Ray& ray)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const SweptTriangle& sweptTri, const Segment& segment)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const SweptTriangle& sweptTri, const Aabb& aabb)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&aabb);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const SweptTriangle& sweptTri, const Capsule& capsule)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&capsule);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const SweptTriangle& sweptTri, const Cylinder& cylinder)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&cylinder);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const SweptTriangle& sweptTri, const Ellipsoid& ellipsoid)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&ellipsoid);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const SweptTriangle& sweptTri, const Frustum& frustum)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&frustum);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const SweptTriangle& sweptTri, const Obb& obb)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&obb);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const SweptTriangle& sweptTri, const Plane& plane)
{
  ErrorIf(true,"Not Implemented");
  return false;
}

bool Overlap(const SweptTriangle& sweptTri, const Sphere& sphere)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&sphere);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const SweptTriangle& sweptTri, const Triangle& triangle)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&triangle);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const SweptTriangle& sweptTri, const Tetrahedron& tetrahedron)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  Intersection::SupportShape b = Intersection::MakeSupport(&tetrahedron);
  return SupportShapeOverlap(a,b);
}

bool Overlap(const SweptTriangle& sweptTri, const ConvexMeshShape& supportShape)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri);
  const Intersection::SupportShape& b = supportShape.mSupport;
  return SupportShapeOverlap(a,b);
}

bool Overlap(const SweptTriangle& sweptTri1, const SweptTriangle& sweptTri2)
{
  //Test for collision.
  Intersection::SupportShape a = Intersection::MakeSupport(&sweptTri1);
  Intersection::SupportShape b = Intersection::MakeSupport(&sweptTri2);
  return SupportShapeOverlap(a,b);
}

}//namespace Zero
