// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{

/// Raverie shader fragment file Resource.
class RaverieFragment : public RaverieDocumentResource
{
public:
  RaverieDeclareType(RaverieFragment, TypeCopyMode::ReferenceType);

  RaverieFragment();

  // DocumentResource Interface.
  void ReloadData(StringRange data) override;

  // ICodeInspector Interface.
  void AttemptGetDefinition(ICodeEditor* editor, size_t cursorPosition, CodeDefinition& definition) override;

  void GetKeywords(Array<Completion>& keywordsOut);
  void AddKeywords(Array<Completion>& keywordsOut, const Array<String>& keyswords, HashSet<String>& keywordsToSkip);
  void AddKeywords(Array<Completion>& keywordsOut, const HashMap<String, AttributeInfo>& keyswordsToTest);

  // RaverieDocumentResource Interface.
  void GetLibraries(Array<Raverie::LibraryRef>& libraries) override;
  void GetLibrariesRecursive(Array<LibraryRef>& libraries, ResourceLibrary* library);
};

class RaverieFragmentLoader : public ResourceLoader
{
  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override;
  HandleOf<Resource> LoadFromBlock(ResourceEntry& entry) override;
  void ReloadFromFile(Resource* resource, ResourceEntry& entry) override;
};

class RaverieFragmentManager : public ResourceManager
{
public:
  DeclareResourceManager(RaverieFragmentManager, RaverieFragment);

  RaverieFragmentManager(BoundType* resourceType);
  ~RaverieFragmentManager();

  /// ResourceManager Interface
  void ValidateNewName(Status& status, StringParam name, BoundType* optionalType);
  void ValidateRawName(Status& status, StringParam name, BoundType* optionalType);

  /// Get the template file for the requested resource type
  String GetTemplateSourceFile(ResourceAdd& resourceAdd) override;

  /// Helper to dispatch script errors.
  void DispatchScriptError(StringParam eventId, StringParam shortMessage, StringParam fullMessage, const Raverie::CodeLocation& location);

  // We ignore duplicate exceptions until the version is incremented
  HashSet<String> mDuplicateExceptions;
  int mLastExceptionVersion;
};

} // namespace Raverie
