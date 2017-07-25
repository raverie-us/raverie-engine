///////////////////////////////////////////////////////////////////////////////
///
/// \file MetaMath.hpp
/// Includes basic math classes Vector, Matrix, and Quaternion.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
#include "Math/MathImports.hpp"
typedef Math::EulerAnglesParam EulerAnglesParam;

struct Aabb;
struct Sphere;
}

namespace Zero
{

inline float Snap(float input, float snapping)
{
  if (snapping < Math::Epsilon())
    return input;

  float normalized = input / snapping;
  float rounded = Math::Floor(normalized + 0.5f) * snapping;
  return rounded;
}

inline Vec2 Snap(Vec2Param input, float snapping)
{
  Vec2 output;
  output.x = Snap(input.x ,snapping);
  output.y = Snap(input.y ,snapping);
  return output;
}

inline Vec3 Snap(Vec3Param input, float snapping)
{
  Vec3 output;
  output.x = Snap(input.x ,snapping);
  output.y = Snap(input.y ,snapping);
  output.z = Snap(input.z ,snapping);
  return output;
}

IntVec2 GetDefaultValue(IntVec2* dummy);
IntVec3 GetDefaultValue(IntVec3* dummy);
Vec2 GetDefaultValue(Vec2* dummy);
Vec3 GetDefaultValue(Vec3* dummy);
Vec4 GetDefaultValue(Vec4* dummy);
Quat GetDefaultValue(Quat* dummy);

void ToValue(StringRange range, IntVec2& value);
void ToValue(StringRange range, IntVec3& value);
void ToValue(StringRange range, IntVec4& value);
void ToValue(StringRange range, Vec2& value);
void ToValue(StringRange range, Vec3& value);
void ToValue(StringRange range, Vec4& value);
void ToValue(StringRange range, Quat& value);
void ToValue(StringRange range, Mat3& value);
void ToValue(StringRange range, Mat4& value);

template <typename T>
String MetaToString(const BoundType* type, const byte* data)
{
  T& value = *(T*)data;
  return ToString(value);
}

struct Ray;

}//namespace Zero
