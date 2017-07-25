///////////////////////////////////////////////////////////////////////////////
///
///  \file RayTests.cpp
///  Source for all of the ray intersection tests.
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

const real cMinPointDist = real(0.0000001);

void MakeBasisFromY(Vec3Param yAxis, Mat3Ptr basis)
{
  Vec3 xAxis, zAxis;
  Math::GenerateOrthonormalBasis(yAxis, &xAxis, &zAxis);
  basis->SetBasis(0, xAxis);
  basis->SetBasis(1, yAxis);
  basis->SetBasis(2, zAxis);
}

real PermutedInnerProduct(Vec3Param uA, Vec3Param vA, 
                          Vec3Param uB, Vec3Param vB)
{
  return Dot(uA, vB) + Dot(uB, vA);
}

} //namespace


//--------------------------------------------------------- Ray Tests (Interval)
//Intersect a ray with an axis aligned bounding box.
Type RayAabb(Vec3Param rayStart, Vec3Param rayDirection, Vec3Param aabbMinPoint,
             Vec3Param aabbMaxPoint, Interval* interval)
{
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  ErrorIf((aabbMinPoint.x > aabbMaxPoint.x) ||
          (aabbMinPoint.y > aabbMaxPoint.y) ||
          (aabbMinPoint.z > aabbMaxPoint.z),
          "Intersection - Axis-aligned bounding box's minimum point is greater"\
          " than it's maximum point.");

  real tMin = real(0.0);
  real tMax = Math::PositiveMax();

  for(uint i = 0; i < 3; ++i)
  {
    //If the ray's direction is perpendicular to the current axis and the ray's
    //start isn't within the bounds of the box's faces on that axis then the 
    //ray is not intersecting the box.
    if(Math::IsZero(rayDirection[i]))
    {
      //Ray is parallel to slab. No hit if the origin is not within the slab.
      if((rayStart[i] < aabbMinPoint[i]) || (rayStart[i] > aabbMaxPoint[i]))
      {
        return None;
      }
    }
    else
    {
      //Compute intersection t-value of ray with near and far plane of slab.
      real ood = real(1.0) / rayDirection[i];
      real t1 = (aabbMinPoint[i] - rayStart[i]) * ood;
      real t2 = (aabbMaxPoint[i] - rayStart[i]) * ood;

      //Make t1 be intersection with near plane, t2 with far plane.
      if(t1 > t2)
      {
        Math::Swap(t1, t2);
      }

      //Compute the intersection of slab intersection intervals.
      if(tMin < t1)
      {
        tMin = t1;
      }
      if(tMax > t2)
      {
        tMax = t2;
      }

      //Exit with no collision as soon as slab intersection becomes empty.
      if(tMin > tMax)
      {
        return None;
      }
    }
  }

  //Ray intersects all 3 slabs. Return intersection t-value.
  interval->Min = tMin;
  interval->Max = tMax;
  return Other;
}

//Intersect a ray with a capsule defined by its center, local axes, radius, and
//half of the distance between the centers of the spherical endcaps.
Type RayCapsule(Vec3Param rayStart, Vec3Param rayDirection,
                Vec3Param capsuleCenter, Mat3Param capsuleBasis, 
                real capsuleRadius, real capsuleSegmentHalfLength, 
                Interval* interval)
{
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");

  *interval = Interval::cInfinite;

  const real halfHeight = capsuleSegmentHalfLength;
  Vec3 capsulePointA = capsuleCenter + (halfHeight * capsuleBasis.BasisY());
  Vec3 capsulePointB = capsuleCenter - (halfHeight * capsuleBasis.BasisY());
  Vec3 normal = Normalized(capsulePointA - capsulePointB);

  //-------------------------------------------------------------------- Plane A
  real planeDistance = Dot(capsulePointA, normal);
  LinePlane(rayStart, rayDirection, normal, planeDistance, interval);

  //-------------------------------------------------------------------- Plane B
  {
    planeDistance = Dot(capsulePointB, -normal);

    Interval temp = Interval::cInfinite;
    LinePlane(rayStart, rayDirection, -normal, planeDistance, &temp);
    *interval = interval->Intersection(temp);
  }

  //---------------------------------------------------------- Infinite Cylinder
  real radiusSq = Math::Sq(capsuleRadius);
  {
    Interval temp = Interval::cInvalid;

    // n*d
    const real dirDotNormal = Dot(rayDirection, normal);

    // m
    const Vec3 relStart = rayStart - capsulePointA;

    // m*d
    const real startDotNormal = Dot(relStart, normal);
    
    // (d*d)(n*n) - (n*d)^2 = 1 - (n*d)^2
    const real a = real(1.0) - Math::Sq(dirDotNormal);

    // (d*d)((m*m) - r^2) - (m*d)^2 = ((m*m) - r^2) - (m*d)^2
    const real c = (LengthSq(relStart) - radiusSq) - Math::Sq(startDotNormal);
    if(Math::Abs(a) < real(0.000001))
    {
      if(c > real(0.0))
      {
        return None;
      }
    }
    else
    {
      // 2 x ((d*d)(m*n) - (n*d)(m*d))
      const real b = real(2.0) * (Dot(relStart, rayDirection) - 
                                  startDotNormal * dirDotNormal);
      real discr = b * b - real(4.0) * a * c;
      if(discr > real(0.0))
      {
        discr = Math::Sqrt(discr);
        temp.Min = (-b - discr) / (real(2.0) * a);
        temp.Max = (-b + discr) / (real(2.0) * a);
        *interval = interval->Intersection(temp);
      }
      else if(discr != real(0.0))
      {
        return None;
      }
    }
  }

  //Ray completely missed the capsule's finite cylinder, but it could still hit 
  //one of the spherical end caps. Invalidate the finite cylinder's interval so 
  //it doesn't affect the results of the sphere intersections
  if(!interval->IsValid())
  {
    *interval = Interval::cInvalid;
  }

  //------------------------------------------------------------------- Sphere A
  {
    Interval temp = Interval::cInvalid;
    RaySphere(rayStart, rayDirection, capsulePointA, capsuleRadius, &temp);

    if(temp.IsValid())
    {
      *interval = interval->Union(temp);
    }
  }

  //------------------------------------------------------------------- Sphere B
  {
    Interval temp = Interval::cInvalid;
    RaySphere(rayStart, rayDirection, capsulePointB, capsuleRadius, &temp);

    if(temp.IsValid())
    {
      *interval = interval->Union(temp);
    }
  }

  //------------------------------------------------ Compute Intersection Points
  if(!interval->IsValid())
  {
    return None;
  }

  return Other;
}

