///////////////////////////////////////////////////////////////////////////////
///
/// \file Intersection.cpp
/// All of the functions that test for intersection.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Intersection
{

uint Manifold::cMaxPoints = 4;

const Interval Interval::cInfinite = Interval(-Math::PositiveMax(), 
                                               Math::PositiveMax());
const Interval Interval::cInvalid = Interval(Math::PositiveMax(), 
                                             -Math::PositiveMax());

namespace
{

const Vec3 cSafeCheckNormal = Vec3(real(0.0), real(0.0), real(1.0));
const real cPlanePlanePlaneZero = real(0.000001);

}// namespace

//--------------------------------------------------------------------- Manifold
Manifold::Manifold(uint pointCount)
  : PointCount(pointCount)
{
  if(PointCount == 0)
  {
    PointCount = 1;
  }
}

IntersectionPoint& Manifold::PointAt(uint index)
{
  return Points[index];
}

//--------------------------------------------------------------------- Interval
Interval::Interval(void)
{
  //
}

Interval::Interval(real min, real max)
  : Min(min), Max(max)
{
  //
}

//Find the complement of this interval (A) in the given interval (B), or 
//otherwise the values in B but not in A. Similar to subtracting the elements of
//A from B.
Interval Interval::Complement(const Interval& b) const
{
  //All comments use the "min -> max" convention.
  /*
     [~~~~~~~~[~~~~~~~~]---------]
     A        B        A         B
  */
  if(Math::InRange(b.Min, Min, Max))
  {
    return Interval(Max, b.Max);
  }
  /*
     [--------[~~~~~~~~]~~~~~~~~~]
     B        A        B         A
  */
  else if(Math::InRange(b.Max, Min, Max))
  {
    return Interval(b.Min, Min);
  }
  /*
     [--------[~~~~~~~~]---------]
     B        A        A         B
  */
  else if(Math::InRange(Min, b.Min, b.Max) && Math::InRange(Max, b.Min, b.Max))
  {
    return Interval(b.Min, Min);
    //return Interval(Max, b.Max);
  }
  else
  {
    return cInvalid;
  }
}

//Intersect the two intervals, storing the largest minimum value and smallest
//maximum value.
Interval Interval::Intersection(const Interval& interval) const
{
  return Interval(Math::Max(Min, interval.Min), Math::Min(Max, interval.Max));
}

//Combine the two intervals, storing the smallest minimum value and largest
//maximum value.
Interval Interval::Union(const Interval& interval) const
{
  return Interval(Math::Min(Min, interval.Min), Math::Max(Max, interval.Max));
}

//Check to see if the interval is valid (validity defined as Min <= Max).
bool Interval::IsValid(void) const
{
  return Min <= Max;
}

//Returns the first valid (non-infinity) value in the interval, since the range
//could include an infinity value in conjunction with a non-infinity value.
real Interval::FirstT(void) const
{
  const real infinity = Math::PositiveMax();
  if(Min != -infinity && Min != infinity)
  {
    return Min;
  }
  else
  {
    return Max;
  }
}

//---------------------------------------------------------------- Support Shape
SupportShape::SupportShape()
{
  mCenter = Vec3::cZero;
  mSupportFunction = nullptr;
  mData = nullptr;
  mDeltaPosition = Vec3::cZero;
  mDeltaRotation = Quat::cIdentity;
}

SupportShape::SupportShape(Vec3Param center, SupportFunction support, 
                           void* data)
  : mCenter(center), mSupportFunction(support), mData(data)
{
  mDeltaPosition = Vec3::cZero;
  mDeltaRotation = Quat::cIdentity;
}

void SupportShape::GetCenter(Vec3Ptr center) const
{
  ErrorIf(center == nullptr, "Physics::Mpr - Null pointer passed, this function "\
                          "needs a valid pointer.");
  *center = mCenter;
}

void SupportShape::Support(Vec3Param direction, Vec3Ptr support) const
{
  ErrorIf(support == nullptr, "Physics::Mpr - Null pointer passed, this function "\
                           "needs a valid pointer.");
  (*mSupportFunction)(this, mData, direction, support);
}

void SupportShape::GetTranslation(Vec3Ptr translation) const
{
  ErrorIf(translation == nullptr, "Physics::Mpr - Null pointer passed, this "\
                               "function needs a valid pointer.");
}

void SupportShape::SetCenter(Vec3Param center)
{
  mCenter = center;
}

Vec3 SupportShape::GetDeltaPosition() const
{
  return mDeltaPosition;
}

Quat SupportShape::GetDeltaRotation() const
{
  return mDeltaRotation;
}

void SupportShape::SetDeltaPosition(Vec3 pos)
{
  mDeltaPosition = pos;
}

void SupportShape::SetDeltaRotation(Quat rot)
{
  mDeltaRotation = rot;
}



//--------------------------------------------------------- File-Scope Functions
Type AabbFaceNormalFromPoint(Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint, 
                             Vec3Ref point)
{
  ErrorIf((aabbMinPoint.x > aabbMaxPoint.x) ||
          (aabbMinPoint.y > aabbMaxPoint.y) ||
          (aabbMinPoint.z > aabbMaxPoint.z),
          "Intersection - Axis-aligned bounding box's minimum point is greater"\
          " than it's maximum point.");

  Type inOrOut = Inside;
  Vec3 normal = Vec3::cZero;
  for(uint i = 0; i < 3; ++i)
  {
    if(point[i] <= aabbMinPoint[i])
    {
      normal[i] = real(-1.0);
      inOrOut = Outside;
    }
    else if(point[i] >= aabbMaxPoint[i])
    {
      normal[i] = real(1.0);
      inOrOut = Outside;
    }
  }
  point = normal;
  return inOrOut;
}

///Not used but useful.
Type ObbFaceNormalFromPoint(Vec3Param obbCenter, Vec3Param obbHalfExtents,
                            Mat3Param obbBasis, Vec3Ref point)
{
  Type inOrOut = Inside;
  Vec3 obbAxes[3] = { obbBasis.GetBasis(0),
                      obbBasis.GetBasis(1),
                      obbBasis.GetBasis(2) };
  Vec3 obbToPoint = point - obbCenter;

  point.Set(real(0.0), real(0.0), real(0.0));

  for(uint i = 0; i < 3; ++i)
  {
    real dot = Dot(obbToPoint, obbAxes[i]);

    //If the point is further than the box's extent on this axis in the
    //positive direction, clamp it
    if(dot >= obbHalfExtents[i])
    {
      point += obbAxes[i];
      inOrOut = Outside;
    }

    //If the point is further than the box's extent on this axis in the
    //negative direction, clamp it
    if(dot <= -obbHalfExtents[i])
    {
      point -= obbAxes[i];
      inOrOut = Outside;
    }
  }
  return inOrOut;
}

//----------------------------------------------------------------- Object Tests
//Intersect an axis aligned bounding box with an axis aligned bounding box.
Type AabbAabb(Vec3Param aabbOneMinPoint, Vec3Param aabbOneMaxPoint, 
              Vec3Param aabbTwoMinPoint, Vec3Param aabbTwoMaxPoint, 
              Manifold* manifold)
{
  ErrorIf((aabbOneMinPoint.x > aabbOneMaxPoint.x) ||
          (aabbOneMinPoint.y > aabbOneMaxPoint.y) ||
          (aabbOneMinPoint.z > aabbOneMaxPoint.z),
          "Intersection - Axis-aligned bounding box's minimum point is greater"\
          " than it's maximum point.");
  ErrorIf((aabbTwoMinPoint.x > aabbTwoMaxPoint.x) ||
          (aabbTwoMinPoint.y > aabbTwoMaxPoint.y) ||
          (aabbTwoMinPoint.z > aabbTwoMaxPoint.z),
          "Intersection - Axis-aligned bounding box's minimum point is greater"\
          " than it's maximum point.");


  //Separated along x-axis?
  if(aabbOneMaxPoint.x < aabbTwoMinPoint.x ||
     aabbOneMinPoint.x > aabbTwoMaxPoint.x)
  {
    return None;
  }

  //Separated along y-axis?
  if(aabbOneMaxPoint.y < aabbTwoMinPoint.y ||
     aabbOneMinPoint.y > aabbTwoMaxPoint.y)
  {
    return None;
  }

  //Separated along z-axis?
  if(aabbOneMaxPoint.z < aabbTwoMinPoint.z || 
     aabbOneMinPoint.z > aabbTwoMaxPoint.z)
  {
    return None;
  }

  if(manifold != nullptr)
  {
    manifold->PointCount = 1;
    Vec3 closestPoint  = aabbOneMinPoint + aabbOneMaxPoint;
         closestPoint += aabbTwoMinPoint + aabbTwoMaxPoint;
         closestPoint *= real(0.25);
    manifold->PointAt(0).Points[0] = closestPoint;
    manifold->PointAt(0).Points[1] = closestPoint;
    ClosestPointOnAabbToPoint(aabbOneMinPoint, aabbOneMaxPoint, 
                              &(manifold->PointAt(0).Points[0]));
    ClosestPointOnAabbToPoint(aabbTwoMinPoint, aabbTwoMaxPoint,
                              &(manifold->PointAt(0).Points[1]));
    closestPoint = manifold->PointAt(0).Points[1] - 
                   manifold->PointAt(0).Points[0];
    if(cGeometrySafeChecks)
    {
      manifold->PointAt(0).Depth = AttemptNormalize(closestPoint);

      //Find the longest span between the centers of the two AABBs, that axis is
      //the normal
      if(Math::IsZero(manifold->PointAt(0).Depth))
      {
        //Center of AABB one
        Vec3 oneCenter = (aabbOneMaxPoint - aabbOneMinPoint) * real(0.5) +
                         aabbOneMinPoint;

        //Center of AABB two
        closestPoint = (aabbTwoMaxPoint - aabbTwoMinPoint) * real(0.5) +
                       aabbTwoMinPoint - oneCenter;

        uint longestAxis = uint(-1);
        real axisLength = real(-1.0);
        for(uint i = 0; i < 3; ++i)
        {
          if(Math::Abs(axisLength) < Math::Abs(closestPoint[i]))
          {
            axisLength = closestPoint[i];
            longestAxis = i;
          }
        }

        /*
          NOTE:
            If it crashes below this point, then "longestAxis" was never set,
            which can only happen if garbage values are passed in.
        */
        closestPoint.ZeroOut();
        closestPoint[longestAxis] = real(1.0) * Math::GetSign(axisLength);
        manifold->PointCount = 1;
        manifold->Normal = closestPoint;
      }
      else
      {
        manifold->PointAt(0).Depth = Normalize(closestPoint);
        manifold->PointCount = 1;
        manifold->Normal = closestPoint;
      }
    }
    else
    {
      manifold->PointAt(0).Depth = Normalize(closestPoint);
      manifold->PointCount = 1;
      manifold->Normal = closestPoint;
    }
  }
  return Point;
}

//Intersect an axis aligned bounding box with a capsule.
Type AabbCapsule(Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint,
                 Vec3Param capsulePointA, Vec3Param capsulePointB, 
                 real capsuleRadius, Manifold* manifold)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf((aabbMinPoint.x > aabbMaxPoint.x) ||
          (aabbMinPoint.y > aabbMaxPoint.y) ||
          (aabbMinPoint.z > aabbMaxPoint.z),
          "Intersection - Axis-aligned bounding box's minimum point is greater"\
          " than it's maximum point.");
  if(manifold != nullptr)
  {
    //
  }
  return Unimplemented;
}

