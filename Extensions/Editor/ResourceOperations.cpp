///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

Resource* AddResourceFromFile(StringParam filePath, StringParam resourceType)
{
  ContentLibrary* contentLibrary = Z::gEditor->mProjectLibrary;
  ResourceLibrary* resourceLibrary = Z::gResources->GetResourceLibrary(contentLibrary->Name);

  if(!FileExists(filePath))
  {
    String message = String::Format("File not found at %s", filePath.c_str());
    DoNotifyError("File not found", message);
    return NULL;
  }

  String fileName = FilePath::GetFileName(filePath);
  ContentItem* newContentItem = contentLibrary->FindContentItemByFileName(fileName);

  //Add to the library and save
  AddContentItemInfo addContent;
  addContent.FileName = fileName;
  addContent.BuilderType = resourceType;
  addContent.Library = contentLibrary;
  addContent.ExternalFile = filePath;
  addContent.OnContentFileConflict = ContentFileConflict::FindNewName;

  Status addContentStatus;

  newContentItem = Z::gContentSystem->AddContentItemToLibrary(addContentStatus, addContent);

  if(addContentStatus.Succeeded())
  {
    //Build a package to load the content items resources
    ResourcePackage package;
    Status status;
    Z::gContentSystem->BuildContentItem(status, newContentItem, package);
    DoNotifyStatus(status);

    Z::gResources->LoadIntoLibrary(status, resourceLibrary, &package, true);
    DoNotifyStatus(status);

    DoEditorSideImporting(&package, NULL);

    DoNotify("File Added", String::Format("File '%s' has been added.", 
      fileName.c_str()) , "Disk");
  }
  else
  {
    String message = BuildString("Failed to import content item. ", addContentStatus.Message);
    DoNotifyError("Failed Import", message);
  }

  return NULL;
}

void AddResourcesFromFiles(const Array<String>& files, StringParam resourceType)
{
  ContentLibrary* contentLibrary = Z::gEditor->mProjectLibrary;
  ResourceLibrary* resourceLibrary = Z::gResources->GetResourceLibrary(contentLibrary->Name);

  if(!contentLibrary || !resourceLibrary)
  {
    DoNotifyError("No library", "No valid content library to add content");
  }

  for(uint i=0;i<files.Size();++i)
    AddResourceFromFile(files[i], resourceType);
}

Resource* AddNewResource(ResourceManager* resourceManager, ResourceAdd& resourceAdd)
{
  String resourceTypeName = resourceManager->mResourceTypeName;

  String resourceExtension = resourceManager->mExtension;

  if(resourceAdd.Library == NULL)
    resourceAdd.Library = Z::gEditor->mProjectLibrary;

  //Writable Check

  // If no name is provided generate one
  if(resourceAdd.Name.Empty())
  {
    //Generate an name and id
    u64 id = GenerateUniqueId64();
    resourceAdd.Name = ToString(id);
    //Use the generated id
    resourceAdd.AddResourceId = id;
  }

  if(resourceAdd.FileName.Empty())
    resourceAdd.FileName = GetResourceFileName(resourceManager, resourceAdd.Name);

  // Name check
  if(resourceManager->ResourceNameMap.FindValue(resourceAdd.Name, 0) != 0)
  {
    DoNotifyError("Failed to add resource", "Name already in use");
    return NULL;
  }

  // Save resource into file in the temp directory
  if(resourceAdd.Template)
    resourceAdd.SourceFile = resourceManager->GetTemplateSourceFile(resourceAdd);

  // If no source resource is provided construct one using meta
  if(resourceAdd.SourceFile.Empty())
  {
    if(resourceAdd.SourceResource == NULL)
      resourceAdd.SourceResource = resourceManager->CreateNewResourceInternal(resourceAdd.Name);

    if(resourceAdd.SourceResource == NULL)
    {
      ErrorIf(true,"The resource returned from CreateNewResource is NULL. "
        "The resource most likely is marked as able to create a new "
        "resource but does not implement the CreateNewResource interface.");
      return NULL;
    }

    // Save the resource to a file
    resourceAdd.SourceFile = FilePath::Combine(GetTemporaryDirectory(), resourceAdd.FileName);
    resourceAdd.SourceResource->Save(resourceAdd.SourceFile);
  }

  // Attempt to Add to the content library
  AddContentItemInfo addContent;
  addContent.FileName = resourceAdd.FileName;
  addContent.Name = resourceAdd.Name;
  addContent.ExternalFile = resourceAdd.SourceFile;
  addContent.OnContentFileConflict = ContentFileConflict::FindNewName;
  addContent.Library = resourceAdd.Library;
  addContent.AddResourceId = resourceAdd.AddResourceId;
  addContent.BuilderType = resourceTypeName;
  addContent.ResourceOwner = resourceAdd.ResourceOwner;

  // Add to the library (this builds the meta data and content file).
  Status addStatus;

  ContentItem* newContentItem = Z::gContentSystem->AddContentItemToLibrary(addStatus, addContent);

  if(addStatus.Succeeded())
  {
    resourceAdd.mSuccess = true;

    //Load the new resource
    resourceAdd.SourceResource = LoadResourceFromNewContentItem(resourceManager, newContentItem, resourceAdd.SourceResource);
  }
  else
  {
    DoNotifyError("Failed to add resource", addStatus.Message);
  }

  return resourceAdd.SourceResource;
}

