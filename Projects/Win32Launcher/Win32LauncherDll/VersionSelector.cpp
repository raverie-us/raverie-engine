///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Josh Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const String BuildsUrlRoot = "https://builds.zeroengine.io/";

namespace Events
{

DefineEvent(InstallStarted);
DefineEvent(InstallCompleted);
DefineEvent(UninstallStarted);
DefineEvent(UninstallCompleted);
DefineEvent(BuildListUpdated);
DefineEvent(TemplateListUpdated);
DefineEvent(NewBuildAvailable);

}//namespace Events

//-------------------------------------------------------------------VersionSorter
/// Sort versions such that newer builds are first. If the branches are different then sort alphabetically.
struct VersionSorter
{
  // Return true if lhs is "newer" than rhs (first things should return true)
  bool operator()(ZeroBuild* lhs, ZeroBuild* rhs)
  {
    BuildId lhsBuildId = lhs->GetBuildId();
    BuildId rhsBuildId = rhs->GetBuildId();
    BuildUpdateState::Enum state = lhsBuildId.CheckForUpdate(rhsBuildId);

    // If the branches are different just sort alphabetically
    if(state == BuildUpdateState::DifferentBranch)
      return lhsBuildId.mExperimentalBranchName < rhsBuildId.mExperimentalBranchName;
    
    // Otherwise, return true if lhs > rhs (aka the update is to an older build)
    bool updatingToOlder = (state == BuildUpdateState::Older || state == BuildUpdateState::OlderBreaking);
    return updatingToOlder;
  }
};

//-------------------------------------------------------------------TemplateSorter
/// Sort templates by their names.
struct TemplateSorter
{
  bool operator()(TemplateProject* lhs, TemplateProject* rhs)
  {
    // If the sort priority is equal then resort to comparing the names
    if(lhs->GetSortPriority() == rhs->GetSortPriority())
      return lhs->GetDisplayName() < rhs->GetDisplayName();

    // Otherwise just use the sort priority
    return lhs->GetSortPriority() < rhs->GetSortPriority();
  }
};

//-------------------------------------------------------------------VersionSelector
VersionSelector::VersionSelector(LauncherConfig* config)
{
  mConfig = config;
}

VersionSelector::~VersionSelector()
{
  DeleteObjectsInContainer(mVersions);
  DeleteObjectsInContainer(mOldVersions);
  DeleteObjectsInContainer(mTemplates);
  DeleteObjectsInContainer(mOldTemplates);
  DeleteObjectsInContainer(mReinstallTasks);
}

String VersionSelector::GetLauncherPhpUrl()
{
  return BuildsUrlRoot;
}

void VersionSelector::FindInstalledVersions()
{
  // Recursively find builds from the root install location
  String rootInstallLocation = GetRootInstallLocation();
  FindInstalledVersions(rootInstallLocation);

  Sort(mVersions.All(), VersionSorter());

  Event e;
  DispatchEvent(Events::BuildListUpdated, &e);
}

void VersionSelector::FindInstalledVersions(StringParam searchPath)
{
  // Iterate over the install directory
  FileRange range(searchPath);
  for(; !range.Empty(); range.PopFront())
  {
    FileEntry entry = range.frontEntry();
    String fullDirName = entry.GetFullPath();
    // We only care about folders
    if(IsDirectory(fullDirName))
    {
      // Add checksum logic to validate installs here?

      // If this folder contains an exe (a build) then load the version, otherwise recurse.
      String zeroExePath = FilePath::CombineWithExtension(fullDirName, "ZeroEditor", ".exe");
      if(FileExists(zeroExePath) == false)
        FindInstalledVersions(fullDirName);
      else
        LoadInstalledBuild(entry.mPath, entry.mFileName);
    }
  }
}

void VersionSelector::LoadInstalledBuild(StringParam directoryPath, StringParam buildFolder)
{
  // By default, parse the build's folder name to
  // attempt to get the build id (mostly for legacy)
  BuildId buildId;
  String year, month, day;
  buildId.Parse(buildFolder, year, month, day);
  
  // Try to load the meta file if it exists. If it doesn't we'll use the parsed build folder id.
  Cog* metaCog = nullptr;
  String fullPath = FilePath::Combine(directoryPath, buildFolder);
  String metaFilePath = ZeroBuild::GetMetaFilePath(fullPath);
  if(FileExists(metaFilePath))
  {
    // Create the meta file's cog for this build (make sure to clear the archetype if it has one).
    // Also, load the build id from the ZeroBuildContent component.
    metaCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), metaFilePath, 0, nullptr);
    // Meta could be null if the file was invalid (all nulls after a bluescreen for instance)
    if(metaCog != nullptr)
    {
      metaCog->ClearArchetype();
      ZeroBuildContent* projectInfo = metaCog->has(ZeroBuildContent);
      if(projectInfo != nullptr)
        buildId = projectInfo->GetBuildId();
    }
  }

  // If this build hasn't already been loaded then we need to create it
  // (we either haven't contacted the server yet or it's not on the server).
  ZeroBuild* localBuild = mVersionMap.FindValue(buildId, nullptr);
  if(localBuild == nullptr)
  {
    localBuild = new ZeroBuild();
    localBuild->mMetaCog = metaCog;
    // If we failed to load a meta cog then this is a legacy build with no meta file,
    // we need to create the meta file and set old data (date-time, revision, etc...)
    bool isLegacy = localBuild->CreateMetaIfNull();
    if(isLegacy)
    {
      ZeroBuildContent* buildInfo = localBuild->GetBuildContent(false);
      buildInfo->mChangeSetDate = String::Format("%s.%s.%s", year.c_str(), month.c_str(), day.c_str());
    }

    localBuild->SetBuildId(buildId);
    // Add this build by id and into an array (sorted by id)
    mVersionMap.Insert(buildId, localBuild);
    mVersions.PushBack(localBuild);
  }

  // If the build has a deprecated component then make sure to add the deprecated tag
  if(localBuild->GetDeprecatedInfo(false) != nullptr)
  {
    ZeroBuildContent* buildInfo = localBuild->GetBuildContent(true);
    buildInfo->AddTag(ZeroBuild::mDeprecatedTag);
  }

  // Save the builds install location so we know where it is to run it
  localBuild->mInstallLocation = fullPath;
  localBuild->mInstallState = InstallState::Installed;
}

