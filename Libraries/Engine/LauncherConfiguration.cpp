// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(ShowDevelopChanged);
DefineEvent(ShowExperimentalBranchesChanged);
} // namespace Events

int LauncherConfig::mCurrentForcedUpdateVersionNumber = 1;
float LauncherConfig::mDefaultReloadFrequency = 60.0f * 60.0f;

ZilchDefineType(LauncherConfig, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZilchBindGetterSetterProperty(AutoRunMode);
}

LauncherConfig::LauncherConfig()
{
  mRunDebuggerMode = false;
  mRestartOnClose = false;
  mShowDevelopmentBuilds = false;
  mDisplayOnlyPreferredPlatform = true;
  mAutoCheckForLauncherUpdates = true;
  mShowExperimentalBranches = false;
  // Check every hour
  mAutoUpdateFrequencyInSeconds = mDefaultReloadFrequency;
}

void LauncherConfig::Serialize(Serializer& stream)
{
  SerializeNameDefault(mLauncherLocation, String());
  SerializeEnumNameDefault(
      LauncherAutoRunMode, mAutoRunMode, LauncherAutoRunMode::None);

  SerializeNameDefault(
      mDefaultProjectSaveLocation,
      FilePath::Combine(GetUserDocumentsDirectory(), "ZeroProjects"));
  SerializeNameDefault(
      mDownloadPath,
      FilePath::Combine(GetUserDocumentsDirectory(), "ZeroLauncher"));
  SerializeNameDefault(mDisplayBuildOnProjects, false);
  SerializeNameDefault(mShowDevelopmentBuilds, false);
  SerializeRename(mShowDevelopmentBuilds, "ShowNightlies");
  SerializeNameDefault(mAutoCheckForLauncherUpdates, true);
  SerializeNameDefault(mShowExperimentalBranches, false);
  SerializeNameDefault(mForcedUpdateVersion, 0);
  SerializeNameDefault(mAutoUpdateFrequencyInSeconds, mDefaultReloadFrequency);
  float everyTwelveHours = 60 * 60 * 12;
  SerializeNameDefault(mNewestLauncherUpdateCheckFrequency, everyTwelveHours);
}

void LauncherConfig::ApplyCommandLineArguments()
{
  String upgradeCommand =
      LauncherStartupArguments::Names[LauncherStartupArguments::Upgrade];
  bool upgrade = Environment::GetValue<bool>(upgradeCommand, false);
  if (upgrade == true)
    mAutoRunMode = LauncherAutoRunMode::None;

  // if we get the command argument to run then try to install and run
  String runCommand =
      LauncherStartupArguments::Names[LauncherStartupArguments::Run];
  bool run = Environment::GetValue<bool>(runCommand, false);
  if (run == true)
    mAutoRunMode = LauncherAutoRunMode::InstallAndRun;

  String debuggerModeCommand =
      LauncherStartupArguments::Names[LauncherStartupArguments::DebuggerMode];
  mRunDebuggerMode =
      Environment::GetValue<bool>(debuggerModeCommand, false);
}

uint LauncherConfig::GetAutoRunMode()
{
  return mAutoRunMode;
}

void LauncherConfig::SetAutoRunMode(uint mode)
{
  mAutoRunMode = mode;
}

String LauncherConfig::GetTemplateInstallPath()
{
  return FilePath::Combine(mDownloadPath, "Templates");
}

String LauncherConfig::GetBuildsInstallPath()
{
  return FilePath::Combine(mDownloadPath, "Builds");
}

ZilchDefineType(LauncherLegacySettings, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZilchBindField(mDisplayLegacyBuilds);

  type->AddAttribute(ObjectAttributes::cHidden);
  type->AddAttribute(ObjectAttributes::cCore);
}

LauncherLegacySettings::LauncherLegacySettings()
{
  mDisplayLegacyBuilds = true;
}

void LauncherLegacySettings::Serialize(Serializer& stream)
{
  SerializeNameDefault(mDisplayLegacyBuilds, true);
}

} // namespace Zero