//Intersect an axis aligned bounding box with an oriented bounding box.
Type AabbObb(Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint, 
             Vec3Param obbCenter, Vec3Param obbHalfExtents, Mat3Param obbBasis,
             Manifold* manifold)
{
  ErrorIf((aabbMinPoint.x > aabbMaxPoint.x) ||
          (aabbMinPoint.y > aabbMaxPoint.y) ||
          (aabbMinPoint.z > aabbMaxPoint.z),
          "Intersection - Axis-aligned bounding box's minimum point is greater"\
          " than it's maximum point.");

  Vec3 aabbHalfExtents = real(0.5) * (aabbMaxPoint - aabbMinPoint);
  Vec3 aabbCenter = aabbMinPoint + aabbHalfExtents;
  Mat3 aabbBasis = Mat3::cIdentity;

  //Code reuse. :D
  Type result = ObbObb(aabbCenter, aabbHalfExtents, aabbBasis,
                       obbCenter, obbHalfExtents, obbBasis, manifold);
  return result;
}

//Intersect an axis aligned bounding box with a plane.
Type AabbPlane(Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint, 
               Vec3Param planeNormal, real planeDistance, Manifold* manifold)
{
  ErrorIf((aabbMinPoint.x > aabbMaxPoint.x) ||
          (aabbMinPoint.y > aabbMaxPoint.y) ||
          (aabbMinPoint.z > aabbMaxPoint.z),
          "Intersection - Axis-aligned bounding box's minimum point is greater"\
          " than it's maximum point.");

  Vec3 halfExtents = real(0.5) * (aabbMaxPoint - aabbMinPoint);
  Vec3 center = halfExtents + aabbMinPoint;

  //Early out with the simple test if no information was requested.
  if(manifold == nullptr)
  {
    //Compute the projection interval radius of box onto L(t) = b.c + t * p.n
    real radius = halfExtents.x * Math::Abs(planeNormal.x) +
                  halfExtents.y * Math::Abs(planeNormal.y) +
                  halfExtents.z * Math::Abs(planeNormal.z);

    //Compute the signed distance of the box's center from the plane
    real signedDistance = Geometry::SignedDistanceToPlane(center, planeNormal, 
                                                          planeDistance);
    
    //Intersection occurs when the signed distance falls within the 
    //[-radius, +radius] interval
    if(Math::Abs(signedDistance) <= radius)
    {
      return Other;
    }
    return None;
  }

  Mat3 aabbBasis = Mat3::cIdentity;

  //Might be a teeny bit slower than writing the special case for AABB-plane but
  //it works for now.
  return ObbPlane(center, halfExtents, aabbBasis, planeNormal, 
                  planeDistance, manifold);
}

