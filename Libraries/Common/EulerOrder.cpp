///////////////////////////////////////////////////////////////////////////////
///
///  \file EulerOrder.cpp
///  Implementation of the Euler angles order as described in Graphic Gems IV,
///  EulerOrder design referenced from Insomniac Games.
///  
///  Authors: Benjamin Strukus
///  Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
namespace Math
{

EulerOrder::EulerOrder(EulerOrders::Enum eulerOrder)
  : Order(eulerOrder)
{
  //
}

EulerOrder& EulerOrder::operator = (EulerOrderParam rhs)
{
  Order = rhs.Order;
  return *this;
}

bool EulerOrder::operator == (EulerOrderParam rhs)
{
  return Order == rhs.Order;
}

bool EulerOrder::operator != (EulerOrderParam rhs)
{
  return Order != rhs.Order;
}

uint EulerOrder::I(void) const
{
  return EulerOrders::Safe[((uint(Order) >> 3) & 3)];
}

uint EulerOrder::J(void) const
{
  return EulerOrders::Next[I() + (OddParity() ? 1 : 0)];
}

uint EulerOrder::K(void) const
{
  return EulerOrders::Next[I() + (OddParity() ? 0 : 1)];
}

uint EulerOrder::H(void) const
{
  return RepeatingAngles() == EulerOrders::No ? K() : I();
}

bool EulerOrder::RepeatingAngles(void) const
{
  return ((uint(Order) >> 1) & 1) == EulerOrders::Yes;
}

bool EulerOrder::RotatingFrame(void) const
{
  return (uint(Order) & 1) == EulerOrders::Rotated;
}

bool EulerOrder::OddParity(void) const
{
  return ((uint(Order) >> 2) & 1) == EulerOrders::Odd;
}

void EulerOrder::GetOrder(EulerOrder order, uint& i, uint& j, uint& k, uint& h,
                          uint& parity, uint& repeated, uint& frame)
{
  uint orderValue = uint(order.Order);
  
  frame = orderValue & 1;
  orderValue >>= 1;
  
  repeated = orderValue & 1;
  orderValue >>= 1;      
  
  parity = orderValue & 1;
  orderValue >>= 1;      
  
  i = EulerOrders::Safe[orderValue & 3];      
  
  j = EulerOrders::Next[i + parity];      
  
  k = EulerOrders::Next[i + 1 - parity];  
  
  h = repeated ? k : i;           
}

}// namespace Math
