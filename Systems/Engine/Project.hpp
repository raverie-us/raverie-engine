///////////////////////////////////////////////////////////////////////////////
///
/// \file Project.hpp
/// Declaration of the Project component class.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//----------------------------------------------------------------------- Events
namespace Events
{
  DeclareEvent(ProjectLoaded);
  DeclareEvent(NoProjectLoaded);
}

//---------------------------------------------------------------------- Project
/// Project component store primary data for a project. Projects are separate
/// games with their own content and settings. 
class ProjectSettings : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

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

  /// Returns the path to the content folder containing editor specific content (screen shot, editor settings, etc...).
  String GetEditorContentFolder();

  void Save(bool overwriteRevisionNumber = true);

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
  /// Project Engine Version. The revision of the engine that saved the project.
  uint ProjectEngineRevision;

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
  bool AutoTakeProjectScreenshot;

  /// Project File Path
  String ProjectFile;

  //Content Library for this Project.
  ContentLibrary* ProjectContentLibrary;
  ResourceLibrary* ProjectResourceLibrary;
  Array<ResourceLibrary*> SharedResourceLibraries;

private:
  Image mScreenshot;
  TimeType mLastLoadedTime;
  Guid mGuid;
};

//---------------------------------------------------- Content Library Reference
class ContentLibraryReference : public Object
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream);
  void SetDefaults() {}

  String mContentLibraryName;
};

//--------------------------------------------------------------- Shared Content
/// Component that enables a project to shared content libraries.
class SharedContent : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;

  /// Shared Content Libraries
  Array<ContentLibraryReference> ExtraContentLibraries;
};

//---------------------------------------------------------- Project Description
class ProjectDescription : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

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

/// Settings for how the application window should be sized when launched.
class WindowLaunchSettings : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;

  /// If a pre-launch popup should be used to determine how application window is sized.
  //bool mUseLaunchOptionsPopup;
  /// If application launches in fullscreen mode.
  bool mLaunchFullscreen;
  /// Resolution of application when launched in windowed mode.
  IntVec2 mWindowedResolution;
};

/// Settings for how the frame rate of the engine should be controlled.
class FrameRateSettings : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// If the frame rate should sync with the monitor's refresh rate, superseded by LimitFrameRate.
  bool mVerticalSync;
  /// If the engine should limit the frame rate.
  bool mLimitFrameRate;
  /// How many frames per second the engine should be limited at.
  int GetFrameRate();
  void SetFrameRate(int frameRate);
  int mFrameRate;
};

}//namespace Zero
