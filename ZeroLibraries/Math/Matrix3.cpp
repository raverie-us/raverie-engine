///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{

const Matrix3 Matrix3::cIdentity(real(1.0), real(0.0), real(0.0),
                                 real(0.0), real(1.0), real(0.0),
                                 real(0.0), real(0.0), real(1.0));

Matrix3::Matrix3(real p00, real p01, real p02, 
                 real p10, real p11, real p12, 
                 real p20, real p21, real p22)
{
  m00 = p00;  m01 = p01;  m02 = p02;
  m10 = p10;  m11 = p11;  m12 = p12; 
  m20 = p20;  m21 = p21;  m22 = p22;
}

Matrix3::Matrix3(ConstRealPointer data_)
{
  m00 = data_[0]; m01 = data_[1]; m02 = data_[2];
  m10 = data_[3]; m11 = data_[4]; m12 = data_[5];
  m20 = data_[6]; m21 = data_[7]; m22 = data_[8];
}

Vector3& Matrix3::operator[](uint index)
{
  return ((Vector3*)this)[index];
}

const Vector3& Matrix3::operator[](uint index) const
{
  return ((Vector3*)this)[index];
}

real& Matrix3::operator()(uint r, uint c)
{
  ErrorIf(r > 2, "Matrix3 - Index out of range.");
  ErrorIf(c > 2, "Matrix3 - Index out of range.");

#if ColumnBasis == 1
  return array[c + r * 3];
#else
  return array[r + c * 3];
#endif
}

real Matrix3::operator()(uint r, uint c) const
{
  ErrorIf(r > 2, "Matrix3 - Index out of range.");
  ErrorIf(c > 2, "Matrix3 - Index out of range.");

#if ColumnBasis == 1
  return array[c + r * 3];
#else
  return array[r + c * 3];
#endif
}

void Matrix3::operator*=(real rhs)
{
  Matrix3& self = *this;
  self[0] *= rhs;
  self[1] *= rhs;
  self[2] *= rhs;
}

void Matrix3::operator/=(real rhs)
{
  Matrix3& self = *this;
  ErrorIf(Math::IsZero(rhs), "Matrix3 - Division by zero.");
  self[0] /= rhs;
  self[1] /= rhs;
  self[2] /= rhs;
}

Matrix3 Matrix3::operator*(real rhs) const
{
  Matrix3 ret = *this;
  ret *= rhs;
  return ret;
}

Matrix3 Matrix3::operator/(real rhs) const
{
  ErrorIf(Math::IsZero(rhs), "Matrix3 - Division by zero.");
  Matrix3 ret = *this;
  ret /= rhs;
  return ret;
}

void Matrix3::operator+=(Mat3Param rhs)
{
  Matrix3& self = *this;
  self[0] += rhs[0];
  self[1] += rhs[1];
  self[2] += rhs[2];
}

void Matrix3::operator-=(Mat3Param rhs)
{
  Matrix3& self = *this;
  self[0] -= rhs[0];
  self[1] -= rhs[1];
  self[2] -= rhs[2];
}

Matrix3 Matrix3::operator+(Mat3Param rhs) const
{
  Matrix3 ret = *this;
  ret += rhs;
  return ret;
}

Matrix3 Matrix3::operator-(Mat3Param rhs) const
{
  Matrix3 ret = *this;
  ret -= rhs;
  return ret;
}

Matrix3 Matrix3::operator*(Mat3Param rhs) const
{
  return Matrix3::Multiply(*this, rhs);
}

bool Matrix3::operator==(Mat3Param rhs) const
{
  const Matrix3& self = *this;
  return self[0] == rhs[0] &&
         self[1] == rhs[1] &&
         self[2] == rhs[2];
}

bool Matrix3::operator!=(Mat3Param rhs) const
{
  return !(*this == rhs);
}

Mat3Ref Matrix3::ZeroOut(void)
{
  Matrix3& self = *this;
  self[0].ZeroOut();
  self[1].ZeroOut();
  self[2].ZeroOut();
  return *this;
}

