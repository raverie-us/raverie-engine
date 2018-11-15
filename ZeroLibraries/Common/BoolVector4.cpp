///////////////////////////////////////////////////////////////////////////////
///
///  \file BoolVector4.cpp
///  Implementation of the BoolVector4 structure.
/// 
///  Authors: Trevor Sundberg
///  Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{

const BoolVector4 BoolVector4::cZero(false, false, false, false);
const BoolVector4 BoolVector4::cXAxis(true, false, false, false);
const BoolVector4 BoolVector4::cYAxis(false, true, false, false);
const BoolVector4 BoolVector4::cZAxis(false, false, true, false);
const BoolVector4 BoolVector4::cWAxis(false, false, false, true);


BoolVector4::BoolVector4(bool x_, bool y_, bool z_, bool w_)
{
  x = x_;
  y = y_;
  z = z_;
  w = w_;
}

bool& BoolVector4::operator[](uint index)
{
  ErrorIf(index > 2, "Math::BoolVector4 - Subscript out of range.");
  return array[index];
}

bool BoolVector4::operator[](uint index) const
{
  ErrorIf(index > 2, "Math::BoolVector4 - Subscript out of range.");
  return array[index];
}

BoolVector4 BoolVector4::operator!(void) const
{
  return BoolVector4(!x, !y, !z, !w);
}

bool BoolVector4::operator==(BoolVec4Param rhs) const
{
  return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
}

bool BoolVector4::operator!=(BoolVec4Param rhs) const
{
  return !((*this) == rhs);
}

void BoolVector4::Set(bool x_, bool y_, bool z_, bool w_)
{
  x = x_;
  y = y_;
  z = z_;
  w = w_;
}

}// namespace Math
