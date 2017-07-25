///////////////////////////////////////////////////////////////////////////////
///
/// \file ObjectStore.hpp
/// Declaration of the ObjectStore.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum3(StoreResult, Added, Replaced, Failed);

/// Object cache is use to store objects at runtime.
class ObjectStore : public ExplicitSingleton<ObjectStore>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  
  /// Set the object store name. This is to prevent
  /// store name conflicts.
  void SetStoreName(StringParam storeName);

  // /Is an object in the store?
  bool IsStored(StringParam name);

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
  //Helper function for file names.
  String GetFile(StringParam name);
  void SetUpDirectory();

  String mStoreName;
  String mStorePath;
};

}
