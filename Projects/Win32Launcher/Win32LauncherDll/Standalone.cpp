///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Josh Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(TemplateProjectPreviewUpdated);
}//namespace Events

void BuildTagSetFromTokenDelimitedList(StringParam tagData, TagSet& tagSet, char delimiter)
{
  tagSet.Clear();

  //split the string
  StringRange range = tagData.All();
  while(!range.Empty())
  {
    Pair<StringRange, StringRange> pair = SplitOnFirst(range, delimiter);
    tagSet.Insert(pair.first.Trim());

    range = pair.second;
    if(!range.Empty() && range.Front() == delimiter)
      range.PopFront();
  }
}

//-------------------------------------------------------------------BuildId
BuildId::BuildId()
{
  mMajorVersion = 0;
  mMinorVersion = 0;
  mPatchVersion = 0;
  mRevisionId = 0;
}

void BuildId::Serialize(Serializer& stream, bool includePlatform)
{
  SerializeNameDefault(mExperimentalBranchName, String());
  SerializeNameDefault(mMajorVersion, 0);
  SerializeNameDefault(mMinorVersion, 0);
  SerializeNameDefault(mPatchVersion, 0);
  SerializeNameDefault(mRevisionId, 0);
  SerializeNameDefault(mShortChangeSet, String());
  // Projects save this save information but without the platform
  if(includePlatform)
    SerializeNameDefault(mPlatform, String());
}

BuildId BuildId::GetCurrentLauncherId()
{
  BuildId id;
  // This major id needs to match "ZeroLauncherMajorId" on the web server in order to determine
  // if a new major version is available which triggers a re-install of the launcher.
  id.mMajorVersion = GetLauncherMajorVersion();
  id.mMinorVersion = 0;
  id.mPatchVersion = 0;
  id.mRevisionId = GetRevisionNumber();
  id.mPlatform = GetPlatformString();
  id.mShortChangeSet = GetShortChangeSetString();
  return id;
}

bool BuildId::Parse(StringParam buildName)
{
  // Match ZeroEngineSetup.#.#.#.RevisionId
  String year, month, day, revisionId;
  return Parse(buildName, year, month, day);
}

bool BuildId::Parse(StringParam buildName, String& year, String& month, String& day)
{
  // Match ZeroEngineSetup.#.#.#.RevisionId
  String revisionId;
  bool parsed = ZeroBuild::ParseLegacyBuildName(buildName, year, month, day, revisionId);
  if(!parsed)
    return false;

  mExperimentalBranchName = String();
  mMajorVersion = 0;
  mMinorVersion = 0;
  mPatchVersion = 0;

  ToValue(revisionId, mRevisionId);
  mPlatform = GetPlatformString();
  return true;
}

String BuildId::ToIdString() const
{
  // To have a stable id we have to always use the full
  // name (including platform) to avoid conflicts
  return GetFullId(true, true, true);
}

String BuildId::ToDisplayString(bool showPlatform) const
{
  return GetFullId(true, false, showPlatform);
}

String BuildId::GetMajorMinorPatchRevisionIdString() const
{
  return GetFullId(false, true, false);
}

String BuildId::GetFullId(bool showExperimentalBranch, bool showChangeset, bool showPlatform) const
{
  StringBuilder builder;

  // If we have a branch then start with that
  if(showExperimentalBranch && !mExperimentalBranchName.Empty())
    builder.AppendFormat("%s.", mExperimentalBranchName.c_str());

  // Append the main 4 id numbers
  builder.AppendFormat("%d.", mMajorVersion);
  builder.AppendFormat("%d.", mMinorVersion);
  builder.AppendFormat("%d.", mPatchVersion);
  builder.AppendFormat("%d", mRevisionId);
  if(showChangeset && !mShortChangeSet.Empty())
    builder.AppendFormat(".%s", mShortChangeSet.c_str());

  // If we display the platform then add that in too
  if(showPlatform)
    builder.AppendFormat("-%s", mPlatform.c_str());

  return builder.ToString();
}

size_t BuildId::Hash() const
{
  // Use the full display string hash
  String id = GetFullId(true, true, false);
  return id.Hash();
}

bool BuildId::operator==(const BuildId& rhs) const
{
  // Compare the builds (no legacy)
  return CompareBuilds(rhs, false);
}

