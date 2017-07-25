///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------CollisionTable
const String cDefaultCollisionGroup = "DefaultGroup";
const String cDefaultCollisionTable = "DefaultCollisionTable";

ZilchDefineType(CollisionTable, builder, type)
{
  ZeroBindDocumented();

  ZeroBindTag(Tags::Physics);

  ZilchBindMethod(CreateRuntime);
  ZilchBindMethod(RuntimeClone);

  ZilchBindOverloadedMethod(FindFilter, (CollisionFilter* (CollisionTable::*)(CollisionGroup*, CollisionGroup*)));
}

CollisionTable::CollisionTable()
{
  mAutoRegister = true;
}

CollisionTable::~CollisionTable()
{
  Clear();
}

void CollisionTable::Serialize(Serializer& stream)
{
  typedef Array<String> StrArray;

  if(stream.GetMode() == SerializerMode::Loading)
  {
    // Load in an array of the registered types
    StrArray RegisteredGroups;
    SerializeName(RegisteredGroups);
    // Register the id of each of these types
    for(uint i = 0; i < RegisteredGroups.Size(); ++i)
    {
      CollisionGroup* group = CollisionGroupManager::Find(RegisteredGroups[i]);
      RegisterGroup(group);
    }
  }
  else
  {
    // Convert each registered id to its string and then add it to an array and serialize it
    StrArray RegisteredGroups;
    RegisteredGroups::range range = mRegisteredGroups.All();
    for(; !range.Empty(); range.PopFront())
    {
      CollisionGroupInstance* instance = range.Front().second;
      RegisteredGroups.PushBack(instance->mResource->ResourceIdName);
    }
    SerializeName(RegisteredGroups);
  }

  SerializeName(mCollisionFilters);
  SerializeNameDefault(mAutoRegister, true);

  // If loading, make sure every filter has the pointer back to this table
  if(stream.GetMode() == SerializerMode::Loading)
  {
    CollisionFilters::range range = mCollisionFilters.All();
    for(; !range.Empty(); range.PopFront())
      range.Front()->mTable = this;
  }
}

void CollisionTable::Save(StringParam filename)
{
  ValidateFilters();

  DataResource::Save(filename);
}

void CollisionTable::Unload()
{
  Clear();
}

HandleOf<CollisionTable> CollisionTable::CreateRuntime()
{
  return CollisionTableManager::CreateRuntime();
}

HandleOf<Resource> CollisionTable::Clone()
{
  return RuntimeClone();
}

HandleOf<CollisionTable> CollisionTable::RuntimeClone()
{
  CollisionTable* table = CollisionTableManager::CreateRuntime();
  CopyTo(table);
  return table;
}

void CollisionTable::CopyTo(CollisionTable* destination)
{
  destination->Clear();

  destination->mAutoRegister = mAutoRegister;
  // Register each collision group that we know about
  for(RegisteredGroups::keyrange range = mRegisteredGroups.Keys(); !range.Empty(); range.PopFront())
  {
    ResourceId id = range.Front();
    CollisionGroup* group = CollisionGroupManager::Find(id);
    destination->RegisterGroup(group);
  }

  // Clone all of our collision filters
  for(size_t i = 0; i < mCollisionFilters.Size(); ++i)
  {
    CollisionFilter* clonedFilter = mCollisionFilters[i]->Clone();
    clonedFilter->mTable = destination;
    destination->mCollisionFilters.PushBack(clonedFilter);
  }
  // Reconfigure the groups (and filters) of the cloned table
  destination->ReconfigureGroups();
}

void CollisionTable::Initialize()
{
  SetDefaults();
  ReconfigureGroups();
}

void CollisionTable::SetDefaults()
{
  // Register the default group
  CollisionGroup* group = CollisionGroupManager::Find(cDefaultCollisionGroup);
  RegisterGroup(group);
}

void CollisionTable::LoadExistingGroups()
{
  // If we don't auto register then we don't load existing groups
  if(!mAutoRegister)
    return;

  // Otherwise, go through all of the groups that exist and register them
  CollisionGroupManager* manager = CollisionGroupManager::GetInstance();
  forRange(Resource* resource, manager->AllResources())
  {
    CollisionGroup* group = static_cast<CollisionGroup*>(resource);
    RegisterGroup(group);
  }
}

