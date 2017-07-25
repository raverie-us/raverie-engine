///////////////////////////////////////////////////////////////////////////////
///
///	\file MeshGenerationTests.cpp
///	Unit tests for the various functions associated with meshes as part of the
/// geometry library.
///	
///	Authors: Benjamin Strukus
///	Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
#include "Intersection/UnitTestCommon.hpp"

using namespace Geometry;

TEST(Triangulator_ConstructTrapezoids)
{
  Vec2 points[] = { Vec2(real(0.0), real(0.0)),
                    Vec2(real(6.0), real(0.0)),
                    Vec2(real(6.0), real(6.0)),
                    Vec2(real(0.0), real(6.0)),
                    
                    Vec2(real(0.5), real(1.0)),
                    Vec2(real(1.0), real(2.0)),
                    Vec2(real(2.0), real(1.5)),

                    Vec2(real(0.5), real(4.0)),
                    Vec2(real(1.0), real(5.0)),
                    Vec2(real(2.0), real(4.5)),

                    Vec2(real(3.0), real(3.0)),
                    Vec2(real(5.0), real(3.5)),
                    Vec2(real(5.0), real(2.5))  };
  uint pointCount[] = { 4, 3, 3, 3 };
  uint contourCount = sizeof(pointCount) / sizeof(*pointCount);
  const uint numberOfPoints = sizeof(points) / sizeof(Vec2);
  //uint triangles[numberOfPoints * 3][3];

  //Triangulator triangulator;
  ////triangulator.TriangulatePolygon(contourCount, pointCount, points, triangles);
  //contourCount = 5;
}
