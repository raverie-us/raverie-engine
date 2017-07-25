///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Math/Reals.hpp"
#include "Math/Vector2.hpp"

namespace Math
{

struct Matrix2;
typedef Matrix2 Mat2;
typedef const Matrix2& Mat2Param;
typedef Matrix2& Mat2Ref;
typedef Matrix2* Mat2Ptr;

struct ZeroShared Matrix2
{
public:
  Matrix2() {};
  Matrix2(real p00, real p01,
          real p10, real p11);
  Matrix2(ConstRealPointer data_);

  Vec2Ref operator[](uint index);
  Vec2Param operator[](uint index) const;
  real& operator()(uint r, uint c);
  real operator()(uint r, uint c) const;

  // Binary operators (reals)
  void operator*=(real rhs);
  void operator/=(real rhs);
  Matrix2 operator*(real rhs) const;
  Matrix2 operator/(real rhs) const;

  // Binary operators (matrices)
  void operator+=(Mat2Param rhs);
  void operator-=(Mat2Param rhs);
  Matrix2 operator+(Mat2Param rhs) const;
  Matrix2 operator-(Mat2Param rhs) const;
  // Matrix multiplication
  Matrix2 operator*(Mat2Param rhs) const;

  // Comparison operators
  bool operator==(Mat2Param rhs) const;
  bool operator!=(Mat2Param rhs) const;

  /// Set all elements to 0.
  Mat2Ref ZeroOut();
  /// Set the matrix to the identity.
  Mat2Ref SetIdentity();
  /// Returns a basis vector of the matrix.
  /// The basis is defined as the basis vector of a pure rotation matrix.
  Vector2 GetBasis(uint index) const;
  /// Set a basis vector.
  void SetBasis(uint index, Vec2Param basis);
  /// Get a cross vector which is defined as the elements perpendicular to the basis vector.
  Vector2 GetCross(uint index) const;
  void SetCross(uint index, Vec2Param cross);
  /// Check if all values are valid.
  bool Valid() const;

  /// Compute the determinant of the matrix.
  real Determinant() const;
  /// Internal helper function to invert a matrix given its inverse determinant.
  void InvertInternal(real invDeterminant);

  /// Transpose the matrix in place.
  static void Transpose(Mat2Ref mat);
  /// Return the transpose of the given matrix.
  static Matrix2 Transposed(Mat2Param mat);
  /// Compute the determinant of the matrix.
  static real Determinant(Mat2Param mat);
  /// Invert the matrix in place
  static void Invert(Mat2Ref mat);
  /// Return the inverted matrix.
  static Matrix2 Inverted(Mat2Param mat);
  /// Inverts the matrix if the determinant is non-zero.
  /// Returns if the matrix was invertible.
  static bool SafeInvert(Mat2Ref mat);

  /// Multiply the two matrices together. Matrix multiplication order is right-to-left.
  static Matrix2 Multiply(Mat2Param lhs, Mat2Param rhs);
  /// Multiply the given vector by a matrix.
  static Vector2 Multiply(Mat2Param lhs, Vec2Param rhs);
  
  /// Generates a two dimensional scale matrix.
  static Matrix2 GenerateScale(Vec2Param scale);
  /// Generates a two dimensional rotation matrix.
  static Matrix2 GenerateRotation(real radians);
  /// Generates a transformation matrix with the given rotation and scale.
  static Matrix2 GenerateTransform(real radians, Vec2Param scale);
  /// Decompose the given matrix into rotation and scale
  static void Decompose(Mat2Param transform, real& radians, Vec2Ref scale);

  //-------------------------------------------------------------------Legacy
  Matrix2 Transposed() const;

  Matrix2 Inverted();

  void Rotate(real radians);
  void Scale(real x, real y);
  void Scale(Vec2Param rhs);
  Vector2 Transform(Vec2Param vector) const;

  union 
  {
    struct
    {
      real m00, m01,
           m10, m11;
    };

    real array[4];
  };

  static const Matrix2 cIdentity;
};

ZeroShared Matrix2 operator*(real lhs, Mat2Param rhs);
ZeroShared Matrix2 Multiply(Mat2Param lhs, Mat2Param rhs);
ZeroShared Vector2 Multiply(Mat2Param lhs, Vec2Param rhs);

}// namespace Math
