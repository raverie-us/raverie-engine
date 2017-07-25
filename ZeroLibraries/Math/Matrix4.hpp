///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Math/MatrixStorage.hpp"
#include "Math/Reals.hpp"
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"

namespace Math
{
///Forward declaration
struct Quaternion;
typedef const Quaternion& QuatParam;

///Forward declaration
struct Matrix3;
typedef const Matrix3& Mat3Param;
typedef Matrix3* Mat3Ptr;

struct Matrix4;
typedef Matrix4 Mat4;
typedef const Matrix4& Mat4Param;
typedef Matrix4& Mat4Ref;
typedef Matrix4* Mat4Ptr;

///4 dimensional square matrix. Supports operations with other 4 dimensional
///square matrices, 3 dimensional vectors, and 4 dimensional vectors.
struct ZeroShared Matrix4
{
#if ColumnBasis == 1
typedef Vector4   BasisVector;
typedef Vec4Param CrossVector;
#else
typedef Vec4Param BasisVector;
typedef Vector4   CrossVector;
#endif

public:
  Matrix4() {};
  Matrix4(real p00, real p01, real p02, real p03,
          real p10, real p11, real p12, real p13,
          real p20, real p21, real p22, real p23,
          real p30, real p31, real p32, real p33);
  Matrix4(Vec4Param basisX, Vec4Param basisY,
          Vec4Param basisZ, Vec4Param basisW);
  Matrix4(ConstRealPointer data_);

  const Vector4& operator[](uint index) const;
  Vector4& operator[](uint index);
  real operator()(uint r, uint c) const;
  real& operator()(uint r, uint c);

  // Binary operators (reals)
  void operator*=(real rhs);
  void operator/=(real rhs);
  Matrix4 operator*(real rhs) const;
  Matrix4 operator/(real rhs) const;

  // Binary operators (matrices)
  void operator+=(Mat4Param rhs);
  void operator-=(Mat4Param rhs);
  Matrix4 operator+(Mat4Param rhs) const;
  Matrix4 operator-(Mat4Param rhs) const;
  // Matrix multiplication
  Matrix4 operator*(Mat4Param rhs) const;

  // Comparison operators
  bool operator==(Mat4Param rhs) const;
  bool operator!=(Mat4Param rhs) const;

  /// Set all elements to 0.
  Mat4Ref ZeroOut();
  /// Set the matrix to the identity.
  Mat4Ref SetIdentity();
  /// Returns a basis vector of the matrix.
  /// The basis is defined as the basis vector of a pure rotation matrix.
  BasisVector GetBasis(uint index) const;
  /// Set a basis vector.
  void SetBasis(uint index, Vec4Param basis);
  /// Accesses the 3-D basis vector at the given index, ignores the last element.
  Vector3 GetBasis3(uint index) const;
  /// Get a cross vector which is defined as the elements perpendicular to the basis vector.
  CrossVector GetCross(uint index) const;
  void SetCross(uint index, Vec4Param cross);
  /// Check if all values are valid.
  bool Valid() const;

  /// Compute the determinant of the matrix.
  real Determinant() const;
  /// Internal helper function to invert a matrix given its inverse determinant.
  void InvertInternal(real invDeterminant);

  /// Transpose the matrix in place.
  static void Transpose(Mat4Ref mat);
  /// Return the transpose of the given matrix.
  static Matrix4 Transposed(Mat4Param mat);
  /// Compute the determinant of the matrix.
  static real Determinant(Mat4Param mat);
  /// Invert the matrix in place
  static void Invert(Mat4Ref mat);
  /// Return the inverted matrix.
  static Matrix4 Inverted(Mat4Param mat);
  /// Inverts the matrix if the determinant is non-zero.
  /// Returns if the matrix was invertible.
  static bool SafeInvert(Mat4Ref mat);

