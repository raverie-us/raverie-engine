///////////////////////////////////////////////////////////////////////////////
///
/// \file Plane.hpp
/// Declaration of the Plane class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

///Infinite plane defined by a normal and a distance to the origin.
struct Plane
{
  Plane();
  Plane(Vec3Param normal, real distance);
  Plane(Vec3Param normal, Vec3Param position);
  Plane(Vec4Param data);
  ~Plane();

  ///Flips the planes normal (keeps it's "position").
  void Flip();

  /// Sets the data (normal and d) in the form of a Vec4.
  /// Read the comment above mData to see how the memory is laid out.
  void Set(Vec4Param data);
  /// Sets the normal and distance to the origin of the plane.
  void Set(Vec3Param normal, real distance);
  /// Sets the normal and distance to the plane (read the explanation of 
  /// the SetPosition function for an explanation of how the position is treated.
  void Set(Vec3Param normal, Vec3Param position);

  /// Returns the normal of the plane.
  Vec3 GetNormal() const;
  /// Returns the distance from the plane to the origin.
  real GetDistance() const;
  /// Sets the passed in values to the normal and distance from the plane to
  /// the origin.
  void GetNormalAndDistance(Vec3Ref normal, real& distance) const;

  /// Returns the distance from the given point to the plane.
  real SignedDistanceToPlane(Vec3Param point);

  /// Returns both the normal and the distance to the origin in a Vec4.
  /// Read the comment above mData to see how the memory is laid out.
  const Vec4& GetData() const;

public:
  //Holds the normal of the plane in the first 3 elements (xyz) and the distance
  //to the origin in the 4th element (w).
  Vec4 mData;
};

}//namespace Zero
