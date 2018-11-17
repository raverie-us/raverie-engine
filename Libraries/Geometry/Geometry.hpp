///////////////////////////////////////////////////////////////////////////////
///
///  \file Geometry.hpp
///  Interface to all functions that deal with geometry but not intersection.
///
///  Authors: Benjamin Strukus
///  Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "GeometryStandard.hpp"

namespace Geometry
{
//-------------------------------------------------------------------- Constants
///Maximum number of support points that will be returned from clipping
const uint cMaxSupportPoints = 8;

//----------------------------------------------------------------- 2D Functions
///Calculate the centroid of the 2D polygon. Assumes the 2D points are ordered
///in such a way that they describe the polygon's perimeter.
void CalculatePolygonCentriod(const Vec2* polyPoints, uint polyPointCount,
                              Vec2Ptr barycenter);

///Given an ordered set of 2D points that describe the perimeter of a polygon,
///return whether the points are clockwise (negative) or 
///counter-clockwise (positive).
real DetermineWindingOrder(const Vec2* polyPoints, uint polyPointCount);

///Generate an axis-aligned bounding box for the given set of 2D points.
void GenerateAabb(const Vec2* points, uint pointCount, Vec2Ptr min, 
                  Vec2Ptr max);

///Returns 2 times the signed triangle area. The result is positive is abc is
///counter-clockwise, negative if abc is clockwise, zero if abc is degenerate.
real Signed2DTriArea(Vec2Param a, Vec2Param b, Vec2Param c);

//----------------------------------------------------------------- 3D Functions
///Generate an axis-aligned bounding box for the given set of 3D points.
void GenerateAabb(const Vec3* points, uint pointCount, Vec3Ptr min, 
                  Vec3Ptr max);

///Get a unit length vector which is orthogonal to the plane that points A, B,
///and C lie on.
Vec3 GenerateNormal(Vec3Param pointA, Vec3Param pointB, Vec3Param pointC);

///Returns whether or not the given triangle is valid for physics.
bool IsDegenerate(Vec3Param pointA, Vec3Param pointB, Vec3Param pointC);

///Get the signed distance of a point to a plane.
real SignedDistanceToPlane(Vec3Param point, Vec3Param planeNormal,
                           real planeDistance);

///Calculate the barycentric coordinates of the point with respect to the
///triangle. Returns XYZ correspond to triangle's ABC points, respectively.
void BarycentricTriangle(Vec3Param point, Vec3Param trianglePointA,
                         Vec3Param trianglePointB, Vec3Param trianglePointC,
                         Vec3Ptr barycentricCoordinates);

///Calculate the barycentric coordinates of the point with respect to the 
///tetrahedron. Returns XYZW correspond to tetrahedron's ABCD points, 
///respectively.
void BarycentricTetrahedron(Vec3Param point, Vec3Param tetrahedronPointA, 
                            Vec3Param tetrahedronPointB, 
                            Vec3Param tetrahedronPointC,
                            Vec3Param tetrahedronPointD, 
                            Vec4Ptr barycentricCoordinates);

///Clip the set of coplanar polygon points against the plane.
uint ClipPolygonWithPlane(const Vec3* polygonPoints, uint polygonPointCount,
                          Vec3Param planeNormal, real planeDistance,
                          Vec3Ptr clippedPoints);

///Clip the set of coplanar polygon points against the given clipping planes.
///The planes are stored in a Vector4 with the plane normal in the (x,y,z) and
///the plane's distance in the w.
uint ClipPolygonWithPlanes(const Vec4* clipperPlanes, uint clipperPlaneCount,
                           const Vec3* polygonPoints, uint polygonPointCount,
                           Vec3Ptr clippedPoints);

///Clip the set of coplanar polygon points against the planes that define the
///clipping polygon.
uint ClipPolygonWithPolygon(const Vec3* clipperPoints, uint clipperPointCount,
                            const Vec3* polygonPoints, uint polygonPointCount,
                            Vec3Ptr clippedPoints);

///Calculate the barycenter of the given point set.
void CalculateBarycenter(const Vec3* points, uint count, Vec3Ptr barycenter);

///Given n-gon specified by points v[], compute a good representative plane p.
void ComputeBestFitPlane(const Vec3* polyPoints, uint polyPointCount, 
                         Vec3Ptr planeNormal, real* planeDistance);

//Default scale used by the following three functions.
#define NoScale Vec3(real(1.0), real(1.0), real(1.0))

///Calculate the volume of a triangular mesh, if no scale is provided then it
///assumes a scale of (1, 1, 1).
real CalculateTriMeshVolume(const Vec3* triMeshPoints, 
                            const uint* triMeshTriangles, 
                            uint triangleCount, Vec3Param scale = NoScale);
real CalculateTriMeshVolume(const Array<Vec3>& triMeshPoints,
                            const Array<uint>& triMeshTriangles, Vec3Param scale = NoScale);

///Calculate the center of mass of a triangular mesh, assuming uniform density.
///If no scale is provided then it assumes a scale of (1, 1, 1).
Vec3 CalculateTriMeshCenterOfMass(const Vec3* triMeshPoints, 
                                  const uint* triMeshTriangles, 
                                  uint triangleCount, 
                                  Vec3Param scale = NoScale);
Vec3 CalculateTriMeshCenterOfMass(const Array<Vec3>& triMeshPoints,
                                  const Array<uint>& triMeshTriangles,
                                  Vec3Param scale = NoScale);

///Scale can be applied afterwards to centerOfMass as scale * centerOfMass
///and volume can be scaled as scale.x * scale.y * scale.z * volume
void CalculateTriMeshCenterOfMassAndVolume(const Vec3* triMeshPoints, 
                                           const uint* triMeshTriangles, 
                                           uint triangleCount, 
                                           Vec3Ref centerOfMass, real& volume);
void CalculateTriMeshCenterOfMassAndVolume(const Array<Vec3>& triMeshPoints,
                                           const Array<uint>& triMeshTriangles,
                                           Vec3Ref centerOfMass, real& volume);

///Calculate the inertia tensor of a triangular mesh. Providing the center of 
///mass for the mesh is optional, but will increase the accuracy of the inertia
///tensor calculations (otherwise the origin is used). Assumes a mass of 1, 
///which allows for the mass to be easily factored in later. If no scale is 
///provided then it assumes a scale of (1, 1, 1).
void CalculateTriMeshInertiaTensor(const Vec3* triMeshPoints,
                                   const uint* triMeshTriangles,
                                   uint triangleCount, Vec3Param centerOfMass,
                                   Mat3Ptr inertiaTensor, 
                                   Vec3Param scale = NoScale);
void CalculateTriMeshInertiaTensor(const Array<Vec3>& triMeshPoints,
                                   const Array<uint>& triMeshTriangles, Vec3Param centerOfMass,
                                   Mat3Ptr inertiaTensor, Vec3Param scale = NoScale);

#undef NoScale

///Combines an inertia tensor that was computed about it's local center of mass with
///one computed about a different center of mass. This is used to compute the total
///inertia tensor of an object made out of sub pieces. (Parallel axis theorem)
void CombineInertiaTensor(Mat3Ref totalInertiaTensor, Vec3Param totalCenterOfMass,
                          Mat3Param localInertiaTensor, Vec3Param localCenterOfMass, real localMass);

//---------------------------------------------------------------------- Support
///Find the point furthest in the direction on an axis-aligned bounding box.
void SupportAabb(Vec3Param direction, Vec3Param aabbMinPoint,
                 Vec3Param aabbMaxPoint, Vec3Ptr support);

///Find the point furthest in the direction on a capsule.
void SupportCapsule(Vec3Param direction, Vec3Param capsulePointA,
                    Vec3Param capsulePointB, real capsuleRadius,
                    Vec3Ptr support);

//Find the point furthest in the direction on a cylinder.
void SupportCylinder(Vec3Param direction, Vec3Param cylinderPointA,
                     Vec3Param cylinderPointB, real cylinderRadius, 
                     Vec3Ptr support);

///Find the point furthest in the direction on a cylinder.
void SupportCylinder(Vec3Param direction, Vec3Param cylinderCenter,
                     real cylinderHalfHeight, real cylinderRadius,
                     Mat3Param cylinderBasis, Vec3Ptr support);

///Find the point furthest in the direction on an ellipsoid.
void SupportEllipsoid(Vec3Param direction, Vec3Param ellipsoidCenter,
                      Vec3Param ellipsoidRadii, Mat3Param ellipsoidBasis,
                      Vec3Ptr support);

///Find the point furthest in the direction on an oriented bounding box.
void SupportObb(Vec3Param direction, Vec3Param obbCenter,
                Vec3Param obbHalfExtents, Mat3Param obbBasis, Vec3Ptr support);

///Find the point furthest in the direction for a given set of points.
void SupportPointSet(Vec3Param direction, const Vec3Ptr points, uint pointCount, 
                     Vec3Param center, Vec3Param scale, Mat3Param basis, 
                     Vec3Ptr support);

///Find the point furthest in the direction on a segment.
void SupportSegment(Vec3Param direction, Vec3Param segmentStart,
                    Vec3Param segmentEnd, Vec3Ptr support);

///Find the point furthest in the direction on a sphere.
void SupportSphere(Vec3Param direction, Vec3Param sphereCenter,
                   real sphereRadius, Vec3Ptr support);

///Find the point furthest in the direction on a tetrahedron.
void SupportTetrahedron(Vec3Param direction, Vec3Param tetrahedronPointA,
                        Vec3Param tetrahedronPointB, 
                        Vec3Param tetrahedronPointC, 
                        Vec3Param tetrahedronPointD, Vec3Ptr support);

///Find the point furthest in the direction on a triangle.
void SupportTriangle(Vec3Param direction, Vec3Param trianglePointA,
                     Vec3Param trianglePointB, Vec3Param trianglePointC,
                     Vec3Ptr support);

//---------------------------------------------------------------------- Normals
///Get the normal on an axis-aligned bounding box at the specified point on the
///given axis-aligned bounding box.
Vec3 NormalFromPointOnAabb(Vec3Param point, Vec3Param aabbMinPoint,
                           Vec3Param aabbMaxPoint);

///Get the normal on a capsule at the specified point on the given capsule.
Vec3 NormalFromPointOnCapsule(Vec3Param point, Vec3Param capsulePointA,
                              Vec3Param capsulePointB, real capsuleRadius);

///Get the normal on a cylinder at the specified point on the given cylinder.
Vec3 NormalFromPointOnCylinder(Vec3Param point, Vec3Param cylinderCenter,
                               real cylinderRadius, real cylinderHalfHeight, 
                               Mat3Param cylinderBasis);

///Get the normal on an ellipsoid at the specified point on the given ellipsoid.
Vec3 NormalFromPointOnEllipsoid(Vec3Param point, Vec3Param ellipsoidCenter,
                                Vec3Param ellipsoidRadii,
                                Mat3Param ellipsoidBasis);

///Get the normal on an oriented bounding box at the specified point on the
///given oriented bounding box.
Vec3 NormalFromPointOnObb(Vec3Param point, Vec3Param obbCenter,
                          Vec3Param obbHalfExtents, Mat3Param obbBasis);

///Get the normal on a sphere at the specified point on the given sphere.
Vec3 NormalFromPointOnSphere(Vec3Param point, Vec3Param sphereCenter,
                             real sphereRadius);

///Get the normal on a torus at the specified point on the given torus. The 
///torus's z-axis is the axis going through its hole.
Vec3 NormalFromPointOnTorus(Vec3Param point, Vec3Param torusCenter, 
                            real torusRingRadius, real torusTubeRadius,
                            Mat3Param torusBasis);

///Get the normal on a triangle at the specified point on the given triangle.
Vec3 NormalFromPointOnTriangle(Vec3Param point, Vec3Param trianglePointA,
                               Vec3Param trianglePointB,
                               Vec3Param trianglePointC);

//---------------------------------------------------------- Texture Coordinates
///Get the texture coordinates on a cylinder at the specified point on the given
///cylinder.
Vec2 TextureCoordinatesFromPointOnCylinder(Vec3Param point, 
                                           Vec3Param cylinderCenter, 
                                           real cylinderHalfHeight,
                                           real cylinderRadius,
                                           Mat3Param cylinderBasis);

///Get the texture coordinates on an ellipsoid at the specified point on the 
///given ellipsoid.
Vec2 TextureCoordinatesFromPointOnEllipsoid(Vec3Param point, 
                                            Vec3Param ellipsoidCenter,
                                            Vec3Param ellipsoidRadii,
                                            Mat3Param ellipsoidBasis);

///Get the texture coordinates on an oriented bounding box at the specified 
///point on the given oriented bounding box.
Vec2 TextureCoordinatesFromPointOnObb(Vec3Param point, 
                                      Vec3Param obbCenter,
                                      Vec3Param obbHalfExtents, 
                                      Mat3Param obbBasis);

///Get the texture coordinates on a sphere at the specified point on the sphere.
Vec2 TextureCoordinatesFromPointOnSphere(Vec3Param point, 
                                         Vec3Param sphereCenter,
                                         real sphereRadius,
                                         Mat3Param sphereBasis);
}// namespace Geometry
