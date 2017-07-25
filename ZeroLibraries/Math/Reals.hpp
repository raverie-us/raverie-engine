///////////////////////////////////////////////////////////////////////////////
///
/// \file Reals.hpp
/// Declaration of the real typedef and utility functions.
/// 
/// Authors: Joshua Davis, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Utility/Standard.hpp"

namespace Math
{

typedef float real;
typedef short half;

//a pointer of the given type
typedef real* RealPointer;
//a const pointer of the given type
typedef const real* ConstRealPointer;
//a reference of the given type
typedef real& RealRef;
//a const reference of the given type
typedef const real& ConstRealRef;

const uint cX = 0;
const uint cY = 1;
const uint cZ = 2;
const uint cW = 3;

//these cannot be constants
extern const real& cInfinite;

extern const real cPi;
extern const real cTwoPi;

//Golden ratio!
const real cGoldenRatio = real(1.6180339887498948482045868343656);

ZeroShared real Epsilon(void);
ZeroShared real PositiveMax(void);
ZeroShared real PositiveMin();
ZeroShared bool Equal(real lhs, real rhs);
ZeroShared bool Equal(real lhs, real rhs, real epsilon);
ZeroShared bool NotEqual(real lhs, real rhs);
ZeroShared bool IsZero(real val);
ZeroShared bool IsNegative(real number);
ZeroShared bool IsPositive(real number);
ZeroShared bool LessThan(real lhs, real rhs);
ZeroShared bool LessThanOrEqual(real lhs, real rhs);
ZeroShared bool GreaterThan(real lhs, real rhs);
ZeroShared bool GreaterThanOrEqual(real lhs, real rhs);
ZeroShared real Sqrt(real val);
ZeroShared bool SafeSqrt(real val, real& result);
ZeroShared real Rsqrt(real val);
ZeroShared real Sq(real sqrt);
ZeroShared real Pow(real base, real exp);
ZeroShared real Log(real val);
ZeroShared real Log(real val, real base);
ZeroShared real Log10(real val);
ZeroShared real Log2(real val);
ZeroShared real Exp(real val);
ZeroShared real Exp2(real val);
ZeroShared u8 Abs(u8 val);
ZeroShared u16 Abs(u16 val);
ZeroShared u32 Abs(u32 val);
ZeroShared u64 Abs(u64 val);
ZeroShared s8 Abs(s8 val);
ZeroShared s16 Abs(s16 val);
ZeroShared s32 Abs(s32 val);
ZeroShared s64 Abs(s64 val);
ZeroShared float Abs(float val);
ZeroShared double Abs(double val);
ZeroShared real FMod(real dividend, real divisor);
ZeroShared bool SafeFMod(real dividend, real divisor, real& result);
ZeroShared real GetSign(real val);
ZeroShared int Sign(real val);
ZeroShared int Sign(int val);
ZeroShared real Cos(real val);
ZeroShared real Sin(real val);
ZeroShared real Tan(real angle);
ZeroShared real Cot(real angle);
ZeroShared real Cosh(real val);
ZeroShared real Sinh(real val);
ZeroShared real Tanh(real angle);
ZeroShared real ArcCos(real angle);
ZeroShared real ArcSin(real angle);
ZeroShared real ArcTan(real angle);
ZeroShared real ArcTan2(real y, real x);
ZeroShared bool SafeArcCos(real radians, real& result);
ZeroShared bool SafeArcSin(real radians, real& result);
ZeroShared real RadToDeg(real radians);
ZeroShared real DegToRad(real degrees);
ZeroShared real Fractional(real val);
ZeroShared real Round(real val);
ZeroShared real Round(real value, int places);
ZeroShared real Round(real value, int places, int base);
ZeroShared real Truncate(real val);
ZeroShared real Truncate(real val, int places);
ZeroShared real Truncate(real val, int places, int base);
ZeroShared real Ceil(real val);
ZeroShared real Ceil(real val, int places);
ZeroShared real Ceil(real val, int places, int base);
ZeroShared real Floor(real val);
ZeroShared real Floor(real val, int places);
ZeroShared real Floor(real val, int places, int base);
ZeroShared real Step(real y, real x);
ZeroShared int CountBits(int value);
ZeroShared bool IsValid(real val);

ZeroShared int IntegerPositiveMax();
ZeroShared int IntegerNegativeMin();

template <typename T>
ZeroSharedTemplate inline T Max(const T lhs, const T rhs)
{
  return lhs > rhs ? lhs : rhs;
}

template <typename T>
ZeroSharedTemplate inline T Min(const T lhs, const T rhs) 
{
  return lhs > rhs ? rhs : lhs;
}

template <typename T>
ZeroSharedTemplate inline T Clamp(const T x, const T xMin, const T xMax)
{
  return Max(xMin, Min(x, xMax));
}

template <typename T>
ZeroSharedTemplate inline T Clamp(const T value) 
{
  return Clamp(value, T(0), T(1));
}

/// Clamps between min and max but it sets a bool saying whether or not a value was clamped.
template <typename T>
ZeroSharedTemplate inline T DebugClamp(const T x, const T xMin, const T xMax, bool& wasClamped)
{
  wasClamped = true;
  if(x < xMin)  
    return xMin;
  if(x > xMax)
    return xMax;
  wasClamped = false;
  return x;
}

template <typename T>
ZeroSharedTemplate inline T ClampIfClose(const T x, const T xMin, const T xMax, const T epsilon)
{
  real value = x < xMin && x > (xMin - epsilon) ? xMin : x;
  value = value > xMax && value < (xMax + epsilon) ? xMax : value;
  return value;
}

template <typename T>
ZeroSharedTemplate inline bool TryClampIfClose(T& x, const T xMin, const T xMax, const T epsilon)
{
  if(x < xMin)
  {
    if(x > (xMin - epsilon))
      x = xMin;
    else
      return false;
  }
  if(x > xMax)
  {
    if(x < (xMax + epsilon))
      x = xMax;
    else
      return false;
  }
  return true;
}

template <typename T>
ZeroSharedTemplate inline real InverseLerp(const T x, const T start, const T end)
{
  if(end == start)
  {
    return real(1.0);
  }

  return (x - start) / (end - start);
}

template <typename T>
ZeroSharedTemplate inline real InverseLerpClamped(const T x, const T start, const T end)
{
  return Clamp(InverseLerp(x, start, end));
}

///Checks to see if x is within the interval of [xMin, xMax]
template <typename T>
ZeroSharedTemplate inline bool InRange(const T x, const T xMin, const T xMax)
{
    return ((xMin <= x) && (x <= xMax));
}

///Checks to see if x is within the interval of (xMin, xMax)
template <typename T>
ZeroSharedTemplate inline bool InBounds(const T x, const T xMin, const T xMax)
{
    return ((xMin < x) && (x < xMax));
}

template <typename T>
ZeroSharedTemplate inline T Wrap(const T x, const T xMin, const T xMax)
{
    return (x < xMin) ? (x + (xMax - xMin)) : 
           ((x > xMax) ? (x - (xMax - xMin)) : x);
}

template <typename T>
ZeroSharedTemplate inline void Swap(T& a, T& b)
{
  T temp(a);
  a = b;
  b = temp;
}

template <typename Data, typename T>
ZeroSharedTemplate inline Data Lerp(const Data& start, const Data& end, T interpolationValue)
{
  return (T(1.0) - interpolationValue) * start + interpolationValue * end;
}

template <typename Data>
ZeroSharedTemplate inline Data SmoothStep(const Data& start, const Data& end, real t)
{
  t = Clamp((t - start) / (end - start));
  
  // 3t^2 - 2t^3
  return t * t * (3 - 2 * t);
}

}// namespace Math