Mat3Ref Matrix3::SetIdentity(void)
{
  Matrix3& self = *this;
  self[0].Set(real(1.0), real(0.0), real(0.0));
  self[1].Set(real(0.0), real(1.0), real(0.0));
  self[2].Set(real(0.0), real(0.0), real(1.0));
  return *this;
}

Matrix3::BasisVector Matrix3::GetBasis(uint index) const
{
  ErrorIf(index > 2, "Matrix3 - Index out of range.");
#if ColumnBasis == 1
  return Vector3(array[index], array[index + 3], array[index + 6]);
#else
  const Matrix3& self = *this;
  return self[index];
#endif
}

void Matrix3::SetBasis(uint index, Vec3Param basis)
{
  ErrorIf(index > 2, "Matrix3 - Index out of range.");
#if ColumnBasis == 1
  array[index + 0] = basis.x;
  array[index + 3] = basis.y;
  array[index + 6] = basis.z;
#else
  Matrix3& self = *this;
  self[index].Set(basis.x, basis.y, basis.z);
#endif
}

Matrix3::CrossVector Matrix3::GetCross(uint index) const
{
  ErrorIf(index > 2, "Matrix3 - Index out of range.");
#if ColumnBasis == 1
  Mat3Param self = *this;
  return self[index];
#else
  return Vector3(array[index], array[index + 3], array[index + 6]);
#endif
}

void Matrix3::SetCross(uint index, Vec3Param cross)
{
  ErrorIf(index > 2, "Matrix3 - Index out of range.");
#if ColumnBasis == 1
  Matrix3& self = *this;
  self[index].Set(cross.x, cross.y, cross.z);
#else
  array[index + 0] = cross.x;
  array[index + 3] = cross.y;
  array[index + 6] = cross.z;
#endif
}

bool Matrix3::Valid() const
{
  const Matrix3& self = *this;
  return self[0].Valid() && self[1].Valid() && self[2].Valid();
}

real Matrix3::Determinant() const
{
  return (m00 * m11 * m22 + m10 * m21 * m02 + m01 * m12 * m20) 
       - (m02 * m11 * m20 + m10 * m01 * m22 + m00 * m21 * m12);
}

void Matrix3::InvertInternal(real invDeterminant)
{
  real t00 = (m11 * m22 - m12 * m21) * invDeterminant;
  real t01 = (m02 * m21 - m01 * m22) * invDeterminant;
  real t02 = (m01 * m12 - m02 * m11) * invDeterminant;
  real t10 = (m12 * m20 - m10 * m22) * invDeterminant;
  real t11 = (m00 * m22 - m02 * m20) * invDeterminant;
  real t12 = (m02 * m10 - m00 * m12) * invDeterminant;
  real t20 = (m10 * m21 - m11 * m20) * invDeterminant;
  real t21 = (m01 * m20 - m00 * m21) * invDeterminant;
  real t22 = (m00 * m11 - m01 * m10) * invDeterminant;

  m00 = t00;  m01 = t01;  m02 = t02;
  m10 = t10;  m11 = t11;  m12 = t12;
  m20 = t20;  m21 = t21;  m22 = t22;
}

void Matrix3::Transpose(Mat3Ref mat)
{
  Math::Swap(mat.m01, mat.m10);
  Math::Swap(mat.m02, mat.m20);
  Math::Swap(mat.m12, mat.m21);
}

Matrix3 Matrix3::Transposed(Mat3Param mat)
{
  Matrix3 result = mat;
  Matrix3::Transpose(result);
  return result;
}

real Matrix3::Determinant(Mat3Param mat)
{
  return mat.Determinant();
}

void Matrix3::Invert(Mat3Ref mat)
{
  real determinant = mat.Determinant();
  ErrorIf(determinant == real(0.0), "Matrix3 - Uninvertible matrix.");

  real invDeterminant = real(1.0) / determinant;
  mat.InvertInternal(invDeterminant);
}

Matrix3 Matrix3::Inverted(Mat3Param mat)
{
  Matrix3 result = mat;
  Matrix3::Invert(result);
  return result;
}

