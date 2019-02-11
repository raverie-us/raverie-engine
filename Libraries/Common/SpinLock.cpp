// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
void SpinLock::Lock()
{
  while (!mLocked.CompareExchange(true, false))
    ;
}

void SpinLock::Unlock()
{
  mLocked = false;
}
} // namespace Zero
