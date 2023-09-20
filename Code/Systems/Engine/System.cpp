// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(System, builder, type)
{
  // Systems exist for the lifetime of any possible scripts that could access
  // them, so it is ok to point directly at them instead of using a safe handle.
  type->HandleManager = RaverieManagerId(PointerManager);
}

} // namespace Raverie
