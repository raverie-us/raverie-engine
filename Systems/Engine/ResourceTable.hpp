///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2013-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class RandomContext;

//-------------------------------------------------------------------ResourceTableEntry
/// An entry from a resource table. The resource type of this entry must match the
/// resource type of the table to add/set. If the value is set via string then the type
/// will be implicitly set to string, otherwise the type must be set via the Resource property.
class ResourceTableEntry
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ResourceTableEntry();

  void Serialize(Serializer& stream);

  /// The string value of this entry. Changes this entry's type to String on Set.
  String GetValue();
  void SetValue(StringParam value);
  /// The resource value of this entry. Returns null if the underlying type is not a resource.
  /// Changes this entry's type to the given resource's type on Set.
  Resource* GetResource();
  void SetResource(Resource* resource);
  /// The weight value used to determine how likely this item is to be sampled.
  /// Note: Weights must be positive. Negative weights will be clamped to 0.
  float GetWeight();
  void SetWeight(float weight);

  /// Returns the resource id if this is a resource, otherwise the internal value.
  String GetValueOrResourceIdName();

  /// Creates a new entry with the same values.
  ResourceTableEntry* Clone();

  String mName;
  String mValue;
  float mWeight;
  String mResourceType;
};

typedef Array<ResourceTableEntry*> ResourceTableEntryList;

//-------------------------------------------------------------------ResourceTable
/// A table of resources (or strings) that can be indexed, searched by name, or
/// sampled randomly. The table can be randomly sampled to return an entry into the table.
class ResourceTable : public DataResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  static String mStringType;

  typedef String ValueType;
  typedef float WeightType;
  typedef Math::WeightedProbabilityTable<ValueType> WeightedTableType;
  typedef HashMap<ValueType, ResourceTableEntry*> EntryMap;
  typedef ResourceTableEntryList EntryList;

  ResourceTable();
  ~ResourceTable();

  // Component Interface
  void Serialize(Serializer& stream) override;
  /// Creates a ResourceTable for run-time modifications.
  static HandleOf<ResourceTable> CreateRuntime();
  HandleOf<Resource> Clone() override;
  /// Creates a clone of this table for run-time modifications.
  HandleOf<ResourceTable> RuntimeClone();
  void CopyTo(ResourceTable* destination);

  /// Add the given entry. If another entry with the same name exists then an error is thrown.
  void AddOrError(ResourceTableEntry* entry);
  /// Add the given entry. If another entry with the same name exists then no operation is performed.
  bool AddOrIgnore(ResourceTableEntry* entry);
  /// Add the given entry. If another entry with the same name exists then it is overwritten.
  bool AddOrOverwrite(ResourceTableEntry* entry);

  /// Returns the entry associated with the given key. If no entry matches the key then the provided default is returned.
  ResourceTableEntry* GetOrDefault(StringParam key, ResourceTableEntry* defaultValue);
  /// Returns the entry associated with the given key. If no entry matches the key then an exception is thrown.
  ResourceTableEntry* GetOrError(StringParam key);
  /// Returns the entry associated with the given key. If no entry matches then null is returned.
  ResourceTableEntry* GetOrNull(StringParam key);
  
  ResourceTableEntry* operator[](int index);
  /// Access an item at the given index
  ResourceTableEntry* Get(int index);
  /// Access an item at the given index
  void Set(int index, ResourceTableEntry* entry);
  /// Hash-Set interface. Gets the item with the same name as the entry.
  ResourceTableEntry* Get(ResourceTableEntry* entry);
  /// Hash-Set interface. Sets the item with the same name as the entry.
  void Set(ResourceTableEntry* entry);
  /// Hash-Map interface. Gets via the provided key.
  ResourceTableEntry* Get(StringParam key);
  /// Hash-Map interface. Sets via the provided key.
  void Set(StringParam key, ResourceTableEntry* entry);
  
  /// Removes the item at the given index.
  void RemoveAt(int index);
  /// Removes the entry associated with the given key. If no entry matches an exception is thrown.
  void RemoveOrError(StringParam key);
  /// Removes the entry associated with the given key. If no entry matches then no operation is performed.
  bool RemoveOrIgnore(StringParam key);

  /// Returns if the given key is contained.
  bool Contains(StringParam key);

  /// How many items are stored in the table.
  uint Size();
  /// How many items are stored in the table.
  int GetCount();
  /// Range to iterate over all entries
  ResourceTableEntryList::range GetAll();

  /// Clear all items in the table
  void Clear();

  /// The kind of resource contained in this table. This is either a resource type or "String".
  String GetResourceType();
  void SetResourceType(StringParam resourceType);
  /// The maximum probability weight value that can be stored in the table. Setting this will clamp all weight values
  WeightType GetMaxWeight();
  void SetMaxWeight(WeightType maxWeight);

  /// Samples the table to return a random index into the table. Takes two
  /// (different) random floats from [0,1) in order to sample.
  uint SampleIndex(float random1, float random2);
  /// Samples the table to return a random index into the table given a random context.
  uint SampleIndex(RandomContext* random);

  /// Samples the table to return a random entry. Takes two (different) random floats
  /// from [0,1) in order to sample. Returns an empty string if the table is empty.
  ResourceTableEntry* Sample(float random1, float random2);
  /// Samples the table to return a random entry given a random context.
  ResourceTableEntry* Sample(RandomContext* random);

  /// Force rebuild the weighted probability table.
  void ForceRebuild();
  
  //-------------------------------------------------------------------Internal
  // Add a new entry solely based upon a value. This is almost entirely used by the ui when a new row is added.
  void AddNewEntry(StringParam value = String());
  bool AddNewEntry(const ValueType& name, const ValueType& value, const WeightType& prob);
  
  /// Mark the table as having changes that need to be processed on the next sample.
  void SetOutOfDate();
  void BuildIfOutOfDate();
  void RebuildMap();
  void RebuildTable();
  void ValidateEntries();
  bool ValidateEntry(ResourceTableEntry* entry);
  bool ValidateEntryType(ResourceTableEntry* entry, bool throwException);
  Resource* GetResource(StringParam resourceIdName, ResourceNotFound::Enum notFoundMode = ResourceNotFound::ReturnDefault);

  /// List used to lookup entries by index.
  EntryList mEntryList;
  /// Map used to lookup entries by name.
  EntryMap mEntryMap;
  /// Table used to lookup entries by random sample.
  WeightedTableType mWeightedTable;
  /// Stores the kind of resource this Contains.
  String mResourceType;
  /// Max weight used to clamp the max weight value of all entries in the table.
  /// Also used to control the ui's max height.
  WeightType mMaxWeight;

  /// Dirty bit
  bool mIsOutOfDate;
};

//-------------------------------------------------------------------ResourceTableManager
/// Simple manager for the ResourceTable. Just manages creating the resources.
class ResourceTableManager : public ResourceManager
{
public:
  DeclareResourceManager(ResourceTableManager, ResourceTable);

  ResourceTableManager(BoundType* resourceType);
  void OnValidateTables(ResourceEvent* e);
};

}//namespace Zero
