///////////////////////////////////////////////////////////////////////////////
///
///	\file RayAndSegmentTests.cpp
///	Unit tests for the ray and line segment intersection tests.
///	
///	Authors: Benjamin Strukus
///	Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
#include "Intersection/UnitTestCommon.hpp"

using namespace Intersection;

namespace
{
Mat3 xRotate, yRotate, zRotate;

void RotateXYZ(real x, real y, real z, Mat3Ref mtx)
{
  xRotate.Rotate(Vec3::cXAxis, Math::DegToRad(x));
  yRotate.Rotate(Vec3::cYAxis, Math::DegToRad(y));
  zRotate.Rotate(Vec3::cZAxis, Math::DegToRad(z));
  mtx = zRotate;
  mtx = Math::Multiply(mtx, yRotate);
  mtx = Math::Multiply(mtx, xRotate);
}
} //namespace

//------------------------------------------------------------------- Ray-Aabb 1
TEST(RayAabb_1)
{
  IntersectionPoint data;
  Vec3 ray[2] = { Vec3(real(0.5), real(0.5), real(0.5)), Vec3::cYAxis };
  Vec3 box[2] = { Vec3(real(-1.0), real(-1.0), real(-1.0)),
                  Vec3(real( 1.0), real( 1.0), real( 1.0)) };
  Intersection::Type result = RayAabb(ray[0], ray[1], box[0], box[1], &data);
  CHECK_EQUAL(Point, result);
}

//------------------------------------------------------------------- Ray-Aabb 2
TEST(RayAabb_2)
{
  IntersectionPoint data;
  Vec3 ray[2] = { Vec3::cZero, Vec3::cYAxis };
  Vec3 box[2] = { Vec3(real(-1.0), real(-1.0), real(-1.0)),
                  Vec3(real( 1.0), real( 1.0), real( 1.0)) };
  Intersection::Type result = RayAabb(ray[0], ray[1], box[0], box[1], &data);
  CHECK_EQUAL(Point, result);
}

//---------------------------------------------------------------- Ray-Capsule 1
TEST(RayCapsule_1)
{
  IntersectionPoint data;
  Vec3 ray[2] = { Vec3::cZero, Vec3::cYAxis };
  Vec3 capsule[2] = { Vec3(real(0.0), real(3.0), real(0.0)),
                      Vec3(real(0.0), real(5.0), real(0.0)) };
  real radius = real(0.5);

  //Capsule at (0,3,0)->(0,5,0) with a radius of 0.5 intersected by ray at 
  //origin pointing directly along the y-axis
  Intersection::Type result = RayCapsule(ray[0], ray[1], capsule[0], capsule[1], radius,
                           &data);
  
  CHECK_EQUAL(Segment, result);

  Vec3 points[2] = { Vec3(real(0.0), real(2.5), real(0.0)),
                     Vec3(real(0.0), real(5.5), real(0.0)) };
  CHECK_VEC3_CLOSE(points[0], data.Points[0], zero);
  CHECK_VEC3_CLOSE(points[1], data.Points[1], zero);
}

//---------------------------------------------------------------- Ray-Capsule 2
TEST(RayCapsule_2)
{
  IntersectionPoint data;
  Vec3 ray[2] = { Vec3::cZero, 
                  Normalized(Vec3(real(1.0), real(0.0), real(1.0))) };
  Vec3 capsule[2] = { Vec3::cXAxis * real(5.0), Vec3::cZAxis * real(5.0) };
  real radius = real(0.5);

  Intersection::Type result = RayCapsule(ray[0], ray[1], capsule[0], capsule[1], radius,
                           &data);
  CHECK_EQUAL(Segment, result);
}

//---------------------------------------------------------------- Ray-Capsule 3
TEST(RayCapsule_3)
{
  IntersectionPoint data;
  Vec3 ray[2] = { Vec3(real(0.8), real(8.0), real(0.0)),
                  Normalized(Vec3(real(-0.7), real(-0.9), real(0.0))) };
  Vec3 capsule[2] = { Vec3(real(0.0), real(3.0), real(0.0)),
                      Vec3(real(0.0), real(6.0), real(0.0)) };
  real radius = real(1.0);

  Intersection::Type result = RayCapsule(ray[0], ray[1], capsule[0], capsule[1], radius,
                           &data);
  CHECK_EQUAL(Segment, result);
}

//---------------------------------------------------------------- Ray-Capsule 4
TEST(RayCapsule_4)
{
  IntersectionPoint data;
  Vec3 ray[2] = { Vec3(real(2.4), real(1.2), real(0.0)),
                  Normalized(Vec3(real(-1.0), real(1.0), real(0.0))) };
  Vec3 capsule[2] = { Vec3(real(0.0), real(3.0), real(0.0)),
                      Vec3(real(0.0), real(6.0), real(0.0)) };
  real radius = real(1.0);

  Intersection::Type result = RayCapsule(ray[0], ray[1], capsule[0], capsule[1], radius,
                           &data);
  CHECK_EQUAL(Segment, result);
}

