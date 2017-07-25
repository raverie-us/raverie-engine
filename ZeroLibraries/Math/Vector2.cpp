///////////////////////////////////////////////////////////////////////////////
///
///  Authors: Joshua Davis, Benjamin Strukus
///  Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{

const Vector2 Vector2::cZero(real(0.0), real(0.0));
const Vector2 Vector2::cXAxis(real(1.0), real(0.0));
const Vector2 Vector2::cYAxis(real(0.0), real(1.0));
const Vector2 Vector2::Axes[] = {Vector2::cXAxis, Vector2::cYAxis};


Vector2::Vector2(real x_, real y_)
{
  x = x_;
  y = y_;
}

Vector2::Vector2(real xy)
{
  x = xy;
  y = xy;
}

Vector2::Vector2(ConstRealPointer data)
{
  array[0] = data[0];
  array[1] = data[1];
}

real& Vector2::operator[](uint index)
{
  ErrorIf(index > 1, "Math::Vector2 - Subscript out of range.");
  return array[index];
}

real Vector2::operator[](uint index) const
{
  ErrorIf(index > 1, "Math::Vector2 - Subscript out of range.");
  return array[index];
}

Vector2 Vector2::operator-() const
{
  return Vector2(-x, -y);
}

void Vector2::operator*=(real rhs)
{
  x *= rhs;
  y *= rhs;
}

void Vector2::operator/=(real rhs)
{
  ErrorIf(Math::IsZero(rhs), "Math::Vector2 - Division by zero.");
  x /= rhs;
  y /= rhs;
}

Vector2 Vector2::operator*(real rhs) const
{
  return Vector2(x * rhs, y * rhs);
}

Vector2 Vector2::operator/(real rhs) const
{
  ErrorIf(Math::IsZero(rhs), "Math::Vector2 - Division by zero.");
  return Vector2(x / rhs, y / rhs);
}

void Vector2::operator+=(Vec2Param rhs)
{
  x += rhs.x;
  y += rhs.y;
}

void Vector2::operator-=(Vec2Param rhs)
{
  x -= rhs.x;
  y -= rhs.y;
}

void Vector2::operator*=(Vec2Param rhs)
{
  x *= rhs.x;
  y *= rhs.y;
}

void Vector2::operator/=(Vec2Param rhs)
{
  x /= rhs.x;
  y /= rhs.y;
}

Vector2 Vector2::operator+(Vec2Param rhs) const
{
  return Vector2(x + rhs.x, y + rhs.y);
}

Vector2 Vector2::operator-(Vec2Param rhs) const
{
  return Vector2(x - rhs.x, y - rhs.y);
}

Vector2 Vector2::operator*(Vec2Param rhs) const
{
  return Vector2(x * rhs.x, y * rhs.y);
}

Vector2 Vector2::operator/(Vec2Param rhs) const
{
  ErrorIf(rhs.x == real(0.0) || rhs.y == real(0.0),
    "Vector2 - Division by zero.");
  return Vector2(x / rhs.x, y / rhs.y);
}

bool Vector2::operator==(Vec2Param rhs) const
{
  return Equal(x, rhs.x) && Equal(y, rhs.y);
}

bool Vector2::operator!=(Vec2Param rhs) const
{
  return !(*this == rhs);
}

BoolVec2 Vector2::operator<(Vec2Param rhs) const
{
  return BoolVec2(x < rhs.x,
                  y < rhs.y);
}

BoolVec2 Vector2::operator<=(Vec2Param rhs) const
{
  return BoolVec2(x <= rhs.x,
                  y <= rhs.y);
}

BoolVec2 Vector2::operator>(Vec2Param rhs) const
{
  return BoolVec2(x > rhs.x,
                  y > rhs.y);
}

BoolVec2 Vector2::operator>=(Vec2Param rhs) const
{
  return BoolVec2(x >= rhs.x,
                  y >= rhs.y);
}

void Vector2::ZeroOut()
{
  array[0] = real(0.0);
  array[1] = real(0.0);
}

void Vector2::Set(real x_, real y_)
{
  x = x_;
  y = y_;
}

void Vector2::Splat(real value)
{
  x = y = value;
}

real Vector2::Dot(Vec2Param lhs, Vec2Param rhs)
{
  return lhs.x * rhs.x + lhs.y * rhs.y;
}

real Vector2::Cross(Vec2Param lhs, Vec2Param rhs)
{
  return lhs.x * rhs.y - rhs.x * lhs.y;
}

Vector2 Vector2::Cross(real lhs, Vec2Param rhs)
{
  return Vector2(-lhs * rhs.y, lhs * rhs.x);
}

