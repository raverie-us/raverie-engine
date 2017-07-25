///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{

const Matrix4 Matrix4::cIdentity(real(1.0), real(0.0), real(0.0), real(0.0),
                                 real(0.0), real(1.0), real(0.0), real(0.0),
                                 real(0.0), real(0.0), real(1.0), real(0.0),
                                 real(0.0), real(0.0), real(0.0), real(1.0));

Matrix4::Matrix4(real p00, real p01, real p02, real p03, real p10, real p11, 
                 real p12, real p13, real p20, real p21, real p22, real p23,
                 real p30, real p31, real p32, real p33)
{
  m00 = p00;  m01 = p01;  m02 = p02;  m03 = p03; 
  m10 = p10;  m11 = p11;  m12 = p12;  m13 = p13; 
  m20 = p20;  m21 = p21;  m22 = p22;  m23 = p23;
  m30 = p30;  m31 = p31;  m32 = p32;  m33 = p33;
}

Matrix4::Matrix4(Vec4Param basisX, Vec4Param basisY,
  Vec4Param basisZ, Vec4Param basisW)
{
  SetBasis(0,basisX);
  SetBasis(1,basisY);
  SetBasis(2,basisZ);
  SetBasis(3,basisW);
}

Matrix4::Matrix4(ConstRealPointer data_)
{
  m00 = data_[0];  m01 = data_[1];  m02 = data_[2];  m03 = data_[3];
  m10 = data_[4];  m11 = data_[5];  m12 = data_[6];  m13 = data_[7];
  m20 = data_[8];  m21 = data_[9];  m22 = data_[10];  m23 = data_[11];
  m30 = data_[12];  m31 = data_[13];  m32 = data_[14];  m33 = data_[15];
}

const Vector4& Matrix4::operator[](uint index) const
{
  return ((Vector4*)this)[index];
}

Vector4& Matrix4::operator[](uint index)
{
  return ((Vector4*)this)[index];
}

real Matrix4::operator()(uint r, uint c) const
{
  ErrorIf(r > 3, "Matrix4 - Index out of range.");
  ErrorIf(c > 3, "Matrix4 - Index out of range.");

#if ColumnBasis == 1
  return array[c + r * 4];
#else
  return array[r + c * 4];
#endif
}

real& Matrix4::operator()(uint r, uint c)
{
  ErrorIf(r > 3, "Matrix4 - Index out of range.");
  ErrorIf(c > 3, "Matrix4 - Index out of range.");

#if ColumnBasis == 1
  return array[c + r * 4];
#else
  return array[r + c * 4];
#endif
}

void Matrix4::operator*=(real rhs)
{
  Matrix4& self = *this;
  self[0] *= rhs;
  self[1] *= rhs;
  self[2] *= rhs;
  self[3] *= rhs;
}

void Matrix4::operator/=(real rhs)
{
  Matrix4& self = *this;
  ErrorIf(Math::IsZero(rhs), "Matrix4 - Division by zero.");
  self[0] /= rhs;
  self[1] /= rhs;
  self[2] /= rhs;
  self[3] /= rhs;
}

Matrix4 Matrix4::operator*(real rhs) const
{
  Matrix4 ret = *this;
  ret *= rhs;
  return ret;
}

Matrix4 Matrix4::operator/(real rhs) const
{
  ErrorIf(Math::IsZero(rhs), "Matrix4 - Division by zero.");
  Matrix4 ret = *this;
  ret /= rhs;
  return ret;
}

void Matrix4::operator+=(Mat4Param rhs)
{
  Matrix4& self = *this;
  self[0] += rhs[0];
  self[1] += rhs[1];
  self[2] += rhs[2];
  self[3] += rhs[3];
}

void Matrix4::operator-=(Mat4Param rhs)
{
  Matrix4& self = *this;
  self[0] -= rhs[0];
  self[1] -= rhs[1];
  self[2] -= rhs[2];
  self[3] -= rhs[3];
}

Matrix4 Matrix4::operator+(Mat4Param rhs) const
{
  Matrix4 ret = *this;
  ret += rhs;
  return ret;
}

Matrix4 Matrix4::operator-(Mat4Param rhs) const
{
  Matrix4 ret = *this;
  ret -= rhs;
  return ret;
}

Matrix4 Matrix4::operator*(Mat4Param rhs) const
{
  return Matrix4::Multiply(*this, rhs);
}

