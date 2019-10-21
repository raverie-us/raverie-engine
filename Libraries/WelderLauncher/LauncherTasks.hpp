// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
// We cache all requests for 4 hours (forced, we don't contact the server).
const u64 cCacheSeconds = 60 * 60 * 4;

// Get the possible versions of zero from the server.
class GetVersionListingTaskJob : public DownloadTaskJob
{
public:
  typedef GetVersionListingTaskJob ZilchSelf;

  GetVersionListingTaskJob(bool launcher);

  /// Job Interface.
  void Execute() override;
  void OnReponse(WebResponseEvent* event);

  void PopulatePackageList();

  Array<Cog*> mPackages;
  bool mLauncher;
};

class DownloadImageTaskJob : public DownloadTaskJob
{
public:
  typedef DownloadImageTaskJob ZilchSelf;

  DownloadImageTaskJob(StringParam url);

  /// Job Interface.
  void Execute() override;
  void OnReponse(WebResponseEvent* event);

  Image mImage;
  bool mImageWasInvalid;
};

class LoadImageFromDiskTaskJob : public DownloadImageTaskJob
{
public:
  typedef LoadImageFromDiskTaskJob ZilchSelf;
  LoadImageFromDiskTaskJob(StringParam path);

  /// Job Interface.
  void Execute() override;

  String mPath;
};

// Gets the data from the url
class GetDataTaskJob : public DownloadTaskJob
{
public:
  typedef GetDataTaskJob ZilchSelf;

  GetDataTaskJob(StringParam url);

  /// Job Interface.
  void Execute() override;
  void OnReponse(WebResponseEvent* event);

  String mData;
};

// A background task to download and install the latest zero
class DownloadStandaloneTaskJob : public DownloadTaskJob
{
public:
  typedef DownloadStandaloneTaskJob ZilchSelf;

  DownloadStandaloneTaskJob(StringParam url);

  /// Job Interface.
  void Execute() override;
  void OnReponse(WebResponseEvent* event);

  String mInstallLocation;
  String mMetaContents;
};

class InstallBuildTaskJob : public BackgroundTaskJob
{
public:
  InstallBuildTaskJob();
  InstallBuildTaskJob(StringParam data);

  void LoadFromFile(StringParam filePath);

  /// Job Interface.
  void Execute() override;
  void InstallBuild();

  String mData;
  String mMetaContents;
  String mInstallLocation;
};

// Deletes a directory on a thread because deleting can take a little bit of
// time. Can also recursively delete empty parent directories up to a certain
// root so as to not leave empty folders.
class DeleteDirectoryJob : public BackgroundTaskJob
{
public:
  typedef DeleteDirectoryJob ZilchSelf;

  DeleteDirectoryJob(StringParam directory, StringParam rootDirectory, bool recursivelyDeleteEmpty);

  /// Job Interface.
  void Execute() override;

  String mDirectory;
  String mRootDirectory;
  bool mRecursivelyDeleteEmpty;
};

// Get the possible templates from the server.
class GetTemplateListingTaskJob : public DownloadTaskJob
{
public:
  typedef GetTemplateListingTaskJob ZilchSelf;

  GetTemplateListingTaskJob(StringParam url);

  /// Job Interface.
  void Execute() override;
  void OnReponse(WebResponseEvent* event);

  void PopulateTemplateList();

  Array<Cog*> mTemplates;
  Array<TemplateProject> mTemplateList;
};

class DownloadTemplateTaskJob : public DownloadTaskJob
{
public:
  typedef DownloadTemplateTaskJob ZilchSelf;
  DownloadTemplateTaskJob(StringParam templateUrl, TemplateProject* project);

  /// Job Interface.
  void Execute() override;
  void OnReponse(WebResponseEvent* event);

  TemplateProject* mTemplate;
  /// Where to install the template to
  String mTemplateInstallLocation;
  /// What the name of the template is to install (so the file name)
  String mTemplateNameWithoutExtension;
  String mMetaContents;
};

// Downloads a template (if needed) and creates a new project from that
// template.
class DownloadAndCreateTemplateTaskJob : public DownloadTemplateTaskJob
{
public:
  typedef DownloadAndCreateTemplateTaskJob ZilchSelf;
  DownloadAndCreateTemplateTaskJob(StringParam templateUrl, TemplateProject* project);

  /// Job Interface.
  void Execute() override;
  void OnReponse(WebResponseEvent* event);

  // Creating a project currently involves serialization which is not thread
  // safe, this must be called on the main thread.
  CachedProject* GetOrCreateCachedProject(ProjectCache* projectCache);

  /// What to name the project that is created from the template
  String mProjectName;
  /// Where to install the project created from the template
  String mProjectInstallLocation;
  /// What build of zero the project should be created for
  BuildId mBuildId;

  HashSet<String> mProjectTags;

private:
  // Create the project from the template file path (must be downloaded to disk)
  void CreateFromTemplateFile(StringParam templateFilePath, ProjectCache* projectCache);

  // A cached project created (on the main thread)
  CachedProject* mCachedProject;
};

// A background task to check if there's a new patch version installer for the
// launcher and if so download it.
class DownloadLauncherPatchInstallerJob : public DownloadTaskJob
{
public:
  typedef DownloadLauncherPatchInstallerJob ZilchSelf;
  DownloadLauncherPatchInstallerJob(StringParam url, StringParam rootDownloadLocation);

  /// Job Interface.
  void Execute() override;
  void OnReponse(WebResponseEvent* event);

  // Extract the patch id of this build from the given archive
  String FindPatchId(Archive& archive);

  /// Is there a new major-version installer for the launcher available?
  bool mIsNewPatchAvailable;
  String mRootDownloadLocation;
};

// A background task to check if there's a new major version installer for the
// launcher and if so download it.
class DownloadLauncherMajorInstallerJob : public DownloadTaskJob
{
public:
  typedef DownloadLauncherMajorInstallerJob ZilchSelf;
  DownloadLauncherMajorInstallerJob(StringParam url);

  /// Job Interface.
  void Execute() override;
  void OnReponse(WebResponseEvent* event);

  /// Is there a new major-version installer for the launcher available?
  bool mIsNewInstallerAvailable;
  /// The location of the new installer if we had one to download.
  String mInstallerPath;
};

// A job to backup a user's project to a directory.
class BackupProjectJob : public BackgroundTaskJob
{
public:
  typedef BackupProjectJob ZilchSelf;
  BackupProjectJob(StringParam projectPath, StringParam destPath);

  /// Job Interface.
  void Execute() override;

  struct ArchiveData
  {
    String mFullFilePath;
    String mRelativePath;
  };
  void GetFileList(StringParam path, StringParam parentPath, Array<ArchiveData>& fileList);

  String mProjectPath;
  String mDestinationFilePath;
  bool mOpenDirectoryOnCompletion;
};

} // namespace Zero
