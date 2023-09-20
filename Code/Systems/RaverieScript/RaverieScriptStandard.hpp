// MIT Licensed (see LICENSE.md).
#pragma once

#include "Systems/Engine/EngineStandard.hpp"

#include "RaverieScript.hpp"
#include "RaverieBase.hpp"

namespace Raverie
{
// Raverie Script library
class RaverieScriptLibrary : public Raverie::StaticLibrary
{
public:
  RaverieDeclareStaticLibraryInternals(RaverieScriptLibrary);

  static void Initialize();
  static void Shutdown();
};

} // namespace Raverie
