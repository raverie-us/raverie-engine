///////////////////////////////////////////////////////////////////////////////
///
///  \file PointContainmentTests.cpp
///  Unit tests for all of the Point_____ functions.
///	
///  Authors: Benjamin Strukus
///  Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
#include "Intersection/UnitTestCommon.hpp"

using namespace Intersection;

//---------------------------------------------------------------- Point-Point 1
TEST(PointPoint_1)
{
  Vec3 points[2] = { Vec3::cXAxis, Vec3::cXAxis };
  Intersection::Type result = PointPoint(points[0], points[1]);
  
  CHECK_EQUAL(Inside, result);
}

//---------------------------------------------------------------- Point-Point 2
TEST(PointPoint_2)
{
  Vec3 points[2] = { Vec3::cXAxis, Vec3::cYAxis };
  Intersection::Type result = PointPoint(points[0], points[1]);
  
  CHECK_EQUAL(None, result);
}

//------------------------------------------------------------------ Point-Ray 1
TEST(PointRay_1)
{
  Vec3 ray[2] = { Vec3::cZero, Vec3(real(1.0), real(1.0), real(1.0)) };
  Normalize(ray[1]);
  Vec3 point = Vec3(real(90.0), real(90.0), real(90.0));
  Intersection::Type result = PointRay(point, ray[0], ray[1]);
  
  CHECK_EQUAL(Inside, result);
}

//------------------------------------------------------------------ Point-Ray 2
TEST(PointRay_2)
{
  Vec3 ray[2] = { Vec3::cZero, Vec3(real(1.0), real(1.0), real(1.0)) };
  Normalize(ray[1]);
  Vec3 point = Vec3(real(-1.0), real(-1.0), real(-1.0));
  Intersection::Type result = PointRay(point, ray[0], ray[1]);

  CHECK_EQUAL(None, result);
}

//------------------------------------------------------------------ Point-Ray 3
TEST(PointRay_3)
{
  Vec3 ray[2] = { Vec3::cZero, Vec3::cXAxis };
  Vec3 point = Vec3::cZero;
  Intersection::Type result = PointRay(point, ray[0], ray[1]);

  CHECK_EQUAL(Inside, result);
}

//-------------------------------------------------------------- Point-Segment 1
TEST(PointSegment_1)
{
  Vec3 segment[2] = { Vec3::cXAxis, Vec3::cYAxis };
  Vec3 point = Vec3::cZAxis;
  Intersection::Type result = PointSegment(point, segment[0], segment[1]);

  CHECK_EQUAL(None, result);
}

//-------------------------------------------------------------- Point-Segment 2
TEST(PointSegment_2)
{
  Vec3 segment[2] = { Vec3::cZAxis, Vec3::cXAxis };
  Vec3 point = Vec3::cZAxis;
  Intersection::Type result = PointSegment(point, segment[0], segment[1]);

  CHECK_EQUAL(Inside, result);
}

//-------------------------------------------------------------- Point-Segment 3
TEST(PointSegment_3)
{
  Vec3 segment[2] = { Vec3::cZAxis, Vec3::cXAxis };
  Vec3 point = Vec3::cXAxis;
  Intersection::Type result = PointSegment(point, segment[0], segment[1]);

  CHECK_EQUAL(Inside, result);
}

//-------------------------------------------------------------- Point-Segment 4
TEST(PointSegment_4)
{
  Vec3 segment[2] = { Vec3::cZAxis, Vec3::cXAxis };
  Vec3 point = Vec3(real(0.5), real(0.0), real(0.5));
  Intersection::Type result = PointSegment(point, segment[0], segment[1]);

  CHECK_EQUAL(Inside, result);
}

//----------------------------------------------------------------- Point-Aabb 1
TEST(PointAabb_1)
{
  //Point outside axis-aligned bounding box
  Vec3 aabb[2] = { Vec3::cZero, Vec3(real(1.0), real(1.0), real(1.0)) };
  Vec3 point = Vec3(real(5.0), real(0.0), real(0.0));
  Intersection::Type result = PointAabb(point, aabb[0], aabb[1]);

  CHECK_EQUAL(None, result);
}

