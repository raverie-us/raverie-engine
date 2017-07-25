///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Josh Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------GetVersionListingTask
GetVersionListingTaskJob::GetVersionListingTaskJob(StringParam url) : DownloadTaskJob(url)
{
}

int GetVersionListingTaskJob::Execute()
{
  ConnectThisTo(&mRequest, Events::WebResponse, OnReponse);
  int ret = DownloadTaskJob::Execute();
  return ret;
}
  
void GetVersionListingTaskJob::OnReponse(WebResponseEvent* event)
{
  if(event->ResponseCode == WebResponseCode::OK)
  {
    mData = event->Data;
    mState = BackgroundTaskState::Completed;
  }
  else
  {
    DoNotifyWarning("Failed Download", "Failed to download newest version");
    Failed();
  }
}

void GetVersionListingTaskJob::PopulatePackageList()
{
  Status status;

  // Create a cog from the received data
  DataTreeLoader loader;
  loader.OpenBuffer(status, mData);
  Cog* rootCog = Z::gFactory->CreateFromStream(Z::gEngine->GetEngineSpace(), loader, 0, nullptr);
  ReturnIf(rootCog == nullptr, , "Invalid root cog created from server list of builds");
  
  // Walk all children and add them to our list of available build packages
  HierarchyList::range children = rootCog->GetChildren();
  for(; !children.Empty(); children.PopFront())
  {
    mPackages.PushBack(&children.Front());
  }
}

//-------------------------------------------------------------------GetDataTask
DownloadImageTaskJob::DownloadImageTaskJob(StringParam url) : DownloadTaskJob(url)
{
  mImageWasInvalid = false;
}

int DownloadImageTaskJob::Execute()
{
  ConnectThisTo(&mRequest, Events::WebResponse, OnReponse);
  int ret = DownloadTaskJob::Execute();
  return ret;
}

void DownloadImageTaskJob::OnReponse(WebResponseEvent* event)
{
  if(event->ResponseCode == WebResponseCode::OK)
  {
    //just save the data
    Status status;
    LoadFromPng(status, &mImage, (byte*)event->Data.Data(), event->Data.SizeInBytes());

    if(status.Failed())
    {
      Failed();
      mImageWasInvalid = true;
      return;
    }
  }
  else
  {
    DoNotifyWarning("Failed Download", "Failed to download image");
    Failed();
  }

  mState = BackgroundTaskState::Completed;
  UpdateDownloadProgress();
}

//-------------------------------------------------------------------LoadImageFromDiskTask
LoadImageFromDiskTaskJob::LoadImageFromDiskTaskJob(StringParam path)
  : DownloadImageTaskJob(path)
{
  mPath = path;
}

int LoadImageFromDiskTaskJob::Execute()
{
  if(FileExists(mPath) == false)
  {
    mState = BackgroundTaskState::Failed;
    return 1;
  }

  Status status;
  LoadFromPng(status, &mImage, mPath);

  if(status.Failed())
  {
    mState = BackgroundTaskState::Failed;
    return 1;
  }

  mState = BackgroundTaskState::Completed;
  UpdateProgress(GetName(), 1.0f);

  return 1;
}

//-------------------------------------------------------------------GetDataTask
GetDataTaskJob::GetDataTaskJob(StringParam url) : DownloadTaskJob(url)
{

}

int GetDataTaskJob::Execute()
{
  ConnectThisTo(&mRequest, Events::WebResponse, OnReponse);
  int ret = DownloadTaskJob::Execute();
  return ret;
}

void GetDataTaskJob::OnReponse(WebResponseEvent* event)
{
  if(event->ResponseCode == WebResponseCode::OK)
  {
    //just save the data
    mData = event->Data;
  }
  else
  {
    DoNotifyWarning("Failed Download", "Failed to download newest version");
    Failed();
  }

  mState = BackgroundTaskState::Completed;
  UpdateDownloadProgress();
}

//-------------------------------------------------------------------DownloadStandaloneTask
DownloadStandaloneTaskJob::DownloadStandaloneTaskJob(StringParam url) : DownloadTaskJob(url)
{
}

int DownloadStandaloneTaskJob::Execute()
{
  ConnectThisTo(&mRequest, Events::WebResponse, OnReponse);
  int ret = DownloadTaskJob::Execute();
  mData = String();
  return ret;
}

