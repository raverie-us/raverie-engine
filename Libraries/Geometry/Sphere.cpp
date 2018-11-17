///////////////////////////////////////////////////////////////////////////////
///
/// \file Sphere.cpp
/// Implementation of the Sphere class.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

Sphere::Sphere()
{
  mCenter.ZeroOut();
  mRadius = real(0.0);
}

Sphere::Sphere(Vec3Param center, real radius)
{
  mCenter = center;
  mRadius = radius;
}

bool Sphere::Overlap(const Sphere& rhs)
{
  Intersection::Type type = Intersection::SphereSphere(mCenter, mRadius, 
                                                       rhs.mCenter,rhs.mRadius);
  return type == Intersection::Point;
}

void Sphere::Compute(const Vec3Array& pts)
{
  mCenter.ZeroOut();
  uint size = pts.Size();
  ErrorIf(size == 0,"Computing the bounding sphere of a vector of zero points.");
  for(uint i = 0; i < size; ++i)
    mCenter += pts[i];
  mCenter /= static_cast<real>(size);

  real maxLengthSq = 0.0f;
  for(uint i = 0; i < size; ++i)
  {
    Vec3 r = pts[i] - mCenter;
    real lengthSq = r.LengthSq();
    if(lengthSq > maxLengthSq)
      maxLengthSq = lengthSq;
  }

  mRadius = Math::Sqrt(maxLengthSq);
}

void Sphere::Compute(const Aabb& aabb)
{
  mCenter = aabb.GetCenter();
  Vec3 halfExtents = aabb.GetHalfExtents();
  mRadius = halfExtents.Length();
}

void Sphere::Expand(Vec3Param point)
{
  Vec3 diff = point - mCenter;
  float sqDist = Math::LengthSq(diff);

  // Make sure that the new point isn't already contained in the sphere
  if(sqDist > mRadius * mRadius)
  {
    // This effectively computes the point on the "opposite" side of the sphere from where the point is 
    // and then constructs a new sphere from this point and the given point.
    float oldRadius = mRadius;
    float dist = Math::Sqrt(sqDist);
    float newRadius = (dist + mRadius) * 0.5f;

    Vec3 normalizedDir = diff / dist;
    mRadius = newRadius;
    mCenter += normalizedDir * (newRadius - oldRadius);
  }
}

Sphere Sphere::Expand(const Sphere& sphere, Vec3Param point)
{
  Sphere result = sphere;
  result.Expand(point);
  return sphere;
}

real Sphere::GetVolume() const
{
  return (real(4.0) / real(3.0)) * Math::cPi * mRadius * mRadius * mRadius;
}

real Sphere::GetSurfaceArea() const
{
  return real(4.0) * Math::cPi * mRadius * mRadius;
}

void Sphere::GetCenter(Vec3Ref center) const
{
  center = mCenter;
}

void Sphere::Support(Vec3Param direction, Vec3Ptr support) const
{
  Geometry::SupportSphere(direction,mCenter,mRadius,support);
}

void Sphere::DebugDraw() const
{
  gDebugDraw->Add(Debug::Sphere(*this));
}

Ellipsoid Sphere::Transform(Mat4Param transformation) const
{
  Vec3 radii;
  radii.Splat(mRadius);
  Mat4 t = Math::BuildTransform(mCenter,Mat3::cIdentity,radii);
  Mat4 worldT = transformation * t;

  Ellipsoid ret;
  worldT.Decompose(&ret.Radii,&ret.Basis,&ret.Center);
  return ret;
}

Sphere Sphere::UniformTransform(Mat4Param transformation) const
{
  //since this is a uniform transform (uniform scale) then we know we'll be
  //returning a sphere. The easiest way to write this is the same as transforming
  //to an ellipsoid, just assuming that the resultant radii are all equal.
  Ellipsoid ellipsoid = Transform(transformation);

  Sphere ret;
  ret.mCenter = ellipsoid.Center;
  ret.mRadius = ellipsoid.Radii[0];
  return ret;
}

}//namespace Zero
