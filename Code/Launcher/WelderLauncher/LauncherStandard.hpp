// MIT Licensed (see LICENSE.md).
#pragma once

#include "Systems/Startup/StartupStandard.hpp"

namespace Zero
{
extern const String mLauncherRegularFont;
extern const String mLauncherBoldFont;

class ZeroNoImportExport LauncherLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(LauncherLibrary, "ZeroEngine");

  static void Initialize();
  static void Shutdown();
};

} // namespace Zero

#include "ForwardDeclarations.hpp"

#include "Standalone.hpp"
#include "VersionSelector.hpp"
#include "LauncherComponents.hpp"

#include "DeveloperNotes.hpp"
#include "Launcher.hpp"
#include "LauncherTasks.hpp"
#include "LauncherWindow.hpp"
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

#include "LauncherStartup.hpp"
#include "LauncherCrashCallbacks.hpp"
