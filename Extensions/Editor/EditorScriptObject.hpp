///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class ResourceEvent;

//-------------------------------------------------------- Editor Script Objects
/// DataType needs the following members:
///   Handle<Archetype> mArchetype;
///   MetaType* mScriptComponentMeta;
///   Handle<Cog> mCog;
/// 
/// And the following members:
///   String GetName();

/// Manages the creation and deletion of objects based on 
template <typename DataType>
class EditorScriptObjects : public EventObject
{
public:
  typedef EditorScriptObjects<DataType> ZilchSelf;

  /// Constructor.
  EditorScriptObjects(StringParam attributeName);

  virtual void AddObject(DataType* object) = 0;
  virtual void RemoveObject(DataType* object) = 0;
  virtual DataType* GetObject(StringParam objectName) = 0;
  virtual uint GetObjectCount() = 0;
  virtual DataType* GetObject(uint index) = 0;
  virtual Space* GetSpace(DataType* object) = 0;

  DataType* AddOrUpdate(Archetype* archetype);
  DataType* AddOrUpdate(BoundType* componentType);

  virtual void CreateOrUpdateCog(DataType* object);

  void RemoveObject(Archetype* archetype);

  /// Walks all Scripts and Archetypes to find objects that should be added.
  void OnProjectLoaded(Event*);

  /// When scripts are compiled, look for new objects (or old ones to remove).
  void OnScriptsCompiled(Event*);
  void OnScriptRemoved(ResourceEvent* e);

  /// Given the modified resource, determines if an object should be
  /// created or removed.
  void OnArchetypeModified(ResourceEvent* e);

  bool IsAutoRegister(BoundType* componentType);
  bool ArchetypeObjectExists(StringParam name);

  String mAttributeName;
  String mAutoRegister;
};

//******************************************************************************
template <typename DataType>
EditorScriptObjects<DataType>::EditorScriptObjects(StringParam attributeName) :
  mAttributeName(attributeName),
  mAutoRegister("autoRegister")
{
  ConnectThisTo(Z::gEditor, Events::ProjectLoaded, OnProjectLoaded);

  ConnectThisTo(ZilchManager::GetInstance(), Events::ScriptsCompiledPostPatch, OnScriptsCompiled);
  ConnectThisTo(ZilchScriptManager::GetInstance(), Events::ResourceRemoved, OnScriptRemoved);
}

//******************************************************************************
template <typename DataType>
DataType* EditorScriptObjects<DataType>::AddOrUpdate(Archetype* archetype)
{
  if(archetype == nullptr)
    return nullptr;

  // If there was already a object for this archetype, update
  // it with the new object
  uint objectCount = GetObjectCount();
  for(uint i = 0; i < objectCount; ++i)
  {
    DataType* currObject = GetObject(i);

    if(currObject == nullptr || currObject->GetName() != archetype->Name)
      continue;

    // Object from archetype gets priority
    currObject->mArchetype = archetype;
    currObject->mScriptComponentType = nullptr;

    CreateOrUpdateCog(currObject);

    return currObject;
  }

  // Create a new object
  DataType* newObject = new DataType(archetype);
  CreateOrUpdateCog(newObject);
  AddObject(newObject);

  return newObject;
}

//******************************************************************************
template <typename DataType>
DataType* EditorScriptObjects<DataType>::AddOrUpdate(BoundType* componentType)
{
  if(componentType == nullptr)
    return nullptr;

  // If there was already an object for this archetype, update
  // it with the new object
  uint objectCount = GetObjectCount();
  for(uint i = 0; i < objectCount; ++i)
  {
    DataType* currObject = GetObject(i);

    if(currObject == nullptr || currObject->GetName() != componentType->Name)
      continue;

    // There's already an object for this meta type and we don't have to do
    // anything because script re-initialization should handle the updating
    // of the script component on the object
    return currObject;
  }

  // Create a new object
  DataType* newObject = new DataType(componentType);
  CreateOrUpdateCog(newObject);
  AddObject(newObject);

  return newObject;
}

//******************************************************************************
template <typename DataType>
void EditorScriptObjects<DataType>::CreateOrUpdateCog(DataType* object)
{
  Space* space = GetSpace(object);

  // Destroy the old one if it exists
  object->mCog.SafeDestroy();

  if(Archetype* archetype = object->mArchetype)
  {
    object->mCog = space->Create(archetype);
  }
  else if(object->mScriptComponentType)
  {
    BoundType* componentType = object->mScriptComponentType;
    // Create an empty cog then add the component
    Archetype* archetype = ArchetypeManager::FindOrNull(CoreArchetypes::Empty);
    Cog* cog = space->Create(archetype);
    cog->AddComponentByType(componentType);
    cog->SetName(componentType->Name);
    cog->ClearArchetype();

    Component* component = cog->QueryComponentType(componentType);

    CogInitializer initializer(space, space->GetGameSession());
    component->ScriptInitialize(initializer);

    object->mCog = cog;
  }
}