void CollisionTable::ValidateFilters()
{
  CollisionFilters::range range = mCollisionFilters.All();
  while(!range.Empty())
  {
    CollisionFilter& filter = *range.Front();
    range.PopFront();

    CollisionGroupManager* manager = CollisionGroupManager::GetInstance();

    /*if(manager->GetResource(filter.TypeA) == nullptr)
    {
    ErrorIf(true, "Resource group %d from pair [%d,%d] does not exist.",
    filter.TypeA, filter.TypeA, filter.TypeB);
    mCollisionFiltersSet.Erase(filter);
    continue;
    }

    if(manager->GetResource(filter.TypeB) == nullptr)
    {
    ErrorIf(true, "Resource group %d from pair [%d,%d] does not exist.",
    filter.TypeB, filter.TypeA, filter.TypeB);
    mCollisionFiltersSet.Erase(filter);
    continue;
    }*/

    cstr typeAName = filter.GetTypeAName().c_str();
    cstr typeBName = filter.GetTypeBName().c_str();

    if(mRegisteredGroups.Find(filter.TypeA).Empty())
    {
      ErrorIf(true, "Resource group %s from pair [%s,%s] was not registered.",
        typeAName, typeAName, typeBName);
      continue;
    }

    if(mRegisteredGroups.Find(filter.TypeB).Empty())
    {
      ErrorIf(true, "Resource group %s from pair [%s,%s] was not registered.",
        typeBName, typeAName, typeBName);
      continue;
    }
    /*
    if((filter.TypeA == filter.TypeB) &&
    filter.mFilterFlags.IsSet(FilterFlags::TypeAMessages) != filter.mFilterFlags.IsSet(FilterFlags::TypeBMessages))
    {
    String msg;
    msg = String::Format("Pair [%s,%s] is not consistent on TypeA and TypeB getting messages. "
    "Please set the flags to be equal. Continuing will automatically set the flag to true.",
    typeAName, typeBName);
    DoNotifyWarning("Invalid Configuration", msg);
    filter.mFilterFlags.SetFlag(FilterFlags::TypeAMessages | FilterFlags::TypeBMessages);
    }

    if(filter.mFilterFlags.IsSet(FilterFlags::TypeAMessages | FilterFlags::TypeBMessages | FilterFlags::SpaceMessages) &&
    !filter.mFilterFlags.IsSet(FilterFlags::StartMessage | FilterFlags::EndMessage))
    {
    String msg = String::Format("Pair [%s,%s] is marked to send messages to "
    "an object or the space, but it does not "
    "send start or end messages.", typeAName, typeBName);
    DoNotifyWarning("Invalid configuration", msg);
    }

    if(!filter.mFilterFlags.IsSet(FilterFlags::TypeAMessages | FilterFlags::TypeBMessages | FilterFlags::SpaceMessages) &&
    filter.mFilterFlags.IsSet(FilterFlags::StartMessage | FilterFlags::EndMessage))
    {
    String msg = String::Format("Pair [%s,%s] sends a start or end message, "
    "but is not marked to send messages to "
    "an object or the space.", typeAName, typeBName);
    DoNotifyWarning("Invalid configuration", msg);
    }*/
  }
}

void CollisionTable::Clear()
{
  // When we remove a collision table, we need to fix all of the spaces that
  // were pointing at it. We also need to make sure that we clean up the memory
  // we allocated for the instances.

  // If CollisionTableManager Instance is null the resource manager is
  // being shutdown so there's no reason to fix spaces.
  if(CollisionTableManager::IsValid() && Name != cDefaultCollisionTable)
  {
    // Get the default filter and fix all spaces that
    // were pointing at this table to use the default table
    CollisionTable* table = CollisionTableManager::Find(cDefaultCollisionTable);
    FixSpaces(table);
  }

  // It's not easy or a wise idea to iterate through a hash map while deleting,
  // so put all of the ids into an array then remove them safely
  Array<ResourceId> ids;
  ids.Reserve(mRegisteredGroups.Size());
  for(RegisteredGroups::range r = mRegisteredGroups.All(); !r.Empty(); r.PopFront())
    ids.PushBack(r.Front().first);

  for(uint i = 0; i < ids.Size(); ++i)
    RemoveGroupInstancesAndFilters(ids[i]);
}

void CollisionTable::RegisterGroup(CollisionGroup* group)
{
  ReturnIf(mRegisteredGroups.Size() >= 31, , "Can only register 32 collision filters per space.");

  // Make a new instance and put it in the map under the group's id
  if(mRegisteredGroups.FindValue(group->mResourceId, nullptr) == nullptr)
  {
    CollisionGroupInstance* instance = group->GetNewInstance();
    instance->mTable = this;
    mRegisteredGroups.Insert(group->mResourceId, instance);
  }

  ReconfigureGroups();
}

