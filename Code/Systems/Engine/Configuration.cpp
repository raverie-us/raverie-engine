// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

bool MainConfig::sConfigCanSave = true;

RaverieDefineType(MainConfig, builder, type)
{
  RaverieBindComponent();
  RaverieBindDocumented();
  RaverieBindGetterProperty(BuildDate);
  RaverieBindGetterProperty(BuildVersion);
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

RaverieDefineType(EditorConfig, builder, type)
{
  RaverieBindComponent();
  RaverieBindDocumented();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  type->AddAttribute(ObjectAttributes::cCore);
  RaverieBindFieldProperty(BugReportUsername);
}

void EditorConfig::Serialize(Serializer& stream)
{
  SerializeNameDefault(EditingProject, String());
  SerializeNameDefault(EditingLevel, String());
  SerializeNameDefault(BugReportUsername, String());
}

RaverieDefineType(ContentConfig, builder, type)
{
  RaverieBindComponent();
  RaverieBindDocumented();
  type->AddAttribute(ObjectAttributes::cCore);
}

void ContentConfig::Serialize(Serializer& stream)
{
  SerializeNameDefault(ContentOutput, String());
  SerializeNameDefault(ToolsDirectory, String());
  SerializeNameDefault(LibraryDirectories, LibraryDirectories);
  SerializeNameDefault(HistoryEnabled, true);
}

RaverieDefineType(UserConfig, builder, type)
{
  RaverieBindComponent();
  RaverieBindDocumented();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindFieldProperty(UserName);
  RaverieBindFieldProperty(UserEmail);
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

RaverieDefineType(DeveloperConfig, builder, type)
{
  RaverieBindComponent();
  RaverieBindDocumented();
  RaverieBindFieldProperty(mDoubleEscapeQuit);
  RaverieBindFieldProperty(mProxyObjectsInPreviews);
  RaverieBindFieldProperty(mCanModifyReadOnlyResources);
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

RaverieDefineType(TextEditorConfig, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();
  RaverieBindFieldProperty(TabWidth);
  RaverieBindFieldProperty(ShowWhiteSpace);
  RaverieBindFieldProperty(LineNumbers);
  RaverieBindFieldProperty(CodeFolding);
  RaverieBindFieldProperty(TextMatchHighlighting);
  RaverieBindFieldProperty(HighlightPartialTextMatch);
  RaverieBindFieldProperty(ConfidentAutoCompleteOnSymbols);
  RaverieBindFieldProperty(LocalWordCompletion);
  RaverieBindFieldProperty(KeywordAndTypeCompletion);
  RaverieBindFieldProperty(AutoCompleteOnEnter);
  RaverieBindField(ColorScheme);
  RaverieBindFieldProperty(FontSize);

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
  SerializeNameDefault(ColorScheme, String("Dark"));
  SerializeNameDefault(LineNumbers, true);
  SerializeNameDefault(CodeFolding, false);
  SerializeNameDefault(TextMatchHighlighting, true);
  SerializeNameDefault(HighlightPartialTextMatch, false);
  SerializeEnumNameDefault(TabWidth, TabWidth, TabWidth::TwoSpaces);
}

String GetConfigDirectory()
{
  return GetUserDocumentsApplicationDirectory();
}

String GetConfigFileName()
{
  return String::Format("ConfigurationV%d.data", GetConfigVersion());
}

String GetRemoteConfigFilePath(StringParam organization, StringParam applicationName)
{
  return FilePath::Combine(GetRemoteUserDocumentsApplicationDirectory(organization, applicationName), GetConfigFileName());
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
  static const String cRoot(".raverie");
  String dir;

  dir = FindDirectoryFromRootFile(GetWorkingDirectory(), cRoot);
  if (!dir.Empty())
    return dir;

  dir = FindDirectoryFromRootFile(GetApplicationDirectory(), cRoot);
  if (!dir.Empty())
    return dir;

  return GetWorkingDirectory();
}

} // namespace Raverie