Vector2 Vector2::Cross(Vec2Param lhs, real rhs)
{
  return Vector2(rhs * lhs.y, -rhs * lhs.x);
}

real Vector2::Length(Vec2Param value)
{
  real sqLength = LengthSq(value);
  return Math::Sqrt(sqLength);
}

real Vector2::LengthSq(Vec2Param value)
{
  return Vector2::Dot(value, value);
}

real Vector2::Distance(Vec2Param lhs, Vec2Param rhs)
{
  real sqDist = Vector2::DistanceSq(lhs, rhs);
  return Math::Sqrt(sqDist);
}

real Vector2::DistanceSq(Vec2Param lhs, Vec2Param rhs)
{
  return Vector2::LengthSq(lhs - rhs);
}

real Vector2::Normalize(Vec2Ref value)
{
  real length = Vector2::Length(value);
  value /= length;
  return length;
}

Vector2 Vector2::Normalized(Vec2Param value)
{
  Vector2 ret = value;
  Vector2::Normalize(ret);
  return ret;
}

real Vector2::AttemptNormalize(Vec2Ref value)
{
  real lengthSq = Vector2::LengthSq(value);

  // Although the squared length may not be zero, the sqrt of a small number
  // may be truncated to zero, causing a divide by zero crash.  This is why
  // we check to make sure that it is larger than our epsilon squared.
  if(lengthSq >= Epsilon() * Epsilon())
  {
    lengthSq = Sqrt(lengthSq);
    value /= lengthSq;
  }
  return lengthSq;
}

Vector2 Vector2::AttemptNormalized(Vec2Param value)
{
  Vector2 result = value;
  Vector2::AttemptNormalize(result);
  return result;
}

Vector2 Vector2::MultiplyAdd(Vec2Param v0, Vec2Param v1, real scalar)
{
  return v0 + v1 * scalar;
}

Vector2 Vector2::MultiplySubtract(Vec2Param v0, Vec2Param v1, real scalar)
{
  return v0 - v1 * scalar;
}

Vector2 Vector2::Abs(Vec2Param value)
{
  return Vector2(Math::Abs(value.x), Math::Abs(value.y));
}

Vector2 Vector2::Min(Vec2Param lhs, Vec2Param rhs)
{
  return Vector2(Math::Min(lhs.x, rhs.x),
                 Math::Min(lhs.y, rhs.y));
}

Vector2 Vector2::Max(Vec2Param lhs, Vec2Param rhs)
{
  return Vector2(Math::Max(lhs.x, rhs.x),
                 Math::Max(lhs.y, rhs.y));
}

Vector2 Vector2::Clamp(Vec2Param value, Vec2Param minValue, Vec2Param maxValue)
{
  Vector2 result = value;
  result.x = Math::Clamp(value.x, minValue.x, maxValue.x);
  result.y = Math::Clamp(value.y, minValue.y, maxValue.y);
  return result;
}

Vector2 Vector2::DebugClamp(Vec2Param value, Vec2Param minValue, Vec2Param maxValue, bool& wasClamped)
{
  Vector2 result = value;
  result.x = Math::DebugClamp(value.x, minValue.x, maxValue.x, wasClamped);
  result.y = Math::DebugClamp(value.y, minValue.y, maxValue.y, wasClamped);
  return result;
}

Vector2 Vector2::Floor(Vec2Param value)
{
  Vector2 result;
  result.x = Math::Floor(value.x);
  result.y = Math::Floor(value.y);
  return result;
}

Vector2 Vector2::Ceil(Vec2Param value)
{
  Vector2 result;
  result.x = Math::Ceil(value.x);
  result.y = Math::Ceil(value.y);
  return result;
}

Vector2 Vector2::Truncate(Vec2Param value)
{
  Vector2 result;
  result.x = Math::Truncate(value.x);
  result.y = Math::Truncate(value.y);
  return result;
}

Vector2 Vector2::Round(Vec2Param value)
{
  Vector2 result;
  result.x = Math::Round(value.x);
  result.y = Math::Round(value.y);
  return result;
}

Vector2 Vector2::Lerp(Vec2Param start, Vec2Param end, real tValue)
{
  WarnIf(!Math::InRange(tValue, real(0.0), real(1.0)),
    "Vector2 - Interpolation value is not in the range of [0, 1]");
  return Math::Lerp<Vector2, real>(start, end, tValue);
}

Vector2 Vector2::Slerp(Vec2Param start, Vec2Param end, real tValue)
{
  real dot = Math::Dot(start, end);
  real theta = Math::ArcCos(dot) * tValue;
  Vector2 relativeVec = end - start * dot;
  relativeVec.Normalize();
  return (start * Math::Cos(theta)) + (relativeVec * Math::Sin(theta));
}