void VersionSelector::SaveInstalledBuildsMeta()
{
  // Save each installed build's meta next to the ".exe"
  for(size_t i = 0; i < mVersions.Size(); ++i)
  {
    ZeroBuild* build = mVersions[i];
    if(build->mInstallState != InstallState::Installed)
      continue;

    String metaFilePath = ZeroBuild::GetMetaFilePath(GetInstallLocation(build));
    build->SaveMetaFile(metaFilePath);
  }
}

BackgroundTask* VersionSelector::GetServerListing()
{
  //start the task to get the version listing
  String url = BuildString(GetLauncherPhpUrl(), "?Commands=RequestBuildList");
  
  // If we have dev config then add an extra command to get some extra templates (for testing)
  DeveloperConfig* devConfig = mConfig->GetOwner()->has(DeveloperConfig);
  if(devConfig != nullptr)
    url = BuildString(url, "&DevTest");

  GetVersionListingTaskJob* job = new GetVersionListingTaskJob(url);
  job->mName = "Version List";
  return Z::gBackgroundTasks->Execute(job, job->mName);
}

void VersionSelector::UpdatePackageListing(GetVersionListingTaskJob* job)
{
  //clear out whether or not each version is on the server
  //(so that if one was removed then it will be updated)
  for(uint i = 0; i < mVersions.Size(); ++i)
    mVersions[i]->mOnServer = false;

  // Since versions are sorted by newest to oldest, the first one is the latest version
  BuildId prevNewestBuildId;
  if(!mVersions.Empty())
    prevNewestBuildId = mVersions[0]->GetBuildId();

  // Have to serialize the job's data on the main thread
  job->PopulatePackageList();
  // Add each build (properly merging with local builds)
  for(uint i = 0; i < job->mPackages.Size(); ++i)
  {
    Cog* buildCog = job->mPackages[i];
    // This build must have a meta cog, otherwise the server screwed up
    ZeroBuildContent* buildContent = buildCog->has(ZeroBuildContent);
    if(buildContent == nullptr)
      continue;

    // Get the build id of the server's build and use it to find
    // if we have the same build locally installed
    BuildId serverBuildId = buildContent->GetBuildId();
    ZeroBuild* localStandalone = mVersionMap.FindValue(serverBuildId, nullptr);
    // If this build wasn't locally installed, then we need to create a ZeroBuild for it
    if(localStandalone == nullptr)
    {
      localStandalone = new ZeroBuild();
      localStandalone->mMetaCog = buildCog;
      // Since it didn't already exist that means it wasn't installed
      localStandalone->mInstallState = InstallState::NotInstalled;

      mVersions.PushBack(localStandalone);
      mVersionMap.Insert(serverBuildId, localStandalone);
    }
    // Otherwise, this build was already installed so we have to update the meta info
    else
    {
      // Overwrite the local build's  meta with the server's
      Cog* oldMetaCog = localStandalone->mMetaCog;
      localStandalone->mMetaCog = buildCog;

      // If there local build had a meta then transfer certain local properties
      if(oldMetaCog != nullptr)
      {
        // If the local build had a user defined deprecated message then transfer that over to the new meta
        ZeroBuildDeprecated* oldDeprecatedInfo = oldMetaCog->has(ZeroBuildDeprecated);
        if(oldDeprecatedInfo != nullptr && !oldDeprecatedInfo->mUserMessage.Empty())
        {
          ZeroBuildDeprecated* deprecatedInfo = localStandalone->GetDeprecatedInfo(true);
          deprecatedInfo->mUserMessage = oldDeprecatedInfo->mUserMessage;
        }
      }
    }

    // Either way, mark this build as being on the server
    localStandalone->mOnServer = true;
    
    // If there is a deprecated component then make sure the build is tagged as deprecated
    if(localStandalone->GetDeprecatedInfo(false) != false)
      buildContent->AddTag(ZeroBuild::mDeprecatedTag);
    // On the flip side, the server could have a build tagged as deprecated without actually
    // having the component (or no message). In this case add the component and set a message if there wasn't one.
    // If the tag says this version is deprecated then mark that it is bad
    if(buildContent->ContainsTag(ZeroBuild::mDeprecatedTag) == true)
    {
      ZeroBuildDeprecated* deprecatedInfo = localStandalone->GetDeprecatedInfo(true);
      if(deprecatedInfo->mServerMessage.Empty())
        deprecatedInfo->mServerMessage = "Version was deprecated";
    }
  }

  // Now we know which versions are on the server so remove all of the templates
  // that are no longer on the server (and haven't been downloaded) and then re-populate the hashmap
  mVersionMap.Clear();
  size_t i = 0;
  while(i < mVersions.Size())
  {
    ZeroBuild* build = mVersions[i];
    if(!build->mOnServer && build->mInstallState == InstallState::NotInstalled)
    {
      // Because people were listening to the builds (they're event objects) we can't
      // safely delete them. Instead just move them to another list that we'll clean-up on
      // close (there shouldn't ever be too many of these unless someone leaves the launcher running for months)
      mOldVersions.PushBack(build);
      mVersions[i] = mVersions.Back();
      mVersions.PopBack();
      continue;
    }
    
    mVersionMap.Insert(build->GetBuildId(), build);
    ++i;
  }


  Sort(mVersions.All(), VersionSorter());
  // Save all build's meta now that we have updated data from this server.
  // This will properly cache release notes, tags, etc...
  SaveInstalledBuildsMeta();

  Event e;
  DispatchEvent(Events::BuildListUpdated, &e);

  // If there is a newer version available, it'll have a name different than the one we cached before,
  // in that case send out an event so someone knows (and can update Ui or something)
  if(!mVersions.Empty() && prevNewestBuildId != mVersions[0]->GetBuildId())
  {
    Event toSend;
    DispatchEvent(Events::NewBuildAvailable, &toSend);
  }
}

