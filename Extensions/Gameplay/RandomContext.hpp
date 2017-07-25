///////////////////////////////////////////////////////////////////////////////
///
/// \file RandomContext.hpp
///
/// Authors: Joshua Davis
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// A random object that contains its own unique random state apart
/// from all other instances of this class.
class RandomContext : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream);

  ///Seeds the random number generator
  uint GetSeed();
  void SetSeed(uint seed);

  ///The max integer value that can be returned
  int GetMaxInt();

  ///Returns a random bool value
  bool Bool();

  ///Returns a random int in the range of [0, MaxInt]
  int Int();

  ///Returns a random float in the range [0,1]
  float Float();

  ///Returns a random real in the range [0,1]
  float Real();

  ///Returns a random double real in the range [0,1]
  double DoubleReal();

  ///Generates a unit length Vec2
  Vec2 UnitVector2();

  ///Randomly generates a Vec2 with its length between min and max
  Vec2 Vector2(float minLength, float maxLength);

  ///Generates a unit length Vec3
  Vec3 UnitVector3();

  ///Randomly generates a Vec3 with its length between min and max
  Vec3 Vector3(float minLength, float maxLength);

  ///Random unit length quaternion. This is also a unit quaternion
  Quat Quaternion();

  ///Int in the range [min, max]
  int RangeInclusiveMax(int min, int max);

  ///Int in the range [min, max)
  int RangeExclusiveMax(int min, int max);

  ///Int in the range [base - variance, base + variance]
  int IntVariance(int base, int variance);

  ///A random float in the range [min,max]
  float Range(float min, float max);

  ///A random float in the range [min,max]
  double DoubleRange(double min, double max);

  ///Returns a float in the range [base - variance, base + variance]
  float FloatVariance(float base, float variance);

  ///Returns a real in the range [base - variance, base + variance]
  float RealVariance(float base, float variance);

  ///Returns a double real in the range [base - variance, base + variance]
  double DoubleRealVariance(double base, double variance);

  ///Randomly rolls a number in the range [1, sides]
  uint DieRoll(uint sides);

  ///Takes a given probability that we get a true value
  bool Probability(float probOfTrue);

  ///Returns true if the coin flips heads
  bool CoinFlip();

  ///Random rotation quaternion. This is the same as calling Quaternion()
  Quat Rotation();

  ///Samples a bell curve with standard normal distribution in the range [0,1]
  ///This is equivalent to a Gaussian distribution with standard deviation of 1
  float BellCurve();

  ///Samples a bell curve with in the range [center - range, center + range]
  ///This uses a standard deviation of 1.
  float BellCurveRange(float center, float range);

  ///Samples a bell curve in the range [center - range, center + range] with the
  ///given standard deviation. Around 68% will lie within the 1st standard deviation
  float BellCurveDistribution(float center, float range, float standardDeviation);

private:
  Math::Random mRandom;

  // Do we use a random seed or the saved one?
  bool mRandomSeed;
};

} // namespace Zero
