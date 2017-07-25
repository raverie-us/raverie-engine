///////////////////////////////////////////////////////////////////////////////
///
///  \file LineTests.cpp
///  Source for all of the line intersection tests.
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

void MakeBasisFromY(Vec3Param yAxis, Mat3Ptr basis)
{
  Vec3 xAxis, zAxis;
  Math::GenerateOrthonormalBasis(yAxis, &xAxis, &zAxis);
  basis->SetBasis(0, xAxis);
  basis->SetBasis(1, yAxis);
  basis->SetBasis(2, zAxis);
}

} // namespace

//-------------------------------------------------------- Line Tests (Interval)

//Intersect a line with an axis-aligned bounding box.
Type LineAabb(Vec3Param linePoint, Vec3Param lineDirection, 
              Vec3Param aabbMinPoint, Vec3Param aabbMaxPoint, 
              Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  return Unimplemented;
}

//Intersect a line with a capsule defined by its center, local axes, radius, and
//half of the distance between the centers of the spherical endcaps.
Type LineCapsule(Vec3Param linePoint, Vec3Param lineDirection,
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

//Intersect a line with a capsule defined by the centers of the spherical 
//endcaps and the radius.
Type LineCapsule(Vec3Param linePoint, Vec3Param lineDirection, 
                 Vec3Param capsulePointA, Vec3Param capsulePointB, 
                 real capsuleRadius, Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  Vec3 yAxis = capsulePointB - capsulePointA;
  real capsuleSegmentHalfLength = real(0.5) * Normalize(yAxis);
  Vec3 center = capsulePointA + (capsuleSegmentHalfLength * yAxis);

  Mat3 basis;
  MakeBasisFromY(yAxis, &basis);
  
  return LineCapsule(linePoint, lineDirection, center, basis, capsuleRadius, 
                     capsuleSegmentHalfLength, interval);
}

//Intersect a line with a cylinder defined by its center, local axes, radius,
//and half of the distance between the cylinder's endcaps.
Type LineCylinder(Vec3Param linePoint, Vec3Param lineDirection, 
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

//Intersect a line with a cylinder defined by the points in the center of either
//end and its radius.
Type LineCylinder(Vec3Param linePoint, Vec3Param lineDirection, 
                  Vec3Param cylinderPointA, Vec3Param cylinderPointB, 
                  real cylinderRadius, Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  Vec3 yAxis = cylinderPointB - cylinderPointA;
  real cylinderHalfHeight = real(0.5) * Normalize(yAxis);
  Vec3 center = cylinderPointA + (cylinderHalfHeight * yAxis);

  Mat3 basis;
  MakeBasisFromY(yAxis, &basis);

  return LineCylinder(linePoint, lineDirection, center, basis, cylinderRadius,
                      cylinderHalfHeight, interval);
}

//Intersect a line with a cylinder defined by its center, local axes, radii, and
//half of the distance between the cylinder's endcaps.
Type LineCylinder(Vec3Param linePoint, Vec3Param lineDirection, 
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

//Intersect a line with an ellipsoid defined by its center, radii, and local
//axes.
Type LineEllipsoid(Vec3Param linePoint, Vec3Param lineDirection, 
                   Vec3Param ellipsoidCenter, Vec3Param ellipsoidRadii, 
                   Mat3Param ellipsoidBasis, Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  return Unimplemented;
}

//Intersect a line with a plane defined by its normal and distance from the 
//origin along that normal.
Type LinePlane(Vec3Param linePoint, Vec3Param lineDirection, 
               Vec3Param planeNormal, real planeDistance, Interval* interval)
{
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");

  real relDir = Dot(lineDirection, planeNormal);

  //First intersection is the plane, second is infinity.
  if(relDir < real(0.0))
  {
    interval->Min = (planeDistance - Dot(linePoint, planeNormal)) / relDir;
    interval->Max = Math::PositiveMax();
  }
  else if(relDir > real(0.0))
  {
    interval->Min = -Math::PositiveMax();
    interval->Max = (planeDistance - Dot(linePoint, planeNormal)) / relDir;
  }
  else
  {
    return None;
  }

  return Other;
}

//Intersect a line with an oriented bounding box defined by its center, half 
//extents, and local axes.
Type LineObb(Vec3Param linePoint, Vec3Param lineDirection, Vec3Param obbCenter,
             Vec3Param obbHalfExtents, Mat3Param obbBasis, Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  return Unimplemented;
}

//Intersect a line with a sphere defined by its center and radius.
Type LineSphere(Vec3Param linePoint, Vec3Param lineDirection, 
                Vec3Param sphereCenter, real sphereRadius, Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  return Unimplemented;
}

//Intersect a line with a torus defined by its center, local axes, ring radius,
//and tube radius.
Type LineTorus(Vec3Param linePoint, Vec3Param lineDirection, 
               Vec3Param torusCenter, Mat3Param torusBasis, 
               real torusRingRadius, real torusTubeRadius, Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  return Unimplemented;
}

//Intersect a line with a triangle defined by its three points.
Type LineTriangle(Vec3Param linePoint, Vec3Param lineDirection, 
                  Vec3Param trianglePointA, Vec3Param trianglePointB, 
                  Vec3Param trianglePointC, Interval* interval)
{
  Error("Intersection - This function hasn't been implemented yet, you "\
        "probably shouldn't be calling this function.");
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
                            "function.");
  return Unimplemented;
}
}// namespace Intersection
