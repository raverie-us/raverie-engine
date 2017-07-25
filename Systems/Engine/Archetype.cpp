///////////////////////////////////////////////////////////////////////////////
///
/// \file Archetype.cpp
/// Implementation of the Cog Archetype resource class.
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"


namespace Zero
{

Memory::Heap* Archetype::CacheHeap = new Memory::Heap("Archetypes", Memory::GetRoot());

//---------------------------------------------------------------------------------------- Archetype
ZilchDefineType(Archetype, builder, type)
{
  ZeroBindDocumented();
}

//**************************************************************************************************
Archetype::Archetype()
{
  mCachedObject = cInvalidCogId;
  mStoredType = nullptr;
  mCachedTree = nullptr;
}

//**************************************************************************************************
Archetype::~Archetype()
{
  ClearBinaryCache();
  ClearDataTreeCache();
  SafeDelete(mCachedTree);
}

//**************************************************************************************************
void Archetype::Save(StringParam filename)
{
  Cog* archetypeObject = mCachedObject;
  if(archetypeObject)
  {
    CogSavingContext savingContext;
    savingContext.SavingArchetype = this;
    Status status;

    //TextSaver stream;
    ObjectSaver saver;
    saver.Open(status, filename.c_str());
    if(status.Succeeded())
    {
      saver.SetSerializationContext(&savingContext);
      saver.SaveDefinition(archetypeObject);
      saver.Close();

      // We need to clear all cached data for us and Archetypes that reference us
      ClearDataTreeCache();
      ClearBinaryCache();

      forRange(Resource* archetypeResource, ArchetypeManager::GetInstance()->AllResources())
      {
        // For now we're clearing the cache for all Archetypes, however this should check to see
        // if each Archetype has a dependency on this Archetype before clearing the Cache
        Archetype* archetype = (Archetype*)archetypeResource;
        archetype->ClearDataTreeCache();
        archetype->ClearBinaryCache();
      }
    }
  }
}

//**************************************************************************************************
void Archetype::UpdateContentItem(ContentItem* contentItem)
{
  mContentItem = contentItem;
  mLoadPath = contentItem->GetFullPath();
  Cog* ignore = mCachedObject;
  ArchetypeRebuilder::RebuildArchetypes(this, ignore);
}

//**************************************************************************************************
void Archetype::BinaryCache(Cog* cog, CogCreationContext* creationContext)
{
  return;
  ArchetypeManager* manager = ArchetypeManager::GetInstance();

  ClearBinaryCache();

  CogSavingContext context;
  context.SavingArchetype = this;

  // If the object being cached not yet initialized. Copy the
  // context ids used when loading. So the ids will not change.
  if(creationContext)
  {
    u32 objectContext = cog->mSubContextId;
    forRange(CogCreationContext::IdMapType::value_type entry, creationContext->mContextIdMap.All())
    {
      // Only add objects under this object
      if((entry.first & cContextIdMask) == objectContext)
        context.ContextIdMap[entry.second.NewId] = entry.first & ~cContextIdMask;
    }
  }

  // Serialize to in memory buffer stream
  BinaryBufferSaver saver;
  saver.Open();

  saver.SetSerializationContext(&context);
  saver.SerializePolymorphic(*cog);

  // Allocate a data block to save cached archetype
  mBinaryCache.Size = saver.GetSize();
  mBinaryCache.Data = (byte*)Archetype::CacheHeap->Allocate(mBinaryCache.Size);

  saver.ExtractInto(mBinaryCache);
}

//**************************************************************************************************
void Archetype::CacheDataTree()
{
  ClearDataTreeCache();

  Status status;
  ObjectLoader loader;
  loader.OpenFile(status, mLoadPath);

  loader.CacheModifications(&mLocalCachedModifications);

  mCachedTree = loader.TakeOwnershipOfRoot();

  // Add all modifications from all the base Archetypes
  mAllCachedModifications.Combine(mLocalCachedModifications);
  forRange(Resource* baseResource, GetBaseResources())
  {
    Archetype* baseArchetype = (Archetype*)baseResource;
    mAllCachedModifications.Combine(baseArchetype->mLocalCachedModifications);
  }

  // When we clone and give this to other data files inheriting from this Archetype, we don't want
  // them to inherit our patched properties and objects as we're in a different context. If they
  // need that information, they can query our cached modifications
  mCachedTree->ClearPatchState();
}

//**************************************************************************************************
void Archetype::ClearDataTreeCache()
{
  SafeDelete(mCachedTree);
  mLocalCachedModifications.Clear();
}

//**************************************************************************************************
DataNode* Archetype::GetCachedDataTree()
{
  if(mCachedTree == nullptr)
    CacheDataTree();
  return mCachedTree;
}

//**************************************************************************************************
CachedModifications& Archetype::GetLocalCachedModifications()
{
  if(mCachedTree == nullptr)
    CacheDataTree();
  return mLocalCachedModifications;
}

//**************************************************************************************************
CachedModifications& Archetype::GetAllCachedModifications()
{
  if(mCachedTree == nullptr)
    CacheDataTree();
  return mAllCachedModifications;
}

//**************************************************************************************************
void Archetype::ClearBinaryCache()
{
  if(mBinaryCache)
  {
    ArchetypeManager* manager = ArchetypeManager::GetInstance();
    CacheHeap->Deallocate(mBinaryCache.Data, mBinaryCache.Size);
    mBinaryCache.Data = nullptr;
    mBinaryCache.Size = 0;
  }
}

//**************************************************************************************************
void AddDependencies(Cog* cog, HashSet<ContentItem*>& dependencies)
{
  // Get all resources used by this component
  HashSet<Resource*> usedResources;
  GetResourcesFromProperties(cog, usedResources);

  // Filter runtime and non-writable resources
  forRange(Resource* resource, usedResources.All())
  {
    if(resource->IsWritable() && !resource->IsRuntime())
    {
      dependencies.Insert(resource->mContentItem);

      // Add all dependencies of the resource
      resource->GetDependencies(dependencies);
    }
  }

  forRange(Cog& child, cog->GetChildren())
  {
    AddDependencies(&child, dependencies);
  }
}

//**************************************************************************************************
void Archetype::GetDependencies(HashSet<ContentItem*>& dependencies,
                                HandleParam instance)
{
  Cog* cog = instance.Get<Cog*>();
  ReturnIf(cog == nullptr,,"Instance must be a cog.");
  AddDependencies(cog, dependencies);
}

//**************************************************************************************************
DataNode* Archetype::GetDataTree()
{
  // Return the cached tree if it exists
  if(mCachedTree == nullptr)
    CacheDataTree();

  return mCachedTree->Clone();
}

//**************************************************************************************************
String Archetype::GetStringData()
{
  return ReadFileIntoString(mLoadPath);
}

//---------------------------------------------------------------------------------- ArchetypeLoader
class ArchetypeLoader : public ResourceLoader
{
public:
  String StoredTypeName;
  BoundType* StoredType;
  String FilterTag;


