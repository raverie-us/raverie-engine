///////////////////////////////////////////////////////////////////////////////
///
/// \file ResourceSystem.cpp
/// Implementation of the Resource Manager.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Z
{
  ResourceSystem* gResources = nullptr;
}

namespace Events
{
  DefineEvent(ResourcesLoaded);
  DefineEvent(ResourcesUnloaded);
  DefineEvent(PackagedStarted);
  DefineEvent(PackagedFinished);
}

//-------------------------------------------------------------- Resource System
ZilchDefineType(ResourceSystem, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);

  ZilchBindMethod(GetResourceByName);
  ZilchBindMethod(GetResourceByTypeAndName);
}

ResourceSystem::ResourceSystem()
{
  mDetailedResources = false;

  // Only need to listen for the resource package for 'Loading'
  ConnectThisTo(this, Events::ResourcesLoaded, OnResourcesLoaded);
}

void ResourceSystem::Initialize()
{
  Z::gResources = new ResourceSystem();
}

void ResourceSystem::OnResourcesLoaded(ResourceEvent* event)
{
  if (event->Name == "Loading")
  {
    Z::gEngine->mHaveLoadingResources = true;
    GetDispatcher()->Disconnect(this);
  }
}

void ResourceSystem::RegisterManager(ResourceManager* manager)
{
  Managers.InsertOrError(manager->mResourceType->Name, manager, "Only one resource manager for each type");
  mResourceManagers.PushBack(manager);
}

ResourceManager* ResourceSystem::GetResourceManager(BoundType* resourceType)
{
  return GetResourceManager(resourceType->Name);
}

ResourceManager* ResourceSystem::GetResourceManager(StringParam resourceTypeName)
{
  return Managers.FindValue(resourceTypeName, nullptr);
}

void ResourceSystem::AddLoader(StringParam resourceTypeId,
                               ResourceLoader* loader)
{
  mLoaderMap.Insert(resourceTypeId, loader);
}

ResourceLibrary* ResourceSystem::LoadPackageFile(StringParam fileName)
{
  ResourcePackage* package = new ResourcePackage();
  package->Load(fileName);
  package->Location = FilePath::GetFileName(fileName);

  Status status;
  ResourceLibrary* library = LoadPackage(status,package);
  if(!status)
    DoNotifyError("Failed to load resource package.",status.Message);
  return library;
}

ResourceLibrary* ResourceSystem::LoadPackage(Status& status, ResourcePackage* package)
{
  PushErrorContextObject("Loading", package);

  ResourceLibrary* currentSet = LoadedResourceLibraries.FindValue(package->Name, nullptr);

  if(currentSet)
  {
    ZPrintFilter(Filter::DefaultFilter, 
      "Resource Package Already Loaded '%s'...\n", package->Name.c_str());
    return currentSet;
  }

  ZPrintFilter(Filter::DefaultFilter, 
    "Loading Resource Package '%s'...\n", package->Name.c_str());

  Timer timer;
  timer.Update();

  ResourceEvent event;
  event.Name = package->Name;
  event.Path = package->Location;
  DispatchEvent(Events::PackagedStarted, &event);
  
  // METAREFACTOR this is the worst way to handle this, but it works until we get true dependencies.
  // Find the last resource library (assuming we load Core first, then others)
  // and pretend we're always dependent upon the previous resource library
  ResourceLibrary* lastResourceLibrary = nullptr;
  forRange(ResourceLibrary* library, LoadedResourceLibraries.Values())
  {
    lastResourceLibrary = library;
  }

  ResourceLibrary* resourceLibrary = new ResourceLibrary();

  resourceLibrary->Location = package->Location;
  resourceLibrary->Name = package->Name;
  LoadedResourceLibraries.InsertOrError(package->Name, resourceLibrary);
  
  if (lastResourceLibrary != nullptr)
    resourceLibrary->AddDependency(lastResourceLibrary);

  LoadIntoLibrary(status, resourceLibrary, package, false);

  event.EventResourceLibrary = resourceLibrary;
  DispatchEvent(Events::PackagedFinished, &event);

  float time = (float)timer.UpdateAndGetTime();
  ZPrintFilter(Filter::DefaultFilter, "Loaded Resource Package '%s' in %.2fs\n", 
    package->Name.c_str(), time);

  return resourceLibrary;
}

ResourceLibrary* ResourceSystem::GetResourceLibrary(StringParam name)
{
  return LoadedResourceLibraries.FindValue(name, nullptr);
}

void ResourceSystem::ReloadPackage(ResourceLibrary* resourceLibrary, ResourcePackage* package)
{
  // Load all resources
  for(uint i = 0; i < package->Resources.Size(); ++i)
  {
    ResourceEntry& entry = package->Resources[i];
    entry.FullPath = FilePath::Combine(package->Location, entry.Location);

    HandleOf<Resource> subResource = ResourceIdMap.FindValue(entry.mResourceId, nullptr);
    if(subResource)
      ReloadEntry(subResource, entry);
    else
    {
      //New Add to the resource library
      Status status;
      subResource = LoadEntry(status, entry);
      if(subResource)
        resourceLibrary->Add(subResource, false);
    }
  }

  //Signal that resources are loaded
  ResourceEvent event;
  Z::gResources->DispatchEvent(Events::ResourcesLoaded, &event);
}