Resource* DuplicateResource(Resource* resource, StringParam expectedNewName)
{
  ContentLibrary* library = Z::gEditor->mProjectLibrary;
  ResourceManager* resourceManager = resource->GetManager();
  String resourceTypeName = resourceManager->mResourceTypeName;
  ContentItem* contentItem = resource->mContentItem;

  ReturnIf(!contentItem->mResourceIsContentItem, NULL, "Can not duplicate resource.");

  String newName = expectedNewName;
  // If there wasn't a name specified, build up the new full file name.
  if(newName.Empty())
  {
    // Try to find a new name. If the name is taken by another resource
    // or the file name is in use try to find another name.
    uint copyCount = 0;
    for(;;)
    {
      ++copyCount;
      String nameSuffix = String::Format("Copy%d", copyCount);
      newName = BuildString(resource->Name, nameSuffix);

      // Has the name been taken?
      Resource* resource = resourceManager->GetResource(newName, ResourceNotFound::ReturnNull);
      if(resource == NULL)
        break;
    }
  }

  // Copy the resource file
  String newFileName = GetResourceFileName(resourceManager, newName, resource->mContentItem);
  String tempFile = FilePath::Combine(GetTemporaryDirectory(), newFileName);
  String sourceFileName = resource->mContentItem->GetFullPath();

  // Duplicate the resource file into temp
  CopyFile(tempFile, sourceFileName);

  // Add to the library (this builds the meta data and content file).
  AddContentItemInfo addContent;
  addContent.FileName = newFileName;
  addContent.ExternalFile = tempFile;
  addContent.Name = newName;
  addContent.Library = library;

  // We want to copy over the tags of the content item as well
  resource->mContentItem->AddTags(addContent.Tags);

  // Try to add the file
  Status addStatus;
  ContentItem* newContentItem = Z::gContentSystem->AddContentItemToLibrary(addStatus, addContent);

  if(addStatus.Failed())
  {
    DoNotifyError("Failed to add duplicate", addStatus.Message);
    return NULL;
  }

  Resource* duplicate = LoadResourceFromNewContentItem(resourceManager, newContentItem, NULL);
  resourceManager->ResourceDuplicated(resource, duplicate);
  return duplicate;
}

bool RenameResource(Resource* resource, StringParam newName)
{
  //if there's no resource and no resource builder, there is nothing to do.
  if(!(resource && resource->mBuilder))
    return false;

  ResourceManager* resourceManager = resource->GetManager();

  String resourceTypeName = resourceManager->mResourceTypeName;

  String oldName = resource->Name;

  ContentLibrary* library = resource->mContentItem->mLibrary;

  // Do not rename resources from read only content libraries.
  if(!library->GetWritable())
    return false;

  ContentItem* contentItem = resource->mContentItem;

  if(contentItem->mResourceIsContentItem)
  {
    String newFileName = GetResourceFileName(resourceManager, newName, resource->mContentItem);
    bool result = Z::gContentSystem->RenameContentItemFile(contentItem, newFileName);
    // Renamed failed, name already in use
    if (result == false)
    {
      DoNotifyWarning("Rename Failed", "Resource name already in use");
      return result;
    }
  }

  // Rename the 'builder' this came from.
  BuilderComponent* builder = resource->mBuilder;
  builder->Rename(newName);

  // Save the changes
  builder->mOwner->SaveMetaFile();

  // Rename the resource in the ResourceNameMap.
  resourceManager->ResourceNameMap.Erase(resource->Name);

  // Map it under new name
  resourceManager->ResourceNameMap[newName] = resource->mResourceId;

  // Set the new name and build the combined string.
  resource->Name = newName;
  // Converting the resourceId to u64 so we don't get the '0x' at the start
  StringRange hexId = ToString(resource->mResourceId, true);
  resource->ResourceIdName = BuildString(hexId, ":", newName);

  resource->UpdateContentItem(contentItem);

  ResourceEvent event;
  event.Manager = resourceManager;
  event.EventResource = resource;
  resourceManager->DispatchEvent(Events::ResourceModified, &event);
  Z::gResources->DispatchEvent(Events::ResourceModified, &event);

  ZPrintFilter(Filter::ResourceFilter, "Renamed %s %s to %s\n", 
    resourceTypeName.c_str(), oldName.c_str(), newName.c_str());

  return true;
}

