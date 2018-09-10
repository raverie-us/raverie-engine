// Authors: Joshua Davis, Nathan Carlson
// Copyright 2014, DigiPen Institute of Technology

#pragma once

namespace Zero
{

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

  void GetKeywords(Array<Completion>& keywordsOut);
  void AddKeywords(Array<Completion>& keywordsOut, const Array<String>& keyswords, HashSet<String>& keywordsToSkip);
  void AddKeywords(Array<Completion>& keywordsOut, const HashMap<String, AttributeInfo>& keyswordsToTest);

  // ZilchDocumentResource Interface.
  void GetLibraries(Array<Zilch::LibraryRef>& libraries) override;
  void GetLibrariesRecursive(Array<LibraryRef>& libraries, ResourceLibrary* library);
};

class ZilchFragmentLoader : public ResourceLoader
{
  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override;
  HandleOf<Resource> LoadFromBlock(ResourceEntry& entry) override;
  void ReloadFromFile(Resource* resource, ResourceEntry& entry) override;
};

class ZilchFragmentManager : public ResourceManager
{
public:
  DeclareResourceManager(ZilchFragmentManager, ZilchFragment);

  ZilchFragmentManager(BoundType* resourceType);
  ~ZilchFragmentManager();

  /// ResourceManager Interface
  void ValidateNewName(Status& status, StringParam name, BoundType* optionalType);
  void ValidateRawName(Status& status, StringParam name, BoundType* optionalType);

  /// Get the template file for the requested resource type
  String GetTemplateSourceFile(ResourceAdd& resourceAdd) override;

  /// Helper to dispatch script errors.
  void DispatchScriptError(StringParam eventId, StringParam shortMessage, StringParam fullMessage, const Zilch::CodeLocation& location);

  // We ignore duplicate exceptions until the version is incremented
  HashSet<String> mDuplicateExceptions;
  int mLastExceptionVersion;
};

} // namespace Zero