//Intersect an axis aligned bounding box with a sphere.
Type AabbSphere(Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint, 
                Vec3Param sphereCenter, real sphereRadius, Manifold* manifold)
{
  ErrorIf((aabbMinPoint.x > aabbMaxPoint.x) ||
          (aabbMinPoint.y > aabbMaxPoint.y) ||
          (aabbMinPoint.z > aabbMaxPoint.z),
          "Intersection - Axis-aligned bounding box's minimum point is greater"\
          " than it's maximum point.");

  //Get the closest point on the AABB's surface to the sphere's center
  Vec3 closestPoint = sphereCenter;
  Type result = ClosestPointOnAabbToPoint(aabbMinPoint, aabbMaxPoint, 
                                          &closestPoint);

  //If the point was found to be outside of the AABB then the sphere's center is
  //outside of the AABB
  if(result != Inside)
  {
    Vec3 pointToSphere = sphereCenter - closestPoint;

    //If the distance between sphere's center and closest point on the AABB is
    //greater than the sphere's radius, there is no intersection.
    if(LengthSq(pointToSphere) > (sphereRadius * sphereRadius))
    {
      return None;
    }

    if(manifold != nullptr)
    {
      //The overlap amount is just the amount of the sphere's radius that is
      //inside of the AABB
      real overlapAmount = sphereRadius;

      //In case sphere center and closest point on AABB are the same
      if(cGeometrySafeChecks)
      {
        overlapAmount -= AttemptNormalize(pointToSphere);
        if(overlapAmount == sphereRadius)
        {
          pointToSphere = sphereCenter;
          AabbFaceNormalFromPoint(aabbMinPoint, aabbMaxPoint, pointToSphere);
          Normalize(pointToSphere);
        }
      }
      else
      {
        overlapAmount -= Normalize(pointToSphere);
      }

      manifold->Normal = pointToSphere;
      manifold->PointAt(0).Depth = overlapAmount;

      //Intersection point on AABB is closest point
      manifold->PointAt(0).Points[0] = closestPoint;

      //Intersection point on sphere is the normal scaled by the -radius
      pointToSphere *= -sphereRadius;
      pointToSphere += sphereCenter;
      manifold->PointAt(0).Points[1] = pointToSphere;
      manifold->PointCount = 1;
    }
    return Point;
  }
  //Now the case where the sphere's center is contained within the AABB
  else
  {
    //This is the case where the center of the sphere is inside of the AABB
    if(manifold != nullptr)
    {
      Vec3 pointToSphere = sphereCenter - closestPoint;

      //Since the sphere is contained within the AABB at this point, the
      //penetration depth is sum of the distance between the sphere's center and
      //the closest point on the AABB
      real overlapAmount = Normalize(pointToSphere) + sphereRadius;

      //Normal is now pointing inside the AABB, so we need to negate it in order
      //to push the sphere outside
      Negate(&pointToSphere);

      manifold->Normal = pointToSphere;
      manifold->PointAt(0).Depth = overlapAmount;

      //Intersection point for AABB is the closest point
      manifold->PointAt(0).Points[0] = closestPoint;

      //Intersection point for sphere is the normal scaled by the -overlap
      pointToSphere *= -overlapAmount;
      pointToSphere += closestPoint;
      manifold->PointAt(0).Points[1] = pointToSphere;
      manifold->PointCount = 1;
    }
    return Other;
  }
  //return None;
}