bool InPackage(Resource* resource, ResourcePackage* package)
{
  forRange(ResourceEntry& entry , package->Resources.All())
  {
    if(resource->mResourceId == entry.mResourceId)
    {
      return true;
    }
  }
  return false;
}

// Unload all resources that where not loaded when the content item was built. Used for content reloading.
void UnloadInactive(ContentItem* contentItem, ResourceLibrary* resourceLibrary, ResourcePackage* package)
{
  Array<Resource*> resourcesToDelete;

  //Gather all resources from the same content item that are not loaded in the new package
  //Do not remove in place since this will modify the loaded resources on the resource library
  forRange(HandleOf<Resource> resourceHandle, resourceLibrary->Resources.All())
  {
    Resource* resource = resourceHandle;

    //Must be save content item
    if(resource->mContentItem == contentItem)
    {
      //And is missing in the new resource package
      if(!InPackage(resource, package))
      {
        resourcesToDelete.PushBack(resource);
      }
    }
  }

  // Delete the resources from the resource managers
  forRange(Resource* resource, resourcesToDelete.All())
  {
    // Now remove from the resource library
    ResourceLibrary* currentLibrary = resource->mResourceLibrary;
    if(currentLibrary)
      currentLibrary->Remove(resource);

    // Remove the resource from the resource manager
    resource->GetManager()->Remove(resource, RemoveMode::Deleted);
  }
}



void ReloadResource(Resource* resource)
{
  ContentItem* contentItem = resource->mContentItem;
  if(contentItem != nullptr)
    ReloadContentItem(contentItem);
}

void ReloadContentItem(ContentItem* contentItem)
{
  ZPrint("Reloading content '%s'\n", contentItem->Filename.c_str());

  // Save any changes made to the meta file for exporting
  // but do not save the resource (not editing the resource directly)
  contentItem->SaveMetaFile();

  // Get the ResourceLibrary for this library
  ResourceLibrary* resourceLibrary = Z::gResources->GetResourceLibrary(contentItem->mLibrary->Name);

  // Rebuild the content item to get new modified resources
  ResourcePackage package;

  Status status;
  Z::gContentSystem->BuildContentItem(status, contentItem, package);
  DoNotifyStatus(status);

  // Remove this content items old resources (may have been removed)
  UnloadInactive(contentItem, resourceLibrary, &package);

  // Reload resources / load new resources
  Z::gResources->ReloadPackage(resourceLibrary, &package);

  // May have editor-side importing to do (archetypes, etc)
  DoEditorSideImporting(&package, NULL);
}

void EditResourceExternal(Resource* resource)
{
  ContentItem* contentItem = resource->mContentItem;
  if(contentItem == NULL)
    return;

  String file = resource->mContentItem->GetFullPath();
  Os::SystemOpenFile(file.c_str(), Os::Verb::Edit);
}

