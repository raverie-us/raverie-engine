/**************************************************************\
* Author: Joshua Davis
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_RANDOM_HPP
#define ZILCH_RANDOM_HPP

namespace Zilch
{
  // Contains utility functions for random generation
  class ZeroShared Random
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Default constructor (grabs the random seed)
    Random();

    // Construct a random generator with a specific seed
    Random(uint seed);

    // The seed controls what random numbers are generated in a sequence (determanistically)
    // The same seed will always generate the same string of random numbers
    void SetSeed(uint seed);
    uint GetSeed();

    // Returns the max integer value that can be returned
    static int GetMaxInteger();

    // Returns a random boolean value
    bool Boolean();

    // Returns a random integer in the range of [0, MaxInt]
    int Integer();

    // Returns a random real in the range [0,1]
    float Real();

    // Returns a random double real in the range [0,1]
    double DoubleReal();

    // Generates a unit length Real2
    Math::Vector2 UnitReal2();

    // Randomly generates a Real2 with its length between min and max
    Math::Vector2 Real2(float minLength, float maxLength);

    // Generates a unit length Real3
    Math::Vector3 UnitReal3();

    // Randomly generates a Real3 with its length between min and max
    Math::Vector3 Real3(float minLength, float maxLength);

    // Random unit length quaternion. This is also a unit quaternion
    Zilch::Quaternion Quaternion();

    // Integer in the range [min, max)
    int RangeInclusiveMax(int min, int max);

    // Integer in the range [min, max]
    int RangeExclusiveMax(int min, int max);

    // Integer in the range [base - variance, base + variance]
    int Variance(int base, int variance);

    // A random Real in the range [min,max]
    float Range(float min, float max);

    // A random DoubleReal in the range [min,max]
    double DoubleRange(double min, double max);

    // Returns a number in the range [base - variance, base + variance]
    float Variance(float base, float variance);

    // Returns a number in the range [base - variance, base + variance]
    double Variance(double base, double variance);

    // Randomly rolls a number in the range [1, sides]
    uint DieRoll(uint sides);

    // Takes a given probability that we get a true value
    bool Probability(float probOfTrue);

    // Returns true if the coin flips heads
    bool CoinFlip();

    // Random rotation quaternion. This is the same as calling Quaternion()
    Zilch::Quaternion Rotation();

    // Samples a bell curve with standard normal distribution in the range [0,1]
    // This is equivalent to a Gaussian distribution with standard deviation of 1
    float BellCurve();

    // Samples a bell curve with in the range [center - range, center + range]
    // This uses a standard deviation of 1.
    float BellCurve(float center, float range);

    // Samples a bell curve in the range [center - range, center + range] with the
    // given standard deviation. Around 68% will lie within the 1st standard deviation
    float BellCurve(float center, float range, float standardDeviation);

  private:

    // The internal random number generator we use
    Math::Random Generator;

    // We store the seed separately so that users can query what the original seed was
    uint OriginalSeed;
  };
}

#endif