//Intersect an axis aligned bounding box with a triangle.
Type AabbTriangle(Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint,
                  Vec3Param trianglePointA, Vec3Param trianglePointB,
                  Vec3Param trianglePointC, Manifold* manifold)
{
  ErrorIf((aabbMinPoint.x > aabbMaxPoint.x) ||
          (aabbMinPoint.y > aabbMaxPoint.y) ||
          (aabbMinPoint.z > aabbMaxPoint.z),
          "Intersection - Axis-aligned bounding box's minimum point is greater"\
          " than it's maximum point.");

  Vec3 aabbHalfExtents = (aabbMaxPoint - aabbMinPoint) * real(0.5);
  Vec3 aabbCenter = aabbMinPoint + aabbHalfExtents;
  Type result = ObbTriangle(aabbCenter, aabbHalfExtents, Mat3::cIdentity, 
                            trianglePointA, trianglePointB, trianglePointC, 
                            manifold);
  return result;
}

//Intersect a capsule with a capsule.
Type CapsuleCapsule(Vec3Param capsuleOnePointA, Vec3Param capsuleOnePointB, 
                    real capsuleOneRadius, Vec3Param capsuleTwoPointA, 
                    Vec3Param capsuleTwoPointB, real capsuleTwoRadius,
                    Manifold* manifold)
{
  Vec3 capsuleOnePoint;
  Vec3 capsuleTwoPoint;
  ClosestPointsOfTwoSegments(capsuleOnePointA, capsuleOnePointB, 
                             capsuleTwoPointA, capsuleTwoPointB, 
                             &capsuleOnePoint, &capsuleTwoPoint);
  Vec3 oneToTwo = capsuleTwoPoint - capsuleOnePoint;
  real radiiSum = capsuleOneRadius + capsuleTwoRadius;

  //Distance between the closest points of the two capsules is greater than the
  //sum of their radii, no overlap has occurred.
  if(LengthSq(oneToTwo) > (radiiSum * radiiSum))
  {
    return None;
  }

  if(manifold != nullptr)
  {
    real overlapAmount = radiiSum;
    //In case closest points of the capsules are the same.
    if(cGeometrySafeChecks)
    {
      overlapAmount -= AttemptNormalize(oneToTwo);
      if(overlapAmount == radiiSum)
      {
        oneToTwo = cSafeCheckNormal;
      }
    }
    else
    {
      overlapAmount -= Normalize(oneToTwo);
    }
    
    manifold->Normal = oneToTwo;
    manifold->PointCount = 1;

    manifold->PointAt(0).Depth = overlapAmount;

    //Point on capsule one is the closest point on capsule one's segment plus
    //the normal scaled by capsule one's radius
    manifold->PointAt(0).Points[0] = capsuleOnePoint;
    manifold->PointAt(0).Points[0] = Math::MultiplyAdd(manifold->PointAt(0).Points[0], oneToTwo, capsuleOneRadius);

    //Point on capsule two is the closest point on capsule two's segment plus
    //the negated normal scaled by capsule two's radius
    manifold->PointAt(0).Points[1] = capsuleTwoPoint;
    manifold->PointAt(0).Points[1] = Math::MultiplyAdd(manifold->PointAt(0).Points[1], oneToTwo, -capsuleTwoRadius);
  }
  return Point;
}

