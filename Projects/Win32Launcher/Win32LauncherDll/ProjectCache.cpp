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

DefineEvent(ScreenshotUpdated);

}//namespace Events

//-------------------------------------------------------------------LauncherProjectInfo
ZilchDefineType(LauncherProjectInfo, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
}

void LauncherProjectInfo::Serialize(Serializer& stream)
{
  SerializeNameDefault(mTags, String());
  mBuildId.Serialize(stream, false);

  // If we're loading and we have no platform then default to Win32 (legacy builds)
  if(stream.GetMode() == SerializerMode::Loading && mBuildId.mPlatform.Empty())
    mBuildId.mPlatform = "Win32";
}

void LauncherProjectInfo::AddTag(StringParam tag)
{
  if(mTags.Empty())
    mTags = tag;
  else
    mTags = BuildString(mTags, ",", tag);
}

void LauncherProjectInfo::GetSortedTagList(Array<String>& tagList)
{
  StringSplitRange range = mTags.Split(",");
  for(; !range.Empty(); range.PopFront())
    tagList.PushBack(range.Front());
  Sort(tagList.All());
}

BuildId LauncherProjectInfo::GetBuildId() const
{
  return mBuildId;
}

void LauncherProjectInfo::SetBuildId(const BuildId& buildId)
{
  mBuildId = buildId;
}

