///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{

const Vector4 Vector4::cZero(real(0.0), real(0.0), real(0.0), real(0.0));
const Vector4 Vector4::cXAxis(real(1.0), real(0.0), real(0.0), real(0.0));
const Vector4 Vector4::cYAxis(real(0.0), real(1.0), real(0.0), real(0.0));
const Vector4 Vector4::cZAxis(real(0.0), real(0.0), real(1.0), real(0.0));
const Vector4 Vector4::cWAxis(real(0.0), real(0.0), real(0.0), real(1.0));
const Vector4 Vector4::Axes[] = {Vector4::cXAxis, Vector4::cYAxis, Vector4::cZAxis, Vector4::cWAxis};


Vector4::Vector4(real x_, real y_, real z_, real w_)
{
  x = x_;
  y = y_;
  z = z_;
  w = w_;
}

Vector4::Vector4(real xyzw)
{
  x = y = z = w = xyzw;
}

Vector4::Vector4(ConstRealPointer data)
{
  array[0] = data[0];
  array[1] = data[1];
  array[2] = data[2];
  array[3] = data[3];
}

real& Vector4::operator[](uint index)
{
  ErrorIf(index > 3, "Math::Vector4 - Subscript out of range.");
  return array[index];
}

real Vector4::operator[](uint index) const
{
  ErrorIf(index > 3, "Math::Vector4 - Subscript out of range.");
  return array[index];
}

Vector4 Vector4::operator-(void) const
{
  return Vector4(-x, -y, -z, -w);
}

void Vector4::operator*=(real rhs)
{
  x *= rhs;
  y *= rhs;
  z *= rhs;
  w *= rhs;
}

void Vector4::operator/=(real rhs)
{
  ErrorIf(Math::IsZero(rhs), "Math::Vector4 - Division by zero.");
  x /= rhs;
  y /= rhs;
  z /= rhs;
  w /= rhs;
}

Vector4 Vector4::operator*(real rhs) const
{
  return Vector4(x * rhs, y * rhs, z * rhs, w * rhs);
}

Vector4 Vector4::operator/(real rhs) const
{
  ErrorIf(Math::IsZero(rhs), "Math::Vector4 - Division by zero.");
  return Vector4(x / rhs, y / rhs, z / rhs, w / rhs);
}

void Vector4::operator+=(Vec4Param rhs)
{
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;
  w += rhs.w;
}

void Vector4::operator-=(Vec4Param rhs)
{
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;
  w -= rhs.w;
}

void Vector4::operator*=(Vec4Param rhs)
{
  x *= rhs.x;
  y *= rhs.y;
  z *= rhs.z;
  w *= rhs.w;
}

void Vector4::operator/=(Vec4Param rhs)
{
  x /= rhs.x;
  y /= rhs.y;
  z /= rhs.z;
  w /= rhs.w;
}

Vector4 Vector4::operator+(Vec4Param rhs) const
{
  return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
}

Vector4 Vector4::operator-(Vec4Param rhs) const
{
  return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
}

Vector4 Vector4::operator*(Vec4Param rhs) const
{
  return Vector4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w);
}

Vector4 Vector4::operator/(Vec4Param rhs) const
{
  ErrorIf(rhs.x == real(0.0) || rhs.y == real(0.0) ||
    rhs.z == real(0.0) || rhs.w == real(0.0),
    "Vector4 - Division by Zero.");
  return Vector4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w);
}

bool Vector4::operator==(Vec4Param rhs) const
{
  return Equal(x, rhs.x) && 
         Equal(y, rhs.y) && 
         Equal(z, rhs.z) &&
         Equal(w, rhs.w);
}

bool Vector4::operator!=(Vec4Param rhs) const
{
  return !(*this == rhs);
}

BoolVec4 Vector4::operator< (Vec4Param rhs) const
{
  return BoolVec4(x < rhs.x,
                  y < rhs.y,
                  z < rhs.z,
                  w < rhs.w);
}

BoolVec4 Vector4::operator<=(Vec4Param rhs) const
{
  return BoolVec4(x <= rhs.x,
                  y <= rhs.y,
                  z <= rhs.z,
                  w <= rhs.w);
}

BoolVec4 Vector4::operator> (Vec4Param rhs) const
{
  return BoolVec4(x > rhs.x,
                  y > rhs.y,
                  z > rhs.z,
                  w > rhs.w);
}