bool Matrix4::operator==(Mat4Param rhs) const
{
  const Matrix4& self = *this;
  return self[0] == rhs[0] &&
         self[1] == rhs[1] &&
         self[2] == rhs[2] &&
         self[3] == rhs[3];
}

bool Matrix4::operator!=(Mat4Param rhs) const
{
  return !(*this == rhs);
}

Mat4Ref Matrix4::ZeroOut()
{
  Matrix4& self = *this;
  self[0].ZeroOut();
  self[1].ZeroOut();
  self[2].ZeroOut();
  self[3].ZeroOut();
  return *this;
}

Mat4Ref Matrix4::SetIdentity()
{
  Matrix4& self = *this;
  self[0].Set(real(1.0), real(0.0), real(0.0), real(0.0));
  self[1].Set(real(0.0), real(1.0), real(0.0), real(0.0));
  self[2].Set(real(0.0), real(0.0), real(1.0), real(0.0));
  self[3].Set(real(0.0), real(0.0), real(0.0), real(1.0));
  return *this;
}

Matrix4::BasisVector Matrix4::GetBasis(uint index) const
{
  ErrorIf(index > 3, "Matrix4 - Index out of range.");
#if ColumnBasis == 1
  return Vector4(array[index], array[4 + index],
    array[8 + index], array[12 + index]);
#else
  Mat4Param self = *this;
  return self[index];
#endif
}

void Matrix4::SetBasis(uint index, Vec4Param basis)
{
  ErrorIf(index > 3, "Matrix4 - Index out of range.");
#if ColumnBasis == 1
  array[index +  0] = basis.x;
  array[index +  4] = basis.y;
  array[index +  8] = basis.z;
  array[index + 12] = basis.w;
#else
  Mat4Param self = *this;
  self[index].Set(basis.x, basis.y, basis.z, basis.w);
#endif
}

Vector3 Matrix4::GetBasis3(uint index) const
{
  ErrorIf(index > 3, "Matrix4 - Index out of range.");
#if ColumnBasis == 1
  return Vector3(array[index], array[4 + index], array[8 + index]);
#else
  Mat4Param self = *this;
  return Vector3(self[index].x, self[index].y, self[index].z);
#endif
}

Matrix4::CrossVector Matrix4::GetCross(uint index) const
{
  ErrorIf(index > 3, "Matrix4 - Index out of range.");
#if ColumnBasis == 1
  Mat4Param self = *this;
  return self[index];
#else
  return Vector4(array[index], array[4 + index],
    array[8 + index], array[12 + index]);
#endif
}

void Matrix4::SetCross(uint index, Vec4Param cross)
{
  ErrorIf(index > 3, "Matrix4 - Index out of range.");
#if ColumnBasis == 1
  Mat4Ref self = *this;
  self[index].Set(cross.x, cross.y, cross.z, cross.w);
#else
  array[index +  0] = cross.x;
  array[index +  4] = cross.y;
  array[index +  8] = cross.z;
  array[index + 12] = cross.w;
#endif
}

bool Matrix4::Valid() const
{
  const Matrix4& self = *this;
  return self[0].Valid() && self[1].Valid() && 
         self[2].Valid() && self[3].Valid();
}

real Matrix4::Determinant() const
{
  real det  = m03 * m12 * m21 * m30 - m02 * m13 * m21 * m30;
       det += m01 * m13 * m22 * m30 - m03 * m11 * m22 * m30;
       det += m02 * m11 * m23 * m30 - m01 * m12 * m23 * m30;
       det += m02 * m13 * m20 * m31 - m03 * m12 * m20 * m31;
       det += m03 * m10 * m22 * m31 - m00 * m13 * m22 * m31;
       det += m00 * m12 * m23 * m31 - m02 * m10 * m23 * m31;
       det += m03 * m11 * m20 * m32 - m01 * m13 * m20 * m32;
       det += m00 * m13 * m21 * m32 - m03 * m10 * m21 * m32;
       det += m01 * m10 * m23 * m32 - m00 * m11 * m23 * m32;
       det += m01 * m12 * m20 * m33 - m02 * m11 * m20 * m33;
       det += m02 * m10 * m21 * m33 - m00 * m12 * m21 * m33;
       det += m00 * m11 * m22 * m33 - m01 * m10 * m22 * m33;
   return det;
}

