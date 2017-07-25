///////////////////////////////////////////////////////////////////////////////
///
/// \file Noise.cpp
/// Perlin Noise Functions
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

//Perlin Noise Functions from Perlin's website

namespace Zero
{

//1d Noise [-1, 1]
float Noise(int x)
{
  x = (x << 13) ^ x;
  return (1.0f - ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

//2d Noise [-1, 1]
float Noise(int x, int y)
{
  int n;
  n = x + y * 57;
  n = (n<<13) ^ n;
  float res = (float)(1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff ) / 1073741824.0);
  return res;
}

//1d Smooth Noise [-1, 1]
float SmoothNoise(int x)
{
  return Noise(x) / 2 + Noise(x - 1) / 4 + Noise(x + 1) / 4;
}

//2d Smooth Noise [-1, 1]
float SmoothNoise(int x, int y)
{
  float corners = (Noise(x - 1, y - 1) + Noise(x + 1, y - 1) + 
                   Noise(x - 1, y + 1) + Noise(x + 1, y + 1)) / 16;
  float sides   = (Noise(x - 1, y) + Noise(x + 1, y) + Noise(x, y - 1) + Noise(x, y + 1)) /  8;
  float center  = Noise(x, y) / 4.0f;
  return corners + sides + center;
}

//Linear Interpolated Noise [-1, 1]
float InterpolatedNoise(float x)
{
  int intX   = int(x);
  float fractX = x - intX;
  float v1 = SmoothNoise(intX);
  float v2 = SmoothNoise(intX+1);
  return  Math::Lerp(v1 , v2 , fractX);
}

//Bilinear Interpolated Noise [-1, 1]
float InterpolatedNoise(float x, float y)
{
  int intX = int(x);
  float fractX = x - intX;

  int intY = int(y);
  float fractY = y - intY;

  if(x < 0.0f && fractX <= 0.0f)
  {
    --intX;
    fractX += 1.0f;
  }

  if(y < 0.0f && fractY <= 0.0f)
  {
    --intY;
    fractY += 1.0f;
  }

  float v1 = SmoothNoise(intX,     intY);
  float v2 = SmoothNoise(intX + 1, intY);
  float v3 = SmoothNoise(intX,     intY + 1);
  float v4 = SmoothNoise(intX + 1, intY + 1);

  float i1 = Math::Lerp(v1 , v2, fractX);
  float i2 = Math::Lerp(v3 , v4, fractX);

  return Math::Lerp(i1, i2, fractY);
}

const float cPersistence = 0.6539856f;
//const float cPersistence = 0.65f;
const float cFrequency = 0.05024735687f;
const int cOctaves = 8;

float MaxRange(float persistence, float frequency)
{
  float total = 0;
  for(int i = 0; i < cOctaves; ++i)
  {
    float amplitude = Math::Pow(persistence , float(i));
    total += 1.0f * amplitude;
  }
  return total;
}

float PerlinNoise(float persistence, float frequency, float limit, float x)
{
  float total = 0;
  for(int i = 0; i < cOctaves; ++i)
  {
    float fi = float(i);
    float f = Math::Pow(2.0f, fi) * frequency;
    float amplitude = Math::Pow(persistence, fi);
    total += InterpolatedNoise(limit * i + x * f) * amplitude;
  }
  return total;
}

float PerlinNoise(float persistence, float frequency, float limit, float x, float y)
{
  float total = 0;
  for(int i = 0; i < cOctaves; ++i)
  {
    float fi = float(i);
    float f = Math::Pow(2.0f, fi) * frequency;
    float amplitude = Math::Pow(persistence, fi);
    total += InterpolatedNoise(limit * i + x * f, limit * i + y * f) * amplitude;
  }
  return total;
}

float PerlinNoise(float x)
{
  float total = 0;
  for(int i = 0; i < cOctaves; ++i)
  {
    float f = (2 * i) * cFrequency;
    float amplitude = cPersistence * i;
    total += InterpolatedNoise(x * f) * amplitude;
  }
  return total;
}


float PerlinNoise(float x, float y)
{
  int seed = 7436340;
  Math::Random random(seed);
  float total = 0.0f;

  for(int i = 0; i < cOctaves; ++i)
  {
    float frequency = Math::Pow(2.0f, (float)i);
    float amplitude = Math::Pow(cPersistence, (float)i);

    float dx = int(random.Next()) * 0.1f;
    float dy = int(random.Next()) * 0.1f;

    total += InterpolatedNoise(x * frequency + dx, y * frequency + dy) * amplitude;
  }
  return total;
}

//float PerlinNoise(float x, float y)
//{
//  float total = 0.0f;
//
//  for(int i=0;i<cOctaves;++i)
//  {
//    float f = (2 * i)*cFrequency;
//    float amplitude = cPersistence * i;
//    total += InterpolatedNoise(x * f, y * f) * amplitude;
//  }
//  return total;
//}

}
