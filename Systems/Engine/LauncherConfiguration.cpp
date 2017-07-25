///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(ShowDevelopChanged);
  DefineEvent(ShowExperimentalBranchesChanged);
}//namespace Events

int LauncherConfig::mCurrentForcedUpdateVersionNumber = 1;

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
  mAutoCheckForMajorUpdates = true;
  mShowExperimentalBranches = false;
}

void LauncherConfig::Serialize(Serializer& stream)
{
  SerializeNameDefault(mLauncherLocation, String());
  SerializeEnumNameDefault(LauncherAutoRunMode, mAutoRunMode, LauncherAutoRunMode::None);

  SerializeNameDefault(mDefaultProjectSaveLocation, FilePath::Combine(GetUserDocumentsDirectory(), "ZeroProjects"));
  SerializeNameDefault(mDownloadPath, FilePath::Combine(GetUserDocumentsDirectory(), "ZeroLauncher"));
  SerializeNameDefault(mDisplayBuildOnProjects, false);
  SerializeNameDefault(mShowDevelopmentBuilds, false);
  SerializeRename(mShowDevelopmentBuilds, "ShowNightlies");
  SerializeNameDefault(mAutoCheckForMajorUpdates, true);
  SerializeNameDefault(mShowExperimentalBranches, false);
  SerializeNameDefault(mForcedUpdateVersion, 0);
}

void LauncherConfig::ApplyCommandLineArguments(const StringMap& arguments)
{
  String upgradeCommand = LauncherStartupArguments::Names[LauncherStartupArguments::Upgrade];
  bool upgrade = GetStringValue<bool>(arguments, upgradeCommand, false);
  if(upgrade == true)
    mAutoRunMode = LauncherAutoRunMode::None;

  //if we get the command argument to run then try to install and run
  String runCommand = LauncherStartupArguments::Names[LauncherStartupArguments::Run];
  bool run = GetStringValue<bool>(arguments, runCommand, false);
  if(run == true)
    mAutoRunMode = LauncherAutoRunMode::InstallAndRun;

  String debuggerModeCommand = LauncherStartupArguments::Names[LauncherStartupArguments::DebuggerMode];
  mRunDebuggerMode = GetStringValue<bool>(arguments, debuggerModeCommand, false);
}

void LauncherConfig::SaveToCache()
{
  CacheLauncherConfig(this, mCachedData);
}

void LauncherConfig::LoadFromCache()
{
  ReloadLauncherConfig(this, mCachedData);
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

//-------------------------------------------------------------------LauncherLegacySettings
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

void SaveLauncherConfig(Cog* launcherConfigCog)
{
  LauncherConfig* launcherConfig = launcherConfigCog->has(LauncherConfig);

  // We have to save to the legacy format for now because old versions of
  // zero try to open the launcher's config to find where the launcher is.
  Status status;
  ObjectSaver saver;
  saver.Open(status, launcherConfig->mSavePath.c_str());
  saver.SaveDefinition(launcherConfigCog);
}

void CopyOldConfigSettings(Cog* newConfigCog, uint configId)
{
  if(newConfigCog == nullptr)
    return;

  String documentsDirectory = GetUserDocumentsDirectory();
  String editorFolder = FilePath::Combine(documentsDirectory, "ZeroEditor");
  String previousConfigName = String::Format("ZeroLauncherConfigurationV%d", configId - 1);
  String previousConfigPath = FilePath::CombineWithExtension(editorFolder, previousConfigName, ".data");
  if(!FileExists(previousConfigPath))
    return;
  
  Cog* previousConfigCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), previousConfigPath, 0, nullptr);
  if(previousConfigCog == nullptr)
    return;
  
  // Copy all of their recent projects over
  RecentProjects* previousRecentProjects = previousConfigCog->has(RecentProjects);
  if(previousRecentProjects != nullptr)
    *HasOrAdd<RecentProjects>(newConfigCog) = *previousRecentProjects;
  
  // Copy their launcher settings over (where to save, auto-run mode, etc...)
  LauncherConfig* previousLauncherConfig = previousConfigCog->has(LauncherConfig);
  if(previousLauncherConfig != nullptr)
    *HasOrAdd<LauncherConfig>(newConfigCog) = *previousLauncherConfig;
}

