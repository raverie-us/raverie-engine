// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(ActionDelay, builder, type)
{
  RaverieBindDocumented();

  RaverieBindField(mTimeLeft);
}

} // namespace Raverie