void RemoveResource(Resource* resource)
{
  ContentItem* contentItem = resource->mContentItem;

  // Does the resource have a content item that needs to be removed?
  if(contentItem == NULL)
    return;

  ContentLibrary* library = contentItem->mLibrary;

  // Do not delete content from read only content libraries.
  if(!library->GetWritable())
    return;

  // Make sure removing the content item succeeded
  if(!Z::gContentSystem->RemoveContentItemFromLibray(contentItem))
    return;

  // Now delete resources
  ResourceLibrary* resourceLibrary = resource->mResourceLibrary;

  // A content item may build more than one resource
  // so delete all resources that came from the same content item
  Array<Resource*> toDelete;

  // Find all resources built from this content item 
  forRange(HandleOf<Resource> resourceHandle, resourceLibrary->Resources.All())
  {
    Resource* currentResource = resourceHandle;
    // Must be the same content item
    if(currentResource->mContentItem == contentItem)
    {
      // Clear references to content
      currentResource->mContentItem = NULL;
      currentResource->mBuilder = NULL;

      toDelete.PushBack(currentResource);
    }
  }

  // Reset all binary archetypes because the resource has been removed.
  // Ids inside the archetype may no longer be valid
  ArchetypeManager::GetInstance()->FlushBinaryArchetypes();

  // Delete the resources from the resource manager
  forRange(Resource* deletingResource, toDelete.All())
  {
    ZPrintFilter(Filter::ResourceFilter, "Removed Resource %s %s.%s.\n",
      ZilchVirtualTypeId(deletingResource)->Name.c_str(), 
      library->Name.c_str(), deletingResource->Name.c_str());

    deletingResource->GetManager()->Remove(deletingResource, RemoveMode::Deleted);

    resourceLibrary->Remove(deletingResource);
  }

  ResourceEvent eventToSend;
  eventToSend.RemoveMode = RemoveMode::Unloading;
  Z::gResources->DispatchEvent(Events::ResourcesUnloaded, &eventToSend);
}

Resource* LoadResourceFromNewContentItem(ResourceManager* resourceManager, ContentItem* newContentItem, Resource* resource)
{
  // Build the content item to get a resource package with one resource
  Status status;
  ResourcePackage package;
  Z::gContentSystem->BuildContentItem(status, newContentItem, package);

  if(status.Failed())
  {
    DoNotifyStatus(status);
    return NULL;
  }

  String resourceTypeName = resourceManager->mResourceTypeName;

  ContentLibrary* contentLibrary = newContentItem->mLibrary;

  // Resource that are added this way only have one resource per content item
  ErrorIf(package.Resources.Size() != 1 && !newContentItem->mIgnoreMultipleResourcesWarning,
    "Multiple resources in content item.");

  if(package.Resources.Size() > 0)
  {
    // ResourceLibrary resource will be loaded into
    ResourceLibrary* resourceLibrary = Z::gResources->GetResourceLibrary(contentLibrary->Name);

    if(resource)
    {
      //if the resource was already created, just add it to the set
      resourceManager->AddResource(package.Resources[0], resource);
      resource->UpdateContentItem(newContentItem);

      if(resourceLibrary)
        resourceLibrary->Add(resource, true);
    }
    else
    {
      //new resource
      Status status;
      // Events are sent by the resource manager
      Z::gResources->LoadIntoLibrary(status, resourceLibrary, &package, true);
      DoNotifyStatus(status);
      
      // RESOURCEREFACTOR Why was this doing what it was doing...????
      //resource = Z::gResources->ResourceIdMap.FindValue(resourceLibrary->Resources.Back().Id, NULL);
      resource = resourceLibrary->Resources.Back();
    }

    if(resource == NULL)
      return NULL;

    // Print that a new resource was added.
    ZPrintFilter(Filter::ResourceFilter, "Added new %s %s.%s.\n", 
      resourceTypeName.c_str(), contentLibrary->Name.c_str(), resource->Name.c_str());

    return resource;
  }

  return NULL;
}

String GetResourceFileName(ResourceManager* resourceManager, StringParam resourceName, ContentItem* current)
{
  String resourceTypeName = resourceManager->mResourceTypeName;

  String resourceExtension = resourceManager->mExtension;

  if(current && resourceExtension.Empty())
  {
    //Use current extension
    resourceExtension = FilePath::GetExtension(current->GetFullPath());
  }

  String fileName = BuildString(resourceName, ".", resourceExtension);

  // In the case of "data" or "bin" files add the resource type name to the 
  // file to prevent conflicts
  if(resourceExtension == DataResourceExtension)
    fileName = BuildString(resourceName, ".", resourceTypeName, ".", resourceExtension);

  return fileName;
}

String GetEditorTrash()
{
  return FilePath::Combine(GetUserDocumentsDirectory(), "ZeroEditor", "Trash");
}

// Code paths for new resource:
//
// 1. No resource, modified
// 2. Has resource, not saving to archetype, owned by an archetype, modified
// 3. Has resource, not saving to archetype, not owned by an archetype, owned by different level
// 4. Has resource, not saving to archetype, not owned by an archetype, owned by level, modified, not only instance
// 5. Has resource, saving to archetype, not owned by an archetype, modified, not only instance
// 6. Has resource, saving to archetype, owned by different archetype

