///////////////////////////////////////////////////////////////////////////////
///
/// \file Aabb.hpp
/// Declaration of the Aabb class.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

///Axis aligned bounding box structure. An Aabb is used to get a quick 
///approximation of the volume that an object takes up and it is very useful 
///when used in broadphase spacial partitioning algorithms.
struct Aabb
{
  Aabb();
  Aabb(Vec3Param center, Vec3Param halfExtents);
  ~Aabb();

  ///Sets the aabb to an invalid value (inf, -inf) which allows
  ///expanding by any point without accidentally including zero.
  void SetInvalid();
  bool Valid() const;
  void AttemptToCorrectInvalid();

  ///Returns whether or not the two aabb's overlap.
  bool Overlap(const Aabb& rhs) const;
  ///Returns whether or not the two aabb's are overlapping on the given axis.
  bool OverlapAxis(const Aabb& rhs, uint axis);

  ///Returns whether or not the given point is inside the Aabb.
  bool ContainsPoint(Vec3Param point);

  ///Combines two Aabb's. Used when a new Aabb is wanted that Contains both
  ///this Aabb and the one passed in.
  void Combine(const Aabb& aabb);
  Aabb Combined(const Aabb& aabb) const;
  static Aabb Combine(const Aabb& lhs, const Aabb& rhs);

  ///Computes the Aabb of the point. This is useful when the Aabb
  ///needs to be reset so that it centers around a given point.
  void Compute(Vec3Param pt);
  void Set(Vec3Param pt) { Compute(pt); }
  ///Computes the Aabb of a vector of points.
  void Compute(const Vec3Array& pts);
  void Compute(const Vec3 pts[8]);
  void Compute(const Vec3* pts, uint count);
  ///Expands the current Aabb by the point passed in.
  void Expand(Vec3Param pt);
  ///Expands the current Aabb by the points passed in.
  void Expand(const Vec3Array& pts);
  /// Returns the current aabb expanded by the given point
  Aabb Expanded(Vec3Param pt) const;
  ///Expands the current Aabb by the point passed in.
  static Aabb Expand(const Aabb& aabb, Vec3Param pt);

  ///Computes a new Aabb the encompasses this one if it was rotated. This is
  ///an approximation that is quicker to calculate than rotating each point
  ///and calculating a new Aabb from there. A note is that this should be
  ///computed from a base Aabb each time, otherwise it will grow continuously.
  void Transform(Mat3Param rot);

  /// Computes an aabb of this aabb after it is transform by the given values.
  Aabb TransformAabbInternal(Vec3Param worldScale, Mat3Param worldRotation, Vec3Param worldTranslation) const;
  /// Transform's a local space aabb into world space. This transform properly deals with
  // a local-space aabb that isn't centered at the origin
  Aabb TransformAabb(Vec3Param worldScale, Mat3Param worldRotation, Vec3Param worldTranslation) const;
  /// First properly translates the aabb to the center, then transforms by the
  /// passed in transformation, then properly translates back with the correct offset.
  Aabb TransformAabb(Mat4Param transformation) const;

  ///Transforms the Aabb into an Obb.
  Obb Transform(Mat4Param transformation) const;
  Obb UniformTransform(Mat4Param transformation) const;
  ///Typedef for templated code to know what the transformed type is.
  typedef Obb TransformedShapeType;

  ///Translates the bounding box by the passed in vector.
  void Translate(Vec3Param translation);

  ///Returns the given Aabb transformed by the inverse 
  ///of the given transformation matrix.
  static Aabb InverseTransform(const Aabb& rhs, Mat4Param worldTransform);

  ///Gets the center of the Aabb.
  Vec3 GetCenter() const;
  ///Gets the Half Extents of the Aabb. The Half Extents is the vector from the
  ///center of the Aabb to the upper right corner. This is also equivalent to
  ///a vector containing the half scale of the x, y and z axis. This is used in
  ///rotating the Aabb and can be useful in space partition constructions.
  Vec3 GetHalfExtents() const;
  ///Gets the full extents of the aabb. Effectively the size.
  Vec3 GetExtents() const;
  ///Gets both the center and the half extents of the Aabb. It is quicker
  ///to compute both together instead of individually. See GetHalfExtents for
  ///an explanation of a half extents.
  void GetCenterAndHalfExtents(Vec3Ref center, Vec3Ref halfExtents) const;

  ///Sets the center of the Aabb.
  void SetCenter(Vec3Param center);
  ///Sets the half extents of the Aabb. See GetHalfExtents for an explanation
  ///of a half extents.
  void SetHalfExtents(Vec3Param halfExtents);
  void SetExtents(Vec3Param extents);
  ///Sets the center and the half extents of the Aabb. It is quicker to set
  ///both simultaneously than individually. See GetHalfExtents for an explanation
  //of a half extents.
  void SetCenterAndHalfExtents(Vec3Param center, Vec3Param halfExtents);
  ///Sets the mins and maxs to the passed in values.
  void SetMinAndMax(Vec3Param mins, Vec3Param maxs);

  ///Computes the volume of the Aabb.
  real GetVolume() const;

  ///Computes the surface area of the Aabb.
  real GetSurfaceArea() const;

  void GetCenter(Vec3Ref center) const;
  void Support(Vec3Param direction, Vec3Ptr support) const;

  void DebugDraw() const;

  ///Zeros out the aabb.
  void Zero(void);

  Vec3 mMin,mMax;
};

}//namespace Zero
