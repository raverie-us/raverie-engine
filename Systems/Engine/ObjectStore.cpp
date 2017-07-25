///////////////////////////////////////////////////////////////////////////////
///
/// \file ObjectStore.cpp
/// Implementation of the ObjectStore.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(ObjectStore, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);

  ZeroBindDocumented();
  ZilchBindMethod(IsStored);
  ZilchBindMethod(Store);
  ZilchBindMethod(Restore);
  ZilchBindMethod(RestoreOrArchetype);
  ZilchBindMethod(Erase);
  ZilchBindMethod(ClearStore);
  ZilchBindMethod(GetDirectoryPath);
}

void ObjectStore::SetStoreName(StringParam storeName)
{
  mStoreName = storeName;
  mStorePath = String();
  SetUpDirectory();
}

void ObjectStore::SetUpDirectory()
{
  if(mStorePath.Empty())
  {
    //Build the paths and create the directory to contain stored objects.

    //In User documents create a folder call Zero.
    String storePath = FilePath::Combine(GetUserDocumentsDirectory(), "Zero", mStoreName);

    CreateDirectoryAndParents(storePath);

    //save the path
    mStorePath = storePath;
  }
}

String ObjectStore::GetFile(StringParam name)
{
  return FilePath::CombineWithExtension(mStorePath, name, ".data");
}

bool ObjectStore::IsStored(StringParam name)
{
  //Check to is if file exists.
  String storeFile = GetFile(name);
  return FileExists(storeFile);
}

String ObjectStore::GetDirectoryPath()
{
  return mStorePath;
}

//Store an object.
StoreResult::Enum ObjectStore::Store(StringParam name, Cog* object)
{
  ReturnIf(object == nullptr, StoreResult::Failed, "Can not store null object.");

  String storeFile = GetFile(name);

  //Default is added
  StoreResult::Enum result = StoreResult::Added;

  //If the file already exists set the result to replaced.
  if(FileExists(storeFile))
    result = StoreResult::Replaced;

  Status status;
  //Create a text serializer
  TextSaver saver;
  // Add a saving context so that ids are relative
  CogSavingContext savingContext;

  //Attempt to open the file
  saver.Open(status, storeFile.c_str());

  if(status)
  {
    saver.SetSerializationContext(&savingContext);
    saver.SerializePolymorphic(*object);
    saver.Close();
  }
  else
  {
    ZPrintFilter(Filter::DefaultFilter, "Failed to store object %s", name.c_str());
    return StoreResult::Failed;
  }

  return result;
}

//Restore an object.
Cog* ObjectStore::Restore(StringParam name, Space* space)
{
  if(space == nullptr)
  {
    DoNotifyException("Invalid ObjectStore Restore", "The space passed in was null. Cannot restore an object to an invalid space.");
    return nullptr;
  }

  String storeFile = GetFile(name);

  //Test to see if a file exists and restore it from
  //file if it exists
  if(FileExists(storeFile))
  {
    Cog* cog =  space->CreateNamed(storeFile);
    return cog;
  }

  return nullptr;
}

Cog* ObjectStore::RestoreOrArchetype(StringParam name, Archetype* archetype, Space* space)
{
  Cog* cog = Restore(name, space);

  if(cog == nullptr && space != nullptr)
  {
    //Object was not present in store use archetype.
    cog = space->Create(archetype);
    if(cog == nullptr)
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
  if(FileExists(storeFile))
  {
    // Get the path to where we're moving the file to (in the trash directory)
    String trashDirectory = FilePath::Combine(GetUserDocumentsDirectory(), "Zero", "Trash");
    String fileDestination = FilePath::Combine(trashDirectory, BuildString(name, ".Data"));

    // Create the trash directory if it doesn't exist
    CreateDirectory(trashDirectory);

    // Move the file
    MoveFile(fileDestination, storeFile);
  }
}

void ObjectStore::ClearStore()
{
  String trashDirectory = FilePath::Combine(GetUserDocumentsDirectory(), "Zero", "Trash");

  MoveFolderContents(trashDirectory, mStorePath);
}

}
