// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

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

} // namespace Zero
