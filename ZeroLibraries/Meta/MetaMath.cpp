///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

IntVec2 GetDefaultValue(IntVec2* dummy){return IntVec2::cZero;}
IntVec3 GetDefaultValue(IntVec3* dummy){return IntVec3::cZero;}
IntVec4 GetDefaultValue(IntVec4* dummy){return IntVec4::cZero;}
Vec2 GetDefaultValue(Vec2* dummy){return Vec2::cZero;}
Vec3 GetDefaultValue(Vec3* dummy){return Vec3::cZero;}
Vec4 GetDefaultValue(Vec4* dummy){return Vec4(1,1,1,1);}
Quat GetDefaultValue(Quat* dummy){return Quat::cIdentity;}
Mat3 GetDefaultValue(Mat3* dummy){return Mat3::cIdentity;}
Mat4 GetDefaultValue(Mat4* dummy){return Mat4::cIdentity;}

// String Parsing for Vector Types
template<int size, typename subtype>
void ToValue(StringRange range, subtype* values)
{
  // Read as CSV
  StringTokenRange tokens(range, ',');

  uint i = 0;
  for(; !tokens.Empty() && i < size; ++i)
  {
    ToValue(tokens.Front(), values[i]);
    tokens.PopFront();
  }

  //Default 
  for(;i < size; ++i)
    values[i] = 0;
}

void ToValue(StringRange range, Vec2& value){ToValue<2>(range, value.array);}
void ToValue(StringRange range, Vec3& value){ToValue<3>(range, value.array);}
void ToValue(StringRange range, Vec4& value){ToValue<4>(range, value.array);}
void ToValue(StringRange range, Quat& value){ToValue<4>(range, &value.x);}
void ToValue(StringRange range, IntVec2& value){ToValue<2>(range, &value.x);}
void ToValue(StringRange range, IntVec3& value){ToValue<3>(range, &value.x);}
void ToValue(StringRange range, IntVec4& value){ToValue<4>(range, &value.x);}
void ToValue(StringRange range, Mat3& value){ToValue<9>(range, value.array);}
void ToValue(StringRange range, Mat4& value){ToValue<16>(range, value.array);}

} // namespace Zero
