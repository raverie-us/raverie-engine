///////////////////////////////////////////////////////////////////////////////
///
///  \file IntVector4.cpp
///  Implementation of the IntVector4 structure.
/// 
///  Authors: Trevor Sundberg
///  Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{

const IntVector4 IntVector4::cZero(0, 0, 0, 0);
const IntVector4 IntVector4::cXAxis(1, 0, 0, 0);
const IntVector4 IntVector4::cYAxis(0, 1, 0, 0);
const IntVector4 IntVector4::cZAxis(0, 0, 1, 0);
const IntVector4 IntVector4::cWAxis(0, 0, 0, 1);


IntVector4::IntVector4(int x_, int y_, int z_, int w_)
{
  x = x_;
  y = y_;
  z = z_;
  w = w_;
}

int& IntVector4::operator[](uint index)
{
  ErrorIf(index > 3, "IntVector4 - Subscript out of range.");
  return array[index];
}

int IntVector4::operator[](uint index) const
{
  ErrorIf(index > 3, "IntVector4 - Subscript out of range.");
  return array[index];
}


////////// Unary Operators /////////////////////////////////////////////////////

IntVector4 IntVector4::operator-(void) const
{
  return IntVector4(-x, -y, -z, -w);
}


////////// Binary Assignment Operators (reals) /////////////////////////////////

void IntVector4::operator*=(int rhs)
{
  x *= rhs;
  y *= rhs;
  z *= rhs;
  w *= rhs;
}

void IntVector4::operator/=(int rhs)
{
  ErrorIf(rhs == 0, "IntVector4 - Division by zero.");
  x /= rhs;
  y /= rhs;
  z /= rhs;
  w /= rhs;
}


////////// Binary Operators (reals) ////////////////////////////////////////////

IntVector4 IntVector4::operator*(int rhs) const
{
  return IntVector4(x * rhs, y * rhs, z * rhs, w * rhs);
}

IntVector4 IntVector4::operator/(int rhs) const
{
  ErrorIf(rhs == 0, "IntVector4 - Division by zero.");
  return IntVector4(x / rhs, y / rhs, z / rhs, w / rhs);
}

IntVector4 IntVector4::operator%(int rhs) const
{
  ErrorIf(rhs == 0, "IntVector4 - Mod by zero.");
  return IntVector4(x % rhs, y % rhs, z % rhs, w % rhs);
}

IntVector4& IntVector4::operator++()
{
  ++x;
  ++y;
  ++z;
  ++w;
  return *this;
}

IntVector4& IntVector4::operator--()
{
  --x;
  --y;
  --z;
  --w;
  return *this;
}

////////// Binary Assignment Operators (Vectors) ///////////////////////////////

void IntVector4::operator+=(IntVec4Param rhs)
{
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;
  w += rhs.w;
}

void IntVector4::operator-=(IntVec4Param rhs)
{
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;
  w -= rhs.w;
}

void IntVector4::operator*=(IntVec4Param rhs)
{
  x *= rhs.x;
  y *= rhs.y;
  z *= rhs.z;
  w *= rhs.w;
}

void IntVector4::operator/=(IntVec4Param rhs)
{
  ErrorIf(rhs.x == 0 || rhs.y == 0 || rhs.z == 0 || rhs.w == 0, 
          "Vector2 - Division by zero.");
  x /= rhs.x;
  y /= rhs.y;
  z /= rhs.z;
  w /= rhs.w;
}


////////// Binary Operators (Vectors) //////////////////////////////////////////

IntVector4 IntVector4::operator+(IntVec4Param rhs) const
{
  return IntVector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
}

IntVector4 IntVector4::operator-(IntVec4Param rhs) const
{
  return IntVector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
}


////////// Binary Vector Comparisons ///////////////////////////////////////////

bool IntVector4::operator==(IntVec4Param rhs) const
{
  return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
}

bool IntVector4::operator!=(IntVec4Param rhs) const
{
  return !(*this == rhs);
}


BoolVec4 IntVector4::operator< (IntVec4Param rhs) const
{
  return BoolVec4(x < rhs.x,
                  y < rhs.y,
                  z < rhs.z,
                  w < rhs.w);
}

BoolVec4 IntVector4::operator<=(IntVec4Param rhs) const
{
  return BoolVec4(x <= rhs.x,
                  y <= rhs.y,
                  z <= rhs.z,
                  w <= rhs.w);
}

BoolVec4 IntVector4::operator> (IntVec4Param rhs) const
{
  return BoolVec4(x > rhs.x,
                  y > rhs.y,
                  z > rhs.z,
                  w > rhs.w);
}

