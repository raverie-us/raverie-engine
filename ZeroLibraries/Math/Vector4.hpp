///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Math/Reals.hpp"
#include "Math/BoolVector4.hpp"

namespace Math
{

struct Vector4;
typedef Vector4 Vec4;
typedef const Vector4& Vec4Param;
typedef Vector4& Vec4Ref;
typedef Vector4* Vec4Ptr;

//--------------------------------------------------------------------- Vector 4
/// Four dimensional vector
struct ZeroShared Vector4
{
  Vector4() {};
  explicit Vector4(real x, real y, real z, real w);
  explicit Vector4(real xyzw);
  explicit Vector4(ConstRealPointer data);

  real& operator[](uint index);
  real operator[](uint index) const;

  //Unary Operators
  Vector4 operator-() const;

  // Binary Assignment (reals)
  void operator*=(real rhs);
  void operator/=(real rhs);
  Vector4 operator*(real rhs) const;
  Vector4 operator/(real rhs) const;

  // Binary Assignment (vectors)
  void operator+=(Vec4Param rhs);
  void operator-=(Vec4Param rhs);
  void operator*=(Vec4Param rhs);
  void operator/=(Vec4Param rhs);
  Vector4 operator+(Vec4Param rhs) const;
  Vector4 operator-(Vec4Param rhs) const;
  Vector4 operator*(Vec4Param rhs) const;
  Vector4 operator/(Vec4Param rhs) const;

  // Comparison operators
  bool operator==(Vec4Param rhs) const;
  bool operator!=(Vec4Param rhs) const;
  BoolVec4 operator< (Vec4Param rhs) const;
  BoolVec4 operator<=(Vec4Param rhs) const;
  BoolVec4 operator> (Vec4Param rhs) const;
  BoolVec4 operator>=(Vec4Param rhs) const;

  /// Set all of elements to 0.
  void ZeroOut();
  /// Set each value of the vector.
  void Set(real x, real y, real z, real w);
  /// Set all elements to the same value.
  void Splat(real xyzw);


  /// Compute the dot product of two vectors.
  static real Dot(Vec4Param lhs, Vec4Param rhs);
  /// Get the length of a vector.
  static real Length(Vec4Param value);
  /// Get the squared length of a vector.
  static real LengthSq(Vec4Param value);
  /// Compute the distance between two vectors.
  static real Distance(Vec4Param lhs, Vec4Param rhs);
  /// Compute the squared distance between two vectors.
  static real DistanceSq(Vec4Param lhs, Vec4Param rhs);
  /// Make the given vector have a length of 1, returns the original length.
  static real Normalize(Vec4Ref value);
  /// Calculate and return a unit-length copy of the given vector.
  static Vector4 Normalized(Vec4Param value);
  /// Try to normalize the given vector if possible. Safeguards against zero divisions.
  static real AttemptNormalize(Vec4Ref value);
  /// Attempts to return a normalized given vector. Safeguards against zero divisions.
  static Vector4 AttemptNormalized(Vec4Param value);

  /// Fused multiply add:  v0 + v1 * scalar
  static Vector4 MultiplyAdd(Vec4Param v0, Vec4Param v1, real scalar);
  /// Fused multiply subtract:  v0 - v1 * scalar
  static Vector4 MultiplySubtract(Vec4Param v0, Vec4Param v1, real scalar);

  /// Returns a copy of value with the absolute value of each element.
  static Vector4 Abs(Vec4Param value);
  /// Returns a vector with the component-wise min between two vectors.
  static Vector4 Min(Vec4Param lhs, Vec4Param rhs);
  /// Returns a vector with the component-wise max between two vectors.
  static Vector4 Max(Vec4Param lhs, Vec4Param rhs);
  /// Returns a vector where each component is clamped between min and max.
  static Vector4 Clamp(Vec4Param value, Vec4Param minValue, Vec4Param maxValue);
  /// Same as clamp, however it fills out whether or not anything was clamped.
  /// Useful when an assert message should be shown if anything was clamped.
  static Vector4 DebugClamp(Vec4Param value, Vec4Param minValue, Vec4Param maxValue, bool& wasClamped);

  /// Return a copy of this vector with each element has been floored.
  static Vector4 Floor(Vec4Param value);
  /// Return a copy of this vector with each element has been ceiled.
  static Vector4 Ceil(Vec4Param value);
  /// Return a copy of this vector with each element has been truncated.
  static Vector4 Truncate(Vec4Param value);
  /// Return a copy of this vector with each element has been rounded.
  static Vector4 Round(Vec4Param value);

  /// Linearly interpolate between the two vectors, the t-value is restricted to [0, 1].
  static Vector4 Lerp(Vec4Param start, Vec4Param end, real tValue);
  /// Projects the input vector onto the given vector (must be normalized)
  static Vector4 ProjectOnVector(Vec4Param input, Vec4Param normalizedVector);
  /// Projects the input vector onto a plane (the normal must be normalized)
  static Vector4 ProjectOnPlane(Vec4Param input, Vec4Param planeNormal);
  /// Calculates the reflection vector across a given vector.
  static Vector4 ReflectAcrossVector(Vec4Param input, Vec4Param normalizedVector);
  /// Calculates the reflection vector across a given plane.
  static Vector4 ReflectAcrossPlane(Vec4Param input, Vec4Param planeNormal);
  /// Calculates the refraction vector through a plane given a certain index of refraction.
  static Vector4 Refract(Vec4Param input, Vec4Param planeNormal, real refractionIndex);
  /// Get the angle between the two vectors in radians.
  static real AngleBetween(Vec4Param a, Vec4Param b);