//Intersect a capsule with a frustum. The 6 planes of the frustum are assumed 
//to be pointing inwards.
Type CapsuleFrustum(Vec3Param capsulePointA, Vec3Param capsulePointB, 
                    real capsuleRadius, const Vec4 frustumPlanes[6], 
                    Manifold* manifold)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  if(manifold != nullptr)
  {
    //
  }
  return Unimplemented;
}

//Intersect a capsule with an oriented bounding box.
Type CapsuleObb(Vec3Param capsulePointA, Vec3Param capsulePointB, 
                real capsuleRadius, Vec3Param obbCenter, 
                Vec3Param obbHalfExtents, Vec3Param obbBasis, 
                Manifold* manifold)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  if(manifold != nullptr)
  {
    //
  }
  return Unimplemented;
}

///Intersect a capsule with a plane.
Type CapsulePlane(Vec3Param capsulePointA, Vec3Param capsulePointB, 
                  real capsuleRadius, Vec3Param planeNormal, real planeDistance,
                  Manifold* manifold)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  if(manifold != nullptr)
  {
    //
  }
  return Unimplemented;
}

//Intersect a capsule with a sphere.
Type CapsuleSphere(Vec3Param capsulePointA, Vec3Param capsulePointB, 
                   real capsuleRadius, Vec3Param sphereCenter, 
                   real sphereRadius, Manifold* manifold)
{
  Vec3 closestPoint = sphereCenter;
  Type result = ClosestPointOnCapsuleToPoint(capsulePointA, capsulePointB, 
                                             capsuleRadius, &closestPoint);

#ifdef VisualizeCapsuleSphere
  Zero::gDebugDraw->Add(Zero::Debug::LineCross(closestPoint, real(0.5)));
#endif

  //Vector from sphere's center to the closest point on the capsule
  Vec3 v = closestPoint - sphereCenter;

  real radiiSum = sphereRadius;// + capsuleRadius;

  //Distance between sphere's center and the closest point on the capsule to 
  //the sphere's center is greater than the sphere's radius. No intersection.
  if(LengthSq(v) > (radiiSum * radiiSum))
  {
    return None;
  }

  if(manifold != nullptr)
  {
    //Overlap occurring with sphere and capsule
    real overlapAmount = radiiSum;

    //In case sphere center and closest point on the capsule are the same.
    if(cGeometrySafeChecks)
    {
      overlapAmount -= AttemptNormalize(v);
      if(overlapAmount == radiiSum)
      {
        v = cSafeCheckNormal;
      }
    }
    else
    {
      overlapAmount -= Normalize(v);
    }

    manifold->Normal = -v;

    //If the sphere's center is inside the capsule, the normal is pointing 
    //inwards, so we need to reverse it
    if(result == Inside)
    {
      manifold->Normal *= real(-1.0);
    }
    manifold->PointAt(0).Depth = overlapAmount;

    //Intersection point for sphere lies along normal scaled by radius
    v *= sphereRadius;
    v += sphereCenter;
    if(result == Inside)
    {
      v *= real(-1.0);
    }
    manifold->PointAt(0).Points[0] = closestPoint;

    //Intersection point for quad is the closest point on the capsule to the
    //sphere's center.
    manifold->PointAt(0).Points[1] = v;

#ifdef VisualizeCapsuleSphere
  Zero::gDebugDraw->Add(Zero::Debug::Sphere(v, real(0.1)).Color(Color::Red));
  Zero::gDebugDraw->Add(Zero::Debug::Sphere(closestPoint, real(0.1)).Color(Color::Blue));
#endif

    manifold->PointCount = 1;
  }
  return Point;
}

