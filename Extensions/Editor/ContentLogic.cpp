///////////////////////////////////////////////////////////////////////////////
///
/// \file ContentLogic.cpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ExtraLibrarySearchPathCallback mExtraLibrarySearchPaths = nullptr;

void LoadContentConfig(Cog* configCog)
{
  InitializeContentSystem();

  ContentSystem* contentSystem = Z::gContentSystem;

  MainConfig* mainConfig = configCog->has(MainConfig);

  Array<String>& librarySearchPaths = contentSystem->LibrarySearchPaths;
  ContentConfig* contentConfig = configCog->has(ContentConfig);
  String appCacheDirectory = GetUserLocalDirectory();
  String applicationDirectory = mainConfig->ApplicationDirectory;
  String documentDirectory = GetUserDocumentsDirectory();
  String workingDirectory = GetWorkingDirectory();

  
  String applicationName = mainConfig->ApplicationName;

  String sourceDirectory = mainConfig->SourceDirectory;

  if(contentConfig)
    librarySearchPaths.InsertAt(0, contentConfig->LibraryDirectories.All());

  CreateDirectory(FilePath::Combine(appCacheDirectory, applicationName));

  //Add application directory resources if available
  if(!applicationDirectory.Empty())
  {
    librarySearchPaths.PushBack(FilePath::Combine(applicationDirectory, "Resources"));
  }

  //Add the source path for resources
  if(!sourceDirectory.Empty())
    librarySearchPaths.PushBack(FilePath::Combine(sourceDirectory, "Resources"));

  //Add working directory resources
  librarySearchPaths.PushBack(FilePath::Combine(workingDirectory, "Resources"));

  // Hack!
  if(mExtraLibrarySearchPaths != nullptr)
    mExtraLibrarySearchPaths(configCog, librarySearchPaths);

  //First try to use the tools directory specified in the user config
  if(contentConfig && !contentConfig->ToolsDirectory.Empty() && FileExists(contentConfig->ToolsDirectory))
    contentSystem->ToolPath = FilePath::Normalize(contentConfig->ToolsDirectory);
  else if(!sourceDirectory.Empty())
    //Use the build info version if it is not empty
    contentSystem->ToolPath = FilePath::Combine(sourceDirectory, "Tools");
  else if(!applicationDirectory.Empty())
    contentSystem->ToolPath = FilePath::Combine(applicationDirectory, "Tools");
  else
    //Just use working directory
    contentSystem->ToolPath = FilePath::Combine(workingDirectory, "Tools");

  contentSystem->mHistoryEnabled = contentConfig->HistoryEnabled;

  //To avoid conflicts of assets of different versions(especially when the version selector goes live)
  //set the content folder to a unique directory based upon the version number
  String revisionChangesetName = BuildString("ZeroVersion", GetRevisionNumberString(), "-", GetChangeSetString());
  contentSystem->ContentOutputPath = FilePath::Combine(appCacheDirectory, "ZeroContent", revisionChangesetName);

  contentSystem->SystemVerbosity = contentConfig->ContentVerbosity;
}

