///////////////////////////////////////////////////////////////////////////////
///
///  \file BoolVector2.cpp
///  Implementation of the BoolVector2 structure.
/// 
///  Authors: Trevor Sundberg
///  Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{

const BoolVector2 BoolVector2::cZero(false, false);
const BoolVector2 BoolVector2::cXAxis(true, false);
const BoolVector2 BoolVector2::cYAxis(false, true);


BoolVector2::BoolVector2(bool x_, bool y_)
{
  x = x_;
  y = y_;
}

bool& BoolVector2::operator[](uint index)
{
  ErrorIf(index > 1, "Math::BoolVector2 - Subscript out of range.");
  return array[index];
}

bool BoolVector2::operator[](uint index) const
{
  ErrorIf(index > 1, "Math::BoolVector2 - Subscript out of range.");
  return array[index];
}

BoolVector2 BoolVector2::operator!(void) const
{
  return BoolVector2(!x, !y);
}

bool BoolVector2::operator==(BoolVec2Param rhs) const
{
  return x == rhs.x && y == rhs.y;
}

bool BoolVector2::operator!=(BoolVec2Param rhs) const
{
  return !((*this) == rhs);
}

void BoolVector2::Set(bool x_, bool y_)
{
  x = x_;
  y = y_;
}

}// namespace Math
