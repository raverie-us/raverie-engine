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
#include "IntersectionTests/UnitTestCommon.hpp"

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
void RayCapsuleShared(Vec3Param rayStart, Vec3Param rayDir, Vec3Param pointA, Vec3Param pointB, real radius, bool expectedResult, Interval& exceptedInterval, CppUnitLite::TestResult& result_, const char* m_name)
{
  Interval interval;
  Intersection::Type resultType = RayCapsule(rayStart, rayDir, pointA, pointB, radius, &interval);
  bool result = resultType >= 0;
  CHECK_EQUAL(expectedResult, result);
  if(result)
  {
    CHECK_CLOSE(exceptedInterval.Min, interval.Min, epsilon[4]);
    CHECK_CLOSE(exceptedInterval.Max, interval.Max, epsilon[4]);
  }
}

TEST(RayCapsule_01)
{
  IntersectionPoint data;
  Vec3 ray[2] = {Vec3::cZero, Vec3::cYAxis};
  Vec3 capsule[2] = {Vec3(real(0.0), real(3.0), real(0.0)),
    Vec3(real(0.0), real(5.0), real(0.0))};
  real radius = real(0.5);

  //Capsule at (0,3,0)->(0,5,0) with a radius of 0.5 intersected by ray at 
  //origin pointing directly along the y-axis
  Intersection::Type result = RayCapsule(ray[0], ray[1], capsule[0], capsule[1], radius,
    &data);

  CHECK_EQUAL(Segment, result);

  Vec3 points[2] = {Vec3(real(0.0), real(2.5), real(0.0)),
    Vec3(real(0.0), real(5.5), real(0.0))};
  CHECK_VEC3_CLOSE(points[0], data.Points[0], zero);
  CHECK_VEC3_CLOSE(points[1], data.Points[1], zero);
}

//---------------------------------------------------------------- Ray-Capsule 2
TEST(RayCapsule_02)
{
  IntersectionPoint data;
  Vec3 ray[2] = {Vec3::cZero,
    Normalized(Vec3(real(1.0), real(0.0), real(1.0)))};
  Vec3 capsule[2] = {Vec3::cXAxis * real(5.0), Vec3::cZAxis * real(5.0)};
  real radius = real(0.5);

  Intersection::Type result = RayCapsule(ray[0], ray[1], capsule[0], capsule[1], radius,
    &data);
  CHECK_EQUAL(Segment, result);
}

//---------------------------------------------------------------- Ray-Capsule 3
TEST(RayCapsule_03)
{
  IntersectionPoint data;
  Vec3 ray[2] = {Vec3(real(0.8), real(8.0), real(0.0)),
    Normalized(Vec3(real(-0.7), real(-0.9), real(0.0)))};
  Vec3 capsule[2] = {Vec3(real(0.0), real(3.0), real(0.0)),
    Vec3(real(0.0), real(6.0), real(0.0))};
  real radius = real(1.0);

  Intersection::Type result = RayCapsule(ray[0], ray[1], capsule[0], capsule[1], radius,
    &data);
  CHECK_EQUAL(Segment, result);
}

//---------------------------------------------------------------- Ray-Capsule 4
TEST(RayCapsule_04)
{
  IntersectionPoint data;
  Vec3 ray[2] = {Vec3(real(2.4), real(1.2), real(0.0)),
    Normalized(Vec3(real(-1.0), real(1.0), real(0.0)))};
  Vec3 capsule[2] = {Vec3(real(0.0), real(3.0), real(0.0)),
    Vec3(real(0.0), real(6.0), real(0.0))};
  real radius = real(1.0);

  Intersection::Type result = RayCapsule(ray[0], ray[1], capsule[0], capsule[1], radius,
    &data);
  CHECK_EQUAL(Segment, result);
}

//---------------------------------------------------------------- Ray-Capsule 5
TEST(RayCapsule_05)
{
  IntersectionPoint data;
  Vec3 ray[2] = {Vec3(real(3.0), real(6.4), real(0.0)),
    Normalized(Vec3(real(-1.0), real(0.0), real(0.0)))};
  Vec3 capsule[2] = {Vec3(real(0.0), real(3.0), real(0.0)),
    Vec3(real(0.0), real(6.0), real(0.0))};
  real radius = real(1.0);

  Intersection::Type result = RayCapsule(ray[0], ray[1], capsule[0], capsule[1], radius,
    &data);
  CHECK_EQUAL(Segment, result);
}

