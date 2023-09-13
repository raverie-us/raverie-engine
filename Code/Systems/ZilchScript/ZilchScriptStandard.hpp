// MIT Licensed (see LICENSE.md).
#pragma once

#include "Systems/Engine/EngineStandard.hpp"

#include "ZilchScript.hpp"
#include "ZilchZero.hpp"

namespace Zero
{
// Zilch Script library
class ZilchScriptLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(ZilchScriptLibrary);

  static void Initialize();
  static void Shutdown();
};

} // namespace Zero
