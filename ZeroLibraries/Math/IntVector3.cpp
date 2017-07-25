///////////////////////////////////////////////////////////////////////////////
///
///  \file IntVector3.cpp
///  Implementation of the IntVector3 structure.
/// 
///  Authors: Trevor Sundberg
///  Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{

const IntVector3 IntVector3::cZero(0, 0, 0);
const IntVector3 IntVector3::cXAxis(1, 0, 0);
const IntVector3 IntVector3::cYAxis(0, 1, 0);
const IntVector3 IntVector3::cZAxis(0, 0, 1);


IntVector3::IntVector3(int x_, int y_, int z_)
{
  x = x_;
  y = y_;
  z = z_;
}

int& IntVector3::operator[](uint index)
{
  ErrorIf(index > 2, "IntVector3 - Subscript out of range.");
  return array[index];
}

int IntVector3::operator[](uint index) const
{
  ErrorIf(index > 2, "IntVector3 - Subscript out of range.");
  return array[index];
}


////////// Unary Operators /////////////////////////////////////////////////////

IntVector3 IntVector3::operator-(void) const
{
  return IntVector3(-x, -y, -z);
}


////////// Binary Assignment Operators (reals) /////////////////////////////////

void IntVector3::operator*=(int rhs)
{
  x *= rhs;
  y *= rhs;
  z *= rhs;
}

void IntVector3::operator/=(int rhs)
{
  ErrorIf(rhs == 0, "IntVector3 - Division by zero.");
  x /= rhs;
  y /= rhs;
  z /= rhs;
}


////////// Binary Operators (reals) ////////////////////////////////////////////

IntVector3 IntVector3::operator*(int rhs) const
{
  return IntVector3(x * rhs, y * rhs, z * rhs);
}

IntVector3 IntVector3::operator/(int rhs) const
{
  ErrorIf(rhs == 0, "IntVector3 - Division by zero.");
  return IntVector3(x / rhs, y / rhs, z / rhs);
}

IntVector3 IntVector3::operator%(int rhs) const
{
  ErrorIf(rhs == 0, "IntVector3 - Mod by zero.");
  return IntVector3(x % rhs, y % rhs, z % rhs);
}

IntVector3& IntVector3::operator++()
{
  ++x;
  ++y;
  ++z;
  return *this;
}

IntVector3& IntVector3::operator--()
{
  --x;
  --y;
  --z;
  return *this;
}

////////// Binary Assignment Operators (Vectors) ///////////////////////////////

void IntVector3::operator+=(IntVec3Param rhs)
{
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;
}

void IntVector3::operator-=(IntVec3Param rhs)
{
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;
}

void IntVector3::operator*=(IntVec3Param rhs)
{
  x *= rhs.x;
  y *= rhs.y;
  z *= rhs.z;
}

void IntVector3::operator/=(IntVec3Param rhs)
{
  ErrorIf(rhs.x == 0 || rhs.y == 0 || rhs.z == 0, 
          "Vector2 - Division by zero.");
  x /= rhs.x;
  y /= rhs.y;
  z /= rhs.z;
}


////////// Binary Operators (Vectors) //////////////////////////////////////////

IntVector3 IntVector3::operator+(IntVec3Param rhs) const
{
  return IntVector3(x + rhs.x, y + rhs.y, z + rhs.z);
}

IntVector3 IntVector3::operator-(IntVec3Param rhs) const
{
  return IntVector3(x - rhs.x, y - rhs.y, z - rhs.z);
}


////////// Binary Vector Comparisons ///////////////////////////////////////////

bool IntVector3::operator==(IntVec3Param rhs) const
{
  return x == rhs.x && y == rhs.y && z == rhs.z;
}

bool IntVector3::operator!=(IntVec3Param rhs) const
{
  return !(*this == rhs);
}


BoolVec3 IntVector3::operator< (IntVec3Param rhs) const
{
  return BoolVec3(x < rhs.x,
                  y < rhs.y,
                  z < rhs.z);
}

BoolVec3 IntVector3::operator<=(IntVec3Param rhs) const
{
  return BoolVec3(x <= rhs.x,
                  y <= rhs.y,
                  z <= rhs.z);
}