bool BuildId::operator!=(const BuildId& rhs) const
{
  return !(*this == rhs);
}

bool BuildId::CompareBuilds(const BuildId& rhs, bool legacyCompare) const
{
  // If anything doesn't match then we aren't equal
  if(mExperimentalBranchName != rhs.mExperimentalBranchName)
    return false;
  if(mMajorVersion != rhs.mMajorVersion)
    return false;
  if(mMinorVersion != rhs.mMinorVersion)
    return false;
  if(mPatchVersion != rhs.mPatchVersion)
    return false;
  if(mRevisionId != rhs.mRevisionId)
    return false;
  // Legacy builds didn't store the changeset. To properly resolve old projects
  // to their builds we have to compare without the changeset.
  if(!legacyCompare && mShortChangeSet != rhs.mShortChangeSet)
    return false;
  if(mPlatform != rhs.mPlatform)
    return false;
  return true;
}

BuildUpdateState::Enum BuildId::CheckForUpdate(const BuildId& rhs) const
{
  // Check if the branches are different
  if(mExperimentalBranchName != rhs.mExperimentalBranchName)
    return BuildUpdateState::DifferentBranch;

  // Check for changes in the major version
  if(mMajorVersion < rhs.mMajorVersion)
    return BuildUpdateState::NewerBreaking;
  if(mMajorVersion > rhs.mMajorVersion)
    return BuildUpdateState::OlderBreaking;
  // Check for changes in the minor version
  if(mMinorVersion < rhs.mMinorVersion)
    return BuildUpdateState::Newer;
  if(mMinorVersion > rhs.mMinorVersion)
    return BuildUpdateState::Older;
  // Check for changes in the patch version
  if(mPatchVersion < rhs.mPatchVersion)
    return BuildUpdateState::Newer;
  if(mPatchVersion > rhs.mPatchVersion)
    return BuildUpdateState::Older;
  // Check for changes in the revision id
  if(mRevisionId < rhs.mRevisionId)
    return BuildUpdateState::Newer;
  if(mRevisionId > rhs.mRevisionId)
    return BuildUpdateState::Older;
  return BuildUpdateState::Same;
}

//-------------------------------------------------------------------ZeroBuild
const String ZeroBuild::mExtension = "zerobuild";
const String ZeroBuild::mDeprecatedTag = "Deprecated";

ZeroBuild::ZeroBuild()
{
  mInstallState = InstallState::NotInstalled;
  mOnServer = false;
  mMetaCog = nullptr;
}

void ZeroBuild::ForwardEvent(Event* e)
{
  DispatchEvent(e->EventId, e);
}

void ZeroBuild::InstallCompleted(Event* e)
{
  ZPrint("Build '%s' install completed\n", GetDebugIdString().c_str());
  mInstallState = InstallState::Installed;
  DispatchEvent(Events::InstallCompleted, e);
}

void ZeroBuild::UninstallCompleted(Event* e)
{
  ZPrint("Build '%s' uninstall completed\n", GetDebugIdString().c_str());
  mInstallState = InstallState::NotInstalled;
  DispatchEvent(Events::UninstallCompleted, e);
}

ZeroBuildContent* ZeroBuild::GetBuildContent(bool createIfNull)
{
  if(createIfNull)
    CreateMetaIfNull();

  if(mMetaCog == nullptr)
    return nullptr;
  return mMetaCog->has(ZeroBuildContent);
}

ZeroBuildDeprecated* ZeroBuild::GetDeprecatedInfo(bool createIfNull)
{
  if(createIfNull)
  {
    CreateMetaIfNull();
    return HasOrAdd<ZeroBuildDeprecated>(mMetaCog);
  }
  
  if(mMetaCog == nullptr)
    return nullptr;
  return mMetaCog->has(ZeroBuildDeprecated);
}

String ZeroBuild::GetDisplayString(bool showPlatform)
{
  return GetBuildId().ToDisplayString(showPlatform);
}

String ZeroBuild::GetDebugIdString()
{
  return GetBuildId().ToIdString();
}

BuildId ZeroBuild::GetBuildId()
{
  // If we have a cog with the build content component on it then return its build id,
  // otherwise just return the current launcher's id
  ZeroBuildContent* zeroBuild = GetBuildContent(false);
  if(zeroBuild != nullptr)
    return zeroBuild->GetBuildId();
  return BuildId::GetCurrentLauncherId();
}

