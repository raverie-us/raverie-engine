///////////////////////////////////////////////////////////////////////////////
///
/// \file ObbIntersection.cpp
/// All of the intersection tests that involve oriented bounding boxes.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Geometry/Intersection.hpp"
#include "Geometry/Geometry.hpp"

namespace Intersection
{

typedef IntersectionType  Type;

namespace
{

}// namespace

//Intersect an axis aligned bounding box with a frustum. The 6 planes of the 
//frustum are assumed to be pointing inwards.
Type AabbFrustumApproximation(Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint,
  const Vec4 frustumPlanes[6], Manifold* manifold)
{
  ErrorIf((aabbMinPoint.x > aabbMaxPoint.x) ||
    (aabbMinPoint.y > aabbMaxPoint.y) ||
    (aabbMinPoint.z > aabbMaxPoint.z),
    "Intersection - Axis-aligned bounding box's minimum point is greater"\
    " than it's maximum point.");

  Vec3 halfExtents = real(0.5) * (aabbMaxPoint - aabbMinPoint);
  Vec3 boxCenter = halfExtents + aabbMinPoint;
  const Vec3* planeNormal = nullptr;

  Type inOrOut = Inside;

  for(uint i = 0; i < 6; ++i)
  {
    planeNormal = reinterpret_cast<const Vec3*>(&(frustumPlanes[i].x));
    real planeDistance = frustumPlanes[i][3];

    Vec3 signs = Vec3(Math::GetSign(planeNormal->x) * halfExtents.x, 
      Math::GetSign(planeNormal->y) * halfExtents.y,
      Math::GetSign(planeNormal->z) * halfExtents.z);

    //First check the box's point that's furthest in the direction of the
    //plane's normal
    Vec3 boxPoint = boxCenter + signs;
    real signedDistance = Geometry::SignedDistanceToPlane(boxPoint, 
      *planeNormal,
      planeDistance);
    if(signedDistance < real(0.0))
    {
      return Outside;
    }
  }
  return inOrOut;
}

///Intersect a frustum with an oriented bounding box. The 6 planes of the 
///frustum are assumed to be pointing inwards.
Type FrustumObbApproximation(const Vec4 frustumPlanes[6], Vec3Param obbCenter, 
                             Vec3Param obbHalfExtents, Mat3Param obbBasis, 
                             Manifold* manifold)
{
  Vec3 obbBasisVectors[3] = { obbBasis.GetBasis(0),
                              obbBasis.GetBasis(1),
                              obbBasis.GetBasis(2) };
  real intersectionIndex = real(-1.0);
  const Vec3* planeNormal = nullptr;

  for(uint i = 0; i < 6; ++i)
  {
    planeNormal = reinterpret_cast<const Vec3*>(&(frustumPlanes[i].x));
    real planeDistance = frustumPlanes[i][3];

    //Compute the projection interval radius of box onto L(t) = b.c + t * p.n
    real radius  = obbHalfExtents[0] * 
                   Math::Abs(Dot(*planeNormal, obbBasisVectors[0]));
         radius += obbHalfExtents[1] * 
                   Math::Abs(Dot(*planeNormal, obbBasisVectors[1]));
         radius += obbHalfExtents[2] *  
                   Math::Abs(Dot(*planeNormal, obbBasisVectors[2]));
    
    //Compute the signed distance of the box's center from the plane
    real signedDistance = Geometry::SignedDistanceToPlane(obbCenter, 
                                                          *planeNormal, 
                                                          planeDistance);

    //Intersection does not occur when the signed distance falls outside the 
    //[-radius, +radius] interval
    if(Math::Abs(signedDistance) > radius)
    {
      if(Math::IsNegative(signedDistance))
      {
        return Outside;
      }
      //Not sure if it's inside all of the frustum's planes, keep going.
      else
      {
        continue;
      }
    }
    else
    {
      intersectionIndex = static_cast<real>(i);
    }
  }

  if(intersectionIndex != real(-1.0))
  {
    if(manifold != nullptr)
    {
      manifold->PointAt(0).Depth = intersectionIndex;
    }
    return Other;
  }
  return Inside;
}

///Intersect a frustum with a plane. The 6 planes of the frustum are assumed to
///be pointing inwards.
Type FrustumPlane(const Vec4 frustumPlanes[6], Vec3Param planeNormal, 
                  real planeDistance, Manifold* manifold)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  if(manifold != nullptr)
  {
    //
  }
  return Unimplemented;
}

