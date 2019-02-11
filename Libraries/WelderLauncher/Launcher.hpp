// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

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

  size_t GetEulaHash();
  bool ShouldOpenEula();

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

} // namespace Zero
