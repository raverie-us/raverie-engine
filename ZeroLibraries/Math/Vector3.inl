///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Diagnostic/Diagnostic.hpp"

namespace Math
{

inline Vector3 Vector3::operator-() const
{
  return Vector3(-x, -y, -z);
}

inline void Vector3::operator*=(real rhs)
{
  x *= rhs;
  y *= rhs;
  z *= rhs;
}

inline void Vector3::operator/=(real rhs)
{
  ErrorIf(rhs == real(0.0), "Math::Vector3 - Division by zero.");
  x /= rhs;
  y /= rhs;
  z /= rhs;
}

inline Vector3 Vector3::operator*(real rhs) const
{
  Vector3 ret = *this;
  ret *= rhs;
  return ret;
}

inline Vector3 Vector3::operator/(real rhs) const
{
  ErrorIf(Math::IsZero(rhs), "Math::Vector3 - Division by zero.");
  Vector3 ret = *this;
  ret /= rhs;
  return ret;
}

inline void Vector3::operator+=(Vec3Param rhs)
{
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;
}

inline void Vector3::operator-=(Vec3Param rhs)
{
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;
}

inline void Vector3::operator*=(Vec3Param rhs)
{
  x *= rhs.x;
  y *= rhs.y;
  z *= rhs.z;
}

inline void Vector3::operator/=(Vec3Param rhs)
{
  x /= rhs.x;
  y /= rhs.y;
  z /= rhs.z;
}

inline Vector3 Vector3::operator+(Vec3Param rhs) const
{
  Vector3 ret = *this;
  ret += rhs;
  return ret;
}

inline Vector3 Vector3::operator-(Vec3Param rhs) const
{
  Vector3 ret = *this;
  ret -= rhs;
  return ret;
}

inline Vector3 Vector3::operator*(Vec3Param rhs) const
{
  return Vector3(x * rhs.x, y * rhs.y, z * rhs.z);
}

inline Vector3 Vector3::operator/(Vec3Param rhs) const
{  
  ErrorIf(rhs.x == real(0.0) || rhs.y == real(0.0) || rhs.z == real(0.0),
          "Vector3 - Division by zero.");
  return Vector3(x / rhs.x, y / rhs.y, z / rhs.z);
}

inline real Vector3::Dot(Vec3Param lhs, Vec3Param rhs)
{
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

inline real Vector3::Dot(Vec3Param rhs) const
{
  return x * rhs.x + y * rhs.y + z * rhs.z;
}

inline real Dot(Vec3Param lhs, Vec3Param rhs)
{  
  return lhs.Dot(rhs);
}

}//namespace Math