void DownloadStandaloneTaskJob::OnReponse(WebResponseEvent* event)
{
  if(event->ResponseCode == WebResponseCode::OK)
  {
    String data = event->Data;

    // Make sure the directory where we're extract to exists
    CreateDirectoryAndParents(mInstallLocation);

    // Validate that we got any data (otherwise archive fails)
    if(data.SizeInBytes() == 0)
    {
      String msg = String::Format("Download of build from url '%s' failed", mRequest.mUrl.c_str());
      DoNotifyWarning("Download failed", msg);
      return;
    }

    // Save the meta file out for this build (cached as a string to avoid threading issues)
    String metaFilePath = ZeroBuild::GetMetaFilePath(mInstallLocation);
    WriteStringRangeToFile(metaFilePath, mMetaContents);

    // Decompress the archive to our install location
    Archive archive(ArchiveMode::Decompressing);
    ByteBufferBlock buffer((byte*)data.Data(), data.SizeInBytes(), false);
    // Unfortunately, archive doesn't return any failed state so we could get back an invalid zip.
    // Currently this should only happen if the server code is wrong.
    archive.ReadBuffer(ArchiveReadFlags::All, buffer);
    archive.ExportToDirectory(ArchiveExportMode::Overwrite, mInstallLocation);
    mRequest.ClearData();

    mState = BackgroundTaskState::Completed;
  }
  else
  {
    DoNotifyWarning("Failed Download", "Failed to download newest version");
    Failed();
  }
}

//-------------------------------------------------------------------InstallBuildTask
InstallBuildTaskJob::InstallBuildTaskJob()
{

}

InstallBuildTaskJob::InstallBuildTaskJob(StringParam data)
{
  mData = data;
}

void InstallBuildTaskJob::LoadFromFile(StringParam filePath)
{
  //build the task to start the install
  size_t fileSize;
  byte* data = ReadFileIntoMemory(filePath.c_str(), fileSize, 1);
  data[fileSize] = 0;
  mData = String((char*)data, fileSize);
}

int InstallBuildTaskJob::Execute()
{
  InstallBuild();

  mState = BackgroundTaskState::Completed;
  UpdateProgress(GetName(), 1.0f);

  return 1;
}

void InstallBuildTaskJob::InstallBuild()
{
  //if the build folder already exists for some reason then replace it (maybe prompt later)
  if(FileExists(mInstallLocation))
    DeleteDirectory(mInstallLocation);

  //make sure the directory where we're extract to exists
  CreateDirectoryAndParents(mInstallLocation);

  // Save the meta file out for this build (cached as a string to avoid threading issues)
  String metaFilePath = ZeroBuild::GetMetaFilePath(mInstallLocation);
  WriteStringRangeToFile(metaFilePath, mMetaContents);

  //decompress the archive to our install location
  Archive archive(ArchiveMode::Decompressing);
  ByteBufferBlock buffer((byte*)mData.Data(), mData.SizeInBytes(), false);
  archive.ReadBuffer(ArchiveReadFlags::All, buffer);
  archive.ExportToDirectory(ArchiveExportMode::Overwrite, mInstallLocation);

  mState = BackgroundTaskState::Completed;
}

//-------------------------------------------------------------------DeleteDirectoryJob
DeleteDirectoryJob::DeleteDirectoryJob(StringParam directory, StringParam rootDirectory, bool recursivelyDeleteEmpty)
{
  mDirectory = directory;
  mRootDirectory = rootDirectory;
  mRecursivelyDeleteEmpty = recursivelyDeleteEmpty;
}

int DeleteDirectoryJob::Execute()
{
  if(DirectoryExists(mDirectory) == false)
    return 1;

  // Try to preserve the user's marked bad data if it existed
  String markedBadPath = FilePath::CombineWithExtension(mDirectory, "VersionMarkedBad", ".txt");
  String markedBadData;
  if(FileExists(markedBadPath))
    markedBadData = ReadFileIntoString(markedBadPath);

  // Remove the directory recursively
  DeleteDirectory(mDirectory);

  // Recursively delete empty parent folders until we reach the root directory
  if(mRecursivelyDeleteEmpty)
  {
    String parentDir = FilePath::GetDirectoryPath(mDirectory);
    while(!parentDir.Empty() && mRootDirectory != parentDir && FileRange(parentDir).Empty())
    {
      DeleteDirectory(parentDir);
      parentDir = FilePath::GetDirectoryPath(parentDir);
    }
  }

  // If it did exist then write the file back out
  if(!markedBadData.Empty())
  {
    CreateDirectoryAndParents(mDirectory);
    WriteStringRangeToFile(markedBadPath, markedBadData);
  }

  mState = BackgroundTaskState::Completed;
  UpdateProgress("DeleteDirectory", 1.0f);
  return 1;
}

