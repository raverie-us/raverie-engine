///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

String ToString(Math::IntVec2Param value, bool shortFormat = false);
String ToString(Math::IntVec3Param value, bool shortFormat = false);
String ToString(Math::IntVec4Param value, bool shortFormat = false);
String ToString(Math::Vec2Param value, bool shortFormat = false);
String ToString(Math::Vec3Param value, bool shortFormat = false);
String ToString(Math::Vec4Param value, bool shortFormat = false);
String ToString(Math::QuatParam value, bool shortFormat = false);
String ToString(Math::Mat2Param value, bool shortFormat = false);
String ToString(Math::Mat3Param value, bool shortFormat = false);
String ToString(Math::Mat4Param value, bool shortFormat = false);

}//namespace Zero
