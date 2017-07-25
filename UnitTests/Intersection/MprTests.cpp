///////////////////////////////////////////////////////////////////////////////
///
///	\file MprTests.cpp
///	Unit tests for the MPR collision detection algorithm.
///	
///	Authors: Benjamin Strukus
///	Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
#include "Intersection/UnitTestCommon.hpp"

namespace
{
class TestShape
{
public:
  TestShape(Vec3Param center)
    : mCenter(center)
  {
    //
  }

  void GetCenter(Vec3Ref center) const
  {
    center = mCenter;
  }

  virtual void Support(Vec3Param direction, Vec3Ptr support) const = 0;

  Intersection::SupportShape GetSupportShape(void)
  {
    return Intersection::MakeSupport(this);
  }

protected:
  Vec3 mCenter;
};

//--------------------------------------------------------------------- Test Box
class TestBox : public TestShape
{
public:
  TestBox(Vec3Param center, Vec3Param halfExtents, Mat3Param orientation)
    : TestShape(center), mHalfExtents(halfExtents), mBasis(orientation)
  {
    //
  }

  void Support(Vec3Param direction, Vec3Ptr support) const
  {
    Geometry::SupportObb(direction, mCenter, mHalfExtents, mBasis, support);
  }

private:
  Vec3 mHalfExtents;
  Mat3 mBasis;
};

//--------------------------------------------------------------- Test Ellipsoid
class TestEllipsoid : public TestShape
{
public:
  TestEllipsoid(Vec3Param center, Vec3Param radii, Mat3Param orientation)
    : TestShape(center), mRadii(radii), mBasis(orientation)
  {
    //
  }

  void Support(Vec3Param direction, Vec3Ptr support) const
  {
    Geometry::SupportEllipsoid(direction, mCenter, mRadii, mBasis, support);
  }

private:
  Vec3 mRadii;
  Mat3 mBasis;
};

//------------------------------------------------------------------ Test Sphere
class TestSphere : public TestShape
{
public:
  TestSphere(Vec3Param center, real radius)
    : TestShape(center), mRadius(radius)
  {
    //
  }

  void Support(Vec3Param direction, Vec3Ptr support) const
  {
    Geometry::SupportSphere(direction, mCenter, mRadius, support);
  }

private:
  real mRadius;
};

}// namespace

TEST(Mpr_1)
{
  Vec3 center = Vec3(real(32.5), real(-19.0), real(2.0));
  Vec3 halfExtents = Vec3(real(37.0), real(0.5), real(1.5));
  Mat3 basis = Mat3::cIdentity;

  TestBox box(center, halfExtents, basis);

  center = Vec3(real(46.771263), real(-15.901074), real(1.4903291));
  halfExtents = Vec3(real(1.0), real(2.5), real(1.0));
  TestEllipsoid ellipsoid(center, halfExtents, basis);

  Intersection::SupportShape boxShape = box.GetSupportShape();
  Intersection::SupportShape ellipsoidShape = ellipsoid.GetSupportShape();

  Intersection::Manifold manifold;
  Intersection::Mpr mprClass;
  mprClass.Test(&boxShape, &ellipsoidShape, &manifold);
}

TEST(Mpr_2)
{
  Vec3 center = Vec3(real(0.25), real(0.0), real(0.0));
  real radius = real(0.5);

  TestSphere sphereA( center, radius);
  TestSphere sphereB(-center, radius);

  Intersection::SupportShape shapes[2] = { sphereA.GetSupportShape(), 
                                           sphereB.GetSupportShape() };
  Intersection::Manifold manifold;
  Intersection::Mpr mprClass;
  //mprClass.Test(&(shapes[0]), &(shapes[1]), &manifold);
}

TEST(Mpr_3)
{
  Vec3 center = Vec3(real(0.19245008972987525483638292683399), 
                     real(0.19245008972987525483638292683399), 
                     real(0.19245008972987525483638292683399));
  real radius = real(0.5);

  TestSphere sphereA( center, radius);
  TestSphere sphereB(-center, radius);

  Intersection::SupportShape shapes[2] = { sphereA.GetSupportShape(), 
                                           sphereB.GetSupportShape() };
  Intersection::Manifold manifold;
  Intersection::Mpr mprClass;
  mprClass.Test(&(shapes[0]), &(shapes[1]), &manifold);
}
