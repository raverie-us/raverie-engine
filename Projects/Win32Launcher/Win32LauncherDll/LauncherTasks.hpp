///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Josh Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------GetVersionListingTask
// Get the possible versions of zero from the server.
class GetVersionListingTaskJob : public DownloadTaskJob
{
public:
  typedef GetVersionListingTaskJob ZilchSelf;

  GetVersionListingTaskJob(StringParam url);

  /// Job Interface.
  int Execute() override;
  void OnReponse(WebResponseEvent* event);

  void PopulatePackageList();

  Array<Cog*> mPackages;
};

//-------------------------------------------------------------------DownloadImageTask
class DownloadImageTaskJob : public DownloadTaskJob
{
public:
  typedef DownloadImageTaskJob ZilchSelf;

  DownloadImageTaskJob(StringParam url);

  /// Job Interface.
  int Execute() override;
  void OnReponse(WebResponseEvent* event);

  Image mImage;
  bool mImageWasInvalid;
};

//-------------------------------------------------------------------LoadImageFromDiskTask
class LoadImageFromDiskTaskJob : public DownloadImageTaskJob
{
public:
  typedef LoadImageFromDiskTaskJob ZilchSelf;
  LoadImageFromDiskTaskJob(StringParam path);

  /// Job Interface.
  int Execute() override;

  String mPath;
};

//-------------------------------------------------------------------GetDataTask
// Gets the data from the url
class GetDataTaskJob : public DownloadTaskJob
{
public:
  typedef GetDataTaskJob ZilchSelf;

  GetDataTaskJob(StringParam url);

  /// Job Interface.
  int Execute() override;
  void OnReponse(WebResponseEvent* event);

  String mData;
};

//-------------------------------------------------------------------DownloadStandaloneTask
// A background task to download and install the latest zero
class DownloadStandaloneTaskJob : public DownloadTaskJob
{
public:
  typedef DownloadStandaloneTaskJob ZilchSelf;

  DownloadStandaloneTaskJob(StringParam url);

  /// Job Interface.
  int Execute() override;
  void OnReponse(WebResponseEvent* event);

  String mInstallLocation;
  String mMetaContents;
};

//-------------------------------------------------------------------InstallBuildTask
class InstallBuildTaskJob : public BackgroundTaskJob
{
public:
  InstallBuildTaskJob();
  InstallBuildTaskJob(StringParam data);

  void LoadFromFile(StringParam filePath);

  /// Job Interface.
  int Execute() override;
  void InstallBuild();

  String mData;
  String mMetaContents;
  String mInstallLocation;
};

//-------------------------------------------------------------------DeleteDirectoryJob
// Deletes a directory on a thread because deleting can take a little bit of time.
// Can also recursively delete empty parent directories up to a certain root so as to not leave empty folders.
class DeleteDirectoryJob : public BackgroundTaskJob
{
public:
  typedef DeleteDirectoryJob ZilchSelf;

  DeleteDirectoryJob(StringParam directory, StringParam rootDirectory, bool recursivelyDeleteEmpty);

  /// Job Interface.
  int Execute() override;

  String mDirectory;
  String mRootDirectory;
  bool mRecursivelyDeleteEmpty;
};

//-------------------------------------------------------------------GetTemplateListingTask
// Get the possible templates from the server.
class GetTemplateListingTaskJob : public DownloadTaskJob
{
public:
  typedef GetTemplateListingTaskJob ZilchSelf;

  GetTemplateListingTaskJob(StringParam url);

  /// Job Interface.
  int Execute() override;
  void OnReponse(WebResponseEvent* event);

  void PopulateTemplateList();

  Array<Cog*> mTemplates;
  Array<TemplateProject> mTemplateList;
};

//-------------------------------------------------------------------DownloadTemplateTask
class DownloadTemplateTaskJob : public DownloadTaskJob
{
public:
  typedef DownloadTemplateTaskJob ZilchSelf;
  DownloadTemplateTaskJob(StringParam templateUrl, TemplateProject* project);

  /// Job Interface.
  int Execute() override;
  void OnReponse(WebResponseEvent* event);

  TemplateProject* mTemplate;
  /// Where to install the template to
  String mTemplateInstallLocation;
  /// What the name of the template is to install (so the file name)
  String mTemplateNameWithoutExtension;
  String mMetaContents;
};

//-------------------------------------------------------------------DownloadAndCreateTemplateTask
// Downloads a template (if needed) and creates a new project from that template.
class DownloadAndCreateTemplateTaskJob : public DownloadTemplateTaskJob
{
public:
  typedef DownloadAndCreateTemplateTaskJob ZilchSelf;
  DownloadAndCreateTemplateTaskJob(StringParam templateUrl, TemplateProject* project);

  /// Job Interface.
  int Execute() override;
  void OnReponse(WebResponseEvent* event);

  // Creating a project currently involves serialization which is not thread safe, this must be called on the main thread.
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

//-------------------------------------------------------------------DownloadLauncherMajorInstallerJob
// A background task to check if there's a new major version installer for the launcher and if so download it.
class CheckForLauncherMajorInstallerJob : public DownloadTaskJob
{
public:
  typedef CheckForLauncherMajorInstallerJob ZilchSelf;
  CheckForLauncherMajorInstallerJob(StringParam url);

  /// Job Interface.
  int Execute() override;
  void OnReponse(WebResponseEvent* event);

  /// Is there a new major-version installer for the launcher available?
  bool mIsNewInstallerAvailable;
};

//-------------------------------------------------------------------DownloadLauncherMajorInstallerJob
// A background task to check if there's a new major version installer for the launcher and if so download it.
class DownloadLauncherMajorInstallerJob : public DownloadTaskJob
{
public:
  typedef DownloadLauncherMajorInstallerJob ZilchSelf;
  DownloadLauncherMajorInstallerJob(StringParam url);

  /// Job Interface.
  int Execute() override;
  void OnReponse(WebResponseEvent* event);

  /// Is there a new major-version installer for the launcher available?
  bool mIsNewInstallerAvailable;
  /// The location of the new installer if we had one to download.
  String mInstallerPath;
};

}//namespace Zero
