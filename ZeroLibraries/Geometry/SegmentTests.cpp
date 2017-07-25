///////////////////////////////////////////////////////////////////////////////
///
///  \file SegmentTests.cpp
///  Source for all of the line segment intersection tests.
/// 
///  Authors: Benjamin Strukus
///  Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Geometry/Intersection.hpp"
#include "Geometry/Geometry.hpp"
#include "Math/Numerical.hpp"
#include "Math/Math.hpp"

namespace Intersection
{

namespace
{
const real cAabbSegmentEpsilon = real(0.0001);

void MakeBasisFromY(Vec3Param yAxis, Mat3Ptr basis)
{
  Vec3 xAxis, zAxis;
  Math::GenerateOrthonormalBasis(yAxis, &xAxis, &zAxis);
  basis->SetBasis(0, xAxis);
  basis->SetBasis(1, yAxis);
  basis->SetBasis(2, zAxis);
}

}// namespace


//----------------------------------------------------- Segment Tests (Interval)

//Intersect a segment with an axis aligned bounding box.
Type SegmentAabb(Vec3Param segmentStart, Vec3Param segmentEnd, 
                 Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint, 
                 Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  return Unimplemented;
}

//Intersect a segment with a capsule defined by its center, local axes, radius,
//and half of the distance between the centers of the spherical endcaps.
Type SegmentCapsule(Vec3Param segmentStart, Vec3Param segmentEnd,
                    Vec3Param capsuleCenter, Mat3Param capsuleBasis, 
                    real capsuleRadius, real capsuleSegmentHalfLength, 
                    Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  return Unimplemented;
}

//Intersect a segment with a capsule defined by the centers of the spherical 
//endcaps and the radius.
Type SegmentCapsule(Vec3Param segmentStart, Vec3Param segmentEnd,
                    Vec3Param capsulePointA, Vec3Param capsulePointB, 
                    real capsuleRadius, Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  Vec3 yAxis = capsulePointB - capsulePointA;
  real halfLength = real(0.5) * Normalize(yAxis);
  Vec3 center = capsulePointA + (halfLength * yAxis);

  Mat3 basis;
  MakeBasisFromY(yAxis, &basis);

  return SegmentCapsule(segmentStart, segmentEnd, center, basis, capsuleRadius,
                        halfLength, interval);
}

//Intersect a segment with a cylinder defined by its center, local axes, 
//radius, and half height.
Type SegmentCylinder(Vec3Param segmentStart, Vec3Param segmentEnd,
                     Vec3Param cylinderCenter, Mat3Param cylinderBasis, 
                     real cylinderRadius, real cylinderHalfHeight, 
                     Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  return Unimplemented;
}

//Intersect a segment with a cylinder defined by the points at the planar 
//endcap and the radius.
Type SegmentCylinder(Vec3Param segmentStart, Vec3Param segmentEnd,
                     Vec3Param cylinderPointA, Vec3Param cylinderPointB, 
                     real cylinderRadius, Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  Vec3 yAxis = cylinderPointB - cylinderPointA;
  real halfLength = real(0.5) * Normalize(yAxis);
  Vec3 center = cylinderPointA + (halfLength * yAxis);

  Mat3 basis;
  MakeBasisFromY(yAxis, &basis);

  return SegmentCylinder(segmentStart, segmentEnd, center, basis, 
                         cylinderRadius, halfLength, interval);
}

//Intersect a segment with an elliptical cylinder defined by its center, local
//axes, major radius (x-axis), minor radius (z-axis), and half height (y-axis).
Type SegmentCylinder(Vec3Param segmentStart, Vec3Param segmentEnd,
                     Vec3Param cylinderCenter, Mat3Param cylinderBasis, 
                     real cylinderMajorRadius, real cylinderMinorRadius, 
                     real cylinderHalfHeight, Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  return Unimplemented;
}

//Intersect a segment with an ellipsoid.
Type SegmentEllipsoid(Vec3Param segmentStart, Vec3Param segmentEnd,
                      Vec3Param ellipsoidCenter, Vec3Param ellipsoidRadii,
                      Mat3Param ellipsoidBasis, Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  return Unimplemented;
}

//Intersect a segment with a plane.
Type SegmentPlane(Vec3Param segmentStart, Vec3Param segmentEnd, 
                  Vec3Param planeNormal, real planeDistance, Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  return Unimplemented;
}

//Intersect a segment with an oriented bounding box.
Type SegmentObb(Vec3Param segmentStart, Vec3Param segmentEnd, 
                Vec3Param obbCenter, Vec3Param obbHalfExtents, 
                Mat3Param obbBasis, Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  return Unimplemented;
}

//Intersect a segment with a sphere.
Type SegmentSphere(Vec3Param segmentStart, Vec3Param segmentEnd,
                   Vec3Param sphereCenter, real sphereRadius, 
                   Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  return Unimplemented;
}

//Intersect a segment with a torus.
Type SegmentTorus(Vec3Param segmentStart, Vec3Param segmentEnd,
                  Vec3Param torusCenter, Mat3Param torusBasis, 
                  real torusRingRadius, real torusTubeRadius, 
                  Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  return Unimplemented;
}

//Intersect a segment with a triangle.
Type SegmentTriangle(Vec3Param segmentStart, Vec3Param segmentEnd,
                     Vec3Param trianglePointA, Vec3Param trianglePointB, 
                     Vec3Param trianglePointC, Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  return Unimplemented;
}


//-------------------------------------------------------- Segment Tests (Point)

//Intersect a segment with an axis aligned bounding box.
Type SegmentAabb(Vec3Param segmentStart, Vec3Param segmentEnd, 
                 Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint, 
                 IntersectionPoint* intersectionPoint)
{
  ErrorIf((aabbMinPoint.x > aabbMaxPoint.x) ||
          (aabbMinPoint.y > aabbMaxPoint.y) ||
          (aabbMinPoint.z > aabbMaxPoint.z),
          "Intersection - Axis-aligned bounding box's minimum point is greater"\
          " than it's maximum point.");

  //Box center point
  Vec3 aabbCenter = real(0.5) * (aabbMaxPoint + aabbMinPoint);

  //Box half extents
  Vec3 aabbHalfExtents = aabbMaxPoint - aabbCenter;

  //Use the more detailed version if intersection information is desired
  if(intersectionPoint != nullptr)
  {
    Vec3 segment = segmentEnd - segmentStart;
    real tMin = -Math::PositiveMax();
    real tMax = Math::PositiveMax();

    //Move the box as if the segment's starting point was the origin
    Vec3 startToCenter = aabbCenter - segmentStart;
    for(int i = 0; i < 3; ++i)
    {
      if(Math::IsZero(segment[i]))
      {
        if((segmentStart[i] < aabbMinPoint[i]) || 
           (segmentStart[i] > aabbMaxPoint[i]))
        {
          return None;
        }
      }
      else
      {
        real signedExtent = Math::GetSign(segment[i]) * aabbHalfExtents[i];
        real divisor = real(1.0) / segment[i];

        real s = (startToCenter[i] - signedExtent) * divisor;
        if(s > tMin)
        {
          tMin = s;
        }

        s = (startToCenter[i] + signedExtent) * divisor;
        if(s < tMax)
        {
          tMax = s;
        }
        if(tMin > tMax)
        {
          return None;
        }
      }
    }

    //If t is negative, segment started inside AABB
    if(tMin < real(0.0))
    {
      //Intersection must happen between endpoints of the segment
      if(Math::InRange(tMax, real(0.0), real(1.0)))
      {
        intersectionPoint->Points[0] = segmentStart;
        intersectionPoint->Points[0] = Vector3::MultiplyAdd(intersectionPoint->Points[0], segment, tMax);
        intersectionPoint->Points[1] = intersectionPoint->Points[0];
        intersectionPoint->T = tMax;
        return Point;
      }
      else
      {
        return Inside;
      }
    }

    if(Math::InRange(tMin, real(0.0), real(1.0)))
    {
      if(tMax > real(1.0))
      {
        intersectionPoint->Points[0] = segmentStart;
        intersectionPoint->Points[0] = Vector3::MultiplyAdd(intersectionPoint->Points[0], segment, tMin);
        intersectionPoint->Points[1] = intersectionPoint->Points[0];
        intersectionPoint->T = tMin;
        return Point;
      }

      intersectionPoint->Points[0] = segmentStart;
      intersectionPoint->Points[0] = Vector3::MultiplyAdd(intersectionPoint->Points[0], segment, tMin);
      intersectionPoint->Points[1] = segmentStart;
      intersectionPoint->Points[1] = Vector3::MultiplyAdd(intersectionPoint->Points[1], segment, tMax);
      intersectionPoint->T = tMin;
      return Segment;
    }
    return None;
  }

  //Use the quicker version if only a boolean test is desired
  //Segment midpoint
  Vec3 segmentMidpoint = real(0.5) * (segmentStart + segmentEnd);

  //Segment halflength vector
  Vec3 segmentHalflength = segmentEnd - segmentMidpoint;

  //Translate box and segment to origin
  segmentMidpoint -= aabbCenter;

  //Try world coordinates as separating axes
  real absSegmentX = Math::Abs(segmentHalflength.x);
  if(Math::Abs(segmentMidpoint.x) > (aabbHalfExtents.x + absSegmentX))
  {
    return None;
  }

  real absSegmentY = Math::Abs(segmentHalflength.y);
  if(Math::Abs(segmentMidpoint.y) > (aabbHalfExtents.y + absSegmentY))
  {
    return None;
  }

  real absSegmentZ = Math::Abs(segmentHalflength.z);
  if(Math::Abs(segmentMidpoint.z) > (aabbHalfExtents.z + absSegmentZ))
  {
    return None;
  }

  //Add in an epsilon term to counteract arithmetic errors when segment is 
  //(near) parallel to a coordinate axis
  absSegmentX += cAabbSegmentEpsilon;
  absSegmentY += cAabbSegmentEpsilon;
  absSegmentZ += cAabbSegmentEpsilon;

  //Try cross products of segment direction vector with coordinate axes
  real cross = Math::Abs(segmentMidpoint.y * segmentHalflength.z - 
                         segmentMidpoint.z * segmentHalflength.y);
  real proj = aabbHalfExtents.y * absSegmentZ + aabbHalfExtents.z * absSegmentY;
  if(cross > proj)
  {
    return None;
  }

  cross = Math::Abs(segmentMidpoint.z * segmentHalflength.x - 
                    segmentMidpoint.x * segmentHalflength.z);
  proj = aabbHalfExtents.x * absSegmentZ + aabbHalfExtents.z * absSegmentX;
  if(cross > proj)
  {
    return None;
  }

  cross = Math::Abs(segmentMidpoint.x * segmentHalflength.y - 
                    segmentMidpoint.y * segmentHalflength.x);
  proj = aabbHalfExtents.x * absSegmentY + aabbHalfExtents.y * absSegmentX;
  if(cross > proj)
  {
    return None;
  }

  //No separating axis found; segment must be overlapping AABB
  return Other;
}

//Intersect a segment with a capsule.
Type SegmentCapsule(Vec3Param segmentStart, Vec3Param segmentEnd, 
                    Vec3Param capsulePointA, Vec3Param capsulePointB, 
                    real capsuleRadius, IntersectionPoint* intersectionPoint)
{
  //This isn't the "fastest" code that can detect intersection with a segment 
  //and a capsule, but it works and makes the most sense (to me)
  real t[5][2] = { { -Math::PositiveMax(),  Math::PositiveMax() },    //Infinity
                   { -Math::PositiveMax(),  Math::PositiveMax() },    //Infinity
                   {  Math::PositiveMax(), -Math::PositiveMax() },    //Invalid
                   {  Math::PositiveMax(), -Math::PositiveMax() },    //Invalid
                   {  Math::PositiveMax(), -Math::PositiveMax() }  }; //Invalid

  Vec3 segment = segmentEnd - segmentStart;
  real segLen = Normalize(segment);
  Vec3 normal = Normalized(capsulePointA - capsulePointB);

  //-------------------------------------------------------------------- Plane A
  real dirDotNormal = Dot(segment, normal);

  //First intersection is the plane, second is infinity
  if(dirDotNormal < real(0.0))
  {
    real planeDistance = Dot(capsulePointA, normal);
    t[0][0] = (planeDistance - Dot(segmentStart, normal)) / dirDotNormal;
  }
  //First intersection is negative infinity, second is the plane
  else if(dirDotNormal > real(0.0))
  {
    real planeDistance = Dot(capsulePointA, normal);
    t[0][1] = (planeDistance - Dot(segmentStart, normal)) / dirDotNormal;
  }

  //-------------------------------------------------------------------- Plane B
  Negate(&normal);
  dirDotNormal = Dot(segment, normal);

  //First intersection is the plane, second is infinity
  if(dirDotNormal < real(0.0))
  {
    real planeDistance = Dot(capsulePointB, normal);
    t[1][0] = (planeDistance - Dot(segmentStart, normal)) / dirDotNormal;
  }
  //First intersection is negative infinity, second is the plane
  else if(dirDotNormal > real(0.0))
  {
    real planeDistance = Dot(capsulePointB, normal);
    t[1][1] = (planeDistance - Dot(segmentStart, normal)) / dirDotNormal;
  }

  //---------------------------------------------------------- Infinite Cylinder
  real radiusSq = Math::Sq(capsuleRadius);
  {
    Vec3 relStart = segmentStart - capsulePointA;
    real startDotNormal = Dot(relStart, normal);
    real a = real(1.0) - Math::Sq(dirDotNormal);
    real b = real(2.0) * (Dot(relStart, segment) - 
                          startDotNormal * dirDotNormal);
    real c = (LengthSq(relStart) - radiusSq) - Math::Sq(startDotNormal);

    real discr = b * b - real(4.0) * a * c;
    if(discr >= real(0.0))
    {
      discr = Math::Sqrt(discr);
      t[2][0] = (-b - discr) / (real(2.0) * a);
      t[2][1] = (-b + discr) / (real(2.0) * a);
    }
    else
    {
      return None;
    }
  }

  //------------------------------------------------- Compare Cylinder Intervals
  real tMin = Math::Max(Math::Max(t[0][0], t[1][0]), t[2][0]);
  real tMax = Math::Min(Math::Min(t[0][1], t[1][1]), t[2][1]);

  //Ray completely missed the capsule's finite cylinder, but it could still hit 
  //one of the spherical end caps. Invalidate the finite cylinder's interval so 
  //it doesn't affect the results of the sphere intersections
  if(tMin > tMax)
  {
    tMin = Math::PositiveMax();
    tMax = -Math::PositiveMax();
  }

  //------------------------------------------------------------------- Sphere A
  {
    Vec3 centerToPoint = segmentStart - capsulePointA;
    real b = real(2.0) * Dot(segment, centerToPoint);
    real c = LengthSq(centerToPoint) - radiusSq;
    real discr = b * b - real(4.0) * c;
    
    //A negative discriminant corresponds to the ray missing the sphere
    if(discr >= real(0.0))
    {
      //Ray now found to intersect sphere, compute smallest t-value of 
      //intersection
      discr = Math::Sqrt(discr);

      t[3][0] = (-b - discr) / real(2.0);
      t[3][1] = (-b + discr) / real(2.0);
    }
    if(t[3][0] < t[3][1])
    {
      tMin = Math::Min(tMin, t[3][0]);
      tMax = Math::Max(tMax, t[3][1]);
    }
  }

  //------------------------------------------------------------------- Sphere B
  {
    Vec3 pc = segmentStart - capsulePointB;
    real b = real(2.0) * Dot(segment, pc);
    real c = LengthSq(pc) - radiusSq;
    real discr = b * b - real(4.0) * c;
    
    //A negative discriminant corresponds to the ray missing the sphere
    if(discr >= real(0.0))
    {
      //Ray now found to intersect sphere, compute smallest t-value of
      //intersection
      discr = Math::Sqrt(discr);

      t[4][0] = (-b - discr) / real(2.0);
      t[4][1] = (-b + discr) / real(2.0);
    }
    if(t[4][0] < t[4][1])
    {
      tMin = Math::Min(tMin, t[4][0]);
      tMax = Math::Max(tMax, t[4][1]);
    }
  }

  //------------------------------------------------ Compute Intersection Points
  if(tMin > tMax)
  {
    return None;
  }

  //Minimum point of intersection lies on the segment
  if(Math::InRange(tMin, real(0.0), segLen))
  {
    //Maximum point of intersection lies on the segment
    if(Math::InRange(tMax, real(0.0), segLen))
    {
      if(intersectionPoint != nullptr)
      {
        intersectionPoint->Points[0] = segmentStart + tMin * segment;
        intersectionPoint->Points[1] = segmentStart + tMax * segment;
        intersectionPoint->T = tMin / segLen;
      }
      return Segment;
    }

    if(intersectionPoint != nullptr)
    {
      intersectionPoint->Points[0] = segmentStart + tMin * segment;
      intersectionPoint->Points[1] = intersectionPoint->Points[0];
      intersectionPoint->T = tMin / segLen;
    }
    return Point;
  }

  //Otherwise, both are negative and there is no intersection
  return None;
}

//Intersect a segment with a cylinder.
Type SegmentCylinder(Vec3Param segmentStart, Vec3Param segmentEnd,
                     Vec3Param cylinderPointA, Vec3Param cylinderPointB,
                     real cylinderRadius, IntersectionPoint* intersectionPoint)
{
  //This isn't the "fastest" code that can detect intersection with a segment 
  //and a cylinder, but it works and makes the most sense (to me)
  real t[3][2] = { { -Math::PositiveMax(),  Math::PositiveMax() },    //Infinity
                   { -Math::PositiveMax(),  Math::PositiveMax() },    //Infinity
                   {  Math::PositiveMax(), -Math::PositiveMax() } };  //Invalid

  Vec3 segment = segmentEnd - segmentStart;
  real segLen = Normalize(segment);
  Vec3 normal = Normalized(cylinderPointA - cylinderPointB);

  //-------------------------------------------------------------------- Plane A
  real dirDotNormal = Dot(segment, normal);

  //First intersection is the plane, second is infinity
  if(dirDotNormal < real(0.0))
  {
    real planeDistance = Dot(cylinderPointA, normal);
    t[0][0] = (planeDistance - Dot(segmentStart, normal)) / dirDotNormal;
  }
  //First intersection is negative infinity, second is the plane
  else if(dirDotNormal > real(0.0))
  {
    real planeDistance = Dot(cylinderPointA, normal);
    t[0][1] = (planeDistance - Dot(segmentStart, normal)) / dirDotNormal;
  }
  //Ray's direction is perpendicular to plane, if ray's origin lies outside of 
  //the plane then there is no intersection with the ray and the plane (as well
  //as the cylinder)
  else if((Dot(cylinderPointA, normal) - Dot(segmentStart, normal)) < real(0.0))
  {
    return None;
  }

  //-------------------------------------------------------------------- Plane B
  Negate(&normal);
  dirDotNormal = Dot(segment, normal);

  //First intersection is the plane, second is infinity
  if(dirDotNormal < real(0.0))
  {
    real planeDistance = Dot(cylinderPointB, normal);
    t[1][0] = (planeDistance - Dot(segmentStart, normal)) / dirDotNormal;
  }
  //First intersection is negative infinity, second is the plane
  else if(dirDotNormal > real(0.0))
  {
    real planeDistance = Dot(cylinderPointB, normal);
    t[1][1] = (planeDistance - Dot(segmentStart, normal)) / dirDotNormal;
  }
  //Ray's direction is perpendicular to plane, if ray's origin lies outside of 
  //the plane then there is no intersection with the ray and the plane (as well
  //as the cylinder)
  else if((Dot(cylinderPointB, normal) - Dot(segmentStart, normal)) < real(0.0))
  {
    return None;
  }

  //---------------------------------------------------------- Infinite Cylinder
  Vec3 relStart = segmentStart - cylinderPointA;
  real startDotNormal = Dot(relStart, normal);
  real a = real(1.0) - Math::Sq(dirDotNormal);
  real b = real(2.0) * (Dot(relStart, segment) - 
                        startDotNormal * dirDotNormal);
  real c = (LengthSq(relStart) - Math::Sq(cylinderRadius)) - 
           Math::Sq(startDotNormal);

  real discr = b * b - real(4.0) * a * c;
  if(discr >= real(0.0))
  {
    discr = Math::Sqrt(discr);
    t[2][0] = (-b - discr) / (real(2.0) * a);
    t[2][1] = (-b + discr) / (real(2.0) * a);
  }
  else
  {
    return None;
  }

  //---------------------------------------------------------- Compare Intervals
  real tMin = Math::Max(Math::Max(t[0][0], t[1][0]), t[2][0]);
  real tMax = Math::Min(Math::Min(t[0][1], t[1][1]), t[2][1]);
  if(tMin > tMax)
  {
    return None;
  }

  //Minimum point of intersection lies on the segment
  if(Math::InRange(tMin, real(0.0), segLen))
  {
    //Maximum point of intersection lies on the segment
    if(Math::InRange(tMax, real(0.0), segLen))
    {
      if(intersectionPoint != nullptr)
      {
        intersectionPoint->Points[0] = segmentStart + tMin * segment;
        intersectionPoint->Points[1] = segmentStart + tMax * segment;
        intersectionPoint->T = tMin / segLen;
      }
      return Segment;
    }

    if(intersectionPoint != nullptr)
    {
      intersectionPoint->Points[0] = segmentStart + tMin * segment;
      intersectionPoint->Points[1] = intersectionPoint->Points[0];
      intersectionPoint->T = tMin / segLen;
    }
    return Point;
  }

  //Otherwise, both are negative and there is no intersection
  return None;
}

///Intersect a segment with an ellipsoid.
Type SegmentEllipsoid(Vec3Param segmentStart, Vec3Param segmentEnd, 
                      Vec3Param ellipsoidCenter, Vec3Param ellipsoidRadii, 
                      Mat3Param ellipsoidBasis, 
                      IntersectionPoint* intersectionPoint)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");

