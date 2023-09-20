// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

u32 CountTrailingZeros(u32 x)
{
  return CountTrailingZerosNonIntrinsic(x);
}

u32 CountLeadingZeros(u32 x)
{
  return CountLeadingZerosNonIntrinsic(x);
}

} // namespace Raverie
