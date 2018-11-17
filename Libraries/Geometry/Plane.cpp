///////////////////////////////////////////////////////////////////////////////
///
/// \file Plane.cpp
/// Implementation of the Plane class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

Plane::Plane()
{
  mData.ZeroOut();
}

Plane::Plane(Vec3Param normal, real distance)
{
  Set(normal, distance);
}

Plane::Plane(Vec3Param normal, Vec3Param position)
{
  Set(normal, position);
}

Plane::Plane(Vec4Param data)
{
  Set(data);
}

Plane::~Plane()
{

}

void Plane::Flip()
{
  mData *= real(-1.0);
}

void Plane::Set(Vec4Param data)
{
  mData = data;
}

///Sets the normal and distance to the origin of the plane.
void Plane::Set(Vec3Param normal, real distance)
{
  mData.Set(normal.x, normal.y, normal.z, distance);
}

///Sets the normal and distance to the plane (read the explanation of 
///the SetPosition function for an explanation of how the position is treated.
void Plane::Set(Vec3Param normal, Vec3Param position)
{
  //Calculate the distance.
  real distance = position.Dot(normal);
  //Set the data.
  mData.Set(normal.x, normal.y, normal.z, distance);
}

///Returns the normal of the plane.
Vec3 Plane::GetNormal() const
{
  //mData is a Vec3, so we need to cast it to a Vec3.
  return *(Vec3*)(&mData);
}

///Returns the distance from the plane to the origin.
real Plane::GetDistance() const
{
  return mData.w;
}

///Sets the passed in values to the normal and distance from the plane to
/// the origin.
void Plane::GetNormalAndDistance(Vec3Ref normal, real& distance) const
{
  normal = GetNormal();
  distance = GetDistance();
}

real Plane::SignedDistanceToPlane(Vec3Param point)
{
  return Geometry::SignedDistanceToPlane(point, GetNormal(), GetDistance());
}

///Returns both the normal and the distance to the origin in a Vec4.
///Read the comment above mData to see how the memory is laid out.
const Vec4& Plane::GetData() const
{
  return mData;
}

}//namespace Zero
