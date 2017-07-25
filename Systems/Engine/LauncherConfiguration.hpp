///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Cog;

/// To try to avoid accidentally changing command-line arguments for the launcher,
/// some of the common ones (ones used for communication) are added here so
/// they're shared between the engine and so on.
DeclareEnum8(LauncherStartupArguments, New, Open, Run, InstallAndRun, Upgrade, Projects, Tags,
  DebuggerMode /*Launcher in a special way so that the launcher gets extra buttons where it will try to communicate back to the active launcher*/
  );

DeclareEnum3(LauncherAutoRunMode, None, IfInstalled, InstallAndRun);

namespace Events
{
  DeclareEvent(ShowDevelopChanged);
  DeclareEvent(ShowExperimentalBranchesChanged);
}//namespace Events

/// Configuration data for the version selector
class LauncherConfig : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  LauncherConfig();
  void Serialize(Serializer& stream);

  void SaveToCache();
  void LoadFromCache();

  void ApplyCommandLineArguments(const StringMap& arguments);

  /// Will the version selector auto run? Will it install it if it wasn't installed?
  uint GetAutoRunMode();
  void SetAutoRunMode(uint mode);

  String GetTemplateInstallPath();
  String GetBuildsInstallPath();

  LauncherAutoRunMode::Type mAutoRunMode;

  /// The location that the launcher last ran.
  /// Used so zero can find out how to call the version selector.
  String mLauncherLocation;

  /// Where is the config saved to
  String mSavePath;
  /// The root directory where templates, versions, etc... are installed to
  String mDownloadPath;
  String mDefaultProjectSaveLocation;

  /// Whether or not to display the build and build state on each
  /// project in the Recent Projects page.
  bool mDisplayBuildOnProjects;
  bool mAutoCheckForMajorUpdates;
  bool mShowDevelopmentBuilds;
  bool mShowExperimentalBranches;
  bool mRunDebuggerMode;
  /// Should the launcher only show the preferred (current) platform builds or show all.
  /// Hardcoded to true for now since we only have one platform.
  bool mDisplayOnlyPreferredPlatform;

  /// Number used to force the launcher to reinstall all versions
  int mForcedUpdateVersion;
  static int mCurrentForcedUpdateVersionNumber;

  String mCachedData;

  // Should the launcher restart when it closes? Used to have
  // the launcher restart in order to update.
  bool mRestartOnClose;
};

//-------------------------------------------------------------------LauncherLegacySettings
class LauncherLegacySettings : public Component
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  LauncherLegacySettings();
  void Serialize(Serializer& stream);

  bool mDisplayLegacyBuilds;
};

void SaveLauncherConfig(Cog* launcherConfigCog);
Cog* LoadLauncherConfig(Cog* zeroConfigCog, const StringMap& arguments, bool overrideApplicationPath = false);

void CacheLauncherConfig(LauncherConfig* config, String& cachedData);
void ReloadLauncherConfig(LauncherConfig* config, String& cachedData);

}//namespace Zero
