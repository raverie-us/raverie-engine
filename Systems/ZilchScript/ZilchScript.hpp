///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------- ZilchScript Resource
/// Zilch script file Resource.
class ZilchScript : public ZilchDocumentResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // DocumentResource Interface.
  void ReloadData(StringRange data) override;

  // ICodeInspector Interface.
  void GetKeywords(Array<Completion>& keywordsOut) override;

  // ZilchDocumentResource Interface.
  void GetLibraries(Array<LibraryRef>& libraries) override;
  void GetLibrariesRecursive(Array<LibraryRef>& libraries, ResourceLibrary* library);
};

//------------------------------------------------------------- ZilchScriptLoader
class ZilchScriptLoader : public ResourceLoader
{
public:
  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override;
  HandleOf<Resource> LoadFromBlock(ResourceEntry& entry) override;
  void ReloadFromFile(Resource* resource, ResourceEntry& entry) override;
};

//------------------------------------------------------------- ZilchScript Manager
// Resource manager for ZilchScript
class ZilchScriptManager : public ResourceManager
{
public:
  DeclareResourceManager(ZilchScriptManager, ZilchScript);

  ZilchScriptManager(BoundType* resourceType);
  
  // ResourceManager Interface
  void ValidateNewName(Status& status, StringParam name, BoundType* optionalType) override;
  void ValidateRawName(Status& status, StringParam name, BoundType* optionalType) override;
  String GetTemplateSourceFile(ResourceAdd& resourceAdd) override;

  //Internals
  void OnResourceLibraryConstructed(ObjectEvent* e);

  static void DispatchScriptError(StringParam eventId, StringParam shortMessage, StringParam fullMessage, const CodeLocation& location);
  static void DispatchZeroZilchError(const CodeLocation& location, StringParam message, Project* buildingProject);
  
  static void OnMemoryLeak(MemoryLeakEvent* event);
  
  // We ignore duplicate exceptions until the version is incremented
  HashSet<String> mDuplicateExceptions;
  int mLastExceptionVersion;
};

}//namespace Zero
