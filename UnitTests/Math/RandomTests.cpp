///////////////////////////////////////////////////////////////////////////////
///
///  \file RandomTests.cpp
///  Unit tests for the Random library.
///  
///  Authors: Benjamin Strukus
///  Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"

typedef signed    __int64 s64;

typedef unsigned int uint;
#include "Math/Random.hpp"

using namespace Math;

#define DoRandomTestsX
#ifdef DoRandomTests

TEST(FastRandom_Float)
{
  printf("-----------------------------------------------\n");
  Math::SeedFastRandFloat(123456.0f);
  for(int i = 0; i < 200; ++i)
  {
    float result = Math::FastRandFloat();
    if((i + 1) % 5)
    {
      printf("%f | ", result);
    }
    else
    {
      printf("%f\n", result);
    }    
  }
}

TEST(FastRandomFloat_PointOnUnitSphere)
{
  printf("-----------------------------------------------\n");
  FastRandomFloat random(8675309.0f);

  for(uint i = 0; i < 100; ++i)
  {
    Vector3 randomVec = random.RandPointOnUnitSphere();
    real length = Length(randomVec);
    if((i + 1) % 5)
    {
      printf("%f | ", length);
    }
    else
    {
      printf("%f\n", length);
    } 
  }
}

TEST(FastRandomFloat_PointOnUnitCircle)
{
  printf("-----------------------------------------------\n");
  FastRandomFloat random(8675309.0f);

  for(uint i = 0; i < 100; ++i)
  {
    Vector2 randomVec = random.RandPointOnUnitCircle();
    real length = Length(randomVec);
    if((i + 1) % 5)
    {
      printf("%f | ", length);
    }
    else
    {
      printf("%f\n", length);
    } 
  }
}

TEST(FastRandomFloat_RandomQuaternion)
{
  printf("-----------------------------------------------\n");
  FastRandomFloat random(8675309.0f);

  for(uint i = 0; i < 100; ++i)
  {
    Quaternion randomQuat = random.RandRotationQuaternion();
    real length = Length(randomQuat);
    if((i + 1) % 5)
    {
      printf("%f | ", length);
    }
    else
    {
      printf("%f\n", length);
    } 
  }
}

TEST(FastRandomFloat_RandomRotationMatrix)
{
  printf("-----------------------------------------------\n");
  FastRandomFloat random(8675309.0f);

  Matrix3 rotationMatrix;
  for(uint i = 0; i < 100; ++i)
  {
    random.RandRotationMatrix(&rotationMatrix);
    real dot = Dot(rotationMatrix.BasisX(), rotationMatrix.BasisY()) + 
               Dot(rotationMatrix.BasisX(), rotationMatrix.BasisZ()) + 
               Dot(rotationMatrix.BasisZ(), rotationMatrix.BasisY());
    if((i + 1) % 5)
    {
      printf("%f | ", dot);
    }
    else
    {
      printf("%f\n", dot);
    } 
  }
}

TEST(Random_Uint)
{
  Random rand;
  unsigned int a = 0;
  while(a != unsigned int(-1))
  {
    a = rand.Uint();
  }
  printf("Done\n");
}

TEST(Random_IntRange)
{
  Random rand;
  int r = 0;
  const int min = -3;
  const int max = 3;
  while(r != max)
  {
    r = rand.IntRange(min, max);
  }
  printf("Done\n");
}

#endif
