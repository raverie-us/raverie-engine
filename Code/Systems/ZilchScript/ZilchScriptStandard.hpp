// MIT Licensed (see LICENSE.md).
#pragma once

#include "Systems/Engine/EngineStandard.hpp"

#include "ZilchScript.hpp"
#include "ZilchZero.hpp"
#include "ZilchPlugin.hpp"

#include "Editor/Editor/EditorStandard.hpp"

namespace Zero
{
// Forward declarations
class ZilchPluginLibrary;

// Zilch Script library
class ZeroNoImportExport ZilchScriptLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(ZilchScriptLibrary, "ZeroEngine");

  static void Initialize();
  static void Shutdown();
};

} // namespace Zero
