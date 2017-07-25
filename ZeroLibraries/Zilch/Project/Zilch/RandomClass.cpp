/**************************************************************\
* Author: Joshua Davis
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  ZilchDefineType(Random, builder, type)
  {
    ZilchFullBindDestructor(builder, type, Random);
    ZilchFullBindConstructor(builder, type, Random, nullptr, uint);
    ZilchFullBindConstructor(builder, type, Random, nullptr);

    // Change to a property later
    ZilchFullBindGetterSetter(builder, type, &Random::GetSeed, ZilchNoOverload, &Random::SetSeed, ZilchNoOverload, "Seed");
    ZilchFullBindGetterSetter(builder, type, &Random::GetMaxInteger, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "MaxInteger");

    // Basic type randoms
    ZilchFullBindMethod(builder, type, &Random::Boolean,    ZilchNoOverload, "Boolean",    nullptr);
    ZilchFullBindMethod(builder, type, &Random::Integer,    ZilchNoOverload, "Integer",    nullptr);
    ZilchFullBindMethod(builder, type, &Random::Real,       ZilchNoOverload, "Real",       nullptr);
    ZilchFullBindMethod(builder, type, &Random::DoubleReal, ZilchNoOverload, "DoubleReal", nullptr);
    ZilchFullBindMethod(builder, type, &Random::UnitReal2,  ZilchNoOverload, "UnitReal2",  nullptr);
    ZilchFullBindMethod(builder, type, &Random::Real2,      ZilchNoOverload, "Real2",      "minLength, maxLength");
    ZilchFullBindMethod(builder, type, &Random::UnitReal3,  ZilchNoOverload, "UnitReal3",  nullptr);
    ZilchFullBindMethod(builder, type, &Random::Real3,      ZilchNoOverload, "Real3",      "minLength, maxLength");
    ZilchFullBindMethod(builder, type, &Random::Quaternion, ZilchNoOverload, "Quaternion", nullptr);

    // Range/variance helpers
    ZilchFullBindMethod(builder, type, &Random::RangeInclusiveMax, ZilchNoOverload, "RangeInclusiveMax", "min, max");
    ZilchFullBindMethod(builder, type, &Random::RangeExclusiveMax, ZilchNoOverload, "RangeExclusiveMax", "min, max");
    ZilchFullBindMethod(builder, type, &Random::Range,       ZilchNoOverload, "Range", "min, max");
    ZilchFullBindMethod(builder, type, &Random::DoubleRange, ZilchNoOverload, "DoubleRange", "min, max");
    ZilchFullBindMethod(builder, type, &Random::Variance, (int   (Random::*)(int,   int)),   "Variance", "baseValue, variance");
    ZilchFullBindMethod(builder, type, &Random::Variance, (float (Random::*)(float, float)), "Variance", "baseValue, variance");
    ZilchFullBindMethod(builder, type, &Random::Variance, (double (Random::*)(double, double)), "Variance", "baseValue, variance");

    // Some more "user friendly" functions for designers
    ZilchFullBindMethod(builder, type, &Random::DieRoll,     ZilchNoOverload, "DieRoll",      nullptr);
    ZilchFullBindMethod(builder, type, &Random::Probability, ZilchNoOverload, "Probability",  "probabilityOfTrue");
    ZilchFullBindMethod(builder, type, &Random::CoinFlip,    ZilchNoOverload, "CoinFlip",     nullptr);
    ZilchFullBindMethod(builder, type, &Random::Rotation,    ZilchNoOverload, "Rotation",     nullptr);

    // Bell curve (Gaussian) distribution
    ZilchFullBindMethod(builder, type, &Random::BellCurve, (float (Random::*)()),                    "BellCurve", nullptr);
    ZilchFullBindMethod(builder, type, &Random::BellCurve, (float (Random::*)(float, float)),        "BellCurve", "center, range");
    ZilchFullBindMethod(builder, type, &Random::BellCurve, (float (Random::*)(float, float, float)), "BellCurve", "center, range, standardDeviation");
  }

  //***************************************************************************
  Random::Random()
  {
    this->OriginalSeed = this->Generator.mSeed;
  }
  
  //***************************************************************************
  Random::Random(uint seed) :
    Generator(seed)
  {
    this->OriginalSeed = seed;
  }
  
  //***************************************************************************
  void Random::SetSeed(uint seed)
  {
    this->Generator = Math::Random(seed);
    this->OriginalSeed = seed;
  }
  
  //***************************************************************************
  uint Random::GetSeed()
  {
    return this->OriginalSeed;
  }
  
  //***************************************************************************
  int Random::GetMaxInteger()
  {
    return Math::Random::cRandMax;
  }
  
  //***************************************************************************
  bool Random::Boolean()
  {
    return this->Generator.IntRangeInIn(0, 1) == 1;
  }
  
  //***************************************************************************
  int Random::Integer()
  {
    return this->Generator.Next();
  }
  
  //***************************************************************************
  float Random::Real()
  {
    return this->Generator.Float();
  }

  //***************************************************************************
  double Random::DoubleReal()
  {
    return this->Generator.Double();
  }

  //***************************************************************************
  Math::Vector2 Random::UnitReal2()
  {
    return this->Generator.PointOnUnitCircle();
  }
  
  //***************************************************************************
  Math::Vector2 Random::Real2(float minLength, float maxLength)
  {
    return this->Generator.ScaledVector2(minLength, maxLength);
  }
  
  //***************************************************************************
  Math::Vector3 Random::UnitReal3()
  {
    return this->Generator.PointOnUnitSphere();
  }
  
  //***************************************************************************
  Math::Vector3 Random::Real3(float minLength, float maxLength)
  {
    return this->Generator.ScaledVector3(minLength, maxLength);
  }
  
  //***************************************************************************
  Zilch::Quaternion Random::Quaternion()
  {
    return this->Generator.RotationQuaternion();
  }
  
  //***************************************************************************
  int Random::RangeInclusiveMax(int min, int max)
  {
    return this->Generator.IntRangeInIn(min, max);
  }
  
  //***************************************************************************
  int Random::RangeExclusiveMax(int min, int max)
  {
    return this->Generator.IntRangeInEx(min, max);
  }
  
  //***************************************************************************
  int Random::Variance(int base, int variance)
  {
    return this->Generator.IntVariance(base, variance);
  }

  //***************************************************************************
  float Random::Range(float min, float max)
  {
    return this->Generator.FloatRange(min, max);
  }

  //***************************************************************************
  double Random::DoubleRange(double min, double max)
  {
    return this->Generator.DoubleRange(min, max);
  }

  //***************************************************************************
  float Random::Variance(float base, float variance)
  {
    return this->Generator.FloatVariance(base, variance);
  }
  
  //***************************************************************************
  double Random::Variance(double base, double variance)
  {
    return this->Generator.DoubleVariance(base, variance);
  }

  //***************************************************************************
  uint Random::DieRoll(uint sides)
  {
    return this->Generator.DieRoll(sides);
  }
  
  //***************************************************************************
  bool Random::Probability(float probOfTrue)
  {
    return this->Generator.Float() < probOfTrue;
  }
  
  //***************************************************************************
  bool Random::CoinFlip()
  {
    return this->Generator.IntRangeInIn(0, 1) == 1;
  }
  
  //***************************************************************************
  Zilch::Quaternion Random::Rotation()
  {
    return this->Generator.RotationQuaternion();
  }
  
  //***************************************************************************
  float Random::BellCurve()
  {
    return this->Generator.BellCurve(0.5f, 0.5f, 1.0f);
  }
  
  //***************************************************************************
  float Random::BellCurve(float center, float range)
  {
    return this->Generator.BellCurve(center, range, 1.0f);
  }
  
  //***************************************************************************
  float Random::BellCurve(float center, float range, float standardDeviation)
  {
    return this->Generator.BellCurve(center, range, standardDeviation);
  }
}
