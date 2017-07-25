///////////////////////////////////////////////////////////////////////////////
///
///  \file IntVector2.cpp
///  Implementation of the IntVector2 structure.
/// 
///  Authors: Trevor Sundberg
///  Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{

const IntVector2 IntVector2::cZero(0, 0);
const IntVector2 IntVector2::cXAxis(1, 0);
const IntVector2 IntVector2::cYAxis(0, 1);


IntVector2::IntVector2(int x_, int y_)
{
  x = x_;
  y = y_;
}

int& IntVector2::operator[](uint index)
{
  ErrorIf(index > 1, "Math::IntVector2 - Subscript out of range.");
  return array[index];
}

int IntVector2::operator[](uint index) const
{
  ErrorIf(index > 1, "Math::IntVector2 - Subscript out of range.");
  return array[index];
}


////////// Unary Operators /////////////////////////////////////////////////////

IntVector2 IntVector2::operator-(void) const
{
  return IntVector2(-x, -y);
}


////////// Binary Assignment Operators (reals) /////////////////////////////////

void IntVector2::operator*=(int rhs)
{
  x *= rhs;
  y *= rhs;
}

void IntVector2::operator/=(int rhs)
{
  ErrorIf(rhs == 0, "Math::IntVector2 - Division by zero.");
  x /= rhs;
  y /= rhs;
}


////////// Binary Operators (reals) ////////////////////////////////////////////

IntVector2 IntVector2::operator*(int rhs) const
{
  return IntVector2(x * rhs, y * rhs);
}

IntVector2 IntVector2::operator/(int rhs) const
{
  ErrorIf(rhs == 0, "Math::IntVector2 - Division by zero.");
  return IntVector2(x / rhs, y / rhs);
}


IntVector2 IntVector2::operator%(int rhs) const
{
  ErrorIf(rhs == 0, "Math::IntVector2 - Mod by zero.");
  return IntVector2(x % rhs, y % rhs);
}

IntVector2& IntVector2::operator++()
{
  ++x;
  ++y;
  return *this;
}

IntVector2& IntVector2::operator--()
{
  --x;
  --y;
  return *this;
}


////////// Binary Assignment Operators (Vectors) ///////////////////////////////

void IntVector2::operator+=(IntVec2Param rhs)
{
  x += rhs.x;
  y += rhs.y;
}

void IntVector2::operator-=(IntVec2Param rhs)
{
  x -= rhs.x;
  y -= rhs.y;
}

void IntVector2::operator*=(IntVec2Param rhs)
{
  x *= rhs.x;
  y *= rhs.y;
}

void IntVector2::operator/=(IntVec2Param rhs)
{
  ErrorIf(rhs.x == 0 || rhs.y == 0, 
          "IntVector2 - Division by zero.");
  x /= rhs.x;
  y /= rhs.y;
}


////////// Binary Operators (Vectors) //////////////////////////////////////////

IntVector2 IntVector2::operator+(IntVec2Param rhs) const
{
  return IntVector2(x + rhs.x, y + rhs.y);
}

IntVector2 IntVector2::operator-(IntVec2Param rhs) const
{
  return IntVector2(x - rhs.x, y - rhs.y);
}


////////// Binary Vector Comparisons ///////////////////////////////////////////

bool IntVector2::operator==(IntVec2Param rhs) const
{
  return x == rhs.x && y == rhs.y;
}

bool IntVector2::operator!=(IntVec2Param rhs) const
{
  return !(*this == rhs);
}

BoolVec2 IntVector2::operator< (IntVec2Param rhs) const
{
  return BoolVec2(x < rhs.x,
                  y < rhs.y);
}

BoolVec2 IntVector2::operator<=(IntVec2Param rhs) const
{
  return BoolVec2(x <= rhs.x,
                  y <= rhs.y);
}

BoolVec2 IntVector2::operator> (IntVec2Param rhs) const
{
  return BoolVec2(x > rhs.x,
                  y > rhs.y);
}

BoolVec2 IntVector2::operator>=(IntVec2Param rhs) const
{
  return BoolVec2(x >= rhs.x,
                  y >= rhs.y);
}


void IntVector2::Set(int x_, int y_)
{
  x = x_;
  y = y_;
}

void IntVector2::ZeroOut(void)
{
  x = 0;
  y = 0;
}


IntVector2 IntVector2::operator/(IntVec2Param rhs) const
{
  ErrorIf(rhs.x == 0 || rhs.y == 0, 
          "IntVector2 - Division by zero.");
  return IntVector2(x / rhs.x, y / rhs.y);
}

IntVector2 IntVector2::operator*(IntVec2Param rhs) const
{
  return IntVector2(x * rhs.x, y * rhs.y);
}

IntVector2 IntVector2::operator%(IntVec2Param rhs) const
{
  ErrorIf(rhs.x == 0 || rhs.y == 0, 
          "IntVector2 - Mod by zero.");
  return IntVector2(x % rhs.x, y % rhs.y);
}

IntVector2 operator*(int lhs, IntVec2Param rhs)
{
  return rhs * lhs;
}

IntVector2 Abs(IntVec2Param vec)
{
  return IntVector2(Math::Abs(vec.x), Math::Abs(vec.y));
}

IntVector2 Min(IntVec2Param lhs, IntVec2Param rhs)
{
  return IntVector2(Math::Min(lhs.x, rhs.x),
                    Math::Min(lhs.y, rhs.y));
}

IntVector2 Max(IntVec2Param lhs, IntVec2Param rhs)
{
  return IntVector2(Math::Max(lhs.x, rhs.x),
                    Math::Max(lhs.y, rhs.y));
}

IntVector2  IntVector2::operator~() const
{
  return IntVector2(~x,
                    ~y);
}

IntVector2  IntVector2::operator<< (IntVec2Param rhs) const
{
  return IntVector2(x << rhs.x,
                    y << rhs.y);
}

IntVector2  IntVector2::operator>> (IntVec2Param rhs) const
{
  return IntVector2(x >> rhs.x,
                    y >> rhs.y);
}

IntVector2  IntVector2::operator|  (IntVec2Param rhs) const
{
  return IntVector2(x | rhs.x,
                    y | rhs.y);
}

IntVector2  IntVector2::operator^  (IntVec2Param rhs) const
{
  return IntVector2(x ^ rhs.x,
                    y ^ rhs.y);
}

IntVector2  IntVector2::operator&  (IntVec2Param rhs) const
{
  return IntVector2(x & rhs.x,
                    y & rhs.y);
}

IntVector2& IntVector2::operator<<=(IntVec2Param rhs)
{
  x <<= rhs.x;
  y <<= rhs.y;
  return *this;
}

IntVector2& IntVector2::operator>>=(IntVec2Param rhs)
{
  x >>= rhs.x;
  y >>= rhs.y;
  return *this;
}

IntVector2& IntVector2::operator|= (IntVec2Param rhs)
{
  x |= rhs.x;
  y |= rhs.y;
  return *this;
}

IntVector2& IntVector2::operator^= (IntVec2Param rhs)
{
  x ^= rhs.x;
  y ^= rhs.y;
  return *this;
}

IntVector2& IntVector2::operator&= (IntVec2Param rhs)
{
  x &= rhs.x;
  y &= rhs.y;
  return *this;
}

}// namespace Math