BoolVec4 Vector4::operator>=(Vec4Param rhs) const
{
  return BoolVec4(x >= rhs.x,
                  y >= rhs.y,
                  z >= rhs.z,
                  w >= rhs.w);
}

void Vector4::ZeroOut()
{
  array[0] = real(0.0);
  array[1] = real(0.0);
  array[2] = real(0.0);
  array[3] = real(0.0);
}

void Vector4::Set(real x_, real y_, real z_, real w_)
{
  x = x_;
  y = y_;
  z = z_;
  w = w_;
}

void Vector4::Splat(real xyzw)
{
  x = y = z = w = xyzw;
}

real Vector4::Dot(Vec4Param lhs, Vec4Param rhs)
{
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

real Vector4::Length(Vec4Param value)
{
  real sqLength = Vector4::LengthSq(value);
  return Math::Sqrt(sqLength);
}

real Vector4::LengthSq(Vec4Param value)
{
  return Vector4::Dot(value, value);
}

real Vector4::Distance(Vec4Param lhs, Vec4Param rhs)
{
  real sqDistance = Vector4::DistanceSq(lhs, rhs);
  return Math::Sqrt(sqDistance);
}

real Vector4::DistanceSq(Vec4Param lhs, Vec4Param rhs)
{
  return Vector4::LengthSq(lhs - rhs);
}

real Vector4::Normalize(Vec4Ref value)
{
  real length = Vector4::Length(value);
  value /= length;
  return length;
}

Vector4 Vector4::Normalized(Vec4Param value)
{
  Vector4 ret = value;
  ret /= Vector4::Length(value);
  return ret;
}

real Vector4::AttemptNormalize(Vec4Ref value)
{
  real lengthSq = Vector4::LengthSq(value);
  if(Math::IsZero(lengthSq) == false)
  {
    lengthSq = Sqrt(lengthSq);
    value /= lengthSq;
  }
  return lengthSq;
}

Vector4 Vector4::AttemptNormalized(Vec4Param value)
{
  Vector4 result = value;
  Vector4::AttemptNormalize(result);
  return result;
}

Vector4 Vector4::MultiplyAdd(Vec4Param v0, Vec4Param v1, real scalar)
{
  return v0 + v1 * scalar;
}

Vector4 Vector4::MultiplySubtract(Vec4Param v0, Vec4Param v1, real scalar)
{
  return v0 - v1 * scalar;
}

Vector4 Vector4::Abs(Vec4Param value)
{
  return Vector4(Math::Abs(value.x), Math::Abs(value.y), 
                 Math::Abs(value.z), Math::Abs(value.w));
}

Vector4 Vector4::Min(Vec4Param lhs, Vec4Param rhs)
{
  return Vector4(Math::Min(lhs.x, rhs.x),
                 Math::Min(lhs.y, rhs.y),
                 Math::Min(lhs.z, rhs.z),
                 Math::Min(lhs.w, rhs.w));
}

Vector4 Vector4::Max(Vec4Param lhs, Vec4Param rhs)
{
  return Vector4(Math::Max(lhs.x, rhs.x),
                 Math::Max(lhs.y, rhs.y),
                 Math::Max(lhs.z, rhs.z),
                 Math::Max(lhs.w, rhs.w));
}

Vector4 Vector4::Clamp(Vec4Param value, Vec4Param minValue, Vec4Param maxValue)
{
  Vector4 result;
  result.x = Math::Clamp(value.x, minValue.x, maxValue.x);
  result.y = Math::Clamp(value.y, minValue.y, maxValue.y);
  result.z = Math::Clamp(value.z, minValue.z, maxValue.z);
  result.w = Math::Clamp(value.w, minValue.w, maxValue.w);
  return result;
}

Vector4 Vector4::DebugClamp(Vec4Param value, Vec4Param minValue, Vec4Param maxValue, bool& wasClamped)
{
  Vector4 result;
  result.x = Math::DebugClamp(value.x, minValue.x, maxValue.x, wasClamped);
  result.y = Math::DebugClamp(value.y, minValue.y, maxValue.y, wasClamped);
  result.z = Math::DebugClamp(value.z, minValue.z, maxValue.z, wasClamped);
  result.w = Math::DebugClamp(value.w, minValue.w, maxValue.w, wasClamped);
  return result;
}

Vector4 Vector4::Floor(Vec4Param value)
{
  Vector4 result;
  result.x = Math::Floor(value.x);
  result.y = Math::Floor(value.y);
  result.z = Math::Floor(value.z);
  result.w = Math::Floor(value.w);
  return result;
}

Vector4 Vector4::Ceil(Vec4Param value)
{
  Vector4 result;
  result.x = Math::Ceil(value.x);
  result.y = Math::Ceil(value.y);
  result.z = Math::Ceil(value.z);
  result.w = Math::Ceil(value.w);
  return result;
}

Vector4 Vector4::Truncate(Vec4Param value)
{
  Vector4 result;
  result.x = Math::Truncate(value.x);
  result.y = Math::Truncate(value.y);
  result.z = Math::Truncate(value.z);
  result.w = Math::Truncate(value.w);
  return result;
}

Vector4 Vector4::Round(Vec4Param value)
{
  Vector4 result;
  result.x = Math::Round(value.x);
  result.y = Math::Round(value.y);
  result.z = Math::Round(value.z);
  result.w = Math::Round(value.w);
  return result;
}

Vector4 Vector4::Lerp(Vec4Param start, Vec4Param end, real tValue)
{
  return Math::Lerp<Vector4, real>(start, end, tValue);
}

Vector4 Vector4::ProjectOnVector(Vec4Param input, Vec4Param normalizedVector)
{
  return GenericProjectOnVector(input, normalizedVector);
}

Vector4 Vector4::ProjectOnPlane(Vec4Param input, Vec4Param planeNormal)
{
  return GenericProjectOnPlane(input, planeNormal);
}

Vector4 Vector4::ReflectAcrossVector(Vec4Param input, Vec4Param normalizedVector)
{
  return GenericReflectAcrossVector(input, normalizedVector);
}

Vector4 Vector4::ReflectAcrossPlane(Vec4Param input, Vec4Param planeNormal)
{
  return GenericReflectAcrossPlane(input, planeNormal);
}

Vector4 Vector4::Refract(Vec4Param input, Vec4Param planeNormal, real refractionIndex)
{
  return GenericRefract(input, planeNormal, refractionIndex);
}

real Vector4::AngleBetween(Vec4Param a, Vec4Param b)
{
  real cosTheta = Dot(a, b) / (a.Length() * b.Length());
  cosTheta = Math::Clamp(cosTheta, real(-1.0), real(1.0));
  return Math::ArcCos(cosTheta);
}

bool Vector4::ApproximatelyEqual(Vec4Param lhs, Vec4Param rhs, real epsilon)
{
  return Math::Equal(lhs.x, rhs.x, epsilon) &&
         Math::Equal(lhs.y, rhs.y, epsilon) &&
         Math::Equal(lhs.z, rhs.z, epsilon) &&
         Math::Equal(lhs.w, rhs.w, epsilon);
}

bool Vector4::Valid() const
{
  return IsValid(x) && IsValid(y) && IsValid(z) && IsValid(w);
}

real Vector4::Dot(Vec4Param rhs) const
{
  return Vector4::Dot(*this, rhs);
}

real Vector4::Length() const
{
  return Vector4::Length(*this);
}

real Vector4::LengthSq() const
{
  return Vector4::LengthSq(*this);
}

real Vector4::Normalize()
{
  return Vector4::Normalize(*this);
}

Vector4 Vector4::Normalized() const
{
  return Vector4::Normalized(*this);
}

real Vector4::AttemptNormalize()
{
  return Vector4::AttemptNormalize(*this);
}

Vec4Ref Vector4::Negate()
{
  (*this) *= real(-1.0);
  return *this;
}

Vector4 Vector4::ProjectOnVector(Vec4Param normalizedVector) const
{
  return Vector4::ProjectOnVector(*this, normalizedVector);
}

Vector4 Vector4::ProjectOnPlane(Vec4Param planeNormal) const
{
  return Vector4::ProjectOnPlane(*this, planeNormal);
}

Vector4 Vector4::ReflectAcrossPlane(Vec4Param planeNormal) const
{
  return Vector4::ReflectAcrossPlane(*this, planeNormal);
}

Vector4 Vector4::ReflectAcrossVector(Vec4Param normalizedVector) const
{
  return Vector4::ReflectAcrossVector(*this, normalizedVector);
}

Vector4 Vector4::Refract(Vec4Param planeNormal, real refractionIndex) const
{
  return Vector4::Refract(*this, planeNormal, refractionIndex);
}

//-------------------------------------------------------------------Global functions
Vector4 operator*(real lhs, Vec4Param rhs)
{
  return rhs * lhs;
}

real Dot(Vec4Param lhs, Vec4Param rhs)
{
  return Vector4::Dot(lhs, rhs);
}

real Length(Vec4Param value)
{
  return Vector4::Length(value);
}

real LengthSq(Vec4Param value)
{
  return Vector4::LengthSq(value);
}

real Distance(Vec4Param lhs, Vec4Param rhs)
{
  return Vector4::Distance(lhs, rhs);
}

real DistanceSq(Vec4Param lhs, Vec4Param rhs)
{
  return Vector4::DistanceSq(lhs, rhs);
}

real Normalize(Vec4Ref value)
{
  return Vector4::Normalize(value);
}

Vector4 Normalized(Vec4Param value)
{
  return Vector4::Normalized(value);
}

real AttemptNormalize(Vec4Ref value)
{
  return Vector4::AttemptNormalize(value);
}

Vector4 AttemptNormalized(Vec4Param value)
{
  return Vector4::AttemptNormalized(value);
}

Vector4 MultiplyAdd(Vec4Param v0, Vec4Param v1, real scalar)
{
  return Vector4::MultiplyAdd(v0, v1, scalar);
}

Vector4 MultiplySubtract(Vec4Param v0, Vec4Param v1, real scalar)
{
  return Vector4::MultiplySubtract(v0, v1, scalar);
}

Vector4 Abs(Vec4Param value)
{
  return Vector4::Abs(value);
}

Vector4 Min(Vec4Param lhs, Vec4Param rhs)
{
  return Vector4::Min(lhs, rhs);
}

Vector4 Max(Vec4Param lhs, Vec4Param rhs)
{
  return Vector4::Max(lhs, rhs);
}

Vector4 Clamp(Vec4Param value, Vec4Param minValue, Vec4Param maxValue)
{
  return Vector4::Clamp(value, minValue, maxValue);
}

Vector4 DebugClamp(Vec4Param value, Vec4Param minValue, Vec4Param maxValue, bool& wasClamped)
{
  return Vector4::DebugClamp(value, minValue, maxValue, wasClamped);
}

Vector4 Floor(Vec4Param value)
{
  return Vector4::Floor(value);
}

Vector4 Ceil(Vec4Param value)
{
  return Vector4::Ceil(value);
}

Vector4 Truncate(Vec4Param value)
{
  return Vector4::Truncate(value);
}

Vector4 Round(Vec4Param value)
{
  return Vector4::Round(value);
}

Vector4 Lerp(Vec4Param start, Vec4Param end, real tValue)
{
  return Vector4::Lerp(start, end, tValue);
}

Vector4 ProjectOnVector(Vec4Param input, Vec4Param normalizedVector)
{
  return Vector4::ProjectOnVector(input, normalizedVector);
}

Vector4 ProjectOnPlane(Vec4Param input, Vec4Param planeNormal)
{
  return Vector4::ProjectOnPlane(input, planeNormal);
}

Vector4 ReflectAcrossVector(Vec4Param input, Vec4Param normalizedVector)
{
  return Vector4::ReflectAcrossVector(input, normalizedVector);
}

Vector4 ReflectAcrossPlane(Vec4Param input, Vec4Param planeNormal)
{
  return Vector4::ReflectAcrossPlane(input, planeNormal);
}

Vector4 Refract(Vec4Param input, Vec4Param planeNormal, real refractionIndex)
{
  return Vector4::Refract(input, planeNormal, refractionIndex);
}

real AngleBetween(Vec4Param a, Vec4Param b)
{
  return Vector4::AngleBetween(a, b);
}

//-------------------------------------------------------------------Legacy
void Negate(Vec4Ptr vec)
{
  ErrorIf(vec == nullptr, "Vector4 - Null pointer passed for vector.");
  *vec *= real(-1.0);
}

Vector4 Negated(Vec4Param vec)
{
  return Vector4(-vec.x, -vec.y, -vec.z, -vec.w);
}

Vector4 Clamped(Vec4Param vec, real min, real max)
{
  Vector4 results;
  results[0] = Math::Clamp(vec[0], min, max);
  results[1] = Math::Clamp(vec[1], min, max);
  results[2] = Math::Clamp(vec[2], min, max);
  results[3] = Math::Clamp(vec[3], min, max);
  return results;
}

}// namespace Math