void VersionSelector::AddCustomBuild(StringParam buildPath, bool shouldInstall)
{
  ZeroBuild* localBuild = new ZeroBuild();

  // Load the build id for this archive. This may come from a meta file in the archive,
  // the file name, or be uniquely generated by the file's contents.
  BuildId buildId;
  GetBuildIdFromArchive(buildPath, *localBuild, buildId);
  localBuild->SetBuildId(buildId);
  localBuild->mInstallLocation = GetDefaultInstallLocation(buildId);

  // Since it didn't already exist that means it wasn't installed
  localBuild->mInstallState = InstallState::Installing;
  
  // If we already had the build installed then remove the old one
  // so we can replace it with the new one
  ZeroBuild* oldBuild = mVersionMap.FindValue(localBuild->GetBuildId(), nullptr);
  if(oldBuild != nullptr)
  {
    mVersions.EraseValue(oldBuild);
    mVersionMap.Erase(localBuild->GetBuildId());
    mOldVersions.PushBack(oldBuild);
  }

  mVersions.PushBack(localBuild);
  mVersionMap.Insert(localBuild->GetBuildId(), localBuild);
  Sort(mVersions.All(), VersionSorter());

  Event e;
  DispatchEvent(Events::BuildListUpdated, &e);

  if(shouldInstall)
  {
    InstallBuildTaskJob* job = new InstallBuildTaskJob();
    job->LoadFromFile(buildPath);
    job->mMetaContents = localBuild->SaveMetaFileToString();
    job->mInstallLocation = localBuild->mInstallLocation;

    //there are several different scenarios that want to install then do
    //something else, because of this just create the task and return it so
    //they can connect to whatever events they want to on the outside
    BackgroundTask* task = Z::gBackgroundTasks->CreateTask(job);
    task->mName = "Latest Install";

    Zero::Connect(task, Events::BackgroundTaskUpdated, localBuild, &ZeroBuild::ForwardEvent);
    Zero::Connect(task, Events::BackgroundTaskCompleted, localBuild, &ZeroBuild::ForwardEvent);
    Zero::Connect(task, Events::BackgroundTaskCompleted, localBuild, &ZeroBuild::InstallCompleted);
    Zero::Connect(task, Events::BackgroundTaskFailed, localBuild, &ZeroBuild::ForwardEvent);

    Event toSend;
    localBuild->DispatchEvent(Events::InstallStarted, &toSend);
    task->Execute();
  }
}

bool VersionSelector::InstallLocalTemplateProject(StringParam filePath)
{
  File file;
  file.Open(filePath, FileMode::Read, FileAccessPattern::Random);

  // Try to find a meta file in the zip. Make sure to open the zip to only read
  // the entries so that we don't have to decompress everything.
  Archive archive(ArchiveMode::Decompressing);
  archive.ReadZip(ArchiveReadFlags::Entries, file);

  // Try and extract the meta file from the archive (as well as any preview images/icons if they exist)
  String sku = FilePath::GetFileNameWithoutExtension(filePath);
  String metaFileName = BuildString(sku, ".meta");
  Cog* metaCog = ExtractLocalTemplateMetaAndImages(archive, file, metaFileName, true);
  // If we failed to find a meta file then we need to parse the file name to get the sku and build id
  if(metaCog == nullptr)
  {
    String fileName = FilePath::GetFileNameWithoutExtension(filePath);
    // Find a string that is [VersionNumber], this can include '.' ',' and '-' in order to specify ranges
    String idGroupingRegex = "(\\[[\\w\\d\\.\\,\\-]+\\])";
    // The expected file name is SKU[UserId][BuildId] where [UserId] is optional.
    String regexStr = String::Format("(\\w+)%s?%s", idGroupingRegex.c_str(), idGroupingRegex.c_str());

    Matches matches;
    Regex regex(regexStr);
    regex.Search(fileName, matches);

    // If we failed to find a match then notify the user
    if(matches.Empty())
      return false;

    // Create a cog for the meta
    metaCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), CoreArchetypes::Empty, 0, nullptr);
    metaCog->ClearArchetype();

    // Create the zero template component and set the sku and version id from what we parsed
    ZeroTemplate* zeroTemplate = HasOrAdd<ZeroTemplate>(metaCog);
    zeroTemplate->mSKU = matches[1];
    String templateIds = matches[3];
    zeroTemplate->mVersionId = templateIds.SubString(templateIds.Begin() + 1, templateIds.End() - 1);
    // Just default the display name to the sku name
    zeroTemplate->mDisplayName = zeroTemplate->mSKU;
    // Parse the ids into a more usable run-time format
    zeroTemplate->ParseVersionId();
  }
  file.Close();

  ZeroTemplate* zeroTemplate = metaCog->has(ZeroTemplate);

  // Always create the template's output directory
  String fullTemplateName = zeroTemplate->GetFullTemplateVersionName();
  String templateInstallDirectory = FilePath::Combine(mConfig->GetTemplateInstallPath(), fullTemplateName);
  CreateDirectoryAndParents(templateInstallDirectory);

  // Create the template project from our meta cog so that we can
  // cache information like where this is installed, icon images, etc...
  TemplateProject* currentProject = CreateTemplateProjectFromMeta(metaCog, templateInstallDirectory);
  // Make sure we set the local path of where this template is installed right away as several other functions use this
  currentProject->mLocalPath = templateInstallDirectory;

  // Always copy over the ".zerotemplate" that is being installed.
  // @JoshD: Remove misc. files like the images and meta later?
  String zeroTemplateFilePathDest = currentProject->GetInstalledTemplatePath();
  CopyFile(zeroTemplateFilePathDest, filePath);
  
  // Always save out the meta file
  String metaFilePath = FilePath::CombineWithExtension(templateInstallDirectory, fullTemplateName, ".meta");
  String metaContents = currentProject->SaveMetaFileToString();
  WriteStringRangeToFile(metaFilePath, metaContents);
  
  // Set the relevant information we loaded and then load the images
  currentProject->mMetaCog = metaCog;
  currentProject->mIsDownloaded = true;
  currentProject->LoadLocalImages();

  // Let everyone know that we've changed our template list
  Event e;
  GetDispatcher()->Dispatch(Events::TemplateListUpdated, &e);
  return true;
}