bool Matrix3::SafeInvert(Mat3Ref mat)
{
  bool success = true;

  real determinant = mat.Determinant();
  real invDeterminant;
  if(Math::Abs(determinant) < Math::PositiveMin())
  {
    invDeterminant = Math::PositiveMin();
    success = false;
  }
  else
    invDeterminant = real(1.0) / determinant;

  mat.InvertInternal(invDeterminant);
  return success;
}

Matrix3 Matrix3::Multiply(Mat3Param lhs, Mat3Param rhs)
{
  Matrix3 ret;

  ret.m00 = Dot(lhs.GetCross(0), rhs.GetBasis(0));
  ret.m01 = Dot(lhs.GetCross(0), rhs.GetBasis(1));
  ret.m02 = Dot(lhs.GetCross(0), rhs.GetBasis(2));

  ret.m10 = Dot(lhs.GetCross(1), rhs.GetBasis(0));
  ret.m11 = Dot(lhs.GetCross(1), rhs.GetBasis(1));
  ret.m12 = Dot(lhs.GetCross(1), rhs.GetBasis(2));

  ret.m20 = Dot(lhs.GetCross(2), rhs.GetBasis(0));
  ret.m21 = Dot(lhs.GetCross(2), rhs.GetBasis(1));
  ret.m22 = Dot(lhs.GetCross(2), rhs.GetBasis(2));

  return ret;
}

Vector3 Matrix3::Multiply(Mat3Param lhs, Vec3Param rhs)
{
  real x = Dot(lhs.GetCross(0), rhs);
  real y = Dot(lhs.GetCross(1), rhs);
  real z = Dot(lhs.GetCross(2), rhs);
  return Vector3(x, y, z);
}

Vector2 Matrix3::MultiplyPoint(Mat3Param lhs, Vec2Param rhs)
{
  real x = Dot(*(Vector2*)&lhs[0], rhs) + lhs[0][2];
  real y = Dot(*(Vector2*)&lhs[1], rhs) + lhs[1][2];
  return Vector2(x, y);
}

Vector2 Matrix3::MultiplyNormal(Mat3Param lhs, Vec2Param rhs)
{
  real x = Dot(*(Vector2*)&lhs[0], rhs);
  real y = Dot(*(Vector2*)&lhs[1], rhs);
  return Vector2(x, y);
}

Matrix3 Matrix3::GenerateScale(Vec2Param scale)
{
  Matrix3 result = Matrix3::cIdentity;
  result.m00 = scale.x;
  result.m11 = scale.y;
  return result;
}

Matrix3 Matrix3::GenerateScale(Vec3Param scale)
{
  Matrix3 result = Matrix3::cIdentity;
  result.m00 = scale.x;
  result.m11 = scale.y;
  result.m22 = scale.z;
  return result;
}

Matrix3 Matrix3::GenerateRotation(real radians)
{
  Matrix3 result = Matrix3::cIdentity;

  Matrix2 rotation = Matrix2::GenerateRotation(radians);
  result.m00 = rotation.m00;
  result.m01 = rotation.m01;
  result.m10 = rotation.m10;
  result.m11 = rotation.m11;
  return result;
}

Matrix3 Matrix3::GenerateRotation(Vec3Param axis, real radians)
{
  Vec3 nAxis = Math::AttemptNormalized(axis);

  Matrix3 result;
  real c0 = Math::Cos(radians);
  real n1C0 = real(1.0) - c0;
  real s0 = Math::Sin(radians);

  //| x^2(1-c0)+c0  xy(1-c0)-zs0  xz(1-c0)+ys0 |
  //| xy(1-c0)+zs0  y^2(1-c0)+c0  yz(1-c0)-xs0 |
  //| xz(1-c0)-ys0  yz(1-c0)+xs0  z^2(1-c0)+c0 |
  real x = nAxis.x;
  real y = nAxis.y;
  real z = nAxis.z;
  result.m00 = x * x * n1C0 + c0;
  result.m01 = x * y * n1C0 - z * s0;
  result.m02 = x * z * n1C0 + y * s0;

  result.m10 = x * y * n1C0 + z * s0;
  result.m11 = y * y * n1C0 + c0;
  result.m12 = y * z * n1C0 - x * s0;

  result.m20 = x * z * n1C0 - y * s0;
  result.m21 = y * z * n1C0 + x * s0;
  result.m22 = z * z * n1C0 + c0;
  return result;
}

