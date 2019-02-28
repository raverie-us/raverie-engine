// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(RecentProjectsUpdated);
} // namespace Events

bool MainConfig::sConfigCanSave = true;

ZilchDefineType(MainConfig, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZilchBindGetterProperty(BuildDate);
  ZilchBindGetterProperty(BuildVersion);
  type->AddAttribute(ObjectAttributes::cCore);
}

void MainConfig::Initialize(CogInitializer& initializer)
{
  mSave = true;
}

String MainConfig::GetBuildDate()
{
  return GetChangeSetDateString();
}

String MainConfig::GetBuildVersion()
{
  return GetBuildVersionName();
}

void MainConfig::Serialize(Serializer& stream)
{
}

ZilchDefineType(EditorConfig, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  type->AddAttribute(ObjectAttributes::cCore);
  ZilchBindFieldProperty(ZeroHubUsername);
}

void EditorConfig::Serialize(Serializer& stream)
{
  SerializeNameDefault(EditingProject, String());
  SerializeNameDefault(EditingLevel, String());
  SerializeNameDefault(ZeroHubUsername, String());
}

ZilchDefineType(ContentConfig, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  type->AddAttribute(ObjectAttributes::cCore);
}

void ContentConfig::Serialize(Serializer& stream)
{
  SerializeNameDefault(ContentOutput, String());
  SerializeNameDefault(ToolsDirectory, String());
  SerializeNameDefault(LibraryDirectories, LibraryDirectories);
  SerializeEnumNameDefault(Verbosity, ContentVerbosity, Verbosity::Minimal);
  SerializeNameDefault(HistoryEnabled, true);
}

ZilchDefineType(UserConfig, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZilchBindFieldProperty(UserName);
  ZilchBindFieldProperty(UserEmail);
  type->AddAttribute(ObjectAttributes::cCore);
}

UserConfig::UserConfig() : LoggedIn(false)
{
}

void UserConfig::Serialize(Serializer& stream)
{
  SerializeNameDefault(UserName, String());
  SerializeNameDefault(UserEmail, String());
  SerializeNameDefault(Authentication, String());

  SerializeNameDefault(LastVersionKnown, 0u);
  SerializeNameDefault(LastVersionUsed, 0u);
}

ZilchDefineType(DeveloperConfig, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZilchBindFieldProperty(mDoubleEscapeQuit);
  ZilchBindFieldProperty(mProxyObjectsInPreviews);
  ZilchBindFieldProperty(mCanModifyReadOnlyResources);
}

DeveloperConfig::DeveloperConfig()
{
  // We don't want to serialize this because it could be dangerous
  mProxyObjectsInPreviews = true;
}

void DeveloperConfig::Serialize(Serializer& stream)
{
  SerializeNameDefault(mDoubleEscapeQuit, false);
  SerializeNameDefault(mCanModifyReadOnlyResources, false);
  SerializeNameDefault(mGenericFlags, HashSet<String>());
}

// ZilchPluginConfig
ZilchDefineType(ZilchPluginConfig, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZilchBindFieldProperty(mAttemptedIdeToolsInstall);
  type->AddAttribute(ObjectAttributes::cCore);
}

ZilchPluginConfig::ZilchPluginConfig()
{
  mAttemptedIdeToolsInstall = false;
}

void ZilchPluginConfig::Initialize(CogInitializer& initializer)
{
}

void ZilchPluginConfig::Serialize(Serializer& stream)
{
  SerializeNameDefault(mAttemptedIdeToolsInstall, false);
}

ZilchDefineType(TextEditorConfig, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
  ZilchBindFieldProperty(TabWidth);
  ZilchBindFieldProperty(ShowWhiteSpace);
  ZilchBindFieldProperty(LineNumbers);
  ZilchBindFieldProperty(CodeFolding);
  ZilchBindFieldProperty(TextMatchHighlighting);
  ZilchBindFieldProperty(HighlightPartialTextMatch);
  ZilchBindFieldProperty(ConfidentAutoCompleteOnSymbols);
  ZilchBindFieldProperty(LocalWordCompletion);
  ZilchBindFieldProperty(KeywordAndTypeCompletion);
  ZilchBindFieldProperty(AutoCompleteOnEnter);
  ZilchBindField(ColorScheme);
  ZilchBindFieldProperty(FontSize);

  type->AddAttribute(ObjectAttributes::cCore);
}

