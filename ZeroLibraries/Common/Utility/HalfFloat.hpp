///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Nathan Carlson, Andrew Colean
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "Typedefs.hpp"

namespace Zero
{

// Conversion from paper:
// Fast Half Float Conversions, Jeroen van der Zijp, November 2008
class HalfFloatConverter
{
public:
  /// Converts a 32-bit float (represented in IEEE 754 format) to a 16-bit "half" float
  static u16 ToHalfFloat(float float32);

  /// Converts a 16-bit "half" float to a 32-bit float (represented in IEEE 754 format)
  static float ToFloat(u16 float16);

private:
  /// Half float converter singleton instance
  static HalfFloatConverter& GetInstance();

  /// Constructor
  HalfFloatConverter();

  /// Initializes half float conversion tables
  void InitializeTables();

  /// Converts a subnormal representation to a normalized representation
  u32 ConvertMantissa(u32 i);

  /// Float to half-float base table
  u16 mBaseTable[512];

  /// Float to half-float shift table
  u8 mShiftTable[512];

  /// Half-float to float mantissa table
  u32 mMantissaTable[2048];

  /// Half-float to float exponent table
  u32 mExponentTable[64];

  /// Half-float to float offset table
  u16 mOffsetTable[64];
};

} // namespace Zero
