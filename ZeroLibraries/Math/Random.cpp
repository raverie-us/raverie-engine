///////////////////////////////////////////////////////////////////////////////
///
/// \file StressRandom.cpp
/// Implementation of the random number and vector generation functions.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
//The following comment is required to use the Mersenne Twister
/* 
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)  
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/
#include "Precompiled.hpp"

using Zero::String;

namespace Math
{

namespace
{
//----------------------------------------------- Mersenne Twister Magic Numbers
const uint cM = 397;                //Period parameter.
const uint cMatrixA = 0x9908B0DF;   //Constant vector A.
const uint cUpperMask = 0x80000000; //Most significant w-r bits.
const uint cLowerMask = 0x7FFFFFFF; //Least significant r bits.
const uint cAllMask = 0xFFFFFFFF;   //All the bits.
}

//----------------------------------------------------------------------- Random
uint Random::mGlobalSeed = 1299827;

Random::Random(void)
{
  //we need something with higher precision than seconds (such as time(0)),
  //so use clock which is the number of clock ticks since the program was executed.
  mSeed = (uint)Zero::Time::GenerateSeed();
  mSeed ^= mGlobalSeed;

  // Call next once just to jumble it up a bit
  Next();
}

Random::Random(int initialSeed)
  : mSeed(initialSeed)
{
  //
}

uint Random::GetSeed()
{
  return mSeed;
}

void Random::SetSeed(uint seed)
{
  mSeed = seed;
}

uint Random::Next(void)
{
  mSeed = 214013 * mSeed + 2531011;
  mGlobalSeed = mSeed;
  return (mSeed >> 16) & cRandMax;
}

u32 Random::Uint32(void)
{
  return (Next() << 16) | Next();
}

u64 Random::Uint64(void)
{
  u64 a = Uint32();
  a = a << 32;
  a |= Uint32();
  return a;
}

float Random::Float(void)
{
  return Next() / float(cRandMax);
}

double Random::Double(void)
{
  return Uint64() / double(cRandMax64);
}

bool Random::Bool(void)
{
  return IntRangeInIn(0, 1) == 1;
}

int Random::IntRangeInIn(int min, int max)
{
  if(min > max)
  {
    String msg = String::Format("The min value '%d' must be less than or equal to the max value '%d'", min, max);
    //DoNotifyException("Invalid range", msg);
    return min;
  }

  ErrorIf(min > max, "Invalid range.");
  int range = max - min;
  return int(Uint32() % (range + 1)) + min;
}

int Random::IntRangeInEx(int min, int max)
{
  if(min >= max)
  {
    String msg = String::Format("The min value '%d' must be less than the max value '%d'", min, max);
    //DoNotifyException("Invalid range", msg);
    return min;
  }

  ErrorIf(min > max, "Invalid range.");
  int range = max - min;
  return (Next() % range) + min;
}

int Random::IntVariance(int base, int variance)
{
  if(variance < 0)
  {
    String msg = String::Format("The variance value '%d' cannot be negative.", variance);
    //DoNotifyException("Invalid variance", msg);
    return base;
  }

  return IntRangeInIn(base - variance, base + variance);
}

float Random::FloatRange(float min, float max)
{
  ErrorIf(min > max, "Invalid range.");
  float range = max - min;
  return (Float() * range) + min;
}

double Random::DoubleRange(double min, double max)
{
  ErrorIf(min > max, "Invalid range.");
  double range = max - min;
  return (Double() * range) + min;
}

float Random::FloatVariance(float base, float variance)
{
  //Map from [0.0f, 1.0f] to [-0.5f, 0.5f]
  float val = Float() - 0.5f;

  //Map from [-0.5f, 0.5f] to [-variance, variance)
  val *= variance * 2.0f;

  //Map from [base - variance, base + variance]
  val += base;
  
  return val;
}

double Random::DoubleVariance(double base, double variance)
{
  //Map from [0.0, 1.0] to [-0.5, 0.5]
  double val = Double() - 0.5;

  //Map from [-0.5, 0.5] to [-variance, variance)
  val *= variance * 2.0;

  //Map from [base - variance, base + variance]
  val += base;

  return val;
}

Vector2 Random::PointOnUnitCircle(void)
{
  const float cTwoPi = float(Math::cTwoPi);
  real angle = real(cTwoPi * Float());
  return Vector2(Math::Cos(angle), Math::Sin(angle));
}

//Returns a point on a unit circle with the x-axis going through the center
//of the circle.
Vector3 Random::PointOnUnitCircleX(void)
{
  Vector2 point = PointOnUnitCircle();
  return Vector3(real(0.0), point.y, point.x);
}

//Returns a point on a unit circle with the y-axis going through the center
//of the circle.
Vector3 Random::PointOnUnitCircleY(void)
{
  Vector2 point = PointOnUnitCircle();
  return Vector3(point.x, real(0.0), point.y);
}

//Returns a point on a unit circle with the z-axis going through the center
//of the circle.
Vector3 Random::PointOnUnitCircleZ(void)
{
  Vector2 point = PointOnUnitCircle();
  return Vector3(point.x, point.y, real(0.0));
}

Vector3 Random::PointOnUnitSphere(void)
{
  Vector3 v = Vector3(real(0.0), real(0.0), FloatRange(real(-1.0), real(1.0)));
  const real t = FloatRange(-cPi, cPi);
  const real radius = Sqrt(real(1.0) - (v.z * v.z));
  v.x = Cos(t) * radius;
  v.y = Sin(t) * radius;
  return Normalized(v);
}

Vector3 Random::PointInUnitSphere(void)
{
  real cubeRoot = real(1.0 / 3.0);
  real scalar = Float();
  return PointOnUnitSphere() * Math::Pow(scalar, cubeRoot);
}

//Generate uniform random quaternion
Quaternion Random::RotationQuaternion(void)
{
  //This algorithm generates a Gaussian deviate for each coordinate, so the
  //total effect is to generate a symmetric 4-D Gaussian distribution, by
  //separability. Projecting onto the surface of the hypersphere gives a uniform
  //distribution
  float x = FloatRange(-1.0f, 1.0f);
  float y = FloatRange(-1.0f, 1.0f);
  float z = FloatRange(-1.0f, 1.0f);
  float w = FloatRange(-1.0f, 1.0f);

  float s1;
  while((s1 = x * x + y * y) > 1.0f)
  {
    x = FloatRange(-1.0f, 1.0f);
    y = FloatRange(-1.0f, 1.0f);
  }

  float s2;
  while((s2 = z * z + w * w) > 1.0f)
  {
    z = FloatRange(-1.0f, 1.0f);
    w = FloatRange(-1.0f, 1.0f);
  }

  //Now the point (x, y) is uniformly distributed in the unit disk, so is the 
  //point (z, w), independently
  float num1 = -2.0f * float(Math::Log(s1));
  float num2 = -2.0f * float(Math::Log(s2));

  //Now "x * sqrt(num1 / s2)" is Gaussian distributed, using polar method.
  //Similarly for y, z, and w, and all are independent
  float r = num1 + num2;  //Sum of squares of four Gaussians
  float root1 = float(Math::Sqrt(num1 / (s1 * r)));
  float root2 = float(Math::Sqrt(num2 / (s2 * r)));

  //Normalizing onto unit sphere gives uniform unit quaternion
  return Quaternion(x * root1, y * root1, z * root2, w * root2);
}

Vector2 Random::ScaledVector2(float minLength, float maxLength)
{
  minLength = Math::Abs(minLength);
  maxLength = Math::Abs(maxLength);

  real size = maxLength - minLength;
  real scalar = Float();
  return PointOnUnitCircle() * (minLength + size * Math::Sqrt(scalar));
}

Vector3 Random::ScaledVector3(float minLength, float maxLength)
{
  minLength = Math::Abs(minLength);
  maxLength = Math::Abs(maxLength);

  real cubeRoot = 1.0f / 3.0f;
  real size = maxLength - minLength;
  real scalar = Float();
  return PointOnUnitSphere() * (minLength + size * Math::Pow(scalar, cubeRoot));
}

//Generate uniform random matrix
Matrix3 Random::RotationMatrix(void)
{
  Matrix3 matrix;
  RotationMatrix(&matrix);
  return matrix;
}

void Random::RotationMatrix(Mat3Ptr matrix)
{
  ErrorIf(matrix == nullptr, "Invalid matrix provided.");
  Quaternion quat = RotationQuaternion();
  ToMatrix3(quat, matrix);
}

int Random::DieRoll(uint sides)
{
  if(sides == 0)
  {
    //DoNotifyException("Invalid die roll", "Cannot roll a zero sided die");
    return 0;
  }
  return IntRangeInEx(0, sides) + 1;
}

float Random::BellCurve(float center, float range, float standardDeviation)
{
  float randVal = 0.0f;
  do
  {
    //This is the box-muller normal distribution algorithm.
    float u1, u2, s;
    do 
    {
      //get two random floats in the range [-1,1]
      u1 = (Float() - .5f) * 2.0f;
      u2 = (Float() - .5f) * 2.0f;
      s = u1 * u1 + u2 * u2;
    } while (s >= 1.0f || s == 0.0f);

    //this technically generates 2 random for every call but we're only saving one now.
    //the other one generated is u2 * multiplier
    float multiplier = Math::Sqrt(-2.0f * Math::Log(s) / s);
    randVal = u1 * multiplier;

    //alter the standard deviation
    randVal *= standardDeviation;
    //Since the user gave us a range, restart if we don't have a value within
    //the valid range. a normal distribution can return a value at an infinite
    //range technically, and clamping the value will alter our probability.
    //Restarting is the only way to properly deal with this.
  } while(randVal < -3.0f || randVal > 3.0f);

  //shift over to the center and remap the [-3,3] range to [center - range, center + range]
  randVal = randVal * (range / 3.0f) + center;

  return randVal;
}

//------------------------------------------------------------- Mersenne Twister
///Seeds with a call to the "time" function.
MersenneTwister::MersenneTwister(void)
  : mIndex(cN + 1)
{
  Initialize(uint(Zero::Time::GenerateSeed()));
}

///Initializes the internal array with a seed.
MersenneTwister::MersenneTwister(uint seed)
  : mIndex(cN + 1)
{
  Initialize(seed);
}

///Initialize by an array with array-length. "keys" is the array for 
///initializing keys. "keyLength" is its length.
MersenneTwister::MersenneTwister(uint keys[], uint keyLength)
  : mIndex(cN + 1)
{
  Initialize(keys, keyLength);
}

///Initializes the values with a seed.
void MersenneTwister::Initialize(uint seed)
{
  const uint cMultiplier = 1812433253;
  uint& i = mIndex;
  mValues[0] = seed & cAllMask;
  for(i = 1; i < cN; ++i)
  {
    //See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. In the previous 
    //versions, MSBs of the seed affect only MSBs of the array mValues[].
    mValues[i] = cMultiplier * (mValues[i - 1] ^ (mValues[i - 1] >> 30)) + i;
    
    //For >32 bit machines
    mValues[i] &= cAllMask;
  }
}

///Initialize by an array with array-length. "keys" is the array for 
///initializing keys. "keyLength" is its length.
void MersenneTwister::Initialize(uint keys[], uint keyLength)
{
  const uint cInitialSeed = 19650218;

  Initialize(cInitialSeed);
  uint i = 1;
  uint j = 0;
  uint k = cN > keyLength ? cN : keyLength;
  for(/* k */; k != 0; k--)
  {
    const uint h = i - 1;

    //Non-linear
    mValues[i] = (mValues[i] ^ ((mValues[h] ^ (mValues[h] >> 30)) * 1664525)) + 
                 keys[j] + j;
    mValues[i] &= cAllMask; //for WORDSIZE > 32 machines
    ++i;
    ++j;
    if(i >= cN)
    {
      mValues[0] = mValues[cN - 1];
      i = 1;
    }
    if(j >= keyLength)
    {
      j = 0;
    }
  }
  for(k = cN - 1; k != 0; --k)
  {
    const uint h = i - 1;

    //Non-linear
    mValues[i] = (mValues[i] ^ ((mValues[h] ^ (mValues[h] >> 30)) * 1566083941)) - i;
    mValues[i] &= cAllMask; //for WORDSIZE > 32 machines
    ++i;
    if(i >= cN)
    {
      mValues[0] = mValues[cN - 1];
      i = 1;
    }
  }
  mValues[0] = cUpperMask;  //MSB is 1; assuring non-zero initial array.
}

