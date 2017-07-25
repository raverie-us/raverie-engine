///////////////////////////////////////////////////////////////////////////////
///
///	\file GeometryTests.cpp
///	Unit tests for the geometry library.
///	
///	Authors: Benjamin Strukus
///	Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
#include "Intersection/UnitTestCommon.hpp"

using namespace Geometry;

namespace
{
} //namespace

TEST(GenerateNormal)
{
  Vec3 points[3] = { Vec3::cXAxis, Vec3::cYAxis, Vec3::cZAxis };

  Vec3 normal = GenerateNormal(points[0], points[1], points[2]);
  CHECK_CLOSE(real(0.57735026918962576450), normal.x, real(0.00001));
  CHECK_CLOSE(real(0.57735026918962576450), normal.y, real(0.00001));
  CHECK_CLOSE(real(0.57735026918962576450), normal.z, real(0.00001));

  points[1] = Vec3::cXAxis;

  normal = GenerateNormal(points[0], points[1], points[2]);
  CHECK_CLOSE(real(0.0), normal.x, real(0.0));
  CHECK_CLOSE(real(0.0), normal.y, real(0.0));
  CHECK_CLOSE(real(0.0), normal.z, real(0.0));
}

TEST(BarycentricTetrahedron_1)
{
  Vec3 points[4] = { Vec3::cZero, Vec3::cXAxis, Vec3::cYAxis, Vec3::cZAxis };
  Vec4 barycentricCoordinates;
  BarycentricTetrahedron(points[0], points[0], points[1], points[2], points[3],
                         &barycentricCoordinates);
  real expected[4] = { real(1.0), real(0.0), real(0.0), real(0.0) };
  CHECK_VEC4(expected, barycentricCoordinates);
}

TEST(BarycentricTetrahedron_2)
{
  Vec3 points[4] = { Vec3::cZero, Vec3::cXAxis, Vec3::cYAxis, Vec3::cZAxis };
  Vec4 barycentricCoordinates;
  BarycentricTetrahedron(points[1], points[0], points[1], points[2], points[3],
                         &barycentricCoordinates);
  real expected[4] = { real(0.0), real(1.0), real(0.0), real(0.0) };
  CHECK_VEC4(expected, barycentricCoordinates);
}

TEST(BarycentricTetrahedron_3)
{
  Vec3 points[4] = { Vec3::cZero, Vec3::cXAxis, Vec3::cYAxis, Vec3::cZAxis };
  Vec4 barycentricCoordinates;
  BarycentricTetrahedron(points[2], points[0], points[1], points[2], points[3],
                         &barycentricCoordinates);
  real expected[4] = { real(0.0), real(0.0), real(1.0), real(0.0) };
  CHECK_VEC4(expected, barycentricCoordinates);
}

TEST(BarycentricTetrahedron_4)
{
  Vec3 points[4] = { Vec3::cZero, Vec3::cXAxis, Vec3::cYAxis, Vec3::cZAxis };
  Vec4 barycentricCoordinates;
  BarycentricTetrahedron(points[3], points[0], points[1], points[2], points[3],
                         &barycentricCoordinates);
  real expected[4] = { real(0.0), real(0.0), real(0.0), real(1.0) };
  CHECK_VEC4(expected, barycentricCoordinates);
}

TEST(BarycentricTetrahedron_5)
{
  Vec3 points[4] = { Vec3::cZero, Vec3::cXAxis, Vec3::cYAxis, Vec3::cZAxis };
  Vec4 barycentricCoordinates;
  Vec3 point = Vec3(real(0.25), real(0.25), real(0.25));
  BarycentricTetrahedron(point, points[0], points[1], points[2], points[3],
                         &barycentricCoordinates);
  real expected[4] = { real(0.25), real(0.25), real(0.25), real(0.25) };
  CHECK_VEC4(expected, barycentricCoordinates);
}

