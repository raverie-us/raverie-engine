///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Math/Reals.hpp"
#include "Math/Vector2.hpp"
#include "Math/BoolVector3.hpp"

namespace Math
{

struct Vector3;
typedef Vector3 Vec3;
typedef const Vector3& Vec3Param;
typedef Vector3& Vec3Ref;
typedef Vector3* Vec3Ptr;

//--------------------------------------------------------------------- Vector 3
/// Three dimensional vector.
struct ZeroShared Vector3
{
  Vector3() {};
  explicit Vector3(real x, real y, real z);
  // Splat all elements
  explicit Vector3(real xyz);
  explicit Vector3(Vec2Param vec2, real z = real(0.0));
  explicit Vector3(ConstRealPointer data);

  real& operator[](uint index);
  real operator[](uint index) const;

  // Unary Operators
  Vector3 operator-() const;

  // Binary Assignment (reals)
  void operator*=(real rhs);
  void operator/=(real rhs);
  Vector3 operator*(real rhs) const;
  Vector3 operator/(real rhs) const;

  // Binary Assignment (vectors)
  void operator+=(Vec3Param rhs);
  void operator-=(Vec3Param rhs);
  void operator*=(Vec3Param rhs);
  void operator/=(Vec3Param rhs);
  Vector3 operator+(Vec3Param rhs) const;
  Vector3 operator-(Vec3Param rhs) const;
  Vector3 operator*(Vec3Param rhs) const;
  Vector3 operator/(Vec3Param rhs) const;

  // Comparison operators
  bool operator==(Vec3Param rhs) const;
  bool operator!=(Vec3Param rhs) const;
  BoolVec3 operator< (Vec3Param rhs) const;
  BoolVec3 operator<=(Vec3Param rhs) const;
  BoolVec3 operator> (Vec3Param rhs) const;
  BoolVec3 operator>=(Vec3Param rhs) const;

  /// Set all of elements to 0.
  void ZeroOut();
  /// Set each value of the vector.
  void Set(real x, real y, real z);
  /// Set all elements to the same value.
  void Splat(real xyz);


  /// Compute the dot product of two vectors.
  static real Dot(Vec3Param lhs, Vec3Param rhs);
  /// Computes the cross-product of the two vectors.
  static Vector3 Cross(Vec3Param lhs, Vec3Param rhs);
  /// Get the length of a vector.
  static real Length(Vec3Param value);
  /// Get the squared length of a vector.
  static real LengthSq(Vec3Param value);
  /// Compute the distance between two vectors.
  static real Distance(Vec3Param lhs, Vec3Param rhs);
  /// Compute the squared distance between two vectors.
  static real DistanceSq(Vec3Param lhs, Vec3Param rhs);
  /// Make the given vector have a length of 1, returns the original length.
  static real Normalize(Vec3Ref value);
  /// Calculate and return a unit-length copy of the given vector.
  static Vector3 Normalized(Vec3Param value);
  /// Try to normalize the given vector if possible. Safeguards against zero divisions.
  static real AttemptNormalize(Vec3Ref value);
  /// Attempts to return a normalized given vector. Safeguards against zero divisions.
  static Vector3 AttemptNormalized(Vec3Param value);

  /// Fused multiply add:  v0 + v1 * scalar
  static Vector3 MultiplyAdd(Vec3Param v0, Vec3Param v1, real scalar);
  /// Fused multiply subtract:  v0 - v1 * scalar
  static Vector3 MultiplySubtract(Vec3Param v0, Vec3Param v1, real scalar);

  /// Returns a copy of value with the absolute value of each element.
  static Vector3 Abs(Vec3Param value);
  /// Returns a vector with the component-wise min between two vectors.
  static Vector3 Min(Vec3Param lhs, Vec3Param rhs);
  /// Returns a vector with the component-wise max between two vectors.
  static Vector3 Max(Vec3Param lhs, Vec3Param rhs);
  /// Returns a vector where each component is clamped between min and max.
  static Vector3 Clamp(Vec3Param value, Vec3Param minValue, Vec3Param maxValue);
  /// Same as clamp, however it fills out whether or not anything was clamped.
  /// Useful when an assert message should be shown if anything was clamped.
  static Vector3 DebugClamp(Vec3Param value, Vec3Param minValue, Vec3Param maxValue, bool& wasClamped);

  /// Return a copy of this vector with each element has been floored.
  static Vector3 Floor(Vec3Param value);
  /// Return a copy of this vector with each element has been ceiled.
  static Vector3 Ceil(Vec3Param value);
  /// Return a copy of this vector with each element has been truncated.
  static Vector3 Truncate(Vec3Param value);
  /// Return a copy of this vector with each element has been rounded.
  static Vector3 Round(Vec3Param value);

