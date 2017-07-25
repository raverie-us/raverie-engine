///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Josh Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{

DeclareEvent(InstallStarted);
DeclareEvent(InstallCompleted);
DeclareEvent(UninstallStarted);
DeclareEvent(UninstallCompleted);
DeclareEvent(BuildListUpdated);
DeclareEvent(NewBuildAvailable);
DeclareEvent(TemplateListUpdated);

}//namespace Events

DeclareEnum3(WarningLevel, None, Basic, Severe);
typedef HashSet<String> TagSet;

// Basic functionality for managing/installing/etc... different versions of zero.
class VersionSelector : public EventObject
{
public:
  typedef VersionSelector ZilchSelf;

  VersionSelector(LauncherConfig* config);
  ~VersionSelector();

  static String GetLauncherPhpUrl();

  // Finds what versions are currently installed and populates the internal version listings
  void FindInstalledVersions();
  void FindInstalledVersions(StringParam searchPath);
  /// Given a directory and build folder name, load the locally installed build's information.
  void LoadInstalledBuild(StringParam directoryPath, StringParam buildFolder);
  /// For all installed builds, overwrite their saved meta files. This fixes
  /// legacy builds before meta existed and this also serves to update any installed
  /// information when the server changes (tags, release notes, etc...)
  void SaveInstalledBuildsMeta();

  // Queue up a task to get the listing of versions from the server
  BackgroundTask* GetServerListing();
  // Updates the internal version listing based upon the results from GetServerListing
  void UpdatePackageListing(GetVersionListingTaskJob* job);
  void AddCustomBuild(StringParam buildPath, bool shouldInstall);
  /// Given a path to a local '.zerotemplate' file, install the template project and extract
  /// all necessary information. Returns false if no valid sku/id could be extracted.
  bool InstallLocalTemplateProject(StringParam filePath);
  
  /// Create a template project from the specified meta file
  TemplateProject* CreateTemplateProjectFromMeta(StringParam metaFilePath);
  TemplateProject* CreateTemplateProjectFromMeta(Cog* metaCog, StringParam localPath);

  // Find all available downloaded templates (checks next to the dll and the downloads location)
  void FindDownloadedTemplates();
  // Recursively searches a directory for templates.
  // Uses the rootDir param to re-base the url from the server.
  void FindDownloadedTemplatesRecursive(StringParam searchPath);
  BackgroundTask* GetTemplateListing();
  void UpdateTemplateListing(GetTemplateListingTaskJob* templates);
  BackgroundTask* DownloadTemplateProject(TemplateProject* project);
  BackgroundTask* CreateProjectFromTemplate(TemplateProject* project, StringParam templateInstallPath, StringParam projectInstallPath, const BuildId& buildId,
                                            const HashSet<String>& projectTags);
  BackgroundTask* DownloadTemplateIcon(TemplateProject* project);
  BackgroundTask* DownloadTemplatePreviews(TemplateProject* project);
  BackgroundTask* DownloadDeveloperNotes();

  // Given a path to a build archive file, generate a build id. This is most commonly
  // for dragging in local builds to be installed. This tries several methods of identifying
  // the build: looking for a meta file in the archive, parsing the old file
  // naming format, finally it may generate a unique id from the hash.
  // Note: the passed in zero build's meta cog will be set to the loaded meta cog or newly created one.
  void GetBuildIdFromArchive(StringParam buildPath, ZeroBuild& zeroBuild, BuildId& buildId);
  /// Given an archive that came from the specified file, find the specified meta file and create it if we can.
  /// Also, try and extract any images specified in the meta file to the same directory that the meta file goes to.
  Cog* ExtractLocalTemplateMetaAndImages(Archive& archive, File& file, StringParam metaFileName, bool extractImages);
  /// Finds a file by name from the given archive
  ArchiveEntry* FindEntryFromArchive(Archive& archive, StringParam fileName, bool ignorePath = true);

