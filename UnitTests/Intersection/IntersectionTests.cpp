///////////////////////////////////////////////////////////////////////////////
///
/// \file IntersectionTests.cpp
/// Unit tests for the intersection library.
///
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
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

void RotateXZY(real x, real y, real z, Mat3Ref mtx)
{
  xRotate.Rotate(Vec3::cXAxis, Math::DegToRad(x));
  yRotate.Rotate(Vec3::cYAxis, Math::DegToRad(y));
  zRotate.Rotate(Vec3::cZAxis, Math::DegToRad(z));
  mtx = yRotate;
  mtx = Math::Multiply(mtx, zRotate);
  mtx = Math::Multiply(mtx, xRotate);
}

} //namespace

//-------------------------------------------------------------------- Aabb-Aabb
TEST(AabbAabb)
{
  MARK_AS_UNIMPLEMENTED();
}

//----------------------------------------------------------------- Aabb-Capsule
TEST(AabbCapsule)
{
  MARK_AS_UNIMPLEMENTED();
}

//----------------------------------------------------------------- Aabb-Frustum
TEST(AabbFrustum)
{
  Vec4 frustumPlanes[6] = { Vec4(real( 0.0), real( 0.0), real(-1.0), real(0.0)),
                            Vec4(real( 0.0), real( 0.0), real( 1.0), real(0.0)),
                            Vec4(real( 1.5), real( 0.0), real(-0.5), real(0.0)),
                            Vec4(real(-1.5), real( 0.0), real(-0.5), real(0.0)),
                            Vec4(real( 0.0), real( 1.5), real(-0.5), real(0.0)),
                            Vec4(real( 0.0), real(-1.5), real(-0.5), real(0.0)) 
                          };
  for(uint i = 0; i < 6; ++i)
  {
    Normalize(frustumPlanes[i]);
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
    plane = reinterpret_cast<Vec3*>(&(frustumPlanes[i].x));
    frustumPlanes[i][3] = Dot(*plane, planePoints[i]);
  };

  Vec3 aabbMin(real(-1.0), real(-1.0), real(-1.0));
  Vec3 aabbMax(real( 1.0), real( 1.0), real( 1.0));

  Intersection::Type result = AabbFrustumApproximation(aabbMin, aabbMax, frustumPlanes);
  CHECK_EQUAL(Inside, result);
}

//--------------------------------------------------------------------- Aabb-Obb
TEST(AabbObb)
{
  MARK_AS_UNIMPLEMENTED();
}

//------------------------------------------------------------------- Aabb-Plane
TEST(AabbPlane)
{
  MARK_AS_UNIMPLEMENTED();
}

//------------------------------------------------------------------ Aabb-Sphere
TEST(AabbSphere)
{
  MARK_AS_UNIMPLEMENTED();
}

//---------------------------------------------------------------- Aabb-Triangle
TEST(AabbTriangle)
{
  MARK_AS_UNIMPLEMENTED();
}

//-------------------------------------------------------------- Capsule-Capsule
TEST(CapsuleCapsule)
{
  MARK_AS_UNIMPLEMENTED();
}

//-------------------------------------------------------------- Capsule-Frustum
TEST(CapsuleFrustum)
{
  MARK_AS_UNIMPLEMENTED();
}

//------------------------------------------------------------------ Capsule-Obb
TEST(CapsuleObb)
{
  MARK_AS_UNIMPLEMENTED();
}

//---------------------------------------------------------------- Capsule-Plane
TEST(CapsulePlane)
{
  MARK_AS_UNIMPLEMENTED();
}

//--------------------------------------------------------------- Capsule-Sphere
TEST(CapsuleSphere)
{
  MARK_AS_UNIMPLEMENTED();
}

//------------------------------------------------------------- Capsule-Triangle
TEST(CapsuleTriangle)
{
  MARK_AS_UNIMPLEMENTED();
}

//--------------------------------------------------------------- Frustum-Sphere
TEST(FrustumSphere)
{
  MARK_AS_UNIMPLEMENTED();
}

//------------------------------------------------------------- Frustum-Triangle
TEST(FrustumTriangle)
{
  MARK_AS_UNIMPLEMENTED();
}

//------------------------------------------------------------------ Frustum-Obb
TEST(FrustumObb)
{
  MARK_AS_UNIMPLEMENTED();
}

