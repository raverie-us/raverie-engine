///////////////////////////////////////////////////////////////////////////////
///
///  \file EulerAngles.hpp
///  Declaration of the EulerAngle structure, interface referenced from
///  Insomniac Games, implementation referenced from Graphics Gems IV.
///  
///  Authors: Benjamin Strukus
///  Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Math/EulerOrder.hpp"
#include "Math/Vector3.hpp"

namespace Math
{

///Forward declaration
struct Matrix3;
typedef const Matrix3& Mat3Param;
typedef Matrix3& Mat3Ref;

///Forward declaration
struct Matrix4;
typedef const Matrix4& Mat4Param;
typedef Matrix4& Mat4Ref;

///Forward declaration
struct Quaternion;
typedef const Quaternion& QuatParam;
typedef Quaternion& QuatRef;

struct EulerAngles;
typedef const EulerAngles&  EulerAnglesParam;
typedef EulerAngles&        EulerAnglesRef;
typedef EulerAngles*        EulerAnglesPtr;

//----------------------------------------------------------------- Euler Angles
///Structure to provide a consistent way to deal with rotations stored as Euler
///angles.
struct ZeroShared EulerAngles
{
  EulerAngles(EulerOrderParam order);
  EulerAngles(Vec3Param xyzRotation, EulerOrderParam order);
  EulerAngles(real xRotation, real yRotation, real zRotation, 
              EulerOrderParam order);
  EulerAngles(Mat3Param matrix, EulerOrderParam order);
  EulerAngles(Mat4Param matrix, EulerOrderParam order);
  EulerAngles(QuatParam quaternion, EulerOrderParam order);

  ///Index operator to access the Angles data directly.
  real operator [] (uint index) const;

  ///Index operator to access the Angles data directly.
  real& operator [] (uint index);


  real I(void) const;
  real J(void) const;
  real K(void) const;
  real H(void) const;

  void I(real i);
  void J(real j);
  void K(real k);
  void H(real h);

  void Reorder(EulerOrderParam newOrder);

  Vector3     Angles;
  EulerOrder  Order;
};

}// namespace Math
