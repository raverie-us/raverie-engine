// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{
RaverieDefineType(Random, builder, type)
{
  type->AddAttribute(ImportDocumentation);

  RaverieFullBindDestructor(builder, type, Random);
  RaverieFullBindConstructor(builder, type, Random, nullptr, uint);
  RaverieFullBindConstructor(builder, type, Random, nullptr);

  // Change to a property later
  RaverieFullBindGetterSetter(builder, type, &Random::GetSeed, RaverieNoOverload, &Random::SetSeed, RaverieNoOverload, "Seed");
  RaverieFullBindGetterSetter(builder, type, &Random::GetMaxInteger, RaverieNoOverload, RaverieNoSetter, RaverieNoOverload, "MaxInteger");

  // Basic type randoms
  RaverieFullBindMethod(builder, type, &Random::Boolean, RaverieNoOverload, "Boolean", nullptr);
  RaverieFullBindMethod(builder, type, &Random::Integer, RaverieNoOverload, "Integer", nullptr);
  RaverieFullBindMethod(builder, type, &Random::Real, RaverieNoOverload, "Real", nullptr);
  RaverieFullBindMethod(builder, type, &Random::DoubleReal, RaverieNoOverload, "DoubleReal", nullptr);
  RaverieFullBindMethod(builder, type, &Random::UnitReal2, RaverieNoOverload, "UnitReal2", nullptr);
  RaverieFullBindMethod(builder, type, &Random::Real2, RaverieNoOverload, "Real2", "minLength, maxLength");
  RaverieFullBindMethod(builder, type, &Random::UnitReal3, RaverieNoOverload, "UnitReal3", nullptr);
  RaverieFullBindMethod(builder, type, &Random::Real3, RaverieNoOverload, "Real3", "minLength, maxLength");
  RaverieFullBindMethod(builder, type, &Random::Quaternion, RaverieNoOverload, "Quaternion", nullptr);

  // Range/variance helpers
  RaverieFullBindMethod(builder, type, &Random::RangeInclusiveMax, RaverieNoOverload, "RangeInclusiveMax", "min, max");
  RaverieFullBindMethod(builder, type, &Random::RangeExclusiveMax, RaverieNoOverload, "RangeExclusiveMax", "min, max");
  RaverieFullBindMethod(builder, type, &Random::Range, RaverieNoOverload, "Range", "min, max");
  RaverieFullBindMethod(builder, type, &Random::DoubleRange, RaverieNoOverload, "DoubleRange", "min, max");
  RaverieFullBindMethod(builder, type, &Random::Variance, (int(Random::*)(int, int)), "Variance", "baseValue, variance");
  RaverieFullBindMethod(builder, type, &Random::Variance, (float(Random::*)(float, float)), "Variance", "baseValue, variance");
  RaverieFullBindMethod(builder, type, &Random::Variance, (double(Random::*)(double, double)), "Variance", "baseValue, variance");

  // Some more "user friendly" functions for designers
  RaverieFullBindMethod(builder, type, &Random::DieRoll, RaverieNoOverload, "DieRoll", nullptr);
  RaverieFullBindMethod(builder, type, &Random::Probability, RaverieNoOverload, "Probability", "probabilityOfTrue");
  RaverieFullBindMethod(builder, type, &Random::CoinFlip, RaverieNoOverload, "CoinFlip", nullptr);
  RaverieFullBindMethod(builder, type, &Random::Rotation, RaverieNoOverload, "Rotation", nullptr);

  // Bell curve (Gaussian) distribution
  RaverieFullBindMethod(builder, type, &Random::BellCurve, (float(Random::*)()), "BellCurve", nullptr);
  RaverieFullBindMethod(builder, type, &Random::BellCurve, (float(Random::*)(float, float)), "BellCurve", "center, range");
  RaverieFullBindMethod(builder, type, &Random::BellCurve, (float(Random::*)(float, float, float)), "BellCurve", "center, range, standardDeviation");
}

Random::Random()
{
  this->OriginalSeed = this->Generator.mSeed;
}

Random::Random(uint seed) : Generator(seed)
{
  this->OriginalSeed = seed;
}

void Random::SetSeed(uint seed)
{
  this->Generator = Math::Random(seed);
  this->OriginalSeed = seed;
}

uint Random::GetSeed()
{
  return this->OriginalSeed;
}

int Random::GetMaxInteger()
{
  return Math::Random::cRandMax;
}

bool Random::Boolean()
{
  return this->Generator.IntRangeInIn(0, 1) == 1;
}

int Random::Integer()
{
  return this->Generator.Next();
}

float Random::Real()
{
  return this->Generator.Float();
}

double Random::DoubleReal()
{
  return this->Generator.Double();
}

Math::Vector2 Random::UnitReal2()
{
  return this->Generator.PointOnUnitCircle();
}

Math::Vector2 Random::Real2(float minLength, float maxLength)
{
  return this->Generator.ScaledVector2(minLength, maxLength);
}

Math::Vector3 Random::UnitReal3()
{
  return this->Generator.PointOnUnitSphere();
}

Math::Vector3 Random::Real3(float minLength, float maxLength)
{
  return this->Generator.ScaledVector3(minLength, maxLength);
}

Raverie::Quaternion Random::Quaternion()
{
  return this->Generator.RotationQuaternion();
}

int Random::RangeInclusiveMax(int min, int max)
{
  return this->Generator.IntRangeInIn(min, max);
}

int Random::RangeExclusiveMax(int min, int max)
{
  return this->Generator.IntRangeInEx(min, max);
}

int Random::Variance(int base, int variance)
{
  return this->Generator.IntVariance(base, variance);
}

float Random::Range(float min, float max)
{
  return this->Generator.FloatRange(min, max);
}

double Random::DoubleRange(double min, double max)
{
  return this->Generator.DoubleRange(min, max);
}

float Random::Variance(float base, float variance)
{
  return this->Generator.FloatVariance(base, variance);
}

double Random::Variance(double base, double variance)
{
  return this->Generator.DoubleVariance(base, variance);
}

uint Random::DieRoll(uint sides)
{
  return this->Generator.DieRoll(sides);
}

bool Random::Probability(float probOfTrue)
{
  return this->Generator.Float() < probOfTrue;
}

bool Random::CoinFlip()
{
  return this->Generator.IntRangeInIn(0, 1) == 1;
}

Raverie::Quaternion Random::Rotation()
{
  return this->Generator.RotationQuaternion();
}

float Random::BellCurve()
{
  return this->Generator.BellCurve(0.5f, 0.5f, 1.0f);
}

float Random::BellCurve(float center, float range)
{
  return this->Generator.BellCurve(center, range, 1.0f);
}

float Random::BellCurve(float center, float range, float standardDeviation)
{
  return this->Generator.BellCurve(center, range, standardDeviation);
}
} // namespace Raverie