//Intersect a capsule with a triangle.
Type CapsuleTriangle(Vec3Param capsulePointA, Vec3Param capsulePointB, 
                     real capsuleRadius, Vec3Param trianglePointA, 
                     Vec3Param trianglePointB, Vec3Param trianglePointC, 
                     Manifold* manifold)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  if(manifold != nullptr)
  {
    //
  }
  return Unimplemented;
}

//Intersect a plane with a plane. Points returned, if intersection data is
//desired, are two points on the line of intersection.
Type PlanePlane(Vec3Param planeOneNormal, real planeOneDistance, 
                Vec3Param planeTwoNormal, real planeTwoDistance, 
                Manifold* manifold)
{
  Vec3 direction = Cross(planeOneNormal, planeTwoNormal);

  //If direction is the zero vector, the planes are parallel (and separated) or
  //coincident, so they're not considered intersecting
  if(Dot(direction, direction) == real(0.0))
  {
    return None;
  }

  if(manifold != nullptr)
  {
    real d11 = Dot(planeOneNormal, planeOneNormal);
    real d12 = Dot(planeOneNormal, planeTwoNormal);
    real d22 = Dot(planeTwoNormal, planeTwoNormal);

    real denom = (d11 * d22) - (d12 * d12);
    real k1 = ((planeOneDistance * d22) - (planeTwoDistance * d12)) / denom;
    real k2 = ((planeTwoDistance * d11) - (planeOneDistance * d12)) / denom;

    manifold->Normal = direction;
    manifold->PointAt(0).Depth = real(0.0);

    Vec3 pointOnLine = planeOneNormal * k1;
         pointOnLine = Math::MultiplyAdd(pointOnLine, planeTwoNormal, k2);
    manifold->PointAt(0).Points[0] = pointOnLine;

    pointOnLine += direction;
    manifold->PointAt(0).Points[1] = pointOnLine;
  }
  return Line;
}

//Intersect a plane with a plane with plane. If intersection data is desired,
//the point of collision is returned (if there is one).
Type PlanePlanePlane(Vec4Param planeA, Vec4Param planeB, Vec4Param planeC, 
                     Manifold* manifold)
{
  const Vec3* planeNormals[3] = { reinterpret_cast<const Vec3*>(&(planeA.x)),
                                  reinterpret_cast<const Vec3*>(&(planeB.x)),
                                  reinterpret_cast<const Vec3*>(&(planeC.x)) };
  Vec3 u = Cross(*(planeNormals[1]), *(planeNormals[2]));
  real denom = Dot(*(planeNormals[0]), u);

  if(Math::Abs(denom) < cPlanePlanePlaneZero)
  {
    return Other;
  }

  if(manifold != nullptr)
  {
    u *= planeA[3];
    u += Cross(*(planeNormals[0]), 
               planeC[3] * *(planeNormals[1]) - planeB[3] * *(planeNormals[2]));
    u /= denom;
    manifold->PointAt(0).Points[0] = u;
  }
  return Point;
}