//Intersect a ray with a capsule. If the result is "Segment", the second point
//isn't guaranteed to be on the surface of the capsule (for now).
Type RayCapsule(Vec3Param rayStart, Vec3Param rayDirection, 
                Vec3Param capsulePointA, Vec3Param capsulePointB, 
                real capsuleRadius, Interval* interval)
{
  Vec3 yAxis = capsulePointB - capsulePointA;
  real halfLength = real(0.5) * Normalize(yAxis);
  Vec3 center = capsulePointA + (halfLength * yAxis);

  Mat3 basis;
  MakeBasisFromY(yAxis, &basis);

  return RayCapsule(rayStart, rayDirection, center, basis, capsuleRadius, 
                    halfLength, interval);
}

//Intersect a ray with a cylinder defined by its center, local axes, radius, and
//half height.
Type RayCylinder(Vec3Param rayStart, Vec3Param rayDirection, 
                 Vec3Param cylinderCenter, Mat3Param cylinderBasis, 
                 real cylinderRadius, real cylinderHalfHeight, 
                 Interval* interval)
{
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");

  const Vec3 cylinderPointA = cylinderCenter +
                              (cylinderBasis.BasisY() * cylinderHalfHeight);
  const Vec3 cylinderPointB = cylinderCenter -
                              (cylinderBasis.BasisY() * cylinderHalfHeight);

  *interval = Interval::cInfinite;

  Vec3 normal = Normalized(cylinderPointA - cylinderPointB);

  //-------------------------------------------------------------------- Plane A
  {
    Interval temp = Interval::cInfinite;
    Type result = RayPlane(rayStart, rayDirection, normal, 
                           Dot(cylinderPointA, normal), &temp);
    if(result != None)
    {
      *interval = interval->Intersection(temp);
    }
  }

  //-------------------------------------------------------------------- Plane B
  {
    Negate(&normal);
    Interval temp = Interval::cInfinite;
    Type result = RayPlane(rayStart, rayDirection, normal, 
                           Dot(cylinderPointB, normal), &temp);
    if(result != None)
    {
      *interval = interval->Intersection(temp);
    }
  }

  //---------------------------------------------------------- Infinite Cylinder
  {
    Interval temp = Interval::cInvalid;

    // n*d
    const Vec3 relStart = rayStart - cylinderPointA;

    // m
    const real dirDotNormal = Dot(rayDirection, normal);

    // m*d
    const real startDotNormal = Dot(relStart, normal);

    // (d*d)((m*m) - r^2) - (m*d)^2 = ((m*m) - r^2) - (m*d)^2
    const real a = real(1.0) - Math::Sq(dirDotNormal);
    
    // (d*d)((m*m) - r^2) - (m*d)^2 = ((m*m) - r^2) - (m*d)^2
    const real c = (LengthSq(relStart) - Math::Sq(cylinderRadius)) - 
                    Math::Sq(startDotNormal);

    const real cCylinderEpsilon = real(0.000001);
    if(Math::Abs(a) < cCylinderEpsilon)
    {
      if(c > real(0.0))
      {
        return None;
      }
    }
    else
    {
      // 2 x ((d*d)(m*n) - (n*d)(m*d))
      const real b = real(2.0) * (Dot(relStart, rayDirection) - 
                                  startDotNormal * dirDotNormal);
      real discr = b * b - real(4.0) * a * c;
      if(discr > real(0.0))
      {
        discr = Math::Sqrt(discr);
        temp.Min = (-b - discr) / (real(2.0) * a);
        temp.Max = (-b + discr) / (real(2.0) * a);
        *interval = interval->Intersection(temp);
      }
      else
      {
        return None;
      }
    }    
  }
  
  //-------------------------------------------------------------- Test Interval
  if(!interval->IsValid() || interval->Max < real(0.0))
  {
    return None;
  }

  //Otherwise, both are negative and there is no intersection
  return Other;
}

