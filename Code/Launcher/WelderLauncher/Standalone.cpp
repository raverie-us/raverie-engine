// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(TemplateProjectPreviewUpdated);
} // namespace Events

void BuildTagSetFromTokenDelimitedList(StringParam tagData, TagSet& tagSet, char delimiter)
{
  tagSet.Clear();

  // split the string
  StringRange range = tagData.All();
  while (!range.Empty())
  {
    Pair<StringRange, StringRange> pair = SplitOnFirst(range, delimiter);
    tagSet.Insert(pair.first.Trim());

    range = pair.second;
    if (!range.Empty() && range.Front() == delimiter)
      range.PopFront();
  }
}

BuildId::BuildId()
{
  mMajorVersion = 0;
  mMinorVersion = 0;
  mPatchVersion = 0;
  mRevisionId = 0;
  mMsSinceEpoch = 0;
}

void BuildId::Serialize(Serializer& stream, bool includePlatform)
{
  SerializeNameDefault(mApplication, String());
  SerializeNameDefault(mBranch, String());
  SerializeNameDefault(mMajorVersion, 0);
  SerializeNameDefault(mMinorVersion, 0);
  SerializeNameDefault(mPatchVersion, 0);
  SerializeNameDefault(mRevisionId, 0);
  SerializeNameDefault(mShortChangeSet, String());
  SerializeNameDefault(mMsSinceEpoch, (u64)0);
  // Projects save this save information but without the platform
  if (includePlatform)
  {
    SerializeNameDefault(mTargetOs, String());
    SerializeNameDefault(mArchitecture, String());
    SerializeNameDefault(mConfig, String());
  }
  SerializeNameDefault(mPackageExtension, String());
}

BuildId BuildId::GetCurrentApplicationId()
{
  BuildId id;
  // This major id needs to match "ZeroLauncherMajorId" on the web server in
  // order to determine if a new major version is available which triggers a
  // re-install of the launcher.

  id.mApplication = GetOrganizationApplicationName(); // Application [WelderEditor]
  id.mBranch = WelderBranchName;                      // Branch [master]
  id.mMajorVersion = GetMajorVersion();               // Major [1]
  id.mMinorVersion = GetMinorVersion();               // Minor [5]
  id.mPatchVersion = GetPatchVersion();               // Patch [0]
  id.mRevisionId = GetRevisionNumber();               // Revision [1501]
  id.mShortChangeSet = GetShortChangeSetString();     // ShortChangeset [fb02756c46a4]
  id.mMsSinceEpoch = WelderMsSinceEpoch;              // MsSinceEpoch [1574702096290]
  id.mTargetOs = WelderTargetOsName;                  // TargetOs [Windows]
  id.mArchitecture = WelderArchitectureName;          // Architecture [x86]
  id.mConfig = WelderConfigName;                      // Config [Release]
  id.mPackageExtension = "zip";                       // Extension [zip]
  return id;
}

bool BuildId::Parse(StringParam buildName)
{
  /*
   * This needs to match
   * index.js:pack/Standalone.cpp:BuildId::Parse/BuildId::GetFullId/BuildVersion.cpp:GetBuildVersionName
   * Application.Branch.Major.Minor.Patch.Revision.ShortChangeset.MsSinceEpoch.TargetOs.Architecture.Config.Extension
   * Example: WelderEditor.master.1.5.0.1501.fb02756c46a4.1574702096290.Windows.x86.Release.zip
   */
  const Regex cBuildNameRegex("([a-zA-Z0-9_]+)\\." // Application [WelderEditor]
                              "([a-zA-Z0-9_]+)\\." // Branch [master]
                              "([0-9]+)\\."        // Major [1]
                              "([0-9]+)\\."        // Minor [5]
                              "([0-9]+)\\."        // Patch [0]
                              "([0-9]+)\\."        // Revision [1501]
                              "([0-9a-fA-F]+)\\."  // ShortChangeset [fb02756c46a4]
                              "([0-9]+)\\."        // MsSinceEpoch [1574702096290]
                              "([a-zA-Z0-9_]+)\\." // TargetOs [Windows]
                              "([a-zA-Z0-9_]+)\\." // Architecture [x86]
                              "([a-zA-Z0-9_]+)\\." // Config [Release]
                              "([a-zA-Z0-9_]+)"    // Extension [zip]
  );

  Matches matches;
  cBuildNameRegex.Search(buildName, matches);
  // Make sure the regular expression matched (if not, it may be some other
  // release that's not a zerobuild).
  if (matches.Empty())
    return false;

  // 1           2      3     4     5     6        7              8            9        10           11     12
  // Application.Branch.Major.Minor.Patch.Revision.ShortChangeset.MsSinceEpoch.TargetOs.Architecture.Config.Extension

  mApplication = matches[1];
  mBranch = matches[2];
  ToValue(matches[3], mMajorVersion);
  ToValue(matches[4], mMinorVersion);
  ToValue(matches[5], mPatchVersion);
  ToValue(matches[6], mRevisionId);
  mShortChangeSet = matches[7];
  ToValue(matches[8], mMsSinceEpoch);
  mTargetOs = matches[9];
  mArchitecture = matches[10];
  mConfig = matches[11];
  mPackageExtension = matches[12];
  return true;
}