void Matrix4::InvertInternal(real invDeterminant)
{
  real t00;
  t00 = m12 * m23 * m31 - m13 * m22 * m31;
  t00 += m13 * m21 * m32 - m11 * m23 * m32;
  t00 += m11 * m22 * m33 - m12 * m21 * m33;
  t00 *= invDeterminant;

  real t01;
  t01 = m03 * m22 * m31 - m02 * m23 * m31;
  t01 += m01 * m23 * m32 - m03 * m21 * m32;
  t01 += m02 * m21 * m33 - m01 * m22 * m33;
  t01 *= invDeterminant;

  real t02;
  t02 = m02 * m13 * m31 - m03 * m12 * m31;
  t02 += m03 * m11 * m32 - m01 * m13 * m32;
  t02 += m01 * m12 * m33 - m02 * m11 * m33;
  t02 *= invDeterminant;

  real t03;
  t03 = m03 * m12 * m21 - m02 * m13 * m21;
  t03 += m01 * m13 * m22 - m03 * m11 * m22;
  t03 += m02 * m11 * m23 - m01 * m12 * m23;
  t03 *= invDeterminant;

  real t10;
  t10 = m13 * m22 * m30 - m12 * m23 * m30;
  t10 += m10 * m23 * m32 - m13 * m20 * m32;
  t10 += m12 * m20 * m33 - m10 * m22 * m33;
  t10 *= invDeterminant;

  real t11;
  t11 = m02 * m23 * m30 - m03 * m22 * m30;
  t11 += m03 * m20 * m32 - m00 * m23 * m32;
  t11 += m00 * m22 * m33 - m02 * m20 * m33;
  t11 *= invDeterminant;

  real t12;
  t12 = m03 * m12 * m30 - m02 * m13 * m30;
  t12 += m00 * m13 * m32 - m03 * m10 * m32;
  t12 += m02 * m10 * m33 - m00 * m12 * m33;
  t12 *= invDeterminant;

  real t13;
  t13 = m02 * m13 * m20 - m03 * m12 * m20;
  t13 += m03 * m10 * m22 - m00 * m13 * m22;
  t13 += m00 * m12 * m23 - m02 * m10 * m23;
  t13 *= invDeterminant;

  real t20;
  t20 = m11 * m23 * m30 - m13 * m21 * m30;
  t20 += m13 * m20 * m31 - m10 * m23 * m31;
  t20 += m10 * m21 * m33 - m11 * m20 * m33;
  t20 *= invDeterminant;

  real t21;
  t21 = m03 * m21 * m30 - m01 * m23 * m30;
  t21 += m00 * m23 * m31 - m03 * m20 * m31;
  t21 += m01 * m20 * m33 - m00 * m21 * m33;
  t21 *= invDeterminant;

  real t22;
  t22 = m01 * m13 * m30 - m03 * m11 * m30;
  t22 += m03 * m10 * m31 - m00 * m13 * m31;
  t22 += m00 * m11 * m33 - m01 * m10 * m33;
  t22 *= invDeterminant;

  real t23;
  t23 = m03 * m11 * m20 - m01 * m13 * m20;
  t23 += m00 * m13 * m21 - m03 * m10 * m21;
  t23 += m01 * m10 * m23 - m00 * m11 * m23;
  t23 *= invDeterminant;

  real t30;
  t30 = m12 * m21 * m30 - m11 * m22 * m30;
  t30 += m10 * m22 * m31 - m12 * m20 * m31;
  t30 += m11 * m20 * m32 - m10 * m21 * m32;
  t30 *= invDeterminant;

  real t31;
  t31 = m01 * m22 * m30 - m02 * m21 * m30;
  t31 += m02 * m20 * m31 - m00 * m22 * m31;
  t31 += m00 * m21 * m32 - m01 * m20 * m32;
  t31 *= invDeterminant;

  real t32;
  t32 = m02 * m11 * m30 - m01 * m12 * m30;
  t32 += m00 * m12 * m31 - m02 * m10 * m31;
  t32 += m01 * m10 * m32 - m00 * m11 * m32;
  t32 *= invDeterminant;

  real t33;
  t33 = m01 * m12 * m20 - m02 * m11 * m20;
  t33 += m02 * m10 * m21 - m00 * m12 * m21;
  t33 += m00 * m11 * m22 - m01 * m10 * m22;
  t33 *= invDeterminant;

  m00 = t00; m01 = t01; m02 = t02; m03 = t03;
  m10 = t10; m11 = t11; m12 = t12; m13 = t13;
  m20 = t20; m21 = t21; m22 = t22; m23 = t23;
  m30 = t30; m31 = t31; m32 = t32; m33 = t33;
}

