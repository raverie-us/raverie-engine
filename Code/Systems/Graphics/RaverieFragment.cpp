// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(RaverieFragment, builder, type)
{
}

RaverieFragment::RaverieFragment()
{
}

void RaverieFragment::ReloadData(StringRange data)
{
  RaverieDocumentResource::ReloadData(data);

  mResourceLibrary->FragmentsModified();

  ResourceEvent event;
  event.Manager = RaverieFragmentManager::GetInstance();
  event.EventResource = this;
  event.Manager->DispatchEvent(Events::ResourceModified, &event);
}

void AddResourceLibraries(Array<Raverie::LibraryRef>& libraries, ResourceLibrary* library)
{
  forRange (ResourceLibrary* dependency, library->Dependencies.All())
    AddResourceLibraries(libraries, dependency);

  libraries.PushBack(library->mSwapFragment.mCurrentLibrary);
}

void RaverieFragment::GetKeywords(Array<Completion>& keywordsOut)
{
  RaverieBase::GetKeywords(keywordsOut);

  GraphicsEngine* graphicsEngine = Z::gEngine->has(GraphicsEngine);
  RaverieShaderGenerator* shaderGenerator = graphicsEngine->mShaderGenerator;
  SpirVNameSettings& nameSettings = shaderGenerator->mSpirVSettings->mNameSettings;

  // Create a map of special keywords that we can't use in shaders
  HashSet<String> badKeywords;
  badKeywords.Insert("any");
  badKeywords.Insert("class");
  badKeywords.Insert("debug");
  badKeywords.Insert("delegate");
  badKeywords.Insert("delete");
  badKeywords.Insert("destructor");
  badKeywords.Insert("do");
  badKeywords.Insert("enum");
  badKeywords.Insert("foreach");
  badKeywords.Insert("loop");
  badKeywords.Insert("memberid");
  badKeywords.Insert("new");
  badKeywords.Insert("null");
  badKeywords.Insert("sends");
  badKeywords.Insert("set");
  badKeywords.Insert("typeid");
  badKeywords.Insert("typeof");
  badKeywords.Insert("event");

  AddKeywords(keywordsOut, Grammar::GetUsedKeywords(), badKeywords);
  AddKeywords(keywordsOut, Grammar::GetSpecialKeywords(), badKeywords);

  AddKeywords(keywordsOut, nameSettings.mAllowedClassAttributes);
  AddKeywords(keywordsOut, nameSettings.mAllowedFunctionAttributes);
  AddKeywords(keywordsOut, nameSettings.mAllowedFieldAttributes);
}

void RaverieFragment::AddKeywords(Array<Completion>& keywordsOut, const Array<String>& keyswords, HashSet<String>& keywordsToSkip)
{
  forRange (String& keyword, keyswords.All())
  {
    if (!keywordsToSkip.Contains(keyword))
      keywordsOut.PushBack(keyword);
  }
}

void RaverieFragment::AddKeywords(Array<Completion>& keywordsOut, const HashMap<String, AttributeInfo>& keyswordsToTest)
{
  typedef HashMap<String, AttributeInfo> AttributeMap;
  forRange (AttributeMap::pair& pair, keyswordsToTest.All())
  {
    if (!pair.second.mHidden)
      keywordsOut.PushBack(pair.first);
  }
}

void RaverieFragment::GetLibraries(Array<Raverie::LibraryRef>& libraries)
{
  // Add the core library so we get auto-completion on things like Console
  Raverie::Core& core = Core::GetInstance();
  libraries.Append(core.GetLibrary());
  // Also add the intrinsics library (to get the 'Shader' type).
  libraries.Append(ShaderIntrinsicsLibrary::GetInstance().GetLibrary());

  GetLibrariesRecursive(libraries, mResourceLibrary);
}

void RaverieFragment::GetLibrariesRecursive(Array<LibraryRef>& libraries, ResourceLibrary* library)
{
  forRange (ResourceLibrary* dependency, library->Dependencies.All())
    GetLibrariesRecursive(libraries, dependency);

  if (library->mSwapFragment.mCurrentLibrary != nullptr)
  {
    GraphicsEngine* graphicsEngine = Z::gEngine->has(GraphicsEngine);
    Raverie::LibraryRef wrapperLibrary = library->mSwapFragment.mCurrentLibrary;
    RaverieShaderIRLibrary* internalFragmentLibrary = graphicsEngine->mShaderGenerator->GetInternalLibrary(wrapperLibrary);
    ErrorIf(internalFragmentLibrary == nullptr, "Didn't find an internal library for a wrapper library");
    libraries.PushBack(internalFragmentLibrary->mRaverieLibrary);
  }
}

void RaverieFragment::AttemptGetDefinition(ICodeEditor* editor, size_t cursorPosition, CodeDefinition& definition)
{
  RaverieDocumentResource::AttemptGetDefinition(editor, cursorPosition, definition);
}

