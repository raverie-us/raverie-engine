///////////////////////////////////////////////////////////////////////////////
///
/// \file Frustum.cpp
/// Implementation of the Frustum class.
///
/// Authors: Joshua Claeys
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
//---------------------------------------------------------------------- Frustum
Frustum::Frustum()
{
  //
}

Frustum::Frustum(Plane* planes)
{
  Set(planes);
}

Frustum::~Frustum()
{

}

///Sets all the planes.
void Frustum::Set(Plane* planes)
{
  for(uint i = 0; i < 6; ++i)
    Planes[i] = planes[i];
}

Plane& Frustum::Get(uint index)
{
  if(index >= 6)
  {
    String msg = String::Format("Index %d is invalid. Frustums only have 6 planes.", index);
    Error("Invalid Index", msg);
    return Planes[0];
  }

  return Planes[index];
}

void Frustum::Set(uint index, const Plane& plane)
{
  if(index >= 6)
  {
    String msg = String::Format("Index %d is invalid. Frustums only have 6 planes.", index);
    Error("Invalid Index", msg);
    return;
  }
  Planes[index] = plane;
}

///Generates a Frustum from the give points.  Point order is described above.
void Frustum::Generate(Vec3 points[8])
{
  Vec3 v0 = points[0];
  Vec3 v1 = points[1];
  Vec3 v2 = points[2];
  Vec3 v3 = points[3];
  Vec3 v4 = points[4];
  Vec3 v5 = points[5];
  Vec3 v6 = points[6];
  Vec3 v7 = points[7];

  Vec3 normalA = Cross(v3 - v2, v1 - v2).Normalized();
  Vec3 normalB = Cross(v5 - v6, v7 - v6).Normalized();
  Vec3 normalC = Cross(v1 - v2, v6 - v2).Normalized();
  Vec3 normalD = Cross(v4 - v7, v3 - v7).Normalized();
  Vec3 normalE = Cross(v6 - v2, v3 - v2).Normalized();
  Vec3 normalF = Cross(v0 - v1, v5 - v1).Normalized();

  Planes[0].Set(normalA, v0);
  Planes[1].Set(normalB, v4);
  Planes[2].Set(normalC, v1);
  Planes[3].Set(normalD, v0);
  Planes[4].Set(normalE, v3);
  Planes[5].Set(normalF, v0);
}

void Frustum::Generate(Vec3Param frontCenter, Vec3Param direction, Vec3Param up,
                       Vec3Param dimensions)
{
  Vec3 right = Cross(direction, up);

  Vec3 far = frontCenter + (direction * dimensions.z);

  Vec3 points[8];
  points[0] = frontCenter + (up * dimensions.y) + (-right * dimensions.x);
  points[1] = frontCenter + (up * dimensions.y) + (right * dimensions.x);
  points[2] = frontCenter + (-up * dimensions.y) + (right * dimensions.x);
  points[3] = frontCenter + (-up * dimensions.y) + (-right * dimensions.x);

  points[4] = far + (up * dimensions.y) + (-right * dimensions.x);
  points[5] = far + (up * dimensions.y) + (right * dimensions.x);
  points[6] = far + (-up * dimensions.y) + (right * dimensions.x);
  points[7] = far + (-up * dimensions.y) + (-right * dimensions.x);

  Generate(points);
  //Draw(points, 10.0f);
}

void Frustum::Generate(Vec3Param position, Mat3Param basis, float near, float far, float aspect, float fov)
{
  Vec3 basisX = basis.BasisX();
  Vec3 basisY = basis.BasisY();
  Vec3 basisZ = -basis.BasisZ();

  float nearScale = Math::Tan(fov * 0.5f) * near;
  float farScale = Math::Tan(fov * 0.5f) * far;

  basisX *= aspect;
  Vec3 nearZ = basisZ * near;
  Vec3 farZ = basisZ * far;

  Vec3 points[8] =
  {
    position + (-basisX + basisY) * nearScale + nearZ,
    position + ( basisX + basisY) * nearScale + nearZ,
    position + ( basisX - basisY) * nearScale + nearZ,
    position + (-basisX - basisY) * nearScale + nearZ,

    position + (-basisX + basisY) * farScale + farZ,
    position + ( basisX + basisY) * farScale + farZ,
    position + ( basisX - basisY) * farScale + farZ,
    position + (-basisX - basisY) * farScale + farZ,
  };

  Generate(points);
}