TemplateProject* VersionSelector::CreateTemplateProjectFromMeta(StringParam metaFilePath)
{
  // Load the meta file
  DataTreeLoader loader;
  Status status;
  loader.OpenFile(status, metaFilePath);
  Cog* metaCog = Z::gFactory->CreateFromStream(Z::gEngine->GetEngineSpace(), loader, 0, nullptr);
  return CreateTemplateProjectFromMeta(metaCog, FilePath::GetDirectoryPath(metaFilePath));
}

TemplateProject* VersionSelector::CreateTemplateProjectFromMeta(Cog* metaCog, StringParam localPath)
{
  // Make sure the meta file contained the necessary info
  if(metaCog == nullptr)
    return nullptr;
  ZeroTemplate* zeroTemplate = metaCog->has(ZeroTemplate);
  if(zeroTemplate == nullptr)
    return nullptr;

  // Check and see if we already had this template
  String key = zeroTemplate->GetIdString();
  TemplateProject* currentProject = mTemplateMap.FindValue(key, nullptr);
  // If we didn't then create a new project and mark it as not existing on the server
  if(currentProject == nullptr)
  {
    currentProject = new TemplateProject();
    currentProject->mMetaCog = metaCog;
    currentProject->mIsOnServer = false;

    mTemplateMap.Insert(key, currentProject);
    mTemplates.PushBack(currentProject);
  }

  // Either way, we need to set that we loaded this meta file
  // from a local path and then load the icons/preview images
  currentProject->mLocalPath = localPath;
  currentProject->mMetaCog = metaCog;
  currentProject->mIsDownloaded = true;
  currentProject->LoadLocalImages();
  return currentProject;
}

void VersionSelector::FindDownloadedTemplates()
{
  // Check the dll install directory
  String dllDownloadDir = mConfig->GetOwner()->has(MainConfig)->ApplicationDirectory;
  String packagedTemplates = FilePath::Combine(dllDownloadDir, "Templates");
  FindDownloadedTemplatesRecursive(packagedTemplates);

  // Check the download directory (for user downloaded templates)
  String dir = FilePath::Combine(mConfig->mDownloadPath, "Templates");
  FindDownloadedTemplatesRecursive(dir);
}

void VersionSelector::FindDownloadedTemplatesRecursive(StringParam searchPath)
{
  // Iterate over the install directory
  FileRange range(searchPath);
  for(; !range.Empty(); range.PopFront())
  {
    FileEntry entry = range.frontEntry();
    String fullDirName = entry.GetFullPath();
    if(IsDirectory(fullDirName))
      FindDownloadedTemplatesRecursive(fullDirName);
    else
    {
      String ext = FilePath::GetExtension(fullDirName);
      if(ext != "meta")
        continue;

      CreateTemplateProjectFromMeta(fullDirName);
    }
  }
}

BackgroundTask* VersionSelector::GetTemplateListing()
{
  String url = BuildString(GetLauncherPhpUrl(), "?Commands=RequestTemplateList");

  // If we have dev config then add an extra command to get some extra templates (for testing)
  DeveloperConfig* devConfig = mConfig->GetOwner()->has(DeveloperConfig);
  if(devConfig != nullptr)
    url = BuildString(url, ",&DevTest");

  GetTemplateListingTaskJob* job = new GetTemplateListingTaskJob(url);
  job->mName = "Template List";
  return Z::gBackgroundTasks->Execute(job, job->mName);
}

void VersionSelector::UpdateTemplateListing(GetTemplateListingTaskJob* templates)
{
  // Mark all templates as not being on the server (so we know which ones to get rid of later)
  for(size_t i = 0; i < mTemplates.Size(); ++i)
    mTemplates[i]->mIsOnServer = false;

  //have to deserialize the job's data on the main thread
  templates->PopulateTemplateList();

  for(size_t i = 0; i < templates->mTemplates.Size(); ++i)
  {
    Cog* templateCog = templates->mTemplates[i];
    ZeroTemplate* serverZeroTemplate = templateCog->has(ZeroTemplate);
    if(serverZeroTemplate == nullptr)
      return;

    String key = serverZeroTemplate->GetIdString();
    TemplateProject* localTemplate = mTemplateMap.FindValue(key, nullptr);
    // If we didn't already have a copy of this template then
    // create a project for it and store its information
    if(localTemplate == nullptr)
    {
      localTemplate = new TemplateProject();
      mTemplates.PushBack(localTemplate);
      mTemplateMap.Insert(key, localTemplate);

      localTemplate->mMetaCog = templateCog;
      localTemplate->mIsOnServer = true;
      localTemplate->DownloadIcon(this);
      continue;
    }

    // Otherwise we already had a copy, but this copy could've been from a previous call to the server or
    // it could be a locally downloaded template. If it is locally downloaded then keep that template's
    // information instead of the servers. Otherwise just update in place.
    if(localTemplate->mIsDownloaded)
    {
      ZeroTemplate* localZeroTemplate = localTemplate->GetZeroTemplate(true);
      localTemplate->mIsOnServer = true;
      localTemplate->mIsDifferentFromServer = (serverZeroTemplate->mDate == localZeroTemplate->mDate);
      continue;
    }

    localTemplate->mMetaCog = templateCog;
    localTemplate->mIsOnServer = true;
    localTemplate->DownloadIcon(this);
  }

  // Now we know which versions are on the server so remove all of the templates
  // that are no longer on the server (and haven't been downloaded) and then re-populate the hashmap
  mTemplateMap.Clear();
  size_t i = 0;
  while(i < mTemplates.Size())
  {
    TemplateProject* project = mTemplates[i];
    if(!project->mIsOnServer && !project->mIsDownloaded)
    {
      // Because people were listening to the templates (they're event objects) we can't
      // safely delete them. Instead just move them to another list that we'll clean-up on
      // close (there shouldn't ever be too many of these unless someone leaves the launcher running for months)
      mOldTemplates.PushBack(mTemplates[i]);
      mTemplates[i] = mTemplates.Back();
      mTemplates.PopBack();
      continue;
    }
    
    String key = project->GetIdString();
    mTemplateMap.Insert(key, project);
    ++i;
  }

  Sort(mTemplates.All(), TemplateSorter());
}