///Intersect a frustum with a sphere. The 6 planes of the frustum are assumed to
///be pointing inwards.
Type FrustumSphereApproximation(const Vec4 frustumPlanes[6], Vec3Param sphereCenter, 
                                real sphereRadius, Manifold* manifold)
{
  real intersectionIndex = real(-1.0);
  const Vec3* planeNormal = nullptr;

  for(uint i = 0; i < 6; ++i)
  {
    planeNormal = reinterpret_cast<const Vec3*>(&(frustumPlanes[i].x));
    real planeDistance = frustumPlanes[i][3];

    //Get the closest point on the plane to the sphere's center.
    Vec3 closestPoint = sphereCenter;
    Type result = ClosestPointOnPlaneToPoint(*planeNormal, planeDistance, 
                                             &closestPoint);

    //Vector from closest point on plane to sphere's center
    Vec3 v = sphereCenter - closestPoint;

    if(LengthSq(v) > (sphereRadius * sphereRadius))
    {
      //If no intersection and sphere is outside this plane (opposite side of 
      //the normal) then sphere is not inside or touching the frustum.
      if(Outside == result)
      {
        return Outside;
      }

      //Otherwise the sphere is completely inside this plane, go to next plane.
      else
      {
        continue;
      }
    }
    //The sphere is intersecting the plane, but we're not sure that it's
    //intersecting inside the frustum. Save off this plane's index for later.
    else
    {
      intersectionIndex = static_cast<real>(i);
    }
  }

  if(intersectionIndex != real(-1.0))
  {
    if(manifold != nullptr)
    {
      manifold->PointAt(0).Depth = intersectionIndex;
    }
    return Other;
  }

  //If the sphere has not intersected any of the frustum's planes, and it got
  //this far, it's completely inside the frustum.
  return Inside;
}

///Intersect a frustum with a triangle. The 6 planes of the frustum are assumed 
///to be pointing inwards.
Type FrustumTriangle(const Vec4 frustumPlanes[6], Vec3Param trianglePointA, 
                     Vec3Param trianglePointB, Vec3Param trianglePointC, 
                     Manifold* manifold)
{
  uint intersectionIndex = uint(-1);
  const Vec3* planeNormal = nullptr;

  for(uint i = 0; i < 6; ++i)
  {
    planeNormal = reinterpret_cast<const Vec3*>(&(frustumPlanes[i].x));
    real a = Geometry::SignedDistanceToPlane(trianglePointA, *planeNormal,
                                             frustumPlanes[i][3]);
    real b = Geometry::SignedDistanceToPlane(trianglePointB, *planeNormal,
                                             frustumPlanes[i][3]);
    real c = Geometry::SignedDistanceToPlane(trianglePointC, *planeNormal,
                                             frustumPlanes[i][3]);
    
    //Add up the signs (negative = -1, positive = 1) and check to see if the 
    //points are all on the same side or if the triangle intersects the plane.
    real result = Math::GetSign(a) + Math::GetSign(b) + Math::GetSign(c);

    //If all three signs are the same, the absolute value of the sum will be 3.
    if(Math::Abs(result) == real(3.0))
    {
      //If the sign of the result is negative, the triangle is outside the plane
      if(result < real(0.0))
      {
        return Outside;
      }
      //Otherwise the triangle is completely inside of the plane, do other tests
      else
      {
        continue;
      }
    }
    //The triangle is intersecting the plane, save this plane's index off
    else
    {
      intersectionIndex = i;
    }
  }

  //If there was an intersection, return "Other"
  if(intersectionIndex != uint(-1))
  {
    if(manifold != nullptr)
    {
      manifold->PointAt(0).Depth = real(intersectionIndex);
    }
    return Other;
  }
  return Inside;
}

}// namespace Intersection
