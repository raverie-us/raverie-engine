///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2013-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

String ResourceTable::mStringType = "String";

//-------------------------------------------------------------------ResourceTableEntry
ZilchDefineType(ResourceTableEntry, builder, type)
{
  ZilchBindDefaultCopyDestructor();
  ZeroBindDocumented();

  ZilchBindFieldProperty(mName);
  ZilchBindGetterSetterProperty(Value);
  ZilchBindGetterSetterProperty(Weight);
  ZilchBindGetterSetterProperty(Resource);
  ZilchBindFieldProperty(mResourceType);
  ZilchBindMethod(Clone);
}

ResourceTableEntry::ResourceTableEntry()
{
  mResourceType = ResourceTable::mStringType;
}

void ResourceTableEntry::Serialize(Serializer& stream)
{
  SerializeNameDefault(mName, String());
  SerializeNameDefault(mValue, String());
  SerializeNameDefault(mWeight, 1.0f);
}

String ResourceTableEntry::GetValue()
{
  return mValue;
}

void ResourceTableEntry::SetValue(StringParam value)
{
  mValue = mName;
  mResourceType = ResourceTable::mStringType;
}

Resource* ResourceTableEntry::GetResource()
{
  Resource* resource = Z::gResources->GetResourceByTypeAndName(mResourceType, mValue);
  return resource;
}

void ResourceTableEntry::SetResource(Resource* resource)
{
  mValue = resource->Name;
  mResourceType = resource->mManager->mResourceTypeName;
}

float ResourceTableEntry::GetWeight()
{
  return mWeight;
}

void ResourceTableEntry::SetWeight(float weight)
{
  mWeight = Math::Max(weight, 0.0f);
}

String ResourceTableEntry::GetValueOrResourceIdName()
{
  Resource* resource = GetResource();
  if(resource != nullptr)
    return resource->ResourceIdName;
  return mValue;
}

ResourceTableEntry* ResourceTableEntry::Clone()
{
  ResourceTableEntry* newEntry = new ResourceTableEntry();
  *newEntry = *this;
  return newEntry;
}

//-------------------------------------------------------------------ResourceTable
ZilchDefineType(ResourceTable, builder, type)
{
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindMethod(CreateRuntime);
  ZilchBindMethod(RuntimeClone);

  ZilchBindMethod(AddOrError);
  ZilchBindMethod(AddOrIgnore);
  ZilchBindMethod(AddOrOverwrite);
  ZilchBindMethod(GetOrDefault);
  ZilchBindMethod(GetOrError);
  ZilchBindMethod(GetOrNull);
  // Array Get/Set
  ZilchBindOverloadedMethod(Get, ZilchInstanceOverload(ResourceTableEntry*, int));
  ZilchBindOverloadedMethod(Set, ZilchInstanceOverload(void, int, ResourceTableEntry*));
  // HashSet Get/Set
  ZilchBindOverloadedMethod(Get, ZilchInstanceOverload(ResourceTableEntry*, ResourceTableEntry*));
  ZilchBindOverloadedMethod(Set, ZilchInstanceOverload(void, ResourceTableEntry*));
  // HashMap Get/Set
  ZilchBindOverloadedMethod(Get, ZilchInstanceOverload(ResourceTableEntry*, StringParam));
  ZilchBindOverloadedMethod(Set, ZilchInstanceOverload(void, StringParam, ResourceTableEntry*));
  ZilchBindMethod(RemoveAt);
  ZilchBindMethod(RemoveOrError);
  ZilchBindMethod(RemoveOrIgnore);
  ZilchBindMethod(Contains);
  ZilchBindGetterProperty(Count);
  ZilchBindGetterProperty(All);
  ZilchBindMethod(Clear);
  ZilchBindGetterSetterProperty(ResourceType);
  ZilchBindGetterSetterProperty(MaxWeight);
  ZilchBindOverloadedMethod(SampleIndex, ZilchInstanceOverload(uint, float, float));
  ZilchBindOverloadedMethod(Sample, ZilchInstanceOverload(ResourceTableEntry*, float, float));
  ZilchBindMethod(ForceRebuild);
}