//---------------------------------------------------------------- Ray-Capsule 5
TEST(RayCapsule_5)
{
  IntersectionPoint data;
  Vec3 ray[2] = { Vec3(real(3.0), real(6.4), real(0.0)),
                  Normalized(Vec3(real(-1.0), real(0.0), real(0.0))) };
  Vec3 capsule[2] = { Vec3(real(0.0), real(3.0), real(0.0)),
                      Vec3(real(0.0), real(6.0), real(0.0)) };
  real radius = real(1.0);

  Intersection::Type result = RayCapsule(ray[0], ray[1], capsule[0], capsule[1], radius,
                           &data);
  CHECK_EQUAL(Segment, result);
}

//---------------------------------------------------------------- Ray-Capsule 6
TEST(RayCapsule_6)
{
  IntersectionPoint data;
  Vec3 ray[2] = { Vec3(real(-2.9), real(3.5), real(0.0)),
                  Normalized(Vec3(real(1.4), real(2.0), real(0.0))) };
  Vec3 capsule[2] = { Vec3(real(0.0), real(3.0), real(0.0)),
                      Vec3(real(0.0), real(6.0), real(0.0)) };
  real radius = real(1.0);

  Intersection::Type result = RayCapsule(ray[0], ray[1], capsule[0], capsule[1], radius,
                           &data);
  CHECK_EQUAL(Segment, result);
}

//---------------------------------------------------------------- Ray-Capsule 7
TEST(RayCapsule_7)
{
  IntersectionPoint data;
  Vec3 ray[2] = { Vec3(real(0.0), real( 4.5), real(0.0)),
                  Vec3(real(0.0), real(-1.0), real(0.0)) };
  Vec3 capsule[2] = { Vec3(real(0.0), real(3.0), real(0.0)),
                      Vec3(real(0.0), real(6.0), real(0.0)) };
  real radius = real(1.0);

  Intersection::Type result = RayCapsule(ray[0], ray[1], capsule[0], capsule[1], radius,
                           &data);
  CHECK_EQUAL(Point, result);
}

//----------------------------------------------------------------- Ray-Cylinder
TEST(RayCylinder)
{
  MARK_AS_UNIMPLEMENTED();
}

//-------------------------------------------------------------- Ray-Ellipsoid 1
TEST(RayEllipsoid_1)
{
  IntersectionPoint data;
  Vec3 ray[2] = { Vec3(real(0.5), real(0.5), real(0.5)), Vec3::cYAxis };
  Vec3 center = Vec3::cZero;
  Vec3 radii = Vec3(real(5.0), real(6.0), real(7.0));
  Mat3 basis = Mat3::cIdentity;

  Intersection::Type result = RayEllipsoid(ray[0], ray[1], center, radii, basis, &data);
  CHECK_EQUAL(Point, result);
}

//------------------------------------------------------------------ Ray-Frustum
TEST(RayFrustum)
{
  MARK_AS_UNIMPLEMENTED();
}

//-------------------------------------------------------------------- Ray-Plane
TEST(RayPlane)
{
  MARK_AS_UNIMPLEMENTED();
}

//---------------------------------------------------------------------- Ray-Obb
TEST(RayObb)
{
  MARK_AS_UNIMPLEMENTED();
//   Vec3 rayStart;
//   Vec3 rayDirection;
//   Vec3 obbCenter;
//   Vec3 obbHalfExtents;
//   Mat3 obbBasis;
//   IntersectionPoint data;
// 
//   // Ray = S(0, 0, 0), D(1, 1, 1)
//   rayStart.Set(real(0.0), real(0.0), real(0.0));
//   rayDirection.Set(real(1.0), real(1.0), real(1.0));
//   Normalize(rayDirection);
// 
//   // Obb = P(3, 2, 3), S(2, 2, 2), R(90, 90, 90)
//   obbCenter.Set(real(3.0), real(2.0), real(3.0));
//   obbHalfExtents.Set(real(1.0), real(1.0), real(1.0));
//   RotateXYZ(real(90.0), real(90.0), real(90.0), obbBasis);
// 
//   Type result = RayObb(rayStart, rayDirection, obbCenter, obbHalfExtents, 
//                        obbBasis, &data);
}

//------------------------------------------------------------------- Ray-Sphere
TEST(RaySphere)
{
  Vec3 ray[2] = { Vec3(real(0.5), real(0.5), real(0.5)), Vec3::cYAxis };
  Vec3 sphereCenter = Vec3::cZero;
  real sphereRadius = real(1.0);
  IntersectionPoint data;
  Intersection::Type result = RaySphere(ray[0], ray[1], sphereCenter, sphereRadius, &data);
  CHECK_EQUAL(Point, result);
}

