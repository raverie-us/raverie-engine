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

namespace
{
uint GenerateDirections(uint granularity, 
                                  Vec3Ptr& directions)
{
  uint vertexCount;
  uint* indexBuffer = NULL;
  uint indexCount;
  Vec3* normalBuffer = NULL;
  Vec2* textureCoords = NULL;
  BuildIcoSphere(granularity, directions, vertexCount, indexBuffer, indexCount);
  return vertexCount;
}
} //namespace

#define MTX(a,b,c,d,e,f,g,h,i)          \
{ { real((a)), real((b)), real((c)) },  \
  { real((d)), real((e)), real((f)) },  \
  { real((g)), real((h)), real((i)) } }

TEST(BoxMeshTest)
{
//   Vec3 boxPoints[8] = { Box::cPoint[0], Box::cPoint[1],
//                         Box::cPoint[2], Box::cPoint[3],
//                         Box::cPoint[4], Box::cPoint[5],
//                         Box::cPoint[6], Box::cPoint[7] };
//   const Vec3 boxExtents = Vec3(Length(boxPoints[0] - boxPoints[1]), 
//                          Length(boxPoints[0] - boxPoints[4]),
//                          Length(boxPoints[0] - boxPoints[3]));
//   const real boxVolume = boxExtents.x * boxExtents.y * boxExtents.z;
//   const real boxInertia[3][3] = MTX(Math::Sq(boxExtents.x) / real(6.0), 
//                                     real(0.0), real(0.0), real(0.0),
//                                     Math::Sq(boxExtents.y) / real(6.0),
//                                     real(0.0), real(0.0), real(0.0),
//                                     Math::Sq(boxExtents.z) / real(6.0));
// 
//   Hull boxHull;
// 
//   boxHull.Build(&(boxPoints[0]), 8);
// 
//   Mesh boxMesh;
//   boxMesh.CopyInfo(boxHull);
// 
//   Vec3* vertexBuffer = NULL;
//   uint* indexBuffer = NULL;
//   uint triCount = boxMesh.GenerateRenderData(&vertexBuffer, &indexBuffer);
// 
//   real volume = CalculateTriMeshVolume(vertexBuffer, indexBuffer, triCount);
//   CHECK_EQUAL(boxVolume, volume);
// 
//   Vec3 centerOfMass = CalculateTriMeshCenterOfMass(vertexBuffer, indexBuffer,
//                                                    triCount);
//   CHECK_VEC3(Vec3::cZero, centerOfMass);
// 
//   Mat3 inertiaTensor;
//   CalculateTriMeshInertiaTensor(vertexBuffer, indexBuffer, triCount, 
//                                 centerOfMass, &inertiaTensor);
// 
//   CHECK_MAT3(boxInertia, inertiaTensor);
// 
//   delete [] vertexBuffer;
//   delete [] indexBuffer;
}

TEST(GeodesicSphereMeshTest_20)
{
//   Vec3* vertexBuffer = NULL;
//   uint vertexCount;
//   uint* indexBuffer = NULL;
//   uint indexCount;
//   Vec3* normalBuffer = NULL;
//   Vec2* textureCoords = NULL;
//   BuildIcoSphere(20, vertexBuffer, vertexCount, indexBuffer, indexCount);
// 
//   real volume = CalculateTriMeshVolume(vertexBuffer, indexBuffer, indexCount);
//   real sphereVolume = Math::cPi * real(4.0) / real(3.0);
//   CHECK_EQUAL(sphereVolume, volume);
// 
//   Vec3 centerOfMass = CalculateTriMeshCenterOfMass(vertexBuffer, indexBuffer, 
//                                                    indexCount);
//   CHECK_VEC3_CLOSE(Vec3::cZero, centerOfMass, real(0.0000001));
// 
//   Mat3 inertiaTensor;
//   CalculateTriMeshInertiaTensor(vertexBuffer, indexBuffer, indexCount, 
//                                 centerOfMass, &inertiaTensor);
// 
//   delete [] vertexBuffer;
//   delete [] indexBuffer;
//   delete [] normalBuffer;
//   delete [] textureCoords;
}

