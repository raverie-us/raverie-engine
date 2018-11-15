///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Nathan Carlson, Andrew Colean
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

// Includes
#include "Precompiled.hpp"
#include "HalfFloat.hpp"

namespace Zero
{

u16 HalfFloatConverter::ToHalfFloat(float float32)
{
  HalfFloatConverter& converter = HalfFloatConverter::GetInstance();

  u32 f = *(u32*)(void*)&float32;
  return converter.mBaseTable[(f >> 23) & 0x1FF] + ((f & 0x007FFFFF) >> converter.mShiftTable[(f >> 23) & 0x1FF]);
}

float HalfFloatConverter::ToFloat(u16 float16)
{
  HalfFloatConverter& converter = HalfFloatConverter::GetInstance();

  u16 h = float16;
  u32 f = converter.mMantissaTable[converter.mOffsetTable[(h >> 10)] + (h & 0x3FF)] + converter.mExponentTable[(h >> 10)];
  return *(float*)(void*)&f;
}

HalfFloatConverter& HalfFloatConverter::GetInstance()
{
  static HalfFloatConverter sInstance;
  return sInstance;
}

HalfFloatConverter::HalfFloatConverter()
{
  InitializeTables();
}

void HalfFloatConverter::InitializeTables()
{
  // Initialize float to half-float tables
  for (uint i = 0; i < 256; ++i)
  {
    int e = i - 127;

    // Very small numbers map to zero
    if (e < -24)
    {
      mBaseTable[i | 0x000] = 0x0000;
      mBaseTable[i | 0x100] = 0x8000;
      mShiftTable[i | 0x000] = 24;
      mShiftTable[i | 0x100] = 24;
    }
    // Small numbers map to denorms
    else if (e < -14)
    {
      mBaseTable[i | 0x000] = (0x0400 >> (-e - 14));
      mBaseTable[i | 0x100] = (0x0400 >> (-e - 14)) | 0x8000;
      mShiftTable[i | 0x000] = -e - 1;
      mShiftTable[i | 0x100] = -e - 1;
    }
    // Normal numbers just lose precision
    else if (e <= 15)
    {
      mBaseTable[i | 0x000] = ((e + 15) << 10);
      mBaseTable[i | 0x100] = ((e + 15) << 10) | 0x8000;
      mShiftTable[i | 0x000] = 13;
      mShiftTable[i | 0x100] = 13;
    }
    // Large numbers map to Infinity
    else if (e < 128)
    {
      mBaseTable[i | 0x000] = 0x7C00;
      mBaseTable[i | 0x100] = 0xFC00;
      mShiftTable[i | 0x000] = 24;
      mShiftTable[i | 0x100] = 24;
    }
    // Infinity and NaN's stay Infinity and NaN's
    else
    {
      mBaseTable[i | 0x000] = 0x7C00;
      mBaseTable[i | 0x100] = 0xFC00;
      mShiftTable[i | 0x000] = 13;
      mShiftTable[i | 0x100] = 13;
    }
  }

  // Initialize half-float to float mantissa table
  mMantissaTable[0] = 0;

  for (uint i = 1; i < 1024; ++i)
    mMantissaTable[i] = ConvertMantissa(i);

  for (uint i = 1024; i < 2048; ++i)
    mMantissaTable[i] = 0x38000000 + ((i - 1024) << 13);

  // Initialize half-float to float exponent table
  mExponentTable[0] = 0;

  for (uint i = 1; i < 31; ++i)
    mExponentTable[i] = (i << 23);

  mExponentTable[31] = 0x47800000;
  mExponentTable[32] = 0x80000000;

  for (uint i = 33; i < 63; ++i)
    mExponentTable[i] = 0x80000000 + ((i - 32) << 23);

  mExponentTable[63] = 0xC7800000;

  // Initialize half-float to float offset table
  mOffsetTable[0] = 0;

  for (uint i = 1; i < 32; ++i)
    mOffsetTable[i] = 1024;

  mOffsetTable[32] = 0;

  for (uint i = 33; i < 64; ++i)
    mOffsetTable[i] = 1024;
}

u32 HalfFloatConverter::ConvertMantissa(u32 i)
{
  // Zero pad mantissa bits
  u32 m = i << 13;

  // Zero exponent
  u32 e = 0;

  // While not normalized
  while(!(m & 0x00800000))
  {
    // Decrement exponent (1 << 23)
    e -= 0x00800000;

    // Shift mantissa
    m <<= 1;
  }

  // Clear leading 1 bit
  m &= ~0x00800000;

  // Adjust bias ((127 - 14) << 23)
  e += 0x38800000;

  // Return combined number
  return (m | e);
}

} // namespace Zero
