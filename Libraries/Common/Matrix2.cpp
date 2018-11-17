///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{

const Matrix2 Matrix2::cIdentity(real(1.0), real(0.0),
                                 real(0.0), real(1.0));

Matrix2::Matrix2(real p00, real p01,
                 real p10, real p11)
{
  m00 = p00; m01 = p01;
  m10 = p10; m11 = p11;
}

Matrix2::Matrix2(ConstRealPointer data_)
{
  m00 = data_[0]; m01 = data_[1];
  m10 = data_[2]; m11 = data_[3];
}

Vec2Ref Matrix2::operator[](uint index)
{
  return ((Vector2*)this)[index];
}

Vec2Param Matrix2::operator[](uint index) const
{
  return ((Vector2*)this)[index];
}

real& Matrix2::operator()(uint r, uint c)
{
  return array[c + r * 2];
}

real Matrix2::operator()(uint r, uint c) const
{
  return array[c + r * 2];
}

void Matrix2::operator*=(real rhs)
{
  Matrix2& self = *this;
  self[0] *= rhs;
  self[1] *= rhs;
}

void Matrix2::operator/=(real rhs)
{
  Matrix2& self = *this;
  self[0] /= rhs;
  self[1] /= rhs;
}

Matrix2 Matrix2::operator*(real rhs) const
{
  Matrix2 ret = *this;
  ret *= rhs;
  return ret;
}

Matrix2 Matrix2::operator/(real rhs) const
{
  Matrix2 ret = *this;
  ret /= rhs;
  return ret;
}

void Matrix2::operator+=(Mat2Param rhs)
{
  Matrix2& self = *this;
  self[0] += rhs[0];
  self[1] += rhs[1];
}

void Matrix2::operator-=(Mat2Param rhs)
{
  Matrix2& self = *this;
  self[0] -= rhs[0];
  self[1] -= rhs[1];
}

Matrix2 Matrix2::operator+(Mat2Param rhs) const
{
  Matrix2 ret = *this;
  ret += rhs;
  return ret;
}

Matrix2 Matrix2::operator-(Mat2Param rhs) const
{
  Matrix2 ret = *this;
  ret -= rhs;
  return ret;
}

Matrix2 Matrix2::operator*(Mat2Param rhs) const
{
  return Matrix2::Multiply(*this, rhs);
}

bool Matrix2::operator==(Mat2Param rhs) const
{
  Mat2Param self = *this;
  return self[0] == rhs[0] &&
         self[1] == rhs[1];
}

bool Matrix2::operator!=(Mat2Param rhs) const
{
  return !(*this == rhs);
}

Mat2Ref Matrix2::ZeroOut()
{
  Mat2Ref self = *this;
  self[0].ZeroOut();
  self[1].ZeroOut();
  return *this;
}

Mat2Ref Matrix2::SetIdentity()
{
  Mat2Ref self = *this;
  self[0].Set(real(1.0), real(0.0));
  self[1].Set(real(0.0), real(1.0));
  return *this;
}

Vector2 Matrix2::GetBasis(uint index) const
{
  //Could be optimized but I don't want to rely on the order of the elements in 
  //the matrix
  switch(index)
  {
  case 0:
    return Vector2(m00, m10);
  case 1:
    return Vector2(m01, m11);
  default:
    //ErrorIf(index > 2, "Invalid index given, matrix dimension is too low.");
    return Vector2(real(0.0), real(0.0));
  }
}

void Matrix2::SetBasis(uint index, Vec2Param basis)
{
  //Could be optimized but I don't want to rely on the order of the elements in 
  //the matrix
  switch(index)
  {
  case 0:
    m00 = basis.x;
    m10 = basis.y;
    break;
  case 1:
    m01 = basis.x;
    m11 = basis.y;
    break;
  }
}

Vector2 Matrix2::GetCross(uint index) const
{
  switch(index)
  {
  case 0:
    return Vector2(m00, m01);
  case 1:
    return Vector2(m10, m11);
  default:
    //ErrorIf(index > 2, "Invalid index given, matrix dimension is too low.");
    return Vector2(real(0.0), real(0.0));
  }
}

void Matrix2::SetCross(uint index, Vec2Param cross)
{
  switch(index)
  {
  case 0:
    m00 = cross.x;
    m01 = cross.y;
    break;
  case 1:
    m10 = cross.x;
    m11 = cross.y;
    break;
  }
}