//---------------------------------------------------------------- Ray-Capsule 6
TEST(RayCapsule_06)
{
  IntersectionPoint data;
  Vec3 ray[2] = {Vec3(real(-2.9), real(3.5), real(0.0)),
    Normalized(Vec3(real(1.4), real(2.0), real(0.0)))};
  Vec3 capsule[2] = {Vec3(real(0.0), real(3.0), real(0.0)),
    Vec3(real(0.0), real(6.0), real(0.0))};
  real radius = real(1.0);

  Intersection::Type result = RayCapsule(ray[0], ray[1], capsule[0], capsule[1], radius,
    &data);
  CHECK_EQUAL(Segment, result);
}

//---------------------------------------------------------------- Ray-Capsule 7
TEST(RayCapsule_07)
{
  IntersectionPoint data;
  Vec3 ray[2] = {Vec3(real(0.0), real(4.5), real(0.0)),
    Vec3(real(0.0), real(-1.0), real(0.0))};
  Vec3 capsule[2] = {Vec3(real(0.0), real(3.0), real(0.0)),
    Vec3(real(0.0), real(6.0), real(0.0))};
  real radius = real(1.0);

  Intersection::Type result = RayCapsule(ray[0], ray[1], capsule[0], capsule[1], radius,
    &data);
  CHECK_EQUAL(Point, result);
}