//----------------------------------------------------------------- Point-Aabb 2
TEST(PointAabb_2)
{
  //Point inside axis-aligned bounding box
  Vec3 aabb[2] = { Vec3::cZero, Vec3(real(1.0), real(1.0), real(1.0)) };
  Vec3 point = Vec3(real(0.5), real(0.1), real(0.7));
  Intersection::Type result = PointAabb(point, aabb[0], aabb[1]);

  CHECK_EQUAL(Inside, result);
}

//----------------------------------------------------------------- Point-Aabb 3
TEST(PointAabb_3)
{
  //Point lying on the surface of the axis-aligned bounding box
  Vec3 aabb[2] = { Vec3::cZero, Vec3(real(1.0), real(1.0), real(1.0)) };
  Vec3 point = Vec3(real(1.0), real(1.0), real(1.0));
  Intersection::Type result = PointAabb(point, aabb[0], aabb[1]);

  CHECK_EQUAL(Inside, result);
}

//-------------------------------------------------------------- Point-Capsule 1
TEST(PointCapsule_1)
{
  //Point lying outside the capsule
  Vec3 capsule[2] = { Vec3::cZero, Vec3(real(10.0), real(10.0), real(10.0)) };
  real radius = real(2.0);
  Vec3 point(real(10.0), real(10.0), real(0.0));
  Intersection::Type result = PointCapsule(point, capsule[0], capsule[1], radius);

  CHECK_EQUAL(None, result);
}

//-------------------------------------------------------------- Point-Capsule 2
TEST(PointCapsule_2)
{
  //Point lying inside the capsule
  Vec3 capsule[2] = { Vec3::cZero, Vec3(real(10.0), real(10.0), real(10.0)) };
  real radius = real(2.0);
  Vec3 point(real(5.0), real(6.0), real(7.0));
  Intersection::Type result = PointCapsule(point, capsule[0], capsule[1], radius);

  CHECK_EQUAL(Inside, result);
}

//-------------------------------------------------------------- Point-Capsule 3
TEST(PointCapsule_3)
{
  //Point lying on the surface of the capsule
  Vec3 capsule[2] = { Vec3::cZero, Vec3(real(10.0), real(10.0), real(10.0)) };
  real radius = real(2.0);
  Vec3 point(real(11.1547005383792515290182975610039), 
             real(11.1547005383792515290182975610039), 
             real(11.1547005383792515290182975610039));
  Intersection::Type result = PointCapsule(point, capsule[0], capsule[1], radius);

  CHECK_EQUAL(Inside, result);
}

//------------------------------------------------------------- Point-Cylinder 1
TEST(PointCylinder_1)
{
  //Point inside all elements of the cylinder
  Vec3 cylinder[2] = { Vec3::cZero, Vec3(real(10.0), real(10.0), real(10.0)) };
  real radius = real(2.0);
  Vec3 point(real(5.0), real(5.0), real(5.0));
  Intersection::Type result = PointCylinder(point, cylinder[0], cylinder[1], radius);

  CHECK_EQUAL(Inside, result);
}

//------------------------------------------------------------- Point-Cylinder 2
TEST(PointCylinder_2)
{
  //Point inside the two planes of the cylinder but not the infinite cylinder
  Vec3 cylinder[2] = { Vec3::cZero, Vec3(real(10.0), real(10.0), real(10.0)) };
  real radius = real(2.0);
  Vec3 point(real(8.0), real(1.0), real(4.0));
  Intersection::Type result = PointCylinder(point, cylinder[0], cylinder[1], radius);

  CHECK_EQUAL(None, result);
}

//------------------------------------------------------------- Point-Cylinder 3
TEST(PointCylinder_3)
{
  //Point inside the infinite cylinder but outside plane at first point
  Vec3 cylinder[2] = { Vec3::cZero, Vec3(real(10.0), real(10.0), real(10.0)) };
  real radius = real(2.0);
  Vec3 point(real(-1.0), real(-1.0), real(-1.0));
  Intersection::Type result = PointCylinder(point, cylinder[0], cylinder[1], radius);

  CHECK_EQUAL(None, result);
}

//------------------------------------------------------------- Point-Cylinder 4
TEST(PointCylinder_4)
{
  //Point inside the infinite cylinder but outside plane at second point
  Vec3 cylinder[2] = { Vec3::cZero, Vec3(real(10.0), real(10.0), real(10.0)) };
  real radius = real(2.0);
  Vec3 point(real(11.0), real(11.0), real(11.0));
  Intersection::Type result = PointCylinder(point, cylinder[0], cylinder[1], radius);

  CHECK_EQUAL(None, result);
}