Vector2 Vector2::SafeSlerp(Vec2Param start, Vec2Param end, real t)
{
  real dot = Math::Dot(start, end);
  real theta = Math::ArcCos(dot) * t;

  Vector2 relativeVec;
  //if end is the negative of start, no direction is better to interpolate than
  //another, so generate a random perpendicular vector to rotate towards
  if(dot == -real(1.0))
    relativeVec = Vec2(-start.y, start.x);
  else
    relativeVec = end - start * dot;
  //attempt normalize (zero vectors and start == end)
  relativeVec.AttemptNormalize();
  return (start * Math::Cos(theta)) + (relativeVec * Math::Sin(theta));
}

Vector2 Vector2::ProjectOnVector(Vec2Param input, Vec2Param normalizedVector)
{
  return GenericProjectOnVector(input, normalizedVector);
}

Vector2 Vector2::ProjectOnPlane(Vec2Param input, Vec2Param planeNormal)
{
  return GenericProjectOnPlane(input, planeNormal);
}

Vector2 Vector2::ReflectAcrossVector(Vec2Param input, Vec2Param normalizedVector)
{
  return GenericReflectAcrossVector(input, normalizedVector);
}

Vector2 Vector2::ReflectAcrossPlane(Vec2Param input, Vec2Param planeNormal)
{
  return GenericReflectAcrossPlane(input, planeNormal);
}

Vector2 Vector2::Refract(Vec2Param input, Vec2Param planeNormal, real refractionIndex)
{
  return GenericRefract(input, planeNormal, refractionIndex);
}

real Vector2::AngleBetween(Vec2Param a, Vec2Param b)
{
  real cosTheta = Dot(a, b) / (a.Length() * b.Length());
  cosTheta = Math::Clamp(cosTheta, real(-1.0), real(1.0));
  return Math::ArcCos(cosTheta);
}

bool Vector2::ApproximatelyEqual(Vec2Param lhs, Vec2Param rhs, real epsilon)
{
  return Math::Equal(lhs.x, rhs.x, epsilon) &&
         Math::Equal(lhs.y, rhs.y, epsilon);
}

bool Vector2::Valid() const
{
  return IsValid(x) && IsValid(y);
}

real Vector2::Dot(Vec2Param rhs) const
{
  return x * rhs.x + y * rhs.y;
}

real Vector2::Length() const
{
  return Sqrt(LengthSq());
}

real Vector2::LengthSq() const
{
  return Dot(*this);
}

real Vector2::Distance(Vec2Param rhs) const
{
  return Sqrt(DistanceSq(rhs));
}

real Vector2::DistanceSq(Vec2Param rhs) const
{
  return Dot(*this - rhs);
}

real Vector2::Normalize()
{
  real length = Length();
  *this /= length;
  return length;
}

Vector2 Vector2::Normalized() const
{
  Vector2 ret = *this;
  ret /= Length();
  return ret;
}

real Vector2::AttemptNormalize()
{
  real lengthSq = LengthSq();

  // Although the squared length may not be zero, the sqrt of a small number
  // may be truncated to zero, causing a divide by zero crash.  This is why
  // we check to make sure that it is larger than our epsilon squared.
  if(lengthSq >= Epsilon() * Epsilon())
  {
    lengthSq = Sqrt(lengthSq);
    *this /= lengthSq;
  }
  return lengthSq;
}

Vector2 Vector2::AttemptNormalized() const
{
  Vector2 result = *this;
  result.AttemptNormalize();
  return result;
}

Vector2 Vector2::ProjectOnVector(Vec2Param normalizedVector) const
{
  return GenericProjectOnVector(*this, normalizedVector);
}

Vector2 Vector2::ProjectOnPlane(Vec2Param planeNormal) const
{
  return GenericProjectOnPlane(*this, planeNormal);
}

Vector2 Vector2::ReflectAcrossVector(Vec2Param normalizedVector) const
{
  return GenericReflectAcrossVector(*this, normalizedVector);
}

Vector2 Vector2::ReflectAcrossPlane(Vec2Param planeNormal) const
{
  return GenericReflectAcrossPlane(*this, planeNormal);
}

Vector2 Vector2::Refract(Vec2Param planeNormal, real refractionIndex) const
{
  return GenericRefract(*this, planeNormal, refractionIndex);
}

Vec2Ref Vector2::Negate(void)
{
  (*this) *= real(-1.0);
  return *this;
}

//-------------------------------------------------------------------Globals
Vector2 operator*(real lhs, Vec2Param rhs)
{
  return rhs * lhs;
}

real Dot(Vec2Param lhs, Vec2Param rhs)
{
  return Vector2::Dot(lhs, rhs);
}