//---------------------------------------------------------------- Frustum-Plane
TEST(FrustumPlane)
{
  MARK_AS_UNIMPLEMENTED();
}

//---------------------------------------------------------------------- Obb-Obb
TEST(ObbObb)
{
  MARK_AS_UNIMPLEMENTED();
//   const Vec3 oneScale(real(0.5), real(0.5), real(0.5));
//   const Vec3 twoScale(real(1.0), real(1.0), real(1.0));
//   const Vec3 threeScale(real(1.5), real(1.5), real(1.5));
// 
//   //------------------------------------------------------------------ Edge-Edge
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, 0)
//   Vec3 obbOneCenter(real(0.0), real(0.0), real(0.0));
//   Vec3 obbOneHalfExtents(twoScale);
//   Mat3 obbOneBasis; obbOneBasis.SetIdentity();
// 
//   // B = P(0, 2.485, -1), S(2, 2, 2), R(-27.040, 62.516, -76.366)
//   Vec3 obbTwoCenter(real(0.0), real(2.485), real(-1.0));
//   Vec3 obbTwoHalfExtents(twoScale);
//   Mat3 obbTwoBasis;
//   RotateXYZ(real(-27.040), real(62.516), real(-76.366), obbTwoBasis);
// 
//   Manifold data;
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis,
//                        obbTwoCenter, obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(EdgeEdge, result);
// 
// 
//   //------------------------------------------------------------------ Edge-Face
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, -45)
//   obbOneCenter.Set(real(0.0), real(0.0), real(0.0));
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(-45.0), obbOneBasis);
// 
//   // B = P(0, 2.25, 0), S(2, 2, 2), R(0, 0, 0)
//   obbTwoCenter.Set(real(0.0), real(2.25), real(0.0));
//   obbTwoHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(EdgeFace, result);
// 
// 
//   //---------------------------------------------------------------- Edge-Face 2
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, -45)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(-45.0), obbOneBasis);
// 
//   // B = P(0, 2.25, 0), S(2, 2, 2), R(0, 45, 0)
//   obbTwoCenter.Set(real(0.0), real(2.25), real(0.0));
//   obbTwoHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(45.0), real(0.0), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(EdgeFace, result);
// 
// 
//   //---------------------------------------------------------------- Edge-Face 3
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, -45)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(-45.0), obbOneBasis);
// 
//   // B = P(0, 1.82, 0), S(1, 1, 1), R(0, 67.645, 0)
//   obbTwoCenter.Set(real(0.0), real(1.82), real(0.0));
//   obbTwoHalfExtents = oneScale;
//   RotateXYZ(real(0.0), real(67.645), real(0.0), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(EdgeFace, result);
// 
// 
//   //---------------------------------------------------------------- Edge-Face 4
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, -45)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(-45.0), obbOneBasis);
// 
//   // B = P(0, 2.241, 0.165), S(2, 2, 2), R(0, 67.645, 0)
//   obbTwoCenter.Set(real(0.0), real(2.241), real(0.165));
//   obbTwoHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(67.645), real(0.0), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(EdgeFace, result);
// 
// 
//   //------------------------------------------------------------------ Face-Edge
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, 0)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbOneBasis);
// 
//   // B = P(0, 2, 0), S(2, 2, 2), R(0, 0, 45)
//   obbTwoCenter.Set(real(0.0), real(2.0), real(0.0));
//   obbTwoHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(45.0), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(FaceEdge, result);
// 
// 
//   //---------------------------------------------------------------- Face-Edge 2
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, 0)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbOneBasis);
// 
//   // B = P(0, 2, -1), S(2, 2, 2), R(0, 0, 45)
//   obbTwoCenter.Set(real(0.0), real(2.0), real(-1.0));
//   obbTwoHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(45.0), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(FaceEdge, result);
// 
// 
//   //---------------------------------------------------------------- Face-Edge 3
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, 0)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbOneBasis);
// 
//   // B = P(0.508, 2.343, -0.654), S(2, 2, 2), R(49.358, 38.928, 61.430)
//   obbTwoCenter.Set(real(0.508), real(2.343), real(-0.654));
//   obbTwoHalfExtents = twoScale;
//   RotateXYZ(real(49.358), real(38.928), real(61.430), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(FaceEdge, result);
// 
//   //-------------------------------------------------------------- Face-Edge 3.5
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, 0)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = oneScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbOneBasis);
// 
//   // B = P(0.508, 2.343, -0.654), S(2, 2, 2), R(49.358, 38.928, 61.430)
//   obbTwoCenter.Set(real(0.254), real(1.172), real(-0.327));
//   obbTwoHalfExtents = oneScale;
//   RotateXYZ(real(49.358), real(38.928), real(61.430), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(FaceEdge, result);
// 
//   //---------------------------------------------------------------- Face-Edge 4
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, 0)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbOneBasis);
// 
//   // B = P(-0.169, 1.606, 0.104), S(1, 1, 1), R(49.358, 38.928, 61.430)
//   obbTwoCenter.Set(real(-0.169), real(1.606), real(0.104));
//   obbTwoHalfExtents = oneScale;
//   RotateXYZ(real(49.358), real(38.928), real(61.430), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(FaceEdge, result);
// 
// 
//   //---------------------------------------------------------------- Face-Edge 5
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, 0)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbOneBasis);
// 
//   // B = P(-0.995, 2.343, -0.654),  S(2, 2, 2), R(49.358, 38.928, 61.430)
//   obbTwoCenter.Set(real(-0.995), real(2.343), real(-0.654));
//   obbTwoHalfExtents = twoScale;
//   RotateXYZ(real(49.358), real(38.928), real(61.430), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(FaceEdge, result);
// 
// 
//   //---------------------------------------------------------------- Face-Edge 6
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, 0)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbOneBasis);
// 
//   // B = P(0, 2.867, 0.374), S(3, 3, 3), R(45, 0, 0)
//   obbTwoCenter.Set(real(0.0), real(2.867), real(0.374));
//   obbTwoHalfExtents = threeScale;
//   RotateXYZ(real(45.0), real(0.0), real(0.0), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(FaceEdge, result);
// 
// 
//   //------------------------------------------------------------------ Face-Face
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, 0)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbOneBasis);
// 
//   // B = P(0, 1.837, 1.414), S(2, 2, 2), R(0, 0, 0)
//   obbTwoCenter.Set(real(0.0), real(1.837), real(1.414));
//   obbTwoHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(FaceFace, result);
// 
// 
//   //---------------------------------------------------------------- Face-Face 2
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, 0)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbOneBasis);
// 
//   // B = P(1.182, 1.837, 1.414), S(2, 2, 2), R(0, 0, 0)
//   obbTwoCenter.Set(real(1.182), real(1.837), real(1.414));
//   obbTwoHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(FaceFace, result);
// 
// 
//   //---------------------------------------------------------------- Face-Face 3
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, 0)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbOneBasis);
// 
//   // B = P(0.717, 1.361, 0.654), S(1, 1, 1), R(0, -30.264, 0)
//   obbTwoCenter.Set(real(0.717), real(1.361), real(0.654));
//   obbTwoHalfExtents = oneScale;
//   RotateXYZ(real(0.0), real(-30.264), real(0.0), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(FaceFace, result);
// 
// 
//   //---------------------------------------------------------------- Face-Face 4
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, 0)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbOneBasis);
// 
//   // B = P(0.5, 1.361, 0.5), S(1, 1, 1), R(0, -45, 0)
//   obbTwoCenter.Set(real(0.5), real(1.361), real(0.5));
//   obbTwoHalfExtents = oneScale;
//   RotateXYZ(real(0.0), real(-45.0), real(0.0), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(FaceFace, result);
// 
// 
//   //---------------------------------------------------------------- Face-Face 5
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, 0)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbOneBasis);
// 
//   // B = P(0, 1.361, 0), S(2, 2, 2), R(0, -45, 0)
//   obbTwoCenter.Set(real(0.0), real(1.361), real(0.0));
//   obbTwoHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(-45.0), real(0.0), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(FaceFace, result);
// 
//   //----------------------------------------------------------------- Face-Point
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, 0)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbOneBasis);
// 
//   // B = P(0, 2.401, -1), S(2, 2, 2), R(45, 45, 45)
//   obbTwoCenter.Set(real(0.0), real(2.401), real(-1.0));
//   obbTwoHalfExtents = twoScale;
//   RotateXYZ(real(45.0), real(45.0), real(45.0), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(FacePoint, result);
// 
// 
//   //--------------------------------------------------------------- Face-Point 2
//   // A = P(0, 0, 0), S(2, 2, 2), R(0, 0, 0)
//   obbOneCenter.Set(0.0, 0.0, 0.0);
//   obbOneHalfExtents = twoScale;
//   RotateXYZ(real(0.0), real(0.0), real(0.0), obbOneBasis);
// 
//   // B = P(0.102, 2.490, 0), S(2, 2, 2), R(-2.414, -36.844, -53.287)
//   obbTwoCenter.Set(real(0.102), real(2.490), real(0.0));
//   obbTwoHalfExtents = twoScale;
//   RotateXYZ(real(-2.414), real(-36.844), real(-53.287), obbTwoBasis);
// 
//   Intersection::Type result = ObbObb(obbOneCenter, obbOneHalfExtents, obbOneBasis, obbTwoCenter,
//                   obbTwoHalfExtents, obbTwoBasis, &data);
//   CHECK_EQUAL(FacePoint, result);
}