Cog* LoadLauncherConfig(Cog* zeroConfigCog, const StringMap& arguments, bool overrideApplicationPath)
{
  bool usedDefault = false;
  uint configId = 1;
  String configName = String::Format("ZeroLauncherConfigurationV%d", configId);
  String documentsDirectory = GetUserDocumentsDirectory();
  String applicationDirectory = GetApplicationDirectory();
  String applicationPath = GetApplication();
  String editorFolder = FilePath::Combine(documentsDirectory, "ZeroEditor");
  String configPath = FilePath::CombineWithExtension(editorFolder, configName, ".data");

  Cog* configCog = nullptr;
  //try to create the config from the documents directory
  if(FileExists(configPath))
    configCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), configPath, 0, nullptr);

  //if we failed to create the config then try to load the old configuration file name
  if(configCog == nullptr)
  {
    usedDefault = true;

    String oldConfigName = String::Format("VersionSelectorConfigurationV%d", configId);
    String oldConfigPath = FilePath::CombineWithExtension(editorFolder, oldConfigName, ".data");
    if(FileExists(oldConfigPath))
      configCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), oldConfigPath, 0, nullptr);

    //if we failed to create the config then create an empty archetype that we'll auto-populate
    if(configCog == nullptr)
    {
      String defaultConfig = FilePath::Combine(applicationDirectory, "DefaultLauncherConfiguration.data");
      if(FileExists(defaultConfig))
        configCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), defaultConfig, 0, nullptr);
    }

    // Since we failed to find the current version config cog we had to create a new one.
    // In that case we'll lose old user settings during the transition so copy the relevant ones over.
    CopyOldConfigSettings(configCog, configId);
  }

  if(configCog == nullptr)
    return nullptr;

  // Force certain config components to exist. There's a few upgrade cases
  // where one of these could be missing otherwise.
  *HasOrAdd<TextEditorConfig>(configCog);
  *HasOrAdd<ContentConfig>(configCog);

  MainConfig* mainConfig = HasOrAdd<MainConfig>(configCog);
  mainConfig->ApplicationName = "ZeroLauncher";
  mainConfig->mConfigDidNotExist = false;
  mainConfig->DataDirectory = FilePath::Combine(applicationDirectory, "Data");
  if(zeroConfigCog != nullptr)
  {
    // Copy the application directory from the zero config (should never be null, but just in case)
    MainConfig* zeroMainConfig = zeroConfigCog->has(MainConfig);
    if(zeroMainConfig != nullptr)
    {
      mainConfig->ApplicationDirectory = zeroMainConfig->ApplicationDirectory;
      mainConfig->DataDirectory = zeroMainConfig->DataDirectory;
    }

    // For now steal a few configs from zero (these should eventually be added on their own)
    *HasOrAdd<TextEditorConfig>(configCog) = *zeroConfigCog->has(TextEditorConfig);
    *HasOrAdd<ContentConfig>(configCog) = *zeroConfigCog->has(ContentConfig);
  }
  
  //Build info is generated by the build process and if present the
  //engine was built locally.
  BuildInfo buildInfo;
  String buildInfoFile = FilePath::Combine(applicationDirectory, "BuildInfo.data");
  bool buildInfoExists = FileExists(buildInfoFile);
  if(buildInfoExists)
  {
    //Engine was built locally so pull data from the source location
    LoadFromDataFile(buildInfo, buildInfoFile);

    mainConfig->SourceDirectory = buildInfo.Source;
    mainConfig->DataDirectory = FilePath::Combine(buildInfo.Source, "Data");
    mainConfig->mLocallyBuilt = true;

    ContentConfig* contentConfig = HasOrAdd<ContentConfig>(configCog);
    contentConfig->ToolsDirectory = FilePath::Combine(buildInfo.Source, "Tools");
  }

  LauncherConfig* versionConfig = HasOrAdd<LauncherConfig>(configCog);
  versionConfig->mSavePath = configPath;
  if(overrideApplicationPath)
    versionConfig->mLauncherLocation = applicationPath;

  //save the cache before any command line arguments get set
  versionConfig->SaveToCache();
  //apply any command line arguments (mostly auto-run settings)
  versionConfig->ApplyCommandLineArguments(arguments);

  RecentProjects* recentProjects = configCog->has(RecentProjects);
  //if we didn't already have a recent projects then try to copy it over from the zero engine config
  if(recentProjects == nullptr)
  {
    recentProjects = HasOrAdd<RecentProjects>(configCog);
    //if we didn't get a valid config object then don't do anything
    if(zeroConfigCog != nullptr)
    {
      RecentProjects* zeroRecentProjects = zeroConfigCog->has(RecentProjects);
      if(zeroRecentProjects != nullptr)
        recentProjects->CopyProjects(zeroRecentProjects);
      //hardcode to 20 projects initially (since we just created the recent projects)
      recentProjects->mMaxRecentProjects = 20;
    }
  }

  if(usedDefault)
    SaveLauncherConfig(configCog);

  return configCog;
}

void CacheLauncherConfig(LauncherConfig* config, String& cachedData)
{
  TextSaver saver;
  saver.OpenBuffer();
  config->Serialize(saver);
  cachedData = saver.GetString();
}

void ReloadLauncherConfig(LauncherConfig* config, String& cachedData)
{
  DataTreeLoader loader;
  Status status;
  loader.OpenBuffer(status, cachedData);
  config->Serialize(loader);
}

}//namespace Zero