real Cross(Vec2Param lhs, Vec2Param rhs)
{
  return Vector2::Cross(lhs, rhs);
}

Vector2 Cross(real lhs, Vec2Param rhs)
{
  return Vector2::Cross(lhs, rhs);
}

Vector2 Cross(Vec2Param lhs, real rhs)
{
  return Vector2::Cross(lhs, rhs);
}

real Length(Vec2Param value)
{
  return Vector2::Length(value);
}

real LengthSq(Vec2Param value)
{
  return Vector2::LengthSq(value);
}

real Distance(Vec2Param lhs, Vec2Param rhs)
{
  return Vector2::Distance(lhs, rhs);
}

real DistanceSq(Vec2Param lhs, Vec2Param rhs)
{
  return Vector2::DistanceSq(lhs, rhs);
}

real Normalize(Vec2Ref value)
{
  return Vector2::Normalize(value);
}

Vector2 Normalized(Vec2Param value)
{
  return Vector2::Normalized(value);
}

real AttemptNormalize(Vec2Ref value)
{
  return Vector2::AttemptNormalize(value);
}

Vector2 AttemptNormalized(Vec2Param value)
{
  return Vector2::AttemptNormalized(value);
}

Vector2 MultiplyAdd(Vec2Param v0, Vec2Param v1, real scalar)
{
  return Vector2::MultiplyAdd(v0, v1, scalar);
}

Vector2 MultiplySubtract(Vec2Param v0, Vec2Param v1, real scalar)
{
  return Vector2::MultiplySubtract(v0, v1, scalar);
}

Vector2 Abs(Vec2Param value)
{
  return Vector2::Abs(value);
}

Vector2 Min(Vec2Param lhs, Vec2Param rhs)
{
  return Vector2::Min(lhs, rhs);
}

Vector2 Max(Vec2Param lhs, Vec2Param rhs)
{
  return Vector2::Max(lhs, rhs);
}

Vector2 Clamp(Vec2Param value, Vec2Param minValue, Vec2Param maxValue)
{
  return Vector2::Clamp(value, minValue, maxValue);
}

Vector2 DebugClamp(Vec2Param value, Vec2Param minValue, Vec2Param maxValue, bool& wasClamped)
{
  return Vector2::DebugClamp(value, minValue, maxValue, wasClamped);
}

Vector2 Floor(Vec2Param value)
{
  return Vector2::Floor(value);
}

Vector2 Ceil(Vec2Param value)
{
  return Vector2::Ceil(value);
}

Vector2 Truncate(Vec2Param value)
{
  return Vector2::Truncate(value);
}

Vector2 Round(Vec2Param value)
{
  return Vector2::Round(value);
}

Vector2 Lerp(Vec2Param start, Vec2Param end, real tValue)
{
  return Vector2::Lerp(start, end, tValue);
}

Vector2 Slerp(Vec2Param start, Vec2Param end, real tValue)
{
  return Vector2::Slerp(start, end, tValue);
}

Vector2 SafeSlerp(Vec2Param start, Vec2Param end, real tValue)
{
  return Vector2::SafeSlerp(start, end, tValue);
}

Vector2 ProjectOnVector(Vec2Param input, Vec2Param normalizedVector)
{
  return Vector2::ProjectOnVector(input, normalizedVector);
}

Vector2 ProjectOnPlane(Vec2Param input, Vec2Param planeNormal)
{
  return Vector2::ProjectOnPlane(input, planeNormal);
}

Vector2 ReflectAcrossVector(Vec2Param input, Vec2Param normalizedVector)
{
  return Vector2::ReflectAcrossVector(input, normalizedVector);
}

Vector2 ReflectAcrossPlane(Vec2Param input, Vec2Param planeNormal)
{
  return Vector2::ReflectAcrossPlane(input, planeNormal);
}

Vector2 Refract(Vec2Param input, Vec2Param planeNormal, real refractionIndex)
{
  return Vector2::Refract(input, planeNormal, refractionIndex);
}

real AngleBetween(Vec2Param a, Vec2Param b)
{
  return Vector2::AngleBetween(a, b);
}

//-------------------------------------------------------------------Legacy
void Clamp(Vec2Ptr vec, real min, real max)
{
  ErrorIf(vec == nullptr, "Null pointer passed into function.");
  vec->x = Math::Clamp(vec->x, min, max);
  vec->y = Math::Clamp(vec->y, min, max);
}

void Negate(Vec2Ptr vec)
{
  ErrorIf(vec == nullptr, "Vector2 - Null pointer passed for vector.");
  *vec *= real(-1.0);
}

Vector2 Negated(Vec2Param vec)
{
  return Vector2(-vec.x, -vec.y);
}

}// namespace Math
