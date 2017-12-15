///////////////////////////////////////////////////////////////////////////////
///
/// \file Random.hpp
/// Declaration of the Random number and vector generation functions.
/// 
/// Authors: Dane Curbow
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Math/Reals.hpp"
#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"
#include "Math/Matrix3.hpp"
#include "Math/Quaternion.hpp"
#include <random>

namespace Math
{
//----------------------------------------------------------------------- Random
class ZeroShared Random
{
public:
  ///Seeds with a call to the "time" function
  Random(void);
  Random(int initialSeed);
  
  /// Re-seeds the random number generator with the provided seed
  void SetSeed(uint seed);
  uint GetSeed();

  ///Generates a random number on the [0 - 18,446,744,073,709,551,615] interval.
  u64 Next(void);

  ///Generates a random number on the [0 - 65,536] interval.
  u16 Uint16(void);

  ///Generates a random number on the [0 - 4,294,967,295] interval.
  u32 Uint32(void);

  ///Generates a random number on the [0 - 18,446,744,073,709,551,615] interval.
  u64 Uint64(void);

  ///Generates a random number on the [-2,147,483,648 - 2,147,483,647] interval.
  int Int(void);

  ///Generates a random number on the [0 - 1]-real-interval.
  float Float(void);

  ///Generates a random number on the [0 - 1]-double real-interval.
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



  // Global seed only when we don't seed the random
  static uint mGlobalSeed;
  uint mSeed;
  
  // Max values for different data types
  static const u16 cRandMaxU16 = USHRT_MAX;
  static const u32 cRandMaxU32 = UINT_MAX;
  static const u64 cRandMaxU64 = ULLONG_MAX;
  static const int cRandMinS32 = INT_MIN;
  static const int cRandMaxS32 = INT_MAX;

  // Typedefs for different number generators and distribution types.
  // Multiplication factor was a randomly chosen prime number.
  typedef std::linear_congruential_engine<std::uint_fast64_t, 753353, 0, cRandMaxU64> RandomNumberGenerator64;
  
  // Uniform distribution class evenly distributes output of a random number generator over the range provided when constructed
  typedef std::uniform_int_distribution<u16>     U16UniformDistribution;
  typedef std::uniform_int_distribution<u32>     U32UniformDistribution;
  typedef std::uniform_int_distribution<int>     S32UniformDistribution;
  typedef std::uniform_real_distribution<float>  FloatUniformDistribution;
  typedef std::uniform_real_distribution<double> DoubleUniformDistribution;

private:
  RandomNumberGenerator64 mRandomGenerator;
};

}// namespace Math