HandleOf<Resource> RaverieFragmentLoader::LoadFromFile(ResourceEntry& entry)
{
  RaverieFragmentManager* manager = RaverieFragmentManager::GetInstance();
  RaverieFragment* fragment = new RaverieFragment();
  fragment->DocumentSetup(entry);
  fragment->mText = ReadFileIntoString(entry.FullPath);
  manager->AddResource(entry, fragment);
  return fragment;
}

HandleOf<Resource> RaverieFragmentLoader::LoadFromBlock(ResourceEntry& entry)
{
  RaverieFragmentManager* manager = RaverieFragmentManager::GetInstance();
  RaverieFragment* fragment = new RaverieFragment();
  fragment->DocumentSetup(entry);
  fragment->mText = String((cstr)entry.Block.Data, entry.Block.Size);
  manager->AddResource(entry, fragment);
  return fragment;
}

void RaverieFragmentLoader::ReloadFromFile(Resource* resource, ResourceEntry& entry)
{
  ((RaverieFragment*)resource)->ReloadData(ReadFileIntoString(entry.FullPath));
}

ImplementResourceManager(RaverieFragmentManager, RaverieFragment);

RaverieFragmentManager::RaverieFragmentManager(BoundType* resourceType) : ResourceManager(resourceType), mLastExceptionVersion(-1)
{
  mCategory = "Code";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("Raverie Fragment (*.raveriefrag)", "*.raveriefrag"));
  mNoFallbackNeeded = true;
  mCanCreateNew = true;
  mCanDuplicate = true;
  mSearchable = true;
  mExtension = FileExtensionManager::GetRaverieFragmentTypeEntry()->GetDefaultExtensionNoDot();
  mCanReload = true;

  AddLoader("RaverieFragment", new RaverieFragmentLoader());
}

RaverieFragmentManager::~RaverieFragmentManager()
{
}

void RaverieFragmentManager::ValidateNewName(Status& status, StringParam name, BoundType* optionalType)
{
  // Check all shader types
  GraphicsEngine* graphicsEngine = Z::gEngine->has(GraphicsEngine);
  RaverieShaderIRLibrary* lastFragmentLibrary = graphicsEngine->mShaderGenerator->GetCurrentInternalProjectLibrary();
  if (lastFragmentLibrary)
  {
    RaverieShaderIRType* shaderType = lastFragmentLibrary->FindType(name, true);
    if (shaderType != nullptr)
    {
      status.SetFailed(String::Format("Type '%s' is already a fragment type", name.c_str()));
      return;
    }
  }

  RaverieDocumentResource::ValidateNewScriptName(status, name);
}

void RaverieFragmentManager::ValidateRawName(Status& status, StringParam name, BoundType* optionalType)
{
  RaverieDocumentResource::ValidateRawScriptName(status, name);
}

String RaverieFragmentManager::GetTemplateSourceFile(ResourceAdd& resourceAdd)
{
  RaverieFragment* fragmentTemplate = Type::DynamicCast<RaverieFragment*, Resource*>(resourceAdd.Template);

  ReturnIf(fragmentTemplate == nullptr, String(), "Invalid resource given to create template.");

  // Get the correct template file name
  String templateFile = BuildString("Template", fragmentTemplate->Name);

  // Replace the fragment name where needed
  Replacements replacements;
  Replacement& nameReplacement = replacements.PushBack();
  nameReplacement.MatchString = "RESOURCE_NAME_";
  nameReplacement.ReplaceString = resourceAdd.Name;

  // Replace the tabs with spaces
  Replacement& tabReplacement = replacements.PushBack();
  tabReplacement.MatchString = "\t";
  tabReplacement.ReplaceString = "    ";

  // Two spaces if specified
  TextEditorConfig* config = Z::gEngine->GetConfigCog()->has(TextEditorConfig);
  if (config && config->TabWidth == TabWidth::TwoSpaces)
    tabReplacement.ReplaceString = "  ";

  String fileData = Replace(replacements, fragmentTemplate->mText);

  // Get template data off of resource
  String sourceFile = FilePath::Combine(GetTemporaryDirectory(), resourceAdd.FileName);
  WriteStringRangeToFile(sourceFile, fileData);
  return sourceFile;
}

void RaverieFragmentManager::DispatchScriptError(StringParam eventId, StringParam shortMessage, StringParam fullMessage, const Raverie::CodeLocation& location)
{
  // This should only happen when a composite has a raverie error. Figure out how
  // to report later?
  if (location.CodeUserData == nullptr)
    return;

  RaverieDocumentResource* resource = (RaverieDocumentResource*)location.CodeUserData;

  if (mLastExceptionVersion != RaverieManager::GetInstance()->mVersion)
  {
    mLastExceptionVersion = RaverieManager::GetInstance()->mVersion;
    mDuplicateExceptions.Clear();
  }

  bool isDuplicate = mDuplicateExceptions.Contains(fullMessage);
  mDuplicateExceptions.Insert(fullMessage);

  if (!isDuplicate)
  {
    ScriptEvent e;
    e.Script = resource;
    e.Message = shortMessage;
    e.Location = location;
    Z::gResources->DispatchEvent(eventId, &e);
  }

  Console::Print(Filter::DefaultFilter, "%s", fullMessage.c_str());
}

} // namespace Raverie