//------------------------------------------------------------- Point-Cylinder 5
TEST(PointCylinder_5)
{
  //Point outside all elements of the cylinder
  Vec3 cylinder[2] = { Vec3::cZero, Vec3(real(10.0), real(10.0), real(10.0)) };
  real radius = real(2.0);
  Vec3 point(real(15.0), real(-15.0), real(80.0));
  Intersection::Type result = PointCylinder(point, cylinder[0], cylinder[1], radius);

  CHECK_EQUAL(None, result);
}

//------------------------------------------------------------- Point-Cylinder 6
TEST(PointCylinder_6)
{
  //Point on the endcap at the cylinder's first point
  Vec3 cylinder[2] = { Vec3::cZero, Vec3(real(10.0), real(10.0), real(10.0)) };
  real radius = real(2.0);
  Vec3 point = Vec3::cZero;
  Intersection::Type result = PointCylinder(point, cylinder[0], cylinder[1], radius);

  CHECK_EQUAL(Inside, result);
}

//------------------------------------------------------------- Point-Cylinder 7
TEST(PointCylinder_7)
{
  //Point on the endcap at the cylinder's second point
  Vec3 cylinder[2] = { Vec3::cZero, Vec3(real(10.0), real(10.0), real(10.0)) };
  real radius = real(2.0);
  Vec3 point(real(10.0), real(10.0), real(10.0));
  Intersection::Type result = PointCylinder(point, cylinder[0], cylinder[1], radius);

  CHECK_EQUAL(Inside, result);
}

//------------------------------------------------------------- Point-Cylinder 8
TEST(PointCylinder_8)
{
  //Point on the infinite cylinder between the cylinder's endcaps
  Vec3 cylinder[2] = { Vec3::cZero, Vec3(real(10.0), real(10.0), real(10.0)) };
  real radius = real(2.0);
  Vec3 point(real(4.1835034190723), 
             real(6.6329931618554), 
             real(4.1835034190723));
  Intersection::Type result = PointCylinder(point, cylinder[0], cylinder[1], radius);

  CHECK_EQUAL(Inside, result);
}

//------------------------------------------------------------ Point-Ellipsoid 1
TEST(PointEllipsoid_1)
{
  //Point trivially inside the ellipsoid
  Vec3 ellipsoid[2] = { Vec3::cZero, Vec3(real(1.0), real(2.0), real(3.0)) };
  Mat3 ellipsoidBasis;
  Vec3 axis = Normalized(Vec3(real(1.0), real(1.0), real(1.0)));
  real radians = Math::cPi / real(4.0);
  Math::ToMatrix3(axis, radians, &ellipsoidBasis);
  Vec3 point(real(0.5), real(0.5), real(0.5));

  Intersection::Type result = PointEllipsoid(point, ellipsoid[0], ellipsoid[1],
                               ellipsoidBasis);
  
  CHECK_EQUAL(Inside, result);
}

//------------------------------------------------------------ Point-Ellipsoid 2
TEST(PointEllipsoid_2)
{
  //Point outside the ellipsoid
  Vec3 ellipsoid[2] = { Vec3::cZero, Vec3(real(1.0), real(2.0), real(3.0)) };
  Mat3 ellipsoidBasis = Mat3::cIdentity;
  Vec3 point = real(5.0) * Vec3::cXAxis;
  Intersection::Type result = PointEllipsoid(point, ellipsoid[0], ellipsoid[1], 
                               ellipsoidBasis);

  CHECK_EQUAL(None, result);
}

//------------------------------------------------------------ Point-Ellipsoid 3
TEST(PointEllipsoid_3)
{
  //Point on the surface of the ellipsoid (along it's x-axis)
  Vec3 ellipsoid[2] = { Vec3::cZero, Vec3(real(1.0), real(2.0), real(8.0)) };
  Mat3 ellipsoidBasis;
  Vec3 axis = Normalized(Vec3(real(1.0), real(1.0), real(1.0)));
  real radians = Math::cPi / real(4.0);
  Math::ToMatrix3(axis, radians, &ellipsoidBasis);
  Vec3 point = ellipsoidBasis.BasisX();
  Intersection::Type result = PointEllipsoid(point, ellipsoid[0], ellipsoid[1], 
                               ellipsoidBasis);

  CHECK_EQUAL(Inside, result);
}

