///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Josh Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Platform/Windows/Windows.hpp"
//because win32 is a jerk...
#undef LoadMenu

#include "LauncherDllStandard.hpp"

// Needed for DllMain
#include "WindowsShell/WindowsSystem.hpp"
#include "WindowsShell/WinUtility.hpp"
#include "Platform/CommandLineSupport.hpp"

#include "Engine/BuildVersion.hpp"

#include "LauncherStartup.hpp"
#include "LauncherCrashCallbacks.hpp"

namespace Zero
{
  extern const String mLauncherRegularFont;
  extern const String mLauncherBoldFont;
}// namespace Zero