TEST(CylinderFromSupports)
{
//   Vec3* directions = NULL;
//   uint directionCount = GenerateDirections(10, directions);  
// 
//   Vec3 center = Vec3::cZero;
//   real radius = real(1.0);
//   real halfHeight = real(1.0);
//   Mat3 basis = Mat3::cIdentity;
// 
//   Vec3* cylinderPoints = new Vec3[directionCount];
//   for(uint i = 0; i < directionCount; ++i)
//   {
//     SupportCylinder(directions[i], center, halfHeight, radius, basis, 
//                     (cylinderPoints + i));
//   }
//   delete [] directions;
// 
//   Hull cylinderHull;
//   cylinderHull.Build(cylinderPoints, directionCount);
//   delete [] cylinderPoints;
// 
//   Mesh cylinderMesh;
//   cylinderMesh.CopyInfo(cylinderHull);
// 
//   uint* cylinderTris;
//   uint triCount = cylinderMesh.GenerateRenderData(&cylinderPoints, 
//                                                   &cylinderTris);
// 
//   real volume = CalculateTriMeshVolume(cylinderPoints, cylinderTris, triCount);
//   Vec3 centerOfMass = CalculateTriMeshCenterOfMass(cylinderPoints, cylinderTris, 
//                                                    triCount);
//   Mat3 inertiaTensor;
//   CalculateTriMeshInertiaTensor(cylinderPoints, cylinderTris, triCount, 
//                                 centerOfMass, &inertiaTensor);
//   int i = 0;
//   i = 5;
}

TEST(EllipsoidFromSupports)
{
//   Vec3* directions = NULL;
//   printf("Generating directions...\n");
//   uint directionCount = GenerateDirections(20, directions);
// 
//   Vec3 center = Vec3::cZero;
//   Vec3 radii = Vec3(real(1.0), real(2.0), real(3.0));
//   real halfHeight = real(1.0);
//   Mat3 basis = Mat3::cIdentity;
// 
//   printf("Mapping directions to points...\n");
//   Vec3* ellipsoidPoints = new Vec3[directionCount];
//   for(uint i = 0; i < directionCount; ++i)
//   {
//     SupportEllipsoid(directions[i], center, radii, basis, 
//                      (ellipsoidPoints + i));
//   }
//   delete [] directions;
// 
//   printf("Computing hull...\n");
//   Hull ellipsoidHull;
//   ellipsoidHull.Build(ellipsoidPoints, directionCount);
//   delete [] ellipsoidPoints;

// 
//   printf("Generating mesh...\n");
//   Mesh ellipsoidMesh;
//   ellipsoidMesh.CopyInfo(ellipsoidHull);
// 
//   printf("Generating render data...\n");
//   uint* ellipsoidTris;
//   uint triCount = ellipsoidMesh.GenerateRenderData(&ellipsoidPoints, 
//                                                    &ellipsoidTris);
// 
//   printf("Calculating volume...\n");
//   real volume = CalculateTriMeshVolume(ellipsoidPoints, ellipsoidTris, 
//                                        triCount);
// 
//   printf("Calculating center of mass...\n");
//   Vec3 centerOfMass = CalculateTriMeshCenterOfMass(ellipsoidPoints, 
//                                                    ellipsoidTris, triCount);
// 
//   printf("Calculating inertia tensor...\n");
//   Mat3 inertiaTensor;
//   CalculateTriMeshInertiaTensor(ellipsoidPoints, ellipsoidTris, triCount, 
//                                 centerOfMass, &inertiaTensor);
// 
//   printf("Volume = %f\n", volume);
//   printf("Center of Mass = [%f, %f, %f]\n", centerOfMass.x, centerOfMass.y, 
//                                             centerOfMass.z);
//   printf("Inertia Tensor = [%f, %f, %f]\n", inertiaTensor.m00, 
//                                             inertiaTensor.m01, 
//                                             inertiaTensor.m02);
//   printf("                 [%f, %f, %f]\n", inertiaTensor.m10, 
//                                             inertiaTensor.m11, 
//                                             inertiaTensor.m12);
//   printf("                 [%f, %f, %f]\n", inertiaTensor.m20, 
//                                             inertiaTensor.m21, 
//                                             inertiaTensor.m22);
//   printf("%i\n", directionCount);
//   delete [] ellipsoidPoints;
//   delete [] ellipsoidTris;
  return;
}

#undef MTX