//------------------------------------------------------------ Point-Ellipsoid 4
TEST(PointEllipsoid_4)
{
  //Point on the surface of the ellipsoid (along it's y-axis)
  Vec3 ellipsoid[2] = { Vec3::cZero, Vec3(real(1.0), real(2.0), real(8.0)) };
  Mat3 ellipsoidBasis;
  Vec3 axis = Normalized(Vec3(real(1.0), real(1.0), real(1.0)));
  real radians = Math::cPi / real(4.0);
  Math::ToMatrix3(axis, radians, &ellipsoidBasis);
  Vec3 point = ellipsoidBasis.BasisY();
  Intersection::Type result = PointEllipsoid(point, ellipsoid[0], ellipsoid[1], 
                               ellipsoidBasis);

  CHECK_EQUAL(Inside, result);
}

//------------------------------------------------------------ Point-Ellipsoid 5
TEST(PointEllipsoid_5)
{
  //Point on the surface of the ellipsoid (along it's z-axis)
  Vec3 ellipsoid[2] = { Vec3::cZero, Vec3(real(1.0), real(2.0), real(8.0)) };
  Mat3 ellipsoidBasis;
  Vec3 axis = Normalized(Vec3(real(1.0), real(1.0), real(1.0)));
  real radians = Math::cPi / real(4.0);
  Math::ToMatrix3(axis, radians, &ellipsoidBasis);
  Vec3 point = ellipsoidBasis.BasisZ();
  Intersection::Type result = PointEllipsoid(point, ellipsoid[0], ellipsoid[1], 
                               ellipsoidBasis);

  CHECK_EQUAL(Inside, result);
}

//-------------------------------------------------------------- Point-Frustum 1
TEST(PointFrustum_1)
{
  //Point is trivially inside the frustum
  Vec4 frustum[6] = { Vec4(real( 0.0), real( 0.0), real(-1.0), real(0.0)),
                      Vec4(real( 0.0), real( 0.0), real( 1.0), real(0.0)),
                      Vec4(real( 1.5), real( 0.0), real(-0.5), real(0.0)),
                      Vec4(real(-1.5), real( 0.0), real(-0.5), real(0.0)),
                      Vec4(real( 0.0), real( 1.5), real(-0.5), real(0.0)),
                      Vec4(real( 0.0), real(-1.5), real(-0.5), real(0.0))  };
  //Set up the frustum
  {
    for(uint i = 0; i < 6; ++i)
    {
      Normalize(frustum[i]);
    };

    Vec3 planePoints[6] = { Vec3(real( 0.0), real( 0.0), real( 5.0)),
                            Vec3(real( 0.0), real( 0.0), real(-5.0)),
                            Vec3(real(-5.0), real( 0.0), real( 0.0)),
                            Vec3(real( 5.0), real( 0.0), real( 0.0)),
                            Vec3(real( 0.0), real(-5.0), real( 0.0)),
                            Vec3(real( 0.0), real( 5.0), real( 0.5)) };
    Vec3* plane = NULL;
    for(uint i = 0; i < 6; ++i)
    {
      plane = reinterpret_cast<Vec3*>(&(frustum[i].x));
      frustum[i][3] = Dot(*plane, planePoints[i]);
    };
  }

  Vec3 point = Vec3::cZero;
  Intersection::Type result = PointFrustum(point, frustum);

  CHECK_EQUAL(Inside, result);
}