Matrix3 Matrix3::GenerateTranslation(Vec2Param translation)
{
  Matrix3 result = Matrix3::cIdentity;
  result.m02 = translation.x;
  result.m12 = translation.y;
  return result;
}

Matrix3 Matrix3::GenerateTransform(Vec2Param translation, real rotationRadians, Vec2Param scale)
{
  Matrix3 result;

  //Translation
  result.m02 = translation.x;
  result.m12 = translation.y;
  result.m22 = real(1.0);

  //Rotation
  result.m00 = Math::Cos(rotationRadians);
  result.m01 = -Math::Sin(rotationRadians);
  result.m10 = -result.m01;
  result.m11 = result.m00;

  //Scale
  result.m00 *= scale.x;
  result.m10 *= scale.x;
  result.m01 *= scale.y;
  result.m11 *= scale.y;

  result.m20 = result.m21 = real(0.0);
  return result;
}

Matrix3 Matrix3::GenerateTransform(QuatParam rotation, Vec3Param scale)
{
  Matrix3 result;
  //Rotational component
  real xx = rotation.x * rotation.x;
  real xy = rotation.x * rotation.y;
  real xz = rotation.x * rotation.z;
  real yy = rotation.y * rotation.y;
  real yz = rotation.y * rotation.z;
  real zz = rotation.z * rotation.z;
  real zw = rotation.z * rotation.w;
  real yw = rotation.y * rotation.w;
  real xw = rotation.x * rotation.w;

  result.m00 = real(1.0) - real(2.0) * (yy + zz);
  result.m01 = real(2.0) * (xy - zw);
  result.m02 = real(2.0) * (xz + yw);

  result.m10 = real(2.0) * (xy + zw);
  result.m11 = real(1.0) - real(2.0) * (xx + zz);
  result.m12 = real(2.0) * (yz - xw);

  result.m20 = real(2.0) * (xz - yw);
  result.m21 = real(2.0) * (yz + xw);
  result.m22 = real(1.0) - real(2.0) * (xx + yy);

  //Scale component
  result.m00 *= scale.x;
  result.m10 *= scale.x;
  result.m20 *= scale.x;

  result.m01 *= scale.y;
  result.m11 *= scale.y;
  result.m21 *= scale.y;

  result.m02 *= scale.z;
  result.m12 *= scale.z;
  result.m22 *= scale.z;
  return result;
}

Matrix3 Matrix3::GenerateTransform(Mat3Param rotation, Vec3Param scale)
{
  Matrix3 result;
  //Rotational component
  result.m00 = rotation.m00;
  result.m01 = rotation.m01;
  result.m02 = rotation.m02;

  result.m10 = rotation.m10;
  result.m11 = rotation.m11;
  result.m12 = rotation.m12;

  result.m20 = rotation.m20;
  result.m21 = rotation.m21;
  result.m22 = rotation.m22;

  //Scale component
  result.m00 *= scale.x;
  result.m10 *= scale.x;
  result.m20 *= scale.x;

  result.m01 *= scale.y;
  result.m11 *= scale.y;
  result.m21 *= scale.y;

  result.m02 *= scale.z;
  result.m12 *= scale.z;
  result.m22 *= scale.z;
  return result;
}

void Matrix3::Decompose(Mat3Param transform, Vec2Ref translation, real& rotationRadians, Vec2Ref scale)
{
}

void Matrix3::Decompose(Mat3Param transform, Mat3Ref rotationRadians, Vec3Ref scale)
{
}

//-------------------------------------------------------------------Legacy

Matrix3 Matrix3::Transposed() const
{
  return Matrix3::Transposed(*this);
}