  /// Linearly interpolate between the two vectors, the t-value is restricted to [0, 1].
  static Vector3 Lerp(Vec3Param start, Vec3Param end, real tValue);
  /// Spherical linear interpolation.
  /// Interpolates along the surface of the unit sphere.
  static Vector3 Slerp(Vec3Param start, Vec3Param end, real tValue);
  /// Same as slerp except this function deals with
  /// invalid vectors. Used for binding to scripting languages.
  static Vector3 SafeSlerp(Vec3Param start, Vec3Param end, real tValue);
  /// Projects the input vector onto the given vector (must be normalized)
  static Vector3 ProjectOnVector(Vec3Param input, Vec3Param normalizedVector);
  /// Projects the input vector onto a plane (the normal must be normalized)
  static Vector3 ProjectOnPlane(Vec3Param input, Vec3Param planeNormal);
  /// Calculates the reflection vector across a given vector.
  static Vector3 ReflectAcrossVector(Vec3Param input, Vec3Param normalizedVector);
  /// Calculates the reflection vector across a given plane.
  static Vector3 ReflectAcrossPlane(Vec3Param input, Vec3Param planeNormal);
  /// Calculates the refraction vector through a plane given a certain index of refraction.
  static Vector3 Refract(Vec3Param input, Vec3Param planeNormal, real refractionIndex);
  /// Get the angle between the two vectors in radians.
  static real AngleBetween(Vec3Param a, Vec3Param b);

  /// Returns if all elements of the two vectors are within epsilon of each other
  static bool ApproximatelyEqual(Vec3Param lhs, Vec3Param rhs, real epsilon);
  /// Checks to see if the values of this vector's elements are usable.
  bool Valid() const;

  /// Compute the dot product of this vector with the given vector.
  real Dot(Vec3Param rhs) const;
  /// Compute the cross product of this vector with the given vector.
  Vector3 Cross(Vec3Param rhs) const;
  /// Get the length of this vector.
  real Length() const;
  /// Get the squared length of this vector.
  real LengthSq() const;
  /// Make this vector have a length of 1, returns the original length.
  real Normalize();
  /// Calculate and return a unit-length copy of this vector.
  Vector3 Normalized() const;
  /// Attempt to give this vector a length of 1, but checks if it's possible.
  /// Instead of crashing, will return 0 if the vector was not able to be 
  /// normalized.
  real AttemptNormalize();
  Vector3 AttemptNormalized() const;
  /// Floor each component of the vector.
  void Floor();
  /// Ceil each component of the vector.
  void Ceil();
  /// Truncate each component of the vector.
  void Truncate();
  /// Round each component of the vector.
  void Round();

  /// Projects this vector onto the given vector (must be normalized)
  Vector3 ProjectOnVector(Vec3Param normalizedVector) const;
  /// Projects this onto a plane (the normal must be normalized)
  Vector3 ProjectOnPlane(Vec3Param planeNormal) const;
  /// Calculates the reflection vector across a given vector.
  Vector3 ReflectAcrossVector(Vec3Param normalizedVector) const;
  /// Calculates the reflection vector across a given plane.
  Vector3 ReflectAcrossPlane(Vec3Param planeNormal) const;
  /// Calculates the refraction vector through a plane given a certain index of refraction.
  Vector3 Refract(Vec3Param planeNormal, real refractionIndex) const;
  /// Flip this vector so it's pointing in the opposite direction.
  Vec3Ref Negate();

  union
  {
    struct  
    {
      real x, y, z;
    };
    real array[3];
  };

