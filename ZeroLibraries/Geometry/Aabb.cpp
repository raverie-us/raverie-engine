///////////////////////////////////////////////////////////////////////////////
///
/// \file Aabb.cpp
/// Implementation of the Aabb class.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

Aabb::Aabb()
{
  SetInvalid();
}

Aabb::Aabb(Vec3Param center, Vec3Param halfExtents)
{
  SetCenterAndHalfExtents(center, halfExtents);
}

Aabb::~Aabb()
{

}

void Aabb::SetInvalid()
{
  mMin.Splat(Math::PositiveMax());
  mMax.Splat(-Math::PositiveMax());
}

bool Aabb::Valid() const
{
  return !((mMin.x > mMax.x) || (mMin.y > mMax.y) || (mMin.z > mMax.z));
}


void Aabb::AttemptToCorrectInvalid()
{
  // Average all the extents of the aabb to try and make a reasonable center, then use that (a zero sized aabb)
  SetCenterAndHalfExtents(GetCenter(), Vec3::cZero);

  // If the object is still not valid (could be nans, infs, etc) then just zero it out
  if(!Valid())
    Zero();
}

bool Aabb::Overlap(const Aabb& rhs) const
{
  Intersection::IntersectionType type = Intersection::AabbAabb(mMin, mMax, rhs.mMin, 
                                                   rhs.mMax);
  return type == Intersection::Point;
}

bool Aabb::OverlapAxis(const Aabb& rhs, uint axis)
{
  if(mMax[axis] < rhs.mMin[axis] || mMin[axis] > rhs.mMax[axis])
    return false;
  return true;
}

bool Aabb::ContainsPoint(Vec3Param point)
{
  return Intersection::PointAabb(point, mMin, mMax) == Intersection::Inside;
}

void Aabb::Combine(const Aabb& aabb)
{
  //get the min on each axis of this aabb and the one passed in
  mMin[0] = Math::Min(mMin[0],aabb.mMin[0]);
  mMin[1] = Math::Min(mMin[1],aabb.mMin[1]);
  mMin[2] = Math::Min(mMin[2],aabb.mMin[2]);

  //get the max on each axis of this aabb and the one passed in
  mMax[0] = Math::Max(mMax[0],aabb.mMax[0]);
  mMax[1] = Math::Max(mMax[1],aabb.mMax[1]);
  mMax[2] = Math::Max(mMax[2],aabb.mMax[2]);
}

Aabb Aabb::Combined(const Aabb& aabb) const
{
  Aabb retAabb = *this;
  retAabb.Combine(aabb);
  return retAabb;
}

Aabb Aabb::Combine(const Aabb& lhs, const Aabb& rhs)
{
  return lhs.Combined(rhs);
}

void Aabb::Compute(Vec3Param pt)
{
  mMin[0] = mMax[0] = pt[0];
  mMin[1] = mMax[1] = pt[1];
  mMin[2] = mMax[2] = pt[2];
}

void Aabb::Compute(const Vec3Array& pts)
{
  //should assert here that pts is not empty

  Vec3Array::const_iterator cIt = pts.Begin();

  //set the initial aabb dimensions
  Compute(*cIt);
  ++cIt;

  //expand the aabb for the remaining points
  while(cIt != pts.End())
  {
    Expand(*cIt);
    ++cIt;
  }
}

void Aabb::Compute(const Vec3 pts[8])
{
  Compute(pts, 8);
}

void Aabb::Compute(const Vec3* pts, uint count)
{
  Compute(pts[0]);

  for(uint i = 1; i < count; ++i)
    Expand(pts[i]);
}

void Aabb::Expand(Vec3Param pt)
{
  //get the min on each axis of what
  //we already had and the point passed in
  mMin[0] = Math::Min(mMin[0],pt[0]);
  mMin[1] = Math::Min(mMin[1],pt[1]);
  mMin[2] = Math::Min(mMin[2],pt[2]);

  //get the max on each axis of what
  //we already had and the point passed in
  mMax[0] = Math::Max(mMax[0],pt[0]);
  mMax[1] = Math::Max(mMax[1],pt[1]);
  mMax[2] = Math::Max(mMax[2],pt[2]);
}

void Aabb::Expand(const Vec3Array& pts)
{
  Vec3Array::const_iterator cIt = pts.Begin();

  //expand the aabb by all of the points
  while(cIt != pts.End())
  {
    Expand(*cIt);
    ++cIt;
  }
}

Aabb Aabb::Expanded(Vec3Param pt) const
{
  Aabb result = *this;
  result.Expand(pt);
  return result;
}

Aabb Aabb::Expand(const Aabb& aabb, Vec3Param pt)
{
  return aabb.Expanded(pt);
}

void Aabb::Transform(Mat3Param rot)
{
  Vec3 e = GetHalfExtents();
  Vec3 halfExtents;

  //some black magic from real time collision detection
  for(uint i = 0; i < 3; ++i)
  {
    halfExtents[i] = Math::Abs(rot(i, 0)) * e[0];
    halfExtents[i] += Math::Abs(rot(i, 1)) * e[1];
    halfExtents[i] += Math::Abs(rot(i, 2)) * e[2];
  }

  SetHalfExtents(halfExtents);
}

