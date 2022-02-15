// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

GetVersionListingTaskJob::GetVersionListingTaskJob(StringParam applicationName) :
    DownloadTaskJob(Urls::cApiBuilds, cCacheSeconds),
    mApplicationName(applicationName)
{
}

void GetVersionListingTaskJob::Execute()
{
  AsyncWebRequest* request = mRequest;
  ConnectThisTo(request, Events::WebResponseComplete, OnReponse);
  DownloadTaskJob::Execute();
}

void GetVersionListingTaskJob::OnReponse(WebResponseEvent* event)
{
  if (event->mResponseCode == WebResponseCode::OK)
  {
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

  static const String cOrigin = "GetVersionListingTaskJob";
  String jsonData = GetData();
  CompilationErrors errors;
  JsonValue* jsonReleases = JsonReader::ReadIntoTreeFromString(errors, jsonData, cOrigin, nullptr);
  ReturnIf(jsonReleases == nullptr, , "Invalid JsonValue created from GitHub API");

  Space* space = Z::gEngine->GetEngineSpace();

  Archetype* emptyArchetype = ArchetypeManager::FindOrNull(CoreArchetypes::Empty);
  ReturnIf(emptyArchetype == nullptr, , "Unable to find empty Cog Archetype");

  static const String cJsonAssets("assets");
  static const String cJsonName("name");
  static const String cJsonDownloadUrl("browser_download_url");
  static const String cJsonUpdatedAt("updated_at");
  static const String cJsonBody("body");

  forRange (JsonValue* jsonRelease, jsonReleases->ArrayElements)
  {
    JsonValue* jsonAssets = jsonRelease->GetMember(cJsonAssets);

    if (!jsonAssets)
      continue;

    // There should only be one, but this is safest to do.
    forRange (JsonValue* jsonAsset, jsonAssets->ArrayElements)
    {
      String name = jsonAsset->MemberAsString(cJsonName);

      BuildId id;
      if (!id.Parse(name))
        continue;
      if (mApplicationName != GetApplicationName())
        continue;

      Cog* cog = space->Create(emptyArchetype);
      ReturnIf(cog == nullptr, , "Unable to create an empty Cog");
      mPackages.PushBack(cog);
      ZeroBuildContent* zeroBuildContent = HasOrAdd<ZeroBuildContent>(cog);
      zeroBuildContent->mPackageName = name;

      zeroBuildContent->mBuildId = id;
      zeroBuildContent->mPackageExtension = id.mPackageExtension;

      zeroBuildContent->mDownloadUrl = jsonAsset->MemberAsString(cJsonDownloadUrl);

      // Parse tags or anything else that we just populated (normally happens
      // during Initialize).
      zeroBuildContent->Parse();

      String releaseNotes = jsonRelease->MemberAsString(cJsonBody, String(), JsonErrorMode::DefaultValue);
      if (!releaseNotes.Empty())
      {
        ZeroBuildReleaseNotes* zeroBuildReleaseNotes = HasOrAdd<ZeroBuildReleaseNotes>(cog);
        zeroBuildReleaseNotes->mNotes = jsonRelease->MemberAsString(cJsonBody);
      }
    }
  }
}

DownloadImageTaskJob::DownloadImageTaskJob(StringParam url) : DownloadTaskJob(url, cCacheSeconds)
{
  mImageWasInvalid = false;
}

void DownloadImageTaskJob::Execute()
{
  AsyncWebRequest* request = mRequest;
  ConnectThisTo(request, Events::WebResponseComplete, OnReponse);
  DownloadTaskJob::Execute();
}

void DownloadImageTaskJob::OnReponse(WebResponseEvent* event)
{
  if (event->mResponseCode == WebResponseCode::OK)
  {
    // just save the data
    Status status;
    LoadImage(status, (byte*)event->mData.Data(), event->mData.SizeInBytes(), &mImage);

    if (status.Failed())
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

LoadImageFromDiskTaskJob::LoadImageFromDiskTaskJob(StringParam path) : DownloadImageTaskJob(path)
{
  mPath = path;
}

void LoadImageFromDiskTaskJob::Execute()
{
  if (FileExists(mPath) == false)
  {
    mState = BackgroundTaskState::Failed;
    return;
  }

  Status status;
  LoadImage(status, mPath, &mImage);

  if (status.Failed())
  {
    mState = BackgroundTaskState::Failed;
    return;
  }

  mState = BackgroundTaskState::Completed;
  UpdateProgress(GetName(), 1.0f);
}

GetDataTaskJob::GetDataTaskJob(StringParam url) : DownloadTaskJob(url, cCacheSeconds)
{
}

void GetDataTaskJob::Execute()
{
  AsyncWebRequest* request = mRequest;
  ConnectThisTo(request, Events::WebResponseComplete, OnReponse);
  DownloadTaskJob::Execute();
}

void GetDataTaskJob::OnReponse(WebResponseEvent* event)
{
  if (event->mResponseCode == WebResponseCode::OK)
  {
    // just save the data
    mData = event->mData;
  }
  else
  {
    DoNotifyWarning("Failed Download", "Failed to download newest version");
    Failed();
  }

  mState = BackgroundTaskState::Completed;
  UpdateDownloadProgress();
}

DownloadStandaloneTaskJob::DownloadStandaloneTaskJob(StringParam url) : DownloadTaskJob(url, cCacheSeconds)
{
}

void DownloadStandaloneTaskJob::Execute()
{
  AsyncWebRequest* request = mRequest;
  ConnectThisTo(request, Events::WebResponseComplete, OnReponse);
  DownloadTaskJob::Execute();
}

void Install(StringParam installLocation, StringParam data)
{
  EnsureEmptyDirectory(installLocation);

  // decompress the archive to our install location
  Archive archive(ArchiveMode::Decompressing);
  ByteBufferBlock buffer((byte*)data.Data(), data.SizeInBytes(), false);
  // Unfortunately, archive doesn't return any failed state so we could get
  // back an invalid zip. Currently this should only happen if the server code
  // is wrong.
  archive.ReadBuffer(ArchiveReadFlags::All, buffer);
  archive.ExportToDirectory(ArchiveExportMode::Overwrite, installLocation);

  String executablePath = FilePath::Combine(installLocation, GetEditorExecutableFileName());
  Os::MarkAsExecutable(executablePath.c_str());

  // For platforms like Emscripten, the build will package the file into a virtual file system.
  // Extract any file that is a zip file.
  FileRange range(installLocation);
  // We have to make a copy of the range so that way we don't walk over files we just extracted.
  Array<String> entries = RangeToArray<String>(range);
  forRange (StringParam fileName, entries)
  {
    String fullPath = FilePath::Combine(installLocation, fileName);
    if (Archive::IsZip(fullPath))
    {
      Archive internalArchive(ArchiveMode::Decompressing);
      internalArchive.ReadZipFile(ArchiveReadFlags::All, fullPath);
      internalArchive.ExportToDirectory(ArchiveExportMode::Overwrite, installLocation);
    }
  }
}

void DownloadStandaloneTaskJob::OnReponse(WebResponseEvent* event)
{
  if (event->mResponseCode == WebResponseCode::OK)
  {
    String data = event->mData;

    // Validate that we got any data (otherwise archive fails)
    if (data.SizeInBytes() == 0)
    {
      String msg = String::Format("Download of build from url '%s' failed", mRequest->mUrl.c_str());
      DoNotifyWarning("Download failed", msg);
      return;
    }

    Install(mInstallLocation, data);
    mState = BackgroundTaskState::Completed;
  }
  else
  {
    DoNotifyWarning("Failed Download", "Failed to download newest version");
    Failed();
  }

  mRequest->ClearAll();
}

InstallBuildTaskJob::InstallBuildTaskJob()
{
}

InstallBuildTaskJob::InstallBuildTaskJob(StringParam data)
{
  mData = data;
}

void InstallBuildTaskJob::LoadFromFile(StringParam filePath)
{
  // build the task to start the install
  size_t fileSize;
  byte* data = ReadFileIntoMemory(filePath.c_str(), fileSize, 1);
  data[fileSize] = 0;
  mData = String((char*)data, fileSize);
}

void InstallBuildTaskJob::Execute()
{
  InstallBuild();

  mState = BackgroundTaskState::Completed;
  UpdateProgress(GetName(), 1.0f);
}

void InstallBuildTaskJob::InstallBuild()
{
  Install(mInstallLocation, mData);
  mState = BackgroundTaskState::Completed;
}

DeleteDirectoryJob::DeleteDirectoryJob(StringParam directory, StringParam rootDirectory, bool recursivelyDeleteEmpty)
{
  mDirectory = directory;
  mRootDirectory = rootDirectory;
  mRecursivelyDeleteEmpty = recursivelyDeleteEmpty;
}

void DeleteDirectoryJob::Execute()
{
  if (DirectoryExists(mDirectory) == false)
    return;

  // Try to preserve the user's marked bad data if it existed
  String markedBadPath = FilePath::CombineWithExtension(mDirectory, "VersionMarkedBad", ".txt");
  String markedBadData;
  if (FileExists(markedBadPath))
    markedBadData = ReadFileIntoString(markedBadPath);

  // Remove the directory recursively
  DeleteDirectory(mDirectory);

  // Recursively delete empty parent folders until we reach the root directory
  if (mRecursivelyDeleteEmpty)
  {
    String parentDir = FilePath::GetDirectoryPath(mDirectory);
    while (!parentDir.Empty() && mRootDirectory != parentDir && FileRange(parentDir).Empty())
    {
      DeleteDirectory(parentDir);
      parentDir = FilePath::GetDirectoryPath(parentDir);
    }
  }

  // If it did exist then write the file back out
  if (!markedBadData.Empty())
  {
    CreateDirectoryAndParents(mDirectory);
    WriteStringRangeToFile(markedBadPath, markedBadData);
  }

  mState = BackgroundTaskState::Completed;
  UpdateProgress("DeleteDirectory", 1.0f);
}

GetTemplateListingTaskJob::GetTemplateListingTaskJob(StringParam url) : DownloadTaskJob(url, cCacheSeconds)
{
}

void GetTemplateListingTaskJob::Execute()
{
  AsyncWebRequest* request = mRequest;
  ConnectThisTo(request, Events::WebResponseComplete, OnReponse);
  DownloadTaskJob::Execute();
}

void GetTemplateListingTaskJob::OnReponse(WebResponseEvent* event)
{
  if (event->mResponseCode == WebResponseCode::OK)
  {
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

  static const String cOrigin = "GetTemplateListingTaskJob";
  String jsonData = GetData();
  CompilationErrors errors;
  JsonValue* jsonReleases = JsonReader::ReadIntoTreeFromString(errors, jsonData, cOrigin, nullptr);
  ReturnIf(jsonReleases == nullptr, , "Invalid JsonValue created from GitHub API");

  Space* space = Z::gEngine->GetEngineSpace();

  Archetype* emptyArchetype = ArchetypeManager::FindOrNull(CoreArchetypes::Empty);
  ReturnIf(emptyArchetype == nullptr, , "Unable to find empty Cog Archetype");

  static const String cJsonAssets("assets");
  static const String cJsonName("name");
  static const String cJsonDownloadUrl("browser_download_url");
  static const String cJsonUpdatedAt("updated_at");
  static const String cJsonBody("body");

  // Find a string that is _VersionNumber, this can include '.' ',' and '-' in
  // order to specify ranges The expected file name is SKU_UserId_BuildId where
  // _UserId is optional.
  static const Regex cNameRegex("(\\w+)(_[\\w\\d\\.\\,\\-]+)?_([\\w\\d\\.\\,\\-]+)\\.zerotemplate");
  static const size_t cNameRegexMatches = 4;

  forRange (JsonValue* jsonRelease, jsonReleases->ArrayElements)
  {
    JsonValue* jsonAssets = jsonRelease->GetMember(cJsonAssets);

    if (!jsonAssets)
      continue;

    // There should only be one, but this is safest to do.
    forRange (JsonValue* jsonAsset, jsonAssets->ArrayElements)
    {
      String name = jsonAsset->MemberAsString(cJsonName);

      Matches matches;
      cNameRegex.Search(name, matches);

      // Make sure the regular expression matched (if not, it may be some other
      // release that's not a zerotemplate). Specificially, if this is a png for
      // the release then it will not match.
      if (matches.Size() != cNameRegexMatches)
        continue;

      Cog* cog = space->Create(emptyArchetype);
      ReturnIf(cog == nullptr, , "Unable to create an empty Cog");
      mTemplates.PushBack(cog);
      ZeroTemplate* zeroTemplate = HasOrAdd<ZeroTemplate>(cog);

      zeroTemplate->mSKU = matches[1];
      zeroTemplate->mVersionId = matches[3];

      zeroTemplate->mDate = jsonAsset->MemberAsString(cJsonUpdatedAt);

      // Remove the time portion (otherwise, the date is in the exact format we
      // expect: YYYY-MM-DD).
      StringIterator begin = zeroTemplate->mDate.Begin();
      StringIterator end = zeroTemplate->mDate.FindFirstOf(Rune('T')).Begin();
      if (!end.Empty())
        zeroTemplate->mDate = StringRange(begin, end);

      zeroTemplate->mDownloadUrl = jsonAsset->MemberAsString(cJsonDownloadUrl);

      // The icon url is always the same as the template url, but with a png
      // extension instead.
      zeroTemplate->mIconUrl = zeroTemplate->mDownloadUrl.Replace(".zerotemplate", ".png");

      zeroTemplate->mDisplayName = jsonRelease->MemberAsString(cJsonName);
      zeroTemplate->mDescription = jsonRelease->MemberAsString(cJsonBody);

      // Parse tags/build-ids and anything else that we just populated (normally
      // happens during Initialize).
      zeroTemplate->Parse();
    }
  }
}

DownloadTemplateTaskJob::DownloadTemplateTaskJob(StringParam templateUrl, TemplateProject* project) :
    DownloadTaskJob(templateUrl, cCacheSeconds)
{
  mTemplate = project;
}

void DownloadTemplateTaskJob::Execute()
{
  AsyncWebRequest* request = mRequest;
  ConnectThisTo(request, Events::WebResponseComplete, OnReponse);
  DownloadTaskJob::Execute();
}

void DownloadTemplateTaskJob::OnReponse(WebResponseEvent* event)
{
  if (event->mResponseCode != WebResponseCode::OK)
  {
    DoNotifyWarning("Failed Download", "Failed to download newest version");
    Failed();
    return;
  }

  String data = event->mData;

  CreateDirectoryAndParents(mTemplateInstallLocation);

  ZeroTemplate* zeroTemplate = mTemplate->GetZeroTemplate(false);

  // Save the template meta file
  FilePath::GetFileNameWithoutExtension(mTemplateNameWithoutExtension);

  // Save the template zip
  String templateFilePath = FilePath::CombineWithExtension(
      mTemplateInstallLocation, mTemplateNameWithoutExtension, TemplateProject::mExtensionWithDot);
  WriteToFile(templateFilePath.c_str(), (byte*)data.c_str(), data.SizeInBytes());

  // Save the icon image
  String iconFilePath = FilePath::Combine(mTemplateInstallLocation, zeroTemplate->mIconUrl);
  Status status;
  SaveImage(status, iconFilePath, &mTemplate->mIconImage, ImageSaveFormat::Png);

  mState = BackgroundTaskState::Completed;
}

DownloadAndCreateTemplateTaskJob::DownloadAndCreateTemplateTaskJob(StringParam templateUrl, TemplateProject* project) :
    DownloadTemplateTaskJob(templateUrl, project)
{
}

void DownloadAndCreateTemplateTaskJob::Execute()
{
  mCachedProject = nullptr;
  // If the template is downloaded and not available on the server then just
  // create from the local path
  if (mTemplate->mIsDownloaded && !mTemplate->mIsOnServer)
  {
    // @JoshD: This is currently broken because the even will be sent before the
    // listener.
    UpdateProgress("CreatedTemplate", 1.0f);
    mState = BackgroundTaskState::Completed;
    return;
  }

  AsyncWebRequest* request = mRequest;
  ConnectThisTo(request, Events::WebResponseComplete, OnReponse);
  DownloadTaskJob::Execute();
}

void DownloadAndCreateTemplateTaskJob::OnReponse(WebResponseEvent* event)
{
  // Nothing to do here (we can only run serialization on the main thread)
}

CachedProject* DownloadAndCreateTemplateTaskJob::GetOrCreateCachedProject(ProjectCache* projectCache)
{
  // The cached project was already created, just return it
  if (mCachedProject != nullptr)
    return mCachedProject;

  if (mRequest->mResponseCode != WebResponseCode::OK)
  {
    DoNotifyWarning("Failed Download", "Failed to download newest version");
    Failed();
    return nullptr;
  }

  mState = BackgroundTaskState::Completed;

  // Save the template file to a temporary location, if it already exists then
  // delete the old file.
  String templatePath = GetTemporaryDirectory();
  String templateFilePath =
      FilePath::CombineWithExtension(templatePath, mTemplateNameWithoutExtension, TemplateProject::mExtensionWithDot);
  if (FileExists(templateFilePath))
    DeleteFile(templateFilePath);
  String data = GetData();
  WriteToFile(templateFilePath.c_str(), (byte*)data.c_str(), data.SizeInBytes());

  // From the downloaded file, create the project
  CreateFromTemplateFile(templateFilePath, projectCache);

  return mCachedProject;
}

void DownloadAndCreateTemplateTaskJob::CreateFromTemplateFile(StringParam templateFilePath, ProjectCache* projectCache)
{
  // Create the project from the template
  String projectFolder = FilePath::Combine(mProjectInstallLocation, mProjectName);
  mCachedProject =
      projectCache->CreateProjectFromTemplate(mProjectName, projectFolder, templateFilePath, mBuildId, mProjectTags);

  if (mCachedProject == nullptr)
    mState = BackgroundTaskState::Failed;
  else
    mState = BackgroundTaskState::Completed;
}

DownloadLauncherPatchInstallerJob::DownloadLauncherPatchInstallerJob(StringParam url,
                                                                     StringParam rootDownloadLocation) :
    DownloadTaskJob(url, cCacheSeconds)
{
  mRootDownloadLocation = rootDownloadLocation;
  mIsNewPatchAvailable = false;
}

void DownloadLauncherPatchInstallerJob::Execute()
{
  AsyncWebRequest* request = mRequest;
  ConnectThisTo(request, Events::WebResponseComplete, OnReponse);
  DownloadTaskJob::Execute();
}

void DownloadLauncherPatchInstallerJob::OnReponse(WebResponseEvent* event)
{
  // Check if there's no new installer available, either from a failed request
  // or getting no data back
  String data = event->mData;
  if (event->mResponseCode != WebResponseCode::OK || data.SizeInBytes() == 0)
  {
    mIsNewPatchAvailable = false;
    return;
  }

  // Otherwise, we have a new installer
  mIsNewPatchAvailable = true;

  // extract the archive to the download directory
  Archive archive(ArchiveMode::Decompressing);
  ByteBufferBlock buffer((byte*)data.Data(), data.SizeInBytes(), false);
  archive.ReadBuffer(ArchiveReadFlags::All, buffer);

  // Find the patch id from the archive. If we failed to find one
  // then something is wrong so report a failure.
  String patchId = FindPatchId(archive);
  if (patchId.Empty())
  {
    mState = BackgroundTaskState::Failed;
    mIsNewPatchAvailable = false;
    return;
  }

  // Extract the patch to the patch folder location
  String downloadPath = FilePath::Combine(mRootDownloadLocation, patchId);
  archive.ExportToDirectory(ArchiveExportMode::Overwrite, downloadPath);

  mState = BackgroundTaskState::Completed;
}

String DownloadLauncherPatchInstallerJob::FindPatchId(Archive& archive)
{
  // Find the version id file and return it's contents
  for (Archive::range range = archive.GetEntries(); !range.Empty(); range.PopFront())
  {
    ArchiveEntry& entry = range.Front();
    if (entry.Name != "ZeroLauncherVersionId.txt")
      continue;

    String patchId((cstr)entry.Full.Data, entry.Full.Size);
    return patchId;
  }
  // Otherwise return an empty string
  return String();
}

DownloadLauncherMajorInstallerJob::DownloadLauncherMajorInstallerJob(StringParam url) :
    DownloadTaskJob(url, cCacheSeconds)
{
  mIsNewInstallerAvailable = false;
}

void DownloadLauncherMajorInstallerJob::Execute()
{
  AsyncWebRequest* request = mRequest;
  ConnectThisTo(request, Events::WebResponseComplete, OnReponse);
  DownloadTaskJob::Execute();
}

void DownloadLauncherMajorInstallerJob::OnReponse(WebResponseEvent* event)
{
  // Check if there's no new installer available, either from a failed request
  // or getting no data back
  String data = event->mData;
  if (event->mResponseCode != WebResponseCode::OK || data.SizeInBytes() == 0)
  {
    mIsNewInstallerAvailable = false;
    return;
  }

  // Otherwise, we have a new installer
  mIsNewInstallerAvailable = true;
  // Write the installer to a temp location
  String temporaryDirectory = GetTemporaryDirectory();
  mInstallerPath = FilePath::Combine(temporaryDirectory, "ZeroLauncherInstaller.exe");
  if (FileExists(mInstallerPath))
    DeleteFile(mInstallerPath);

  WriteToFile(mInstallerPath.c_str(), (byte*)data.c_str(), data.SizeInBytes());
  mState = BackgroundTaskState::Completed;
}

BackupProjectJob::BackupProjectJob(StringParam projectPath, StringParam destFilePath)
{
  mOpenDirectoryOnCompletion = true;
  mProjectPath = projectPath;
  mDestinationFilePath = destFilePath;
}

void BackupProjectJob::Execute()
{
  String targetDirectory = FilePath::GetDirectoryPath(mDestinationFilePath);
  CreateDirectoryAndParents(targetDirectory);

  // Collect all of the files to archive
  Array<ArchiveData> files;
  GetFileList(mProjectPath, String(), files);

  // Add each file to the archive, sending out progress events every so often
  Archive projectArchive(ArchiveMode::Compressing);
  for (size_t i = 0; i < files.Size(); ++i)
  {
    ArchiveData& data = files[i];
    projectArchive.AddFile(data.mFullFilePath, data.mRelativePath);

    if (i % 5)
      UpdateProgress("ArchivingProject", i / (float)files.Size());
  }

  // Write the zip to the final file location
  projectArchive.WriteZipFile(mDestinationFilePath);

  // Mark that we've finished
  mState = BackgroundTaskState::Completed;
  UpdateProgress("ArchivingProject", 1.0f);

  // If requested, open the target directory on completion.
  if (mOpenDirectoryOnCompletion)
    Os::ShellOpenDirectory(targetDirectory);
  Download(targetDirectory);
}

void BackupProjectJob::GetFileList(StringParam path, StringParam parentPath, Array<ArchiveData>& fileList)
{
  FileRange fileRange(path);
  for (; !fileRange.Empty(); fileRange.PopFront())
  {
    String localPath = fileRange.Front();
    String fullPath = FilePath::Combine(path, fileRange.Front());
    String relativePath = FilePath::Combine(parentPath, localPath);

    // Recurse down directories
    if (DirectoryExists(fullPath))
    {
      String subPath = FilePath::Combine(path, localPath);
      GetFileList(subPath, relativePath, fileList);
    }
    // Add files (need relative path for archive)
    else
    {
      ArchiveData& data = fileList.PushBack();
      data.mFullFilePath = fullPath;
      data.mRelativePath = relativePath;
    }
  }
}

} // namespace Zero