///Calculates the 8 points of the Aabb.
void Frustum::GetPoints(Vec3 points[8]) const
{
  const Plane& A = Planes[0];
  const Plane& B = Planes[1];
  const Plane& C = Planes[2];
  const Plane& D = Planes[3];
  const Plane& E = Planes[4];
  const Plane& F = Planes[5];

  //Intersect the front plane
  Intersection::Manifold mADF;
  PlanePlanePlane(A.GetData(), D.GetData(), F.GetData(), &mADF);

  Intersection::Manifold mACF;
  PlanePlanePlane(A.GetData(), C.GetData(), F.GetData(), &mACF);

  Intersection::Manifold mADE;
  PlanePlanePlane(A.GetData(), D.GetData(), E.GetData(), &mADE);

  Intersection::Manifold mACE;
  PlanePlanePlane(A.GetData(), C.GetData(), E.GetData(), &mACE);

  //Back plane
  Intersection::Manifold mBDF;
  PlanePlanePlane(B.GetData(), D.GetData(), F.GetData(), &mBDF);

  Intersection::Manifold mBCF;
  PlanePlanePlane(B.GetData(), C.GetData(), F.GetData(), &mBCF);

  Intersection::Manifold mBDE;
  PlanePlanePlane(B.GetData(), D.GetData(), E.GetData(), &mBDE);

  Intersection::Manifold mBCE;
  PlanePlanePlane(B.GetData(), C.GetData(), E.GetData(), &mBCE);

  points[0] = mADF.Points[0].Points[0];
  points[1] = mACF.Points[0].Points[0];
  points[2] = mACE.Points[0].Points[0];
  points[3] = mADE.Points[0].Points[0];
  points[4] = mBDF.Points[0].Points[0];
  points[5] = mBCF.Points[0].Points[0];
  points[6] = mBCE.Points[0].Points[0];
  points[7] = mBDE.Points[0].Points[0];
}

Aabb Frustum::GetAabb() const
{
  Vec3 points[8];
  GetPoints(points);

  Aabb aabb;
  aabb.Compute(points);
  return aabb;
}

///Tests if the given point is inside the frustum.
bool Frustum::Overlaps(Vec3Param point)
{
  for(uint i = 0; i < PlaneDim; ++i)
  {
    Plane& plane = Planes[i];
    if( plane.SignedDistanceToPlane(point) < 0)
      return false;
  }

  return true;
}

///Tests if the given Aabb is inside the frustum.
bool Frustum::Overlaps(const Aabb& aabb)
{
  return false;
}

///Tests if the given Sphere is inside the frustum.
bool Frustum::Overlaps(const Sphere& sphere)
{
  return false;
}

/*     4---------------5  A - Front Plane
      /|              /|  B - Back Plane
     / |      B     /  |  C - Right Plane
    /  |          /    |  D - Left Plane
   /   |        /      |  E - Bottom Plane
  / D  7------/--------6  F - Top Plane
 /    /     /    C   /
0---------1       /
|  /      |    /
|/   A    | /
3---------2          */

const Vec4* Frustum::GetIntersectionData() const
{
  return &Planes[0].GetData();
}

void Frustum::GetCenter(Vec3Ref center) const
{
  Vec3 points[8];
  GetPoints(points);

  center = Vec3::cZero;
  for(uint i = 0; i < 8; ++i)
    center += points[i];
  center /= real(8.0);
}

