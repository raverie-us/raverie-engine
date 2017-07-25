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

DeclareEvent(ScreenshotUpdated);

}//namespace Events

//-------------------------------------------------------------------LauncherProjectInfo
class LauncherProjectInfo : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;

  void AddTag(StringParam tag);
  void GetSortedTagList(Array<String>& tagList);

  BuildId GetBuildId() const;
  void SetBuildId(const BuildId& buildId);

  String mTags;
  BuildId mBuildId;
};

//------------------------------------------------------------------------ CachedProject
/// A cached project with it's latest screenshot texture.
class CachedProject : public EventObject
{
public:
  typedef CachedProject ZilchSelf;

  CachedProject();
  bool Load(StringParam projectFilePath);

  void UpdateTexture();
  void OnImageLoaded(BackgroundTaskEvent* e);

  BuildId GetBuildId() const;
  void SetBuildId(const BuildId& buildId);

  /// A display string for this build associated with this project.
  /// Note: this should only ever be used as a display string, not a unique id.
  String GetDisplayString(bool showPlatform = false) const;
  /// A string for debug printing the build id
  String GetDebugIdString();

  // The location of the project ".zeroproj" file
  String GetProjectPath();
  void SetProjectPath(StringParam projectFilePath);

  // The directory of the project's ".zeroproj"
  String GetProjectFolder();
  // The name of the project inside the ".zeroproj"
  String GetProjectName();
  /// Rename this project to the given name. Also renames the folder containing the
  /// zeroproj to the new name (assumes the user has checked for existing folders/files of the new name).
  void RenameAndMoveProject(StringParam newProjectName);

  void GetTags(HashSet<String>& tags);
  String GetTagsDisplayString(StringParam separator = " / ");

  void Save(bool overwriteRevisionNumber);

  //-------------------------------------------------------------------Internal
  void ParseTags(LauncherProjectInfo* launcherInfo, StringParam tags);
  DataNode* GetComponentPropertyNode(StringParam componentType, StringParam propertyName);
  String GetChildComponentPropertyValue(StringParam componentType, StringParam propertyName);
  void SetChildComponentPropertyValue(StringParam componentType, StringParam propertyName, StringParam value);
  void AddOrReplaceDataNodeComponent(Component* component);

  // Helper to get a value from the "Project" component. This is special as the Project component has been renamed.
  String GetProjectPropertyValue(StringParam propertyName);
  // Helper to set a value from the "Project" component. See GetProjectPropertyValue for why this is special.
  void SetProjectPropertyValue(StringParam propertyName, StringParam value);

  String mProjectPath;
  String mProjectFolder;

  Cog* mProjectCog;
  LauncherProjectInfo* mLauncherInfo;
  ObjectLoader mLoader;

  HandleOf<Texture> mScreenshotTexture;
  TimeType mLastLoadTime;
};

//------------------------------------------------------------------------ ProjectCache
/// Caches all projects and their latest screenshot textures.
class ProjectCache
{
public:
  ProjectCache(Cog* configCog);
  ~ProjectCache();

  /// Load the project and create a cache entry for it.
  CachedProject* LoadProjectFile(StringParam projectFilePath);
  /// Reload the Project on a cached project (get any new property changes from a running zero)
  void ReloadProjectFile(CachedProject* cachedProject, bool preserveVersionId);
  /// Create a project from a template file zip
  CachedProject* CreateProjectFromTemplate(StringParam projectName, StringParam projectDir, StringParam templatePath, const BuildId& buildId,
    const HashSet<String>& projectTags);

  /// Update all of the cached project's textures.
  void UpdateAllTextures();

private:
  /// Given a path and an expected project name, find the zero project (not currently recursive)
  String FindZeroProj(StringParam searchpath, StringParam projectName);

public:

  // Cached projects and their screenshot textures
  typedef HashMap<String, CachedProject*> CachedProjectMap;
  CachedProjectMap mProjectMap;

  Cog* mConfigCog;
  RecentProjects* mRecentProjects;
};

}//namespace Zero