void ZeroBuild::SetBuildId(const BuildId& buildId)
{
  ZeroBuildContent* buildContent = GetBuildContent(true);
  buildContent->SetBuildId(buildId);
}


void ZeroBuild::SaveMetaFile(StringParam filePath)
{
  // Save our meta cog to a file
  Status status;
  ObjectSaver saver;
  saver.Open(status, filePath.c_str());
  saver.SaveDefinition(mMetaCog);
}

String ZeroBuild::SaveMetaFileToString()
{
  // Save our meta cog to a string. Needed for creating a project on another
  // thread as serialization is not currently thread safe.
  ObjectSaver saver;
  saver.OpenBuffer();
  saver.SaveDefinition(mMetaCog);
  return saver.GetString();
}

String ZeroBuild::GetMetaFilePath(StringParam basePath)
{
  return FilePath::CombineWithExtension(basePath, "ZeroEditor", ".meta");
}

bool ZeroBuild::CreateMetaIfNull()
{
  bool metaIsNull = mMetaCog == nullptr;
  if(metaIsNull)
  {
    mMetaCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), CoreArchetypes::Empty, 0, nullptr);
    mMetaCog->ClearArchetype();
  }

  // Make sure the metaCog always has a ZeroBuildContent component
  HasOrAdd<ZeroBuildContent>(mMetaCog);
  return metaIsNull;
}

bool ZeroBuild::ParseLegacyBuildName(StringParam buildName, String& year, String& month, String& day, String& revisionId)
{
  Matches matches;
  Regex regex("\\w*\\.(\\d+)\\.(\\d+)\\.(\\d+)\\.(\\d+)");
  regex.Search(buildName, matches);
  if(matches.Size() == 5)
  {
    year = matches[1];
    month = matches[2];
    day = matches[3];
    revisionId = matches[4];
    return true;
  }
  return false;
}

void ZeroBuild::GetReleaseDate(String& year, String& month, String& day)
{
  // Get the build content component and split the release date field on it
  ZeroBuildContent* zeroBuild = GetBuildContent(true);
  
  Matches matches;
  Regex regex("(\\d+)\\-(\\d+)\\-(\\d+)");
  regex.Search(zeroBuild->mChangeSetDate, matches);
  if(matches.Size() == 4)
  {
    year = matches[1];
    month = matches[2];
    day = matches[3];
  }
}

bool ZeroBuild::IsBad()
{
  return GetDeprecatedInfo(false) != nullptr;
}

String ZeroBuild::GetReleaseNotes()
{
  if(mMetaCog != nullptr)
  {
    ZeroBuildReleaseNotes* releaseNotes = mMetaCog->has(ZeroBuildReleaseNotes);
    if(releaseNotes != nullptr)
      return releaseNotes->mNotes;
  }
  return String();
}

String ZeroBuild::GetDeprecatedString()
{
  ZeroBuildDeprecated* deprecatedInfo = GetDeprecatedInfo(false);
  if(deprecatedInfo == nullptr)
    return String();

  return deprecatedInfo->GetDeprecatedString();
}

String ZeroBuild::GetTagsString()
{
  return GetBuildContent(true)->GetTagString();
}

bool ZeroBuild::ContainsTag(StringParam tag)
{
  return GetBuildContent(true)->ContainsTag(tag);
}

//-------------------------------------------------------------------TemplateProject
const String TemplateProject::mExtensionWithoutDot = "zerotemplate";
const String TemplateProject::mExtensionWithDot = ".zerotemplate";
const String TemplateProject::mBackupIconTextureName = "BackupProjectIcon";

TemplateProject::TemplateProject()
{
  mIsOnServer = false;
  mIsDownloaded = false;
  mIsDifferentFromServer = false;
}

ZeroTemplate* TemplateProject::GetZeroTemplate(bool createIfNull)
{
  if(createIfNull)
    CreateMetaIfNull();

  if(mMetaCog == nullptr)
    return nullptr;
  return mMetaCog->has(ZeroTemplate);
}

bool TemplateProject::CreateMetaIfNull()
{
  bool metaIsNull = mMetaCog == nullptr;
  if(metaIsNull)
  {
    mMetaCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), CoreArchetypes::Empty, 0, nullptr);
    mMetaCog->ClearArchetype();
  }

  // Make sure the metaCog always has a ZeroTemplate component
  HasOrAdd<ZeroTemplate>(mMetaCog);
  return metaIsNull;
}