void TextEditorConfig::Serialize(Serializer& stream)
{
  // This has to be done since the original values were serialized as garbage
  // (didn't use SerializeNameDefault, and had no constructor / SetDefaults)
  // Therefore we had to tell it to serialize as another name, so we never
  // pulled from the incorrectly saved data Eventually when we roll a new
  // config, these should be changed back to just SerializeNameDefault
  stream.SerializeFieldDefault("ShowWhiteSpace2", ShowWhiteSpace, true);
  stream.SerializeFieldDefault("ConfidentAutoCompleteOnSymbols6", ConfidentAutoCompleteOnSymbols, true);
  stream.SerializeFieldDefault("LocalWordCompletion", LocalWordCompletion, true);
  stream.SerializeFieldDefault("KeywordAndTypeCompletion", KeywordAndTypeCompletion, true);
  stream.SerializeFieldDefault("AutoCompleteOnEnter6", AutoCompleteOnEnter, true);
  SerializeNameDefault(FontSize, uint(13));
  SerializeNameDefault(ColorScheme, String("DarkZero"));
  SerializeNameDefault(LineNumbers, true);
  SerializeNameDefault(CodeFolding, false);
  SerializeNameDefault(TextMatchHighlighting, true);
  SerializeNameDefault(HighlightPartialTextMatch, false);
  SerializeEnumNameDefault(TabWidth, TabWidth, TabWidth::TwoSpaces);
}

uint RecentProjects::mAbsoluteMaxRecentProjects = 50;

ZilchDefineType(RecentProjects, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
}

void RecentProjects::Serialize(Serializer& stream)
{
  SerializeNameDefault(mMaxRecentProjects, 20u);
  SerializeNameDefault(mRecentProjects, mRecentProjects);
}

void RecentProjects::Initialize(CogInitializer& initializer)
{
  RemoveMissingProjects();
}

void RecentProjects::AddRecentProject(StringParam file, bool sendsEvent)
{
  // See if we can make room for this project if needed
  RemoveMissingProjects();

  // If we already contain the file being added then don't remove or add
  // anything
  if (mRecentProjects.Contains(file) == false)
  {
    // If we've hit the limit of recent projects, remove the oldest project
    if (mRecentProjects.Size() >= mAbsoluteMaxRecentProjects)
      RemoveRecentProject(GetOldestProject());

    mRecentProjects.Insert(file);
  }

  if (sendsEvent)
  {
    Event toSend;
    DispatchEvent(Events::RecentProjectsUpdated, &toSend);
  }
}

void RecentProjects::RemoveRecentProject(StringParam projectFile, bool sendsEvent)
{
  mRecentProjects.Erase(projectFile);

  if (sendsEvent)
  {
    Event toSend;
    DispatchEvent(Events::RecentProjectsUpdated, &toSend);
  }
}

class FileTimeSorter
{
public:
  bool operator()(StringParam left, StringParam right)
  {
    return CheckFileTime(left, right) == 1;
  }
};

void RecentProjects::GetProjectsByDate(Array<String>& projects)
{
  // Make sure to remove all missing projects first
  RemoveMissingProjects();

  projects.Reserve(mRecentProjects.Size());

  forRange (String location, mRecentProjects.All())
  {
    projects.PushBack(location);
  }

  // Sort the projects on when they were last opened.
  Sort(projects.All(), FileTimeSorter());
  // Only give them that max number of projects they request
  if (projects.Size() > mMaxRecentProjects)
    projects.Resize(mMaxRecentProjects);
}

void RecentProjects::CopyProjects(RecentProjects* source)
{
  mRecentProjects = source->mRecentProjects;
  mMaxRecentProjects = source->mMaxRecentProjects;
  RemoveMissingProjects();
}

size_t RecentProjects::GetRecentProjectsCount() const
{
  return mRecentProjects.Size();
}

String RecentProjects::GetOldestProject()
{
  String oldestProject;
  TimeType time = cTimeMax;
  forRange (String currProjectPath, mRecentProjects.All())
  {
    TimeType currTime = 0;

    // Missing projects are considered old
    if (FileExists(currProjectPath))
      currTime = GetFileModifiedTime(currProjectPath);

    if (currTime < time)
    {
      time = currTime;
      oldestProject = currProjectPath;
    }
  }

  return oldestProject;
}

void RecentProjects::UpdateMaxNumberOfProjects(uint maxRecentProjects, bool sendsEvent)
{
  mMaxRecentProjects = maxRecentProjects;

  // See if we can make room
  RemoveMissingProjects();

  // Remove all older projects until we reach the limit
  while (mRecentProjects.Size() > mAbsoluteMaxRecentProjects)
    RemoveRecentProject(GetOldestProject());

  if (sendsEvent)
  {
    Event toSend;
    DispatchEvent(Events::RecentProjectsUpdated, &toSend);
  }
}

void RecentProjects::RemoveMissingProjects()
{
  HashSet<String> recentProjects(mRecentProjects);
  mRecentProjects.Clear();

  forRange (String project, recentProjects.All())
  {
    if (FileExists(project))
      mRecentProjects.Insert(project);
  }
}

