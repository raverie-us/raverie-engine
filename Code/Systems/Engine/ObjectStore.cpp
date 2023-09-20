// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(ObjectStore, builder, type)
{
  type->HandleManager = RaverieManagerId(PointerManager);

  RaverieBindDocumented();

  RaverieBindGetter(EntryCount);

  RaverieBindMethodAs(IsEntryStored, "IsStored")->AddAttribute(DeprecatedAttribute);
  RaverieBindMethod(IsEntryStored);
  RaverieBindMethod(GetEntryAt);
  RaverieBindMethod(Store);
  RaverieBindMethod(Restore);
  RaverieBindMethod(RestoreOrArchetype);
  RaverieBindMethod(Erase);
  RaverieBindMethod(ClearStore);
  RaverieBindMethod(GetDirectoryPath);
}

void ObjectStore::SetStoreName(StringParam storeName)
{
  String storePath = FilePath::Combine(GetUserDocumentsApplicationDirectory(), "Store", storeName);
  if (DirectoryExists(storePath))
    PopulateEntries(storePath);

  mStoreName = storeName;
  mStorePath = String();
}

void ObjectStore::SetupDirectory()
{
  if (mStorePath.Empty())
  {
    // Build the paths and create the directory to contain stored objects.

    // In User documents create a folder call Raverie.
    String storePath = FilePath::Combine(GetUserDocumentsApplicationDirectory(), "Store", mStoreName);

    CreateDirectoryAndParents(storePath);

    // save the path
    mStorePath = storePath;
    PopulateEntries(storePath);
  }
}

void ObjectStore::PopulateEntries(StringParam storePath)
{
  mEntries.Clear();

  FileRange filesInDirectory(storePath);
  for (; !filesInDirectory.Empty(); filesInDirectory.PopFront())
  {
    String filename = filesInDirectory.Front();
    String name = FilePath::GetFileNameWithoutExtension(filename);
    mEntries.PushBack(name);
  }
}

String ObjectStore::GetFile(StringParam name)
{
  SetupDirectory();
  return FilePath::CombineWithExtension(mStorePath, name, ".data");
}

bool ObjectStore::IsEntryStored(StringParam name)
{
  if (!mEntries.Empty())
  {
    int count = mEntries.Size();
    for (int i = 0; i < count; ++i)
    {
      if (mEntries[i] == name)
        return true;
    }

    return false;
  }
  else
  {
    // Check to is if file exists.
    String storeFile = GetFile(name);
    return FileExists(storeFile);
  }
}

uint ObjectStore::GetEntryCount()
{
  return (uint)mEntries.Size();
}

String ObjectStore::GetEntryAt(uint index)
{
  if (mEntries.Empty())
    return String();

  if (index >= mEntries.Size())
    return String();

  return mEntries[index];
}

String ObjectStore::GetDirectoryPath()
{
  SetupDirectory();
  return mStorePath;
}

// Store an object.
StoreResult::Enum ObjectStore::Store(StringParam name, Cog* object)
{
  ReturnIf(object == nullptr, StoreResult::Failed, "Can not store null object.");

  String storeFile = GetFile(name);

  // Default is added
  StoreResult::Enum result = StoreResult::Added;

  // If the file already exists set the result to replaced.
  if (FileExists(storeFile))
    result = StoreResult::Replaced;

  Status status;
  // Create a text serializer
  ObjectSaver saver;
  // Add a saving context so that ids are relative
  CogSavingContext savingContext;

  // Attempt to open the file
  saver.Open(status, storeFile.c_str());

  if (status)
  {
    if (result == StoreResult::Added)
      mEntries.PushBack(name);

    saver.SetSerializationContext(&savingContext);
    saver.SaveFullObject(object);
    saver.Close();
  }
  else
  {
    ZPrintFilter(Filter::DefaultFilter, "Failed to store object %s", name.c_str());
    return StoreResult::Failed;
  }

  return result;
}

// Restore an object.
Cog* ObjectStore::Restore(StringParam name, Space* space)
{
  if (space == nullptr)
  {
    DoNotifyException("Invalid ObjectStore Restore",
                      "The space passed in was null. Cannot restore an object "
                      "to an invalid space.");
    return nullptr;
  }

  String storeFile = GetFile(name);

  // Test to see if a file exists and restore it from
  // file if it exists
  if (FileExists(storeFile))
  {
    Cog* cog = space->CreateNamed(storeFile);
    return cog;
  }

  return nullptr;
}

Cog* ObjectStore::RestoreOrArchetype(StringParam name, Archetype* archetype, Space* space)
{
  Cog* cog = Restore(name, space);

  if (cog == nullptr && space != nullptr)
  {
    // Object was not present in store use archetype.
    cog = space->Create(archetype);
    if (cog == nullptr)
    {
      ZPrintFilter(Filter::DefaultFilter, "Failed to restore object %s", name.c_str());
    }
  }

  return cog;
}

void ObjectStore::Erase(StringParam name)
{
  // Get the name of the file to remove
  String storeFile = GetFile(name);

  // First check if it exists
  if (FileExists(storeFile))
  {
    // Get the path to where we're moving the file to (in the trash directory)
    String trashDirectory = FilePath::Combine(GetUserDocumentsApplicationDirectory(), "StoreTrash");
    String fileDestination = FilePath::Combine(trashDirectory, BuildString(name, ".Data"));

    // Create the trash directory if it doesn't exist
    CreateDirectoryAndParents(trashDirectory);

    // Move the file
    MoveFile(fileDestination, storeFile);

    mEntries.EraseValue(name);
  }
}

void ObjectStore::ClearStore()
{
  mEntries.Clear();

  SetupDirectory();
  String trashDirectory = FilePath::Combine(GetUserDocumentsApplicationDirectory(), "StoreTrash");

  MoveFolderContents(trashDirectory, mStorePath);
}

} // namespace Raverie