///Generates a random number on the [-2,147,483,648, 2,147,483,648] interval.
int MersenneTwister::Int(void)
{
  uint i = Uint();
  return *reinterpret_cast<int*>(&i);
}

///Generates a random number on the [0, 4,294,967,295] interval.
uint MersenneTwister::Uint(void)
{
  //mag01[x] = x * cMatrixA for x = 0, 1
  static uint mag01[2] = { 0x0, cMatrixA }; 

  //Generate N words at one time.
  if(mIndex >= cN)
  {
    //If "Initialize" has not been called, a default initial seed is used.
    if(mIndex == cN + 1)
    {
      Initialize(5489);
    }

    uint y;
    int kk;
    for(kk = 0; kk < cN - cM; ++kk)
    {
      y = (mValues[kk] & cUpperMask) | (mValues[kk + 1] & cLowerMask);
      mValues[kk] = mValues[kk + cM] ^ (y >> 1) ^ mag01[y & 0x1];
    }
    for(/*kk*/; kk < cN - 1; ++kk)
    {
      y = (mValues[kk] & cUpperMask) | (mValues[kk + 1] & cLowerMask);
      mValues[kk] = mValues[kk + (cM - cN)] ^ (y >> 1) ^ mag01[y & 0x1];
    }
    y = (mValues[cN - 1] & cUpperMask) | (mValues[0] & cLowerMask);
    mValues[cN - 1] = mValues[cM - 1] ^ (y >> 1) ^ mag01[y & 0x1];

    mIndex = 0;
  }

  uint y = mValues[mIndex++];
 
  //Tempering
  y ^= (y >> 11);
  y ^= (y << 7) & 0x9D2C5680;
  y ^= (y << 15) & 0xEFC60000;
  y ^= (y >> 18);
  return y;
}

///Generates a random number on the [0,1)-real-interval.
float MersenneTwister::Float(void)
{
  //Divided by 2^32
  return float(Uint()) * (1.0f / 4294967295.0f);
}

}// namespace Math