String GetRemoteConfigDirectory(StringParam organization, StringParam applicationName)
{
  return FilePath::Combine(GetUserDocumentsDirectory(), BuildString(organization, applicationName));
}

String GetConfigDirectory()
{
  return GetRemoteConfigDirectory(GetOrganization(), GetApplicationName());
}

String GetConfigFileName()
{
  return String::Format("ConfigurationV%d.data", GetConfigVersion());
}

String GetRemoteConfigFilePath(StringParam organization, StringParam applicationName)
{
  return FilePath::Combine(GetRemoteConfigDirectory(organization, applicationName), GetConfigFileName());
}

String GetConfigFilePath()
{
  return FilePath::Combine(GetConfigDirectory(), GetConfigFileName());
}

void SaveConfig()
{
  if (!MainConfig::sConfigCanSave)
    return;

  String configDirectory = GetConfigDirectory();
  CreateDirectoryAndParents(configDirectory);
  String configFile = GetConfigFilePath();

  ObjectSaver saver;
  Status status;
  saver.Open(status, configFile.c_str());
  ErrorIf(status.Failed(), "Failed to save config file");
  saver.SaveInstance(Z::gEngine->mConfigCog); // SaveDefinition?
}

void RemoveConfig()
{
  String configFile = GetConfigFilePath();
  DeleteFile(configFile);
}

Cog* LoadRemoteConfig(StringParam organization, StringParam applicationName)
{
  String path = GetRemoteConfigFilePath(organization, applicationName);
  return Z::gFactory->Create(Z::gEngine->GetEngineSpace(), path, 0, nullptr);
}

Cog* LoadConfig(ModifyConfigFn modifier, void* userData)
{
  // If we're in safe mode, just use the default config
  bool useDefault = Environment::GetValue<bool>("safe", false);

  ZPrint("Build Version: %s\n", GetBuildVersionName().c_str());

  ZPrint("Loading configuration for %s...\n", GetApplicationName().c_str());

  static const String cDataDirectoryName("Data");

  String appCacheDirectory = GetUserLocalDirectory();
  String documentDirectory = GetUserDocumentsDirectory();
  String applicationDirectory = GetApplicationDirectory();
  String configFile = GetConfigFilePath();
  String sourceDirectory = FindSourceDirectory();
  String dataDirectory = FilePath::Combine(sourceDirectory, cDataDirectoryName);
  const String defaultConfigFile = String::Format("Default%sConfiguration.data", GetApplicationName().c_str());

  Cog* configCog = nullptr;
  bool userConfigExists = false;

  // Locations to look for the config file.
  // Some of them are absolute, some are relative to the working directory.
  Array<String> searchConfigPaths;

  if (!useDefault)
  {
    // In the working directory (for exported projects).
    searchConfigPaths.PushBack(GetConfigFileName());
    // The user config in the documents directory.
    searchConfigPaths.PushBack(configFile);
  }
  // In the source's Data directory.
  searchConfigPaths.PushBack(FilePath::Combine(dataDirectory, defaultConfigFile));

  forRange (StringParam path, searchConfigPaths)
  {
    if (FileExists(path))
    {
      configCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), path, 0, nullptr);

      // Make sure we successfully created the config Cog from the data file.
      if (configCog != nullptr)
      {
        userConfigExists = (path == configFile);
        ZPrintFilter(Filter::DefaultFilter, "Using config '%s'.\n", path.c_str());
        break;
      }
    }
  }

  if (configCog == nullptr)
  {
    FatalEngineError("Failed to find or open the configuration file");
    return nullptr;
  }

  modifier(configCog, userData);

  MainConfig* mainConfig = HasOrAdd<MainConfig>(configCog);
  mainConfig->SourceDirectory = sourceDirectory;
  mainConfig->DataDirectory = dataDirectory;

  Z::gEngine->mConfigCog = configCog;

  SaveConfig();

  configCog->mFlags.SetFlag(CogFlags::Protected);
  return configCog;
}

String FindDirectoryFromRootFile(String dir, StringParam root)
{
  while (!dir.Empty())
  {
    String rootFile = FilePath::Combine(dir, root);
    if (FileExists(rootFile))
    {
      return dir;
    }

    dir = FilePath::GetDirectoryPath(dir);
  }

  return String();
}

String FindSourceDirectory()
{
  static const String cRoot(".welder");
  String dir;

  dir = FindDirectoryFromRootFile(GetWorkingDirectory(), cRoot);
  if (!dir.Empty())
    return dir;

  dir = FindDirectoryFromRootFile(GetApplicationDirectory(), cRoot);
  if (!dir.Empty())
    return dir;

  return GetWorkingDirectory();
}

} // namespace Zero
