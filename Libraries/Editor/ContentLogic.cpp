// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

void LoadContentConfig()
{
  InitializeContentSystem();

  ContentSystem* contentSystem = Z::gContentSystem;

  Cog* configCog = Z::gEngine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);

  Array<String>& librarySearchPaths = contentSystem->LibrarySearchPaths;
  ContentConfig* contentConfig = configCog->has(ContentConfig);
  String appCacheDirectory = GetUserLocalDirectory();

  String sourceDirectory = mainConfig->SourceDirectory;
  ErrorIf(sourceDirectory.Empty(), "Expected a source directory");

  if (contentConfig)
    librarySearchPaths.InsertAt(0, contentConfig->LibraryDirectories.All());

  librarySearchPaths.PushBack(FilePath::Combine(sourceDirectory, "Resources"));

  CreateDirectoryAndParents(FilePath::Combine(appCacheDirectory, GetApplicationName()));

  contentSystem->ToolPath = FilePath::Combine(sourceDirectory, "Tools");

  contentSystem->mHistoryEnabled = contentConfig->HistoryEnabled;

  // To avoid conflicts of assets of different versions(especially when the
  // version selector goes live) set the content folder to a unique directory
  // based upon the version number
  String revisionChangesetName = BuildString("ZeroVersion", GetRevisionNumberString(), "-", GetChangeSetString());
  contentSystem->ContentOutputPath = FilePath::Combine(appCacheDirectory, "ZeroContent", revisionChangesetName);

  // If we don't already have the content output directory, then see if we have
  // local prebuilt content that can be copied into the output directory (this
  // is faster than building the content ourselves, if it exists).
  if (!DirectoryExists(contentSystem->ContentOutputPath))
  {
    String prebuiltContent = FilePath::Combine(sourceDirectory, "PrebuiltZeroContent");
    if (DirectoryExists(prebuiltContent))
    {
      ZPrint("Copying prebuilt content from '%s' to '%s'\n",
             prebuiltContent.c_str(),
             contentSystem->ContentOutputPath.c_str());
      CopyFolderContents(contentSystem->ContentOutputPath, prebuiltContent);
    }
  }

  contentSystem->SystemVerbosity = contentConfig->ContentVerbosity;
}

bool LoadContentLibrary(StringParam name, bool isCore)
{
  ContentLibrary* library = Z::gContentSystem->Libraries.FindValue(name, nullptr);
  if (library)
  {
    if (isCore)
      library->SetReadOnly(true);

    Status status;
    ResourcePackage package;
    Z::gContentSystem->BuildLibrary(status, library, package, false);

    if (status)
    {
      if (isCore)
      {
        forRange (ResourceEntry& entry, package.Resources.All())
        {
          if (entry.mLibrarySource)
          {
            if (ContentEditorOptions* options = entry.mLibrarySource->has(ContentEditorOptions))
              entry.mLibrarySource->ShowInEditor = options->mShowInEditor;
            else
              entry.mLibrarySource->ShowInEditor = false;
          }
        }
      }

      Status status;
      Z::gResources->LoadPackage(status, &package);
      if (!status)
        DoNotifyError("Failed to load resource package.", status.Message);

      return (bool)status;
    }
    else
    {
      return false;
    }

    return true;
  }
  else
  {
    ZPrint("Failed to find core content library %s.\n", name.c_str());
    return false;
  }
}

ZilchDefineType(EditorPackageLoader, builder, type)
{
}

EditorPackageLoader::EditorPackageLoader()
{
  ConnectThisTo(Z::gContentSystem, Events::PackageBuilt, OnPackagedBuilt);
}

void EditorPackageLoader::OnPackagedBuilt(ContentSystemEvent* event)
{
  if (Z::gEditor)
  {
    LoadPackage(Z::gEditor, Z::gEditor->mProject, event->mLibrary, event->mPackage);
  }
}

bool EditorPackageLoader::LoadPackage(Editor* editor,
                                      Cog* projectCog,
                                      ContentLibrary* library,
                                      ResourcePackage* package)
{
  ProjectSettings* project = projectCog->has(ProjectSettings);

  ResourceSystem* resourceSystem = Z::gResources;

  if (project->ProjectContentLibrary == library)
  {
    // Load all packages
    forRange (ResourcePackage* dependentPackage, PackagesToLoad.All())
    {
      Status status;
      ResourceLibrary* library = resourceSystem->LoadPackage(status, dependentPackage);
      if (!status)
        DoNotifyError("Failed to load resource package.", status.Message);

      project->SharedResourceLibraries.PushBack(library);
      delete dependentPackage;
    }
    PackagesToLoad.Clear();

    // Set the content library so Loading may try to create new content for
    // fixing old content elements.
    editor->mProjectLibrary = library;

    Status status;
    project->ProjectResourceLibrary = resourceSystem->LoadPackage(status, package);
    if (!status)
      DoNotifyError("Failed to load resource package.", status.Message);

    //?
    DoEditorSideImporting(package, nullptr);
    //delete package;

    Z::gEditor->SetExploded(false, true);
    Z::gEditor->ProjectLoaded();
    return true;
  }
  else
  {
    PackagesToLoad.PushBack(package);
  }

  return false;
}

template <typename ManagerType>
void ShowBuiltInResource(StringParam name)
{
  Resource* resource = ManagerType::Find(name);
  if (resource && resource->mContentItem)
    resource->mContentItem->ShowInEditor = true;
}

bool LoadCoreContent(Array<String>& coreLibs)
{
  Z::gContentSystem->DefaultBuildStream = new TextStreamDebugPrint();
  Z::gContentSystem->EnumerateLibraries();

  ZPrint("Loading Editor Content...\n");
  EditorPackageLoader* loader = EditorPackageLoader::GetInstance();

  TimerBlock timer("Loading Content");

  String docDirectory = GetUserDocumentsDirectory();

  bool coreContent = LoadContentLibrary("FragmentCore", true);
  coreContent = coreContent && LoadContentLibrary("Loading", true);

  forRange (String libraryName, coreLibs.All())
  {
    coreContent = coreContent && LoadContentLibrary(libraryName, true);
  }

  if (!coreContent)
  {
    FatalEngineError("Failed to load core content library for editor. Resources"
                     " need to be in the working directory.");
    return false;
  }

  // Show all default resources
  forRange (ResourceManager* manager, Z::gResources->Managers.Values())
  {
    if (manager->mCanCreateNew)
      ErrorIf(manager->mExtension.Empty(), "Must set an extension on %s", manager->GetResourceType()->Name.c_str());

    Resource* resource = manager->GetResource(manager->DefaultResourceName, ResourceNotFound::ReturnNull);
    if (resource && resource->mContentItem)
    {
      resource->mContentItem->ShowInEditor = true;

      // Moved default font to the Loading library for progress display
      ErrorIf(resource->mContentItem->mLibrary->Name != "ZeroCore" &&
                  resource->mContentItem->mLibrary->Name != "Loading",
              "Only resources that are in core can be defaults");
    }
    else
    {
      ErrorIf(!manager->mNoFallbackNeeded,
              "Failed to find default resource for resource type %s",
              manager->mResourceTypeName.c_str());
    }
  }

  return true;
}

} // namespace Zero
