// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

DeclareEnum3(StoreResult, Added, Replaced, Failed);

/// Object cache is use to store objects at runtime.
class ObjectStore : public ExplicitSingleton<ObjectStore>
{
public:
  RaverieDeclareType(ObjectStore, TypeCopyMode::ReferenceType);

  /// Set the object store name. This is to prevent
  /// store name conflicts.
  void SetStoreName(StringParam storeName);

  /// Is there an entry record for the object in the store?
  bool IsEntryStored(StringParam name);

  /// Get number of entries in the ObjectStore.
  uint GetEntryCount();

  /// Get the ObjectStore entry at the specified index.
  String GetEntryAt(uint index);

  /// Store an object.
  StoreResult::Enum Store(StringParam name, Cog* object);

  /// Restore an object to the space.
  Cog* Restore(StringParam name, Space* space);

  /// Restore an object if it is not stored use the archetype to create it.
  Cog* RestoreOrArchetype(StringParam name, Archetype* archetype, Space* space);

  /// Attempts to remove an object from the store.
  void Erase(StringParam name);

  /// Clear the store
  void ClearStore();

  /// Returns the directory path to the object store
  String GetDirectoryPath();

private:
  // Helper function for file names.
  String GetFile(StringParam name);
  void SetupDirectory();

  /// Populate the internal array of file names in the ObjectStore, if the
  /// proper ObjectStore directory exists.
  void PopulateEntries(StringParam storePath);

  String mStoreName;
  String mStorePath;

  Array<String> mEntries;
};

} // namespace Raverie
