// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace Events
{
DeclareEvent(ProjectLoaded);
DeclareEvent(NoProjectLoaded);
} // namespace Events

/// Project component store primary data for a project. Projects are separate
/// games with their own content and settings.
class ProjectSettings : public Component
{
public:
  RaverieDeclareType(ProjectSettings, TypeCopyMode::ReferenceType);

  /// Constructor / Destructor.
  ProjectSettings();
  ~ProjectSettings();

  /// Component Interface.
  void Serialize(Serializer& stream) override;

  /// Returns the Project's Name
  String GetProjectName();

  /// Returns the path to the folder containing this project.
  String GetProjectFolder();

  /// Returns the path to the generated content folder for this project.
  String GetContentFolder();

  /// Returns the path to the content folder containing editor specific content
  /// (screen shot, editor settings, etc...).
  String GetEditorContentFolder();

  void Save();

  /// The full path including file name and extension to the screen shot file.
  String GetScreenshotFile();

  /// Whether or not any screen shot is available (currently loaded or not).
  bool ScreenshotAvailable();

  /// Returns the screen shot image for this project. If 'forceReload' is false,
  /// it will only load a new image if it has been modified, otherwise it
  /// will return the previously loaded image.
  Image* GetScreenshot(bool forceReload);

  /// Whether or not a screen shot has been loaded into memory.
  bool ScreenshotLoaded();

  /// If the screen shot has been modified since it was last loaded into memory.
  bool NewScreenshotAvailable();

  /// Takes the given image and saves it to the screen shot file.
  void SaveScreenshotFromImage(Image& image);

  /// Generates a guid for this project. Used for unique project identification.
  void GenerateProjectGuid();

  /// Returns the guid for this project. Used for unique project identification.
  Guid GetProjectGuid();

  /// Name of the project.
  String ProjectName;
  /// Owner of the project.
  String ProjectOwner;

  /// Now deprecated
  /// Default Level to load.
  String DefaultLevel;
  /// Project Space
  String ProjectSpace;

  /// The path to the project folder that Contains this project.
  String ProjectFolder;
  /// Project Content Folder
  String ContentFolder;
  /// Editor specific content (screen shot, editor settings, etc...)
  String EditorContentFolder;
  /// Should a screenshot of the project be taken every time the project is
  /// saved.
  bool AutoTakeProjectScreenshot;

  /// Project File Path
  String ProjectFile;

  // Content Library for this Project.
  ContentLibrary* ProjectContentLibrary;
  ResourceLibrary* ProjectResourceLibrary;
  Array<ResourceLibrary*> SharedResourceLibraries;

private:
  Image mScreenshot;
  TimeType mLastLoadedTime;
  Guid mGuid;
};

class ContentLibraryReference : public SafeId32Object
{
public:
  RaverieDeclareType(ContentLibraryReference, TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream);
  void SetDefaults()
  {
  }

  String mContentLibraryName;
};

/// Component that enables a project to shared content libraries.
class SharedContent : public Component
{
public:
  RaverieDeclareType(SharedContent, TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;

  /// Shared Content Libraries
  Array<ContentLibraryReference> ExtraContentLibraries;
};

class ProjectDescription : public Component
{
public:
  RaverieDeclareType(ProjectDescription, TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;

  String GetTagsString(StringParam splitChar);

  /// Short description of project usually a short sentence.
  String ShortDescription;
  /// Long description of project.
  String LongDescription;
  /// Tags for this project.
  String Tags;

  HashSet<String> mProjectTags;
};

class DebugSettings : public Component
{
public:
  RaverieDeclareType(DebugSettings, TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Maximum number of debug objects allowed at any one time to prevent
  /// accidentally running out of memory.
  int GetMaxDebugObjects();
  void SetMaxDebugObjects(int maxDebugObjects);
  int mMaxDebugObjects;
};

class ExportSettings : public Component
{
public:
  RaverieDeclareType(ExportSettings, TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;

  // String mOutputDirectory;
  HashSet<String> mActiveTargets;
};

} // namespace Raverie