Aabb Aabb::TransformAabbInternal(Vec3Param worldScale, Mat3Param worldRotation, Vec3Param worldTranslation) const
{
  Aabb result = *this;

  Vec3 center, halfExtents;
  result.GetCenterAndHalfExtents(center, halfExtents);
  halfExtents *= worldScale;
  result.SetCenterAndHalfExtents(center, halfExtents);

  result.Translate(worldTranslation);
  result.Transform(worldRotation);
  return result;
}

Aabb Aabb::TransformAabb(Vec3Param worldScale, Mat3Param worldRotation, Vec3Param worldTranslation) const
{
  Aabb result = *this;
  //compute the offset of the aabb from origin in local space
  Vec3 center = result.GetCenter();
  //put the aabb at the origin so we can scale and rotate it
  result.Translate(-center);
  //scale, rotate and world translate it
  result = result.TransformAabbInternal(worldScale, worldRotation, worldTranslation);
  //now rotate and scale that local offset to get the final world translation
  result.Translate(Math::Transform(worldRotation, worldScale * center));
  return result;
}

Aabb Aabb::TransformAabb(Mat4Param transformation) const
{
  Math::DecomposedMatrix4 decomposedMatrix(transformation);
  return TransformAabb(decomposedMatrix.Scale, decomposedMatrix.Rotation, decomposedMatrix.Translation);
}

Obb Aabb::Transform(Mat4Param transformation) const
{
  Vec3 center, halfExtents;
  GetCenterAndHalfExtents(center, halfExtents);

  Obb ret;
  transformation.Decompose(&ret.HalfExtents, &ret.Basis, &ret.Center);

  ret.Center = Math::TransformPoint(transformation, center);
  ret.HalfExtents *= halfExtents;

  return ret;
}

Obb Aabb::UniformTransform(Mat4Param transformation) const
{
  return Transform(transformation);
}

void Aabb::Translate(Vec3Param translation)
{
  mMin += translation;
  mMax += translation;
}

Aabb Aabb::InverseTransform(const Aabb& rhs, Mat4Param worldTransform)
{
  //Pull out the translation
  Vec3 translation = worldTransform.GetBasis3(3);

  //Pull out the inverted rotation and scale into a matrix3
  Mat3 scaleRot = Math::ToMatrix3(worldTransform.Inverted());
  

  //Calculate the center of the new Aabb
  Vec3 center = Math::Transform(scaleRot, rhs.GetCenter() - translation);

  //Create the new Aabb
  Aabb newAabb;
  newAabb.SetCenterAndHalfExtents(center, rhs.GetHalfExtents());
  newAabb.Transform(scaleRot);

  return newAabb;
}

Vec3 Aabb::GetCenter() const
{
  return (mMin + mMax) / 2.0f;
}

Vec3 Aabb::GetHalfExtents() const
{
  return (mMax - mMin) / 2.0f;
}

Vec3 Aabb::GetExtents() const
{
  return mMax - mMin;
}

void Aabb::GetCenterAndHalfExtents(Vec3Ref center, Vec3Ref halfExtents) const
{
  center = (mMin + mMax) / 2.0f;
  halfExtents = mMax - center;
}

void Aabb::SetCenter(Vec3Param center)
{
  Vec3 offset = center - GetCenter();
  mMin += offset;
  mMax += offset;
}

void Aabb::SetHalfExtents(Vec3Param halfExtents)
{
  Vec3 offset = halfExtents - GetHalfExtents();
  mMin -= offset;
  mMax += offset;
}

void Aabb::SetExtents(Vec3Param extents)
{
  SetHalfExtents(extents * real(0.5));
}

void Aabb::SetCenterAndHalfExtents(Vec3Param center, Vec3Param halfExtents)
{
  mMin = center - halfExtents;
  mMax = center + halfExtents;
}

void Aabb::SetMinAndMax(Vec3Param mins, Vec3Param maxs)
{
  mMin = mins;
  mMax = maxs;
}

real Aabb::GetVolume() const
{
  ////volume is the product of the width, height and depth
  Vec3 scale = mMax - mMin;
  return scale[0] * scale[1] * scale[2];
}

real Aabb::GetSurfaceArea() const
{
  real surfaceArea = 0;
  Vec3 extents = mMax - mMin;

  //Top / Bottom
  surfaceArea += real(2.0) * (extents.x * extents.z);

  //Front / Back
  surfaceArea += real(2.0) * (extents.x * extents.y);

  //Left / Right
  surfaceArea += real(2.0) * (extents.z * extents.y);

  return surfaceArea;
}

void Aabb::GetCenter(Vec3Ref center) const
{
  center = GetCenter();
}

void Aabb::Support(Vec3Param direction, Vec3Ptr support) const
{
  Geometry::SupportAabb(direction,mMin,mMax,support);
}

void Aabb::DebugDraw() const
{
  gDebugDraw->Add(Debug::Obb(*this));
}

void Aabb::Zero(void)
{
  mMin.ZeroOut();
  mMax.ZeroOut();
}

}//namespace Zero