Mat3Ref Matrix3::Transpose()
{
  Matrix3::Transpose(*this);
  return *this;
}

Matrix3 Matrix3::Inverted() const
{
  return Matrix3::Inverted(*this);
}

Mat3Ref Matrix3::Invert(void)
{
  Matrix3::Invert(*this);
  return *this;
}

bool Matrix3::SafeInvert()
{
  return Matrix3::SafeInvert(*this);
}

Matrix3 Matrix3::SafeInverted() const
{
  Matrix3 result = *this;
  Matrix3::SafeInvert(result);
  return result;
}

void Matrix3::Scale(real x, real y, real z)
{
  *this = Matrix3::GenerateScale(Vector3(x, y, z));
}

void Matrix3::Scale(Vec3Param rhs)
{
  *this = Matrix3::GenerateScale(rhs);
}

void Matrix3::Rotate(real x, real y, real z, real radian)
{
  Vector3 axis = Vector3(x, y, z);
  *this = Matrix3::GenerateRotation(axis, radian);
}

void Matrix3::Rotate(Vec3Param rhs, real radian)
{
  *this = Matrix3::GenerateRotation(rhs, radian);
}

void Matrix3::Translate(real x, real y)
{
  Vector2 translation = Vector2(x, y);
  *this = Matrix3::GenerateTranslation(translation);
}

void Matrix3::Translate(Vec2Param rhs)
{
  *this = Matrix3::GenerateTranslation(rhs);
}

void Matrix3::BuildTransform(Vec2Param translate, real radians, Vec2Param scale)
{
  *this = Matrix3::GenerateTransform(translate, radians, scale);
}

void Matrix3::BuildTransform(QuatParam rotate, Vec3Param scale)
{
  *this = Matrix3::GenerateTransform(rotate, scale);
}

void Matrix3::BuildTransform(Mat3Param rotate, Vec3Param scale)
{
  *this = Matrix3::GenerateTransform(rotate, scale);
}

Mat3Ref Matrix3::Orthonormalize()
{
  Vector3 basis[3] = { BasisX(), BasisY(), BasisZ() };
  Normalize(basis[0]);

  basis[1] = Vector3::MultiplyAdd(basis[1], basis[0], -Dot(basis[1], basis[0]));
  Normalize(basis[1]);

  basis[2] = Vector3::MultiplyAdd(basis[2], basis[0], -Dot(basis[2], basis[0]));
  basis[2] = Vector3::MultiplyAdd(basis[2], basis[1], -Dot(basis[2], basis[1]));
  Normalize(basis[2]);

  SetBasis(cX, basis[0]);
  SetBasis(cY, basis[1]);
  SetBasis(cZ, basis[2]);
  return *this;
}

Matrix3::BasisVector Matrix3::BasisX() const
{
#if ColumnBasis == 1
  return Vector3(array[0], array[3], array[6]);
#else
  const Matrix3& self = *this;
  return self[0];
#endif
}

Matrix3::BasisVector Matrix3::BasisY() const
{
#if ColumnBasis == 1
  return Vector3(array[1], array[4], array[7]);
#else
  const Matrix3& self = *this;
  return self[1];
#endif
}

Matrix3::BasisVector Matrix3::BasisZ() const
{
#if ColumnBasis == 1
  return Vector3(array[2], array[5], array[8]);
#else
  const Matrix3& self = *this;
  return self[2];
#endif
}

void Matrix3::SetBasis(uint index, real x, real y, real z)
{
  Vector3 basisVector = Vector3(x, y, z);
  SetBasis(index, basisVector);
}

void Matrix3::SetCross(uint index, real x, real y, real z)
{
  Vector3 crossVector = Vector3(x, y, z);
  SetCross(index, crossVector);
}

//-------------------------------------------------------------------Global Functions
Matrix3 operator*(real lhs, Mat3Param rhs)
{
  return rhs * lhs;
}

Matrix3 Multiply(Mat3Param lhs, Mat3Param rhs)
{
  return Matrix3::Multiply(lhs, rhs);
}

