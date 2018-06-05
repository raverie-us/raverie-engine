///////////////////////////////////////////////////////////////////////////////
///
///	\file Hull3dTests.cpp
///	Unit tests for the Hull3d alrgorithm.
///	
///	Authors: Joshua Davis
///	Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
#include "IntersectionTests/UnitTestCommon.hpp"
#include "Geometry/QuickHull3D.hpp"

using Zero::QuickHull3D;

namespace
{
} //namespace

void VerifyTriangles(CppUnitLite::TestResult& result_,const char * m_name, QuickHull3D& hull, Vec3Array& origPoints, real epsilon = real(.0001))
{
  //for every triangle on the hull, make sure that no point is positive
  //in the direction of the normal (ignoring points on the triangle)
  QuickHull3D::FaceList::range faces = hull.GetFaces();
  for(; !faces.Empty(); faces.PopFront())
  {
    QuickHull3D::QuickHullFace& face = faces.Front();
    face.RecomputeCenterAndNormal();
    Vec3 normal = face.mNormal;
    
    for(uint j = 0; j < origPoints.Size(); ++j)
    {    
      //check the the distance from the plane, if it's positive the point
      //is on the wrong side and this triangle is invalid
      real distance = Math::Dot(origPoints[j] - face.mCenter,normal);
      if(distance - epsilon > 0)
      {
        CHECK_EQUAL(0,distance);
      }
    }
  }
}

void CheckEuler(CppUnitLite::TestResult& result_,const char * m_name, QuickHull3D& hull)
{
  uint vertexCount = hull.ComputeVertexCount();
  uint edgeCount = hull.ComputeEdgeCount();
  uint faceCount = hull.ComputeFaceCount();

  CHECK_EQUAL(2, vertexCount + faceCount - edgeCount);
  // This looks like it assumes triangles
  //CHECK_EQUAL(faceCount, 2 * vertexCount - 4);
  //CHECK_EQUAL(2 * edgeCount, 3 * faceCount);
}

void VerifyHull(CppUnitLite::TestResult& result_,const char * m_name, QuickHull3D& hull, Vec3Array& origPoints)
{
  VerifyTriangles(result_,m_name,hull,origPoints);
  CheckEuler(result_,m_name,hull);
}

TEST(NewConvexHull1)
{
  Vec3Array points;
  points.PushBack(Vec3(-1,0,0));
  points.PushBack(Vec3( 1,0,0));
  points.PushBack(Vec3( 0,0,1));
  points.PushBack(Vec3( 0,1,0));

  QuickHull3D hull;
  hull.Build(points);

  VerifyHull(result_,m_name,hull,points);
}

TEST(TetrahedronConvexHull2)
{
  Vec3Array points;
  points.PushBack(Vec3(-1,-1,-1));
  points.PushBack(Vec3( 1,-1,-1));
  points.PushBack(Vec3( 0,-1, 1));
  points.PushBack(Vec3( 0, 1, 0));
  points.PushBack(Vec3( 1, 0,-2));


  QuickHull3D hull;
  hull.Build(points);

  VerifyHull(result_,m_name,hull,points);
}

TEST(TetrahedronConvexHull3)
{
  Vec3Array points;
  points.PushBack(Vec3(-1,-1,-1));
  points.PushBack(Vec3( 1,-1,-1));
  points.PushBack(Vec3( 0,.5, 0));
  points.PushBack(Vec3( 0,-1, 1));
  points.PushBack(Vec3( 0, 1, 0));


  QuickHull3D hull;
  hull.Build(points);

  VerifyHull(result_,m_name,hull,points);
}

TEST(TetrahedronConvexHull4)
{
  Vec3Array points;
  points.PushBack(Vec3(8.452484,-0.000000,-16.087555));
  points.PushBack(Vec3(38.487213,-0.000000,-16.087555));
  points.PushBack(Vec3(8.452484,0.000000,-34.449585));
  points.PushBack(Vec3(38.487213,0.000000,-34.449585));
  points.PushBack(Vec3(8.452484,12.217231,-16.087555));
  points.PushBack(Vec3(38.487213,12.217231,-16.087555));
  points.PushBack(Vec3(8.452484,12.217231,-34.449585));
  points.PushBack(Vec3(38.487213,12.217231,-34.449585));
  points.PushBack(Vec3(-17.225021,-0.000000,12.961136));
  points.PushBack(Vec3(-0.768448,-0.000000,12.961136));
  points.PushBack(Vec3(-17.225021,0.000000,-0.768875));
  points.PushBack(Vec3(-0.768448,0.000000,-0.768875));
  points.PushBack(Vec3(-17.225021,9.125648,12.961136));
  points.PushBack(Vec3(-0.768448,9.125648,12.961136));
  points.PushBack(Vec3(-17.225021,9.125648,-0.768875));
  points.PushBack(Vec3(-0.768448,9.125648,-0.768875));


  QuickHull3D hull;
  hull.Build(points);

  VerifyHull(result_,m_name,hull,points);
}

void RandomHullTest(CppUnitLite::TestResult& result_,const char * m_name, uint pointCount, real scale, Math::Random& rand)
{
  //build random points within a sphere of the given size
  Vec3Array points;
  for(uint i = 0; i < pointCount; ++i)
    points.PushBack(rand.PointInUnitSphere() * scale);

  //now build and test the hull
  QuickHull3D hull;
  hull.Build(points);

  VerifyHull(result_,m_name,hull,points);
}

TEST(RandomConvexHull1)
{
  //test a medium size sphere with a few points
  Math::Random rand(0);
  uint pointCount = rand.IntRangeInEx(10,50);
  real scale = rand.FloatRange(5.0f,10.0f);
  RandomHullTest(result_,m_name,pointCount,scale,rand);
}

TEST(RandomConvexHull2)
{
  //test a large number of points with a decently large sphere
  Math::Random rand(0);
  uint pointCount = rand.IntRangeInEx(50,999999);
  real scale = rand.FloatRange(1.0f,100.0f);
  RandomHullTest(result_,m_name,pointCount,scale,rand);
}

//safe number of elements macro
template <typename T, size_t SizeOfArray>
char (*NumElementsHelper(T (&Array)[SizeOfArray]))[SizeOfArray];
#define NumElements(Array)\
    sizeof(*NumElementsHelper(Array))

void TestPoints(CppUnitLite::TestResult& result_, const char * m_name, const Vec3* inputPoints, uint size)
{
  Zero::Array<Vec3> points;
  points.Resize(size);
  for(uint i = 0; i < size; ++i)
    points[i] = inputPoints[i];

  QuickHull3D hull;
  hull.Build(points);

  VerifyHull(result_,m_name,hull,points);
}

TEST(OctahedronConvexHull)
{
  uint size = NumElements(Geometry::PlatonicSolids::cOctahedronPoints);
  TestPoints(result_,m_name,Geometry::PlatonicSolids::cOctahedronPoints, size);
}