  /// Multiply the two matrices together. Matrix multiplication order is right-to-left.
  static Matrix4 Multiply(Mat4Param lhs, Mat4Param rhs);
  /// Multiply the given vector by a matrix.
  static Vector4 Multiply(Mat4Param lhs, Vec4Param rhs);
  /// Multiply the given vector by a matrix. The vector is promoted to the point Vec4(x, y, z, 1). No homogeneous division is applied.
  static Vector3 MultiplyPoint(Mat4Param lhs, Vec3Param rhs);
  /// Multiply the given vector by a matrix. The vector is promoted to the vector Vec4(x, y, z, 0).
  static Vector3 MultiplyNormal(Mat4Param lhs, Vec3Param rhs);


  /// Generates a three dimensional scale matrix.
  static Matrix4 GenerateScale(Vec3Param scale);
  /// Generates a three dimensional rotation matrix.
  static Matrix4 GenerateRotation(Vec3Param axis, real rotationRadians);
  /// Generates a three dimensional translation matrix.
  static Matrix4 GenerateTranslation(Vec3Param translation);
  /// Generates a transformation matrix.
  static Matrix4 GenerateTransform(Vec3Param translation, QuatParam rotatation, Vec3Param scale);
  /// Generates a transformation matrix.
  static Matrix4 GenerateTransform(Vec3Param translation, Mat3Param rotatation, Vec3Param scale);
  /// Decompose the given matrix into translation, rotation, and scale
  static void Decompose(Mat4Param transform, Vec3Ref translation, Mat3Ref rotation, Vec3Ref scale);
  /// Attempts to decompose this matrix into a scale, rotation, and translational
  /// component while removing shear. The "original" matrix is not necessarily
  /// recovered, though the resulting matrix is orthonormal among its components.
  static void Decompose(Mat4Param transform, Vec3Ref translation, Mat3Ref rotation, Vec3Ref shear, Vec3Ref scale);

  //-------------------------------------------------------------------Legacy

  /// Transposes this matrix in place.
  Mat4Ref Transpose();
  /// Returns a copy of this matrix with its elements transposed.
  Matrix4 Transposed() const;
  /// Inverts this matrix in place.
  Mat4Ref Invert();
  /// Returns the inverse of this matrix.
  Matrix4 Inverted() const;
  /// Inverts in place, but clamps the determinant to the smallest positive float number.
  bool SafeInvert();
  /// Inverts, but clamps the determinant to the smallest positive float number.
  Matrix4 SafeInverted() const;

  /// Converts this matrix into a pure scaling matrix.
  void Scale(real x, real y, real z);
  /// Converts this matrix into a pure scaling matrix.
  void Scale(Vec3Param axis);
  /// Converts this matrix into a pure rotation matrix, given an axis-angle pair.
  void Rotate(real x, real y, real z, real radians);
  /// Converts this matrix into a pure rotation matrix, given an axis-angle pair.
  void Rotate(Vec3Param axis, real radians);
  /// Converts this matrix into a pure translation matrix for 3-D vectors.
  void Translate(real x, real y, real z);
  /// Converts this matrix into a pure translation matrix for 3-D vectors.
  void Translate(Vec3Param axis);
  /// Converts this matrix into a transformation matrix, incorporating 
  /// translation, rotation, and scale. Meant for 3-D vectors.
  void BuildTransform(Vec3Param translate, QuatParam rotate, Vec3Param scale);
  /// Converts this matrix into a transformation matrix, incorporating 
  /// translation, rotation, and scale. Meant for 3-D vectors.
  void BuildTransform(Vec3Param translate, Mat3Param rotate, Vec3Param scale);
  /// Decomposes this matrix into its scale, rotation, and translational
  /// components.
  void Decompose(Vec3Ptr scale, Mat3Ptr rotate, Vec3Ptr translate) const;
  /// Attempts to decompose this matrix into a scale, rotation, and translational
  /// component while removing shear. The "original" matrix is not necessarily
  /// recovered, though the resulting matrix is orthonormal among its components.
  void Decompose(Vec3Ptr scale, Vec3Ptr shear, Mat3Ptr rotate, 
                 Vec3Ptr translate) const;