ResourceTable::ResourceTable()
{
  mIsOutOfDate = false;
}

ResourceTable::~ResourceTable()
{
  Clear();
}

void ResourceTable::Serialize(Serializer& stream)
{
  SerializeNameDefault(mResourceType, String("String"));
  SerializeNameDefault(mEntryList, EntryList());
  SerializeNameDefault(mMaxWeight, 1.0f);

  // On loading, make sure to mark the table as dirty and to set each entry's resource type
  if(stream.GetMode() == SerializerMode::Loading)
  {
    SetOutOfDate();
    for(size_t i = 0; i < Size(); ++i)
      mEntryList[i]->mResourceType = mResourceType;
  }
}

HandleOf<ResourceTable> ResourceTable::CreateRuntime()
{
  return ResourceTableManager::CreateRuntime();
}

HandleOf<Resource> ResourceTable::Clone()
{
  return RuntimeClone();
}

HandleOf<ResourceTable> ResourceTable::RuntimeClone()
{
  ResourceTable* clone = ResourceTableManager::CreateRuntime();
  CopyTo(clone);
  return clone;
}

void ResourceTable::CopyTo(ResourceTable* destination)
{
  destination->Clear();
  destination->mResourceType = mResourceType;
  destination->mMaxWeight = mMaxWeight;
  for(size_t i = 0; i < Size(); ++i)
  {
    ResourceTableEntry* entry = mEntryList[i];
    destination->AddOrError(entry);
  }
  ForceRebuild();
}

void ResourceTable::AddOrError(ResourceTableEntry* entry)
{
  if(!ValidateEntryType(entry, true))
    return;

  // Try to add the item
  bool didExist = AddOrIgnore(entry);
  // If the item already exists then report an error
  if(didExist)
  {
    String msg = String::Format("An entry with the name '%s' already exists", entry->mName.c_str());
    DoNotifyException("Invalid Add", msg);
  }
}

bool ResourceTable::AddOrIgnore(ResourceTableEntry* entry)
{
  if(!ValidateEntryType(entry, true))
    return false;

  // See if we already have an item with this entry's key.
  // If so just return that the item already exists.
  ResourceTableEntry* existingEntry = GetOrNull(entry->mName);
  if(existingEntry != nullptr)
    return true;

  // Otherwise, add the entry and return that this entry didn't already exist
  AddNewEntry(entry->mName, entry->mValue, entry->mWeight);
  return false;
}

bool ResourceTable::AddOrOverwrite(ResourceTableEntry* entry)
{
  if(!ValidateEntryType(entry, true))
    return false;

  // No matter what, the table is now dirty
  SetOutOfDate();

  // Check to see if this item already exists, if it does then just replace it
  ResourceTableEntry* existingEntry = GetOrNull(entry->mName);
  if(existingEntry != nullptr)
  {
    *existingEntry = *entry;
    return true;
  }

  // Otherwise, just add the entry
  AddNewEntry(entry->mName, entry->mValue, entry->mWeight);
  return false;
}

ResourceTableEntry* ResourceTable::GetOrDefault(StringParam key, ResourceTableEntry* defaultValue)
{
  ResourceTableEntry* result = GetOrNull(key);
  // If the item doesn't exist then return the provided value
  if(result == nullptr)
    return defaultValue;
  return result;
}

ResourceTableEntry* ResourceTable::GetOrError(StringParam key)
{
  ResourceTableEntry* result = GetOrNull(key);
  // If the item doesn't exist then throw an exception
  if(result == nullptr)
  {
    DoNotifyException("Invalid key", String::Format("The key '%s' was not found within the map", key.c_str()));
    return nullptr;
  }
  return result;
}

ResourceTableEntry* ResourceTable::GetOrNull(StringParam key)
{
  return mEntryMap.FindValue(key, nullptr);
}

ResourceTableEntry* ResourceTable::operator[](int index)
{
  // Operator[] doesn't do any checking on the index (use Get instead)
  return mEntryList[index];
}

ResourceTableEntry* ResourceTable::Get(int index)
{
  // Validate the index
  if(index >= GetCount())
  {
    DoNotifyException("Invalid Index", "Accessed array out of bounds.");
    return nullptr;
  }
  return mEntryList[index];
}