BackgroundTask* VersionSelector::DownloadTemplateProject(TemplateProject* project)
{
  String url = BuildString(GetLauncherPhpUrl(), "?Commands=RequestTemplateProject&TemplatePath=", project->GetTemplateUrl());
  DownloadTemplateTaskJob* job = new DownloadTemplateTaskJob(url, project);

  ZeroTemplate* zeroTemplate = project->GetZeroTemplate(true);
  job->mTemplateInstallLocation = FilePath::Combine(mConfig->GetTemplateInstallPath(), zeroTemplate->GetFullTemplateVersionName());
  job->mTemplateNameWithoutExtension = FilePath::GetFileNameWithoutExtension(project->GetLocalTemplateFileName());
  job->mMetaContents = project->SaveMetaFileToString();

  return Z::gBackgroundTasks->Execute(job, job->mName);
}

BackgroundTask* VersionSelector::CreateProjectFromTemplate(TemplateProject* project, StringParam templateInstallPath, StringParam projectInstallPath, const BuildId& buildId,
                                                           const HashSet<String>& projectTags)
{
  String url = BuildString(GetLauncherPhpUrl(), "?Commands=RequestTemplateProject&TemplatePath=", project->GetTemplateUrl());
  DownloadAndCreateTemplateTaskJob* job = new DownloadAndCreateTemplateTaskJob(url, project);

  job->mTemplateInstallLocation = templateInstallPath;
  job->mTemplateNameWithoutExtension = project->GetLocalTemplateFileName();
  job->mProjectInstallLocation = FilePath::GetDirectoryPath(projectInstallPath);
  job->mProjectName = FilePath::GetFileNameWithoutExtension(projectInstallPath);
  job->mBuildId = buildId;
  job->mName = "Template Download";
  job->mProjectTags = projectTags;

  return Z::gBackgroundTasks->Execute(job, job->mName);
}

BackgroundTask* VersionSelector::DownloadTemplateIcon(TemplateProject* project)
{
  String iconUrl = project->GetIconUrl();
  String url = BuildString(GetLauncherPhpUrl(), "?Commands=RequestTemplateIcon&IconPath=", iconUrl);
  DownloadImageTaskJob* job = new DownloadImageTaskJob(url);
  job->mName = "Download Preview";

  return Z::gBackgroundTasks->Execute(job, job->mName);
}

BackgroundTask* VersionSelector::DownloadTemplatePreviews(TemplateProject* project)
{
  // @JoshD: Update to get the remaining preview icons later...
  return nullptr;
}

BackgroundTask* VersionSelector::DownloadDeveloperNotes()
{
  String url = BuildString(GetLauncherPhpUrl(), "?Commands=ListDevUpdates");
  DownloadTaskJob* job = new DownloadTaskJob(url);
  job->mName = "Download DevUpdates";

  return Z::gBackgroundTasks->Execute(job, job->mName);
}

void VersionSelector::GetBuildIdFromArchive(StringParam buildPath, ZeroBuild& zeroBuild, BuildId& buildId)
{
  // Keep track of if we successfully loaded our build info from a meta file
  bool buildInfoLoadedFromMeta = false;

  File file;
  file.Open(buildPath, FileMode::Read, FileAccessPattern::Random);

  // Try to find a meta file in the zip. Make sure to open the zip to only read
  // the entries so that we don't have to decompress everything.
  String metaFileName = "ZeroEditor.meta";
  Archive archive(ArchiveMode::Decompressing);
  archive.ReadZip(ArchiveReadFlags::Entries, file);
  // Find the meta file's entry
  for(Archive::range range = archive.GetEntries(); !range.Empty(); range.PopFront())
  {
    ArchiveEntry& entry = range.Front();
    if(entry.Name != metaFileName)
      continue;

    // Decompress this entry from the same file and then create the meta cog from the contents
    archive.DecompressEntry(entry, file);
    String metaContents((cstr)entry.Full.Data, entry.Full.Size);

    DataTreeLoader loader;
    Status status;
    loader.OpenBuffer(status, metaContents);
    Cog* metaCog = Z::gFactory->CreateFromStream(Z::gEngine->GetEngineSpace(), loader, 0, nullptr);
    // If we failed to create the meta then maybe we'll get lucky with another entry?
    if(metaCog == nullptr)
      continue;
    // Same with creating the cog but failing to find the build information
    ZeroBuildContent* buildInfo = metaCog->has(ZeroBuildContent);
    if(buildInfo == nullptr)
      continue;

    // Otherwise, we successfully got the build id so we're done
    buildId = buildInfo->GetBuildId();
    zeroBuild.mMetaCog = metaCog;
    return;
  }

  // Otherwise, we failed to find a meta file to load build information from.
  // So first always create a meta cog, then we need to try and extract the build id.
  // In this case we first want to try and parse the old file naming format to extract a build id.
  zeroBuild.CreateMetaIfNull();
  ZeroBuildContent* buildInfo = zeroBuild.mMetaCog->has(ZeroBuildContent);

  String year, month, day;
  bool oldFormatParsed = buildId.Parse(buildPath, year, month, day);
  // If the old parsing method succeeded then make sure to set the release date
  if(oldFormatParsed)
  {
    buildInfo->mChangeSetDate = String::Format("%s.%s.%s", year.c_str(), month.c_str(), day.c_str());
    return;
  }

  // If we failed to parse then we have to generate a unique random name
  buildId = BuildId::GetCurrentLauncherId();
  // First set the branch to mark this as a unique build
  buildId.mExperimentalBranchName = "Custom";

  // Set the major version to a hash of the file
  Array<byte> bytes;
  Zilch::Sha1Builder sha1Builder;
  sha1Builder.Append(file);
  sha1Builder.OutputHash(bytes);
  buildId.mMajorVersion = Math::Abs((int)HashString((char*)bytes.Data(), bytes.Size()));

  // Set the revision id to a hash of the file's date time
  CalendarDateTime dateTime;
  GetFileDateTime(buildPath, dateTime);
  buildId.mRevisionId = Math::Abs((int)HashString((char*)&dateTime, sizeof(CalendarDateTime)));

  // Also set the release date string
  buildInfo->mChangeSetDate = String::Format("%d.%d.%d", dateTime.Year, dateTime.Month + 1, dateTime.Day);
}