TEST(RayCapsule0)
{
  Vec3 rayStart(1, 1, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule1)
{
  Vec3 rayStart(1, 0.55, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.711059, 1.33955);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule2)
{
  Vec3 rayStart(1, 0, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.707107, 1.91421);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule3)
{
  Vec3 rayStart(1, -0.55, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.707107, 2.12132);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule4)
{
  Vec3 rayStart(1, -1, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.707107, 2.12132);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule5)
{
  Vec3 rayStart(1, -1.4, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.707107, 2.12132);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule6)
{
  Vec3 rayStart(1, -2, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.914213, 2.12132);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule7)
{
  Vec3 rayStart(1, -2.6, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(1.5739, 2.10305);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule8)
{
  Vec3 rayStart(1, 2.6, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(1.5739, 2.10305);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule9)
{
  Vec3 rayStart(1, 2, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.914213, 2.12132);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule10)
{
  Vec3 rayStart(1, 1.55, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.710334, 2.12132);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule11)
{
  Vec3 rayStart(1, 1.4, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.707107, 2.12132);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule12)
{
  Vec3 rayStart(1, 1, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.707107, 2.12132);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule13)
{
  Vec3 rayStart(1, 0.55, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.707107, 2.12132);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule14)
{
  Vec3 rayStart(1, 0, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.707107, 1.91421);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule15)
{
  Vec3 rayStart(1, -0.65, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.757744, 1.15144);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule16)
{
  Vec3 rayStart(1, 1.6, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule17)
{
  Vec3 rayStart(1, 1.1, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.510102, 1.4899);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule18)
{
  Vec3 rayStart(1, 0.75, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.5, 1.5);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule19)
{
  Vec3 rayStart(1, 0, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.5, 1.5);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule20)
{
  Vec3 rayStart(1, -0.75, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.5, 1.5);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule21)
{
  Vec3 rayStart(1, -1.1, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.510102, 1.4899);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule22)
{
  Vec3 rayStart(1, -1.6, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule23)
{
  Vec3 rayStart(0, 1.6, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule24)
{
  Vec3 rayStart(0, 1.1, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.489898, 0.489898);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule25)
{
  Vec3 rayStart(0, 0.75, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.5, 0.5);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule26)
{
  Vec3 rayStart(0, 0, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.5, 0.5);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule27)
{
  Vec3 rayStart(0, -0.75, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.5, 0.5);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule28)
{
  Vec3 rayStart(0, -1.1, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.489898, 0.489898);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule29)
{
  Vec3 rayStart(0, -1.6, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule30)
{
  Vec3 rayStart(-1, 1.6, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule31)
{
  Vec3 rayStart(-1, 1.1, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(-1.4899, -0.510102);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule32)
{
  Vec3 rayStart(-1, 0.75, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(-1.43301, -0.566987);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule33)
{
  Vec3 rayStart(-1, 0, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule34)
{
  Vec3 rayStart(-1, -0.75, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(-1.43301, -0.566987);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule35)
{
  Vec3 rayStart(-1, -1.1, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(-1.4899, -0.510102);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule36)
{
  Vec3 rayStart(-1, -1.6, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule37)
{
  Vec3 rayStart(1, -2, 0);
  Vec3 rayDir(0, 1, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule38)
{
  Vec3 rayStart(0, -2, 0);
  Vec3 rayDir(0, 1, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(0.5, 3.5);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule39)
{
  Vec3 rayStart(0, -1.25, 0);
  Vec3 rayDir(0, 1, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.25, 2.75);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule40)
{
  Vec3 rayStart(0, -0.75, 0);
  Vec3 rayDir(0, 1, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.75, 2.25);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule41)
{
  Vec3 rayStart(0, 0, 0);
  Vec3 rayDir(0, 1, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-1.5, 1.5);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule42)
{
  Vec3 rayStart(0, 0.75, 0);
  Vec3 rayDir(0, 1, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-2.25, 0.75);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule43)
{
  Vec3 rayStart(0, 1.25, 0);
  Vec3 rayDir(0, 1, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-2.75, 0.25);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule44)
{
  Vec3 rayStart(0, 2, 0);
  Vec3 rayDir(0, 1, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(-3.5, -0.5);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule45)
{
  Vec3 rayStart(0, 1.75, 0);
  Vec3 rayDir(0.970142, 0.242536, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule46)
{
  Vec3 rayStart(0, 1.25, 0);
  Vec3 rayDir(0.970142, 0.242536, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.497871, 0.376603);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule47)
{
  Vec3 rayStart(0, 0.75, 0);
  Vec3 rayDir(0.970142, 0.242536, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.515388, 0.515388);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule48)
{
  Vec3 rayStart(0, 0, 0);
  Vec3 rayDir(0.970142, 0.242536, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.515388, 0.515388);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule49)
{
  Vec3 rayStart(0, -0.75, 0);
  Vec3 rayDir(0.970142, 0.242536, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.515388, 0.515388);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule50)
{
  Vec3 rayStart(0, -1.25, 0);
  Vec3 rayDir(0.970142, 0.242536, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.376603, 0.497871);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule51)
{
  Vec3 rayStart(0, -1.75, 0);
  Vec3 rayDir(0.970142, 0.242536, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule52)
{
  Vec3 rayStart(0, 1.75, 0);
  Vec3 rayDir(0.970142, -0.242536, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule53)
{
  Vec3 rayStart(0, 1.25, 0);
  Vec3 rayDir(0.970142, -0.242536, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.376603, 0.497871);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule54)
{
  Vec3 rayStart(0, 0.75, 0);
  Vec3 rayDir(0.970142, -0.242536, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.515388, 0.515388);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule55)
{
  Vec3 rayStart(0, 0, 0);
  Vec3 rayDir(0.970142, -0.242536, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.515388, 0.515388);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule56)
{
  Vec3 rayStart(0, -0.75, 0);
  Vec3 rayDir(0.970142, -0.242536, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.515388, 0.515388);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule57)
{
  Vec3 rayStart(0, -1.25, 0);
  Vec3 rayDir(0.970142, -0.242536, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = true;
  Interval expectedInterval(-0.497871, 0.376603);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCapsule58)
{
  Vec3 rayStart(0, -1.75, 0);
  Vec3 rayDir(0.970142, -0.242536, 0);
  Vec3 pointA(0, -1, 0);
  Vec3 pointB(0, 1, 0);
  real radius = 0.5;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCapsuleShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

//----------------------------------------------------------------- Ray-Cylinder
void RayCylinderShared(Vec3Param rayStart, Vec3Param rayDir, Vec3Param pointA, Vec3Param pointB, real radius, bool expectedResult, Interval& exceptedInterval, CppUnitLite::TestResult& result_, const char* m_name)
{
  Interval interval;
  Intersection::Type resultType = RayCylinder(rayStart, rayDir, pointA, pointB, radius, &interval);
  bool result = resultType >= 0;
  CHECK_EQUAL(expectedResult, result);
  if(result)
  {
    CHECK_CLOSE(exceptedInterval.Min, interval.Min, epsilon[4]);
    CHECK_CLOSE(exceptedInterval.Max, interval.Max, epsilon[4]);
  }
}

TEST(RayCylinder0)
{
  Vec3 rayStart(0, 0, -2);
  Vec3 rayDir(0, 0, 1);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(1, 3);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder1)
{
  Vec3 rayStart(-2, 0, 0);
  Vec3 rayDir(1, 0, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(1, 3);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder2)
{
  Vec3 rayStart(0, 0, 2);
  Vec3 rayDir(0, 0, -1);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(1, 3);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder3)
{
  Vec3 rayStart(2, 0, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(1, 3);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder4)
{
  Vec3 rayStart(2, 3.6, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder5)
{
  Vec3 rayStart(2, 3, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder6)
{
  Vec3 rayStart(2, 2, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder7)
{
  Vec3 rayStart(2, 1, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder8)
{
  Vec3 rayStart(2, 0, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder9)
{
  Vec3 rayStart(2, -1, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(1.41421, 2.12132);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder10)
{
  Vec3 rayStart(2, -2, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(2.12132, 3.53553);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder11)
{
  Vec3 rayStart(2, -3, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(3.53553, 4.24264);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder12)
{
  Vec3 rayStart(2, -3.6, 0);
  Vec3 rayDir(-0.707107, 0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder13)
{
  Vec3 rayStart(2, 3.6, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder14)
{
  Vec3 rayStart(2, 3, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(3.53553, 4.24264);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder15)
{
  Vec3 rayStart(2, 2, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(2.12132, 3.53553);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder16)
{
  Vec3 rayStart(2, 1, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(1.41421, 2.12132);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder17)
{
  Vec3 rayStart(2, 0, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder18)
{
  Vec3 rayStart(2, -1, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder19)
{
  Vec3 rayStart(2, -2, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder20)
{
  Vec3 rayStart(2, -3, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder21)
{
  Vec3 rayStart(2, -3.6, 0);
  Vec3 rayDir(-0.707107, -0.707107, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder22)
{
  Vec3 rayStart(2, 0.55, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder23)
{
  Vec3 rayStart(0, 0.55, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder24)
{
  Vec3 rayStart(-2, 0.55, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder25)
{
  Vec3 rayStart(2, -0.55, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder26)
{
  Vec3 rayStart(0, -0.55, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder27)
{
  Vec3 rayStart(-2, -0.55, 0);
  Vec3 rayDir(-1, 0, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder28)
{
  Vec3 rayStart(0, -1, 0);
  Vec3 rayDir(0, 1, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(0.5, 1.5);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder29)
{
  Vec3 rayStart(0, 0, 0);
  Vec3 rayDir(0, 1, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(-0.5, 0.5);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder30)
{
  Vec3 rayStart(0, 1, 0);
  Vec3 rayDir(0, 1, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder31)
{
  Vec3 rayStart(0, -1, 0);
  Vec3 rayDir(0, -1, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder32)
{
  Vec3 rayStart(0, 0, 0);
  Vec3 rayDir(0, -1, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(-0.5, 0.5);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder33)
{
  Vec3 rayStart(0, 1, 0);
  Vec3 rayDir(0, -1, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(0.5, 1.5);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder34)
{
  Vec3 rayStart(0, 1.1, 0);
  Vec3 rayDir(-0.894427, 0.447214, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder35)
{
  Vec3 rayStart(0, 0.25, 0);
  Vec3 rayDir(-0.894427, 0.447214, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(-1.11803, 0.559017);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder36)
{
  Vec3 rayStart(0, -0.25, 0);
  Vec3 rayDir(-0.894427, 0.447214, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(-0.559017, 1.11803);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder37)
{
  Vec3 rayStart(0, -0.75, 0);
  Vec3 rayDir(-0.894427, 0.447214, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(0.559017, 1.11803);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder38)
{
  Vec3 rayStart(0, -1.1, 0);
  Vec3 rayDir(-0.894427, 0.447214, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder39)
{
  Vec3 rayStart(0, 1.1, 0);
  Vec3 rayDir(-0.894427, -0.447214, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder40)
{
  Vec3 rayStart(0, 0.25, 0);
  Vec3 rayDir(-0.894427, -0.447214, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(-0.559017, 1.11803);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder41)
{
  Vec3 rayStart(0, -0.25, 0);
  Vec3 rayDir(-0.894427, -0.447214, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(-1.11803, 0.559017);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder42)
{
  Vec3 rayStart(0, -0.75, 0);
  Vec3 rayDir(-0.894427, -0.447214, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder43)
{
  Vec3 rayStart(0, -1.1, 0);
  Vec3 rayDir(-0.894427, -0.447214, 0);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder44)
{
  Vec3 rayStart(2, 0, 0);
  Vec3 rayDir(-0.995037, 0.0995037, -1.18617e-07);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(1.00499, 3.01496);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder45)
{
  Vec3 rayStart(0, 0, 0);
  Vec3 rayDir(-0.995037, 0.0995037, -1.18617e-07);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = true;
  Interval expectedInterval(-1.00499, 1.00499);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
}

TEST(RayCylinder46)
{
  Vec3 rayStart(-2, 0, 0);
  Vec3 rayDir(-0.995037, 0.0995037, -1.18617e-07);
  Vec3 pointA(0, -0.5, 0);
  Vec3 pointB(0, 0.5, 0);
  real radius = 1;
  bool expectedResult = false;
  Interval expectedInterval(0, 0);
  RayCylinderShared(rayStart, rayDir, pointA, pointB, radius, expectedResult, expectedInterval, result_, m_name);
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
