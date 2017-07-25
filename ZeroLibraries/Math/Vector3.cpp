///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{

const Vector3 Vector3::cZero(real(0.0), real(0.0), real(0.0));
const Vector3 Vector3::cXAxis(real(1.0), real(0.0), real(0.0));
const Vector3 Vector3::cYAxis(real(0.0), real(1.0), real(0.0));
const Vector3 Vector3::cZAxis(real(0.0), real(0.0), real(1.0));
const Vector3 Vector3::Axes[] = {Vector3::cXAxis, Vector3::cYAxis, Vector3::cZAxis};

Vector3::Vector3(real xx, real yy, real zz)
{
  x = xx;
  y = yy;
  z = zz;
}

Vector3::Vector3(real xyz)
{
  x = xyz;
  y = xyz;
  z = xyz;
}

Vector3::Vector3(Vec2Param rhs, real zz)
{
  x = rhs.x;
  y = rhs.y;
  z = zz;
}

Vector3::Vector3(ConstRealPointer data)
{
  array[0] = data[0];
  array[1] = data[1];
  array[2] = data[2];
}

real& Vector3::operator[](uint index)
{
  ErrorIf(index > 2, "Math::Vector3 - Subscript out of range.");
  return array[index];
}

real Vector3::operator[](uint index) const
{
  ErrorIf(index > 2, "Math::Vector3 - Subscript out of range.");
  return array[index];
}

bool Vector3::operator==(Vec3Param rhs) const
{
  return Math::Equal(x, rhs.x) &&
         Math::Equal(y, rhs.y) &&
         Math::Equal(z, rhs.z);
}

bool Vector3::operator!=(Vec3Param rhs) const
{
  return !(*this == rhs);
}

BoolVec3 Vector3::operator<(Vec3Param rhs) const
{
  return BoolVec3(x < rhs.x,
                  y < rhs.y,
                  z < rhs.z);
}

BoolVec3 Vector3::operator<=(Vec3Param rhs) const
{
  return BoolVec3(x <= rhs.x,
                  y <= rhs.y,
                  z <= rhs.z);
}

BoolVec3 Vector3::operator>(Vec3Param rhs) const
{
  return BoolVec3(x > rhs.x,
                  y > rhs.y,
                  z > rhs.z);
}

BoolVec3 Vector3::operator>=(Vec3Param rhs) const
{
  return BoolVec3(x >= rhs.x,
                  y >= rhs.y,
                  z >= rhs.z);
}

void Vector3::ZeroOut()
{
  array[0] = real(0.0);
  array[1] = real(0.0);
  array[2] = real(0.0);
}

void Vector3::Set(real x_, real y_, real z_)
{
  x = x_;
  y = y_;
  z = z_;
}

void Vector3::Splat(real xyz)
{
  x = y = z = xyz;
}

Vector3 Vector3::Cross(Vec3Param lhs, Vec3Param rhs)
{
  Vector3 ret;
  ret.x = lhs.y * rhs.z - lhs.z * rhs.y;
  ret.y = lhs.z * rhs.x - lhs.x * rhs.z;
  ret.z = lhs.x * rhs.y - lhs.y * rhs.x;
  return ret;
}

real Vector3::Length(Vec3Param value)
{
  real sqLength = Vector3::LengthSq(value);
  return Math::Sqrt(sqLength);
}

real Vector3::LengthSq(Vec3Param value)
{
  return Vector3::Dot(value, value);
}

real Vector3::Distance(Vec3Param lhs, Vec3Param rhs)
{
  real sqDistance = Vector3::DistanceSq(lhs, rhs);
  return Math::Sqrt(sqDistance);
}

real Vector3::DistanceSq(Vec3Param lhs, Vec3Param rhs)
{
  return Vector3::LengthSq(lhs - rhs);
}

real Vector3::Normalize(Vec3Ref value)
{
  real length = Vector3::Length(value);
  value /= length;
  return length;
}