BoolVec3 IntVector3::operator> (IntVec3Param rhs) const
{
  return BoolVec3(x > rhs.x,
                  y > rhs.y,
                  z > rhs.z);
}

BoolVec3 IntVector3::operator>=(IntVec3Param rhs) const
{
  return BoolVec3(x >= rhs.x,
                  y >= rhs.y,
                  z >= rhs.z);
}

void IntVector3::Set(int x_, int y_, int z_)
{
  x = x_;
  y = y_;
  z = z_;
}

void IntVector3::ZeroOut(void)
{
  x = 0;
  y = 0;
  z = 0;
}


IntVector3 IntVector3::operator/(IntVec3Param rhs) const
{
  ErrorIf(rhs.x == 0 || rhs.y == 0 || rhs.z == 0, 
          "IntVector3 - Division by zero.");
  return IntVector3(x / rhs.x, y / rhs.y, z / rhs.z);
}

IntVector3 IntVector3::operator*(IntVec3Param rhs) const
{
  return IntVector3(x * rhs.x, y * rhs.y, z * rhs.z);
}

IntVector3 IntVector3::operator%(IntVec3Param rhs) const
{
  ErrorIf(rhs.x == 0 || rhs.y == 0 || rhs.z == 0, 
          "IntVector3 - Mod by zero.");
  return IntVector3(x % rhs.x, y % rhs.y, z % rhs.z);
}

IntVector3 operator*(int lhs, IntVec3Param rhs)
{
  return rhs * lhs;
}

IntVector3 Abs(IntVec3Param vec)
{
  return IntVector3(Math::Abs(vec.x), Math::Abs(vec.y), Math::Abs(vec.z));
}

IntVector3 Min(IntVec3Param lhs, IntVec3Param rhs)
{
  return IntVector3(Math::Min(lhs.x, rhs.x),
                    Math::Min(lhs.y, rhs.y),
                    Math::Min(lhs.z, rhs.z));
}

IntVector3 Max(IntVec3Param lhs, IntVec3Param rhs)
{
  return IntVector3(Math::Max(lhs.x, rhs.x),
                    Math::Max(lhs.y, rhs.y),
                    Math::Max(lhs.z, rhs.z));
}

IntVector3  IntVector3::operator~() const
{
  return IntVector3(~x,
                    ~y,
                    ~z);
}

IntVector3  IntVector3::operator<< (IntVec3Param rhs) const
{
  return IntVector3(x << rhs.x,
                    y << rhs.y,
                    z << rhs.z);
}

IntVector3  IntVector3::operator>> (IntVec3Param rhs) const
{
  return IntVector3(x >> rhs.x,
                    y >> rhs.y,
                    z >> rhs.z);
}

IntVector3  IntVector3::operator|  (IntVec3Param rhs) const
{
  return IntVector3(x | rhs.x,
                    y | rhs.y,
                    z | rhs.z);
}

IntVector3  IntVector3::operator^  (IntVec3Param rhs) const
{
  return IntVector3(x ^ rhs.x,
                    y ^ rhs.y,
                    z ^ rhs.z);
}

IntVector3  IntVector3::operator&  (IntVec3Param rhs) const
{
  return IntVector3(x & rhs.x,
                    y & rhs.y,
                    z & rhs.z);
}

IntVector3& IntVector3::operator<<=(IntVec3Param rhs)
{
  x <<= rhs.x;
  y <<= rhs.y;
  z <<= rhs.z;
  return *this;
}

IntVector3& IntVector3::operator>>=(IntVec3Param rhs)
{
  x >>= rhs.x;
  y >>= rhs.y;
  z >>= rhs.z;
  return *this;
}

IntVector3& IntVector3::operator|= (IntVec3Param rhs)
{
  x |= rhs.x;
  y |= rhs.y;
  z |= rhs.z;
  return *this;
}

IntVector3& IntVector3::operator^= (IntVec3Param rhs)
{
  x ^= rhs.x;
  y ^= rhs.y;
  z ^= rhs.z;
  return *this;
}

IntVector3& IntVector3::operator&= (IntVec3Param rhs)
{
  x &= rhs.x;
  y &= rhs.y;
  z &= rhs.z;
  return *this;
}

}// namespace Math
