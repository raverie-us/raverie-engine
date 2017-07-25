///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016 DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Engine/EngineStandard.hpp"
#include "Editor/EditorStandard.hpp"

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

}//namespace Zero

#include "ZilchScript.hpp"
#include "ZilchZero.hpp"
#include "ZilchPlugin.hpp"