  /// Accesses the elements in the "x-axis" of the matrix, with the "x-axis" 
  /// defined as the x-axis of a pure rotation matrix.
  BasisVector BasisX() const;
  /// Accesses the elements in the "y-axis" of the matrix, with the "y-axis" 
  /// defined as the y-axis of a pure rotation matrix.
  BasisVector BasisY() const;
  /// Accesses the elements in the "z-axis" of the matrix, with the "z-axis" 
  /// defined as the z-axis of a pure rotation matrix.
  BasisVector BasisZ() const;
  /// Accesses the elements in the "w-axis" of the matrix, with the "w-axis" 
  /// defined as the w-axis of a pure rotation matrix.
  BasisVector BasisW() const;
  void SetBasis(uint index, Vec3Param basisVector3, real w);
  void SetBasis(uint index, real x, Vec3Param basisVector3);
  void SetBasis(uint index, real x, real y, real z, real w);

  /// Accesses the cross vector at the given index, with the cross vector defined
  /// as the elements in the matrix perpendicular to that of the corresponding
  /// basis vector.
  void SetCross(uint index, Vec3Param crossVector3, real w);
  void SetCross(uint index, real x, real y, real z, real w);

  union 
  {
    struct
    {
#if ColumnBasis == 1
      real m00, m01, m02, m03,
           m10, m11, m12, m13,
           m20, m21, m22, m23,
           m30, m31, m32, m33;
#else
      real m00, m10, m20, m30,
           m01, m11, m21, m31,
           m02, m12, m22, m32,
           m03, m13, m23, m33;
#endif
    };

    real array[16];
  };

  static const Matrix4 cIdentity;
};

ZeroShared Matrix4 operator*(real lhs, Mat4Param rhs);

/// Multiply the two matrices together. Matrix multiplication order is right-to-left.
ZeroShared Matrix4 Multiply(Mat4Param lhs, Mat4Param rhs);
/// Multiply the given vector by a matrix.
ZeroShared Vector4 Multiply(Mat4Param lhs, Vec4Param rhs);
/// Multiply the given vector by a matrix. The vector is promoted to the point Vec4(x, y, z, 1). No homogeneous division is applied.
ZeroShared Vector3 MultiplyPoint(Mat4Param lhs, Vec3Param rhs);
/// Multiply the given vector by a matrix. The vector is promoted to the vector Vec4(x, y, z, 0).
ZeroShared Vector3 MultiplyNormal(Mat4Param lhs, Vec3Param rhs);

//-------------------------------------------------------------------Legacy
ZeroShared Vector4 Transform(Mat4Param mat, Vec4Param vector);
ZeroShared void Transform(Mat4Param mat, Vec4Ptr vector);
///Applies transformation with the translation (p.x, p.y, p.z, 1)
ZeroShared Vector3 TransformPoint(Mat4Param matrix, Vec3Param point);
///Applies transformation without the translation (n.x, n.y, n.z, 0)
ZeroShared Vector3 TransformNormal(Mat4Param matrix, Vec3Param normal);
///Applies transform and projects back to (w = 1)
ZeroShared Vector3 TransformPointProjected(Mat4Param matrix, Vec3Param point);
ZeroShared Vector3 TransformNormalCol(Mat4Param matrix, Vec3Param normal);
ZeroShared Vector3 TransformPointCol(Mat4Param matrix, Vec3Param point);
ZeroShared Vector3 TransformPointProjectedCol(Mat4Param matrix, Vec3Param point);
ZeroShared Vector3 TransformPointProjectedCol(Mat4Param matrix, Vec3Param point, real* w);

ZeroShared Matrix4 BuildTransform(Vec3Param translate, QuatParam rotate, Vec3Param scale);
ZeroShared Matrix4 BuildTransform(Vec3Param translate, Mat3Param rotate, Vec3Param scale);

ZeroShared real Trace(Mat4Param matrix);

}// namespace Math
