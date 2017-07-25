///////////////////////////////////////////////////////////////////////////////
///
///  \file EulerAngles.cpp
///  Implementation of the EulerAngle structure, design referenced from
///  Insomniac Games.
///  
///  Authors: Benjamin Strukus
///  Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{
//----------------------------------------------------------------- Euler Angles
EulerAngles::EulerAngles(EulerOrderParam order)
  : Order(order)
{
  //
}

EulerAngles::EulerAngles(Vec3Param xyzRotation, EulerOrderParam order)
  : Angles(xyzRotation), Order(order)
{
  //
}

EulerAngles::EulerAngles(real xRotation, real yRotation, real zRotation, 
                         EulerOrderParam order)
  : Angles(xRotation, yRotation, zRotation), Order(order)
{
  //
}

EulerAngles::EulerAngles(Mat3Param matrix, EulerOrderParam order)
  : Order(order)
{
  Math::ToEulerAngles(matrix, this);
}

EulerAngles::EulerAngles(Mat4Param matrix, EulerOrderParam order)
  : Order(order)
{
  Math::ToEulerAngles(matrix, this);
}

EulerAngles::EulerAngles(QuatParam quaternion, EulerOrderParam order)
  : Order(order)
{
  Math::ToEulerAngles(quaternion, this);
}

real EulerAngles::operator [] (uint index) const
{
  return Angles[index];
}

real& EulerAngles::operator [] (uint index)
{
  return Angles[index];
}

real EulerAngles::I(void) const
{
  return Angles[Order.I()];
}

real EulerAngles::J(void) const
{
  return Angles[Order.J()];
}

real EulerAngles::K(void) const
{
  return Angles[Order.K()];
}

real EulerAngles::H(void) const
{
  return Angles[Order.H()];
}

void EulerAngles::I(real i)
{
  Angles[Order.I()] = i;
}

void EulerAngles::J(real j)
{
  Angles[Order.J()] = j;
}

void EulerAngles::K(real k)
{
  Angles[Order.K()] = k;
}

void EulerAngles::H(real h)
{
  Angles[Order.H()] = h;
}

void EulerAngles::Reorder(EulerOrderParam newOrder)
{
  Order = newOrder;
}
}// namespace Math
