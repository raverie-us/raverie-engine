///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Math/MatrixStorage.hpp"
#include "Math/Reals.hpp"
#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"

namespace Math
{

///Forward declaration
struct Quaternion;
typedef const Quaternion& QuatParam;

///Forward declaration
struct Matrix4;
typedef const Matrix4& Mat4Param;

struct Matrix3;
typedef Matrix3 Mat3;
typedef const Matrix3& Mat3Param;
typedef Matrix3& Mat3Ref;
typedef Matrix3* Mat3Ptr;

//--------------------------------------------------------------------- Matrix 3
///3 dimensional square matrix. Supports operations with other 3 dimensional
///square matrices, 2 dimensional vectors, and 3 dimensional vectors.
struct ZeroShared Matrix3
{
#if ColumnBasis == 1
typedef Vector3   BasisVector;
typedef Vec3Param CrossVector;
#else
typedef Vec3Param BasisVector;
typedef Vector3   CrossVector;
#endif

public:
  Matrix3(void) {};
  explicit Matrix3(real p00, real p01, real p02, 
                   real p10, real p11, real p12,
                   real p20, real p21, real p22);
  explicit Matrix3(ConstRealPointer data_);

  Vector3& operator[](uint index);
  const Vector3& operator[](uint index) const;
  real& operator()(uint r, uint c);
  real operator()(uint r, uint c) const;

  // Binary operators (reals)
  void operator*=(real rhs);
  void operator/=(real rhs);
  Matrix3 operator*(real rhs) const;
  Matrix3 operator/(real rhs) const;

  // Binary operators (matrices)
  void operator+=(Mat3Param rhs);
  void operator-=(Mat3Param rhs);
  Matrix3 operator+(Mat3Param rhs) const;
  Matrix3 operator-(Mat3Param rhs) const;
  // Matrix multiplication
  Matrix3 operator*(Mat3Param rhs) const;

  // Comparison operators
  bool operator==(Mat3Param rhs) const;
  bool operator!=(Mat3Param rhs) const;

  /// Set all elements to 0.
  Mat3Ref ZeroOut();
  /// Set the matrix to the identity.
  Mat3Ref SetIdentity();
  /// Returns a basis vector of the matrix.
  /// The basis is defined as the basis vector of a pure rotation matrix.
  BasisVector GetBasis(uint index) const;
  /// Set a basis vector.
  void SetBasis(uint index, Vec3Param basis);
  /// Get a cross vector which is defined as the elements perpendicular to the basis vector.
  CrossVector GetCross(uint index) const;
  void SetCross(uint index, Vec3Param cross);
  /// Check if all values are valid.
  bool Valid() const;

  /// Compute the determinant of this matrix.
  real Determinant() const;
  /// Internal helper function to invert a matrix given its inverse determinant.
  void InvertInternal(real invDeterminant);

  /// Transpose the matrix in place.
  static void Transpose(Mat3Ref mat);
  /// Return the transpose of the given matrix.
  static Matrix3 Transposed(Mat3Param mat);
  /// Compute the determinant of the matrix.
  static real Determinant(Mat3Param mat);
  /// Invert the matrix in place
  static void Invert(Mat3Ref mat);
  /// Return the inverted matrix.
  static Matrix3 Inverted(Mat3Param mat);
  /// Inverts the matrix if the determinant is non-zero.
  /// Returns if the matrix was invertible.
  static bool SafeInvert(Mat3Ref mat);

  /// Multiply the two matrices together. Matrix multiplication order is right-to-left.
  static Matrix3 Multiply(Mat3Param lhs, Mat3Param rhs);
  /// Multiply the given vector by a matrix.
  static Vector3 Multiply(Mat3Param lhs, Vec3Param rhs);
  /// Multiply the given vector by a matrix. The vector is promoted to the point Vec3(x, y, 1). No homogeneous division is applied.
  static Vector2 MultiplyPoint(Mat3Param lhs, Vec2Param rhs);
  /// Multiply the given vector by a matrix. The vector is promoted to the vector Vec3(x, y, 0).
  static Vector2 MultiplyNormal(Mat3Param lhs, Vec2Param rhs);

  /// Generates a two dimensional scale matrix.
  static Matrix3 GenerateScale(Vec2Param scale);
  /// Generates a three dimensional scale matrix.
  static Matrix3 GenerateScale(Vec3Param scale);
  /// Generates a two dimensional rotation matrix.
  static Matrix3 GenerateRotation(real radians);
  /// Generates a three dimensional rotation matrix from an axis-angle.
  static Matrix3 GenerateRotation(Vec3Param axis, real radians);
  /// Generates a two dimensional translation matrix.
  static Matrix3 GenerateTranslation(Vec2Param translation);
  /// Generates a two-dimensions transform.
  static Matrix3 GenerateTransform(Vec2Param translation, real rotationRadians, Vec2Param scale);
  /// Generates a three-dimensions transform.
  static Matrix3 GenerateTransform(QuatParam rotation, Vec3Param scale);
  /// Generates a three-dimensions transform.
  static Matrix3 GenerateTransform(Mat3Param rotation, Vec3Param scale);
  /// Decompose the given two-dimensional transform.
  static void Decompose(Mat3Param transform, Vec2Ref translation, real& rotationRadians, Vec2Ref scale);
  /// Decompose the given three-dimensional transform.
  static void Decompose(Mat3Param transform, Mat3Ref rotationRadians, Vec3Ref scale);