  // Starts a task to installs the given version
  BackgroundTask* InstallVersion(ZeroBuild* standalone);
  // Starts a task to delete (uninstall) the given version
  BackgroundTask* DeleteVersion(ZeroBuild* standalone);
  // Checks to see if any instances of the given build are currently running (so we can try to uninstall it).
  bool CheckForRunningBuild(ZeroBuild* build);
  // Kills any instances of the given build that are currently running (so we can try to uninstall it).
  void ForceCloseRunningBuilds(ZeroBuild* build);
  /// Take all installed builds and uninstall->reinstall them (mostly for patching old issues).
  void ForceUpdateAllBuilds();
  /// Run a background task to check if there's a new major version of the launcher.
  BackgroundTask* CheckForMajorLauncherUpdate();
  /// Run a background task to download a new major version of the launcher.
  BackgroundTask* DownloadMajorLauncherUpdate();

  /// Find the standalone that is the exact version desired by the project
  ZeroBuild* FindExactVersion(CachedProject* cachedProject);
  ZeroBuild* GetLatestBuild();
  /// Get the latest build available with certain tags
  ZeroBuild* GetLatestBuild(const HashSet<String>& requiredTags, const HashSet<String>& rejectionTags);

  /// Returns how many builds are currently installed
  size_t GetInstalledBuildsCount() const;
  void OnForwardEvent(Event* e);

  // Given a set of current acceptance and rejection tags as well as the current search string, returns what
  // versions match this along with what tags within these versions match the current search.
  void FindVersionsWithTags(TagSet& activeTags, TagSet& rejectionTags, StringParam activeSearch,
                            Array<ZeroBuild*>& results, TagSet& resultTags);
  void FindTemplateWithTags(const BuildId& buildId, TagSet& activeTags, TagSet& rejectionTags, StringParam activeSearch,
                            Array<TemplateProject*>& results, TagSet& resultTags);
  // Checks the given version to see if it is compatible for the project.
  // If the versions aren't an exact match then the warning string will be updated to
  // reflect why they don't match. If the warning level is "severe" then it's likely
  // a bad idea to update and the user should be warned even more so.
  WarningLevel::Enum CheckVersionForProject(ZeroBuild* standalone, CachedProject* cachedProject, String& warningString, bool warnForUpgrading = true);

  // Runs the given version with the specified project (assumes the version is installed)
  void RunProject(ZeroBuild* standalone, StringParam projectPath);
  void RunProject(ZeroBuild* standalone, CachedProject* cachedProject);
  // Runs the given version and tells it to create a new project with the given name
  // (if possible, some old versions can't properly specify the name). Also assumes the version is installed.
  void RunNewProject(ZeroBuild* standalone, StringParam projectName, Cog* configCog);
  
  // Locally marks the version as valid
  void MarkVersionValid(ZeroBuild* standalone);
  // Locally marks the version as invalid
  void MarkVersionInvalid(ZeroBuild* standalone, StringParam userDescription);

  String GetRootInstallLocation();
  String GetInstallExePath(ZeroBuild* build);
  String GetInstallLocation(ZeroBuild* standalone);
  String GetDefaultInstallLocation(const BuildId& buildId);
  String GetMarkedBadPath(ZeroBuild* standalone);

  
  typedef Array<ZeroBuild*> BuildList;
  BuildList mVersions;
  BuildList mOldVersions;
  // Version number (as string) to install
  typedef HashMap<BuildId, ZeroBuild*> VersionMap;
  VersionMap mVersionMap;

  typedef Array<TemplateProject*> TemplateList;
  TemplateList mTemplates;
  typedef HashMap<String, TemplateProject*> TemplateMap;
  TemplateMap mTemplateMap;
  // Templates that are no longer on the server and should be destroyed but
  // can't be because someone could be listening to them.
  TemplateList mOldTemplates;
  Array<ReinstallHelper*> mReinstallTasks;

  LauncherConfig* mConfig;
};

//-------------------------------------------------------------------ReinstallTask
class ReinstallHelper : public EventObject
{
public:
  ReinstallHelper(ZeroBuild* build, VersionSelector* versionSelector);
  void OnReinstall(Event* e);

  typedef ReinstallHelper ZilchSelf;
  ZeroBuild* mBuild;
  VersionSelector* mVersionSelector;
};

}//namespace Zero