//-------------------------------------------------------------- Point-Frustum 2
TEST(PointFrustum_2)
{
  //Point is trivially outside the frustum
  Vec4 frustum[6] = { Vec4(real( 0.0), real( 0.0), real(-1.0), real(0.0)),
                      Vec4(real( 0.0), real( 0.0), real( 1.0), real(0.0)),
                      Vec4(real( 1.5), real( 0.0), real(-0.5), real(0.0)),
                      Vec4(real(-1.5), real( 0.0), real(-0.5), real(0.0)),
                      Vec4(real( 0.0), real( 1.5), real(-0.5), real(0.0)),
                      Vec4(real( 0.0), real(-1.5), real(-0.5), real(0.0))  };
  //Set up the frustum
  {
    for(uint i = 0; i < 6; ++i)
    {
      Normalize(frustum[i]);
    };

    Vec3 planePoints[6] = { Vec3(real( 0.0), real( 0.0), real( 5.0)),
                            Vec3(real( 0.0), real( 0.0), real(-5.0)),
                            Vec3(real(-5.0), real( 0.0), real( 0.0)),
                            Vec3(real( 5.0), real( 0.0), real( 0.0)),
                            Vec3(real( 0.0), real(-5.0), real( 0.0)),
                            Vec3(real( 0.0), real( 5.0), real( 0.5)) };
    Vec3* plane = NULL;
    for(uint i = 0; i < 6; ++i)
    {
      plane = reinterpret_cast<Vec3*>(&(frustum[i].x));
      frustum[i][3] = Dot(*plane, planePoints[i]);
    };
  }

  Vec3 point = real(8.0) * Vec3::cZAxis;
  Intersection::Type result = PointFrustum(point, frustum);

  CHECK_EQUAL(None, result);
}

//-------------------------------------------------------------- Point-Frustum 3
TEST(PointFrustum_3)
{
  //Point lies on the surface of the frustum
  Vec4 frustum[6] = { Vec4(real( 0.0), real( 0.0), real(-1.0), real(0.0)),
                      Vec4(real( 0.0), real( 0.0), real( 1.0), real(0.0)),
                      Vec4(real( 1.5), real( 0.0), real(-0.5), real(0.0)),
                      Vec4(real(-1.5), real( 0.0), real(-0.5), real(0.0)),
                      Vec4(real( 0.0), real( 1.5), real(-0.5), real(0.0)),
                      Vec4(real( 0.0), real(-1.5), real(-0.5), real(0.0))  };
  //Set up the frustum
  {
    for(uint i = 0; i < 6; ++i)
    {
      Normalize(frustum[i]);
    };

    Vec3 planePoints[6] = { Vec3(real( 0.0), real( 0.0), real( 5.0)),
                            Vec3(real( 0.0), real( 0.0), real(-5.0)),
                            Vec3(real(-5.0), real( 0.0), real( 0.0)),
                            Vec3(real( 5.0), real( 0.0), real( 0.0)),
                            Vec3(real( 0.0), real(-5.0), real( 0.0)),
                            Vec3(real( 0.0), real( 5.0), real( 0.5)) };
    Vec3* plane = NULL;
    for(uint i = 0; i < 6; ++i)
    {
      plane = reinterpret_cast<Vec3*>(&(frustum[i].x));
      frustum[i][3] = Dot(*plane, planePoints[i]);
    };
  }

  Vec3 point = real(5.0) * Vec3::cZAxis;
  Intersection::Type result = PointFrustum(point, frustum);

  CHECK_EQUAL(Inside, result);
}

//------------------------------------------------------------------ Point-Obb 1
TEST(PointObb_1)
{
  //Point outside the oriented-bounding box
  Vec3 obb[2] = { Vec3::cZero, Vec3(real(0.5), real(9.2), real(20.0)) };
  Mat3 obbBasis = Mat3::cIdentity;

  Vec3 point(real(1.0), real(8.0), real(5.0));
  Intersection::Type result = PointObb(point, obb[0], obb[1], obbBasis);

  CHECK_EQUAL(None, result);
}

//------------------------------------------------------------------ Point-Obb 2
TEST(PointObb_2)
{
//   //Point outside the oriented-bounding box
//   Vec3 obb[2] = { Vec3::cZero, Vec3(real(0.5), real(9.2), real(20.0)) };
//   Vec3 axis(real(1.0), real(1.0), real(1.0));
//   Mat3 obbBasis = Math::AxisAngleToMatrix3();
// 
//   Vec3 point(real(1.0), real(8.0), real(5.0));
//   Intersection::Type result = PointObb(point, obb[0], obb[1], obbBasis);
// 
//   CHECK_EQUAL(None, result);
}

//---------------------------------------------------------------- Point-Plane 1
TEST(PointPlane_1)
{
  //Point lying outside the plane
  Vec3 normal = Vec3::cYAxis;
  real distance = real(5.0);
  Vec3 point(real(0.0), real(6.0), real(0.0));
  Intersection::Type result = PointPlane(point, normal, distance);

  CHECK_EQUAL(None, result);
}

