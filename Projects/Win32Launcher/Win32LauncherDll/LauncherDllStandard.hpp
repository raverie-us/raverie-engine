///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Startup/StartupStandard.hpp"

namespace Zero
{

// LauncherDll library
class ZeroNoImportExport LauncherDllLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(LauncherDllLibrary, "ZeroEngine");

  static void Initialize();
  static void Shutdown();
};

}//namespace Zero

#include "ForwardDeclarations.hpp"

#include "Standalone.hpp"
#include "VersionSelector.hpp"
#include "LauncherComponents.hpp"

#include "DeveloperNotes.hpp"
#include "Eula.hpp"
#include "Launcher.hpp"
#include "LauncherTasks.hpp"
#include "LauncherWindow.hpp"
#include "MiscHelpers.hpp"
#include "ProjectCache.hpp"
#include "TagFiltering.hpp"


// Widgets
#include "BuildStatus.hpp"

// Menus
#include "ActiveProjectMenu.hpp"
#include "BuildsMenu.hpp"
#include "DiscoverMenu.hpp"
#include "NewProjectMenu.hpp"
#include "RecentProjectsMenu.hpp"
#include "SettingsMenu.hpp"

#include "ExtraModals.hpp"