//------------------------------------------------------------ Ray-Tetrahedron 1
TEST(RayTetrahedron_1)
{
  Vec3 ray[2] = { Vec3(real(-0.5), real(0.25), real(0.25)), Vec3::cXAxis };
  Vec3 tetrahedron[4] = { Vec3::cZero,  Vec3::cXAxis, 
                         Vec3::cYAxis, Vec3::cZAxis };
  IntersectionPoint data;
  Intersection::Type result = RayTetrahedron(ray[0], ray[1], tetrahedron[0], tetrahedron[1],
                               tetrahedron[2], tetrahedron[3], &data);
  CHECK_EQUAL(Segment, result);
}

//------------------------------------------------------------ Ray-Tetrahedron 2
TEST(RayTetrahedron_2)
{
  Vec3 ray[2] = { Vec3(real(-0.5), real(3.25), real(0.25)), Vec3::cXAxis };
  Vec3 tetrahedron[4] = { Vec3::cZero,  Vec3::cXAxis, 
                         Vec3::cYAxis, Vec3::cZAxis };
  IntersectionPoint data;
  Intersection::Type result = RayTetrahedron(ray[0], ray[1], tetrahedron[0], tetrahedron[1],
                               tetrahedron[2], tetrahedron[3], &data);
  CHECK_EQUAL(None, result);
}

//--------------------------------------------------------------- Ray-Triangle 1
TEST(RayTriangle_1)
{
  Vec3 ray[2] = { Vec3(real(0.0), real(0.0), real(0.0)),
                  Vec3(real(1.0), real(1.0), real(1.0))  };
  Vec3 triangle[3] = { Vec3(real(1.0), real(0.0), real(0.0)),
                       Vec3(real(0.0), real(1.0), real(0.0)),
                       Vec3(real(0.0), real(0.0), real(1.0)) };
  IntersectionPoint data;
  
  Intersection::Type result = RayTriangle(ray[0], ray[1], triangle[0], triangle[1],
                            triangle[2], &data);
  CHECK_EQUAL(Point, result);
}

//--------------------------------------------------------------- Ray-Triangle 2
TEST(RayTriangle_2)
{
  //{x=15.399451 y=9.1002789 z=19.057619 ...}
  Vec3 rayStart(15.399451f, 9.1002789f, 19.057619f);
  //{x=-0.24733621 y=-0.15453021 z=-0.95652765 ...}
  Vec3 rayDir(-0.24733621f, -0.15453021f, -0.95652765f);
  //{x=3.6769860 y=0.44818470 z=-0.34568286 ...}
  Vec3 p0(3.6769860f, 0.44818470f, -0.34568286f);
  //{x=4.1888504 y=0.37608734 z=-0.45132145 ...}
  Vec3 p1(4.1888504f, 0.37608734f, -0.45132145f);
  //{x=4.3038340 y=0.50659364 z=-0.81975907 ...}
  Vec3 p2(4.3038340f, 0.50659364f, -0.81975907f);

  //Intersect the ray against the triangle
  IntersectionPoint point;
  Intersection::Type resultType = RayTriangle(rayStart, rayDir, p0, p1, p2, &point);
  CHECK_EQUAL(None, resultType);
}

//----------------------------------------------------------------- Segment-Aabb
TEST(SegmentAabb)
{
  MARK_AS_UNIMPLEMENTED();
}

//-------------------------------------------------------------- Segment-Capsule
TEST(SegmentCapsule)
{
  MARK_AS_UNIMPLEMENTED();
}

//------------------------------------------------------------- Segment-Cylinder
TEST(SegmentCylinder)
{
  MARK_AS_UNIMPLEMENTED();
}

//-------------------------------------------------------------- Segment-Frustum
TEST(SegmentFrustum)
{
  MARK_AS_UNIMPLEMENTED();
}

//---------------------------------------------------------------- Segment-Plane
TEST(SegmentPlane)
{
  MARK_AS_UNIMPLEMENTED();
}

//------------------------------------------------------------------ Segment-Obb
TEST(SegmentObb)
{
  MARK_AS_UNIMPLEMENTED();
}

//--------------------------------------------------------------- Segment-Sphere
TEST(SegmentSphere)
{
  MARK_AS_UNIMPLEMENTED();
}

//------------------------------------------------------------- Segment-Triangle
TEST(SegmentTriangle)
{
  Vec3 segment[2] = { Vec3(real(0.0), real(0.0), real(0.0)),
                      Vec3(real(1.0), real(1.0), real(1.0)) };
  Vec3 triangle[3] = { Vec3(real(1.0), real(0.0), real(0.0)),
                       Vec3(real(0.0), real(1.0), real(0.0)),
                       Vec3(real(0.0), real(0.0), real(1.0)) };
  IntersectionPoint data;
  
  Intersection::Type result = RayTriangle(segment[0], segment[1], triangle[0], triangle[1],
                            triangle[2], &data);
  CHECK_EQUAL(Point, result);
}