void CollisionTable::UnRegisterGroup(CollisionGroup* group)
{
  // Remove the instance and all filters for the given id.
  // Also fix the remaining group masks since a group was removed.
  RemoveGroupInstancesAndFilters(group->mResourceId);
}

CollisionGroupInstance* CollisionTable::GetGroupInstance(ResourceId groupId, RegisteredGroupInstanceAccessMode::Enum accessMode)
{
  RegisteredGroups::range range = mRegisteredGroups.Find(groupId);
  // If we find the item then return it
  if(!range.Empty())
    return range.Front().second;

  // We didn't find it and the user requested null in this case
  if(accessMode == RegisteredGroupInstanceAccessMode::ReturnNull)
    return nullptr;

  // The user wants the default resource
  CollisionGroup* group = CollisionGroupManager::Find(cDefaultCollisionGroup);
  range = mRegisteredGroups.Find(group->mResourceId);
  return range.Front().second;
}

CollisionFilter* CollisionTable::FindFilter(CollisionFilter& pair)
{
  HashedFilters::range range = mHashedFilters.Find(&pair);
  if(range.Empty())
    return nullptr;
  return range.Front();
}

CollisionFilter* CollisionTable::FindFilter(CollisionGroup* groupA, CollisionGroup* groupB)
{
  if(!groupA || !groupB)
    return nullptr;

  CollisionFilter pair(groupA->mResourceId, groupB->mResourceId);
  HashedFilters::range range = mHashedFilters.Find(&pair);
  if(range.Empty())
    return nullptr;
  return range.Front();
}

void CollisionTable::RemoveGroupInstancesAndFilters(ResourceId groupId)
{
  // Attempt to find the collision group in the registered list
  RegisteredGroups::range r = mRegisteredGroups.Find(groupId);
  // If it wasn't found, there's no point in continuing
  if(r.Empty())
    return;

  // Otherwise, remove the group (we delete at the end of the function)
  CollisionGroupInstance* instance = r.Front().second;
  mRegisteredGroups.Erase(groupId);

  // Make sure to remove any filter that referenced this group.
  // We have to iterate from the back to the front so as to not invalidate the index
  uint size = mCollisionFilters.Size();
  for(uint i = size - 1; i < size; --i)
  {
    CollisionFilter* filter = mCollisionFilters[i];
    if(filter->mPair.first == groupId || filter->mPair.second == groupId)
    {
      mCollisionFilters.EraseValueError(filter);
      delete filter;
    }
  }

  // Wince we changed the filters, we have to reconfigure the
  // remaining objects (could be done more efficiently, but shouldn't matter)
  ReconfigureGroups();

  // Finally, we have to fix any objects that were pointing at the old group.
  // This could be done more efficiently, but just reset the spaces
  // that referenced us with a new filter of (this), which will try
  // to reset all groups on every collider.
  FixSpaces(this);

  // We owned the instance so now delete it
  // (if we deleted it earlier someone might have had a reference to it)
  delete instance;
}

void CollisionTable::ReconfigureGroups()
{
  // Pick new group ids for each instance and then
  // set their old mask so they resolve with everything
  uint index = 0;
  RegisteredGroups::range registeredRange = mRegisteredGroups.All();
  for(; !registeredRange.Empty(); registeredRange.PopFront())
  {
    CollisionGroupInstance* instance = registeredRange.Front().second;
    instance->mGroupId = 1 << index;
    instance->mDetectionMask = u32(-1);
    instance->mResolutionMask = u32(-1);
    ++index;
  }

  CollisionGroupInstance* instance1;
  CollisionGroupInstance* instance2;
  // Anything that has a filter means that we need to clear the
  // mask bits corresponding to the pair.
  CollisionFilters::range range = mCollisionFilters.All();
  for(; !range.Empty(); range.PopFront())
  {
    CollisionFilter& pair = *range.Front();

    // Find the group instances for this filter from the registered groups
    RegisteredGroups::range registered = mRegisteredGroups.Find(pair.first());
    instance1 = registered.Front().second;

    registered = mRegisteredGroups.Find(pair.second());
    instance2 = registered.Front().second;

    // Make sure the detection mask is set correctly
    if(!pair.mFilterFlags.IsSet(FilterFlags::SkipDetectingCollision))
    {
      instance1->mDetectionMask |= instance2->mGroupId;
      instance2->mDetectionMask |= instance1->mGroupId;
    }
    else
    {
      instance1->mDetectionMask &= ~instance2->mGroupId;
      instance2->mDetectionMask &= ~instance1->mGroupId;
    }

    // Now make sure the resolution mask is setup correctly
    if(!pair.mFilterFlags.IsSet(FilterFlags::SkipResolution))
    {
      instance1->mResolutionMask |= instance2->mGroupId;
      instance2->mResolutionMask |= instance1->mGroupId;
    }
    else
    {
      instance1->mResolutionMask &= ~instance2->mGroupId;
      instance2->mResolutionMask &= ~instance1->mGroupId;
    }
  }

  // We may have new filters added/removed when this step was called, so
  // remove all of the hashed items and just re-populate from the array
  mHashedFilters.Clear();

  range = mCollisionFilters.All();
  for(; !range.Empty(); range.PopFront())
    mHashedFilters.Insert(range.Front());
}