Cog* VersionSelector::ExtractLocalTemplateMetaAndImages(Archive& archive, File& file, StringParam metaFileName, bool extractImages)
{
  // Find the meta file as a file with the same name as the sku
  ArchiveEntry* metaFileEntry = FindEntryFromArchive(archive, metaFileName);
  if(metaFileEntry == nullptr)
  {
    // Maybe find another meta file if this failed?
    return nullptr;
  }

  // Load the meta file's data into a string
  archive.DecompressEntry(*metaFileEntry, file);
  String metaContents((cstr)metaFileEntry->Full.Data, metaFileEntry->Full.Size);

  Status status;
  DataTreeLoader loader;
  loader.OpenBuffer(status, metaContents);
  Cog* metaCog = Z::gFactory->CreateFromStream(Z::gEngine->GetEngineSpace(), loader, 0, nullptr);
  // Make sure we successfully created the cog and that it has the required component
  if(metaCog == nullptr)
    return nullptr;
  ZeroTemplate* zeroTemplate = metaCog->has(ZeroTemplate);
  if(zeroTemplate == nullptr)
    return nullptr;

  // If we want to display previews from a template we need to extract them from the
  // archive and put them in the template folder where we can easily load them
  if(extractImages)
  {
    // Make sure the template's directory exists
    String templateInstallDirectory = FilePath::Combine(mConfig->GetTemplateInstallPath(), zeroTemplate->GetFullTemplateVersionName());
    CreateDirectoryAndParents(templateInstallDirectory);

    // Find the icon file in the archive specified by the meta file
    ArchiveEntry* iconEntry = FindEntryFromArchive(archive, zeroTemplate->mIconUrl);
    if(iconEntry != nullptr)
    {
      // Decompress the entry and save it out to the specified relative path
      archive.DecompressEntry(*iconEntry, file);
      String iconFilePathDest = FilePath::Combine(templateInstallDirectory, zeroTemplate->mIconUrl);
      WriteToFile(iconFilePathDest.c_str(), iconEntry->Full.Data, iconEntry->Full.Size);
    }
  }
  return metaCog;
}

ArchiveEntry* VersionSelector::FindEntryFromArchive(Archive& archive, StringParam fileName, bool ignorePath)
{
  // Find the specified file's entry
  for(Archive::range range = archive.GetEntries(); !range.Empty(); range.PopFront())
  {
    ArchiveEntry& entry = range.Front();
    String entryName = entry.Name;
    // Strip the file path from the entry if desired
    if(ignorePath)
      entryName = FilePath::GetFileName(entryName);

    if(entryName == fileName)
      return &entry;
  }
  return nullptr;
}

BackgroundTask* VersionSelector::InstallVersion(ZeroBuild* standalone)
{
  if(standalone->mInstallState == InstallState::Installed || 
     standalone->mInstallState == InstallState::Installing)
    return nullptr;

  ZPrint("Installing build '%s'\n", standalone->GetDebugIdString().c_str());

  //mark the version as now being installed (maybe have this happen on the outside in case something failed?)
  standalone->mInstallState = InstallState::Installing;

  //build the task to start the install
  BuildId buildId = standalone->GetBuildId();
  String url = BuildString(GetLauncherPhpUrl(), "?Commands=RequestBuild&BuildId=", buildId.ToIdString());
  // If we have dev config then add an extra command to get some extra templates (for testing)
  DeveloperConfig* devConfig = mConfig->GetOwner()->has(DeveloperConfig);
  if(devConfig != nullptr)
    url = BuildString(url, "&DevTest");

  DownloadStandaloneTaskJob* job = new DownloadStandaloneTaskJob(url);


  // Get the install path for this build
  standalone->mInstallLocation = GetDefaultInstallLocation(buildId);

  job->mMetaContents = standalone->SaveMetaFileToString();
  job->mInstallLocation = standalone->mInstallLocation;

  //there are several different scenarios that want to install then do
  //something else, because of this just create the task and return it so
  //they can connect to whatever events they want to on the outside
  BackgroundTask* task = Z::gBackgroundTasks->CreateTask(job);
  task->mName = "Latest Install";

  Zero::Connect(task, Events::BackgroundTaskUpdated, standalone, &ZeroBuild::ForwardEvent);
  Zero::Connect(task, Events::BackgroundTaskCompleted, standalone, &ZeroBuild::ForwardEvent);
  Zero::Connect(task, Events::BackgroundTaskCompleted, standalone, &ZeroBuild::InstallCompleted);
  Zero::Connect(task, Events::BackgroundTaskFailed, standalone, &ZeroBuild::ForwardEvent);
  ConnectThisTo(standalone, Events::InstallStarted, OnForwardEvent);
  ConnectThisTo(standalone, Events::InstallCompleted, OnForwardEvent);
  
  Event toSend;
  standalone->DispatchEvent(Events::InstallStarted, &toSend);
  task->Execute();

  return task;
}

BackgroundTask* VersionSelector::DeleteVersion(ZeroBuild* standalone)
{
  ZPrint("Uninstalling build '%s'\n", standalone->GetDebugIdString().c_str());
  standalone->mInstallState = InstallState::Uninstalling;

  // If this build wasn't on the server then we need to move it to the list
  // of unavailable builds (we don't know how to re-install it)
  if(standalone->mOnServer == false)
  {
    mVersions.EraseValue(standalone);
    mVersionMap.Erase(standalone->GetBuildId());

    mOldVersions.PushBack(standalone);
  }
  
  String installLocation = GetInstallLocation(standalone);
  // Delete the directory of the build. Also recursively delete empty
  // parent directories up to the root install location.
  DeleteDirectoryJob* job = new DeleteDirectoryJob(installLocation, GetRootInstallLocation(), true);

  BackgroundTask* task = Z::gBackgroundTasks->CreateTask(job);
  task->mName = "DeleteVersion";

  Zero::Connect(task, Events::BackgroundTaskUpdated, standalone, &ZeroBuild::ForwardEvent);
  Zero::Connect(task, Events::BackgroundTaskCompleted, standalone, &ZeroBuild::ForwardEvent);
  Zero::Connect(task, Events::BackgroundTaskCompleted, standalone, &ZeroBuild::UninstallCompleted);
  Zero::Connect(task, Events::BackgroundTaskFailed, standalone, &ZeroBuild::ForwardEvent);
  
  Event toSend;
  standalone->DispatchEvent(Events::UninstallStarted, &toSend);
  task->Execute();

  return task;
}