  /// Returns if all elements of the two vectors are within epsilon of each other
  static bool ApproximatelyEqual(Vec4Param lhs, Vec4Param rhs, real epsilon);
  /// Checks to see if the values of this vector's elements are usable.
  bool Valid() const;


  /// Compute the dot product of two vectors.
  real Dot(Vec4Param rhs) const;
  /// Get the length of a vector.
  real Length() const;
  /// Get the squared length of a vector.
  real LengthSq() const;
  real Normalize();
  Vector4 Normalized() const;
  real AttemptNormalize();
  Vec4Ref Negate();

  /// Projects this vector onto the given vector (must be normalized)
  Vector4 ProjectOnVector(Vec4Param normalizedVector) const;
  /// Projects this onto a plane (the normal must be normalized)
  Vector4 ProjectOnPlane(Vec4Param planeNormal) const;
  
  /// Calculates the reflection vector across a given vector.
  Vector4 ReflectAcrossVector(Vec4Param normalizedVector) const;
  /// Calculates the reflection vector across a given plane.
  Vector4 ReflectAcrossPlane(Vec4Param planeNormal) const;

  /// Calculates the refraction vector through a plane given a certain index of refraction.
  Vector4 Refract(Vec4Param planeNormal, real refractionIndex) const;

  union
  {
    struct
    {
      real x, y, z, w;
    };
    real array[4];
  };

  static const Vector4 cZero;
  static const Vector4 cXAxis;
  static const Vector4 cYAxis;
  static const Vector4 cZAxis;
  static const Vector4 cWAxis;
  static const Vector4 Axes[4];
};

ZeroShared Vector4 operator*(real lhs, Vec4Param rhs);
/// Compute the dot product of two vectors.
ZeroShared real Dot(Vec4Param lhs, Vec4Param rhs);
/// Get the length of a vector.
ZeroShared real Length(Vec4Param value);
/// Get the squared length of a vector.
ZeroShared real LengthSq(Vec4Param value);
/// Compute the distance between two vectors.
ZeroShared real Distance(Vec4Param lhs, Vec4Param rhs);
/// Compute the squared distance between two vectors.
ZeroShared real DistanceSq(Vec4Param lhs, Vec4Param rhs);
/// Make the given vector have a length of 1, returns the original length.
ZeroShared real Normalize(Vec4Ref value);
/// Calculate and return a unit-length copy of the given vector.
ZeroShared Vector4 Normalized(Vec4Param value);
/// Try to normalize the given vector if possible. Safeguards against zero divisions.
ZeroShared real AttemptNormalize(Vec4Ref value);
/// Attempts to return a normalized given vector. Safeguards against zero divisions.
ZeroShared Vector4 AttemptNormalized(Vec4Param value);

/// Fused multiply add:  v0 + v1 * scalar
ZeroShared Vector4 MultiplyAdd(Vec4Param v0, Vec4Param v1, real scalar);
/// Fused multiply subtract:  v0 - v1 * scalar
ZeroShared Vector4 MultiplySubtract(Vec4Param v0, Vec4Param v1, real scalar);

/// Returns a copy of value with the absolute value of each element.
ZeroShared Vector4 Abs(Vec4Param value);
/// Returns a vector with the component-wise min between two vectors.
ZeroShared Vector4 Min(Vec4Param lhs, Vec4Param rhs);
/// Returns a vector with the component-wise max between two vectors.
ZeroShared Vector4 Max(Vec4Param lhs, Vec4Param rhs);
/// Returns a vector where each component is clamped between min and max.
ZeroShared Vector4 Clamp(Vec4Param value, Vec4Param minValue, Vec4Param maxValue);
/// Same as clamp, however it fills out whether or not anything was clamped.
/// Useful when an assert message should be shown if anything was clamped.
ZeroShared Vector4 DebugClamp(Vec4Param value, Vec4Param minValue, Vec4Param maxValue, bool& wasClamped);

/// Return a copy of this vector with each element has been floored.
ZeroShared Vector4 Floor(Vec4Param value);
/// Return a copy of this vector with each element has been ceiled.
ZeroShared Vector4 Ceil(Vec4Param value);
/// Return a copy of this vector with each element has been truncated.
ZeroShared Vector4 Truncate(Vec4Param value);
/// Return a copy of this vector with each element has been rounded.
ZeroShared Vector4 Round(Vec4Param value);

/// Linearly interpolate between the two vectors, the t-value is restricted to [0, 1].
ZeroShared Vector4 Lerp(Vec4Param start, Vec4Param end, real tValue);

/// Projects the input vector onto the given vector (must be normalized)
ZeroShared Vector4 ProjectOnVector(Vec4Param input, Vec4Param normalizedVector);
/// Projects the input vector onto a plane (the normal must be normalized)
ZeroShared Vector4 ProjectOnPlane(Vec4Param input, Vec4Param planeNormal);
/// Calculates the reflection vector across a given vector.
ZeroShared Vector4 ReflectAcrossVector(Vec4Param input, Vec4Param normalizedVector);
/// Calculates the reflection vector across a given plane.
ZeroShared Vector4 ReflectAcrossPlane(Vec4Param input, Vec4Param planeNormal);
/// Calculates the refraction vector through a plane given a certain index of refraction.
ZeroShared Vector4 Refract(Vec4Param input, Vec4Param planeNormal, real refractionIndex);
/// Get the angle between the two vectors in radians.
ZeroShared real AngleBetween(Vec4Param a, Vec4Param b);

//-------------------------------------------------------------------Legacy
ZeroShared void Negate(Vec4Ptr vec);
ZeroShared Vector4 Negated(Vec4Param vec);
ZeroShared Vector4 Clamped(Vec4Param vec, real min, real max);

}// namespace Math