//---------------------------------------------------------------- Point-Plane 2
TEST(PointPlane_2)
{
  //Point lying inside the plane
  Vec3 normal = Vec3::cYAxis;
  real distance = real(5.0);
  Vec3 point(real(0.0), real(4.0), real(0.0));
  Intersection::Type result = PointPlane(point, normal, distance);

  CHECK_EQUAL(Inside, result);
}

//---------------------------------------------------------------- Point-Plane 3
TEST(PointPlane_3)
{
  //Point lying outside the plane
  Vec3 normal = Vec3::cYAxis;
  real distance = real(5.0);
  Vec3 point(real(0.0), real(5.0), real(0.0));
  Intersection::Type result = PointPlane(point, normal, distance);

  CHECK_EQUAL(Inside, result);
}

//--------------------------------------------------------------- Point-Sphere 1
TEST(PointSphere_1)
{
  //Point lying outside the sphere
  Vec3 center = Vec3::cZero;
  real radius = real(1.0);
  Vec3 point = Vec3(real(1.0), real(1.0), real(1.0));
  Intersection::Type result = PointSphere(point, center, radius);

  CHECK_EQUAL(None, result);
}

//--------------------------------------------------------------- Point-Sphere 2
TEST(PointSphere_2)
{
  //Point lying outside the sphere
  Vec3 center = Vec3::cZero;
  real radius = real(1.0);
  Vec3 point = Vec3(real(0.5), real(0.5), real(0.5));
  Intersection::Type result = PointSphere(point, center, radius);

  CHECK_EQUAL(Inside, result);
}

//--------------------------------------------------------------- Point-Sphere 3
TEST(PointSphere_3)
{
  //Point lying on the surface of the sphere
  Vec3 center = Vec3::cZero;
  real radius = real(1.0);
  Vec3 point = Vec3(real(0.57735026918962576450914878050196),
                    real(0.57735026918962576450914878050196),
                    real(0.57735026918962576450914878050196));
  Intersection::Type result = PointSphere(point, center, radius);

  CHECK_EQUAL(Inside, result);
}

//------------------------------------------------------------- Point-Triangle 1
TEST(PointTriangle_1)
{
  //Point lying on a different plane than the triangle but still inside
  Vec3 triangle[3] = { Vec3::cXAxis, Vec3::cYAxis, Vec3::cZAxis };
  Vec3 point = Vec3::cZero;
  Intersection::Type result = PointTriangle(point, triangle[0], triangle[1], triangle[2]);

  CHECK_EQUAL(Inside, result);
}

//------------------------------------------------------------- Point-Triangle 2
TEST(PointTriangle_2)
{
  //Point lying on the edge of the triangle
  Vec3 triangle[3] = { Vec3::cXAxis, Vec3::cYAxis, Vec3::cZAxis };
  Vec3 point = real(0.5) * (Vec3::cXAxis + Vec3::cZAxis);
  Intersection::Type result = PointTriangle(point, triangle[0], triangle[1], triangle[2]);

  CHECK_EQUAL(Inside, result);
}

//------------------------------------------------------------- Point-Triangle 3
TEST(PointTriangle_3)
{
  //Point lying outside the triangle
  Vec3 triangle[3] = { Vec3::cXAxis, Vec3::cYAxis, Vec3::cZAxis };
  Vec3 point = Vec3(real(5.0), real(0.0), real(0.0));
  Intersection::Type result = PointTriangle(point, triangle[0], triangle[1], triangle[2]);

  CHECK_EQUAL(None, result);
}

//------------------------------------------------------------- Point-Triangle 3
TEST(PointTriangle_4)
{
  //Point lying outside the triangle
  Vec3 triangle[3] = { Vec3(real(-1.5), real(-1.0), real(0.0)), 
                       Vec3(real( 0.0), real( 1.0), real(0.0)), 
                       Vec3(real( 0.0), real( 1.5), real(0.0)) };
  Vec3 point = Vec3(real(0.0), real(0.0), real(0.0));
  Intersection::Type result = PointTriangle(point, triangle[0], triangle[1], triangle[2]);

  CHECK_EQUAL(None, result);
}