BoolVec4 IntVector4::operator>=(IntVec4Param rhs) const
{
  return BoolVec4(x >= rhs.x,
                  y >= rhs.y,
                  z >= rhs.z,
                  w >= rhs.w);
}

void IntVector4::Set(int x_, int y_, int z_, int w_)
{
  x = x_;
  y = y_;
  z = z_;
  w = w_;
}

void IntVector4::ZeroOut(void)
{
  x = 0;
  y = 0;
  z = 0;
  w = 0;
}


IntVector4 IntVector4::operator/(IntVec4Param rhs) const
{
  ErrorIf(rhs.x == 0 || rhs.y == 0 || rhs.z == 0 || rhs.w == 0, 
          "IntVector4 - Division by zero.");
  return IntVector4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w);
}

IntVector4 IntVector4::operator*(IntVec4Param rhs) const
{
  return IntVector4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w);
}

IntVector4 IntVector4::operator%(IntVec4Param rhs) const
{
  ErrorIf(rhs.x == 0 || rhs.y == 0 || rhs.z == 0 || rhs.w == 0, 
          "IntVector4 - Mod by zero.");
  return IntVector4(x % rhs.x, y % rhs.y, z % rhs.z, w % rhs.w);
}

IntVector4 operator*(int lhs, IntVec4Param rhs)
{
  return rhs * lhs;
}

IntVector4 Abs(IntVec4Param vec)
{
  return IntVector4(Math::Abs(vec.x), Math::Abs(vec.y), Math::Abs(vec.z), Math::Abs(vec.w));
}

IntVector4 Min(IntVec4Param lhs, IntVec4Param rhs)
{
  return IntVector4(Math::Min(lhs.x, rhs.x),
                    Math::Min(lhs.y, rhs.y),
                    Math::Min(lhs.z, rhs.z),
                    Math::Min(lhs.w, rhs.w));
}

IntVector4 Max(IntVec4Param lhs, IntVec4Param rhs)
{
  return IntVector4(Math::Max(lhs.x, rhs.x),
                    Math::Max(lhs.y, rhs.y),
                    Math::Max(lhs.z, rhs.z),
                    Math::Max(lhs.w, rhs.w));
}

IntVector4  IntVector4::operator~() const
{
  return IntVector4(~x,
                    ~y,
                    ~z,
                    ~w);
}

IntVector4  IntVector4::operator<< (IntVec4Param rhs) const
{
  return IntVector4(x << rhs.x,
                    y << rhs.y,
                    z << rhs.z,
                    w << rhs.w);
}

IntVector4  IntVector4::operator>> (IntVec4Param rhs) const
{
  return IntVector4(x >> rhs.x,
                    y >> rhs.y,
                    z >> rhs.z,
                    w >> rhs.w);
}

IntVector4  IntVector4::operator|  (IntVec4Param rhs) const
{
  return IntVector4(x | rhs.x,
                    y | rhs.y,
                    z | rhs.z,
                    w | rhs.w);
}

IntVector4  IntVector4::operator^  (IntVec4Param rhs) const
{
  return IntVector4(x ^ rhs.x,
                    y ^ rhs.y,
                    z ^ rhs.z,
                    w ^ rhs.w);
}

IntVector4  IntVector4::operator&  (IntVec4Param rhs) const
{
  return IntVector4(x & rhs.x,
                    y & rhs.y,
                    z & rhs.z,
                    w & rhs.w);
}

IntVector4& IntVector4::operator<<=(IntVec4Param rhs)
{
  x <<= rhs.x;
  y <<= rhs.y;
  z <<= rhs.z;
  w <<= rhs.w;
  return *this;
}

IntVector4& IntVector4::operator>>=(IntVec4Param rhs)
{
  x >>= rhs.x;
  y >>= rhs.y;
  z >>= rhs.z;
  w >>= rhs.w;
  return *this;
}

IntVector4& IntVector4::operator|= (IntVec4Param rhs)
{
  x |= rhs.x;
  y |= rhs.y;
  z |= rhs.z;
  w |= rhs.w;
  return *this;
}

IntVector4& IntVector4::operator^= (IntVec4Param rhs)
{
  x ^= rhs.x;
  y ^= rhs.y;
  z ^= rhs.z;
  w ^= rhs.w;
  return *this;
}

IntVector4& IntVector4::operator&= (IntVec4Param rhs)
{
  x &= rhs.x;
  y &= rhs.y;
  z &= rhs.z;
  w &= rhs.w;
  return *this;
}

}// namespace Math
