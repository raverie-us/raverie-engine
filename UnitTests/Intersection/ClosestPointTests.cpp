///////////////////////////////////////////////////////////////////////////////
///
///  \file ClosestPointTests.cpp
///  Unit tests for the ClosestPointOn______ToPoint functions.
///	
///  Authors: Benjamin Strukus
///  Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
#include "Intersection/UnitTestCommon.hpp"

using namespace Intersection;

//------------------------------------------------------------------------ Ray 1
TEST(ClosestPointOnRayToPoint_1)
{
  //Test outside, closest point is the ray's start
  Vec3 ray[2] = { Vec3::cZero, Vec3(real(1.0), real(1.0), real(1.0)) };
  Normalize(ray[1]);
  Vec3 point(real(-1.0), real(-1.0), real(-1.0));
  Intersection::Type result = ClosestPointOnRayToPoint(ray[0], ray[1], &point);
  
  CHECK_EQUAL(Outside, result);
  CHECK_VEC3_CLOSE(ray[0], point, epsilon[5]);
}

//------------------------------------------------------------------------ Ray 2
TEST(ClosestPointOnRayToPoint_2)
{
  //Test inside, closest point is the point
  Vec3 ray[2] = { Vec3::cZero, Vec3(real(1.0), real(1.0), real(1.0)) };
  Vec3 point(real(1.0), real(1.0), real(1.0));
  Vec3 oldPoint = point;

  Normalize(ray[1]);
  
  Intersection::Type result = ClosestPointOnRayToPoint(ray[0], ray[1], &point);
  
  CHECK_EQUAL(Inside, result);
  CHECK_VEC3_CLOSE(oldPoint, point, epsilon[5]);
}

//------------------------------------------------------------------------ Ray 3
TEST(ClosestPointOnRayToPoint_3)
{
  //Test inside, closest point is not the point
  Vec3 ray[2] = { Vec3::cZero, Vec3::cXAxis };
  Vec3 point(real(5.0), real(8.0), real(-86.0));
  Intersection::Type result = ClosestPointOnRayToPoint(ray[0], ray[1], &point);
  
  Vec3 expected(real(5.0), real(0.0), real(0.0));

  CHECK_EQUAL(Inside, result);
  CHECK_VEC3_CLOSE(expected, point, epsilon[last]);
}

//-------------------------------------------------------------------- Segment 1
TEST(ClosestPointOnSegmentToPoint_1)
{
  //Test outside, segment's start is the closest point
  Vec3 segment[2] = { Vec3(real(5.0), real(-2.0), real(20.0)), 
                      Vec3(real(10.0), real(8.0), real(10.0)) };
  Vec3 point(real(1.0), real(-5.0), real(30.0));
  Intersection::Type result = ClosestPointOnSegmentToPoint(segment[0], segment[1], &point);

  CHECK_EQUAL(Outside, result);
  CHECK_VEC3_CLOSE(segment[0], point, epsilon[last]);
}

//-------------------------------------------------------------------- Segment 2
TEST(ClosestPointOnSegmentToPoint_2)
{
  //Test outside, segment's end is the closest point
  Vec3 segment[2] = { Vec3(real(5.0), real(-2.0), real(20.0)),
                      Vec3(real(10.0), real(8.0), real(10.0)) };
  Vec3 point(real(11.0), real(10.0), real(5.0));
  Intersection::Type result = ClosestPointOnSegmentToPoint(segment[0], segment[1], &point);

  CHECK_EQUAL(Outside, result);
  CHECK_VEC3_CLOSE(segment[1], point, epsilon[last]);
}

//-------------------------------------------------------------------- Segment 3
TEST(ClosestPointOnSegmentToPoint_3)
{
  //Test inside, on the segment
  Vec3 segment[2] = { Vec3(real(-1.0), real(-1.0), real(-1.0)),
                      Vec3(real(1.0),  real(1.0),  real(1.0))  };
  Vec3 point(real(0.0), real(0.0), real(0.0));
  Intersection::Type result = ClosestPointOnSegmentToPoint(segment[0], segment[1], &point);

  CHECK_EQUAL(Inside, result);
  CHECK_VEC3_CLOSE(Vec3::cZero, point, zero);
}

//-------------------------------------------------------------------- Segment 4
TEST(ClosestPointOnSegmentToPoint_4)
{
  //Test outside, closest point is between the segment's endpoints
  Vec3 segment[2] = { Vec3(real(20.0), real(10.0), real(8.0)),
                      Vec3(real(12.0), real(15.0), real(-1.0)) };
  Vec3 point(real(16.0), real(11.0), real(4.0));
  Intersection::Type result = ClosestPointOnSegmentToPoint(segment[0], segment[1], &point);

  //Computed by hand!
  Vec3 closest(real(16.564705882352941176470588235294),
               real(12.147058823529411764705882352941),
               real(4.1352941176470588235294117647058));

  CHECK_EQUAL(Inside, result);
  CHECK_VEC3_CLOSE(closest, point, epsilon[4]);
}

//------------------------------------------------------------------------- Aabb
TEST(ClosestPointOnAabbToPoint)
{
  MARK_AS_UNIMPLEMENTED();
}

//---------------------------------------------------------------------- Capsule
TEST(ClosestPointOnCapsuleToPoint)
{
  MARK_AS_UNIMPLEMENTED();
}

//----------------------------------------------------------------------- On Obb
TEST(ClosestPointOnObbToPoint)
{
  MARK_AS_UNIMPLEMENTED();
}

//----------------------------------------------------------------------- In Obb
TEST(ClosestPointInObbToPoint)
{
  MARK_AS_UNIMPLEMENTED();
}

//------------------------------------------------------------------------ Plane
TEST(ClosestPointOnPlaneToPoint)
{
  MARK_AS_UNIMPLEMENTED();
}

//----------------------------------------------------------------------- Sphere
TEST(ClosestPointOnSphereToPoint)
{
  MARK_AS_UNIMPLEMENTED();
}

//--------------------------------------------------------------------- Triangle
TEST(ClosestPointOnTriangleToPoint)
{
  MARK_AS_UNIMPLEMENTED();
}

//-------------------------------------------------------------------- Two Lines
TEST(ClosestPointsOfTwoLines)
{
  MARK_AS_UNIMPLEMENTED();
}

//----------------------------------------------------------------- Two Segments
TEST(ClosestPointsOfTwoSegments)
{
  MARK_AS_UNIMPLEMENTED();
}
