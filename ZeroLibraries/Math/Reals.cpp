///////////////////////////////////////////////////////////////////////////////
///
/// \file Reals.cpp
/// Implementation of the real typedef and utility functions.
/// 
/// Authors: Joshua Davis, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Reals.hpp"

namespace Math
{

namespace
{
const real cTemp     = real(-0.0);
const uint cSignBit  = *reinterpret_cast<uint const*>(&cTemp);
const real cNegative = real(-1.0);
const real cPositive = real(1.0);

real gZeroForInf = real(0.0);
real gInfinite = real(1.0) / gZeroForInf;
}

const real& cInfinite = gInfinite;
const real cPi = real(3.1415926535897932384626433832795);
const real cTwoPi = real(2.0) * cPi;

real Epsilon(void) 
{
  return real(0.00000001);
}

real PositiveMax(void) 
{
  return FLT_MAX;
}

real PositiveMin()
{
  return FLT_MIN;
}

bool Equal(real lhs, real rhs) 
{
  return Abs(lhs - rhs) <= Epsilon() * (Abs(lhs) + Abs(rhs) + real(1.0));
}

bool Equal(real lhs, real rhs, real epsilon)
{
  return Abs(lhs - rhs) <= epsilon * (Abs(lhs) + Abs(rhs) + real(1.0));
}

bool NotEqual(real lhs, real rhs)
{
  return !Equal(lhs, rhs);
}

bool IsZero(real val) 
{
  return Abs(val) <= Epsilon();
}

bool IsNegative(real number)
{
  return GetSign(number) == cNegative;
}

bool IsPositive(real number)
{
  return GetSign(number) == cPositive;
}

bool LessThan(real lhs, real rhs)
{ 
  return lhs < rhs;
}

bool LessThanOrEqual(real lhs, real rhs)  
{ 
  return lhs <= rhs;
}

bool GreaterThan(real lhs, real rhs)  
{ 
  return lhs > rhs;
}

bool GreaterThanOrEqual(real lhs, real rhs)  
{ 
  return lhs >= rhs;
}

real Sqrt(real val) 
{ 
  return std::sqrt(val);
}

bool SafeSqrt(real val, real& result)
{
  if(val < 0)
    return false;

  result = Sqrt(val);
  return true;
}

real Rsqrt(real val) 
{
  return real(1.0) / std::sqrt(val);
}

real Sq(real sqrt)
{
  return sqrt * sqrt;
}

real Pow(real base, real exp)
{
  return std::pow(base, exp);
}

real Log(real val)
{
  return std::log(val);
}

real Log(real val, real base)
{
  return Log(val) / Log(base);
}

real Log10(real val)
{
  return std::log10(val);
}

real Log2(real val)
{
  //log(2) is a constant (and actually calling log(2.0) caused a linker crash so use the constant)
  real log2 = real(0.693147182);
  return Log(val) / log2;
}

real Exp(real val)
{
  return std::exp(val);
}

real Exp2(real val)
{
  return Pow(2, val);
}

u8 Abs(u8 val)
{
  return val;
}

u16 Abs(u16 val)
{
  return val;
}

u32 Abs(u32 val)
{
  return val;
}

u64 Abs(u64 val)
{
  return val;
}

s8 Abs(s8 val)
{
  return std::abs(val);
}

s16 Abs(s16 val)
{
  return std::abs(val);
}

s32 Abs(s32 val)
{
  return std::abs(val);
}

s64 Abs(s64 val)
{
  return std::abs(val);
}

float Abs(float val)
{
  return std::abs(val);
}

double Abs(double val)
{
  return std::abs(val);
}

real FMod(real dividend, real divisor)
{
  return std::fmod(dividend, divisor);
}

bool SafeFMod(real dividend, real divisor, real& result)
{
  if(divisor == 0)
    return false;

  result = FMod(dividend, divisor);
  return true;
}

real GetSign(real val) 
{
  return (*reinterpret_cast<uint*>(&val) & cSignBit) != 0 ? cNegative 
                                                          : cPositive;
  //return lhs >= real(0.0) ? real(1.0) : real(-1.0);
}

int Sign(real val)
{
  if(val < 0)
    return -1;
  return 1;
}

int Sign(int val)
{
  if(val < 0)
    return -1;
  return 1;
}

real Cos(real val)
{
  return std::cos(val);
}

real Sin(real val)
{
  return std::sin(val);
}

real Tan(real angle)
{
  return std::tan(angle);
}

real Cot(real angle)
{
  return std::tan(cPi * 0.5f - angle);
}

real Cosh(real val)
{
  return std::cosh(val);
}

real Sinh(real val)
{
  return std::sinh(val);
}

real Tanh(real angle)
{
  return std::tanh(angle);
}

real ArcCos(real angle)
{
  angle = Math::ClampIfClose(angle, real(-1.0), real(1.0), real(0.00001));
  return std::acos(angle);
}

real ArcSin(real angle)
{
  angle = Math::ClampIfClose(angle, real(-1.0), real(1.0), real(0.00001));
  return std::asin(angle);
}

real ArcTan(real angle)
{
  return std::atan(angle);
}

real ArcTan2(real y, real x)
{
  return std::atan2(y, x);
}

bool SafeArcCos(real radians, real& result)
{
  bool isSafe = Math::TryClampIfClose(radians, -1.0f, 1.0f, 0.00001f);
  if(isSafe)
    result = Math::ArcCos(radians);
  return isSafe;
}

bool SafeArcSin(real radians, real& result)
{
  bool isSafe = Math::TryClampIfClose(radians, -1.0f, 1.0f, 0.00001f);
  if(isSafe)
    result = Math::ArcSin(radians);
  return isSafe;
}

real RadToDeg(real radians)
{
  return (real(180.0) / cPi) * radians;
}

real DegToRad(real degrees)
{
  return (cPi / real(180.0)) * degrees;
}

bool IsValid(real val)
{
#ifdef _MSC_VER
  return _finite(val) != 0;
#else
  return val == val;
#endif
}

int IntegerPositiveMax()
{
  return INT_MAX;
}

int IntegerNegativeMin()
{
  return INT_MIN;
}

real Round(real value)
{
  return std::floor(value + real(0.5));
}

real Round(real value, int places)
{
  return Round(value, places, 10);
}

real Round(real value, int places, int base)
{
  real scale = std::pow(real(base), places);
  return Round(value / scale) * scale;
}

real Truncate(real value)
{
  return (real)(int)value;
}

real Truncate(real value, int places)
{
  return Truncate(value, places, 10);
}

real Truncate(real value, int places, int base)
{
  real scale = std::pow(real(base), places);
  return Truncate(value / scale) * scale;
}

real Fractional(real val)
{
  real signedFrac = val - Truncate(val);
  return Abs(signedFrac);
}

real Ceil(real value)
{
  return std::ceil(value);
}

real Ceil(real value, int places)
{
  return Ceil(value, places, 10);
}

real Ceil(real value, int places, int base)
{
  real scale = std::pow(real(base), places);
  return Ceil(value / scale) * scale;
}

real Floor(real value)
{
  return std::floor(value);
}

real Floor(real value, int places)
{
  return Floor(value, places, 10);
}

real Floor(real value, int places, int base)
{
  real scale = std::pow(real(base), places);
  return Floor(value / scale) * scale;
}

real Step(real y, real x)
{
  return (x >= y) ? real(1.0) : real(0.0);
}

int CountBits(int value)
{
  uint count = 0;
  //trick that is O(k) where k is the number of bits set
  while(value != 0)
  {
    value = (value - 1) & value;
    ++count;
  }
  return count;
}

}// namespace Math