void ResourceSystem::UnloadAll()
{
  // Unload all libraries. We want to unload libraries that no one depends on first
  while(!LoadedResourceLibraries.Empty())
  {
    // Find the next leaf library (doesn't have any dependents)
    forRange(ResourceLibrary* library, LoadedResourceLibraries.Values())
    {
      if(library->Dependents.Empty())
      {
        LoadedResourceLibraries.Erase(library->Name);
        library->Unload();
        break;
      }
    }
  }

  // Delete Managers in the reverse order due to dependencies
  int size = (int)mResourceManagers.Size();
  for(int i = size - 1; i >= 0; --i)
    SafeDelete(mResourceManagers[i]);

  mResourceManagers.Clear();

  DeleteObjectsInContainer(mLoaderMap);
}

Resource* ResourceSystem::GetResourceByTypeAndName(StringParam resourceType, 
                                      StringParam resourceName)
{
  //Find resource manager for given type
  ManagerMapType::range r = Managers.Find(resourceType);
  if(r.Empty())
    return nullptr;

  ResourceManager* manager = r.Front().second;
  ///Get the resource from that manager
  Resource* resource = manager->GetResource(resourceName, ResourceNotFound::ReturnNull);
  return resource;
}

Resource* ResourceSystem::GetResourceByName(StringParam resourceIdAndName)
{
  // We need to account for a possible "0x" at the start of the resource id
  uint offset = StringStartsWith0x(resourceIdAndName.All()) ? 2 : 0;
  StringRange resId = resourceIdAndName.SubStringFromByteIndices(offset, cHex64Size + offset);
  ResourceId resourceId = ReadHexString(resId);
  return GetResource(resourceId);
}

Resource* ResourceSystem::GetResource(ResourceId resourceId)
{
  return ResourceIdMap.FindValue(resourceId, nullptr);
}

void ResourceSystem::LoadIntoLibrary(Status& status, ResourceLibrary* resourceLibrary,
                                 ResourcePackage* resourcePackage, bool isNew)
{
  Z::gEngine->LoadingStart();

  StringBuilder errorString;
  uint count = resourcePackage->Resources.Size();

  ProgressType::Enum progressType = count > 1 ? ProgressType::Normal : ProgressType::None;

  for(uint i = 0; i < count; ++i)
  {
    ResourceEntry& entry = resourcePackage->Resources[i];
    entry.mLibrary = resourceLibrary;

    // Blocking update of progress
    Z::gEngine->LoadingUpdate("Loading", resourcePackage->Name, entry.Name, progressType, (float)(i + 1) / count);

    entry.FullPath = FilePath::Combine(resourcePackage->Location, entry.Location);

    Status entryStatus;
    HandleOf<Resource> resource = LoadEntry(entryStatus, entry);
    if(!entryStatus)
    {
      continue;
      // Collect all error messages
      status.SetFailed(String());
      errorString << entryStatus.Message;
      errorString << " \n";
    }

    if(resource)
      resourceLibrary->Add(resource, isNew);
  }

  if(!status)
  {
    status.Message = BuildString("Resource package failed to load. \n", errorString.ToString());
  }

  Z::gEngine->LoadingFinish();

  ResourceEvent event;
  event.Name = resourceLibrary->Name;
  Z::gResources->DispatchEvent(Events::ResourcesLoaded, &event);
}


// ErrorContext for loading resources
class ErrorContextResourceEntry : public ErrorContext
{
public:
  ResourceEntry* mEntry;

  ErrorContextResourceEntry(ResourceEntry* entry)
    :mEntry(entry)
  {
  }

  String GetDescription() override
  {
    return BuildString("Loading ", mEntry->ToString());
  }
};


HandleOf<Resource> ResourceSystem::LoadEntry(Status& status, ResourceEntry& element)
{
  //TimerBlock block(element.Name);

  ErrorContextResourceEntry loadingResourceContext(&element);

  if(mDetailedResources)
    ZPrintFilter(Filter::ResourceFilter, "Loading Resource '%s'\n", element.Name.c_str());

  if(!FileExists(element.FullPath))
  {
    String errMsg = String::Format("Resource file '%s' does not exist.", element.FullPath.c_str());
    status.SetFailed(errMsg);
    ZPrint("%s\n", errMsg.c_str());
    return nullptr;
  }

  // Replace legacy type with Cog
  if (element.Type == "LevelSettings")
    element.Type = "Cog";

  LoaderRange range = mLoaderMap.Find(element.Type);
  if(!range.Empty())
  {
    HandleOf<Resource> newResource =  range.Front().second->LoadFromFile(element);
    //ideally we'd do a check here, but some resources don't load anything (fragments)
    return newResource;
  }
  else
  {
    String errMsg = String::Format("Failed to load file. Could not find loader for type"
                                   "'%s' for file '%s'.", element.Type.c_str(), element.Location.c_str());
    ZPrint("%s\n", errMsg.c_str());
    status.SetFailed(errMsg);
  }
  return nullptr;
}

void ResourceSystem::ReloadEntry(Resource* resource, ResourceEntry& entry)
{
  ResourceManager* manager = resource->GetManager();
  if(manager->mCanReload)
  {
    LoaderRange range = mLoaderMap.Find(entry.Type);
    if(!range.Empty())
    {
      range.Front().second->ReloadFromFile(resource, entry);

      resource->UpdateContentItem(entry.mLibrarySource);

      if(entry.mLibrarySource->EditMode == ContentEditMode::ResourceObject)
        entry.mLibrarySource->mRuntimeResource = resource->mResourceId;
    }
  }
  else
  {
    resource->UpdateContentItem(entry.mLibrarySource);

    String message = String::Format("Can not reload resource type %s named %s." 
      "Restart engine for changes to have an effect.",
      manager->mResourceTypeName.c_str(), resource->Name.c_str());
    //DoNotifyError("Can not reload", message);
  }
}

}//namespace Zero