void Matrix4::Transpose(Mat4Ref mat)
{
  Math::Swap(mat.m01, mat.m10);
  Math::Swap(mat.m02, mat.m20);
  Math::Swap(mat.m03, mat.m30);
  Math::Swap(mat.m12, mat.m21);
  Math::Swap(mat.m13, mat.m31);
  Math::Swap(mat.m23, mat.m32);
}

Matrix4 Matrix4::Transposed(Mat4Param mat)
{
  Matrix4 result = mat;
  Matrix4::Transpose(result);
  return result;
}

real Matrix4::Determinant(Mat4Param mat)
{
  return mat.Determinant();
}

void Matrix4::Invert(Mat4Ref mat)
{
  real determinant = mat.Determinant();
  ErrorIf(Math::IsZero(determinant), "Matrix4 - Uninvertible matrix.");
  
  real invDeterminant = real(1.0) / determinant;
  mat.InvertInternal(invDeterminant);
}

Matrix4 Matrix4::Inverted(Mat4Param mat)
{
  Matrix4 result = mat;
  Matrix4::Invert(result);
  return result;
}

bool Matrix4::SafeInvert(Mat4Ref mat)
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

Matrix4 Matrix4::Multiply(Mat4Param lhs, Mat4Param rhs)
{
  Matrix4 result;

  result.m00 = Dot(lhs.GetCross(0), rhs.GetBasis(0));
  result.m01 = Dot(lhs.GetCross(0), rhs.GetBasis(1));
  result.m02 = Dot(lhs.GetCross(0), rhs.GetBasis(2));
  result.m03 = Dot(lhs.GetCross(0), rhs.GetBasis(3));

  result.m10 = Dot(lhs.GetCross(1), rhs.GetBasis(0));
  result.m11 = Dot(lhs.GetCross(1), rhs.GetBasis(1));
  result.m12 = Dot(lhs.GetCross(1), rhs.GetBasis(2));
  result.m13 = Dot(lhs.GetCross(1), rhs.GetBasis(3));

  result.m20 = Dot(lhs.GetCross(2), rhs.GetBasis(0));
  result.m21 = Dot(lhs.GetCross(2), rhs.GetBasis(1));
  result.m22 = Dot(lhs.GetCross(2), rhs.GetBasis(2));
  result.m23 = Dot(lhs.GetCross(2), rhs.GetBasis(3));

  result.m30 = Dot(lhs.GetCross(3), rhs.GetBasis(0));
  result.m31 = Dot(lhs.GetCross(3), rhs.GetBasis(1));
  result.m32 = Dot(lhs.GetCross(3), rhs.GetBasis(2));
  result.m33 = Dot(lhs.GetCross(3), rhs.GetBasis(3));

  return result;
}

Vector4 Matrix4::Multiply(Mat4Param lhs, Vec4Param rhs)
{
  real x = Dot(lhs.GetCross(0), rhs);
  real y = Dot(lhs.GetCross(1), rhs);
  real z = Dot(lhs.GetCross(2), rhs);
  real w = Dot(lhs.GetCross(3), rhs);
  return Vector4(x, y, z, w);
}

Vector3 Matrix4::MultiplyPoint(Mat4Param lhs, Vec3Param rhs)
{
  real x = Dot(*(Vector3*)&lhs[0], rhs) + lhs[0][3];
  real y = Dot(*(Vector3*)&lhs[1], rhs) + lhs[1][3];
  real z = Dot(*(Vector3*)&lhs[2], rhs) + lhs[2][3];
  return Vector3(x, y, z);
}

Vector3 Matrix4::MultiplyNormal(Mat4Param lhs, Vec3Param rhs)
{
  real x = Dot(*(Vector3*)&lhs[0], rhs);
  real y = Dot(*(Vector3*)&lhs[1], rhs);
  real z = Dot(*(Vector3*)&lhs[2], rhs);
  return Vector3(x, y, z);
}