  static const Vector3 cZero;
  static const Vector3 cXAxis;
  static const Vector3 cYAxis;
  static const Vector3 cZAxis;
  static const Vector3 Axes[3];
};

ZeroShared Vector3 operator*(real lhs, Vec3Param rhs);
/// Compute the dot product of two vectors.
ZeroShared real Dot(Vec3Param lhs, Vec3Param rhs);
/// Computes the cross-product.
ZeroShared Vector3 Cross(Vec3Param lhs, Vec3Param rhs);
/// Get the length of a vector.
ZeroShared real Length(Vec3Param value);
/// Get the squared length of a vector.
ZeroShared real LengthSq(Vec3Param value);
/// Compute the distance between two vectors.
ZeroShared real Distance(Vec3Param lhs, Vec3Param rhs);
/// Compute the squared distance between two vectors.
ZeroShared real DistanceSq(Vec3Param lhs, Vec3Param rhs);
/// Make the given vector have a length of 1, returns the original length.
ZeroShared real Normalize(Vec3Ref value);
/// Calculate and return a unit-length copy of the given vector.
ZeroShared Vector3 Normalized(Vec3Param value);
/// Try to normalize the given vector if possible. Safeguards against zero divisions.
ZeroShared real AttemptNormalize(Vec3Ref value);
/// Attempts to return a normalized given vector. Safeguards against zero divisions.
ZeroShared Vector3 AttemptNormalized(Vec3Param value);

/// Fused multiply add:  v0 + v1 * scalar
ZeroShared Vector3 MultiplyAdd(Vec3Param v0, Vec3Param v1, real scalar);
/// Fused multiply subtract:  v0 - v1 * scalar
ZeroShared Vector3 MultiplySubtract(Vec3Param v0, Vec3Param v1, real scalar);

/// Returns a copy of value with the absolute value of each element.
ZeroShared Vector3 Abs(Vec3Param value);
/// Returns a vector with the component-wise min between two vectors.
ZeroShared Vector3 Min(Vec3Param lhs, Vec3Param rhs);
/// Returns a vector with the component-wise max between two vectors.
ZeroShared Vector3 Max(Vec3Param lhs, Vec3Param rhs);
/// Returns a vector where each component is clamped between min and max.
ZeroShared Vector3 Clamp(Vec3Param value, Vec3Param minValue, Vec3Param maxValue);
/// Same as clamp, however it fills out whether or not anything was clamped.
/// Useful when an assert message should be shown if anything was clamped.
ZeroShared Vector3 DebugClamp(Vec3Param value, Vec3Param minValue, Vec3Param maxValue, bool& wasClamped);

/// Return a copy of this vector with each element has been floored.
ZeroShared Vector3 Floor(Vec3Param value);
/// Return a copy of this vector with each element has been ceiled.
ZeroShared Vector3 Ceil(Vec3Param value);
/// Return a copy of this vector with each element has been truncated.
ZeroShared Vector3 Truncate(Vec3Param value);
/// Return a copy of this vector with each element has been rounded.
ZeroShared Vector3 Round(Vec3Param value);

/// Linearly interpolate between the two vectors, the t-value is restricted to [0, 1].
ZeroShared Vector3 Lerp(Vec3Param start, Vec3Param end, real tValue);
/// Spherical linear interpolation.
/// Interpolates along the surface of the unit sphere.
ZeroShared Vector3 Slerp(Vec3Param start, Vec3Param end, real tValue);
/// Same as slerp except this function deals with
/// invalid vectors. Used for binding to scripting languages.
ZeroShared Vector3 SafeSlerp(Vec3Param start, Vec3Param end, real tValue);

/// Projects the input vector onto the given vector (must be normalized)
ZeroShared Vector3 ProjectOnVector(Vec3Param input, Vec3Param normalizedVector);
/// Projects the input vector onto a plane (the normal must be normalized)
ZeroShared Vector3 ProjectOnPlane(Vec3Param input, Vec3Param planeNormal);
/// Calculates the reflection vector across a given vector.
ZeroShared Vector3 ReflectAcrossVector(Vec3Param input, Vec3Param normalizedVector);
/// Calculates the reflection vector across a given plane.
ZeroShared Vector3 ReflectAcrossPlane(Vec3Param input, Vec3Param planeNormal);
/// Calculates the refraction vector through a plane given a certain index of refraction.
ZeroShared Vector3 Refract(Vec3Param input, Vec3Param planeNormal, real refractionIndex);
/// Get the angle between the two vectors in radians.
ZeroShared real AngleBetween(Vec3Param a, Vec3Param b);

//-------------------------------------------------------------------Legacy
ZeroShared bool Equal(Vec3Param lhs, Vec3Param rhs, real epsilon);

/// Compute the cross product of the two given vectors for 2d.
/// The result is only the z axis of the cross product.
ZeroShared Vector3 Cross2d(Vec3Param lhs, Vec3Param rhs);

/// Flips the given vector so it's pointing in the opposite direction.
ZeroShared void Negate(Vec3Ptr vec);

/// Returns a vector pointing in the opposite direction of the given vector.
ZeroShared Vector3 Negated(Vec3Param vec);

/// Calculates and returns the given vector with its values clamped to the range
/// [min, max].
ZeroShared Vector3 Clamped(Vec3Param vec, real min, real max);

/// Returns if all values in lhs are less than all values in rhs
ZeroShared bool AllLess(Vec3Param lhs, Vec3Param rhs);

///Returns if any value in lhs is less than any value in rhs
ZeroShared bool AnyLess(Vec3Param lhs, Vec3Param rhs);

/// Returns if all values in lhs are greater than all values in rhs
ZeroShared bool AllGreater(Vec3Param lhs, Vec3Param rhs);

/// Returns if any value in lhs is greater than any value in rhs
ZeroShared bool AnyGreater(Vec3Param lhs, Vec3Param rhs);

ZeroShared real DistanceToLineSq(Vec3Param start, Vec3Param end, Vec3Param point);

}// namespace Math

#include "Vector3.inl"
