////////////////////////////////////////////////////////////////////////////////
///
/// \file ClosestPoint.cpp
/// Functions to calculate the closest point on the specified object to a given
/// point.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Geometry/Intersection.hpp"
#include "GeometryStandard.hpp"
#include "Math/Math.hpp"

namespace Intersection
{
namespace
{
const real cSegmentSegmentZero = real(0.000001);
}

//Find the closest point on a ray to the given point.
Type ClosestPointOnRayToPoint(Vec3Param rayStart, Vec3Param rayDirection, 
                              Vec3Ptr point)
{
  ErrorIf(point == nullptr, "Intersection - Null pointer passed, this function"\
                         " needs a valid pointer.");

  //Project p onto ab, computing parameterized position d(t) = a + t * (b - a)
  real t = Dot(*point - rayStart, rayDirection);
  if(t <= real(0.0))
  {
    //c projects outside the [a,inf] interval, on the a side; clamp to a
    *point = rayStart;
    return Outside;
  }
  else
  {
    //This variable is now being treated as the closest point
    *point = rayStart;
    *point = Math::MultiplyAdd(*point, rayDirection, t);
    return Inside;
  }
}

//Find the closest point on a line segment to the given point.
Type ClosestPointOnSegmentToPoint(Vec3Param segmentPointA, 
                                  Vec3Param segmentPointB, Vec3Ptr point)
{
  ErrorIf(point == nullptr, "Intersection - Null pointer passed, this function"\
                         " needs a valid pointer.");

  Vec3 lineSegment = segmentPointB - segmentPointA;
  //Project c onto ab, computing parameterized position d(t) = a + t * (b - a)
  real t = Dot(*point - segmentPointA, lineSegment);
  if(t <= real(0.0))
  {
    //c projects outside the [a,b] interval, on the a side; clamp to a
    *point = segmentPointA;
    return Outside;
  }
  else
  {
    real denom = Dot(lineSegment, lineSegment);
    if(t >= denom)
    {
      //c projects outside the [a,b] interval, on the b side; clamp to b
      *point = segmentPointB;
      return Outside;
    }
    else
    {
      //c projects inside the [a,b] interval; must do deferred divide now
      t /= denom;

      //This variable is now being treated as the closest point
      lineSegment *= t;
      lineSegment += segmentPointA;
      *point = lineSegment;
      return Inside;
    }
  }
}

//-------------------------------------------------------- Closest Point Queries
//Find the closest point on an axis aligned bounding box to the given point. 
//The point returned will always be on the surface of the axis aligned bounding
//box.
Type ClosestPointOnAabbToPoint(Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint,
                               Vec3Ptr point)
{
  ErrorIf(point == nullptr, "Intersection - Null pointer passed, this function"\
                         " needs a valid pointer.");
  ErrorIf((aabbMinPoint.x > aabbMaxPoint.x) || 
          (aabbMinPoint.y > aabbMaxPoint.y) || 
          (aabbMinPoint.z > aabbMaxPoint.z),
          "Intersection - Axis-aligned bounding box's minimum point is greater"\
          " than it's maximum point.");

  const Type features[3] = { Face, Edge, Point };
  bool isInside = true;
  uint featureTracker = 0;

  //For all of the axes
  for(uint axis = 0; axis < 3; ++axis)
  {
    //Point's value on the current axis
    real pointValue = (*point)[axis];

    //Past the range of the box's minimum value, not inside the box
    if(pointValue <= aabbMinPoint[axis])
    {
      (*point)[axis] = aabbMinPoint[axis];
      isInside = false;
      ++featureTracker;
    }

    //Past the range of the box's maximum value, not inside the box
    else if(pointValue >= aabbMaxPoint[axis])
    {
      (*point)[axis] = aabbMaxPoint[axis];
      isInside = false;
      ++featureTracker;
    }
  }

  //If the point is not inside the box then it is on the surface.
  if(isInside == false)
  {
    return features[featureTracker - 1];
  }

  //----------------------------------------------------------------------------
  //The point is inside of the AABB, push the point out in the direction of the
  //face on the AABB that is closest to the point.
  
  //First get the box's center.
  Vec3 boxToPoint = real(0.5) * (aabbMaxPoint - aabbMinPoint);
  Vec3 aabbHalfExtents = boxToPoint;  
  boxToPoint += aabbMinPoint;
  boxToPoint = *point - boxToPoint;

  //Find the closest AABB face
  real shortestDistance = Math::PositiveMax();
  uint shortestIndex = uint(-1);
  for(uint axis = 0; axis < 3; ++axis)
  {
    for(int sign = -1; sign < 2; sign += 2)
    {
      real distance = boxToPoint[axis] - real(sign) * aabbHalfExtents[axis];
      if(Math::Abs(distance) < Math::Abs(shortestDistance))
      {
        shortestDistance = distance;
        shortestIndex = axis;
      }
    }
  }

  /*
    NOTE:
      If it crashes below this point, then "shortestIndex" was never set, which 
      can only happen if garbage values are passed in.
  */

  //Project the closest point to the closest face
  if(boxToPoint[shortestIndex] < real(0.0))
  {
    (*point)[shortestIndex] = aabbMinPoint[shortestIndex];
  }
  else
  {
    (*point)[shortestIndex] = aabbMaxPoint[shortestIndex];
  }
  return Inside;
}

//Find the closest point on a capsule to the given point. The point returned
//will always be on the surface of the capsule.
Type ClosestPointOnCapsuleToPoint(Vec3Param capsulePointA, 
                                  Vec3Param capsulePointB, real capsuleRadius, 
                                  Vec3Ptr point)
{
  ErrorIf(point == nullptr, "Intersection - Null pointer passed, this function"\
                         " needs a valid pointer.");

  Vec3 closestPoint = *point;
  ClosestPointOnSegmentToPoint(capsulePointA, capsulePointB, &closestPoint);

  //Oddly named but represents the vector from the closest point on the line
  //segment (the one running through the capsule) to the given point
  Vec3 closestPointToPoint = *point - closestPoint;

  //In the case where the point being checked is the same as the closest point
  //on the segment, return the point and consider it to be "Inside" the capsule.
  if(cGeometrySafeChecks)
  {
    real distance = AttemptNormalize(closestPointToPoint);
    if(distance == real(0.0))
    {
      //Create a point on the capsule's surface.
      Vec3 segment = capsulePointB - capsulePointA;
      Vec3 xAxis, zAxis;
      GenerateOrthonormalBasis(segment, &xAxis, &zAxis);

      *point = closestPoint + (xAxis * capsuleRadius);
      return Inside;
    }
    closestPointToPoint *= capsuleRadius;
    closestPoint += closestPointToPoint;
    *point = closestPoint;
    if(distance < capsuleRadius)
    {
      return Inside;
    }
    else
    {
      return Outside;
    }
  }

  real distance = Normalize(closestPointToPoint);
  closestPointToPoint *= capsuleRadius;
  closestPoint += closestPointToPoint;
  *point = closestPoint;
  if(distance < capsuleRadius)
  {
    return Inside;
  }
  else
  {
    return Outside;
  }
}

//Find the closest point on an oriented bounding box to the given point. The
//point returned will always be on the surface of the oriented bounding box.
Type ClosestPointOnObbToPoint(Vec3Param obbCenter, Vec3Param obbHalfExtents,
                              Mat3Param obbBasis, Vec3Ptr point)
{
  ErrorIf(point == nullptr, "Intersection - Null pointer passed, this function"\
                         " needs a valid pointer.");

  *point -= obbCenter;
  Math::TransposedTransform(obbBasis, point);
  Type inOrOut = ClosestPointOnAabbToPoint(-obbHalfExtents, obbHalfExtents, 
                                           point);
  Math::Transform(obbBasis, point);
  *point += obbCenter;
  return inOrOut;
}

//Find the closest point in or on an oriented bounding box to the given point.
//The point returned can be inside or on the surface of the oriented bounding
//box.
Type ClosestPointInObbToPoint(Vec3Param obbCenter, Vec3Param obbHalfExtents,
                              Mat3Param obbBasis, Vec3Ptr point)
{
  ErrorIf(point == nullptr, "Intersection - Null pointer passed, this function"\
                         " needs a valid pointer.");

  Type theType = Inside; 
  Vec3 obbToPoint = *point - obbCenter;
  for(uint i = 0; i < 3; ++i)
  {
    Vec3 axis = obbBasis.GetBasis(i);
    real dot = Dot(obbToPoint, axis);
    real extent = obbHalfExtents[i];

    if(dot > extent)
    {
      *point = Math::MultiplyAdd(*point, axis, obbHalfExtents[i] - dot);
      theType = Outside;
    }
    if(dot < -extent)
    {
      *point = Math::MultiplyAdd(*point, axis, -dot - obbHalfExtents[i]);
      theType = Outside;
    }
  }
  return theType;
}

//Find the closest point in or on an oriented bounding box to the given point.
//The point returned can be inside or on the surface of the oriented bounding
//box.
Type ClosestPointInObbToPoint(Vec3Param obbCenter, Vec3Param obbHalfExtents,
                              const Vec3* obbAxes, Vec3Ptr point)
{
  ErrorIf(point == nullptr, "Intersection - Null pointer passed, this function"\
                         " needs a valid pointer.");

  Type theType = Inside;
  Vec3 obbToPoint = *point - obbCenter;
  for(uint i = 0; i < 3; ++i)
  {
    real dot = Dot(obbToPoint, obbAxes[i]);
    real extent = obbHalfExtents[i];

    if(dot > extent)
    {
      *point = Math::MultiplyAdd(*point, obbAxes[i], obbHalfExtents[i] - dot);
      theType = Outside;
    }
    if(dot < -extent)
    {
      *point = Math::MultiplyAdd(*point, obbAxes[i], -dot - obbHalfExtents[i]);
      theType = Outside;
    }
  }
  return theType;
}

//Find the closest point on a plane to the given point. Assumes that the plane
//normal is normalized.
Type ClosestPointOnPlaneToPoint(Vec3Param planeNormal, real planeDistance,
                                Vec3Ptr point)
{
  ErrorIf(point == nullptr, "Intersection - Null pointer passed, this function"\
                         " needs a valid pointer.");

  real t = Dot(planeNormal, *point) - planeDistance;
  *point = Math::MultiplyAdd(*point, planeNormal, -t);
  if(t < real(0.0))
  {
    return Outside;
  }
  else
  {
    return Inside;
  }
}

//Find the closest point on a sphere to the given point. The point returned 
//will always be on the surface of the sphere.
Type ClosestPointOnSphereToPoint(Vec3Param sphereCenter, real sphereRadius,
                                 Vec3Ptr point)
{
  ErrorIf(point == nullptr, "Intersection - Null pointer passed, this function"\
                         " needs a valid pointer.");

  //Closest point on the sphere to the given point is found by taking the vector
  //from the point to the sphere, normalizing it, and then scaling it by the
  //sphere's radius.
  Vec3 sphereToPoint = *point - sphereCenter;
  real distance = Normalize(sphereToPoint);
  sphereToPoint *= sphereRadius;
  sphereToPoint += sphereCenter;
  *point = sphereToPoint;
  if(distance <= sphereRadius)
  {
    return Inside;
  }
  else
  {
    return Outside;
  }
}

//Find the closest point on a triangle to the given point.
Type ClosestPointOnTriangleToPoint(Vec3Param trianglePointA,
                                   Vec3Param trianglePointB, 
                                   Vec3Param trianglePointC, Vec3Ptr point)
{
  ErrorIf(point == nullptr, "Intersection - Null pointer passed, this function"\
                         " needs a valid pointer.");

  //Check if point is in the vertex region outside A
  Vec3 aToB = trianglePointB - trianglePointA;
  Vec3 aToC = trianglePointC - trianglePointA;
  Vec3 aToPoint = *point - trianglePointA;
  real d1 = Dot(aToB, aToPoint);
  real d2 = Dot(aToC, aToPoint);
  if(d1 <= real(0.0) && d2 <= real(0.0))
  {
    //Closest point has barycentric coordinates (1,0,0)
    *point = trianglePointA;
    return Outside;
  }

  //Check if the point is in the vertex region outside B
  Vec3 bToPoint = *point - trianglePointB;
  real d3 = Dot(aToB, bToPoint);
  real d4 = Dot(aToC, bToPoint);
  if(d3 >= real(0.0) && d4 <= d3)
  {
    //Closest point has barycentric coordinates (0,1,0)
    *point = trianglePointB;
    return Outside;
  }

  //Check if the point is in the edge region of AB, if so return the projection
  //of the point onto AB
  real vc = (d1 * d4) - (d3 * d2);
  if(vc <= real(0.0) && d1 >= real(0.0) && d3 <= real(0.0))
  {
    //Closest point has barycentric coordinates (1-v,v,0)
    real v = d1 / (d1 - d3);
    aToB *= v;
    aToB += trianglePointA;
    *point = aToB;
    return Outside;
  }

  //Check if the point is in the vertex region outside C
  Vec3 cToPoint = *point - trianglePointC;
  real d5 = Dot(aToB, cToPoint);
  real d6 = Dot(aToC, cToPoint);
  if(d6 >= real(0.0) && d5 <= d6)
  {
    //Closest point has barycentric coordinates (0,0,1)
    *point = trianglePointC;
    return Outside;
  }

  //Check if the point is in the edge region of AC, if so return projection of
  //the point onto AC
  real vb = (d5 * d2) - (d1 * d6);
  if(vb <= real(0.0) && d2 >= real(0.0) && d6 <= real(0.0))
  {
    //Closest point has barycentric coordinates (1-w,0,w)
    real w = d2 / (d2 - d6);
    aToC *= w;
    aToC += trianglePointA;
    *point = aToC;
    return Outside;
  }

  //Check if the point is in the edge region of BC, if so return projection of
  //the point onto BC
  real lhs = (d3 * d6) - (d5 * d4);
  if(lhs <= real(0.0) && (d4 - d3) >= real(0.0) && (d5 - d6) >= real(0.0))
  {
    //Closest point has barycentric coordinates (0,1-w,w)
    real w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
    cToPoint  = trianglePointC - trianglePointB;
    cToPoint *= w;
    cToPoint += trianglePointB;
    *point = cToPoint;
    return Outside;
  }

  //The point is inside the face region. Compute the closest point through its
  //barycentric coordinates (u,v,w)
  real denom = real(1.0) / (lhs + vb + vc);
  real v = vb * denom;
  real w = vc * denom;
  aToC *= w;
  aToC = Math::MultiplyAdd(aToC, aToB, v);
  aToC += trianglePointA;
  *point = aToC;
  return Inside;  //= u*a + v*b + w*c, u = va * denom = 1.0 - v - w
}

//Find the closest points between two lines. Assumes that the line directions
//provided are unit length.
Type ClosestPointsOfTwoLines(Vec3Param lineStartA, Vec3Param lineDirectionA, 
                             Vec3Param lineStartB, Vec3Param lineDirectionB, 
                             Vec3Ptr closestPointA, Vec3Ptr closestPointB,
                             Vec2Ptr interpolationValues)
{
  ErrorIf(closestPointA == nullptr, "Intersection - Null pointer passed, this "\
                                   "function needs a valid pointer.");
  ErrorIf(closestPointB == nullptr, "Intersection - Null pointer passed, this "\
                                   "function needs a valid pointer.");

  Vec3 bToA = lineStartA - lineStartB;
  real b = Dot(lineDirectionA, lineDirectionB);
  real c = Dot(lineDirectionA, bToA);
  real f = Dot(lineDirectionB, bToA);
  real d = real(1.0) - (b * b);

  //Parallel lines
  if(Math::IsZero(d))
  {
    *closestPointA = lineStartA;

    real t = f / Dot(bToA, bToA);
    *closestPointB = lineStartB;
    *closestPointB = Math::MultiplyAdd(*closestPointB, lineDirectionB, t);
    return Other;
  }

  real s = (b * f - c) / d;
  *closestPointA = lineStartA;
  *closestPointA = Math::MultiplyAdd(*closestPointA, lineDirectionA, s);

  real t = (f - b * c) / d;
  *closestPointB = lineStartB;
  *closestPointB = Math::MultiplyAdd(*closestPointB, lineDirectionB, t);

  if(interpolationValues != nullptr)
  {
    (*interpolationValues)[0] = s;
    (*interpolationValues)[1] = t;
  }
  return Point;
}

//Find the closest points between two line segments.
Type ClosestPointsOfTwoSegments(Vec3Param segmentOnePointA, 
                                Vec3Param segmentOnePointB, 
                                Vec3Param segmentTwoPointA, 
                                Vec3Param segmentTwoPointB, 
                                Vec3Ptr closestPointA, Vec3Ptr closestPointB)
{
  ErrorIf(closestPointA == nullptr, "Intersection - Null pointer passed, this "\
                                   "function needs a valid pointer.");
  ErrorIf(closestPointB == nullptr, "Intersection - Null pointer passed, this "\
                                   "function needs a valid pointer.");

  //Interpolation values for the segments
  real segmentOneS, segmentTwoT;

  //Direction vector of segment one
  Vec3 segmentOne = segmentOnePointB - segmentOnePointA;

  //Direction vector of segment two
  Vec3 segmentTwo = segmentTwoPointB - segmentTwoPointA;

  //Vector from the start of segment two to the start of segment one
  Vec3 startToStart = segmentOnePointA - segmentTwoPointA;

  //Squared length of segment one, always nonnegative
  real oneSqLength = LengthSq(segmentOne);

  //Squared length of segment two, always nonnegative
  real twoSqLength = LengthSq(segmentTwo);

  //Orientation relation between segment two and the vector from the start of 
  //segment two to the start of segment one
  real twoStartToStart = Dot(segmentTwo, startToStart);

  //Check if either or both segments degenerate into points
  if(oneSqLength <= cSegmentSegmentZero && twoSqLength <= cSegmentSegmentZero)
  {
    //Both segments degenerate into points
    *closestPointA = segmentOnePointA;
    *closestPointB = segmentTwoPointA;
    return Point;
  }
  //First segment degenerates into a point
  if(oneSqLength <= cSegmentSegmentZero)
  {
    segmentOneS = real(0.0);

    //s = 0 => t = (b*s + f) / e = f / e
    segmentTwoT = twoStartToStart / twoSqLength;
    segmentTwoT = Math::Clamp(segmentTwoT, real(0.0), real(1.0));
  }
  else
  {
    real oneStartToStart = Dot(segmentOne, startToStart);

    //Second segment degenerates into a point
    if(twoSqLength <= cSegmentSegmentZero)
    {
      segmentTwoT = real(0.0);

      //t = 0 => s = (b*t - c) / a = -c / a
      segmentOneS = -oneStartToStart / oneSqLength;
      segmentOneS = Math::Clamp(segmentOneS, real(0.0), real(1.0));
    }
    //The general nondegenerate case starts here
    else
    {
      real oneDotTwo = Dot(segmentOne, segmentTwo);

      //Always nonnegative
      real denom = (oneSqLength * twoSqLength) - (oneDotTwo * oneDotTwo);

      //If segments not parallel, compute closest point on line one to line two
      //and clamp to segment one. Else pick arbitrary segment one value (here 0)
      if(denom != real(0.0))
      {
        segmentOneS  = (oneDotTwo * twoStartToStart);
        segmentOneS -= (oneStartToStart * twoSqLength);
        segmentOneS /= denom;
        segmentOneS = Math::Clamp(segmentOneS, real(0.0), real(1.0));
      }
      else
      {
        segmentOneS = real(0.0);
      }

      //Compute point on line two closest to segment one at the segment one 
      //value using t = Dot((P1 + D1*s) - P2,D2) / Dot(D2,D2) = (b*s + f) / e
      real tnom = oneDotTwo * segmentOneS + twoStartToStart;
      if(tnom < real(0.0))
      {
        segmentTwoT = real(0.0);
        segmentOneS = -oneStartToStart / oneSqLength;
        segmentOneS = Math::Clamp(segmentOneS, real(0.0), real(1.0));
      }
      //If t in [0,1] done. Else clamp t, recompute s for the new value of t 
      //using s = Dot((P2 + D2*t) - P1,D1) / Dot(D1,D1) = (t*b - c) / a and 
      //clamp s to [0,1]
      else if(tnom > twoSqLength)
      {
        segmentTwoT = real(1.0);
        segmentOneS = (oneDotTwo - oneStartToStart) / oneSqLength;
        segmentOneS = Math::Clamp(segmentOneS, real(0.0), real(1.0));
      }
      else
      {
        segmentTwoT = tnom / twoSqLength;
      }
    }
  }

  *closestPointA = segmentOnePointA;
  *closestPointA = Math::MultiplyAdd(*closestPointA, segmentOne, segmentOneS);
  *closestPointB = segmentTwoPointA;
  *closestPointB = Math::MultiplyAdd(*closestPointB, segmentTwo, segmentTwoT);
  return Segment;
}
}// namespace Intersection