  //************************************************************************************************
  ArchetypeLoader(StringParam objectTypeName, StringParam filterTag)
    :StoredTypeName(objectTypeName), FilterTag(filterTag), StoredType(nullptr)
  {
  }

  //************************************************************************************************
  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override
  {
    Archetype* archetype = new Archetype();

    // If loading in the editor use the content library file.
    if(entry.mLibrarySource)
      archetype->mLoadPath = entry.mLibrarySource->GetFullPath();
    else
      archetype->mLoadPath = entry.FullPath;

    // Find the stored type by typename and save it
    if(StoredType == nullptr)
      StoredType = MetaDatabase::GetInstance()->FindType(StoredTypeName);
    ErrorIf(StoredType == nullptr, "Can not find type named %s", StoredTypeName.c_str());

    // Store the meta type on the archetype
    archetype->mStoredType = StoredType;
    archetype->FilterTag = FilterTag;

    Status status;
    ObjectLoader loader;
    // We just want to read if there's inheritance on the root node, so we don't
    // need it to do any patching. Even if we wanted it to, not all resources
    // have been loaded in yet, so we can't
    loader.mIgnoreDataInheritance = true;
    loader.OpenFile(status, archetype->mLoadPath);

    // Pull the inherit id from the Cog node
    DataNode* root = loader.GetNext();
    archetype->mBaseResourceIdName = root->mInheritedFromId;

    ArchetypeManager::GetInstance()->AddResource(entry, archetype);

    return archetype;
  }

  //************************************************************************************************
  HandleOf<Resource> LoadFromBlock(ResourceEntry& entry)
  {
    WriteToFile(entry.FullPath.c_str(), entry.Block.Data, entry.Block.Size);
    return LoadFromFile(entry);
  }
};

//-------------------------------------------------------------------------------- Archetype Manager
ImplementResourceManager(ArchetypeManager, Archetype);

//**************************************************************************************************
ArchetypeManager::ArchetypeManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  AddLoader("Cog", new ArchetypeLoader("Cog", "Archetype"));
  AddLoader("Space", new ArchetypeLoader("Space", "Space"));
  AddLoader("GameSession", new ArchetypeLoader("GameSession", "GameSession"));

  DefaultResourceName = "DefaultArchetype";
  mCanDuplicate = true;
  mPreview = true;
  mSearchable = true;
  mExtension = DataResourceExtension;
}

//**************************************************************************************************
ArchetypeManager::~ArchetypeManager()
{

}