Vector3 Vector3::Normalized(Vec3Param value)
{
  Vector3 ret = value;
  ret /= Vector3::Length(value);
  return ret;
}

real Vector3::AttemptNormalize(Vec3Ref value)
{
  real lengthSq = Vector3::LengthSq(value);

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

Vector3 Vector3::AttemptNormalized(Vec3Param value)
{
  Vector3 result = value;
  Vector3::AttemptNormalize(result);
  return result;
}

Vector3 Vector3::MultiplyAdd(Vec3Param v0, Vec3Param v1, real scalar)
{
  return v0 + v1 * scalar;
}

Vector3 Vector3::MultiplySubtract(Vec3Param v0, Vec3Param v1, real scalar)
{
  return v0 - v1 * scalar;
}

Vector3 Vector3::Abs(Vec3Param value)
{
  return Vector3(Math::Abs(value.x), Math::Abs(value.y), Math::Abs(value.z));
}

Vector3 Vector3::Min(Vec3Param lhs, Vec3Param rhs)
{
  return Vector3(Math::Min(lhs.x, rhs.x),
                 Math::Min(lhs.y, rhs.y),
                 Math::Min(lhs.z, rhs.z));
}

Vector3 Vector3::Max(Vec3Param lhs, Vec3Param rhs)
{
  return Vector3(Math::Max(lhs.x, rhs.x),
                 Math::Max(lhs.y, rhs.y),
                 Math::Max(lhs.z, rhs.z));
}

Vector3 Vector3::Clamp(Vec3Param value, Vec3Param minValue, Vec3Param maxValue)
{
  Vector3 result;
  result.x = Math::Clamp(value.x, minValue.x, maxValue.x);
  result.y = Math::Clamp(value.y, minValue.y, maxValue.y);
  result.z = Math::Clamp(value.z, minValue.z, maxValue.z);
  return result;
}

Vector3 Vector3::DebugClamp(Vec3Param value, Vec3Param minValue, Vec3Param maxValue, bool& wasClamped)
{
  Vector3 result;
  result.x = Math::DebugClamp(value.x, minValue.x, maxValue.x, wasClamped);
  result.y = Math::DebugClamp(value.y, minValue.y, maxValue.y, wasClamped);
  result.z = Math::DebugClamp(value.z, minValue.z, maxValue.z, wasClamped);
  return result;
}

Vector3 Vector3::Floor(Vec3Param value)
{
  Vector3 result;
  result.x = Math::Floor(value.x);
  result.y = Math::Floor(value.y);
  result.z = Math::Floor(value.z);
  return result;
}

Vector3 Vector3::Ceil(Vec3Param value)
{
  Vector3 result;
  result.x = Math::Ceil(value.x);
  result.y = Math::Ceil(value.y);
  result.z = Math::Ceil(value.z);
  return result;
}

Vector3 Vector3::Truncate(Vec3Param value)
{
  Vector3 result;
  result.x = Math::Truncate(value.x);
  result.y = Math::Truncate(value.y);
  result.z = Math::Truncate(value.z);
  return result;
}

Vector3 Vector3::Round(Vec3Param value)
{
  Vector3 result;
  result.x = Math::Round(value.x);
  result.y = Math::Round(value.y);
  result.z = Math::Round(value.z);
  return result;
}

Vector3 Vector3::Lerp(Vec3Param start, Vec3Param end, real tValue)
{
  return Math::Lerp<Vector3, real>(start, end, tValue);
}

Vector3 Vector3::Slerp(Vec3Param start, Vec3Param end, real tValue)
{
  real dot = Dot(start, end);
  real theta = Math::ArcCos(dot) * tValue;
  Vector3 relativeVec = end - start * dot;
  relativeVec.Normalize();
  return (start * Math::Cos(theta)) + (relativeVec * Math::Sin(theta));
}

Vector3 Vector3::SafeSlerp(Vec3Param start, Vec3Param end, real tValue)
{
  Vec3 normalizedStart = start.AttemptNormalized();
  Vec3 normalizedEnd = end.AttemptNormalized();

  real dot = Dot(normalizedStart, normalizedEnd);
  // Safeguard for non-normalized and slight floating point errors
  dot = Math::Clamp(dot, real(-1.0), real(1.0));
  real theta = Math::ArcCos(dot) * tValue;

  Vector3 relativeVec;
  // If end is the negative of start, no direction is better to interpolate than
  // another, so generate a random perpendicular vector to rotate towards
  if(dot == -real(1.0))
  {
    // Unfortunately, a 3d perpendicular vector is not as simple, so try doing the 2d
    // perpendicular with [x,y], but if x is zero then switch to [y,z] instead
    if(normalizedStart.x != real(0.0))
      relativeVec = Vec3(-normalizedStart.y, normalizedStart.x, normalizedStart.z);
    else
      relativeVec = Vec3(normalizedStart.x, -normalizedStart.z, normalizedStart.y);
  }
  else
    relativeVec = normalizedEnd - normalizedStart * dot;
  // Attempt normalize (zero vectors and start == end)
  relativeVec.AttemptNormalize();
  return (normalizedStart * Math::Cos(theta)) + (relativeVec * Math::Sin(theta));
}

Vector3 Vector3::ProjectOnVector(Vec3Param input, Vec3Param normalizedVector)
{
  return GenericProjectOnVector(input, normalizedVector);
}

Vector3 Vector3::ProjectOnPlane(Vec3Param input, Vec3Param planeNormal)
{
  return GenericProjectOnPlane(input, planeNormal);
}

Vector3 Vector3::ReflectAcrossVector(Vec3Param input, Vec3Param normalizedVector)
{
  return GenericReflectAcrossVector(input, normalizedVector);
}

Vector3 Vector3::ReflectAcrossPlane(Vec3Param input, Vec3Param planeNormal)
{
  return GenericReflectAcrossPlane(input, planeNormal);
}

Vector3 Vector3::Refract(Vec3Param input, Vec3Param planeNormal, real refractionIndex)
{
  return GenericRefract(input, planeNormal, refractionIndex);
}

real Vector3::AngleBetween(Vec3Param a, Vec3Param b)
{
  real cosTheta = Dot(a, b) / (a.Length() * b.Length());
  cosTheta = Math::Clamp(cosTheta, real(-1.0), real(1.0));
  return Math::ArcCos(cosTheta);
}

bool Vector3::ApproximatelyEqual(Vec3Param lhs, Vec3Param rhs, real epsilon)
{
   return Math::Equal(lhs.x, rhs.x, epsilon) &&
          Math::Equal(lhs.y, rhs.y, epsilon) &&
          Math::Equal(lhs.z, rhs.z, epsilon);
}

bool Vector3::Valid() const
{
  return IsValid(x) && IsValid(y) && IsValid(z);
}

Vector3 Vector3::Cross(Vec3Param rhs) const
{
  return Vector3::Cross(*this, rhs);
}

real Vector3::Length() const
{
  return Vector3::Length(*this);
}

real Vector3::LengthSq() const
{
  return Vector3::LengthSq(*this);
}

real Vector3::Normalize()
{
  return Vector3::Normalize(*this);
}

Vector3 Vector3::Normalized() const
{
  return Vector3::Normalized(*this);
}

real Vector3::AttemptNormalize()
{
  return Vector3::AttemptNormalize(*this);
}

Vector3 Vector3::AttemptNormalized() const
{
  return Vector3::AttemptNormalized(*this);
}

void Vector3::Floor()
{
  *this = Vector3::Floor(*this);
}

void Vector3::Ceil()
{
  *this = Vector3::Ceil(*this);
}

void Vector3::Truncate(void)
{
  *this = Vector3::Truncate(*this);
}

void Vector3::Round(void)
{
  *this = Vector3::Round(*this);
}

Vector3 Vector3::ProjectOnVector(Vec3Param normalizedVector) const
{
  return Vector3::ProjectOnVector(*this, normalizedVector);
}

Vector3 Vector3::ProjectOnPlane(Vec3Param planeNormal) const
{
  return Vector3::ProjectOnPlane(*this, planeNormal);
}

Vector3 Vector3::ReflectAcrossVector(Vec3Param normalizedVector) const
{
  return Vector3::ReflectAcrossVector(*this, normalizedVector);
}

Vector3 Vector3::ReflectAcrossPlane(Vec3Param planeNormal) const
{
  return Vector3::ReflectAcrossPlane(*this, planeNormal);
}

Vector3 Vector3::Refract(Vec3Param planeNormal, real refractionIndex) const
{
  return Vector3::Refract(*this, planeNormal, refractionIndex);
}

Vec3Ref Vector3::Negate()
{
  (*this) *= real(-1.0);
  return *this;
}

//-------------------------------------------------------------------Global Functions
Vector3 operator*(real lhs, Vec3Param rhs)
{
  return rhs * lhs;
}

Vector3 Cross(Vec3Param lhs, Vec3Param rhs)
{
  return Vector3::Cross(lhs, rhs);
}

real Length(Vec3Param value)
{
  return Vector3::Length(value);
}

real LengthSq(Vec3Param value)
{
  return Vector3::LengthSq(value);
}

real Distance(Vec3Param lhs, Vec3Param rhs)
{
  return Vector3::Distance(lhs, rhs);
}

real DistanceSq(Vec3Param lhs, Vec3Param rhs)
{
  return Vector3::DistanceSq(lhs, rhs);
}

real Normalize(Vec3Ref value)
{
  return Vector3::Normalize(value);
}

Vector3 Normalized(Vec3Param value)
{
  return Vector3::Normalized(value);
}

real AttemptNormalize(Vec3Ref value)
{
  return Vector3::AttemptNormalize(value);
}

Vector3 AttemptNormalized(Vec3Param value)
{
  return Vector3::AttemptNormalized(value);
}

Vector3 MultiplyAdd(Vec3Param v0, Vec3Param v1, real scalar)
{
  return Vector3::MultiplyAdd(v0, v1, scalar);
}

Vector3 MultiplySubtract(Vec3Param v0, Vec3Param v1, real scalar)
{
  return Vector3::MultiplySubtract(v0, v1, scalar);
}

Vector3 Abs(Vec3Param value)
{
  return Vector3::Abs(value);
}

Vector3 Min(Vec3Param lhs, Vec3Param rhs)
{
  return Vector3::Min(lhs, rhs);
}

Vector3 Max(Vec3Param lhs, Vec3Param rhs)
{
  return Vector3::Max(lhs, rhs);
}

Vector3 Clamp(Vec3Param value, Vec3Param minValue, Vec3Param maxValue)
{
  return Vector3::Clamp(value, minValue, maxValue);
}

Vector3 DebugClamp(Vec3Param value, Vec3Param minValue, Vec3Param maxValue, bool& wasClamped)
{
  return Vector3::DebugClamp(value, minValue, maxValue, wasClamped);
}

Vector3 Floor(Vec3Param value)
{
  return Vector3::Floor(value);
}

Vector3 Ceil(Vec3Param value)
{
  return Vector3::Ceil(value);
}

Vector3 Truncate(Vec3Param value)
{
  return Vector3::Truncate(value);
}

Vector3 Round(Vec3Param value)
{
  return Vector3::Round(value);
}

Vector3 Lerp(Vec3Param start, Vec3Param end, real tValue)
{
  return Vector3::Lerp(start, end, tValue);
}

Vector3 Slerp(Vec3Param start, Vec3Param end, real tValue)
{
  return Vector3::Slerp(start, end, tValue);
}

Vector3 SafeSlerp(Vec3Param start, Vec3Param end, real tValue)
{
  return Vector3::SafeSlerp(start, end, tValue);
}

Vector3 ProjectOnVector(Vec3Param input, Vec3Param normalizedVector)
{
  return Vector3::ProjectOnVector(input, normalizedVector);
}

Vector3 ProjectOnPlane(Vec3Param input, Vec3Param planeNormal)
{
  return Vector3::ProjectOnPlane(input, planeNormal);
}

Vector3 ReflectAcrossVector(Vec3Param input, Vec3Param normalizedVector)
{
  return Vector3::ReflectAcrossVector(input, normalizedVector);
}

Vector3 ReflectAcrossPlane(Vec3Param input, Vec3Param planeNormal)
{
  return Vector3::ReflectAcrossPlane(input, planeNormal);
}

Vector3 Refract(Vec3Param input, Vec3Param planeNormal, real refractionIndex)
{
  return Vector3::Refract(input, planeNormal, refractionIndex);
}

real AngleBetween(Vec3Param a, Vec3Param b)
{
  return Vector3::AngleBetween(a, b);
}

//------------------------------------------------------------- Legacy
bool Equal(Vec3Param lhs, Vec3Param rhs, real epsilon)
{
  return Math::Equal(lhs.x, rhs.x, epsilon) &&
         Math::Equal(lhs.y, rhs.y, epsilon) &&
         Math::Equal(lhs.z, rhs.z, epsilon);
}

Vector3 Cross2d(Vec3Param lhs, Vec3Param rhs)
{
  Vector3 result = Vector3::cZero;
  result.z = lhs.x * rhs.y - rhs.x * lhs.y;
  return result;
}

void Negate(Vec3Ptr vec)
{
  ErrorIf(vec == nullptr, "Vector3 - Null pointer passed for vector.");
  *vec *= real(-1.0);
}

Vector3 Negated(Vec3Param vec)
{
  return Vector3(-vec.x, -vec.y, -vec.z);
}

void Clamp(Vec3Ptr vec, real min, real max)
{
  ErrorIf(vec == nullptr, "Vector3 - Null pointer passed for vector.");
  vec->x = Math::Clamp(vec->x, min, max);
  vec->y = Math::Clamp(vec->y, min, max);
  vec->z = Math::Clamp(vec->z, min, max);
}

Vector3 Clamped(Vec3Param vec, real min, real max)
{
  Vector3 results;
  results[0] = Math::Clamp(vec[0], min, max);
  results[1] = Math::Clamp(vec[1], min, max);
  results[2] = Math::Clamp(vec[2], min, max);
  return results;
}

bool AllLess(Vec3Param lhs, Vec3Param rhs)
{
  return (lhs.x < rhs.x && lhs.y < rhs.y && lhs.z < rhs.z);
}

bool AnyLess(Vec3Param lhs, Vec3Param rhs)
{
  return (lhs.x < rhs.x || lhs.y < rhs.y || lhs.z < rhs.z);
}

bool AllGreater(Vec3Param lhs, Vec3Param rhs)
{
  return (lhs.x > rhs.x && lhs.y > rhs.y && lhs.z > rhs.z);
}

bool AnyGreater(Vec3Param lhs, Vec3Param rhs)
{
  return (lhs.x > rhs.x || lhs.y > rhs.y || lhs.z > rhs.z);
}

real DistanceToLineSq(Vec3Param start, Vec3Param end, Vec3Param point)
{
  // Describes the line segment
  Vec3 d = end - start;
  Vec3 n = d.AttemptNormalized();
  // Vector from the point to the start of the line
  Vec3 ptoa = start - point;
  // Project the point onto the line segment d
  Vec3 proj = ptoa.Dot(n) * n;
  // Check to see if the projected point is before start on the line
  if (n.Dot(proj) > 0)
  {
    return ptoa.LengthSq();
  }
  // Check to see if the projected point is after end on the line
  if (proj.LengthSq() > d.LengthSq())
  {
    // Proj is passed the end so we subtract the length of d
    // since proj is relative to start
    return ptoa.LengthSq() - d.LengthSq();
  }
  // Otherwise the projected point is within the line segment d
  return (ptoa - proj).LengthSq();
}

}// namespace Math