//Intersect a ray with a cylinder defined by the points at the planar endcaps
//and the radius.
Type RayCylinder(Vec3Param rayStart, Vec3Param rayDirection, 
                 Vec3Param cylinderPointA, Vec3Param cylinderPointB, 
                 real cylinderRadius, Interval* interval)
{
  Vec3 yAxis = cylinderPointB - cylinderPointA;
  real halfLength = real(0.5) * Normalize(yAxis);
  Vec3 center = cylinderPointA + (halfLength * yAxis);

  Mat3 basis;
  MakeBasisFromY(yAxis, &basis);

  return RayCylinder(rayStart, rayDirection, center, basis, cylinderRadius, 
                     halfLength, interval);
}

//Intersect a ray with an elliptical cylinder defined by its center, local
//axes, major radius (x-axis), minor radius (z-axis), and half height (y-axis).
Type RayCylinder(Vec3Param rayStart, Vec3Param rayDirection, 
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

//Intersect a ray with an ellipsoid, the inverse scaled basis is the combination
//of the ellipsoid's basis with its radii and then inverted.
Type RayEllipsoid(Vec3Param rayStart, Vec3Param rayDirection, 
                  Vec3Param ellipsoidCenter, Mat3Param ellipsoidInvScaledBasis, 
                  Interval* interval)
{
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");

  Vec3 p = Math::Transform(ellipsoidInvScaledBasis, rayStart - ellipsoidCenter);
  Vec3 d = Math::Transform(ellipsoidInvScaledBasis, rayDirection);

  real a = LengthSq(d);
  real b = real(2.0) * Dot(d, p);
  real c = LengthSq(p) - real(1.0);

  //Exit if the ray's origin is outside the sphere (c > 0) and the ray's 
  //direction is pointing away from the sphere (b > 0).
  if(c > real(0.0) && b > real(0.0))
  {
    return None;
  }

  real discriminant = (b * b) - (real(4.0) * a * c);

  //A negative discriminant corresponds to the ray missing the sphere. A 
  //discriminant of 0 means that the ray hits the sphere tangentially, which is
  //not counted as a "hit".
  if(discriminant <= real(0.0))
  {
    return None;
  }

  //Ray now found to intersect the sphere.
  discriminant = Math::Sqrt(discriminant);
  interval->Min = (-b - discriminant) / (real(2.0) * a);
  interval->Max = (-b + discriminant) / (real(2.0) * a);
  return Segment;
}

//Intersect a ray with an ellipsoid.
Type RayEllipsoid(Vec3Param rayStart, Vec3Param rayDirection, 
                  Vec3Param ellipsoidCenter, Vec3Param ellipsoidRadii,
                  Mat3Param ellipsoidBasis, Interval* interval)
{
  //Object->World = T * R * S * v
  //World->Object = S^-1 * R^-1 * T^-1 * v
  Mat3 worldToObject;
  worldToObject.SetCross(0, ellipsoidBasis.BasisX() / ellipsoidRadii.x);
  worldToObject.SetCross(1, ellipsoidBasis.BasisY() / ellipsoidRadii.y);
  worldToObject.SetCross(2, ellipsoidBasis.BasisZ() / ellipsoidRadii.z);

  return RayEllipsoid(rayStart, rayDirection, ellipsoidCenter, worldToObject,
                      interval);
}

//Intersect a ray with a plane.
Type RayPlane(Vec3Param rayStart, Vec3Param rayDirection, Vec3Param planeNormal,
              real planeDistance, Interval* interval)
{
  const real relativeDirection = Dot(rayDirection, planeNormal);
  const real frontOrBehind = planeDistance - Dot(rayStart, planeNormal);

  //Ray's direction and plane's normal are perpendicular, ray can only intersect
  //plane if ray lies on plane. However, I will not consider this to be 
  //considered an intersection due to the unlikelihood of the infinitely thin 
  //ray lying on the infinitely thin plane.
  if(Math::Abs(relativeDirection) <= real(0.00001))
  {
    return None;
  }

  /*
                    ^            [t, INF]
        |\          |           *
          \         |          /
           \        |         /
  ----------\----------------/----------
             \              /
              \           |/
               *
            [-INF, t]
  */
  if(relativeDirection < real(0.0))
  {
    interval->Min = frontOrBehind / relativeDirection;
    interval->Max = Math::PositiveMax();
  }
  else if(relativeDirection > real(0.0))
  {
    interval->Min = -Math::PositiveMax();
    interval->Max = frontOrBehind / relativeDirection;
  }
  return Other;
}

//Intersect a ray with an oriented bounding box.
Type RayObb(Vec3Param rayStart, Vec3Param rayDirection, Vec3Param obbCenter, 
            Vec3Param obbHalfExtents, Mat3Param obbBasis, Interval* interval)
{
  //Take everything into the box's body space and use the ray-aabb function
  Vec3 newRayStart = rayStart - obbCenter;
  Math::TransposedTransform(obbBasis, &newRayStart);
  Vec3 newRayDirection = Math::TransposedTransform(obbBasis, rayDirection);
  return RayAabb(newRayStart, newRayDirection, -obbHalfExtents, obbHalfExtents, 
                 interval);
}

//Intersect a ray with a sphere.
Type RaySphere(Vec3Param rayStart, Vec3Param rayDirection, 
               Vec3Param sphereCenter, real sphereRadius, Interval* interval)
{
  Vec3 m = rayStart - sphereCenter;
  real c = Dot(m, m) - sphereRadius * sphereRadius;
  real b = Dot(m, rayDirection);

  //Exit if r's origin outside s (c > 0) and r pointing away from s (b > 0)
  if(c > real(0.0) && b > real(0.0))
  {
    return None;
  }

  real discr = b * b - c;

  //A negative discriminant corresponds to ray missing sphere. If 
  //discriminant = 0 (or close to 0 in our case), ray intersects sphere
  //tangentially and is not counted as a "hit".
  if(discr < real(0.0))
  {
    return None;
  }

  //Ray now found to intersect sphere, compute smallest value of intersection
  real sqrtDiscr = Math::Sqrt(discr);
  interval->Min = -b - sqrtDiscr;
  interval->Max = -b + sqrtDiscr;
  return Other;
}

//Intersect a ray with a tetrahedron.
Type RayTetrahedron(Vec3Param rayStart, Vec3Param rayDirection, 
                    Vec3Param tetrahedronPointA, Vec3Param tetrahedronPointB, 
                    Vec3Param tetrahedronPointC, Vec3Param tetrahedronPointD, 
                    Interval* interval)
{
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to "\
                            "function.");

  //This unique implementation was taken from the paper "Fast Ray-Tetrahedron
  //Intersection using Plucker Coordinates".

  //Fenter = NIL
  //Fleave = NIL
  *interval = Interval::cInvalid;

  bool hits[2] = { false, false };

  //Pi is the convention used in the paper to represent the normal values as 
  //Plucker coordinates.
  Vec3 piRay[2] = { rayDirection, Cross(rayDirection, rayStart) };

  //Faces are 012, 132, 302, 031
  Vec3 tetrahedron[4] = { tetrahedronPointA, tetrahedronPointB, 
                          tetrahedronPointC, tetrahedronPointD };
  uint points[4][4] = { { 0, 1, 2, 3 }, { 1, 3, 2, 0 }, 
                        { 3, 0, 2, 1 }, { 0, 3, 1, 2 } };

  //For each face
  for(uint i = 0; i < 4; ++i)
  {
    //Determine the correct winding order of the points
    Vec3 aB = tetrahedron[points[i][1]] - tetrahedron[points[i][0]];  //Tri edge
    Vec3 aC = tetrahedron[points[i][2]] - tetrahedron[points[i][0]];  //Tri edge
    Vec3 aD = tetrahedron[points[i][3]] - tetrahedron[points[i][0]];  //Other

    //Take the 3 points that are on the face and find the normal to that face
    //Get the 4th point relative to one of the 3 points on the face and check
    //to see if the resulting vector is in the same direction as the computed
    //normal.
    Vec3 normal = Cross(aB, aC);
    if(Dot(aD, normal) > real(0.0))
    {
      //If the check returns true, then swap two of the points on the face, as
      // the face is wound incorrectly
      Math::Swap(points[i][1], points[i][2]);
    }

    /*
              c
             /\
            /  \
           /    \
          /______\
         a        b
    */
    uint a = points[i][0];
    uint b = points[i][1];
    uint c = points[i][2];

    Vec3 piEdges[3][2] = { { tetrahedron[b] - tetrahedron[a],
                             Cross(tetrahedron[b], tetrahedron[a]) },
                           { tetrahedron[c] - tetrahedron[b], 
                             Cross(tetrahedron[c], tetrahedron[b]) },
                           { tetrahedron[a] - tetrahedron[c], 
                             Cross(tetrahedron[a], tetrahedron[c]) } };
    real signs[3];
    for(uint j = 0; j < 3; ++j)
    {
      signs[j] = PermutedInnerProduct(piRay[0], piRay[1], 
                                      piEdges[j][0], piEdges[j][1]);
    }

    //Counter-clockwise, ray is entering the triangle.
    if(signs[0] > real(0.0) && signs[1] > real(0.0) && signs[2] > real(0.0)
       && hits[0] == false)
    {
      real signSum = signs[0] + signs[1] + signs[2];
      real baryCoords[3] = { signs[0] / signSum, 
                             signs[1] / signSum,
                             signs[2] / signSum };
      real sum = baryCoords[0] + baryCoords[1] + baryCoords[2];
      Vec3 point = tetrahedron[a] * baryCoords[1] +
                   tetrahedron[b] * baryCoords[2] +
                   tetrahedron[c] * baryCoords[0];
      point -= rayStart;
      interval->Min = Dot(point, rayDirection);
      hits[0] = true;
    }
    //Clockwise, ray is leaving the triangle.
    else if(signs[0] < real(0.0) && signs[1] < real(0.0) && signs[2] < real(0.0)
            && hits[1] == false)
    {
      real signSum = signs[0] + signs[1] + signs[2];
      real baryCoords[3] = { signs[0] / signSum, 
                             signs[1] / signSum,
                             signs[2] / signSum };
      Vec3 point = tetrahedron[a] * baryCoords[1] +
                   tetrahedron[b] * baryCoords[2] +
                   tetrahedron[c] * baryCoords[0];
      point -= rayStart;
      interval->Max = Dot(point, rayDirection);
      hits[1] = true;
    }
  }

  if(hits[0] == false && hits[1] == false)
  {
    return None;
  }
  return Segment;
}

//Intersect a ray with a torus.
Type RayTorus(Vec3Param rayStart, Vec3Param rayDirection, Vec3Param torusCenter,
              Mat3Param torusBasis, real torusRingRadius, real torusTubeRadius,
              Interval* interval)
{
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");

  //Vector from the torus's center to the ray's start in the torus's space
  Vec3 p = rayStart - torusCenter;
  Math::TransposedTransform(torusBasis, &p);

  //Ray's direction vector in the torus's space
  Vec3 dir = Math::TransposedTransform(torusBasis, rayDirection);

  real pd = Dot(p, dir);
  real pp = Dot(p, p);
  real pSq[3] = { Math::Sq(p.x), Math::Sq(p.y), Math::Sq(p.z) };
  real r = Math::Sq(torusTubeRadius);
  real R = Math::Sq(torusRingRadius);
  real rDiff[2] = { R - r,    //Inner tube radius
                    R + r };  //Outer tube radius

  //Compute the coefficients to the parametric equation
  real a[5] = { //x^0 term
                Math::Sq(pSq[0]) + Math::Sq(pSq[1]) + Math::Sq(pSq[2]) + 
                Math::Sq(rDiff[0]) + 
                real(2.0) * (pSq[0] * pSq[1] + pSq[2] * rDiff[0]) +
                real(2.0) * (pSq[0] + pSq[1]) * (pSq[2] - rDiff[1]),
                
                //x^1 term
                real(8.0) * R * p.z * dir.z + 
                real(4.0) * pd * (pp - rDiff[1]),
                
                //x^2 term
                real(2.0) * (pp + rDiff[0]) + 
                real(4.0) * (Math::Sq(pd) - R * (real(1.0) - Math::Sq(dir.z))),

                //x^3 term
                real(4.0) * pd,

                //x^4 term
                real(1.0) };  

  real roots[4];
  uint rootCount = Math::SolveQuartic(a[0], a[1], a[2], a[3], a[4], roots);

  //No real roots, no intersection
  if(rootCount == 0)
  {
    return None;
  }

  real t = Math::PositiveMax();
  for(uint i = 0; i < rootCount; ++i)
  {
    if(roots[i] >= real(0.0) && t > roots[i])
    {
      t = roots[i];
    }
  }

  interval->Min = t;
  interval->Max = t;
  return Other;
}

//Intersect a ray with a triangle.
Type RayTriangle(Vec3Param rayStart, Vec3Param rayDirection, 
                 Vec3Param trianglePointA, Vec3Param trianglePointB, 
                 Vec3Param trianglePointC, Interval* interval, real epsilon)
{
  Vec3 aToB = trianglePointB - trianglePointA;
  Vec3 aToC = trianglePointC - trianglePointA;

  //Compute triangle normal. Can be precalculated or cached if intersecting
  //multiple rays against the same triangle.
  Vec3 normal = Cross(aToB, aToC);

  if (normal == Vec3::cZero)
    return None;

  Normalize(normal);

  real planeDistance = Dot(normal, trianglePointA);
  real frontOrBehind = planeDistance - Dot(rayStart, normal);
  real relativeDirection = Dot(rayDirection, normal);

  //Ray's direction and triangle's plane's normal are perpendicular, ray can
  //only intersect plane if ray lies on triangle's plane. However, I will not 
  //consider this to be considered an intersection due to the unlikelihood of
  //the infinitely thin ray lying on the infinitely thin plane.
  if(Math::Abs(relativeDirection) < real(0.00001))
  {
    return None;
  }

  //Ray might intersect triangle's plane, depends on where ray's starting point
  //and direction are in relation to the triangle's plane
  real t = frontOrBehind / relativeDirection;
  if(t < real(0.0))
  {
    return None;
  }

  Vec3 pointOnPlane = rayStart + t * rayDirection;

  Type result = PointTriangle(pointOnPlane, trianglePointA, trianglePointB, 
                              trianglePointC, epsilon);

  if(result == Inside)
  {
    if(relativeDirection < real(0.0))
    {
      interval->Min = t;
      interval->Max = Math::PositiveMax();
    }
    else
    {
      interval->Min = -Math::PositiveMax();
      interval->Max = t;
    }
    return Other;
  }
  return None;
}


//------------------------------------------------------------ Ray Tests (Point)

//Intersect a ray with an axis aligned bounding box.
Type RayAabb(Vec3Param rayStart, Vec3Param rayDirection, Vec3Param aabbMinPoint,
             Vec3Param aabbMaxPoint, IntersectionPoint* intersectionPoint)
{
  Interval interval;
  Type result = RayAabb(rayStart, rayDirection, aabbMinPoint, aabbMaxPoint, 
                        &interval);
  if(result != None)
  {
    if(Math::IsZero(interval.Min))
    {
      if(intersectionPoint != nullptr)
      {
        intersectionPoint->Points[0] = rayStart + (rayDirection * interval.Max);
        intersectionPoint->Points[1] = intersectionPoint->Points[0];
        intersectionPoint->T = interval.Max;
      }
      return Point;
    }
    else
    {
      if(intersectionPoint != nullptr)
      {
        intersectionPoint->Points[0] = rayStart + (rayDirection * interval.Min);
        intersectionPoint->Points[1] = rayStart + (rayDirection * interval.Max);
        intersectionPoint->T = interval.Min;
      }
      return Segment;
    }
  }
  return None;
}

//Intersect a ray with a capsule. If the result is "Segment", the second point
//isn't guaranteed to be on the surface of the capsule (for now).
Type RayCapsule(Vec3Param rayStart, Vec3Param rayDirection, 
                Vec3Param capsulePointA, Vec3Param capsulePointB, 
                real capsuleRadius, IntersectionPoint* intersectionPoint)
{
  Interval interval;
  Type result = RayCapsule(rayStart, rayDirection, capsulePointA, capsulePointB, 
                           capsuleRadius, &interval);

  //Minimum point of intersection is positive. Two intersection points.
  if(result == Other)
  {
    if(interval.Min > real(0.0))
    {
      if(intersectionPoint != nullptr)
      {
        intersectionPoint->Points[0] = rayStart + interval.Min * rayDirection;
        intersectionPoint->Points[1] = rayStart + interval.Max * rayDirection;
        intersectionPoint->T = interval.Min;
      }
      return Segment;
    }
    //Minimum point of intersection is negative, but maximum point of 
    //intersection is positive. One intersection point.
    else if(interval.Max > real(0.0))
    {
      if(intersectionPoint != nullptr)
      {
        intersectionPoint->Points[0] = rayStart + interval.Max * rayDirection;
        intersectionPoint->Points[1] = intersectionPoint->Points[0];
        intersectionPoint->T = interval.Max;
      }
      return Point;
    }
  }
  return None;
}

//Intersect a ray with a cylinder defined by its center, local axes, radius,
//and half height.
Type RayCylinder(Vec3Param rayStart, Vec3Param rayDirection, 
                 Vec3Param cylinderCenter, Mat3Param cylinderBasis, 
                 real cylinderRadius, real cylinderHalfHeight, 
                 IntersectionPoint* intersectionPoint)
{
  Interval interval;
  Type result = RayCylinder(rayStart, rayDirection, cylinderCenter, 
                            cylinderBasis, cylinderRadius, cylinderHalfHeight,
                            &interval);
  if(result != None)
  {
    //Minimum point of intersection is positive. Two intersection points.
    if(interval.Min > real(0.0))
    {
      if(intersectionPoint != nullptr)
      {
        intersectionPoint->Points[0] = rayStart + interval.Min * rayDirection;
        intersectionPoint->Points[1] = rayStart + interval.Max * rayDirection;
        intersectionPoint->T = interval.Min;
      }
      return Segment;
    }
    //Minimum point of intersection is negative, but maximum point of intersection
    //is positive. One intersection point.
    else if(interval.Max > real(0.0))
    {
      if(intersectionPoint != nullptr)
      {
        intersectionPoint->Points[0] = rayStart + interval.Max * rayDirection;
        intersectionPoint->Points[1] = intersectionPoint->Points[0];
        intersectionPoint->T = interval.Max;
      }
      return Point;
    }
  }
  return None;
}

//Intersect a ray with a cylinder defined by the points at the planar endcaps
//and the radius.
Type RayCylinder(Vec3Param rayStart, Vec3Param rayDirection, 
                 Vec3Param cylinderPointA, Vec3Param cylinderPointB, 
                 real cylinderRadius, IntersectionPoint* intersectionPoint)
{
  Interval interval;
  Type result = RayCylinder(rayStart, rayDirection, cylinderPointA, 
                            cylinderPointB, cylinderRadius, &interval);
  if(result != None)
  {
    //Minimum point of intersection is positive. Two intersection points.
    if(interval.Min > real(0.0))
    {
      if(intersectionPoint != nullptr)
      {
        intersectionPoint->Points[0] = rayStart + interval.Min * rayDirection;
        intersectionPoint->Points[1] = rayStart + interval.Max * rayDirection;
        intersectionPoint->T = interval.Min;
      }
      return Segment;
    }
    //Minimum point of intersection is negative, but maximum point of intersection
    //is positive. One intersection point.
    else if(interval.Max > real(0.0))
    {
      if(intersectionPoint != nullptr)
      {
        intersectionPoint->Points[0] = rayStart + interval.Max * rayDirection;
        intersectionPoint->Points[1] = intersectionPoint->Points[0];
        intersectionPoint->T = interval.Max;
      }
      return Point;
    }
  }
  return None;
}

//Intersect a ray with an ellipsoid.
Type RayEllipsoid(Vec3Param rayStart, Vec3Param rayDirection, 
                  Vec3Param ellipsoidCenter, Vec3Param ellipsoidRadii, 
                  Mat3Param ellipsoidBasis, 
                  IntersectionPoint* intersectionPoint)
{
  Interval interval;
  Type result = RayEllipsoid(rayStart, rayDirection, ellipsoidCenter, 
                             ellipsoidRadii, ellipsoidBasis, &interval);
  if(result != None)
  {
    if(intersectionPoint != nullptr)
    {
      if(interval.Min < real(0.0))
      {
        real t = interval.Max;
        intersectionPoint->Points[0] = rayStart + (rayDirection * t);
        intersectionPoint->Points[1] = intersectionPoint->Points[0];
        intersectionPoint->T = t;
        return Point;
      }
      else
      {
        real t = interval.Min;
        intersectionPoint->Points[0] = rayStart + (rayDirection * t);
        intersectionPoint->T = t;
        t = interval.Max;
        intersectionPoint->Points[1] = rayStart + (rayDirection * t);
        return Segment;
      }
    }
    return Other;
  }
  return None;
}

//Internal "test ray against a triangle mesh buffer" that is called by the outer
//function.
template<typename indexType>
Type RayMeshBuffer(Vec3Param rayStart, Vec3Param rayDirection, byte* vertexData, 
                   uint vertexStride, uint baseOffset, byte* indexBuffer, 
                   uint triCount, bool backfaceCulling, bool anyIntersection, 
                   uint* hitTriIndex, IntersectionPoint* intersectionPoint)
{
  indexType* indices = reinterpret_cast<indexType*>(indexBuffer);
  vertexData += baseOffset;
  
  Type endResult = None;
  IntersectionPoint tempPoint;
  real closestT = Math::PositiveMax();
  uint closestIndex = uint(-1);

  for(uint i = 0; i < triCount; ++i)
  {
    //Get the indices of the triangle.
    uint a = uint(indices[(i * 3) + 0]);
    uint b = uint(indices[(i * 3) + 1]);
    uint c = uint(indices[(i * 3) + 2]);

    //Get the points of the triangle.
    Vec3* pointA = reinterpret_cast<Vec3*>(vertexData + (a * vertexStride));
    Vec3* pointB = reinterpret_cast<Vec3*>(vertexData + (b * vertexStride));
    Vec3* pointC = reinterpret_cast<Vec3*>(vertexData + (c * vertexStride));

    {
      real ab = LengthSq(*pointA - *pointB);
      real ac = LengthSq(*pointA - *pointC);
      real bc = LengthSq(*pointB - *pointC);
      if(ab < cMinPointDist || ac < cMinPointDist || bc < cMinPointDist)
      {
        continue;
      }
    }

    //Don't test against this triangle if backface culling was requested.
    if(backfaceCulling)
    {
      //If the triangle normal and ray's direction are pointing in the same
      //direction, represented by a positive dot product, then the triangle is
      //facing away from the ray.
      Vec3 normal = Cross(*pointB - *pointA, *pointC - *pointA);
      if(Dot(normal, rayDirection) > real(0.0))
      {
        continue;
      }
    }

    //Test the ray against the triangle.
    Type result = RayTriangle(rayStart, rayDirection, *pointA, *pointB, *pointC,
                              &tempPoint);
    if(result != None)
    {
      //If any intersection will do, meaning that the actual first intersection
      //between the ray and the mesh is irrelevant, then gather all the 
      //information requested and quit out.
      if(anyIntersection)
      {
        //Save the index of the hit triangle, if it was requested.
        if(hitTriIndex != nullptr)
        {
          *hitTriIndex = i;
        }

        //Save off the point of intersection and t-value, if it was requested.
        if(intersectionPoint != nullptr)
        {
          intersectionPoint->T = closestT;
          intersectionPoint->Points[0] = rayStart + (rayDirection * closestT);
          intersectionPoint->Points[1] = intersectionPoint->Points[0];
        }
        return result;
      }

      //Save the end result and save off the t-value of the intersection if it
      //it closer than the currently saved closest t-value.
      endResult = result;
      if(tempPoint.T < closestT)
      {
        closestIndex = i;
        closestT = tempPoint.T;
      }
    }
  }

  //Save the index of the hit triangle, if it was requested.
  if(hitTriIndex != nullptr)
  {
    *hitTriIndex = closestIndex;
  }

  //Save off the point of intersection and t-value, if it was requested and 
  //if anything was hit.
  if(intersectionPoint != nullptr && endResult != None)
  {
    intersectionPoint->T = closestT;
    intersectionPoint->Points[0] = rayStart + (rayDirection * closestT);
    intersectionPoint->Points[1] = intersectionPoint->Points[0];
  }
  return endResult;
}

//Intersect a ray with an arbitrary collection of triangles. Base offset is the
//number of bytes from the beginning to the first value, vertex stride is the 
//byte distance between vertices, and the size of the index refers to the number
//of bytes used to represent the indices (generally 2 or 4 bytes).
Type RayMeshBuffer(Vec3Param rayStart, Vec3Param rayDirection, byte* vertexData, 
                   uint vertexStride, uint baseOffset, byte* indices, 
                   uint sizeOfIndex, uint triCount, bool backfaceCulling, 
                   bool anyIntersection, uint* hitTriIndex, 
                   IntersectionPoint* intersectionPoint)
{
  if(sizeOfIndex == 2)
  {
    return RayMeshBuffer<u16>(rayStart, rayDirection, vertexData, vertexStride, 
                              baseOffset, indices, triCount, backfaceCulling, 
                              anyIntersection, hitTriIndex, intersectionPoint);
  }
  else if(sizeOfIndex == 4)
  {
    return RayMeshBuffer<u32>(rayStart, rayDirection, vertexData, vertexStride, 
                              baseOffset, indices, triCount, backfaceCulling, 
                              anyIntersection, hitTriIndex, intersectionPoint);
  }
  Error("Unsupported size of index passed to the ray vs mesh buffer function.");
  return None;
}


//Intersect a ray with a plane.
Type RayPlane(Vec3Param rayStart, Vec3Param rayDirection, Vec3Param planeNormal,
              real planeDistance, IntersectionPoint* intersectionPoint)
{
  Interval interval;
  Type result = RayPlane(rayStart, rayDirection, planeNormal, planeDistance, 
                         &interval);
  if(result != None)
  {
    if(intersectionPoint != nullptr)
    {
      real t = interval.FirstT();
      intersectionPoint->Points[0] = rayStart + (rayDirection * t);
      intersectionPoint->Points[1] = intersectionPoint->Points[0];
      intersectionPoint->T = t;
    }
    return Point;
  }
  return None;
}

//Intersect a ray with an oriented bounding box.
Type RayObb(Vec3Param rayStart, Vec3Param rayDirection, Vec3Param obbCenter, 
            Vec3Param obbHalfExtents, Mat3Param obbBasis, 
            IntersectionPoint* intersectionPoint)
{
  //Take everything into the box's body space and use the ray-aabb function
  Vec3 newRayStart = rayStart - obbCenter;
  Math::TransposedTransform(obbBasis, &newRayStart);
  Vec3 newRayDirection = Math::TransposedTransform(obbBasis, rayDirection);
  Type result = RayAabb(newRayStart, newRayDirection, -obbHalfExtents, 
                        obbHalfExtents, intersectionPoint);

  if(result != None)
  {
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
  }
  return result;
}

//Intersect a ray with a sphere.
Type RaySphere(Vec3Param rayStart, Vec3Param rayDirection, 
               Vec3Param sphereCenter, real sphereRadius, 
               IntersectionPoint* intersectionPoint)
{
  //There is a faster way to test if no intersection information is required,
  //only do this check once.
  if(intersectionPoint != nullptr)
  {
    Interval interval;
    Type result = RaySphere(rayStart, rayDirection, sphereCenter, sphereRadius,
                            &interval);
    if(result != None)
    {
      //Ray starts inside the sphere
      if(interval.Min < real(0.0))
      {
        intersectionPoint->Points[0] = rayStart + (rayDirection * interval.Max);
        intersectionPoint->Points[1] = intersectionPoint->Points[0];
        intersectionPoint->T = interval.Max;
        return Point;
      }

      intersectionPoint->Points[0] = rayStart + (rayDirection * interval.Min);
      intersectionPoint->Points[1] = rayStart + (rayDirection * interval.Max);
      intersectionPoint->T = interval.Min;
      return Segment;
    }
    return None;
  }
  else
  {
    Vec3 m = rayStart - sphereCenter;
    real c = Dot(m, m) - sphereRadius * sphereRadius;

    //If there is definitely at least one real root, there must be an 
    //intersection
    if(c <= real(0.0))
    {
      return Point;
    }

    real b = Dot(m, rayDirection);

    //Early exit if ray origin outside sphere and ray pointing away from sphere
    if(b > real(0.0))
    {
      return None;
    }

    real discr = (b * b) - c;

    //A negative discriminant corresponds to ray missing sphere
    if(discr < real(0.0))
    {
      return None;
    }

    //Now ray must hit sphere
    return Other;
  }
}

//Intersect a ray with a tetrahedron.
Type RayTetrahedron(Vec3Param rayStart, Vec3Param rayDirection,
                    Vec3Param tetrahedronPointA, Vec3Param tetrahedronPointB,
                    Vec3Param tetrahedronPointC, Vec3Param tetrahedronPointD,
                    IntersectionPoint* intersectionPoint)
{
  Interval interval;
  Type result = RayTetrahedron(rayStart, rayDirection, tetrahedronPointA,
                               tetrahedronPointB, tetrahedronPointC, 
                               tetrahedronPointD, &interval);
  if(result != None)
  {
    if(intersectionPoint != nullptr)
    {
      intersectionPoint->T = interval.Min;
      intersectionPoint->Points[0] = rayStart + (interval.Min * rayDirection);
      intersectionPoint->Points[1] = rayStart + (interval.Max * rayDirection);
    }
    return result;
  }
  return None;
}

//Intersect a ray with a torus.
Type RayTorus(Vec3Param rayStart, Vec3Param rayDirection, Vec3Param torusCenter,
              Mat3Param torusBasis, real torusRingRadius, real torusTubeRadius,
              IntersectionPoint* intersectionPoint)
{
  Interval interval;
  Type result = RayTorus(rayStart, rayDirection, torusCenter, torusBasis, 
                         torusRingRadius, torusTubeRadius, &interval);
  if(result != None)
  {
    if(intersectionPoint != nullptr)
    {
      intersectionPoint->T = interval.Min;
      intersectionPoint->Points[0] = rayStart + (interval.Min * rayDirection);
      intersectionPoint->Points[1] = intersectionPoint->Points[0];
    }
    return Other;
  }
  return None;
}

//Intersect a ray with a triangle.
Type RayTriangle(Vec3Param rayStart, Vec3Param rayDirection, 
                 Vec3Param trianglePointA, Vec3Param trianglePointB, 
                 Vec3Param trianglePointC, IntersectionPoint* intersectionPoint, real epsilon)
{
  Interval interval;
  Type result = RayTriangle(rayStart, rayDirection, trianglePointA, 
                            trianglePointB, trianglePointC, &interval, epsilon);
  if(result != None)
  {
    if(intersectionPoint != nullptr)
    {
      real t = interval.FirstT();
      intersectionPoint->Points[0] = rayStart;
      intersectionPoint->Points[0] = Vector3::MultiplyAdd(intersectionPoint->Points[0], rayDirection, t);
      intersectionPoint->Points[1] = intersectionPoint->Points[0];
      intersectionPoint->T = t;
    }
    return Point;
  }
  return None;
}

}// namespace Intersection