void ResourceTable::Set(int index, ResourceTableEntry* entry)
{
  if(!ValidateEntryType(entry, true))
    return;

  // For simplicity, always mark the table as dirty, even if we just throw an exception.
  SetOutOfDate();

  // Validate the index
  if(index >= GetCount())
  {
    DoNotifyException("Invalid Index", "Accessed array out of bounds.");
    return;
  }

  ResourceTableEntry* oldEntry = mEntryList[index];
  // If the old entry has the same name then simply replace it
  // (the key is the same, no hash-map changes are needed)
  if(oldEntry->mName == entry->mName)
  {
    *oldEntry = *entry;
    return;
  }

  // If the new name conflicts with another entry then throw an exception
  ResourceTableEntry* conflictingEntry = GetOrNull(entry->mName);
  if(conflictingEntry != nullptr)
  {
    String msg = String::Format("Cannot set entry with name '%s' because the name already exists in the table", entry->mName.c_str());
    DoNotifyException("Invalid Set", msg);
    return;
  }

  // Otherwise we want to replace the old entry but update it in the map
  mEntryMap.Erase(oldEntry->mName);
  *oldEntry = *entry;
  mEntryMap.Insert(oldEntry->mName, oldEntry);
}

ResourceTableEntry* ResourceTable::Get(ResourceTableEntry* entry)
{
  return GetOrError(entry->mName);
}

void ResourceTable::Set(ResourceTableEntry* entry)
{
  AddOrOverwrite(entry);
}

ResourceTableEntry* ResourceTable::Get(StringParam key)
{
  return GetOrError(key);
}

void ResourceTable::Set(StringParam key, ResourceTableEntry* entry)
{
  // We are providing a hash-map interface even though the table behaves more
  // like a hash set. To do this, ensure that the entry's name matches the key.
  if(key != entry->mName)
  {
    String msg = String::Format("Entry name '%s' must match key '%s'", entry->mName.c_str(), key.c_str());
    DoNotifyException("Invalid Set", msg);
    return;
  }

  AddOrOverwrite(entry);
}

void ResourceTable::RemoveAt(int index)
{
  // Validate the index
  if(index >= GetCount())
  {
    DoNotifyException("Invalid Index", "Accessed array out of bounds.");
    return;
  }

  SetOutOfDate();
  // Remove the entry from the list and table and then delete it
  ResourceTableEntry* entry = mEntryList[index];
  mEntryMap.Erase(entry->mName);
  mEntryList.EraseAt(index);
  delete entry;
}

void ResourceTable::RemoveOrError(StringParam key)
{
  bool existed = RemoveOrIgnore(key);
  if(!existed)
    DoNotifyException("Invalid Remove", String::Format("Key '%s' does not exists", key.c_str()));
}

bool ResourceTable::RemoveOrIgnore(StringParam key)
{
  ResourceTableEntry* entry = mEntryMap.FindValue(key, nullptr);
  // If the entry doesn't exist then do nothing
  if(entry == nullptr)
    return false;

  SetOutOfDate();
  // Remove the first occurrence of this pointer from the list (have to perform a linear search)
  mEntryList.EraseValueError(entry);
  mEntryMap.Erase(key);
  // Return that we did remove something
  return true;
}

bool ResourceTable::Contains(StringParam key)
{
  return GetOrNull(key) != nullptr;
}

uint ResourceTable::Size()
{
  return mEntryList.Size();
}

int ResourceTable::GetCount()
{
  return Size();
}

ResourceTableEntryList::range ResourceTable::GetAll()
{
  return mEntryList.All();
}

void ResourceTable::Clear()
{
  DeleteObjectsInContainer(mEntryList);
  mEntryList.Clear();
  mEntryMap.Clear();
  mWeightedTable.Clear();
  mIsOutOfDate = false;
}

String ResourceTable::GetResourceType()
{
  return mResourceType;
}