Matrix4 Matrix4::GenerateScale(Vec3Param scale)
{
  Matrix4 result = cIdentity;
  result.m00 = scale.x;
  result.m11 = scale.y;
  result.m22 = scale.z;
  return result;
}

Matrix4 Matrix4::GenerateRotation(Vec3Param axis, real rotationRadians)
{
  Vec3 nAxis = Math::AttemptNormalized(axis);

  Matrix4 result;
  real c0 = Math::Cos(rotationRadians);
  real n1C0 = real(1.0) - c0;
  real s0 = Math::Sin(rotationRadians);

  //| x^2(1-c0)+c0  xy(1-c0)-zs0  xz(1-c0)+ys0 |
  //| xy(1-c0)+zs0  y^2(1-c0)+c0  yz(1-c0)-xs0 |
  //| xz(1-c0)-ys0  yz(1-c0)+xs0  z^2(1-c0)+c0 |
  real x = nAxis.x;
  real y = nAxis.y;
  real z = nAxis.z;
  result.SetCross(cX, x * x * n1C0 + c0, x * y * n1C0 - z * s0, x * z * n1C0 + y * s0, real(0.0));
  result.SetCross(cY, x * y * n1C0 + z * s0, y * y * n1C0 + c0, y * z * n1C0 - x * s0, real(0.0));
  result.SetCross(cZ, x * z * n1C0 - y * s0, y * z * n1C0 + x * s0, z * z * n1C0 + c0, real(0.0));
  result.SetCross(cW, real(0.0), real(0.0), real(0.0), real(1.0));
  return result;
}

Matrix4 Matrix4::GenerateTranslation(Vec3Param translation)
{
  Matrix4 result = cIdentity;
  result.m03 = translation.x;
  result.m13 = translation.y;
  result.m23 = translation.z;
  return result;
}

Matrix4 Matrix4::GenerateTransform(Vec3Param translation, QuatParam rotatation, Vec3Param scale)
{
  Matrix4 result;
  //Translation component
  result.m03 = translation.x;
  result.m13 = translation.y;
  result.m23 = translation.z;
  result.m33 = real(1.0);

  //Rotational component
  real xx = rotatation.x * rotatation.x;
  real xy = rotatation.x * rotatation.y;
  real xz = rotatation.x * rotatation.z;
  real yy = rotatation.y * rotatation.y;
  real yz = rotatation.y * rotatation.z;
  real zz = rotatation.z * rotatation.z;
  real zw = rotatation.z * rotatation.w;
  real yw = rotatation.y * rotatation.w;
  real xw = rotatation.x * rotatation.w;

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

  result.m30 = result.m31 = result.m32 = real(0.0);
  return result;
}

Matrix4 Matrix4::GenerateTransform(Vec3Param translation, Mat3Param rotatation, Vec3Param scale)
{
  Matrix4 result;
  //Translation component
  result.m03 = translation.x;
  result.m13 = translation.y;
  result.m23 = translation.z;
  result.m33 = real(1.0);

  //Rotational component
  result.m00 = rotatation.m00;
  result.m01 = rotatation.m01;
  result.m02 = rotatation.m02;

  result.m10 = rotatation.m10;
  result.m11 = rotatation.m11;
  result.m12 = rotatation.m12;

  result.m20 = rotatation.m20;
  result.m21 = rotatation.m21;
  result.m22 = rotatation.m22;

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

  result.m30 = result.m31 = result.m32 = real(0.0);
  return result;
}

void Matrix4::Decompose(Mat4Param transform, Vec3Ref translation, Mat3Ref rotation, Vec3Ref scale)
{
  Vector3 shear;
  Matrix4::Decompose(transform, translation, rotation, shear, scale);
}

