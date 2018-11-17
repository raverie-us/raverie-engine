///////////////////////////////////////////////////////////////////////////////
///
///  \file BoolVector3.cpp
///  Implementation of the BoolVector3 structure.
/// 
///  Authors: Trevor Sundberg
///  Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{

const BoolVector3 BoolVector3::cZero(false, false, false);
const BoolVector3 BoolVector3::cXAxis(true, false, false);
const BoolVector3 BoolVector3::cYAxis(false, true, false);
const BoolVector3 BoolVector3::cZAxis(false, false, true);


BoolVector3::BoolVector3(bool x_, bool y_, bool z_)
{
  x = x_;
  y = y_;
  z = z_;
}

bool& BoolVector3::operator[](uint index)
{
  ErrorIf(index > 2, "Math::BoolVector3 - Subscript out of range.");
  return array[index];
}

bool BoolVector3::operator[](uint index) const
{
  ErrorIf(index > 2, "Math::BoolVector3 - Subscript out of range.");
  return array[index];
}

BoolVector3 BoolVector3::operator!(void) const
{
  return BoolVector3(!x, !y, !z);
}

bool BoolVector3::operator==(BoolVec3Param rhs) const
{
  return x == rhs.x && y == rhs.y && z == rhs.z;
}

bool BoolVector3::operator!=(BoolVec3Param rhs) const
{
  return !((*this) == rhs);
}

void BoolVector3::Set(bool x_, bool y_, bool z_)
{
  x = x_;
  y = y_;
  z = z_;
}

}// namespace Math