//Intersect a plane with a sphere.
Type PlaneSphere(Vec3Param planeNormal, real planeDistance, 
                 Vec3Param sphereCenter, real sphereRadius, Manifold* manifold)
{
  //Get the closest point on the plane to the sphere's center.
  Vec3 closestPoint = sphereCenter;
  ClosestPointOnPlaneToPoint(planeNormal, planeDistance, &closestPoint);

  //Vector from closest point on plane to sphere's center
  Vec3 v = sphereCenter - closestPoint;

  //Distance between sphere's center and the closest point on the plane to the
  //sphere's center is greater than the sphere's radius. No intersection.
  if(LengthSq(v) > (sphereRadius * sphereRadius))
  {
    return None;
  }

  if(manifold != nullptr)
  {
    real overlapAmount = sphereRadius;

    //In case sphere's center and the closest point on the plane are the same.
    if(cGeometrySafeChecks)
    {
      overlapAmount -= AttemptNormalize(v);
    }
    else
    {
      overlapAmount -= Normalize(v);
    }

    manifold->Normal = v;
    manifold->PointAt(0).Depth = overlapAmount;

    //Intersection point for the plane is the closest point
    manifold->PointAt(0).Points[0] = closestPoint;

    //Intersection point for the sphere is negated normal scaled by radius
    v *= -sphereRadius;
    v += sphereCenter;
    manifold->PointAt(0).Points[1] = v;

    manifold->PointCount = 1;
  }
  return Point;
}

//Intersect a plane with a triangle.
Type PlaneTriangle(Vec3Param planeNormal, real planeDistance, 
                   Vec3Param trianglePointA, Vec3Param trianglePointB, 
                   Vec3Param trianglePointC, Manifold* manifold)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  if(manifold != nullptr)
  {
    //
  }
  return Unimplemented;
}

//Intersect a sphere with a sphere.
Type SphereSphere(Vec3Param sphereOneCenter, real sphereOneRadius, 
                  Vec3Param sphereTwoCenter, real sphereTwoRadius, 
                  Manifold* manifold)
{
  Vec3 oneToTwo = sphereTwoCenter - sphereOneCenter;

  //If the distance between the two spheres is greater than their combined radii
  //then they are not intersecting. Below, the test uses squared lengths and 
  //radii, which works out to be the same for the boolean test.
  real sphereRadii = sphereOneRadius + sphereTwoRadius;
  if(LengthSq(oneToTwo) > (sphereRadii * sphereRadii))
  {
    return None;
  }

  if(manifold != nullptr)
  {
    real overlapAmount = sphereRadii;
    
    //In case the spheres' centers are the same.
    if(cGeometrySafeChecks)
    {
      overlapAmount -= AttemptNormalize(oneToTwo);
      if(overlapAmount == sphereRadii)
      {
        oneToTwo = cSafeCheckNormal;
      }
    }
    else
    {
      overlapAmount -= Normalize(oneToTwo);
    }

    manifold->Normal = oneToTwo;
    manifold->PointAt(0).Depth = overlapAmount;

    Vec3 point = sphereOneCenter;
         point = Math::MultiplyAdd(point, oneToTwo, sphereOneRadius);
    manifold->PointAt(0).Points[0] = point;

    point = sphereTwoCenter;
    point = Math::MultiplyAdd(point, oneToTwo, -sphereTwoRadius);
    manifold->PointAt(0).Points[1] = point;
    manifold->PointCount = 1;
  }
  return Point;
}

//Intersect a sphere with a triangle.
Type SphereTriangle(Vec3Param sphereCenter, real sphereRadius, 
                    Vec3Param trianglePointA, Vec3Param trianglePointB, 
                    Vec3Param trianglePointC, Manifold* manifold)
{
  Vec3 closestPoint = sphereCenter;
  ClosestPointOnTriangleToPoint(trianglePointA, trianglePointB, trianglePointC, 
                                &closestPoint);

  //Vector from sphere's center to the closest point on the triangle
  Vec3 v = closestPoint - sphereCenter;

  //Distance between sphere's center and the closest point on the triangle to 
  //the sphere's center is greater than the sphere's radius. No intersection.
  if(LengthSq(v) > (sphereRadius * sphereRadius))
  {
    return None;
  }

  if(manifold != nullptr)
  {
    //Overlap occurring with sphere and triangle
    real overlapAmount = sphereRadius;

    //In case sphere center and closest point on triangle are the same.
    if(cGeometrySafeChecks)
    {
      overlapAmount -= AttemptNormalize(v);
      if(overlapAmount == sphereRadius)
      {
        v = cSafeCheckNormal;
      }
    }
    else
    {
      overlapAmount -= Normalize(v);
    }

    manifold->Normal = v;
    manifold->PointAt(0).Depth = overlapAmount;

    //Intersection point for sphere lies along normal scaled by radius
    v *= sphereRadius;
    v += sphereCenter;
    manifold->PointAt(0).Points[0] = v;

    //Intersection point for triangle is the closest point on the triangle to 
    //the sphere's center.
    manifold->PointAt(0).Points[1] = closestPoint;

    manifold->PointCount = 1;
  }
  return Point;
}