void Matrix4::Decompose(Mat4Param transform, Vec3Ref translation, Mat3Ref rotation, Vec3Ref shear, Vec3Ref scale)
{
  //Translation is the last basis vector
  translation.x = transform.m03;
  translation.y = transform.m13;
  translation.z = transform.m23;

  //X' == first basis vector
  //Y' == second basis vector
  //Z' == third basis vector

  //                         X'                             Y'                             Z'
  rotation.m00 = transform.m00;  rotation.m01 = transform.m01;  rotation.m02 = transform.m02;
  rotation.m10 = transform.m10;  rotation.m11 = transform.m11;  rotation.m12 = transform.m12;
  rotation.m20 = transform.m20;  rotation.m21 = transform.m21;  rotation.m22 = transform.m22;

  //ScaleX is the magnitude of X'
  scale.x = Math::Sqrt(Math::Sq(transform.m00) + Math::Sq(transform.m10) + Math::Sq(transform.m20));

  //X' is normalized
  rotation.m00 /= scale.x;
  rotation.m10 /= scale.x;
  rotation.m20 /= scale.x;

  //ShearXY is the dot product of X' and Y'
  shear.z = rotation.m00 * rotation.m01 +
            rotation.m10 * rotation.m11 +
            rotation.m20 * rotation.m21;

  //Make Y' orthogonal to X' by " Y' = Y' - (ShearXY * X') "
  rotation.m01 -= shear.z * rotation.m00;
  rotation.m11 -= shear.z * rotation.m10;
  rotation.m21 -= shear.z * rotation.m20;

  //ScaleY is the magnitude of the modified Y'
  scale.y = Math::Sqrt(Math::Sq(transform.m01) + Math::Sq(transform.m11) + Math::Sq(transform.m21));

  //Y' is normalized
  rotation.m01 /= scale.y;
  rotation.m11 /= scale.y;
  rotation.m21 /= scale.y;

  //ShearXY is divided by ScaleY to get it's final value
  shear.z /= scale.y;

  //ShearXZ is the dot product of X' and Z'
  shear.y = rotation.m00 * rotation.m02 +
            rotation.m10 * rotation.m12 +
            rotation.m20 * rotation.m22;

  //ShearYZ is the dot product of Y' and Z'
  shear.x = rotation.m01 * rotation.m02 +
            rotation.m11 * rotation.m12 +
            rotation.m21 * rotation.m22;

  //Make Z' orthogonal to X' by " Z' = Z' - (ShearXZ * X') "
  rotation.m02 -= shear.y * rotation.m00;
  rotation.m12 -= shear.y * rotation.m10;
  rotation.m22 -= shear.y * rotation.m20;

  //Make Z' orthogonal to Y' by " Z' = Z' - (ShearYZ * Y') "
  rotation.m02 -= shear.x * rotation.m01;
  rotation.m12 -= shear.x * rotation.m11;
  rotation.m22 -= shear.x * rotation.m21;

  //ScaleZ is the magnitude of the modified Z'
  scale.z = Math::Sqrt(Math::Sq(transform.m02) + Math::Sq(transform.m12) + Math::Sq(transform.m22));

  //Z' is normalized
  rotation.m02 /= scale.z;
  rotation.m12 /= scale.z;
  rotation.m22 /= scale.z;

  //ShearXZ is divided by ScaleZ to get it's final value
  shear.y /= scale.z;

  //ShearYZ is divided by ScaleZ to get it's final value
  shear.x /= scale.z;

  //If the determinant is negative, then the rotation and scale contain a flip
  Vector3 v = Vector3(rotation.m11 * rotation.m22 - rotation.m21 * rotation.m12,
              rotation.m21 * rotation.m02 - rotation.m01 * rotation.m22,
              rotation.m01 * rotation.m12 - rotation.m11 * rotation.m02);
  real dot = v.x * rotation.m00 + v.y * rotation.m10 + v.z * rotation.m20;
  if(dot < real(0.0))
  {
    rotation *= real(-1.0);
    scale *= real(-1.0);
  }
}

//-------------------------------------------------------------------Legacy

Mat4Ref Matrix4::Transpose()
{
  Matrix4::Transpose(*this);
  return *this;
}

Matrix4 Matrix4::Transposed() const
{
  return Matrix4::Transposed(*this);
}

Mat4Ref Matrix4::Invert()
{
  Matrix4::Invert(*this);
  return *this;
}

Matrix4 Matrix4::Inverted() const
{
  return Matrix4::Inverted(*this);
}

bool Matrix4::SafeInvert()
{
  return Matrix4::SafeInvert(*this);
}

Matrix4 Matrix4::SafeInverted() const
{
  Matrix4 result(*this);
  Matrix4::SafeInvert(result);
  return result;
}

void Matrix4::Scale(real x, real y, real z)
{
  Vector3 scale(x, y, z);
  *this = Matrix4::GenerateScale(scale);
}

void Matrix4::Scale(Vec3Param axis)
{
  *this = Matrix4::GenerateScale(axis);
}