  //-------------------------------------------------------------------Legacy
  /// Returns a copy of this matrix with its elements transposed.
  Matrix3 Transposed() const;

  /// Transposes this matrix in place.
  Mat3Ref Transpose();

  /// Returns the inverse of this matrix.
  Matrix3 Inverted() const;

  /// Inverts this matrix in place.
  Mat3Ref Invert();

  /// Inverts in place, but clamps the determinant to the smallest positive float number.
  bool SafeInvert();

  /// Inverts, but clamps the determinant to the smallest positive float number.
  Matrix3 SafeInverted() const;

  /// Converts this matrix into a pure scaling matrix.
  void Scale(real x, real y, real z);

  /// Converts this matrix into a pure scaling matrix.
  void Scale(Vec3Param rhs);

  /// Converts this matrix into a pure rotation matrix, given an axis-angle pair.
  void Rotate(real x, real y, real z, real radian);

  /// Converts this matrix into a pure rotation matrix, given an axis-angle pair.
  void Rotate(Vec3Param rhs, real radian);

  /// Converts this matrix into a pure translation matrix for 2-D vectors.
  void Translate(real x, real y);

  /// Converts this matrix into a pure translation matrix for 2-D vectors.
  void Translate(Vec2Param rhs);

  /// Builds a matrix that should be used on 2D points/vectors
  void BuildTransform(Vec2Param translate, real radians, Vec2Param scale);

  /// Builds a matrix that should be used on 3D points/vectors
  void BuildTransform(QuatParam rotate, Vec3Param scale);
  void BuildTransform(Mat3Param rotate, Vec3Param scale);

  /// Forces the current matrix to be orthonormal.
  Mat3Ref Orthonormalize();

  /// Accesses the elements in the "x-axis" of the matrix, with the "x-axis" 
  /// defined as the x-axis of a pure rotation matrix.
  BasisVector BasisX() const;

  /// Accesses the elements in the "y-axis" of the matrix, with the "y-axis" 
  /// defined as the y-axis of a pure rotation matrix.
  BasisVector BasisY() const;

  /// Accesses the elements in the "z-axis" of the matrix, with the "z-axis" 
  /// defined as the z-axis of a pure rotation matrix.
  BasisVector BasisZ() const;

  void SetBasis(uint index, real x, real y, real z);
  void SetCross(uint index, real x, real y, real z);

  union 
  {
    struct
    {
#if ColumnBasis == 1
      real m00, m01, m02,
           m10, m11, m12,
           m20, m21, m22;
#else
      real m00, m10, m20,
           m01, m11, m21,
           m02, m12, m22;
#endif 
    };
    real array[9];
  };

  static const Matrix3 cIdentity;
};

ZeroShared Matrix3 operator*(real lhs, Mat3Param rhs);

/// Multiply the two matrices together. Matrix multiplication order is right-to-left.
ZeroShared Matrix3 Multiply(Mat3Param lhs, Mat3Param rhs);
/// Multiply the given vector by a matrix.
ZeroShared Vector3 Multiply(Mat3Param lhs, Vec3Param rhs);
/// Multiply the given vector by a matrix. The vector is promoted to the point Vec3(x, y, 1). No homogeneous division is applied.
ZeroShared Vector2 MultiplyPoint(Mat3Param lhs, Vec2Param rhs);
/// Multiply the given vector by a matrix. The vector is promoted to the vector Vec3(x, y, 0).
ZeroShared Vector2 MultiplyNormal(Mat3Param lhs, Vec2Param rhs);

//-------------------------------------------------------------------Legacy
///This builds a matrix that should be used on 2D points/vectors
ZeroShared Matrix3 BuildTransform(Vec2Param translate, real radians, Vec2Param scale);

///This builds a matrix that should be used on 3D points/vectors
ZeroShared Matrix3 BuildTransform(QuatParam rotate, Vec3Param scale);

ZeroShared Vector3 Transform(Mat3Param mat, Vec3Param vector);
ZeroShared void Transform(Mat3Param matrix, Vec3Ptr vector);

/// Applies transformation with the translation (p.x, p.y, 1)
ZeroShared Vector2 TransformPoint(Mat3Param matrix, Vec2Param vector);

/// Applies transformation without the translation (n.x, n.y, 0)
ZeroShared Vector2 TransformNormal(Mat3Param matrix, Vec2Param normal);

///Transforms the given vector by the matrix as if the matrix was transposed.
ZeroShared Vector3 TransposedTransform(Mat3Param mat, Vec3Param vector);

///Transforms the given vector by the matrix as if the matrix was transposed.
ZeroShared void TransposedTransform(Mat3Param matrix, Vec3Ptr vector);

ZeroShared real Trace(Mat3Param matrix);

ZeroShared real Cofactor(Mat3Param matrix, uint row, uint column);

///Takes a symmetric matrix and diagonalizes it.
ZeroShared void Diagonalize(Mat3Ptr matrix);
ZeroShared Matrix3 Diagonalized(Mat3Param matrix);

ZeroShared void Invert(Mat3Ptr matrix);
ZeroShared Matrix3 Inverted(Mat3Param matrix);

}// namespace Math