  if(intersectionPoint != nullptr)
  {
    //
  }
  return Unimplemented;
}

//Intersect a segment with a plane.
Type SegmentPlane(Vec3Param segmentStart, Vec3Param segmentEnd, 
                  Vec3Param planeNormal, real planeDistance, 
                  IntersectionPoint* intersectionPoint)
{
  //Compute the t value for the directed line ab intersecting the plane
  Vec3 segment = segmentEnd - segmentStart;

  real t = Dot(planeNormal, segment);
  if(Math::Abs(t) < real(0.00001))
  {
    return None;
  }

  t = (planeDistance - Dot(planeNormal, segmentStart)) / t;

  //If t in [0..1] compute and return intersection point
  if(Math::InRange(t, real(0.0), real(1.0)))
  {
    if(intersectionPoint != nullptr)
    {
      intersectionPoint->Points[0] = segmentStart;
      intersectionPoint->Points[0] = Vector3::MultiplyAdd(intersectionPoint->Points[0], segment, t);
      intersectionPoint->T = t;
    }
    return Point;
  }
  return None;
}

//Intersect a segment with an oriented bounding box.
Type SegmentObb(Vec3Param segmentStart, Vec3Param segmentEnd, 
                Vec3Param obbCenter, Vec3Param obbHalfExtents, 
                Mat3Param obbBasis, IntersectionPoint* intersectionPoint)
{
  //Take everything into the box's body space and use the ray-aabb function
  Vec3 segStart = segmentStart - obbCenter;
  Vec3 segEnd = segmentEnd - obbCenter;
  Math::TransposedTransform(obbBasis, &segStart);
  Math::TransposedTransform(obbBasis, &segEnd);
  Type result = SegmentAabb(segStart, segEnd, -obbHalfExtents, obbHalfExtents, 
                            intersectionPoint);

  if(intersectionPoint != nullptr)
  {
    //Transform the intersection information back into world space
    Math::Transform(obbBasis, &(intersectionPoint->Points[0]));
    intersectionPoint->Points[0] += obbCenter;
    if(result == Segment)
    {
      //If there were two points of intersection, transform both
      Math::Transform(obbBasis, &(intersectionPoint->Points[1]));
      intersectionPoint->Points[1] += obbCenter;
    }
    else
    {
      intersectionPoint->Points[1] = intersectionPoint->Points[0];
    }
  }
  return result;
}