//-------------------------------------------------------------------- Obb-Plane
TEST(ObbPlane)
{
  MARK_AS_UNIMPLEMENTED();
}

//----------------------------------------------------------------- Obb-Sphere 1
TEST(ObbSphere_1)
{
  // Obb = P(0, 0, 0), S(2, 2, 2), R(0, 0, 0)
  Vec3 obbCenter(real(0.0), real(0.0), real(0.0));
  Vec3 obbHalfExtents(real(1.0), real(1.0), real(1.0));
  Mat3 obbBasis;
  RotateXYZ(real(0.0), real(0.0), real(0.0), obbBasis);

  // Sphere = P(0, 1.945, 0), S(1, 1, 1), R(0, 0, 0)
  Vec3 sphereCenter(real(0.0), real(1.945), real(0.0));
  real sphereRadius = real(1.0);

  Manifold data;
  Intersection::Type result = ObbSphere(obbCenter, obbHalfExtents, obbBasis, sphereCenter, 
                          sphereRadius, &data);
  CHECK_EQUAL(Point, result);
}

TEST(ObbSphere_2)
{
  //--------------------------------------------------------------- Obb-Sphere 2
  // Obb = P(0, 0, 0), S(2, 2, 2), R(-5.624, 0, 0)
  Vec3 obbCenter(real(0.0), real(0.0), real(0.0));
  Vec3 obbHalfExtents(real(1.0), real(1.0), real(1.0));
  Mat3 obbBasis;
  RotateXYZ(real(-5.624), real(0.0), real(0.0), obbBasis);

  // Sphere = P(-1.112, -0.172, -1.617), S(1, 1, 1), R(0, 0, 0)
  Vec3 sphereCenter(real(-1.112), real(-0.172), real(-1.617));
  real sphereRadius = real(1.0);

  Manifold data;
  Intersection::Type result = ObbSphere(obbCenter, obbHalfExtents, obbBasis, sphereCenter, 
                     sphereRadius, &data);
  CHECK_EQUAL(Point, result);
}

