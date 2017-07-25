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
  void ValidateName(Status& status, StringParam name) override;
  String GetTemplateSourceFile(ResourceAdd& resourceAdd) override;

  //Internals
  void OnResourcesLoaded(ResourceEvent* event);
  void OnPreZilchProjectCompilation(ZilchPreCompilationEvent* e);
  // We need to periodically pump the debugger for messages
  void OnEngineUpdate(Event* event);

  static void TypeParsedCallback(Zilch::ParseEvent* e, void* userData);
  static void ValidateAttribute(Attribute& attribute, CodeLocation& location, HashSet<String>& allowedAttributes, StringParam attributeClassification, Project* buildingProject);
  static void ValidateAttributes(Array<Attribute>& attributes, CodeLocation& location, HashSet<String>& allowedAttributes, StringParam attributeClassification, Project* buildingProject);
  static void CheckDependencies(BoundType* classType, Property* property, Project* buildingProject);
  static void DispatchScriptError(StringParam eventId, StringParam shortMessage, StringParam fullMessage, const CodeLocation& location);
  static void DispatchZeroZilchError(const CodeLocation& location, StringParam message, Project* buildingProject);
  
  static void OnMemoryLeak(MemoryLeakEvent* event);
  
  // The debugger interface that we register states with
  //Debugger mDebugger;

  // All allowed attributes for properties, classes, and functions
  HashSet<String> mAllowedClassAttributes;
  HashSet<String> mAllowedFunctionAttributes;
  HashSet<String> mAllowedGetSetAttributes;
};

}//namespace Zero
