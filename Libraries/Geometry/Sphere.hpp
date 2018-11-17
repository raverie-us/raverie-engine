///////////////////////////////////////////////////////////////////////////////
///
/// \file Sphere.hpp
/// Declaration of the Sphere class.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct Aabb;

struct Sphere
{
  Sphere();
  Sphere(Vec3Param center, real radius);

  ///Returns whether or not the two sphere's overlap.
  bool Overlap(const Sphere& rhs);

  ///Computes the bounding sphere of a vector of points.
  void Compute(const Vec3Array& pts);
  ///Computes the bounding sphere of the bounding box.
  void Compute(const Aabb& aabb);

  /// Minimally expand the sphere by the given point
  void Expand(Vec3Param point);
  static Sphere Expand(const Sphere& sphere, Vec3Param point);

  ///Computes the volume of the sphere.
  real GetVolume() const;
  ///Computes the surface area of the sphere.
  real GetSurfaceArea() const;

  void GetCenter(Vec3Ref center) const;
  void Support(Vec3Param direction, Vec3Ptr support) const;

  void DebugDraw() const;

  ///Transforms this sphere, the result is an ellipsoid
  ///(even though it could be a spherical ellipsoid).
  Ellipsoid Transform(Mat4Param transformation) const;
  Sphere UniformTransform(Mat4Param transformation) const;
  ///Typedef for templated code to know what the transformed type is.
  typedef Ellipsoid TransformedShapeType;

  Vec3 mCenter;
  real mRadius;
};

}//namespace Zero
