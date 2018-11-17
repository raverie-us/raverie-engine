///////////////////////////////////////////////////////////////////////////////
///
///  \file EulerOrder.hpp
///  Declaration of the Euler angles order as described in Graphic Gems IV,
///  EulerOrder design referenced from Insomniac Games.
///  
///  Authors: Benjamin Strukus
///  Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Math
{

/*
  Ken Shoemake, 1993

  Order type constants, constructors, extractors
    There are 24 possible conventions, designated by:
       - EulAxI = axis used initially
       - EulPar = parity of axis permutation
       - EulRep = repetition of initial axis as last
       - EulFrm = frame from which axes are taken
    Axes I, J, K will be a permutation of X, Y, Z.
    Axis H will be either I or K, depending on EulRep.
    Frame S takes axes from initial static frame.
    If ord = (AxI = X, Par = Even, Rep = No, Frm = S), then
    {a, b, c, ord} means Rz(c)Ry(b)Rx(a), where Rz(c)v
    rotates v around Z by c radians
*/
namespace EulerOrders
{

const uint Safe[4] = { 0, 1, 2, 0 };
const uint Next[4] = { 1, 2, 0, 1 };

///The two different types of reference frames
const uint Static  = 0;
const uint Rotated = 1;

///The two different states of "is there a repeated axis?"
const uint No  = 0;
const uint Yes = 1;

///Two different states of parity
const uint Even = 0;
const uint Odd  = 1;

///CreateOrder creates an order value between 0 and 23 from 4-tuple choices.
#define CreateOrder(axis, parity, repeated, frame) (((((((axis) << 1)      + \
                                                         (parity)) << 1)   + \
                                                         (repeated)) << 1) + \
                                                         (frame))

///Bit fields to describe the different permutations of rotations, reference
///frames, repeated axes, and parity
enum Enum
{
  X = 0,
  Y,
  Z,

  //Static axes
  XYZs = CreateOrder(X, Even, No,  Static),
  XYXs = CreateOrder(X, Even, Yes, Static),
  XZYs = CreateOrder(X, Odd,  No,  Static),
  XZXs = CreateOrder(X, Odd,  Yes, Static),

  YZXs = CreateOrder(Y, Even, No,  Static),
  YZYs = CreateOrder(Y, Even, Yes, Static),
  YXZs = CreateOrder(Y, Odd,  No,  Static),
  YXYs = CreateOrder(Y, Odd,  Yes, Static),
  
  ZXYs = CreateOrder(Z, Even, No,  Static),
  ZXZs = CreateOrder(Z, Even, Yes, Static),
  ZYXs = CreateOrder(Z, Odd,  No,  Static),
  ZYZs = CreateOrder(Z, Odd,  Yes, Static),

  //Rotating axes
  ZYXr = CreateOrder(X, Even, No,  Rotated),
  XYXr = CreateOrder(X, Even, Yes, Rotated),
  YZXr = CreateOrder(X, Odd,  No,  Rotated),
  XZXr = CreateOrder(X, Odd,  Yes, Rotated),
  
  XZYr = CreateOrder(Y, Even, No,  Rotated),
  YZYr = CreateOrder(Y, Even, Yes, Rotated),
  ZXYr = CreateOrder(Y, Odd,  No,  Rotated),
  YXYr = CreateOrder(Y, Odd,  Yes, Rotated),

  YXZr = CreateOrder(Z, Even, No,  Rotated),
  ZXZr = CreateOrder(Z, Even, Yes, Rotated),
  XYZr = CreateOrder(Z, Odd,  No,  Rotated),
  ZYZr = CreateOrder(Z, Odd,  Yes, Rotated)
};

#undef CreateOrder

}// namespace EulerOrders

struct EulerOrder;
typedef EulerOrder&       EulerOrderRef;
typedef const EulerOrder& EulerOrderParam;

///Structure to hold the order of rotations when dealing with Euler angles,
///whether or not there are any repeating angles, and if the rotations are being
///described in a fixed or rotated frame of reference.
struct ZeroShared EulerOrder
{
  EulerOrder(EulerOrders::Enum eulerOrder);

  EulerOrder& operator = (EulerOrderParam rhs);

  bool operator == (EulerOrderParam rhs);
  bool operator != (EulerOrderParam rhs);

  ///Get the index of the first angle in the Euler angle rotation sequence.
  uint I(void) const;

  ///Get the index of the second angle in the Euler angle rotation sequence.
  uint J(void) const;

  ///Get the index of the third angle in the Euler angle rotation sequence.
  uint K(void) const;

  ///Will be I if there are repeating angles, will be K otherwise.
  uint H(void) const;

  bool RepeatingAngles(void) const;
  bool RotatingFrame(void) const;
  bool OddParity(void) const;

  ///Unpacks all useful information about order simultaneously.
  static void GetOrder(EulerOrder order, uint& i, uint& j, uint& k, uint& h, 
                       uint& parity, uint& repeated, uint& frame);

  EulerOrders::Enum Order;
};

}// namespace Math