//Intersect a triangle with a triangle. This may be slow, and the manifold 
//doesn't do anything.
Type TriangleTriangle(Vec3Param triOnePointA, Vec3Param triOnePointB,
                      Vec3Param triOnePointC, Vec3Param triTwoPointA,
                      Vec3Param triTwoPointB, Vec3Param triTwoPointC, 
                      Manifold* manifold)
{
  //----------------------------------------------------------------------------
  //Compute the plane equation of triangle A.
  Vec3 planeNormalOne = Geometry::GenerateNormal(triOnePointA, triOnePointB,
                                                 triOnePointC);
  real planeDistanceOne = Dot(planeNormalOne, triOnePointA);

  //Exit with no intersection if the vertices of triangle B are on the same side
  //of this plane. Save these results for later usage.
  Type pointSideOne[3] = { PointPlane(triTwoPointA, planeNormalOne, 
                                      planeDistanceOne), 
                           PointPlane(triTwoPointB, planeNormalOne, 
                                      planeDistanceOne),
                           PointPlane(triTwoPointC, planeNormalOne, 
                                      planeDistanceOne)             };
  if((pointSideOne[0] == pointSideOne[1]) && 
     (pointSideOne[0] == pointSideOne[2]) &&
     (pointSideOne[1] == pointSideOne[2]))
  {
    return None;
  }

  //----------------------------------------------------------------------------
  //Compute the plane equation of triangle B.
  Vec3 planeNormalTwo = Geometry::GenerateNormal(triTwoPointA, triTwoPointB, 
                                                 triTwoPointC);
  real planeDistanceTwo = Dot(planeNormalTwo, triTwoPointA);

  //Exit with no intersection if the vertices of triangle A are on the same side
  //of this plane. Save these results for later usage.
  Type pointSideTwo[3] = { PointPlane(triOnePointA, planeNormalTwo, 
                                      planeDistanceTwo), 
                           PointPlane(triOnePointB, planeNormalTwo, 
                                      planeDistanceTwo),
                           PointPlane(triOnePointC, planeNormalTwo, 
                                      planeDistanceTwo)             };
  if((pointSideTwo[0] == pointSideTwo[1]) && 
     (pointSideTwo[0] == pointSideTwo[2]) &&
     (pointSideTwo[1] == pointSideTwo[2]))
  {
    return None;
  }

  //----------------------------------------------------------------------------
  //Compute the line L of intersection between the two planes.
  Manifold planesInfo;
  PlanePlane(planeNormalOne, planeDistanceOne, planeNormalTwo, planeDistanceTwo,
             &planesInfo);
  Vec3 line = Abs(planesInfo.PointAt(0).Points[0] - 
                  planesInfo.PointAt(0).Points[1]);

  //Determine which principal coordinate axis is most parallel with the 
  //intersection line L.
  uint axis = (line.x > line.y && line.x > line.z) ? 0 :
              (line.y > line.z ? 1 : 2);

  //Compute scalar intersection intervals for each triangle with L, as projected
  //onto this principal axis.
  real projOne[2];
  projOne[0] = Math::Min(Math::Min(triOnePointA[axis], triOnePointB[axis]),    
                         triOnePointC[axis]);
  projOne[1] = Math::Max(Math::Max(triOnePointA[axis], triOnePointB[axis]),
                         triOnePointC[axis]);

  real projTwo[2];
  projTwo[0] = Math::Min(Math::Min(triTwoPointA[axis], triTwoPointA[axis]),    
                         triTwoPointA[axis]);
  projTwo[1] = Math::Max(Math::Max(triTwoPointA[axis], triTwoPointA[axis]),
                         triTwoPointA[axis]);
  real proj[2] = { Math::Max(projOne[0], projTwo[0]),
                   Math::Min(projOne[1], projTwo[1])  };

  //----------------------------------------------------------------------------
  //The triangles intersect if the intersection intervals overlap; otherwise,
  //they do not.
  if(proj[0] < proj[1])
  {
    return Other;
  }
  return None;
}

}//namespace Intersection
