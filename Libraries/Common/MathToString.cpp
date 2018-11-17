///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

template<int size, typename subtype>
String ToStringArray(subtype* values, bool shortFormat = false)
{
  StringBuilder builder;
  for(uint i = 0; i < size; ++i)
  {
    if(i != 0) builder.Append(", ");
    builder.Append(ToString(values[i], shortFormat));
  }
  return builder.ToString();
}

String ToString(Math::IntVec2Param value, bool shortFormat) { return ToStringArray<2>(&value.x, shortFormat); }
String ToString(Math::IntVec3Param value, bool shortFormat) { return ToStringArray<3>(&value.x, shortFormat); }
String ToString(Math::IntVec4Param value, bool shortFormat) { return ToStringArray<4>(&value.x, shortFormat); }
String ToString(Math::Vec2Param value, bool shortFormat) { return ToStringArray<2>(&value.x, shortFormat); }
String ToString(Math::Vec3Param value, bool shortFormat) { return ToStringArray<3>(&value.x, shortFormat); }
String ToString(Math::Vec4Param value, bool shortFormat) { return ToStringArray<4>(&value.x, shortFormat); }
String ToString(Math::QuatParam value, bool shortFormat) { return ToStringArray<4>(&value.x, shortFormat); }
String ToString(Math::Mat2Param value, bool shortFormat) { return ToStringArray<4>(value.array, shortFormat); }
String ToString(Math::Mat3Param value, bool shortFormat) { return ToStringArray<9>(value.array, shortFormat); }
String ToString(Math::Mat4Param value, bool shortFormat) { return ToStringArray<16>(value.array, shortFormat); }

}//namespace Zero
