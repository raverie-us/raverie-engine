// MIT Licensed (see LICENSE.md).
#pragma once
#include "Reals.hpp"
#include "Vector2.hpp"
#include "Vector3.hpp"
#include "Matrix3.hpp"
#include "Quaternion.hpp"

namespace Math
{
class ZeroShared Random
{
public:
  /// Seeds with a call to the "time" function
  Random(void);
  Random(int initialSeed);

  uint GetSeed();
  void SetSeed(uint seed);

  /// Generates a random number on the [0, 32,767] interval.
  uint Next(void);

  /// Generates a random number on the [0, 4,294,967,295] interval.
  u32 Uint32(void);

  /// Generates a random number on the [0, 18,446,744,073,709,551,616] interval.
  u64 Uint64(void);

  /// Generates a random number on the [0,1]-real-interval.
  float Float(void);

  /// Generates a random number on the [0,1]-double real-interval.
  double Double(void);

  /// Generates a random boolean value;
  bool Bool(void);

  /// Returns an integer value in the range of [min, max]
  int IntRangeInIn(int min, int max);

  /// Returns an integer value in the range of [min, max)
  int IntRangeInEx(int min, int max);

  /// Returns a integer value in the range of
  ///[base - variance, base + variance]
  int IntVariance(int base, int variance);

  /// Returns a floating point value in the range of [min, max]
  float FloatRange(float min, float max);

  /// Returns a floating point value in the range of [min, max]
  double DoubleRange(double min, double max);

  /// Returns a floating point value in the range of
  ///[base - variance, base + variance]
  float FloatVariance(float base, float variance);

  /// Returns a floating point value in the range of
  ///[base - variance, base + variance]
  double DoubleVariance(double base, double variance);

  Vector2 PointOnUnitCircle(void);

  /// Returns a point on a unit circle with the x-axis going through the center
  /// of the circle.
  Vector3 PointOnUnitCircleX(void);

  /// Returns a point on a unit circle with the y-axis going through the center
  /// of the circle.
  Vector3 PointOnUnitCircleY(void);

  /// Returns a point on a unit circle with the z-axis going through the center
  /// of the circle.
  Vector3 PointOnUnitCircleZ(void);

  Vector3 PointOnUnitCircle(uint axis);

  Vector3 PointOnUnitSphere(void);

  Vector3 PointInUnitSphere(void);

  /// Generate uniform random quaternion
  Quaternion RotationQuaternion(void);

  /// Randomly generates a Vec22 with its length between min and max
  Vector2 ScaledVector2(float minLength, float maxLength);

  /// Randomly generates a Vec3 with its length between min and max
  Vector3 ScaledVector3(float minLength, float maxLength);

  /// Generate uniform random matrix
  Matrix3 RotationMatrix(void);
  void RotationMatrix(Mat3Ptr matrix);

  // Randomly rolls a number in the range [1, sides]
  int DieRoll(uint sides);

  float BellCurve(float center, float range, float standardDeviation);

  static const uint cRandMax = 0x7FFF;
  // Since our rand 32 and 64 just take a rand16 and shift into position
  // instead of our max being 0x7FFFFFFFFFFFFFFF it is 0x7FFF7FFF7FFF7FFF
  static const u64 cRandMax64 = 0x7FFF7FFF7FFF7FFF;

  // Global seed only when we don't seed the random
  // This is set every time anyone calls 'Next()', even if it's not used by the
  // class
  static uint mGlobalSeed;

  uint mSeed;
};

} // namespace Math
