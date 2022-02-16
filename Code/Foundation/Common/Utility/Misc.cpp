// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Z
{
static u32 gLexicographicMask = 0xffffffff;
static u64 gLexicographicUpperMask = static_cast<u64>(gLexicographicMask) << 32;
static u64 gLexicographicLowerMask = static_cast<u64>(gLexicographicMask);
} // namespace Z

const TimeType cTimeMax = LONG_MAX;

u64 GetLexicographicId(u32 id1, u32 id2)
{
  u64 id = 0;

  // put the smaller number in the top 32 bits and the larger in the bottom 16
  if (id1 < id2)
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

u32 NextPowerOfTwo(u32 x)
{
  u32 leadingZeros = CountLeadingZeros(x);
  return 1 << (32 - leadingZeros);
}

} // namespace Zero

// Used for counting printf statement lengths
char gDiscardBuffer[2] = {0};
