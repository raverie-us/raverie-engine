///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Nathan Carlson
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------ZilchFragment
/// Zilch shader fragment file Resource.
class ZilchFragment : public ZilchDocumentResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ZilchFragment();

  // DocumentResource Interface.
  void ReloadData(StringRange data) override;

  // ICodeInspector Interface.
  void AttemptGetDefinition(ICodeEditor* editor, size_t cursorPosition, CodeDefinition& definition) override;

  // ZilchDocumentResource Interface.
  void GetLibraries(Array<Zilch::LibraryRef>& libraries) override;
  void GetLibrariesRecursive(Array<LibraryRef>& libraries, ResourceLibrary* library);
};

//-------------------------------------------------------------------ZilchFragmentLoader
class ZilchFragmentLoader : public ResourceLoader
{
  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override;
  HandleOf<Resource> LoadFromBlock(ResourceEntry& entry) override;
  void ReloadFromFile(Resource* resource, ResourceEntry& entry) override;
};

//-------------------------------------------------------------------ZilchFragmentManager
class ZilchFragmentManager : public ResourceManager
{
public:
  DeclareResourceManager(ZilchFragmentManager, ZilchFragment);

  ZilchFragmentManager(BoundType* resourceType);
  ~ZilchFragmentManager();

  /// ResourceManager Interface
  void ValidateName(Status& status, StringParam name);

  /// Get the template file for the requested resource type
  String GetTemplateSourceFile(ResourceAdd& resourceAdd) override;

  /// Helper to dispatch script errors.
  void DispatchScriptError(StringParam eventId, StringParam shortMessage, StringParam fullMessage, const Zilch::CodeLocation& location);
};

}//namespace Zero
