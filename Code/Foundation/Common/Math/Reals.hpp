// MIT Licensed (see LICENSE.md).
#pragma once

#include "Utility/Standard.hpp"

namespace Math
{

typedef float real;
typedef short half;

// a pointer of the given type
typedef real* RealPointer;
// a const pointer of the given type
typedef const real* ConstRealPointer;
// a reference of the given type
typedef real& RealRef;
// a const reference of the given type
typedef const real& ConstRealRef;

const uint cX = 0;
const uint cY = 1;
const uint cZ = 2;
const uint cW = 3;

// these cannot be constants
extern const real& cInfinite;

extern const real cPi;
extern const real cTwoPi;

// Golden ratio!
const real cGoldenRatio = real(1.6180339887498948482045868343656);

real Epsilon(void);
real PositiveMax(void);
real PositiveMin();
bool Equal(real lhs, real rhs);
bool Equal(real lhs, real rhs, real epsilon);
bool NotEqual(real lhs, real rhs);
bool IsZero(real val);
bool IsNegative(real number);
bool IsPositive(real number);
bool LessThan(real lhs, real rhs);
bool LessThanOrEqual(real lhs, real rhs);
bool GreaterThan(real lhs, real rhs);
bool GreaterThanOrEqual(real lhs, real rhs);
real Sqrt(real val);
bool SafeSqrt(real val, real& result);
real Rsqrt(real val);
real Sq(real sqrt);
real Pow(real base, real exp);
real Log(real val);
real Log(real val, real base);
real Log10(real val);
real Log2(real val);
real Exp(real val);
real Exp2(real val);
u8 Abs(u8 val);
u16 Abs(u16 val);
u32 Abs(u32 val);
u64 Abs(u64 val);
s8 Abs(s8 val);
s16 Abs(s16 val);
s32 Abs(s32 val);
s64 Abs(s64 val);
float Abs(float val);
double Abs(double val);
real FMod(real dividend, real divisor);
bool SafeFMod(real dividend, real divisor, real& result);
real GetSign(real val);
int Sign(real val);
int Sign(int val);
real Cos(real val);
real Sin(real val);
real Tan(real angle);
real Cot(real angle);
real Cosh(real val);
real Sinh(real val);
real Tanh(real angle);
real ArcCos(real angle);
real ArcSin(real angle);
real ArcTan(real angle);
real ArcTan2(real y, real x);
bool SafeArcCos(real radians, real& result);
bool SafeArcSin(real radians, real& result);
real RadToDeg(real radians);
real DegToRad(real degrees);
real Fractional(real val);
real Round(real val);
real Round(real value, int places);
real Round(real value, int places, int base);
real Truncate(real val);
real Truncate(real val, int places);
real Truncate(real val, int places, int base);
real Ceil(real val);
real Ceil(real val, int places);
real Ceil(real val, int places, int base);
real Floor(real val);
real Floor(real val, int places);
real Floor(real val, int places, int base);
/// If y <= x then 1 is returned, otherwise 0 is returned.
real Step(real y, real x);
int CountBits(int value);
bool IsValid(real val);

double DoublePositiveMax();
double DoublePositiveMin();
byte BytePositiveMax();
byte BytePositiveMin();
int IntegerPositiveMax();
int IntegerNegativeMin();
long long int DoubleIntegerPositiveMax();
long long int DoubleIntegerNegativeMin();

template <typename T>
inline T Max(const T lhs, const T rhs)
{
  return lhs > rhs ? lhs : rhs;
}

template <typename T>
inline T Min(const T lhs, const T rhs)
{
  return lhs > rhs ? rhs : lhs;
}

template <typename T>
inline T Clamp(const T x, const T xMin, const T xMax)
{
  return Max(xMin, Min(x, xMax));
}

template <typename T>
inline T Clamp(const T value)
{
  return Clamp(value, T(0), T(1));
}

/// Clamps between min and max but it sets a bool saying whether or not a value
/// was clamped.
template <typename T>
inline T DebugClamp(const T x, const T xMin, const T xMax, bool& wasClamped)
{
  wasClamped = true;
  if (x < xMin)
    return xMin;
  if (x > xMax)
    return xMax;
  wasClamped = false;
  return x;
}

template <typename T>
inline T ClampIfClose(const T x, const T xMin, const T xMax, const T epsilon)
{
  real value = x < xMin && x > (xMin - epsilon) ? xMin : x;
  value = value > xMax && value < (xMax + epsilon) ? xMax : value;
  return value;
}

template <typename T>
inline bool TryClampIfClose(T& x, const T xMin, const T xMax, const T epsilon)
{
  if (x < xMin)
  {
    if (x > (xMin - epsilon))
      x = xMin;
    else
      return false;
  }
  if (x > xMax)
  {
    if (x < (xMax + epsilon))
      x = xMax;
    else
      return false;
  }
  return true;
}

template <typename T>
inline real InverseLerp(const T x, const T start, const T end)
{
  if (end == start)
  {
    return real(1.0);
  }

  return (x - start) / (end - start);
}

template <typename T>
inline real InverseLerpClamped(const T x, const T start, const T end)
{
  return Clamp(InverseLerp(x, start, end));
}

/// Checks to see if x is within the interval of [xMin, xMax]
template <typename T>
inline bool InRange(const T x, const T xMin, const T xMax)
{
  return ((xMin <= x) && (x <= xMax));
}

/// Checks to see if x is within the interval of (xMin, xMax)
template <typename T>
inline bool InBounds(const T x, const T xMin, const T xMax)
{
  return ((xMin < x) && (x < xMax));
}

template <typename T>
inline T Wrap(const T x, const T xMin, const T xMax)
{
  return (x < xMin) ? (x + (xMax - xMin)) : ((x > xMax) ? (x - (xMax - xMin)) : x);
}

template <typename T>
inline void Swap(T& a, T& b)
{
  T temp(a);
  a = b;
  b = temp;
}

template <typename Data, typename T>
inline Data Lerp(const Data& start, const Data& end, T interpolationValue)
{
  return (Data)((T(1.0) - interpolationValue) * start + interpolationValue * end);
}

template <typename Data>
inline Data SmoothStep(const Data& start, const Data& end, real t)
{
  t = Clamp((t - start) / (end - start));

  // 3t^2 - 2t^3
  return t * t * (3 - 2 * t);
}

} // namespace Math