void Frustum::Support(Vec3Param direction, Vec3Ptr support) const
{
  Vec3 points[8];
  GetPoints(points);

  Vec3 center = Vec3::cZero;
  for(uint i = 0; i < 8; ++i)
    center += points[i];
  center /= real(8.0);

  real maxDot = -Math::PositiveMax();
  for(uint i = 0; i < 8; ++i)
  {
    real proj = Math::Dot(direction, points[i] - center);
    if(proj > maxDot)
    {
      maxDot = proj;
      *support = points[i];
    }
  }
}

Frustum Frustum::Transform(Mat4Param transformation) const
{
  //interestingly enough, it seems calling get points the generate
  //from those points does not return the same frustum.

  //To deal with that issue (and this might be better anyways), instead of
  //trying to transform the points, we can transform the planes of the frustum.
  //This involves transforming the normal (simple) and generating a point on
  //the plane to transform (since I have no clue if you can transform 'd' somehow).

  //to properly transform a normal you have to use the inverse transpose
  Mat4 invTransposedTransform = transformation.Inverted().Transposed();

  Frustum ret;
  for(uint i = 0; i < Frustum::PlaneDim; ++i)
  {
    Vec3 normal = Planes[i].GetNormal();
    Vec3 point = Planes[i].GetDistance() * normal;

    Vec3 newNormal = Math::TransformNormal(invTransposedTransform, normal).Normalized();
    Vec3 newPoint = Math::TransformPoint(transformation, point);

    ret.Planes[i].Set(newNormal, newPoint);
  }
  return ret;
}

Frustum Frustum::UniformTransform(Mat4Param transformation) const
{
  return Transform(transformation);
}

Frustum Frustum::TransformInverse(Mat4Param transformation) const
{
  Mat4 invTransform = transformation.Inverted();
  //to properly transform a normal you have to use the inverse
  //transpose, which is just the transpose (since we were inverting)
  Mat4 transposedTransform = transformation.Transposed();

  Frustum ret;
  for(uint i = 0; i < Frustum::PlaneDim; ++i)
  {
    Vec3 normal = Planes[i].GetNormal();
    Vec3 point = Planes[i].GetDistance() * normal;

    Vec3 newNormal = Math::TransformNormal(transposedTransform, normal).Normalized();
    Vec3 newPoint = Math::TransformPoint(invTransform, point);

    ret.Planes[i].Set(newNormal, newPoint);
  }
  return ret;
}

void Frustum::PointsAtDepth(Vec3 boxPoints[4], float depth) const
{
  // Get all the points of the frustum
  Vec3 points[8];
  GetPoints(points);

  // Compute center points of frustum using the mid point of diagonal corners
  Vec3 nearCenter = (points[2] + points[0]) * 0.5f;
  Vec3 farCenter = (points[4] + points[6]) * 0.5f;

  // Compute the eye/center vector
  Vec3 centerDir = (farCenter - nearCenter);
  float nearToFar = centerDir.Normalize();

  // Build a view aligned plane at depth
  Plane plane(centerDir, nearCenter + centerDir * depth);

  // Build edge vector from the upper left corner
  Vec3 edgeDir = points[4] - points[0];
  Vec3 edgeStart = points[0];

  // Intersect plane at depth with edge ray
  Intersection::IntersectionPoint point;
  Intersection::Type type = Intersection::RayPlane(edgeStart, edgeDir, plane.GetNormal(), plane.GetDistance(), &point);

  // All edges have the same normalized t
  float t = point.T;

  // Compute all the edge points
  boxPoints[0] = Math::Lerp(points[0], points[4], t);
  boxPoints[1] = Math::Lerp(points[1], points[5], t);
  boxPoints[2] = Math::Lerp(points[2], points[6], t);
  boxPoints[3] = Math::Lerp(points[3], points[7], t);
}

}//namespace Zero