void CollisionTable::FixSpaces(CollisionTable* newTable)
{
  PhysicsEngine* engine = Z::gEngine->has(PhysicsEngine);
  PhysicsEngine::SpaceList::range spaceRange = engine->GetSpaces();
  for(; !spaceRange.Empty(); spaceRange.PopFront())
  {
    // If a space used this collision table then fix it to use the default
    PhysicsSpace& space = spaceRange.Front();
    if(space.GetCollisionTable() == this)
      space.FixCollisionTable(newTable);
  }
}

CollisionGroup* CollisionTable::GetDefaultGroup()
{
  CollisionGroup* group = CollisionGroupManager::Find(cDefaultCollisionGroup);
  return group;
}

//-------------------------------------------------------------------CollisionTableManager
ImplementResourceManager(CollisionTableManager, CollisionTable);

CollisionTableManager::CollisionTableManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  DefaultResourceName = cDefaultCollisionTable;
  mCanAddFile = true;
  AddLoader("CollisionTable", new TextDataFileLoader<CollisionTableManager>());
  mCategory = "Physics";
  mCanCreateNew = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.CollisionTable.data"));
  mExtension = DataResourceExtension;
  mCanDuplicate = true;

  CollisionGroupManager* manager = CollisionGroupManager::GetInstance();
  ConnectThisTo(manager, Events::ResourceAdded, OnCollisionGroupAdded);
  ConnectThisTo(manager, Events::ResourceRemoved, OnCollisionGroupRemoved);
}

CollisionTable* CollisionTableManager::CreateNewResourceInternal(StringParam name)
{
  CollisionTable* table = new CollisionTable();
  table->SetDefaults();
  table->Initialize();
  table->LoadExistingGroups();
  table->ReconfigureGroups();
  table->mManager = this;
  table->Name = name;
  return table;
}

void CollisionTableManager::OnCollisionGroupAdded(ResourceEvent* event)
{
  // When a new collision group was added, we need to go through all tables
  // and register this new group on them if it is set up for auto-registering.
  CollisionGroup* group = (CollisionGroup*)event->EventResource;

  forRange(Resource* resource, AllResources())
  {
    CollisionTable* table = (CollisionTable*)resource;
    if(!table->CanReference(group))
      continue;

    if(table->mAutoRegister)
      table->RegisterGroup(group);
    // We have to make sure to save the table so that when we load again this
    // group is registered (otherwise a user will try to use this group which
    // should be registered and it won't, resulting in problems)
    if(table->Name != cDefaultCollisionTable && table->mContentItem)
      table->mContentItem->SaveContent();
  }
}

void CollisionTableManager::OnCollisionGroupRemoved(ResourceEvent* event)
{
  // If we are unloading, then our destructor will take care of cleaning up our memory
  if(event->RemoveMode == RemoveMode::Unloading)
    return;

  // When a collision group is removed, we need to go through all tables
  // and unregister this group.
  CollisionGroup* group = (CollisionGroup*)event->EventResource;

  // Take all of the tables and remove the given collision group from it.
  forRange(Resource* resource, AllResources())
  {
    CollisionTable* table = (CollisionTable*)resource;
    if(!table->CanReference(group))
      continue;

    table->UnRegisterGroup(group);
    // Don't re-save the collision table here because we will handle missing items during load.
    // (when we save here our content item might be unloading)
  }
}

}//namespace Zero