// Simple macros to wrap getting/setting a property from a component in a data tree.
// Just used to turn the type-name into a string to help renames work correctly.
#define GetChildComponentPropertyValueMacro(component, propertyName) \
  GetChildComponentPropertyValue(#component, "ProjectName")
#define SetChildComponentPropertyValueMacro(component, propertyName, newValue) \
  SetChildComponentPropertyValue(#component, propertyName, newValue)

//------------------------------------------------------------------------ CachedProject
CachedProject::CachedProject()
{
  mProjectCog = nullptr;
  mLauncherInfo = nullptr;
}

bool CachedProject::Load(StringParam projectFilePath)
{
  SetProjectPath(projectFilePath);

  mLoader.Close();
  Status status;
  mLoader.OpenFile(status, projectFilePath);

  // Load the project file and make sure everything succeeds
  mProjectCog = Z::gFactory->CreateFromStream(Z::gEngine->GetEngineSpace(), mLoader, 0, nullptr);
  if(mProjectCog == nullptr)
    return false;

  // Add a launcher info if it doesn't already exist. If it didn't exist then we have to copy
  // old values from other components to the launcher's component. If it did exist then keep all old values the same.
  mLauncherInfo = mProjectCog->has(LauncherProjectInfo);
  if(mLauncherInfo == nullptr)
  {
    mLauncherInfo = HasOrAdd<LauncherProjectInfo>(mProjectCog);

    // Load the build version number
    String buildNumberText = GetProjectPropertyValue("ProjectEngineRevision");
    // For legacy projects overwrite the current project's revision id
    // and platform and then re-set the build id
    BuildId buildId = mLauncherInfo->GetBuildId();
    ToValue(buildNumberText, buildId.mRevisionId);
    buildId.mPlatform = "Win32";
    mLauncherInfo->SetBuildId(buildId);

    // Load the project tags
    String descriptionTags = GetChildComponentPropertyValueMacro(ProjectDescription, "Tags");
    ParseTags(mLauncherInfo, descriptionTags);

    AddOrReplaceDataNodeComponent(mLauncherInfo);
  }

  // Try to update the cached texture (may invoke a threaded load)
  UpdateTexture();

  return true;
}

void CachedProject::UpdateTexture()
{
  String editorContentFolder = FilePath::Combine(GetProjectFolder(), "EditorContent");
  String screenShotFilePath = FilePath::Combine(editorContentFolder, "ProjectScreenshot.png");

  Texture* currentTexture = mScreenshotTexture;
  // If we already had a texture and it is not newer then don't re-load it
  if(currentTexture != nullptr && FileExists(screenShotFilePath))
  {
    TimeType modifiedTime = GetFileModifiedTime(screenShotFilePath);
    if(modifiedTime == mLastLoadTime)
      return;

    mLastLoadTime = GetFileModifiedTime(screenShotFilePath);
  }  

  // Create a background task to load the screenshot from disk to an Image
  LoadImageFromDiskTaskJob* job = new LoadImageFromDiskTaskJob(screenShotFilePath);
  BackgroundTask* task = Z::gBackgroundTasks->Execute(job, "Update texture");
  ConnectThisTo(task, Events::BackgroundTaskCompleted, OnImageLoaded);
}

void CachedProject::OnImageLoaded(BackgroundTaskEvent* e)
{
  LoadImageFromDiskTaskJob* job = (LoadImageFromDiskTaskJob*)e->mTask->GetFinishedJob();

  // Load the image into a texture (must be done on the main thread)
  Image& image = job->mImage;
  mScreenshotTexture = Texture::CreateRuntime();
  mScreenshotTexture->SetMipMapping(TextureMipMapping::GpuGenerated);
  mScreenshotTexture->SetFiltering(TextureFiltering::Trilinear);
  mScreenshotTexture->Upload(image);
  image.Deallocate();

  // Tell whoever cares that we have a new preview texture
  Event toSend;
  DispatchEvent(Events::ScreenshotUpdated, &toSend);
}

BuildId CachedProject::GetBuildId() const
{
  return mLauncherInfo->GetBuildId();
}

void CachedProject::SetBuildId(const BuildId& buildId)
{
  mLauncherInfo->SetBuildId(buildId);
}

String CachedProject::GetDisplayString(bool showPlatform) const
{
  return GetBuildId().ToDisplayString(showPlatform);
}

String CachedProject::GetDebugIdString()
{
  return GetBuildId().ToIdString();
}

String CachedProject::GetProjectPath()
{
  return mProjectPath;
}

void CachedProject::SetProjectPath(StringParam projectFilePath)
{
  mProjectPath = projectFilePath;
  mProjectFolder = FilePath::GetDirectoryPath(projectFilePath);
}

String CachedProject::GetProjectFolder()
{
  return mProjectFolder;
}

String CachedProject::GetProjectName()
{
  return GetProjectPropertyValue("ProjectName");
}

void CachedProject::RenameAndMoveProject(StringParam newProjectName)
{
  // Rename the zeroproj file in the old location (avoid calling delete where possible)
  String renamedZeroProjPath = FilePath::CombineWithExtension(mProjectFolder, newProjectName, ".zeroproj");
  MoveFile(renamedZeroProjPath, mProjectPath);
  
  String parentDirectory = FilePath::GetDirectoryPath(mProjectFolder);
  String newFolder = FilePath::Combine(parentDirectory, newProjectName);
  // Move the project's folder
  MoveFolderContents(newFolder, mProjectFolder);
  // Move folder doesn't delete empty directories, so delete the old path
  DeleteDirectory(mProjectFolder);

  // Update the project path and the project name in the zeroproj
  SetProjectPropertyValue("ProjectName", newProjectName);

  // Update the path of the zero proj file
  mProjectPath = FilePath::CombineWithExtension(newFolder, newProjectName, ".zeroproj");

  // Save out the new zeroproj file that should have up-to-date properties
  Save(false);
}

void CachedProject::GetTags(HashSet<String>& tags)
{
  Array<String> sortedTags;
  mLauncherInfo->GetSortedTagList(sortedTags);

  for(size_t i = 0; i < sortedTags.Size(); ++i)
    tags.Insert(sortedTags[i]);
}

String CachedProject::GetTagsDisplayString(StringParam separator)
{
  Array<String> sortedTags;
  mLauncherInfo->GetSortedTagList(sortedTags);
  
  return String::JoinRange(separator, sortedTags.All());
}

void CachedProject::Save(bool overwriteRevisionNumber)
{
  if(overwriteRevisionNumber)
    SetBuildId(GetBuildId());

  // Overwrite the launcher's info in the data tree
  AddOrReplaceDataNodeComponent(mLauncherInfo);

  // At some point we should update the project description information...
  //ProjectDescription* projDescription = mProjectCog->has(ProjectDescription);
  //if(projDescription != nullptr)
  //  AddOrReplaceDataNodeComponent(projDescription);

  // Create the saver
  Status status;
  TextSaver saver;
  saver.Open(status, mProjectPath.c_str(), (DataVersion::Enum)mLoader.mLoadedFileVersion);

  // Save out the entire tree
  mLoader.Reset();
  mLoader.GetCurrent()->SaveToStream(saver);
  saver.Close();
}

void CachedProject::ParseTags(LauncherProjectInfo* launcherInfo, StringParam tags)
{
  // Tags are comma separated
  StringSplitRange tagRange = tags.Split(",");
  for(; !tagRange.Empty(); tagRange.PopFront())
  {
    // In some versions of the engine, tags were just a comma separated list.
    // In later versions the tags were split with ':' to give the tag a category 
    // for Project or User tags. We only parse project tags and throw away user tags now
    // (user tags were used to filter things to only ProjectFun or similar things).
    StringRange tag = tagRange.Front();

    // Skip empty tags
    if(tag.Empty())
      continue;

    StringRange foundRange = tag.FindFirstOf(":");
    if(foundRange.Empty())
    {
      launcherInfo->AddTag(tag);
      continue;
    }

    StringRange tagName = tag.SubString(tag.Begin(), foundRange.Begin());
    StringRange tagType = tag.SubString(foundRange.Begin() + 1, tag.End());
    if(tagType == "Project")
      launcherInfo->AddTag(tagName);
  }
}

DataNode* CachedProject::GetComponentPropertyNode(StringParam componentType, StringParam propertyName)
{
  // Reset to tree otherwise the root won't be correct
  mLoader.Reset();
  DataNode* root = mLoader.GetCurrent();

  // Find the component node
  bool dummy;
  DataNode* componentNode = root->FindChildWithTypeName(componentType, String(), dummy);
  if(componentNode == nullptr)
    return nullptr;

  // Find the property's node
  DataNode* propertyNode = componentNode->FindChildWithName(propertyName);
  if(propertyNode == nullptr || propertyNode->mNodeType != DataNodeType::Value)
    return nullptr;

  return propertyNode;
}

String CachedProject::GetChildComponentPropertyValue(StringParam componentType, StringParam propertyName)
{
  DataNode* valueNode = GetComponentPropertyNode(componentType, propertyName);
  if(valueNode != nullptr)
    return valueNode->mTextValue;
  return String();
}

void CachedProject::SetChildComponentPropertyValue(StringParam componentType, StringParam propertyName, StringParam value)
{
  DataNode* valueNode = GetComponentPropertyNode(componentType, propertyName);
  if(valueNode != nullptr)
    valueNode->mTextValue = value;
}

void CachedProject::AddOrReplaceDataNodeComponent(Component* component)
{
  // In order to update a component's node in the tree, we have to remove the old node if it
  // exists and then re-add the new data as a child node. To do this requires serializing
  // the component and then creating a data set from it.

  mLoader.Reset();
  DataNode* root = mLoader.GetCurrent();

  // Remove an old node by the same name if it exists
  bool foundDuplicate;
  String componentName = component->ZilchGetDerivedType()->Name;
  DataNode* child = root->FindChildWithTypeName(componentName, String(), foundDuplicate);
  if(child != nullptr)
    child->Destroy();

  Status status;
  // Save the component to a stream
  ObjectSaver saver;
  saver.OpenBuffer((DataVersion::Enum)mLoader.mLoadedFileVersion);
  saver.SaveDefinition(component);

  // Extract the text as a data block and then read that block into a data node
  DataBlock dataBlock = saver.ExtractAsDataBlock();
  uint fileVersion;
  DataNode* set = ReadDataSet(status, String((char*)dataBlock.Data, dataBlock.Size),
                              String(), &mLoader, &fileVersion);
  // Add this node into the tree
  set->AttachTo(root);
}

String CachedProject::GetProjectPropertyValue(StringParam propertyName)
{
  DataNode* valueNode = GetComponentPropertyNode("ProjectSettings", propertyName);
  if(valueNode != nullptr)
    return valueNode->mTextValue;
  valueNode = GetComponentPropertyNode("Project", propertyName);
  if(valueNode != nullptr)
    return valueNode->mTextValue;
  return String();
}

void CachedProject::SetProjectPropertyValue(StringParam propertyName, StringParam value)
{
  DataNode* valueNode = GetComponentPropertyNode("ProjectSettings", propertyName);
  if(valueNode != nullptr)
  {
    valueNode->mTextValue = value;
    return;
  }
  valueNode = GetComponentPropertyNode("Project", propertyName);
  if(valueNode != nullptr)
  {
    valueNode->mTextValue = value;
    return;
  }
}

//------------------------------------------------------------------------ ProjectCache
ProjectCache::ProjectCache(Cog* configCog)
{
  mConfigCog = configCog;
  mRecentProjects = mConfigCog->has(RecentProjects);
}

ProjectCache::~ProjectCache()
{
  CachedProjectMap::valuerange range = mProjectMap.Values();
  for(; !range.Empty(); range.PopFront())
  {
    CachedProject* cachedProject = range.Front();
    delete cachedProject;
  }
}

CachedProject* ProjectCache::LoadProjectFile(StringParam projectFilePath)
{
  // If the project already exists then just return it
  CachedProject* cachedProject = mProjectMap.FindValue(projectFilePath, nullptr);
  if(cachedProject != nullptr)
    return cachedProject;

  // Otherwise create the project. If we fail to load then delete the project we just created.
  cachedProject = new CachedProject();
  if(!cachedProject->Load(projectFilePath))
  {
    delete cachedProject;
    return nullptr;
  }
  
  // Set our map of zeroproj files paths to project
  mProjectMap[projectFilePath] = cachedProject;

  return cachedProject;
}

void ProjectCache::ReloadProjectFile(CachedProject* cachedProject, bool preserveVersionId)
{
  BuildId buildId = cachedProject->GetBuildId();

  String projectFilePath = cachedProject->GetProjectPath();
  cachedProject->Load(projectFilePath);

  if(preserveVersionId)
    cachedProject->SetBuildId(buildId);
}

CachedProject* ProjectCache::CreateProjectFromTemplate(StringParam projectName, StringParam projectDir, StringParam templatePath, const BuildId& buildId,
  const HashSet<String>& projectTags)
{
  // Extract the zip to the project's install location
  Archive archive(ArchiveMode::Decompressing);
  archive.ReadZipFile(ArchiveReadFlags::All, templatePath);
  archive.ExportToDirectory(ArchiveExportMode::Overwrite, projectDir);

  // Get the old zero proj file (by the template name)
  String oldProjectName = FilePath::GetFileNameWithoutExtension(templatePath);
  String oldProjectFilePath = FindZeroProj(projectDir, oldProjectName);
  // Build the new project's full path
  String newProjectFilePath = FilePath::CombineWithExtension(projectDir, projectName, ".zeroproj");
  
  // Rename the project to the new project's name
  if(FileExists(oldProjectFilePath))
    MoveFile(newProjectFilePath, oldProjectFilePath);

  // Create the cached project from this template zero proj
  CachedProject* cachedProject = new CachedProject();
  bool loadedSuccessfully = cachedProject->Load(newProjectFilePath);
  // If we failed to load a valid project file (typically due to some invalid naming) then notify the user and exit
  if(!loadedSuccessfully)
  {
    DoNotifyError("Failed to load project", String::Format("The project file at %s is invalid.", newProjectFilePath.c_str()));
    delete cachedProject;
    return nullptr;
  }
  cachedProject->SetBuildId(buildId);
  cachedProject->SetProjectPropertyValue("ProjectName", projectName);
  mProjectMap[newProjectFilePath] = cachedProject;

  // Add the tag info to the project (for legacy tags)
  ProjectDescription* projectInfo = HasOrAdd<ProjectDescription>(cachedProject->mProjectCog);
  projectInfo->mProjectTags = projectTags;
  // Add the tag info to the launcher's project data
  LauncherProjectInfo* launcherInfo = HasOrAdd<LauncherProjectInfo>(cachedProject->mProjectCog);
  AutoDeclare(tagRange, projectTags.All());
  for(; !tagRange.Empty(); tagRange.PopFront())
    launcherInfo->AddTag(tagRange.Front());

  // Then save the project under its new name
  cachedProject->Save(false);
  
  return cachedProject;
}

void ProjectCache::UpdateAllTextures()
{
  CachedProjectMap::valuerange range = mProjectMap.Values();
  for(; !range.Empty(); range.PopFront())
  {
    CachedProject* cachedProject = range.Front();
    cachedProject->UpdateTexture();
  }
}

String ProjectCache::FindZeroProj(StringParam searchpath, StringParam projectName)
{
  // Get the old zero proj file (by the template name)
  String expectedProjectFilePath = FilePath::CombineWithExtension(searchpath, projectName, ".zeroproj");

  // If this file exists then we can just return its path
  if(FileExists(expectedProjectFilePath))
    return expectedProjectFilePath;

  // Otherwise, search the path for any zero proj file and return the first we find
  FileRange fileRange(searchpath);
  for(; !fileRange.Empty(); fileRange.PopFront())
  {
    String fileName = fileRange.Front();
    String ext = FilePath::GetExtension(fileName);
    if(ext.ToLower() == "zeroproj")
    {
      String foundProjectName = fileName;
      expectedProjectFilePath = FilePath::Combine(searchpath, fileName);
      break;
    }
  }

  return expectedProjectFilePath;
}

}//namespace Zero