//**************************************************************************************************
Archetype* ArchetypeManager::MakeNewArchetypeWith(Cog* cog, StringParam newName,
                                                  ResourceId addResourceId, 
                                                  Archetype* inheritedArchetype)
{
  // Empty name is null archetype
  if(newName.Empty())
    return nullptr;

  Archetype* archetype = FindOrNull(newName);
  if(archetype)
  {
    if(inheritedArchetype)
      archetype->mBaseResourceIdName = inheritedArchetype->ResourceIdName;

    if(archetype->mStoredType != ZilchVirtualTypeId(cog))
    {
      DoNotifyWarning("Can not set", "Archetype Contains a different composition type than object's composition type.");
      return nullptr;
    }
    // Archetype already exists, check to make sure it is valid to reuse
    if(!archetype->IsWritable())
    {
      DoNotifyWarning("Can not override", String::Format("Archetype %s is read only. "
                      "Core engine objects can not be changed.", archetype->Name.c_str()));
      return nullptr;
    }
    cog->SetArchetype(archetype);
    return archetype;
  }
  else
  {
    // Check the archetype name
    Status status;
    if(!IsValidName(newName, status))
    {
      String errorString = String::Format("The name %s is not valid. %s", newName.c_str(), status.Message.c_str());
      DoNotifyWarning("Invalid name", errorString.c_str());
      return nullptr;
    }

    // Create a new archetype and save the cog to content
    Archetype* newArchetype = new Archetype();
    newArchetype->Name = newName;
    
    if(inheritedArchetype)
      newArchetype->mBaseResourceIdName = inheritedArchetype->ResourceIdName;

    SaveToContent(cog, newArchetype, addResourceId);

    return newArchetype;
  }
}

//**************************************************************************************************
bool ArchetypeManager::SaveToContent(Cog* object, Archetype* archetype, ResourceId addResourceId)
{
  archetype->mCachedObject = object;
  archetype->mStoredType = ZilchVirtualTypeId(object);
  archetype->FilterTag = archetype->mStoredType->Name;

  if(Z::gRuntimeEditor == nullptr)
  {
    DoNotifyError("Failed to upload", "Can not upload without editor.");
    return false;
  }

  // We can modify read only resources if specified by dev config
  bool canModifyReadOnly = false;
  if(DeveloperConfig* devConfig = Z::gEngine->GetConfigCog()->has(DeveloperConfig))
    canModifyReadOnly = devConfig->mCanModifyReadOnlyResources;

  // Archetype has no content item so create one and upload.
  if(archetype->mContentItem == nullptr)
  {
    // Try to add the resource
    ResourceAdd resourceAdd;
    resourceAdd.Name = archetype->Name;
    resourceAdd.SourceResource = archetype;
    resourceAdd.AddResourceId = addResourceId;

    // Make sure the Archetype is set on the object before we save it
    // The Archetype has not yet been assigned a Resource Id, so the handle
    // stored on the Cog will have a raw pointer to it, so after we save,
    // we need to re-assign the Archetype after adding to the resource system
    object->SetArchetype(archetype);

    Z::gRuntimeEditor->AddResource(this, resourceAdd);

    // Re-assign to get a handle with the resource id in it instead of a raw pointer
    object->SetArchetype(archetype);

    if(resourceAdd.WasSuccessful())
    {
      archetype->mCachedObject = cInvalidCogId;
      archetype->BinaryCache(object);
      archetype->mLoadPath = archetype->mContentItem->GetFullPath();

      ZPrintFilter(Filter::ResourceFilter, "Uploaded to new archetype %s.%s.\n",
        archetype->mContentItem->mLibrary->Name.c_str(),
        archetype->Name.c_str());

      return true;
    }
    else
    {
      DoNotifyError("Failed to save", "Failed to create archetype content item");
      return false;
    }

  }
  else if(archetype->IsWritable() || canModifyReadOnly)
  {
    // Archetype has a writable content item override it
    archetype->mContentItem->SaveContent();
    archetype->mCachedObject = cInvalidCogId;
    archetype->BinaryCache(object);

    ZPrintFilter(Filter::ResourceFilter, "Uploaded to archetype %s.%s.\n",
      archetype->mContentItem->mLibrary->Name.c_str(),
      archetype->Name.c_str());

    archetype->SendModified();

    return true;
  }
  else
  {
    DoNotifyWarning("Can not upload", String::Format("Archetype %s is read only."
      " Built in engine resources can not be changed.", archetype->Name.c_str()));
    return false;
  }
}

//**************************************************************************************************
void ArchetypeManager::FlushBinaryArchetypes()
{
  // A structure change (new serialized values) has invalidated binary 
  // cached archetypes. For simplicity just clear all archetypes.
  forRange(Resource* resource, ResourceIdMap.Values())
  {
    Archetype* archetype = (Archetype*) resource;
    archetype->ClearBinaryCache();
  }
}

//**************************************************************************************************
void ArchetypeManager::ArchetypeModified(Archetype* archetype)
{
  if(archetype == nullptr)
    return;

  /// Archetype has changed ui need to reload.
  ResourceEvent event;
  event.Manager = this;
  event.EventResource = archetype;
  this->DispatchEvent(Events::ResourceReload, &event);

  // Clear the cache for all Archetypes that inherit from the modified Archetype
  forRange(Resource* resource, ArchetypeManager::GetInstance()->AllResources())
  {
    while(resource)
    {
      // Clear the cache
      if(resource == archetype)
      {
        ((Archetype*)resource)->ClearDataTreeCache();

        // An Archetype cannot be in the inheritance chain twice
        break;
      }

      resource = resource->GetBaseResource();
    }
  }
}

}//namespace Zero
