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
DeclareEvent(TemplateProjectPreviewUpdated);
}//namespace Events

DeclareEnum4(InstallState, NotInstalled, Installing, Installed, Uninstalling);
// How different two build ids are (typically for project to build)
DeclareEnum6(BuildUpdateState, DifferentBranch, Older, Newer, NewerBreaking, OlderBreaking, Same);

typedef HashSet<String> TagSet;
void BuildTagSetFromTokenDelimitedList(StringParam tagData, TagSet& tagSet, char delimiter);

//-------------------------------------------------------------------BuildId
/// Common storage and querying of a build's id. Allows easy searching,
/// displaying, serialization, and updating of what the id of a build is (in case new information is needed).
class BuildId
{
public:
  BuildId();

  /// This conditionally saves the platform so that builds and projects can
  /// avoid re-writing the save logic as projects don't save the platform.
  void Serialize(Serializer& stream, bool includePlatform);

  /// Get a build id for the current version of the launcher.
  /// Mostly used as a fallback method for some cases that should never happen.
  static BuildId GetCurrentLauncherId();

  /// Legacy for when no meta exists. Parse a build's name and extract id information.
  bool Parse(StringParam buildName);
  /// Same as above, but also saves the date-time information while setting the revision id.
  bool Parse(StringParam buildName, String& year, String& month, String& day);

  /// Get the display string for this build. Optionally show
  /// the platform (depending on various settings).
  String ToDisplayString(bool showPlatform = false) const;
  /// returns the string of all the basic version numbers put together.
  /// Currently used to create the folder for the build.
  String GetMajorMinorPatchRevisionIdString() const;
  /// Get a unique string that will always be consistent.
  /// Needed for uniquely identifying/hashing a build.
  String ToIdString() const;
  /// Returns the full unique id of a build. Parts of the id can be
  /// disabled for things like display names and whatnot. A shared helper function.
  String GetFullId(bool showExperimentalBranch, bool showChangeset, bool showPlatform) const;

  // HashPolicy Interface
  size_t Hash() const;
  bool operator==(const BuildId& rhs) const;
  bool operator!=(const BuildId& rhs) const;
  bool CompareBuilds(const BuildId& rhs, bool legacyCompare) const;

  // What kind of a change is updating from this to rhs. 
  BuildUpdateState::Enum CheckForUpdate(const BuildId& rhs) const;

  String mExperimentalBranchName;
  String mPlatform;
  int mMajorVersion;
  int mMinorVersion;
  int mPatchVersion;
  int mRevisionId;
  String mShortChangeSet;
};

//-------------------------------------------------------------------ZeroBuild
/// Represents a standalone "installer" of zero that is either installed locally or on the server.
class ZeroBuild : public EventObject
{
public:
  ZeroBuild();
  
  void ForwardEvent(Event* e);
  void InstallCompleted(Event* e);
  void UninstallCompleted(Event* e);

  /// Get the build content component
  ZeroBuildContent* GetBuildContent(bool createIfNull);
  ZeroBuildDeprecated* GetDeprecatedInfo(bool createIfNull);

  /// A display string for this build. Note: this is not safe as a
  /// unique identifier. Use the BuildId directly as a unique Id.
  String GetDisplayString(bool showPlatform = false);
  /// A string for debug printing the build id
  String GetDebugIdString();

  BuildId GetBuildId();
  void SetBuildId(const BuildId& buildId);

  /// Save the stored meta cog to a file
  void SaveMetaFile(StringParam filePath);
  /// Save the store meta cog to a string. Needed to safely run on another
  // thread while serialization isn't thread-safe.
  String SaveMetaFileToString();
  static String GetMetaFilePath(StringParam basePath);

  // If we don't already have a meta cog, then create one from the empty archetype
  // and make sure it has all of the necessary components. Returns true if the meta was null.
  bool CreateMetaIfNull();

  // Given a legacy build name, parse out the date-time and revision id.
  // Returns false if the build's name didn't match the legacy pattern.
  static bool ParseLegacyBuildName(StringParam buildName, String& year, String& month, String& day, String& revisionId);
  void GetReleaseDate(String& year, String& month, String& day);

  bool IsBad();

  String GetReleaseNotes();
  String GetDeprecatedString();
  String GetTagsString();
  bool ContainsTag(StringParam tag);

  // No copying allowed
private:
  ZeroBuild(const ZeroBuild& rhs) {};
  void operator=(const ZeroBuild& rhs) {};
public:

  InstallState::Enum mInstallState;
  bool mOnServer;

  /// A cog representing the meta for this build. This cog is assumed to never die except on shutdown.
  Cog* mMetaCog;

  /// The folder where this build is installed (if it's been installed), aka the folder with ZeroEditor.exe.
  String mInstallLocation;

  // The extension used for builds. Makes it easier to change all at once.
  const static String mExtension;
  const static String mDeprecatedTag;
};

/// Represents information about a template project for the user. This may reside locally or on the server.
class TemplateProject : public EventObject
{
public:
  typedef TemplateProject ZilchSelf;
  
  TemplateProject();

  /// Get the serialized data representing this template
  ZeroTemplate* GetZeroTemplate(bool createIfNull);
  /// If we don't already have a meta cog, then create one from the empty archetype
  /// and make sure it has all of the necessary components. Returns true if the meta was null.
  bool CreateMetaIfNull();
  /// Saves the meta file to a string so that it can be saved out somewhere.
  /// Typically used to save on another thread where serialization doesn't currently work.
  String SaveMetaFileToString();

  /// The key used to hash template projects (built from the SKU and VersionIdString).
  String GetIdString();
  /// The name to display to users for the template.
  String GetDisplayName();
  /// The server location of the template.
  String GetTemplateUrl();
  /// The server location of the preview icon
  String GetIconUrl();
  /// A bias to sort some templates before others (such as the 2d project)
  float GetSortPriority();
  /// The name of the template file if it has been downloaded.
  String GetLocalTemplateFileName();
  /// Returns the path to the installed template file. If the template wasn't installed then this path will be invalid.
  String GetInstalledTemplatePath();

  /// Can this template project run with the given build id?
  bool ContainsVersion(const BuildId& buildId);
  /// Downloads the icon image from the server
  void DownloadIcon(VersionSelector* versionSelector);
  /// Loads all locally downloaded images from the disk
  void LoadLocalImages();
  void OnPreviewImageDownloaded(BackgroundTaskEvent* e);

  /// The local path to the template package (if downloaded)
  String mLocalPath;

  bool mIsOnServer;
  bool mIsDownloaded;
  bool mIsDifferentFromServer;

  /// A cog representing the meta for this project. This cog is assumed to never die except on shutdown.
  Cog* mMetaCog;
  
  /// Cached information for the display icon
  Image mIconImage;
  HandleOf<Texture> mIconTexture;

  // The extension used for template projects. Makes it easier to change all at once.
  const static String mExtensionWithDot;
  // A few places need the extension without the dot due to how file path currently works
  // (needs the '.' to combine but returns without the '.' when parsing)
  const static String mExtensionWithoutDot;
  const static String mBackupIconTextureName;

private:
  TemplateProject(const TemplateProject& rhs) {};
  void operator=(const TemplateProject& rhs) {};
};

}//namespace Zero
