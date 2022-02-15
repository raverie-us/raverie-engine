// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(ActionDelay, builder, type)
{
  ZeroBindDocumented();

  ZilchBindField(mTimeLeft);
}

} // namespace Zero