//******************************************************************************
template <typename DataType>
void EditorScriptObjects<DataType>::RemoveObject(Archetype* archetype)
{
  uint objectCount = GetObjectCount();
  for(uint i = 0; i < objectCount; ++i)
  {
    DataType* currObject = GetObject(i);

    if(currObject && (Archetype*)currObject->mArchetype == archetype)
    {
      if(Cog* cog = currObject->mCog)
        cog->Destroy();

      bool removeObject = true;

      // Now that the object is no longer created from the Archetype,
      // there could still be a script component with 'autoRegister', so we
      // have to attempt to update this object with the autoRegister
      if(BoundType* componentType = MetaDatabase::GetInstance()->FindType(currObject->GetName()))
      {
        if(IsAutoRegister(componentType))
        {
          currObject->mArchetype = nullptr;
          currObject->mScriptComponentType = componentType;
          CreateOrUpdateCog(currObject);
          removeObject = false;
        }
      }

      // Delete the object and remove it
      if(removeObject)
        RemoveObject(currObject);

      return;
    }
  }
}

//******************************************************************************
template <typename DataType>
void EditorScriptObjects<DataType>::OnProjectLoaded(Event*)
{
  // Walk all resources
  forRange(Resource* resource, ArchetypeManager::GetInstance()->ResourceIdMap.Values())
  {
    // Skip core resources
    if(!resource->IsWritable())
      continue;

    // Add the tool if it has the Tool tag
    if(resource->mContentItem->HasTag(mAttributeName))
      AddOrUpdate((Archetype*)resource);
  }

  ConnectThisTo(ArchetypeManager::GetInstance(), Events::ResourceTagsModified, OnArchetypeModified);
  ConnectThisTo(ArchetypeManager::GetInstance(), Events::ResourceRemoved, OnArchetypeModified);
  ConnectThisTo(ArchetypeManager::GetInstance(), Events::ResourceAdded, OnArchetypeModified);
  ConnectThisTo(ArchetypeManager::GetInstance(), Events::ResourceReload, OnArchetypeModified);
}

//******************************************************************************
template <typename DataType>
void EditorScriptObjects<DataType>::OnScriptsCompiled(Event*)
{
  Array<BoundType*> components;
  MetaComposition* composition = ZilchTypeId(Cog)->HasInherited<MetaComposition>();
  composition->Enumerate(components, EnumerateAction::All);

  forRange(BoundType* componentType, components.All())
  {
    if(IsAutoRegister(componentType))
    {
      // Create a new object if there's not already an object archetype
      // with the same name
      if(!ArchetypeObjectExists(componentType->Name))
        AddOrUpdate(componentType);
    }
    else
    {
      DataType* oldObject = GetObject(componentType->Name);
      if(oldObject && oldObject->mScriptComponentType == componentType)
        RemoveObject(oldObject);
    }
  }

  // Look for commands created from scripts who's names may have changed
  // and remove them
  uint objectCount = GetObjectCount();
  for(uint i = 0; i < objectCount; ++i)
  {
    DataType* currObject = GetObject(i);

    if(currObject && currObject->mScriptComponentType && !components.Contains(currObject->mScriptComponentType))
    {
      RemoveObject(currObject);

      // Continue iteration
      i -= 1;
      objectCount -= 1;
    }
  }
}

//******************************************************************************
template <typename DataType>
void EditorScriptObjects<DataType>::OnScriptRemoved(ResourceEvent* e)
{
  Array<BoundType*> components;
  MetaComposition* composition = ZilchTypeId(Cog)->HasInherited<MetaComposition>();
  composition->Enumerate(components, EnumerateAction::All);

  forRange(BoundType* componentType, components.All())
  {
    MetaResource* metaResource = componentType->HasInherited<MetaResource>();
    if(metaResource == nullptr)
      continue;

    // If the resource that "owns" this meta type is being removed, we 
    // want to remove the object
    if(metaResource->mResourceId == e->EventResource->mResourceId)
    {
      uint objectCount = GetObjectCount();
      for(uint i = 0; i < objectCount; ++i)
      {
        DataType* currObject = GetObject(i);

        if(currObject && currObject->mScriptComponentType == componentType)
        {
          RemoveObject(currObject);
          return;
        }
      }
    }
  }
}

//******************************************************************************
template <typename DataType>
void EditorScriptObjects<DataType>::OnArchetypeModified(ResourceEvent* e)
{
  Archetype* archetype = (Archetype*)e->EventResource;

  if(e->RemoveMode != RemoveMode::None)
  {
    RemoveObject(archetype);
    return;
  }

  // Ignore resources that aren't user created
  if(!archetype->IsWritable())
    return;

  bool objectExists = (GetObject(archetype->Name) != nullptr);
  bool hasTag = archetype->mContentItem->HasTag(mAttributeName);

  // Only add it if it doesn't already exist
  if(hasTag)
    AddOrUpdate(archetype);
  else if(objectExists && !hasTag)
    RemoveObject(archetype);
}

//******************************************************************************
template <typename DataType>
bool EditorScriptObjects<DataType>::IsAutoRegister(BoundType* componentType)
{
  // We only care about scripted components
  if (componentType->IsA(ZilchTypeId(ZilchComponent)) == false)
    return false;

  // It must have the correct attribute
  forRange(Attribute& attribute, componentType->Attributes.All())
  {
    if(attribute.Name != mAttributeName)
      continue;

    // Check to see if auto is enabled
    forRange(AttributeParameter& parameter, attribute.Parameters.All())
    {
      if(parameter.Name == mAutoRegister && parameter.BooleanValue)
        return true;
    }
  }

  return false;
}

//******************************************************************************
template <typename DataType>
bool EditorScriptObjects<DataType>::ArchetypeObjectExists(StringParam name)
{
  Archetype* archetype = ArchetypeManager::FindOrNull(name);
  if(archetype)
    return archetype->HasTag(mAttributeName);
  return false;
}

}//namespace Zero