bool BuildId::ParseRequired(StringParam buildName)
{
  if (Parse(buildName))
    return true;
  DoNotifyError("Failed", "Unable to parse build name " + buildName);
  return false;
}

String BuildId::GetFullId() const
{
  /*
   * This needs to match
   * index.js:pack/Standalone.cpp:BuildId::Parse/BuildId::GetFullId/BuildVersion.cpp:GetBuildVersionName
   * Application.Branch.Major.Minor.Patch.Revision.ShortChangeset.MsSinceEpoch.TargetOs.Architecture.Config.Extension
   * Example: WelderEditor.master.1.5.0.1501.fb02756c46a4.1574702096290.Windows.x86.Release.zip
   */
  StringBuilder builder;
  builder.AppendFormat("%s.", mApplication.c_str());     // Application [WelderEditor]
  builder.AppendFormat("%s.", mBranch.c_str());          // Branch [master]
  builder.AppendFormat("%d.", mMajorVersion);            // Major [1]
  builder.AppendFormat("%d.", mMinorVersion);            // Minor [5]
  builder.AppendFormat("%d.", mPatchVersion);            // Patch [0]
  builder.AppendFormat("%d.", mRevisionId);              // Revision [1501]
  builder.AppendFormat("%s.", mShortChangeSet.c_str());  // ShortChangeset [fb02756c46a4]
  builder.AppendFormat("%llu.", mMsSinceEpoch);          // MsSinceEpoch [1574702096290]
  builder.AppendFormat("%s.", mTargetOs.c_str());        // TargetOs [Windows]
  builder.AppendFormat("%s.", mArchitecture.c_str());    // Architecture [x86]
  builder.AppendFormat("%s.", mConfig.c_str());          // Config [Release]
  builder.AppendFormat("%s", mPackageExtension.c_str()); // Extension [zip]
  return builder.ToString();
}

String BuildId::GetVersionString() const
{
  StringBuilder builder;
  builder.AppendFormat("%d.", mMajorVersion); // Major [1]
  builder.AppendFormat("%d.", mMinorVersion); // Minor [5]
  builder.AppendFormat("%d.", mPatchVersion); // Patch [0]
  builder.AppendFormat("%d", mRevisionId);    // Revision [1501]
  return builder.ToString();
}

bool BuildId::IsEmpty() const
{
  return CompareBuilds(BuildId());
}

size_t BuildId::Hash() const
{
  // Use the full display string hash
  return GetFullId().Hash();
}

bool BuildId::operator==(const BuildId& rhs) const
{
  // Compare the builds (no legacy)
  return CompareBuilds(rhs);
}

bool BuildId::operator!=(const BuildId& rhs) const
{
  return !(*this == rhs);
}

bool BuildId::CompareBuilds(const BuildId& rhs) const
{
  return GetFullId() == rhs.GetFullId();
}

BuildUpdateState::Enum BuildId::CheckForUpdate(const BuildId& rhs) const
{
  if (!mApplication.Empty() && !rhs.mApplication.Empty() && mApplication != rhs.mApplication)
    return BuildUpdateState::DifferentApplication;

  // Check if the branches are different
  if (!mBranch.Empty() && !rhs.mBranch.Empty() && mBranch != rhs.mBranch)
    return BuildUpdateState::DifferentBranch;

  // Check for changes in the major version
  if (mMajorVersion < rhs.mMajorVersion)
    return BuildUpdateState::NewerBreaking;
  if (mMajorVersion > rhs.mMajorVersion)
    return BuildUpdateState::OlderBreaking;
  // Check for changes in the minor version
  if (mMinorVersion < rhs.mMinorVersion)
    return BuildUpdateState::Newer;
  if (mMinorVersion > rhs.mMinorVersion)
    return BuildUpdateState::Older;
  // Check for changes in the patch version
  if (mPatchVersion < rhs.mPatchVersion)
    return BuildUpdateState::Newer;
  if (mPatchVersion > rhs.mPatchVersion)
    return BuildUpdateState::Older;
  // Check for changes in the revision id
  if (mRevisionId < rhs.mRevisionId)
    return BuildUpdateState::Newer;
  if (mRevisionId > rhs.mRevisionId)
    return BuildUpdateState::Older;
  return BuildUpdateState::Same;
}

bool BuildId::IsOlderThan(const BuildId& rhs) const
{
  BuildUpdateState::Enum upgradeState = CheckForUpdate(rhs);
  if (upgradeState == BuildUpdateState::Newer || upgradeState == BuildUpdateState::NewerBreaking)
    return true;
  return false;
}