void ResourceTable::SetResourceType(StringParam resourceType)
{
  SetOutOfDate();
  mResourceType = resourceType;
  // Make sure all entries have the correct resource type set
  for(size_t i = 0; i < mEntryList.Size(); ++i)
    mEntryList[i]->mResourceType = mResourceType;
}

ResourceTable::WeightType ResourceTable::GetMaxWeight()
{
  return mMaxWeight;
}

void ResourceTable::SetMaxWeight(WeightType maxWeight)
{
  SetOutOfDate();
  // Clamp all of the weights to the new max weight
  mMaxWeight = maxWeight;
  for(uint i = 0; i < mEntryList.Size(); ++i)
  {
    WeightType newWeight = Math::Min(maxWeight, mEntryList[i]->mWeight);
    mEntryList[i]->mWeight = newWeight;
  }
}

uint ResourceTable::SampleIndex(float random1, float random2)
{
  // Deal with the table being empty...there's no legit index to return
  if(Size() == 0)
    return 0;

  // Rebuild the weight table and the map if they're out of date
  BuildIfOutOfDate();

  // Make sure the samples are in the correct range
  if(random1 >= 1.000001f || random1 < -0.000001f || random2 >= 1.000001f || random2 < -0.000001f)
  {
    String msg = String::Format("Random values expected to be in the range "
      "of [0,1), given %g and %g. Clamping values but this will "
      "result in a incorrect distribution.", random1, random2);
    DoNotifyWarning("Invalid random range.",msg);
  }
  random1 = Math::Clamp(random1, 0.0f, .999f);
  random2 = Math::Clamp(random2, 0.0f, .999f);

  return mWeightedTable.SampleIndex(random1, random2);
}

ResourceTableEntry* ResourceTable::Sample(float random1, float random2)
{
  // Can't sample the table if it's empty
  if(Size() == 0)
    return nullptr;

  // Sample for an index and then return the corresponding entry
  uint index = SampleIndex(random1, random2);
  ResourceTableEntry* result = mEntryList[index];
  return result;
}

void ResourceTable::ForceRebuild()
{
  mIsOutOfDate = false;

  // Rebuild the map and table
  RebuildMap();
  RebuildTable();
}

void ResourceTable::AddNewEntry(StringParam value)
{
  // Start with a name based upon the current size
  uint index = mEntryList.Size() + 1;
  String entryName = String::Format("Name%d", index);
  // However, we need unique item names so make sure that we don't
  // have one with the same name already. If so just keep incrementing
  // the index until we get a valid name
  while(GetOrNull(entryName) != nullptr)
  {
    ++index;
    entryName = String::Format("Name%d", index);
  }

  // Now set the value string for this item. If no value was passed in
  // then use the same index that we used for the name
  String entryValue = value;
  if(entryValue.Empty())
    entryValue = String::Format("Value%d", index);

  // Finally, insert that item into the table with a default weight of 1
  AddNewEntry(entryName, entryValue, real(1.0));
}

bool ResourceTable::AddNewEntry(const ValueType& name, const ValueType& value, const WeightType& prob)
{
  // Check to see if we already have an item by this name.
  // If so we can't add it so return false.
  if(mEntryMap.FindPointer(name) != nullptr)
    return false;

  SetOutOfDate();
  // Create the entry
  ResourceTableEntry* entry = new ResourceTableEntry();
  entry->mName = name;
  entry->mValue = value;
  entry->mWeight = prob;
  entry->mResourceType = mResourceType;

  // Grab the manager (if we're in string mode this fails, but that's
  // fine because then we'll just use the passed in value
  ResourceManager* manager = Z::gResources->Managers.FindValue(mResourceType, nullptr);
  if(manager != nullptr)
  {
    // If there was a manager, make sure that the given value is actually a
    // resource. Otherwise use the default resource
    if(manager->GetResource(value, ResourceNotFound::ReturnNull) == nullptr)
      entry->mValue = manager->GetDefaultResource()->ResourceIdName;
  }

  // Add the entry to both the list and map, the weighted-table will be built on demand.
  mEntryList.PushBack(entry);
  mEntryMap.Insert(name, entry);
  return true;
}

void ResourceTable::BuildIfOutOfDate()
{
  if(mIsOutOfDate)
    ForceRebuild();
}