//Intersect a segment with a sphere.
Type SegmentSphere(Vec3Param segmentStart, Vec3Param segmentEnd, 
                   Vec3Param sphereCenter, real sphereRadius, 
                   IntersectionPoint* intersectionPoint)
{
  Vec3 m = segmentStart - sphereCenter;
  Vec3 d = segmentEnd - segmentStart;
  real segmentLength = Normalize(d);
  real c = Dot(m, m) - sphereRadius * sphereRadius;

  real b = Dot(m, d);
  //Exit if the segment's start is outside the sphere (c > 0) and the segment's 
  //direction is pointing away from the sphere (b > 0)
  if(c > real(0.0) && b > real(0.0))
  {
    return None;
  }

  real discr = b * b - c;

  //A negative discriminant corresponds to the segment missing the sphere
  if(discr < real(0.0))
  {
    return None;
  }

  //If discriminant = 0 (or close to 0 in our case), the segment might intersect
  //the sphere tangentially
  if(Math::IsZero(discr))
  {
    //Intersection must happen between endpoints of the segment
    real t = -b;
    if(Math::InRange(t, real(0.0), segmentLength))
    {
      if(intersectionPoint != nullptr)
      {
        intersectionPoint->Points[0] = segmentStart;
        intersectionPoint->Points[0] = Vector3::MultiplyAdd(intersectionPoint->Points[0], d, t);
        intersectionPoint->Points[1] = intersectionPoint->Points[0];
        intersectionPoint->T = t / segmentLength;
      }
      return Point;
    }
    return None;
  }

  //Segment now might intersect sphere, compute smallest value of intersection
  real sqrtDiscr = Math::Sqrt(discr);
  real t0 = -b - sqrtDiscr;
  real t1 = -b + sqrtDiscr;

  //If t is negative, ray started inside sphere
  if(t0 < real(0.0))
  {
    //Intersection must happen between endpoints of the segment
    if(Math::InRange(t1, real(0.0), segmentLength))
    {
      if(intersectionPoint != nullptr)
      {
        intersectionPoint->Points[0] = segmentStart;
        intersectionPoint->Points[0] = Vector3::MultiplyAdd(intersectionPoint->Points[0], d, t1);
        intersectionPoint->Points[1] = intersectionPoint->Points[0];
        intersectionPoint->T = t1 / segmentLength;
      }
      return Point;
    }
    else
    {
      //give back the start and end as the intersection
      intersectionPoint->Points[0] = segmentStart;
      intersectionPoint->Points[1] = segmentEnd;
      intersectionPoint->T = real(0.0);
      return Inside;
    }
  }

  if(Math::InRange(t0, real(0.0), segmentLength))
  {
    //Segment starts outside of sphere and ends inside of sphere
    if(t1 > real(segmentLength))
    {
      if(intersectionPoint != nullptr)
      {
        intersectionPoint->Points[0] = segmentStart;
        intersectionPoint->Points[0] = Vector3::MultiplyAdd(intersectionPoint->Points[0], d, t0);
        intersectionPoint->Points[1] = intersectionPoint->Points[0];
        intersectionPoint->T = t0 / segmentLength;
      } 
      return Point;
    }

    //Segment starts and ends outside of sphere as well as intersecting it
    if(intersectionPoint != nullptr)
    {
      intersectionPoint->Points[0] = segmentStart;
      intersectionPoint->Points[0] = Vector3::MultiplyAdd(intersectionPoint->Points[0], d, t0);
      intersectionPoint->Points[1] = segmentStart;
      intersectionPoint->Points[1] = Vector3::MultiplyAdd(intersectionPoint->Points[1], d, t1);
      intersectionPoint->T = t0 / segmentLength;
    }  
    return Segment;
  }
  return None;
}

