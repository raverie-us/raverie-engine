///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//--------------------------------------------------------------------- Launcher
class Launcher
{
public:
  /// Constructor.
  Launcher(Cog* configCog, StringMap& arguments);

  void Initialize();

  /// Handles logic for making sure the newest Eula is accepted before opening
  /// the launcher itself.
  void Startup();

  /// The Eula window should call this function if the Eula is accepted.
  /// This will store that the Eula was accepted and start the launcher.
  void EulaAccepted();

private:
  void OpenEulaWindow();
  void OpenLauncherWindow();
  void OpenTweakablesWindow();

  TimeType GetEulaDateTime();
  bool ShouldOpenEula();
  MainWindow* CreateOsWindow(Cog* configCog, const IntVec2& minWindowSize,
                             StringParam windowName, bool mainWindow,
                             bool visible);

  OsWindow* mOsWindow;
  MainWindow* mMainWindow;
  Composite* mEulaWindow;
  Composite* mLauncherWindow;
  Cog* mConfigCog;
  StringMap mArguments;

  static IntVec2 mEulaWindowSize;
  static IntVec2 mLauncherWindowSize;
};

// Global Access
namespace Z
{
  extern Launcher* gLauncher;
}

}//namespace Zero
