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
#include "Intersection/UnitTestCommon.hpp"

using Geometry::Hull3D;

namespace
{
} //namespace

void VerifyTriangles(CppUnitLite::TestResult& result_,const char * m_name, Hull3D& hull, Vec3Array& origPoints, real epsilon = real(.0001))
{
  //for every triangle on the hull, make sure that no point is positive
  //in the direction of the normal (ignoring points on the triangle)
  Hull3D::FaceList::range faces = hull.GetFaces();
  for(; !faces.Empty(); faces.PopFront())
  {
    Hull3D::Face& face = faces.Front();
    uint i0 = face.Vertices[0]->Id;
    uint i1 = face.Vertices[1]->Id;
    uint i2 = face.Vertices[2]->Id;
    Vec3 p0 = face.Vertices[0]->Position;
    Vec3 p1 = face.Vertices[1]->Position;
    Vec3 p2 = face.Vertices[2]->Position;

    Zero::Triangle tri(p0,p1,p2);
    Vec3 normal = tri.GetNormal();

    for(uint j = 0; j < origPoints.Size(); ++j)
    {
      //skip this vertex if it's part of the triangle
      if(j == i0 || j == i1 || j == i2)
        continue;

      //check the the distance from the plane, if it's positive the point
      //is on the wrong side and this triangle is invalid
      real distance = Math::Dot(origPoints[j] - p0,normal);
      if(distance - epsilon > 0)
      {
        CHECK_EQUAL(0,distance);
      }
    }
  }
}

void CheckForInvalidTriangles(CppUnitLite::TestResult& result_,const char * m_name, Hull3D& hull, Vec3Array& origPoints)
{
  //build an adjacency list for every triangle (based upon edges).
  //Make sure that each triangle has exactly 3 adjacent triangles.
  typedef Zero::HashMap<uint,uint> AdjacencyInfo;
  AdjacencyInfo adjacencyCount;

  Hull3D::FaceList::range faces = hull.GetFaces();
  for(; !faces.Empty(); faces.PopFront())
  {
    Hull3D::Face& face = faces.Front();
    adjacencyCount.Insert(face.Id,0);
  }

  Hull3D::EdgeList::range edges = hull.GetEdges();
  for(; !edges.Empty(); edges.PopFront())
  {
    Hull3D::Edge& edge = edges.Front();
    uint id0 = edge.AdjFaces[0]->Id;
    uint id1 = edge.AdjFaces[1]->Id;

    ++adjacencyCount[id0];
    ++adjacencyCount[id1];
  }

  AdjacencyInfo::range r = adjacencyCount.All();
  for(; !r.Empty(); r.PopFront())
  {
    uint adjacencyCount = r.Front().second;
    CHECK_EQUAL(3,adjacencyCount);
  }
}

void CheckEuler(CppUnitLite::TestResult& result_,const char * m_name, Hull3D& hull)
{
  uint vertexCount = hull.GetHullVertexCount();
  uint edgeCount = hull.GetEdgeCount();
  uint faceCount = hull.GetFaceCount();

  CHECK_EQUAL(2, vertexCount + faceCount - edgeCount);
  CHECK_EQUAL(faceCount, 2 * vertexCount - 4);
  CHECK_EQUAL(2 * edgeCount, 3 * faceCount);
}

void VerifyHull(CppUnitLite::TestResult& result_,const char * m_name, Hull3D& hull, Vec3Array& origPoints)
{
  VerifyTriangles(result_,m_name,hull,origPoints);
  CheckForInvalidTriangles(result_,m_name,hull,origPoints);
  CheckEuler(result_,m_name,hull);
}

TEST(NewConvexHull1)
{
  Vec3Array points;
  points.PushBack(Vec3(-1,0,0));
  points.PushBack(Vec3( 1,0,0));
  points.PushBack(Vec3( 0,0,1));
  points.PushBack(Vec3( 0,1,0));

  Hull3D hull;
  hull.Build(points.Data(),points.Size());

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


  Hull3D hull;
  hull.Build(points.Data(),points.Size());

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


  Hull3D hull;
  hull.Build(points.Data(),points.Size());

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


  Hull3D hull;
  hull.Build(points.Data(),points.Size());

  VerifyHull(result_,m_name,hull,points);
}

void RandomHullTest(CppUnitLite::TestResult& result_,const char * m_name, uint pointCount, real scale, Math::Random& rand)
{
  //build random points within a sphere of the given size
  Vec3Array points;
  for(uint i = 0; i < pointCount; ++i)
    points.PushBack(rand.PointInUnitSphere() * scale);

  //now build and test the hull
  Hull3D hull;
  hull.Build(points.Data(),points.Size());

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

  Hull3D hull;
  hull.Build(points);

  VerifyHull(result_,m_name,hull,points);
}

TEST(OctahedronConvexHull)
{
  uint size = NumElements(Geometry::PlatonicSolids::cOctahedronPoints);
  TestPoints(result_,m_name,Geometry::PlatonicSolids::cOctahedronPoints, size);
}