//Intersect a segment with a triangle.
Type SegmentTriangle(Vec3Param segmentStart, Vec3Param segmentEnd,
                     Vec3Param trianglePointA, Vec3Param trianglePointB,
                     Vec3Param trianglePointC, 
                     IntersectionPoint* intersectionPoint)
{
  Vec3 aToB = trianglePointB - trianglePointA;
  Vec3 aToC = trianglePointC - trianglePointA;

  //Compute triangle normal. Can be precalculated or cached if intersecting
  //multiple rays against the same triangle.
  Vec3 normal = Cross(aToB, aToC);
  Normalize(normal);

  Vec3 segmentDirection = segmentEnd - segmentStart;
  real planeDistance = Dot(normal, trianglePointA);
  real frontOrBehind = planeDistance - Dot(segmentStart, normal);
  real relativeDirection = Dot(segmentDirection, normal);

  //Ray's direction and triangle's plane's normal are perpendicular, ray can
  //only intersect plane if ray lies on triangle's plane. However, I will not 
  //consider this to be considered an intersection due to the unlikelihood of
  //the infinitely thin ray lying on the infinitely thin plane.
  if(Math::Abs(relativeDirection) < real(0.0001))
  {
    return None;
  }

  //Ray might intersect triangle's plane, depends on where ray's starting point
  //and direction are in relation to the triangle's plane
  else
  {
    real t = frontOrBehind / relativeDirection;
    if(t < real(0.0) || t > real(1.0))
    {
      return None;
    }

    Vec3 pointOnPlane = segmentStart + t * segmentDirection;
    Type result = PointTriangle(pointOnPlane, trianglePointA, 
                                           trianglePointB, trianglePointC);
    if(result == Inside)
    {
      if(intersectionPoint != nullptr)
      {
        intersectionPoint->Points[0] = pointOnPlane;
        intersectionPoint->T = t;
      }
      return Point;
    }
  }
  return None;
}
} // namespace Intersection