TEST(ObbSphere_3)
{
  //--------------------------------------------------------------- Obb-Sphere 3
  // Obb = P(0, 0, 0), S(3, 5, 1), R(-6.265, 33.444, 18.32)
  Vec3 obbCenter(real(0.0), real(0.0), real(0.0));
  Vec3 obbHalfExtents(real(1.5), real(2.5), real(0.5));
  Mat3 obbBasis;
  RotateXYZ(real(-6.265), real(33.444), real(18.32), obbBasis);

  // Sphere = P(-1.112, 2.842, -1.617), S(1, 1, 1), R(0, 0, 0)
  Vec3 sphereCenter(real(-1.112), real(2.842), real(-1.617));
  real sphereRadius = real(1.0);

  Manifold data;
  Intersection::Type result = ObbSphere(obbCenter, obbHalfExtents, obbBasis, sphereCenter, 
                          sphereRadius, &data);
  CHECK_EQUAL(Point, result);
}

//----------------------------------------------------------------- Obb-Sphere 4
TEST(ObbSphere_4)
{
  // Obb = P(0, 0, 0), S(3, 3, 3), R(0, 0, 69.984)
  Vec3 obbCenter(real(0.0), real(0.0), real(0.0));
  Vec3 obbHalfExtents(real(1.5), real(1.5), real(1.5));
  Mat3 obbBasis;
  RotateXYZ(real(0.0), real(0.0), real(68.984), obbBasis);

  // Sphere = P(-1.112, 2.842, -1.617), S(1, 1, 1), R(0, 0, 0)
  Vec3 sphereCenter(real(-1.112), real(2.842), real(-1.617));
  real sphereRadius = real(1.0);

  Manifold data;
  Intersection::Type result = ObbSphere(obbCenter, obbHalfExtents, obbBasis, sphereCenter, 
                          sphereRadius, &data);
  CHECK_EQUAL(Point, result);
}