void ResourceTable::SetOutOfDate()
{
  mIsOutOfDate = true;
}

void ResourceTable::RebuildMap()
{
  // Clear out the map and re-insert all the items by name
  mEntryMap.Clear();

  for(uint i = 0; i < mEntryList.Size(); ++i)
    mEntryMap.Insert(mEntryList[i]->mName, mEntryList[i]);
}

void ResourceTable::RebuildTable()
{
  // Clear the underlying weighted probability table
  mWeightedTable.Clear();

  // Re-add all of the items and rebuild the table
  for(uint i = 0; i < mEntryList.Size(); ++i)
    mWeightedTable.AddItem(mEntryList[i]->mValue, mEntryList[i]->mWeight);
  mWeightedTable.BuildTable();
}

void ResourceTable::ValidateEntries()
{
  // Get a manager (if we don't find one then whatever we have must be valid)
  ResourceManager* manager = Z::gResources->Managers.FindValue(mResourceType, nullptr);
  if(manager == nullptr)
    return;

  // Iterate through all items and try to get the resource by name.
  // If that fails then fall back to the default resource
  for(uint i = 0; i < mEntryList.Size(); ++i)
  {
    String resourceIdName = mEntryList[i]->mValue;
    Resource* resource = manager->GetResource(resourceIdName, ResourceNotFound::ReturnNull);
    if(resource == nullptr)
    {
      ErrorContextObject materialBlockMsg("Loading Resource Table", this);
      
      String msg = String::Format("Could not find %s '%s' in table.", mResourceType.c_str(), resourceIdName.c_str());
      DoNotifyErrorWithContext(msg);
    }
  }
}

bool ResourceTable::ValidateEntry(ResourceTableEntry* entry)
{
  // Get a manager (if we don't find one then whatever we have must be valid)
  ResourceManager* manager = Z::gResources->Managers.FindValue(mResourceType, nullptr);
  if(manager == nullptr)
    return true;

  Resource* resource = manager->GetResource(entry->mValue, ResourceNotFound::ReturnNull);
  if(resource == nullptr)
    return false;
  return true;
}

bool ResourceTable::ValidateEntryType(ResourceTableEntry* entry, bool throwException)
{
  if(entry->mResourceType != mResourceType)
  {
    if(throwException)
    {
      String msg = String::Format("Entry type '%s' is invalid with table of type '%s'", entry->mResourceType.c_str(), mResourceType.c_str());
      DoNotifyException("Invalid Entry Type", msg);
    }
    return false;
  }
  return true;
}

Resource* ResourceTable::GetResource(StringParam resourceIdName, ResourceNotFound::Enum notFoundMode)
{
  ResourceManager* manager = Z::gResources->Managers.FindValue(mResourceType, nullptr);
  if(manager == nullptr)
    return nullptr;

  return manager->GetResource(resourceIdName, notFoundMode);
}

//-------------------------------------------------------------------ResourceTableManager
ImplementResourceManager(ResourceTableManager, ResourceTable);

ResourceTableManager::ResourceTableManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  AddLoader("ResourceTable", new TextDataFileLoader<ResourceTableManager>());
  DefaultResourceName = "DefaultResourceTable";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.ResourceTable.data"));
  mCanCreateNew = true;
  mCanDuplicate = true;
  mExtension = DataResourceExtension;

  // Listen for when a package is finished loading so we can
  // validate that all of the resources in the table exist
  ConnectThisTo(Z::gResources, Events::PackagedFinished, OnValidateTables);
}

void ResourceTableManager::OnValidateTables(ResourceEvent* e)
{
  // Validate all resource tables that were from the library that was just built
  forRange(Resource* resource, AllResources())
  {
    ResourceTable* table = static_cast<ResourceTable*>(resource);

    if(table->mContentItem != nullptr)
    {
      String libraryName = table->mContentItem->mLibrary->Name;
      // If this resource table is from a different library then don't validate or rebuild it
      if(libraryName != e->Name)
        continue;
    }

    table->ValidateEntries();
    table->ForceRebuild();
  }
}

}//namespace Zero
