// MIT Licensed (see LICENSE.md).
#pragma once

#include "Reals.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "Matrix3.hpp"
#include "Matrix4.hpp"

namespace Math
{

/// Forward declaration
struct Matrix3;
typedef const Matrix3& Mat3Param;
typedef Matrix3& Mat3Ref;

/// Forward declaration
struct Matrix4;
typedef const Matrix4& Mat4Param;
typedef Matrix4& Mat4Ref;

struct Quaternion;
typedef const Quaternion& QuatParam;
typedef Quaternion& QuatRef;
typedef Quaternion* QuatPtr;
typedef Quaternion Quat;

struct ZeroShared Quaternion
{
  static const Quaternion cIdentity;

  Quaternion(){};
  explicit Quaternion(real x, real y, real z, real w);

  real& operator[](uint index);
  real operator[](uint index) const;

  // Unary Operators
  Quaternion operator-() const;

  // Binary operators (reals)
  void operator*=(real rhs);
  void operator/=(real rhs);
  Quaternion operator*(real rhs) const;
  Quaternion operator/(real rhs) const;

  // Binary operators (quaternions)
  void operator+=(QuatParam rhs);
  void operator-=(QuatParam rhs);
  Quaternion operator+(QuatParam rhs) const;
  Quaternion operator-(QuatParam rhs) const;
  // Quaternion multiplication
  void operator*=(QuatParam rhs);
  Quaternion operator*(QuatParam rhs) const;

  // Comparison operators
  bool operator==(QuatParam rhs) const;
  bool operator!=(QuatParam rhs) const;

  /// Set the quaternion to the identity.
  void SetIdentity();
  /// Set each value of the quaternion.
  void Set(real x, real y, real z, real w);
  /// Checks to see if the values of this quaternion are usable.
  bool Valid() const;

  /// Get the vector 3 portion of the quaternion.
  Vector3& V3();
  /// Get the quaternion as a vector 4.
  Vector4& V4();
  /// Get the vector 3 portion of the quaternion.
  const Vector3& V3() const;
  /// Get the quaternion as a vector 4.
  const Vector4& V4() const;

  /// Compute the dot product of two quaternions.
  static real Dot(QuatParam lhs, QuatParam rhs);
  /// Get the length of a quaternion.
  static real Length(QuatParam value);
  /// Get the squared length of a quaternion.
  static real LengthSq(QuatParam value);
  /// Make the given quaternion have a length of 1, returns the original length.
  static real Normalize(QuatRef value);
  /// Calculate and return a unit-length copy of the given quaternion.
  static Quaternion Normalized(QuatParam value);

  /// Set the quaternion to its conjugate.
  static void Conjugate(QuatRef value);
  /// Return the quaternion conjugate.
  static Quaternion Conjugated(QuatParam value);
  /// Invert the quaternion in place.
  static void Invert(QuatRef value);
  /// Return the inverted quaternion.
  static Quaternion Inverted(QuatParam value);

  /// Multiply the two quaternions together. Quaternion multiplication order is
  /// right-to-left.
  static Quaternion Multiply(QuatParam lhs, QuatParam rhs);
  /// Multiply the given vector by a quaternion.
  static Vector3 Multiply(QuatParam lhs, Vec3Param rhs);

  /// Linearly interpolate between two quaternions (n-lerp). This should rarely
  /// be used over Slerp.
  static Quaternion Lerp(QuatParam start, QuatParam end, real tValue);
  /// Spherical linear interpolation between two quaternions. Used to
  /// interpolate between two rotations. Will normalize the inputs.
  static Quaternion Slerp(QuatParam start, QuatParam end, real tValue);
  /// Spherical linear interpolation between two quaternions. Used to
  /// interpolate between two rotations. Assumes the inputs are normalized.
  static Quaternion SlerpUnnormalized(QuatParam start, QuatParam end, real tValue);
  /// Compute the exponential of a quaternion.
  static Quaternion Exponent(QuatParam value);
  /// Compute the logarithm of a quaternion.
  static Quaternion Logarithm(QuatParam value);
  /// Get the angle between the two quaternions in radians.
  static real AngleBetween(QuatParam a, QuatParam b);
  /// Small angle approximation to rotate a quaternion with an angular
  /// velocity. Equivalent to integrating a rotation.
  static Quaternion Integrate(QuatParam rotation, Vec3Param angularVelocity, real dt);

  real Dot(QuatParam rhs) const;
  real Length() const;
  real LengthSq() const;
  real Normalize();
  Quaternion Normalized() const;
  void Conjugate();
  Quaternion Conjugated() const;
  void Invert();
  Quaternion Inverted() const;

  real x, y, z, w;
};

ZeroShared Quaternion operator*(real lhs, QuatParam rhs);

ZeroShared real Dot(QuatParam lhs, QuatParam rhs);
ZeroShared real Length(QuatParam quaternion);
ZeroShared real LengthSq(QuatParam quaternion);
ZeroShared void Normalize(QuatRef quaternion);
ZeroShared Quaternion Normalized(QuatParam quaternion);
ZeroShared Quaternion Multiply(QuatParam lhs, QuatParam rhs);
ZeroShared Vector3 Multiply(QuatParam lhs, Vec3Param rhs);
ZeroShared Quaternion Lerp(QuatParam start, QuatParam end, real tValue);
ZeroShared Quaternion Slerp(QuatParam start, QuatParam end, real tValue);
ZeroShared Quaternion SlerpUnnormalized(QuatParam start, QuatParam end, real tValue);
ZeroShared real AngleBetween(QuatParam a, QuatParam b);

ZeroShared Quaternion CreateDiagonalizer(Mat3Param matrix);

} // namespace Math