bool VersionSelector::CheckForRunningBuild(ZeroBuild* build)
{
  String buildInstallPath = GetInstallExePath(build);

  Array<ProcessInfo> processes;
  GetProcesses(processes);
  // If we find any processes that match the exe path for the given build then return true
  for(size_t i = 0; i < processes.Size(); ++i)
  {
    if(processes[i].mProcessPath == buildInstallPath)
    {
      return true;
    }
  }

  return false;
}

void VersionSelector::ForceCloseRunningBuilds(ZeroBuild* build)
{
  String buildInstallPath = GetInstallExePath(build);

  Array<ProcessInfo> processes;
  GetProcesses(processes);
  // If we find any processes that match the exe path for the given build then kill it
  for(size_t i = 0; i < processes.Size(); ++i)
  {
    if(processes[i].mProcessPath == buildInstallPath)
    {
      KillProcess(processes[i].mProcessId);
    }
  }
  Os::Sleep(100);
}

void VersionSelector::ForceUpdateAllBuilds()
{
  // Copy all of the builds into an array beforehand as uninstalling a build could
  // possibly remove it from the array we're iterating through otherwise
  Array<ZeroBuild*> buildsToRemove = mVersions;
  for(size_t i = 0; i < buildsToRemove.Size(); ++i)
  {
    ZeroBuild* build = buildsToRemove[i];
    if(build->mInstallState == InstallState::NotInstalled)
      continue;

    mReinstallTasks.PushBack(new ReinstallHelper(build, this));
  }
}

BackgroundTask* VersionSelector::CheckForMajorLauncherUpdate()
{
  // Ask if there's a new launcher installer given our current major version
  BuildId currentBuild = BuildId::GetCurrentLauncherId();
  String url = BuildString(GetLauncherPhpUrl(), "?Commands=CheckForMajorLauncherUpdate&MajorId=", ToString(currentBuild.mMajorVersion));

  CheckForLauncherMajorInstallerJob* job = new CheckForLauncherMajorInstallerJob(url);
  job->mName = "Check For Major Installer";

  return Z::gBackgroundTasks->Execute(job, job->mName);
}

BackgroundTask* VersionSelector::DownloadMajorLauncherUpdate()
{
  // Ask if there's a new launcher installer given our current major version
  BuildId currentBuild = BuildId::GetCurrentLauncherId();
  String url = BuildString(GetLauncherPhpUrl(), "?Commands=DownloadMajorLauncherUpdate");

  DownloadLauncherMajorInstallerJob* job = new DownloadLauncherMajorInstallerJob(url);
  job->mName = "Download Major Installer";

  return Z::gBackgroundTasks->Execute(job, job->mName);
}

ZeroBuild* VersionSelector::FindExactVersion(CachedProject* cachedProject)
{
  //if no project file was selected then don't do anything
  if(cachedProject == nullptr)
    return nullptr;

  // Find a build for this project given its build id (full match). If we only display the
  // preferred platform then always try to find the current launcher's platform build.
  BuildId buildId = cachedProject->GetBuildId();
  if(mConfig->mDisplayOnlyPreferredPlatform)
    buildId.mPlatform = GetPlatformString();

  // If we find a matching build then return it
  ZeroBuild* matchingBuild = mVersionMap.FindValue(buildId, nullptr);
  if(matchingBuild != nullptr)
    return matchingBuild;

  // Otherwise there's a bit of a legacy problem here. Builds previously didn't store the
  // changeset as part of it's id. Now that they do, old projects may not have this field saved.
  // If they don't then the hash of BuildId won't resolve properly even though the build may exist. 
  // Do a linear search instead to fix these legacy projects. After loading them they should properly
  // store the changeset and resolve quickly next time.
  for(size_t i = 0; i < mVersions.Size(); ++i)
  {
    ZeroBuild* build = mVersions[i];
    if(build->GetBuildId().CompareBuilds(buildId, true))
      return build;
  }
  return nullptr;
}

ZeroBuild* VersionSelector::GetLatestBuild()
{
  HashSet<String> rejectionTags;
  if(mConfig->mShowDevelopmentBuilds == false)
    rejectionTags.Insert("Develop");
  HashSet<String> legacyTags;
  return GetLatestBuild(legacyTags, rejectionTags);
}

ZeroBuild* VersionSelector::GetLatestBuild(const HashSet<String>& requiredTags, const HashSet<String>& rejectionTags)
{
  // We need to find the first build that satisfies the user tags
  forRange(ZeroBuild* build, mVersions.All())
  {
    // Ignore builds that aren't on the server
    if(!build->mOnServer)
      continue;

    bool containsUserTags = true;
    forRange(String userTag, requiredTags.All())
    {
      if(!build->ContainsTag(userTag))
      {
        containsUserTags = false;
        break;
      }
    }
    bool containsRejectionTag = false;
    forRange(String rejectTag, rejectionTags.All())
    {
      if(build->ContainsTag(rejectTag))
      {
        containsRejectionTag = true;
        break;
      }
    }

    if(containsUserTags && !containsRejectionTag)
      return build;
  }
  
  return nullptr;
}

size_t VersionSelector::GetInstalledBuildsCount() const
{
  size_t count = 0;
  forRange(ZeroBuild* build, mVersions.All())
  {
    if(build->mInstallState == InstallState::Installed)
      ++count;
  }

  return count;
}

void VersionSelector::OnForwardEvent(Event* e)
{
  DispatchEvent(e->EventId, e);
}

void VersionSelector::FindVersionsWithTags(TagSet& activeTags, TagSet& rejectionTags, StringParam activeSearch,
                                           Array<ZeroBuild*>& results, TagSet& resultTags)
{
  ZeroBuildTagPolicy policy;
  FilterDataSetWithTags(activeTags, rejectionTags, activeSearch, mVersions, results, resultTags, policy);
}