// Code paths for new   1 2 3 4 5 6
// ---------------------------------  O = yes
// Has Resource        |X|O|O|O|O|O|  X = no
// Saving to archetype | |X|X|X|O|O|  ! = yes, but not the one being saved to
// Owned by archetype  | |O|X|X|X|!|
// Owned by level      | | |!|O| | |
// Modified            |O|O| |O|O| |
// Multiple instances  | | | |O|O| |

// 1. Create new object, modify it
// 2. Create one or more copies of an object, use one of them to upload to an archetype, modify the non-archetype
// 3. Copy an object and paste it in a different level
// 4. Create one or more copies of an object, modify one of them
// 5. Create one or more copies of an object, upload a modification of one of them to an archetype
// 6. Upload to an archetype, use archetype to upload to a different archetype
Resource* NewResourceOnWrite(ResourceManager* resourceManager, BoundType* resourceType, StringParam property, Space* space, Resource* resource, Archetype* archetype, bool modified)
{
  // Check for valid resource manager
  if (!resourceManager)
    return resource;

  // Verify correct meta property request regardless of code path
  Property* metaProperty = resourceType->GetProperty(property);
  ErrorIf(!metaProperty, "MetaProperty '%s' not found on type '%s'.", property.c_str(), resourceType->Name.c_str());

  // Must have a space, be in editor mode, and have a level
  if (!space)
    return resource;

  if (!space->IsEditorMode())
    return resource;

  Level* activeLevel = space->GetCurrentLevel();
  if (!activeLevel)
    return resource;

  // Only check for copy if a current resource exists, otherwise go straight to creation
  if (resource)
  {
    // Find number of objects in the level using this resource
    uint instanceCount = 0;
    forRange (Cog& cog, space->AllObjects())
    {
      Component* component = cog.QueryComponentType(resourceType);
      if(!component)
        continue;

      Any var = metaProperty->GetValue(component);
      Resource* foundResource = var.Get<Resource*>();

      if(foundResource == resource)
        ++instanceCount;
    }

    // Possible to get here without a builder on the resource
    if (!resource->mBuilder)
      return resource;

    String resourceIdName = resource->mBuilder->GetResourceOwner();

    // Get the archetype that this resource may belong to from meta data
    ArchetypeManager* archetypeManager = ArchetypeManager::GetInstance();
    Resource* archetypeOwner = archetypeManager->GetResource(resourceIdName, ResourceNotFound::ReturnNull);

    // If saving to archetype
    if (archetype)
    {
      // Uploading to current archetype
      if (archetypeOwner == archetype)
        return resource;

      // If resource does not belong to an archetype, and is not a modification of multiple instances,
      // then this archetype will take ownership
      if (!archetypeOwner && (!modified || instanceCount < 2))
      {
        resource->mBuilder->SetResourceOwner(archetype->ResourceIdName);

        MetaOperations::NotifyObjectModified(resource);

        return resource;
      }
    }
    // When a non-archetype modifies a resource owned by an archetype then it should copy and not check the level
    else if (!archetypeOwner)
    {
      // Get the level that this resource may belong to from meta data
      LevelManager* levelManager = LevelManager::GetInstance();
      Resource* levelOwner = levelManager->GetResource(resourceIdName, ResourceNotFound::ReturnNull);

      // Assign to this level if no previous owner
      if (!levelOwner)
      {
        resource->mBuilder->SetResourceOwner(activeLevel->ResourceIdName);

        MetaOperations::NotifyObjectModified(resource);

        return resource;
      }

      // Belongs to this level and is only instance
      if (activeLevel == levelOwner && (!modified || instanceCount < 2))
        return resource;
    }
  }
  else if (!modified)
  {
    return resource;
  }

  // Create new resource ...

  // Generate name from level and resource type
  String levelName = activeLevel->Name;
  String resourceName;
  uint id = 0;

  do
  {
    resourceName = BuildString(levelName, "_", resourceType->Name, String::Format("%.2d", id));
    ++id;
  } while (resourceManager->GetResource(resourceName, ResourceNotFound::ReturnNull));

  resource = resourceManager->CreateNewResourceInternal(String());

  ResourceAdd resourceAdd;
  resourceAdd.SourceResource = resource;
  resourceAdd.Name = resourceName;

  // Must belong to a level or an archetype
  if (archetype)
    resourceAdd.ResourceOwner = archetype->ResourceIdName;
  else
    resourceAdd.ResourceOwner = activeLevel->ResourceIdName;

  AddNewResource(resourceManager, resourceAdd);

  return resource;
}

}