void Matrix4::Rotate(real x, real y, real z, real radians)
{
  Vector3 axis(x, y, z);
  *this = Matrix4::GenerateRotation(axis, radians);
}

void Matrix4::Rotate(Vec3Param axis, real radians)
{
  *this = Matrix4::GenerateRotation(axis, radians);
}

void Matrix4::Translate(real x, real y, real z)
{
  Vector3 translation(x, y, z);
  *this = Matrix4::GenerateTranslation(translation);
}

void Matrix4::Translate(Vec3Param axis)
{
  *this = Matrix4::GenerateTranslation(axis);
}

void Matrix4::BuildTransform(Vec3Param translate, QuatParam rotate, 
                             Vec3Param scale)
{
  *this = Matrix4::GenerateTransform(translate, rotate, scale);
}

void Matrix4::BuildTransform(Vec3Param translate, Mat3Param rotate, 
                             Vec3Param scale)
{
  *this = Matrix4::GenerateTransform(translate, rotate, scale);
}

void Matrix4::Decompose(Vec3Ptr scale, Mat3Ptr rotate, Vec3Ptr translate) const
{
  ErrorIf(scale == nullptr, "Matrix4 - Null pointer passed for scale.");
  ErrorIf(rotate == nullptr, "Matrix4 - Null pointer passed for rotation.");
  ErrorIf(translate == nullptr, "Matrix4 - Null pointer passed for translation.");
  Matrix4::Decompose(*this, *translate, *rotate, *scale);
}

//Shear values that are calculated are XY, XZ, and YZ. They are stored as the 
//element their name does not contain, so shear->x would have YZ in it
void Matrix4::Decompose(Vec3Ptr scale, Vec3Ptr shear, Mat3Ptr rotate, 
                        Vec3Ptr translate) const
{
  ErrorIf(scale == nullptr, "Matrix4 - Null pointer passed for scale.");
  ErrorIf(shear == nullptr, "Matrix4 - Null pointer passed for shear.");
  ErrorIf(rotate == nullptr, "Matrix4 - Null pointer passed for rotation.");
  ErrorIf(translate == nullptr, "Matrix4 - Null pointer passed for translation.");

  Matrix4::Decompose(*this, *translate, *rotate, *shear, *scale);
}

Matrix4::BasisVector Matrix4::BasisX() const
{
#if ColumnBasis == 1
  return Vector4(array[0], array[4], array[8], array[12]);
#else
  Mat4Param self = *this;
  return self[0];
#endif
}

Matrix4::BasisVector Matrix4::BasisY() const
{
#if ColumnBasis == 1
  return Vector4(array[1], array[5], array[9], array[13]);
#else
  Mat4Param self = *this;
  return self[1];
#endif
}

Matrix4::BasisVector Matrix4::BasisZ() const
{
#if ColumnBasis == 1
  return Vector4(array[2], array[6], array[10], array[14]);
#else
  Mat4Param self = *this;
  return self[2];
#endif
}

Matrix4::BasisVector Matrix4::BasisW() const
{
#if ColumnBasis == 1
  return Vector4(array[3], array[7], array[11], array[15]);
#else
  Mat4Param self = *this;
  return self[3];
#endif
}

void Matrix4::SetBasis(uint index, Vec3Param basisVector3, real w)
{
  SetBasis(index, Vector4(basisVector3.x, basisVector3.y, basisVector3.z, w));
}

void Matrix4::SetBasis(uint index, real x, Vec3Param basisVector3)
{
  SetBasis(index, Vector4(x, basisVector3.x, basisVector3.y, basisVector3.z));
}

void Matrix4::SetBasis(uint index, real x, real y, real z, real w)
{
  SetBasis(index, Vector4(x, y, z, w));
}

void Matrix4::SetCross(uint index, Vec3Param crossVector3, real w)
{
  SetCross(index, Vector4(crossVector3.x, crossVector3.y, crossVector3.z, w));
}

void Matrix4::SetCross(uint index, real x, real y, real z, real w)
{
  SetCross(index, Vector4(x, y, z, w));
}

//-------------------------------------------------------------------Global Functions
Matrix4 operator*(real lhs, Mat4Param rhs)
{
  return rhs * lhs;
}

Matrix4 Multiply(Mat4Param lhs, Mat4Param rhs)
{
  return Matrix4::Multiply(lhs, rhs);
}