void VersionSelector::FindTemplateWithTags(const BuildId& buildId, TagSet& activeTags, TagSet& rejectionTags, StringParam activeSearch,
  Array<TemplateProject*>& results, TagSet& resultTags)
{
  TemplatePackageTagPolicy policy;
  policy.mBuildId = buildId;
  FilterDataSetWithTags(activeTags, rejectionTags, activeSearch, mTemplates, results, resultTags, policy);
}

WarningLevel::Enum VersionSelector::CheckVersionForProject(ZeroBuild* standalone, CachedProject* cachedProject, String& warningString, bool warnForUpgrading)
{
  // If the build was marked bad for any reason then it is a sever warning level
  if(standalone->IsBad() == true)
  {
    warningString = BuildString("Are you sure? ", standalone->GetDeprecatedString());
    return WarningLevel::Severe;
  }

  BuildId buildId = standalone->GetBuildId();
  BuildId projectId = cachedProject->GetBuildId();
  BuildUpdateState::Enum updateState = projectId.CheckForUpdate(buildId);

  // If we cross build branches
  if(updateState == BuildUpdateState::DifferentBranch)
  {
    warningString = "Changing to a different branch. Are you Sure?";
    return WarningLevel::Basic;
  }
  // If we try to go to an older version (breaking changes older as well)
  else if(updateState == BuildUpdateState::Older || updateState == BuildUpdateState::OlderBreaking)
  {
    warningString = "Reverting to an old version. Are you Sure?";
    return WarningLevel::Basic;
  }
  // If we try to update past a major version
  else if(updateState == BuildUpdateState::NewerBreaking)
  {
    warningString = "Upgrading past breaking changes. Are you sure?";
    return WarningLevel::Basic;
  }
  // If we update but no breaking changes should be included.
  else if(updateState == BuildUpdateState::Newer)
  {
    if(warnForUpgrading)
    {
      warningString = "Going to a newer version. This may include breaking changes. Are you sure?";
      return WarningLevel::Basic;
    }
  }

  return WarningLevel::None;
}

void VersionSelector::RunProject(ZeroBuild* standalone, StringParam projectPath)
{
  //get the location to zero
  String zeroExePath = GetInstallExePath(standalone);

  ZPrint("Running project '%s' with build '%s'\n", projectPath.c_str(), zeroExePath.c_str());

  //call zero with our project file
  String commandLine = String::Format("-file \"%s\"", projectPath.c_str());
  Os::SystemOpenFile(zeroExePath.c_str(), Os::Verb::Default, commandLine.c_str());
}

void VersionSelector::RunProject(ZeroBuild* standalone, CachedProject* cachedProject)
{
  String projectFilePath = cachedProject->GetProjectPath();
  RunProject(standalone, projectFilePath);
}

void VersionSelector::RunNewProject(ZeroBuild* standalone, StringParam projectName, Cog* configCog)
{
  //get the location to zero
  String zeroExePath = GetInstallExePath(standalone);

  ZPrint("Running project '%s' with build '%s'\n", projectName.c_str(), zeroExePath.c_str());

  //old versions before the version selector do not understand the -newProject
  //command (because it didn't exist back then) so we have to clear the editing
  //project from the config to achieve a similar result (we can't set the project name though)
  if(standalone->ContainsTag("PreVersionSelector"))
  {
    EditorConfig* editorConfig = HasOrAdd<EditorConfig>(configCog);
    editorConfig->EditingProject = String();

    SaveLauncherConfig(configCog);
  }

  String commandLine = "-newProject";
  //if a project name was specified then set the name of the new project to run
  if(projectName.Empty() == false)
    commandLine = String::Format("-newProject \"%s\"", projectName.c_str());

  Os::SystemOpenFile(zeroExePath.c_str(), Os::Verb::Default, commandLine.c_str());
}

void VersionSelector::MarkVersionValid(ZeroBuild* build)
{
  ZeroBuildDeprecated* deprecatedInfo = build->GetDeprecatedInfo(false);
  if(deprecatedInfo == nullptr)
    return;

  deprecatedInfo->GetOwner()->RemoveComponent(deprecatedInfo);
}

void VersionSelector::MarkVersionInvalid(ZeroBuild* build, StringParam userDescription)
{
  ZeroBuildDeprecated* deprecatedInfo = build->GetDeprecatedInfo(true);
  deprecatedInfo->mUserMessage = userDescription;
}

String VersionSelector::GetRootInstallLocation()
{
  return mConfig->GetBuildsInstallPath();
}

String VersionSelector::GetInstallExePath(ZeroBuild* build)
{
  //get the location to zero
  String installLocation = GetInstallLocation(build);
  return FilePath::Combine(installLocation, "ZeroEditor.exe");
}

String VersionSelector::GetInstallLocation(ZeroBuild* standalone)
{
  return standalone->mInstallLocation;
}

String VersionSelector::GetDefaultInstallLocation(const BuildId& buildId)
{
  // Build the build's path as: Branch/Major.Minor.Patch.RevisionId/Platform
  String branch = buildId.mExperimentalBranchName;
  if(branch.Empty())
    branch = "Master";
  String coreVersionId = buildId.GetMajorMinorPatchRevisionIdString();
  return FilePath::Combine(GetRootInstallLocation(), branch, coreVersionId, buildId.mPlatform);
}

String VersionSelector::GetMarkedBadPath(ZeroBuild* standalone)
{
  String markedBadPath = FilePath::CombineWithExtension(GetInstallLocation(standalone), "VersionMarkedBad", ".txt");
  return markedBadPath;
}

//-------------------------------------------------------------------ReinstallTask
ReinstallHelper::ReinstallHelper(ZeroBuild* build, VersionSelector* versionSelector)
{
  mBuild = build;
  mVersionSelector = versionSelector;

  mVersionSelector->ForceCloseRunningBuilds(build);
  BackgroundTask* task = mVersionSelector->DeleteVersion(build);
  ConnectThisTo(task, Events::BackgroundTaskCompleted, OnReinstall);
}

void ReinstallHelper::OnReinstall(Event* e)
{
  mVersionSelector->InstallVersion(mBuild);
}

}//namespace Zero
