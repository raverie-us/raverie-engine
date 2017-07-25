///////////////////////////////////////////////////////////////////////////////
///
/// \file WindowsShellSystemStandard.hpp
/// 
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Dependencies
#include "Engine/EngineStandard.hpp"
#include "Platform/PlatformStandard.hpp"

namespace Zero
{

// WindowsShellSystem library
class ZeroNoImportExport WindowsShellSystemLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(WindowsShellSystemLibrary, "ZeroEngine");

  static void Initialize();
  static void Shutdown();
};

}//namespace Zero

// Internals
#include "RcInput.hpp"
#include "RawInput.hpp"
#include "WindowsShell.hpp"