bool BuildId::IsPlatformEmpty() const
{
  return mTargetOs.Empty() && mArchitecture.Empty();
}

bool BuildId::IsForThisPlatform() const
{
  return mTargetOs == WelderTargetOsName && mArchitecture == WelderArchitectureName && mConfig == WelderConfigName;
}

void BuildId::SetToThisPlatform()
{
  mTargetOs = WelderTargetOsName;
  mArchitecture = WelderArchitectureName;
  mConfig = WelderConfigName;
}

String BuildId::GetChangeSetDate() const
{
  char buffer[256];
  time_t time = mMsSinceEpoch / 1000;
  auto tm = localtime(&time);
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", tm);
  return buffer;
}

String BuildId::GetMasterBranch()
{
  static const String cMaster("master");
  return cMaster;
}

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
  if (createIfNull)
    CreateMetaIfNull();

  if (mMetaCog == nullptr)
    return nullptr;
  return mMetaCog->has(ZeroBuildContent);
}

ZeroBuildDeprecated* ZeroBuild::GetDeprecatedInfo(bool createIfNull)
{
  if (createIfNull)
  {
    CreateMetaIfNull();
    return HasOrAdd<ZeroBuildDeprecated>(mMetaCog);
  }

  if (mMetaCog == nullptr)
    return nullptr;
  return mMetaCog->has(ZeroBuildDeprecated);
}

String ZeroBuild::GetDisplayString()
{
  return GetBuildId().GetVersionString();
}

String ZeroBuild::GetDebugIdString()
{
  return GetBuildId().GetFullId();
}

BuildId ZeroBuild::GetBuildId()
{
  // If we have a cog with the build content component on it then return its
  // build id, otherwise just return the current launcher's id
  ZeroBuildContent* zeroBuild = GetBuildContent(false);
  if (zeroBuild != nullptr)
    return zeroBuild->GetBuildId();
  return BuildId::GetCurrentApplicationId();
}

void ZeroBuild::SetBuildId(const BuildId& buildId)
{
  ZeroBuildContent* buildContent = GetBuildContent(true);
  buildContent->SetBuildId(buildId);
}

String ZeroBuild::GetDownloadUrl()
{
  ZeroBuildContent* zeroBuild = GetBuildContent(false);
  if (zeroBuild != nullptr)
    return zeroBuild->mDownloadUrl;
  return String();
}

bool ZeroBuild::CreateMetaIfNull()
{
  bool metaIsNull = mMetaCog == nullptr;
  if (metaIsNull)
  {
    mMetaCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), CoreArchetypes::Empty, 0, nullptr);
    mMetaCog->ClearArchetype();
  }

  // Make sure the metaCog always has a ZeroBuildContent component
  HasOrAdd<ZeroBuildContent>(mMetaCog);
  return metaIsNull;
}

bool ZeroBuild::IsBad()
{
  return GetDeprecatedInfo(false) != nullptr;
}

String ZeroBuild::GetReleaseNotes()
{
  if (mMetaCog != nullptr)
  {
    ZeroBuildReleaseNotes* releaseNotes = mMetaCog->has(ZeroBuildReleaseNotes);
    if (releaseNotes != nullptr)
      return releaseNotes->mNotes;
  }
  return String();
}

String ZeroBuild::GetDeprecatedString()
{
  ZeroBuildDeprecated* deprecatedInfo = GetDeprecatedInfo(false);
  if (deprecatedInfo == nullptr)
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
  if (createIfNull)
    CreateMetaIfNull();

  if (mMetaCog == nullptr)
    return nullptr;
  return mMetaCog->has(ZeroTemplate);
}

bool TemplateProject::CreateMetaIfNull()
{
  bool metaIsNull = mMetaCog == nullptr;
  if (metaIsNull)
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
  return zeroTemplate->mDownloadUrl;
}

String TemplateProject::GetIconUrl()
{
  ZeroTemplate* zeroTemplate = GetZeroTemplate(true);
  return zeroTemplate->mIconUrl;
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
  // When we finish downloading the preview image we need to update our texture
  // (on the main thread)
  BackgroundTask* task = versionSelector->DownloadTemplateIcon(this);
  ConnectThisTo(task, Events::BackgroundTaskCompleted, OnPreviewImageDownloaded);
}

void TemplateProject::LoadLocalImages()
{
  ZeroTemplate* zeroTemplate = GetZeroTemplate(true);
  String iconPath = FilePath::Combine(mLocalPath, zeroTemplate->mIconUrl);
  // If a valid icon path exists then queue up a job to load it
  if (!zeroTemplate->mIconUrl.Empty() && FileExists(iconPath))
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

  // If we got a valid image then load it (different than having a valid or
  // complete job)
  if (!job->mImageWasInvalid)
  {
    // This event can get dispatched multiple times. If the image is empty then
    // don't swap again
    if (job->mImage.SizeInBytes == 0)
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

} // namespace Zero