bool Matrix2::Valid() const
{
  Mat2Param self = *this;
  return self[0].Valid() && self[1].Valid();
}

real Matrix2::Determinant() const
{
  return m00 * m11 - m01 * m10;
}

void Matrix2::InvertInternal(real invDeterminant)
{
  float t00 = m11;
  float t01 = -m01;
  float t10 = -m10;
  float t11 = m00;

  m00 = t00 * invDeterminant;
  m01 = t01 * invDeterminant;
  m10 = t10 * invDeterminant;
  m11 = t11 * invDeterminant;
}

void Matrix2::Transpose(Mat2Ref mat)
{
  Math::Swap(mat.m01, mat.m10);
}

Matrix2 Matrix2::Transposed(Mat2Param mat)
{
  Matrix2 ret = mat;
  Matrix2::Transpose(ret);
  return ret;
}

real Matrix2::Determinant(Mat2Param mat)
{
  return mat.Determinant();
}

void Matrix2::Invert(Mat2Ref mat)
{
  real determinant = Matrix2::Determinant(mat);
  real invDeterminant = real(1.0) / determinant;
  return mat.InvertInternal(invDeterminant);
}

Matrix2 Matrix2::Inverted(Mat2Param mat)
{
  Matrix2 ret = mat;
  Matrix2::Invert(ret);
  return ret;
}

bool Matrix2::SafeInvert(Mat2Ref mat)
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

Matrix2 Matrix2::Multiply(Mat2Param lhs, Mat2Param rhs)
{
  Matrix2 ret;

  Vector2 cross0 = lhs.GetCross(0);
  Vector2 cross1 = lhs.GetCross(1);

  ret.m00 = Dot(cross0, Vector2(rhs.m00, rhs.m10));
  ret.m01 = Dot(cross0, Vector2(rhs.m01, rhs.m11));

  ret.m11 = Dot(cross1, Vector2(rhs.m01, rhs.m11));
  ret.m10 = Dot(cross1, Vector2(rhs.m00, rhs.m10));

  return ret;
}

Vector2 Matrix2::Multiply(Mat2Param lhs, Vec2Param rhs)
{
  real x = Dot(lhs.GetCross(0), rhs);
  real y = Dot(lhs.GetCross(1), rhs);
  return Vector2(x, y);
}

Matrix2 Matrix2::GenerateScale(Vec2Param scale)
{
  Matrix2 result;
  result.m00 = scale.x;
  result.m01 = 0;
  result.m10 = 0;
  result.m11 = scale.y;
  return result;
}

Matrix2 Matrix2::GenerateRotation(real radians)
{
  real cosTheta = Math::Cos(radians);
  real sinTheta = Math::Sin(radians);
  Matrix2 result;
  result.m00 = cosTheta;
  result.m01 = -sinTheta;
  result.m10 = sinTheta;
  result.m11 = cosTheta;
  return result;
}

Matrix2 Matrix2::GenerateTransform(real radians, Vec2Param scale)
{
  Matrix2 result = Matrix2::GenerateRotation(radians);
  result.m00 *= scale.x;
  result.m10 *= scale.x;
  result.m01 *= scale.y;
  result.m11 *= scale.y;
  return result;
}

void Matrix2::Decompose(Mat2Param transform, real& radians, Vec2Ref scale)
{

}

//-------------------------------------------------------------------Legacy
Matrix2 Matrix2::Transposed() const
{
  return Matrix2::Transposed(*this);
}

Matrix2 Matrix2::Inverted()
{
  return Matrix2::Inverted(*this);
}

void Matrix2::Rotate(real radians)
{
  *this = Matrix2::GenerateRotation(radians);
}

void Matrix2::Scale(real x, real y)
{
  *this = Matrix2::GenerateScale(Vector2(x, y));
}

void Matrix2::Scale(Vec2Param rhs)
{
  *this = Matrix2::GenerateScale(rhs);
}

Vector2 Matrix2::Transform(Vec2Param vector) const
{
  return Matrix2::Multiply(*this, vector);
}

//-------------------------------------------------------------------Globals
Matrix2 operator*(real lhs, Mat2Param rhs)
{
  return rhs * lhs;
}

Matrix2 Multiply(Mat2Param lhs, Mat2Param rhs)
{
  return Matrix2::Multiply(lhs, rhs);
}

Vector2 Multiply(Mat2Param lhs, Vec2Param rhs)
{
  return Matrix2::Multiply(lhs, rhs);
}

}// namespace Math
