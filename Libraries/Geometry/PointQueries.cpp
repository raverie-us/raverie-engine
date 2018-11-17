///////////////////////////////////////////////////////////////////////////////
///
/// \file PointQueries.cpp
/// Simple boolean functions to test if a point lies inside of a shape.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Geometry/Intersection.hpp"
#include "Geometry/Geometry.hpp"
#include "Geometry/Mpr.hpp"

namespace Intersection
{

namespace
{
const real cDistanceEpsilon = real(0.000001);

void SupportPoint(const SupportShape* shape, void* data, Vec3Param center, Vec3Ptr support)
{
  *support = *reinterpret_cast<Vec3*>(data);
}

}// namespace

//Test to see if the given point lies on or inside the given point.
Type PointPoint(Vec3Param pointA, Vec3Param pointB)
{
  if(pointA == pointB)
  {
    return Inside;
  }
  return None;
}

//Test to see if the given point lies on or inside the given ray.
Type PointRay(Vec3Param point, Vec3Param rayStart, Vec3Param rayDirection)
{
  Vec3 closestPoint = point;
  ClosestPointOnRayToPoint(rayStart, rayDirection, &closestPoint);
  closestPoint -= point;
  if(LengthSq(closestPoint) > cDistanceEpsilon)
  {
    return None;
  }
  return Inside;
}

//Test to see if the given point lies on or inside the given segment.
Type PointSegment(Vec3Param point, Vec3Param segmentStart, Vec3Param segmentEnd)
{
  Vec3 closestPoint = point;
  ClosestPointOnSegmentToPoint(segmentStart, segmentEnd, &closestPoint);
  closestPoint -= point;
  if(LengthSq(closestPoint) > cDistanceEpsilon)
  {
    return None;
  }
  return Inside;
}

//Test to see if the given point lies on or inside the given axis-aligned
//bounding box.
Type PointAabb(Vec3Param point, Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint)
{
  ErrorIf((aabbMinPoint.x > aabbMaxPoint.x) || 
          (aabbMinPoint.y > aabbMaxPoint.y) || 
          (aabbMinPoint.z > aabbMaxPoint.z),
          "Intersection - Axis-aligned bounding box's minimum point is greater"\
          " than it's maximum point.");

  for(uint i = 0; i < 3; ++i)
  {
    if((point[i] < aabbMinPoint[i]) || (aabbMaxPoint[i] < point[i]))
    {
      return None;
    }
  }
  return Inside;
}

//Test to see if the given point lies on or inside the given capsule.
Type PointCapsule(Vec3Param point, Vec3Param capsulePointA, 
                  Vec3Param capsulePointB, real capsuleRadius)
{
  Vec3 closestPoint = point;
  ClosestPointOnSegmentToPoint(capsulePointA, capsulePointB, &closestPoint);
  closestPoint -= point;
  if(LengthSq(closestPoint) > (capsuleRadius * capsuleRadius))
  {
    return None;
  }
  return Inside;
}

//Test to see if the given point lies on or inside the given support shape.
Type PointConvexShape(Vec3Param point, const SupportShape& convexShape)
{
  Vec3 center = point;
  SupportShape pointShape(point, SupportPoint, &center);
  Mpr mpr;
  return mpr.Test(&pointShape, &convexShape);
}

//Test to see if the given point lies on or inside the given cylinder.
Type PointCylinder(Vec3Param point, Vec3Param cylinderPointA, 
                   Vec3Param cylinderPointB, real cylinderRadius)
{
  Vec3 closestPoint = point;

  //First treat the cylinder as a capsule and do a radius test. This 
  //overestimates the size of the cylinder, but if it returns "None" then it is
  //correct, otherwise we have to test the actual cylinder.
  if(None == PointCapsule(point, cylinderPointA, cylinderPointB, 
                          cylinderRadius))
  {
    return None;
  }

  //Now check to see if the point is outside of the planes defined by the 
  //cylinder's end caps. Test against the plane at point A first.
  Vec3 normal = Normalized(cylinderPointA - cylinderPointB);
  real distance = Geometry::SignedDistanceToPlane(point, normal, 
                                                  Dot(normal, cylinderPointA));
  if(distance > cDistanceEpsilon)
  {
    return None;
  }
  
  //Now test against the plane at point B.
  Negate(&normal);
  distance = Geometry::SignedDistanceToPlane(point, normal, 
                                             Dot(normal, cylinderPointB));
  if(distance > cDistanceEpsilon)
  {
    return None;
  }
  return Inside;
}

//Test to see if the given point lies on or inside the given ellipsoid.
Type PointEllipsoid(Vec3Param point, Vec3Param ellipsoidCenter,
                    Vec3Param ellipsoidRadii, Mat3Param ellipsoidBasis)
{
  //First get the point in the reference frame of the ellipsoid's center
  Vec3 testPoint = point - ellipsoidCenter;

  //Bring the point into the ellipsoid's model space
  Math::TransposedTransform(ellipsoidBasis, &testPoint);

  //Scale the point by the ellipsoid's radii to bring it to the space where the
  //ellipsoid is the unit sphere (radius = 1)
  testPoint.x /= ellipsoidRadii.x;
  testPoint.y /= ellipsoidRadii.y;
  testPoint.z /= ellipsoidRadii.z;

  //Test the point against the sphere
  real lengthSquared = LengthSq(testPoint);
  if(lengthSquared > real(1.0))
  {
    return None;
  }
  return Inside;
}

//Test to see if the given point lies inside the given frustum. The 6 planes of
//the frustum are assumed to be pointing inwards.
Type PointFrustum(Vec3Param point, const Vec4 frustumPlanes[6])
{
  const Vec3* planeNormal = nullptr;
  for(uint i = 0; i < 6; ++i)
  {
    planeNormal = reinterpret_cast<const Vec3*>(&(frustumPlanes[i].x));
    //If positive, the point is inside this plane.
    real d = Geometry::SignedDistanceToPlane(point, *planeNormal, 
                                             frustumPlanes[i][3]);

    //If the point is found to be outside even one plane, it is not inside the
    //frustum.
    if(d < -cDistanceEpsilon)
    {
      return None;
    }
  }
  return Inside;
}

//Test to see if the given point lies on or inside the given oriented-bounding
//box.
Type PointObb(Vec3Param point, Vec3Param obbCenter, Vec3Param obbHalfExtents, 
              Mat3Param obbBasis)
{
  Vec3 obbToPoint = point - obbCenter;
  for(uint i = 0; i < 3; ++i)
  {
    real dot = Dot(obbToPoint, obbBasis.GetBasis(i));
    real extent = obbHalfExtents[i];

    //Check if the point is in between the box's center and the faces on
    //this axis. If it isn't, then the point is not inside.
    if(Math::InRange(dot, -extent, extent) == false)
    {
      return None;
    }
  }
  return Inside;
}

//Test to see if the given point lies on or inside the given plane.
Type PointPlane(Vec3Param point, Vec3Param planeNormal, real planeDistance)
{
  real d = Geometry::SignedDistanceToPlane(point, planeNormal, planeDistance);
  if(d > cDistanceEpsilon)
  {
    return None;
  }
  return Inside;
}

//Test to see if the given point lies on or inside the given sphere.
Type PointSphere(Vec3Param point, Vec3Param sphereCenter, real sphereRadius)
{
  Vec3 pointToSphere = sphereCenter - point;
  real lengthSquared = LengthSq(pointToSphere);

  //First check if the point is outside of the sphere
  if(lengthSquared > (sphereRadius * sphereRadius))
  {
    return None;
  }
  return Inside;
}

//Test to see if the given point lies on or inside the given tetrahedron.
Type PointTetrahedron(Vec3Param point, Vec3Param tetrahedronPointA,
                      Vec3Param tetrahedronPointB, Vec3Param tetrahedronPointC,
                      Vec3Param tetrahedronPointD)
{
  Vec4 barycentricCoordinates;
  Geometry::BarycentricTetrahedron(point, tetrahedronPointA, tetrahedronPointB, 
                                          tetrahedronPointC, tetrahedronPointD, 
                                          &barycentricCoordinates);
  for(uint i = 0; i < 4; ++i)
  {
    if(!Math::InRange(barycentricCoordinates[i], real(0.0), real(1.0)))
    {
      return Outside;
    }
  }
  return Inside;;
}

//Test to see if the given point lies on or inside the given counterclockwise 
//triangle. Treats the point as if it was lying on the plane of the triangle, so
//this can be more accurately described as "point vs triangular prism".
Type PointTriangle(Vec3Param point, Vec3Param trianglePointA, 
                   Vec3Param trianglePointB, Vec3Param trianglePointC, real epsilon)
{ 
  //Compute the vectors
  Vec3 vec[3] = { trianglePointC - trianglePointA, 
                  trianglePointB - trianglePointA,
                  point - trianglePointA          };

  //Compute the dot products
  real dot00 = Dot(vec[0], vec[0]);
  real dot01 = Dot(vec[0], vec[1]);
  real dot02 = Dot(vec[0], vec[2]);
  real dot11 = Dot(vec[1], vec[1]);
  real dot12 = Dot(vec[1], vec[2]);
  real dot22 = Dot(vec[2], vec[2]);

  //Compute barycentric coordinates
  real denom = dot00 * dot11 - dot01 * dot01;
  //check for a zero division
  if(denom == real(0.0))
    return None;
  denom = real(1.0) / denom;
  real u = (dot11 * dot02 - dot01 * dot12) * denom;
  real v = (dot00 * dot12 - dot01 * dot02) * denom;

  //Check if point is in triangle
  if(u >= -epsilon && v >= -epsilon && (u + v <= real(1.0) + epsilon))
  {
    return Inside;
  }
  return None;
}

}// namespace Intersection