bool LoadContentLibrary(StringParam name, bool isCore)
{
  ContentLibrary* library = Z::gContentSystem->Libraries.FindValue(name, nullptr);
  if(library)
  {
    if(isCore)
      library->SetReadOnly(true);
    
    Status status;
    ResourcePackage package;
    Z::gContentSystem->BuildLibrary(status, library, package);

    if(status)
    {
      if(isCore)
      {
        forRange(ResourceEntry& entry, package.Resources.All())
        {
          if(entry.mLibrarySource)
            entry.mLibrarySource->ShowInEditor = false;
        }
      }

      Status status;
      Z::gResources->LoadPackage(status, &package);
      if(!status)
        DoNotifyError("Failed to load resource package.",status.Message);

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

//-------------------------------------------------------- Editor Package Loader
ZilchDefineType(EditorPackageLoader, builder, type)
{
}

EditorPackageLoader::EditorPackageLoader()
{
  ConnectThisTo(Z::gContentSystem, Events::PackageBuilt, OnPackagedBuilt);
}

void EditorPackageLoader::OnPackagedBuilt(ContentSystemEvent* event)
{
  LoadPackage(Z::gEditor, Z::gEditor->mProject, event->mLibrary, event->mPackage);
}

bool EditorPackageLoader::LoadPackage(Editor* editor, Cog* projectCog, ContentLibrary* library,
                  ResourcePackage* package)
{
  ProjectSettings* project = projectCog->has(ProjectSettings);

  ResourceSystem* resourceSystem = Z::gResources;

  if(project->ProjectContentLibrary == library)
  {
    //Load all packages
    forRange(ResourcePackage* dependentPackage, PackagesToLoad.All())
    {
      Status status;
      ResourceLibrary* library = resourceSystem->LoadPackage(status, dependentPackage);
      if(!status)
        DoNotifyError("Failed to load resource package.",status.Message);

      project->SharedResourceLibraries.PushBack(library);
      delete dependentPackage;
    }
    PackagesToLoad.Clear();

    //Set the content library so Loading may try to create new content for
    //fixing old content elements.
    editor->mProjectLibrary = library;

    Status status;
    project->ProjectResourceLibrary = resourceSystem->LoadPackage(status, package);
    if(!status)
      DoNotifyError("Failed to load resource package.",status.Message);

    //?
    DoEditorSideImporting(package, nullptr);
    delete package;

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

template<typename ManagerType>
void ShowBuiltInResource(StringParam name)
{
  Resource* resource = ManagerType::Find(name);
  if(resource && resource->mContentItem)
    resource->mContentItem->ShowInEditor = true;
}

bool LoadEditorContent(Cog* configCog)
{
  Z::gContentSystem->DefaultBuildStream = new TextStreamDebugPrint();
  Z::gContentSystem->EnumerateLibraries();

  ZPrint("Loading Editor Content...\n");
  EditorPackageLoader* loader = EditorPackageLoader::GetInstance();

  Timer timer;
  timer.Update();

  String docDirectory = GetUserDocumentsDirectory();
  
  LoadContentLibrary("FragmentCore", true);
  bool coreContent = LoadContentLibrary("Loading", true);

  Array<String> coreLibs;
  coreLibs.PushBack("ZeroCore");
  coreLibs.PushBack("Editor");
  coreLibs.PushBack("EditorUi");
  coreLibs.PushBack("EditorScripts");

  forRange(String libraryName, coreLibs.All())
  {
    coreContent = coreContent && LoadContentLibrary(libraryName, true);
  }

  // Hack!
  if(mCustomLibraryLoader != nullptr)
    mCustomLibraryLoader(configCog);

  if(!coreContent)
  {
    FatalEngineError("Failed to load core content library for editor. Resources"
                     " need to be in the working directory.");
    return false;
  }

  //Show all default resources
  forRange(ResourceManager* manager, Z::gResources->Managers.Values())
  {
    if(manager->mCanCreateNew)
      ErrorIf(manager->mExtension.Empty(), "Must set an extension on %s", manager->GetResourceType()->Name.c_str());

    Resource* resource = manager->GetResource(manager->DefaultResourceName, ResourceNotFound::ReturnNull);
    if(resource && resource->mContentItem)
    {
      resource->mContentItem->ShowInEditor = true;

      // Moved default font to the Loading library for progress display
      ErrorIf(resource->mContentItem->mLibrary->Name != "ZeroCore" && resource->mContentItem->mLibrary->Name != "Loading", 
        "Only resources that are in core can be defaults");
    }
    else
    {
      ErrorIf(!manager->mNoFallbackNeeded, "Failed to find default resource for resource type %s", 
              manager->mResourceTypeName.c_str());
    }
  }

  // The UVS on these need to be verified (currently they are incorrect)
  //MeshManager::Find("Cube")->PrimitiveShape = MeshPrimitiveShape::Box;
  //MeshManager::Find("Sphere")->PrimitiveShape = MeshPrimitiveShape::Sphere;
  //MeshManager::Find("Cylinder")->PrimitiveShape = MeshPrimitiveShape::Cylinder;
  
  //To move to data files
  ShowBuiltInResource<ArchetypeManager>("Sprite");
  ShowBuiltInResource<ArchetypeManager>("Cube");
  ShowBuiltInResource<ArchetypeManager>("Sphere");
  ShowBuiltInResource<ArchetypeManager>("Camera");
  ShowBuiltInResource<ArchetypeManager>("Cylinder");
  ShowBuiltInResource<ArchetypeManager>("DefaultTile");

  ShowBuiltInResource<MaterialManager>("AdditiveSprite");
  ShowBuiltInResource<MaterialManager>("AlphaCut");
  ShowBuiltInResource<MaterialManager>("AlphaSprite");
  ShowBuiltInResource<MaterialManager>("DebugDraw");
  ShowBuiltInResource<MaterialManager>("DebugDrawOnTop");
  ShowBuiltInResource<MaterialManager>("DefaultHeightMapMaterial");
  ShowBuiltInResource<MaterialManager>("DirectionalLight");
  ShowBuiltInResource<MaterialManager>("DirectionalLightShadows");
  ShowBuiltInResource<MaterialManager>("EmptyMaterial");
  ShowBuiltInResource<MaterialManager>("OpaqueFlat");
  ShowBuiltInResource<MaterialManager>("ZeroMaterial");

  ShowBuiltInResource<RenderGroupManager>("AdditiveBlend");
  ShowBuiltInResource<RenderGroupManager>("AlphaBlend");
  ShowBuiltInResource<RenderGroupManager>("AlphaCut");
  ShowBuiltInResource<RenderGroupManager>("DebugDraw");
  ShowBuiltInResource<RenderGroupManager>("DebugDrawOnTop");
  ShowBuiltInResource<RenderGroupManager>("Lights");
  ShowBuiltInResource<RenderGroupManager>("Opaque");
  ShowBuiltInResource<RenderGroupManager>("ShadowCasters");
  ShowBuiltInResource<RenderGroupManager>("ZSort");

  ShowBuiltInResource<MeshManager>("Cube");
  ShowBuiltInResource<MeshManager>("Cylinder");
  ShowBuiltInResource<MeshManager>("Quad");
  ShowBuiltInResource<MeshManager>("Sphere");
  ShowBuiltInResource<MeshManager>("Triangle");
  ShowBuiltInResource<MeshManager>("Wedge");

  ShowBuiltInResource<TextureManager>("Black");
  ShowBuiltInResource<TextureManager>("BlueNoise");
  ShowBuiltInResource<TextureManager>("EnvironmentBrdfLut");
  ShowBuiltInResource<TextureManager>("FlatNormal");
  ShowBuiltInResource<TextureManager>("Grey");
  ShowBuiltInResource<TextureManager>("SimpleSkybox");
  ShowBuiltInResource<TextureManager>("SsaoRandom4x4");
  ShowBuiltInResource<TextureManager>("White");
  ShowBuiltInResource<TextureManager>("ZeroAlbedo");
  ShowBuiltInResource<TextureManager>("ZeroMetallic");
  ShowBuiltInResource<TextureManager>("ZeroNormal");
  ShowBuiltInResource<TextureManager>("ZeroRoughness");

  ShowBuiltInResource<FontManager>("NotoSans-Bold");

  ShowBuiltInResource<ColorGradientManager>("FadeIn");
  ShowBuiltInResource<ColorGradientManager>("FadeOut");
  ShowBuiltInResource<ColorGradientManager>("FadeInOut");

  // NetChannelConfigs
  ShowBuiltInResource<NetChannelConfigManager>("Transform");
  ShowBuiltInResource<NetChannelConfigManager>("ClientAuthority");

  //tiles...
  ShowBuiltInResource<PhysicsMeshManager>("Box");
  ShowBuiltInResource<PhysicsMeshManager>("DoubleSlopeLeft1");
  ShowBuiltInResource<PhysicsMeshManager>("DoubleSlopeLeft2");
  ShowBuiltInResource<PhysicsMeshManager>("DoubleSlopeLeftInv1");
  ShowBuiltInResource<PhysicsMeshManager>("DoubleSlopeLeftInv2");
  ShowBuiltInResource<PhysicsMeshManager>("DoubleSlopeRight1");
  ShowBuiltInResource<PhysicsMeshManager>("DoubleSlopeRight2");
  ShowBuiltInResource<PhysicsMeshManager>("DoubleSlopeRightInv1");
  ShowBuiltInResource<PhysicsMeshManager>("DoubleSlopeRightInv2");
  ShowBuiltInResource<PhysicsMeshManager>("HalfBoxBottom");
  ShowBuiltInResource<PhysicsMeshManager>("HalfBoxLeft");
  ShowBuiltInResource<PhysicsMeshManager>("HalfBoxRight");
  ShowBuiltInResource<PhysicsMeshManager>("HalfBoxTop");
  ShowBuiltInResource<PhysicsMeshManager>("HalfSlopeLeft1");
  ShowBuiltInResource<PhysicsMeshManager>("HalfSlopeLeft2");
  ShowBuiltInResource<PhysicsMeshManager>("HalfSlopeLeftInv1");
  ShowBuiltInResource<PhysicsMeshManager>("HalfSlopeLeftInv2");
  ShowBuiltInResource<PhysicsMeshManager>("HalfSlopeRight1");
  ShowBuiltInResource<PhysicsMeshManager>("HalfSlopeRight2");
  ShowBuiltInResource<PhysicsMeshManager>("HalfSlopeRightInv1");
  ShowBuiltInResource<PhysicsMeshManager>("HalfSlopeRightInv2");
  ShowBuiltInResource<PhysicsMeshManager>("SlopeLeft");
  ShowBuiltInResource<PhysicsMeshManager>("SlopeLeftInv");
  ShowBuiltInResource<PhysicsMeshManager>("SlopeRight");
  ShowBuiltInResource<PhysicsMeshManager>("SlopeRightInv");

  ShowBuiltInResource<SpriteSourceManager>("CameraIcon");
  ShowBuiltInResource<SpriteSourceManager>("Circle");
  ShowBuiltInResource<SpriteSourceManager>("CircleBordered");
  ShowBuiltInResource<SpriteSourceManager>("ConnectionIcon");
  ShowBuiltInResource<SpriteSourceManager>("LightIcon");
  ShowBuiltInResource<SpriteSourceManager>("SelectIcon");
  ShowBuiltInResource<SpriteSourceManager>("Square");
  ShowBuiltInResource<SpriteSourceManager>("SquareBordered");

  ShowBuiltInResource<PhysicsSolverConfigManager>("Baumgarte");
  ShowBuiltInResource<PhysicsSolverConfigManager>("PostStabilization");


  float time = (float)timer.UpdateAndGetTime();
  ZPrint("Finished Loading Editor Content in %.2f\n", time);
  return true;
}

} // namespace Zero
