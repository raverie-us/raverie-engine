///////////////////////////////////////////////////////////////////////////////
///
/// \file Misc.cpp
/// Miscellaneous functions.
///
/// Authors: 
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#if _MSC_VER
#include <intrin.h>
#endif  

namespace Zero
{

namespace Z
{
static u32 gLexicographicMask = 0xffffffff;
static u64 gLexicographicUpperMask = static_cast<u64>(gLexicographicMask) << 32;
static u64 gLexicographicLowerMask = static_cast<u64>(gLexicographicMask);
}//namespace Z

const TimeType cTimeMax = LONG_MAX;

u64 GetLexicographicId(u32 id1, u32 id2)
{
  u64 id = 0;

  //put the smaller number in the top 32 bits and the larger in the bottom 16
  if(id1 < id2)
  {
    id |= (static_cast<u64>(id1) & Z::gLexicographicMask) << 32;
    id |= (static_cast<u64>(id2) & Z::gLexicographicMask);
  }
  else
  {
    id |= (static_cast<u64>(id2) & Z::gLexicographicMask) << 32;
    id |= (static_cast<u64>(id1) & Z::gLexicographicMask);
  }

  /*could also do
  u32* start = reinterpret_cast<u32*>(&id);
  if(id1 < id2)
  {
    start[0] = id2;
    start[1] = id1;
  }
  else
  {
    start[0] = id1;
    start[1] = id2;
  }
  although endianness matters, which would only screw up if
  sending a pair id from one machine to another
  */

  return id;
}

void UnPackLexicographicId(u32& id1, u32& id2, u64 pairId)
{
  id1 = static_cast<u32>(pairId & Z::gLexicographicLowerMask);
  id2 = static_cast<u32>((pairId & Z::gLexicographicUpperMask) >> 32);

  /*could also do 
  u32* start = reinterpret_cast<u32*>(&pairId);
  id1 = *start;
  id2 = *(start + 1);*/
}

bool IsBigEndian()
{
  int i = 1;
  byte* lowByte = (byte*)&i;

  return (*lowByte == 0);
}

#if _MSC_VER

#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)

u32 CountTrailingZeros(u32 x)
{
  unsigned long ret = 32;
  _BitScanForward(&ret, x);
  return ret;
}

u32 CountLeadingZeros(u32 x)
{
  unsigned long ret = 32;
  _BitScanReverse(&ret, x);
  return 31 - ret;
}

#else

static const u32 CtzLookupTable[] =
{
  8, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
  4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
};

u32 CountTrailingZeros(u32 x)
{
  u32 n = 0;
  if ((x & 0x0000FFFF) == 0) { n += 16; x >>= 16; }
  if ((x & 0x000000FF) == 0) { n +=  8; x >>=  8; }
  // if ((x & 0x0000000F) == 0) { n +=  4; x >>=  4; }
  // if ((x & 0x00000003) == 0) { n +=  2; x >>=  2; }
  // if ((x & 0x00000001) == 0) { n +=  1; }
  return n + CtzLookupTable[x & 0xFF];
}

u32 CountLeadingZeros(u32 x)
{
  u32 n = 0;
  if ((x & 0xFFFF0000) == 0) { n += 16; x <<= 16; }
  if ((x & 0xFF000000) == 0) { n +=  8; x <<=  8; }
  if ((x & 0xF0000000) == 0) { n +=  4; x <<=  4; }
  if ((x & 0xC0000000) == 0) { n +=  2; x <<=  2; }
  if ((x & 0x80000000) == 0) { n +=  1; }
  return n;
}

#endif

u32 NextPowerOfTwo(u32 x)
{
  u32 leadingZeros = CountLeadingZeros(x);
  return 1 << (32 - leadingZeros);
}

}//namespace Zero

// Used for counting printf statement lengths
char gDiscardBuffer[2] = {0};