Vector4 Multiply(Mat4Param lhs, Vec4Param rhs)
{
  return Matrix4::Multiply(lhs, rhs);
}

Vector3 MultiplyPoint(Mat4Param lhs, Vec3Param rhs)
{
  return Matrix4::MultiplyPoint(lhs, rhs);
}

Vector3 MultiplyNormal(Mat4Param lhs, Vec3Param rhs)
{
  return Matrix4::MultiplyNormal(lhs, rhs);
}

//-------------------------------------------------------------------Legacy
Vector4 Transform(Mat4Param mat, Vec4Param vector)
{
  return Matrix4::Multiply(mat, vector);
}

void Transform(Mat4Param mat, Vec4Ptr vector)
{
  ErrorIf(vector == nullptr, "Matrix4 - Null pointer passed for vector.");

  Vector4 result = Matrix4::Multiply(mat, *vector);
  *vector = result;
}

Vector3 TransformPoint(Mat4Param matrix, Vec3Param point)
{
  return Matrix4::MultiplyPoint(matrix, point);
}

Vector3 TransformNormal(Mat4Param matrix, Vec3Param normal)
{
  return Matrix4::MultiplyNormal(matrix, normal);
}

Vector3 TransformPointProjected(Mat4Param matrix, Vec3Param point)
{
  real x = Dot(*(Vector3*)&matrix[0], point) + matrix[3][0];
  real y = Dot(*(Vector3*)&matrix[1], point) + matrix[3][1];
  real z = Dot(*(Vector3*)&matrix[2], point) + matrix[3][2];
  real w = Dot(*(Vector3*)&matrix[3], point) + matrix[3][3];
  return Vector3(x / w, y / w, z / w);
}

Vector3 TransformNormalCol(Mat4Param matrix, Vec3Param normal)
{
  real x = Dot(Vector3(matrix.m00, matrix.m10, matrix.m20), normal);
  real y = Dot(Vector3(matrix.m01, matrix.m11, matrix.m21), normal);
  real z = Dot(Vector3(matrix.m02, matrix.m12, matrix.m22), normal);
  return Vector3(x, y, z);
}

Vector3 TransformPointCol(Mat4Param matrix, Vec3Param point)
{
  real x = Dot(Vector3(matrix.m00, matrix.m10, matrix.m20), point) + matrix[3][0];
  real y = Dot(Vector3(matrix.m01, matrix.m11, matrix.m21), point) + matrix[3][1];
  real z = Dot(Vector3(matrix.m02, matrix.m12, matrix.m22), point) + matrix[3][2];
  return Vector3(x, y, z);
}

Vector3 TransformPointProjectedCol(Mat4Param matrix, Vec3Param point)
{
  real x = Dot(Vector3(matrix.m00, matrix.m10, matrix.m20), point) + matrix[3][0];
  real y = Dot(Vector3(matrix.m01, matrix.m11, matrix.m21), point) + matrix[3][1];
  real z = Dot(Vector3(matrix.m02, matrix.m12, matrix.m22), point) + matrix[3][2];
  real w = Dot(Vector3(matrix.m03, matrix.m13, matrix.m23), point) + matrix[3][3];
  return Vector3(x / w, y / w, z / w);
}

Vector3 TransformPointProjectedCol(Mat4Param matrix, Vec3Param point, real* wOut)
{
  real x = Dot(Vector3(matrix.m00, matrix.m10, matrix.m20), point) + matrix[3][0];
  real y = Dot(Vector3(matrix.m01, matrix.m11, matrix.m21), point) + matrix[3][1];
  real z = Dot(Vector3(matrix.m02, matrix.m12, matrix.m22), point) + matrix[3][2];
  real w = Dot(Vector3(matrix.m03, matrix.m13, matrix.m23), point) + matrix[3][3];
  *wOut = w;
  return Vector3(x / w, y / w, z / w);
}

Matrix4 BuildTransform(Vec3Param translate, QuatParam rotate, Vec3Param scale)
{
  return Matrix4::GenerateTransform(translate, rotate, scale);
}

Matrix4 BuildTransform(Vec3Param translate, Mat3Param rotate, Vec3Param scale)
{
  return Matrix4::GenerateTransform(translate, rotate, scale);
}

real Trace(Mat4Param matrix)
{
  return matrix.m00 + matrix.m11 + matrix.m22 + matrix.m33;
}

}// namespace Math