//-------------------------------------------------------------------GetTemplateListingTask
GetTemplateListingTaskJob::GetTemplateListingTaskJob(StringParam url) : DownloadTaskJob(url)
{

}

int GetTemplateListingTaskJob::Execute()
{
  ConnectThisTo(&mRequest, Events::WebResponse, OnReponse);
  int ret = DownloadTaskJob::Execute();
  return ret;
}

void GetTemplateListingTaskJob::OnReponse(WebResponseEvent* event)
{
  if(event->ResponseCode == WebResponseCode::OK)
  {
    mData = event->Data;
    mState = BackgroundTaskState::Completed;
  }
  else
  {
    DoNotifyWarning("Failed Download", "Failed to download newest version");
    Failed();
  }
}

void GetTemplateListingTaskJob::PopulateTemplateList()
{
  Status status;
  // Create a cog from the received data
  DataTreeLoader loader;
  loader.OpenBuffer(status, mData);
  Cog* rootCog = Z::gFactory->CreateFromStream(Z::gEngine->GetEngineSpace(), loader, 0, nullptr);
  ReturnIf(rootCog == nullptr, , "Invalid root cog created from server list of builds");

  // Walk all children and add them to our list of available build packages
  HierarchyList::range children = rootCog->GetChildren();
  for(; !children.Empty(); children.PopFront())
  {
    mTemplates.PushBack(&children.Front());
  }
}

//-------------------------------------------------------------------DownloadTemplateTask
DownloadTemplateTaskJob::DownloadTemplateTaskJob(StringParam templateUrl, TemplateProject* project)
  : DownloadTaskJob(templateUrl)
{
  mTemplate = project;
}

int DownloadTemplateTaskJob::Execute()
{
  ConnectThisTo(&mRequest, Events::WebResponse, OnReponse);
  int ret = DownloadTaskJob::Execute();
  return ret;
}

void DownloadTemplateTaskJob::OnReponse(WebResponseEvent* event)
{
  if(event->ResponseCode != WebResponseCode::OK)
  {
    DoNotifyWarning("Failed Download", "Failed to download newest version");
    Failed();
    return;
  }

  String data = event->Data;

  CreateDirectoryAndParents(mTemplateInstallLocation);

  ZeroTemplate* zeroTemplate = mTemplate->GetZeroTemplate(false);

  // Save the template meta file
  FilePath::GetFileNameWithoutExtension(mTemplateNameWithoutExtension);
  String metaFilePath = FilePath::CombineWithExtension(mTemplateInstallLocation, mTemplateNameWithoutExtension, ".meta");
  WriteToFile(metaFilePath.c_str(), (byte*)mMetaContents.c_str(), mMetaContents.SizeInBytes());

  // Save the template zip
  String templateFilePath = FilePath::CombineWithExtension(mTemplateInstallLocation, mTemplateNameWithoutExtension, TemplateProject::mExtensionWithDot);
  WriteToFile(templateFilePath.c_str(), (byte*)data.c_str(), data.SizeInBytes());

  // Save the icon image
  String iconFilePath = FilePath::Combine(mTemplateInstallLocation, zeroTemplate->mIconUrl);
  Status status;
  SaveToPng(status, &mTemplate->mIconImage, iconFilePath);
  
  mState = BackgroundTaskState::Completed;
}

//-------------------------------------------------------------------DownloadAndCreateTemplateTask
DownloadAndCreateTemplateTaskJob::DownloadAndCreateTemplateTaskJob(StringParam templateUrl, TemplateProject* project)
  : DownloadTemplateTaskJob(templateUrl, project)
{
  
}

int DownloadAndCreateTemplateTaskJob::Execute()
{
  mCachedProject = nullptr;
  // If the template is downloaded and not available on the server then just create from the local path
  if(mTemplate->mIsDownloaded && !mTemplate->mIsOnServer)
  {
    // @JoshD: This is currently broken because the even will be sent before the listener.
    UpdateProgress("CreatedTemplate", 1.0f);
    mState = BackgroundTaskState::Completed;
    return 1;
  }

  ConnectThisTo(&mRequest, Events::WebResponse, OnReponse);
  int ret = DownloadTaskJob::Execute();
  return ret;
}

