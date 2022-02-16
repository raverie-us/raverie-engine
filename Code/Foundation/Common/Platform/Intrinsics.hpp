// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

/// Returns number of least significant zeros
/// If x is strictly a power of 2, will result in n where 2^n=x, values [0, 31]
/// More information: http://en.wikipedia.org/wiki/Find_first_set
u32 CountTrailingZeros(u32 x);
/// Returns number of most significant zeros
u32 CountLeadingZeros(u32 x);

/// These versions can be called for platform implementations without the
/// intrinsic.
u32 CountTrailingZerosNonIntrinsic(u32 x);
u32 CountLeadingZerosNonIntrinsic(u32 x);

} // namespace Zero