//----------------------------------------------------------------- Obb-Sphere 5
TEST(ObbSphere_5)
{
  // Obb = P(0, 0, 0), S(3, 3, 3), R(0, 0, 69.984)
  Vec3 obbCenter(real(0.0), real(0.0), real(0.0));
  Vec3 obbHalfExtents(real(1.5), real(1.5), real(1.5));
  Mat3 obbBasis;
  RotateXYZ(real(0.0), real(0.0), real(68.984), obbBasis);

  // Sphere = P(0.472, 0.239, -0.432), S(1, 1, 1), R(0, 0, 0)
  Vec3 sphereCenter(real(0.472), real(0.239), real(-0.432));
  real sphereRadius = real(1.0);

  Manifold data;
  Intersection::Type result = ObbSphere(obbCenter, obbHalfExtents, obbBasis, sphereCenter, 
                          sphereRadius, &data);
  CHECK_EQUAL(Other, result);
}

//----------------------------------------------------------------- Obb-Sphere 6
TEST(ObbSphere_6)
{
  // Obb = P(-0.192, 3.155, -1.231), S(1, 1, 1), R(0, 0, 69.984)
  Vec3 obbCenter(real(-0.192), real(3.155), real(-1.231));
  Vec3 obbHalfExtents(real(0.5), real(0.5), real(0.5));
  Mat3 obbBasis;
  RotateXYZ(real(0.0), real(0.0), real(68.984), obbBasis);

  // Sphere = P(-1.112, 2.842, -1.617), S(2, 2, 2), R(0, 0, 0)
  Vec3 sphereCenter(real(-1.112), real(2.842), real(-1.617));
  real sphereRadius = real(2.0);

  Manifold data;
  Intersection::Type result = ObbSphere(obbCenter, obbHalfExtents, obbBasis, sphereCenter, 
                          sphereRadius, &data);
  CHECK_EQUAL(Point, result);
}

//----------------------------------------------------------------- Obb-Triangle
TEST(ObbTriangle)
{
  MARK_AS_UNIMPLEMENTED();
}

//------------------------------------------------------------------ Plane-Plane
TEST(PlanePlane)
{
  MARK_AS_UNIMPLEMENTED();
}

//------------------------------------------------------------ Plane-Plane-Plane
TEST(PlanePlanePlane)
{
  Vec4 planes[3] = { Vec4(real(1.0), real(0.0), real(0.0), real(0.0)), 
                           Vec4(real(0.0), real(1.0), real(0.0), real(0.0)), 
                           Vec4(real(0.0), real(0.0), real(1.0), real(0.0)) };

  Intersection::Type result = PlanePlanePlane(planes[0], planes[1], planes[2]);
  CHECK_EQUAL(Point, result);
}

//----------------------------------------------------------------- Plane-Sphere
TEST(PlaneSphere)
{
  MARK_AS_UNIMPLEMENTED();
}

//--------------------------------------------------------------- Plane-Triangle
TEST(PlaneTriangle)
{
  MARK_AS_UNIMPLEMENTED();
}

//---------------------------------------------------------------- Sphere-Sphere
TEST(SphereSphere)
{
  MARK_AS_UNIMPLEMENTED();
}

//-------------------------------------------------------------- Sphere-Triangle
TEST(SphereTriangle)
{
  MARK_AS_UNIMPLEMENTED();
}

//------------------------------------------------------------ Triangle-Triangle
TEST(TriangleTriangle)
{
  MARK_AS_UNIMPLEMENTED();
}