void DownloadAndCreateTemplateTaskJob::OnReponse(WebResponseEvent* event)
{
  // Nothing to do here (we can only run serialization on the main thread)
}

CachedProject* DownloadAndCreateTemplateTaskJob::GetOrCreateCachedProject(ProjectCache* projectCache)
{
  // The cached project was already created, just return it
  if(mCachedProject != nullptr)
    return mCachedProject;

  if(mRequest.mResponseCode != WebResponseCode::OK)
  {
    DoNotifyWarning("Failed Download", "Failed to download newest version");
    Failed();
    return nullptr;
  }

  mState = BackgroundTaskState::Completed;

  // Save the template file to a temporary location, if it already exists then delete the old file.
  String templatePath = GetTemporaryDirectory();
  String templateFilePath = FilePath::CombineWithExtension(templatePath, mTemplateNameWithoutExtension, TemplateProject::mExtensionWithDot);
  if(FileExists(templateFilePath))
    DeleteFile(templateFilePath);
  WriteToFile(templateFilePath.c_str(), (byte*)mData.c_str(), mData.SizeInBytes());

  // From the downloaded file, create the project
  CreateFromTemplateFile(templateFilePath, projectCache);

  return mCachedProject;
}

void DownloadAndCreateTemplateTaskJob::CreateFromTemplateFile(StringParam templateFilePath, ProjectCache* projectCache)
{
  // Create the project from the template
  String projectFolder = FilePath::Combine(mProjectInstallLocation, mProjectName);
  mCachedProject = projectCache->CreateProjectFromTemplate(mProjectName, projectFolder, templateFilePath, mBuildId, mProjectTags);
  
  if(mCachedProject == nullptr)
    mState = BackgroundTaskState::Failed;
  else
    mState = BackgroundTaskState::Completed;
}

//-------------------------------------------------------------------CheckForLauncherMajorInstallerJob
CheckForLauncherMajorInstallerJob::CheckForLauncherMajorInstallerJob(StringParam url) : DownloadTaskJob(url)
{
  mIsNewInstallerAvailable = false;
}

int CheckForLauncherMajorInstallerJob::Execute()
{
  ConnectThisTo(&mRequest, Events::WebResponse, OnReponse);
  return DownloadTaskJob::Execute();
}

void CheckForLauncherMajorInstallerJob::OnReponse(WebResponseEvent* event)
{
  // Check if there's no new installer available, either from a failed request or getting no data back
  String data = event->Data;
  if(event->ResponseCode != WebResponseCode::OK || data.SizeInBytes() == 0)
  {
    mIsNewInstallerAvailable = false;
    return;
  }

  // Otherwise, we have a new installer
  mIsNewInstallerAvailable = true;
}

//-------------------------------------------------------------------DownloadLauncherMajorInstallerJob
DownloadLauncherMajorInstallerJob::DownloadLauncherMajorInstallerJob(StringParam url) : DownloadTaskJob(url)
{
  mIsNewInstallerAvailable = false;
}

int DownloadLauncherMajorInstallerJob::Execute()
{
  ConnectThisTo(&mRequest, Events::WebResponse, OnReponse);
  return DownloadTaskJob::Execute();
}

void DownloadLauncherMajorInstallerJob::OnReponse(WebResponseEvent* event)
{
  // Check if there's no new installer available, either from a failed request or getting no data back
  String data = event->Data;
  if(event->ResponseCode != WebResponseCode::OK || data.SizeInBytes() == 0)
  {
    mIsNewInstallerAvailable = false;
    return;
  }

  // Otherwise, we have a new installer
  mIsNewInstallerAvailable = true;
  // Write the installer to a temp location
  String temporaryDirectory = GetTemporaryDirectory();
  mInstallerPath = FilePath::Combine(temporaryDirectory, "ZeroLauncherInstaller.exe");
  if(FileExists(mInstallerPath))
    DeleteFile(mInstallerPath);

  WriteToFile(mInstallerPath.c_str(), (byte*)data.c_str(), data.SizeInBytes());
  mState = BackgroundTaskState::Completed;
}

}//namespace Zero
