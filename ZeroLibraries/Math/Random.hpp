///////////////////////////////////////////////////////////////////////////////
///
/// \file Random.hpp
/// Declaration of the Random number and vector generation functions.
/// 
/// Authors: Chris Peters, Benjamin Strukus
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
#pragma once
#include "Math/Reals.hpp"
#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"
#include "Math/Matrix3.hpp"
#include "Math/Quaternion.hpp"

namespace Math
{
//----------------------------------------------------------------------- Random
class ZeroShared Random
{
public:
  ///Seeds with a call to the "time" function
  Random(void);
  Random(int initialSeed);

  uint GetSeed();
  void SetSeed(uint seed);

  ///Generates a random number on the [0, 32,767] interval.
  uint Next(void);

  ///Generates a random number on the [0, 4,294,967,295] interval.
  u32 Uint32(void);

  ///Generates a random number on the [0, 18,446,744,073,709,551,616] interval.
  u64 Uint64(void);

  ///Generates a random number on the [0,1]-real-interval.
  float Float(void);

  ///Generates a random number on the [0,1]-double real-interval.
  double Double(void);

  ///Generates a random boolean value;
  bool Bool(void);

  ///Returns an integer value in the range of [min, max]
  int IntRangeInIn(int min, int max);

  ///Returns an integer value in the range of [min, max)
  int IntRangeInEx(int min, int max);

  ///Returns a integer value in the range of 
  ///[base - variance, base + variance]
  int IntVariance(int base, int variance);

  ///Returns a floating point value in the range of [min, max]
  float FloatRange(float min, float max);

  ///Returns a floating point value in the range of [min, max]
  double DoubleRange(double min, double max);

  ///Returns a floating point value in the range of 
  ///[base - variance, base + variance]
  float FloatVariance(float base, float variance);

  ///Returns a floating point value in the range of 
  ///[base - variance, base + variance]
  double DoubleVariance(double base, double variance);

  Vector2 PointOnUnitCircle(void);

  ///Returns a point on a unit circle with the x-axis going through the center
  ///of the circle.
  Vector3 PointOnUnitCircleX(void);

  ///Returns a point on a unit circle with the y-axis going through the center
  ///of the circle.
  Vector3 PointOnUnitCircleY(void);

  ///Returns a point on a unit circle with the z-axis going through the center
  ///of the circle.
  Vector3 PointOnUnitCircleZ(void);
  

  Vector3 PointOnUnitCircle(uint axis);

  Vector3 PointOnUnitSphere(void);

  Vector3 PointInUnitSphere(void);

  ///Generate uniform random quaternion
  Quaternion RotationQuaternion(void);

  ///Randomly generates a Vec22 with its length between min and max
  Vector2 ScaledVector2(float minLength, float maxLength);

  ///Randomly generates a Vec3 with its length between min and max
  Vector3 ScaledVector3(float minLength, float maxLength);

  ///Generate uniform random matrix
  Matrix3 RotationMatrix(void);
  void RotationMatrix(Mat3Ptr matrix);

  // Randomly rolls a number in the range [1, sides]
  int DieRoll(uint sides);

  float BellCurve(float center, float range, float standardDeviation);

  static const uint cRandMax = 0x7FFF;
  // Since our rand 32 and 64 just take a rand16 and shift into position
  // instead of our max being 0x7FFFFFFFFFFFFFFF it is 0x7FFF7FFF7FFF7FFF
  static const u64  cRandMax64 = 0x7FFF7FFF7FFF7FFF;

  // Global seed only when we don't seed the random
  // This is set every time anyone calls 'Next()', even if it's not used by the class
  static uint mGlobalSeed;

  uint mSeed;
};

//------------------------------------------------------------- Mersenne Twister
class ZeroShared MersenneTwister
{
public:
  ///Seeds with a call to the "time" function.
  MersenneTwister(void);

  ///Initializes the internal array with a seed.
  MersenneTwister(uint seed);

  ///Initialize by an array with array-length. "keys" is the array for 
  ///initializing keys. "keyLength" is its length.
  MersenneTwister(uint keys[], uint keyLength);

  ///Initializes the values with a seed.
  void Initialize(uint seed);

  ///Initialize by an array with array-length. "keys" is the array for 
  ///initializing keys. "keyLength" is its length.
  void Initialize(uint keys[], uint keyLength);

  ///Generates a random number on the [-2,147,483,648, 2,147,483,648] interval.
  int Int(void);

  ///Generates a random number on the [0, 4,294,967,295] interval.
  uint Uint(void);

  ///Generates a random number on the [0,1]-real-interval.
  float Float(void);

private:
  static const uint cN = 624;
  uint mValues[cN]; //The array for the state vector.
  uint mIndex;       //Index == cN+1 means the values aren't initialized.
};

}// namespace Math