String TemplateProject::SaveMetaFileToString()
{
  // Save our meta cog to a string. Needed for creating a project on another
  // thread as serialization is not currently thread safe.
  ObjectSaver saver;
  saver.OpenBuffer();
  saver.SaveDefinition(mMetaCog);
  return saver.GetString();
}

String TemplateProject::GetIdString()
{
  ZeroTemplate* zeroTemplate = GetZeroTemplate(true);
  return zeroTemplate->GetIdString();
}

String TemplateProject::GetDisplayName()
{
  ZeroTemplate* zeroTemplate = GetZeroTemplate(true);
  return zeroTemplate->mDisplayName;
}

String TemplateProject::GetTemplateUrl()
{
  ZeroTemplate* zeroTemplate = GetZeroTemplate(true);
  return BuildString(zeroTemplate->mTemplatePath, "/", zeroTemplate->mTemplateFileName);
}

String TemplateProject::GetIconUrl()
{
  ZeroTemplate* zeroTemplate = GetZeroTemplate(true);
  return BuildString(zeroTemplate->mTemplatePath, "/", zeroTemplate->mIconUrl);
}

float TemplateProject::GetSortPriority()
{
  ZeroTemplate* zeroTemplate = GetZeroTemplate(true);
  return zeroTemplate->mSortPriority;
}

String TemplateProject::GetLocalTemplateFileName()
{
  ZeroTemplate* zeroTemplate = GetZeroTemplate(true);
  return BuildString(zeroTemplate->GetFullTemplateVersionName(), TemplateProject::mExtensionWithDot);
}

String TemplateProject::GetInstalledTemplatePath()
{
  ZeroTemplate* zeroTemplate = GetZeroTemplate(true);
  return FilePath::Combine(mLocalPath, GetLocalTemplateFileName());
}

bool TemplateProject::ContainsVersion(const BuildId& buildId)
{
  ZeroTemplate* zeroTemplate = GetZeroTemplate(true);
  return zeroTemplate->TestBuildId(buildId);
}

void TemplateProject::DownloadIcon(VersionSelector* versionSelector)
{
  // When we finish downloading the preview image we need to update our texture (on the main thread)
  BackgroundTask* task = versionSelector->DownloadTemplateIcon(this);
  ConnectThisTo(task, Events::BackgroundTaskCompleted, OnPreviewImageDownloaded);
}

void TemplateProject::LoadLocalImages()
{
  ZeroTemplate* zeroTemplate = GetZeroTemplate(true);
  String iconPath = FilePath::Combine(mLocalPath, zeroTemplate->mIconUrl);
  // If a valid icon path exists then queue up a job to load it
  if(!zeroTemplate->mIconUrl.Empty() && FileExists(iconPath))
  {
    LoadImageFromDiskTaskJob* job = new LoadImageFromDiskTaskJob(iconPath);
    BackgroundTask* task = Z::gBackgroundTasks->Execute(job, "Load preview");
    ConnectThisTo(task, Events::BackgroundTaskCompleted, OnPreviewImageDownloaded);
  }
  else
  {
    mIconTexture = TextureManager::Find(mBackupIconTextureName);
  }
}

void TemplateProject::OnPreviewImageDownloaded(BackgroundTaskEvent* e)
{
  DownloadImageTaskJob* job = (DownloadImageTaskJob*)e->mTask->GetFinishedJob();

  // If we got a valid image then load it (different than having a valid or complete job)
  if(!job->mImageWasInvalid)
  {
    // This event can get dispatched multiple times. If the image is empty then don't swap again
    if(job->mImage.SizeInBytes == 0)
      return;

    // Load the image into a texture (must be done on the main thread)
    mIconImage.Swap(&job->mImage);

    Image& image = mIconImage;
    mIconTexture = Texture::CreateRuntime();
    mIconTexture->Upload(image);
    
  }
  // Otherwise, use the backup icon texture
  else
    mIconTexture = TextureManager::Find(mBackupIconTextureName);
  
  // Tell whoever cares that we have a new preview image
  Event toSend;
  DispatchEvent(Events::TemplateProjectPreviewUpdated, &toSend);
}

}//namespace Zero