Vector3 Multiply(Mat3Param lhs, Vec3Param rhs)
{
  return Matrix3::Multiply(lhs, rhs);
}

Vector2 MultiplyPoint(Mat3Param lhs, Vec2Param rhs)
{
  return Matrix3::MultiplyPoint(lhs, rhs);
}

Vector2 MultiplyNormal(Mat3Param lhs, Vec2Param rhs)
{
  return Matrix3::MultiplyNormal(lhs, rhs);
}

//-------------------------------------------------------------------Legacy
Matrix3 BuildTransform(Vec2Param translate, real radians, Vec2Param scale)
{
  return Matrix3::GenerateTransform(translate, radians, scale);
}

Matrix3 BuildTransform(QuatParam rotate, Vec3Param scale)
{
  return Matrix3::GenerateTransform(rotate, scale);
}

Vector3 Transform(Mat3Param matrix, Vec3Param vector)
{
  return Matrix3::Multiply(matrix, vector);
}

void Transform(Mat3Param matrix, Vec3Ptr vector)
{
  ErrorIf(vector == nullptr, "Matrix3 - Null pointer passed for vector.");
  Vector3 result = Matrix3::Multiply(matrix, *vector);
  
  vector->Set(result.x, result.y, result.z);
}

Vector2 TransformPoint(Mat3Param matrix, Vec2Param vector)
{
  return Matrix3::MultiplyPoint(matrix, vector);
}

Vector2 TransformNormal(Mat3Param matrix, Vec2Param normal)
{
  return Matrix3::MultiplyNormal(matrix, normal);
}

Vector3 TransposedTransform(Mat3Param matrix, Vec3Param vector)
{
  real x = Dot(matrix.GetBasis(0), vector);
  real y = Dot(matrix.GetBasis(1), vector);
  real z = Dot(matrix.GetBasis(2), vector);
  return Vector3(x, y, z);
}

void TransposedTransform(Mat3Param matrix, Vec3Ptr vector)
{
  ErrorIf(vector == nullptr, "Matrix3 - Null pointer passed for vector.");
  real x = Dot(matrix.GetBasis(0), *vector);
  real y = Dot(matrix.GetBasis(1), *vector);
  real z = Dot(matrix.GetBasis(2), *vector);
  vector->Set(x, y, z);
}

real Trace(Mat3Param matrix)
{
  return matrix.m00 + matrix.m11 + matrix.m22;
}

real Cofactor(Mat3Param matrix, uint row, uint column)
{
  ErrorIf(row > 2, "Matrix3 - Row index out of range.");
  ErrorIf(column > 2, "Matrix3 - Column index out of range.");

  // Negative if r+c is odd, positive if even
  real sign = ((row + column) % 2) == 1 ? real(-1.0) : real(1.0);

  real matrix2[4];
  uint i = 0;
  for(uint r = 0; r < 3; ++r)
  {
    if(r != row)
    {
      for(uint c = 0; c < 3; ++c)
      {
        if(c != column)
        {
          matrix2[i] = matrix(r, c);
          ++i;
        }
      }
    }
  }

  return sign * ((matrix2[0] * matrix2[3]) - (matrix2[1] * matrix2[2]));
}

void Diagonalize(Mat3Ptr matrix)
{
  ///Diagonalizes a symmetric matrix (M = M^T)
  ErrorIf(matrix == nullptr, "Matrix3 - Null pointer passed for matrix.");
  
  Matrix3 quatMatrix = ToMatrix3(CreateDiagonalizer(*matrix));
  *matrix = Multiply(Multiply(quatMatrix, *matrix), quatMatrix.Transposed());
}

Matrix3 Diagonalized(Mat3Param matrix)
{
  Matrix3 newMatrix = matrix;
  Diagonalize(&newMatrix);
  return newMatrix;
}

void Invert(Mat3Ptr matrix)
{
  Matrix3::Invert(*matrix);
}

Matrix3 Inverted(Mat3Param matrix)
{
  return Matrix3::Inverted(matrix);
}

}// namespace Math
