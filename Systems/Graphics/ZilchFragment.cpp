// Authors: Joshua Davis, Nathan Carlson
// Copyright 2014, DigiPen Institute of Technology

#include "Precompiled.hpp"

namespace Zero
{

//**************************************************************************************************
ZilchDefineType(ZilchFragment, builder, type)
{
}

//**************************************************************************************************
ZilchFragment::ZilchFragment()
{
}

//**************************************************************************************************
void ZilchFragment::ReloadData(StringRange data)
{
  ZilchDocumentResource::ReloadData(data);

  mResourceLibrary->FragmentsModified();

  ResourceEvent event;
  event.Manager = ZilchFragmentManager::GetInstance();
  event.EventResource = this;
  event.Manager->DispatchEvent(Events::ResourceModified, &event);
}

//**************************************************************************************************
void AddResourceLibraries(Array<Zilch::LibraryRef>& libraries, ResourceLibrary* library)
{
  forRange(ResourceLibrary* dependency, library->Dependencies.All())
    AddResourceLibraries(libraries, dependency);

  libraries.PushBack(library->mSwapFragment.mCurrentLibrary);
}

//**************************************************************************************************
void ZilchFragment::GetKeywords(Array<Completion>& keywordsOut)
{
  ZilchBase::GetKeywords(keywordsOut);

  GraphicsEngine* graphicsEngine = Z::gEngine->has(GraphicsEngine);
  ZilchShaderGenerator* shaderGenerator = graphicsEngine->mShaderGenerator;
  NameSettings& nameSettings = shaderGenerator->mSettings->mNameSettings;

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

//**************************************************************************************************
void ZilchFragment::AddKeywords(Array<Completion>& keywordsOut, const Array<String>& keyswords, HashSet<String>& keywordsToSkip)
{
  forRange(String& keyword, keyswords.All())
  {
    if(!keywordsToSkip.Contains(keyword))
      keywordsOut.PushBack(keyword);
  }
}

//**************************************************************************************************
void ZilchFragment::AddKeywords(Array<Completion>& keywordsOut, const HashMap<String, AttributeInfo>& keyswordsToTest)
{
  typedef HashMap<String, AttributeInfo> AttributeMap;
  forRange(AttributeMap::pair& pair, keyswordsToTest.All())
  {
    if(!pair.second.mHidden)
      keywordsOut.PushBack(pair.first);
  }
}

//**************************************************************************************************
void ZilchFragment::GetLibraries(Array<Zilch::LibraryRef>& libraries)
{
  // Add the core library so we get auto-completion on things like Console
  Zilch::Core& core = Core::GetInstance();
  libraries.Append(core.GetLibrary());
  // Also add the intrinsics library (to get the 'Shader' type).
  libraries.Append(ShaderIntrinsicsLibrary::GetInstance().GetLibrary());

  GetLibrariesRecursive(libraries, mResourceLibrary);
}

//**************************************************************************************************
void ZilchFragment::GetLibrariesRecursive(Array<LibraryRef>& libraries, ResourceLibrary* library)
{
  forRange(ResourceLibrary* dependency, library->Dependencies.All())
    GetLibrariesRecursive(libraries, dependency);

  if(library->mSwapFragment.mCurrentLibrary != nullptr)
  {
    GraphicsEngine* graphicsEngine = Z::gEngine->has(GraphicsEngine);
    Zilch::LibraryRef wrapperLibrary = library->mSwapFragment.mCurrentLibrary;
    ZilchShaderLibrary* internalFragmentLibrary = graphicsEngine->mShaderGenerator->GetInternalLibrary(wrapperLibrary);
    ErrorIf(internalFragmentLibrary == nullptr, "Didn't find an internal library for a wrapper library");
    libraries.PushBack(internalFragmentLibrary->mZilchLibrary);
  }
}

//**************************************************************************************************
void ZilchFragment::AttemptGetDefinition(ICodeEditor* editor, size_t cursorPosition, CodeDefinition& definition)
{
  ZilchDocumentResource::AttemptGetDefinition(editor, cursorPosition, definition);
}

//**************************************************************************************************
HandleOf<Resource> ZilchFragmentLoader::LoadFromFile(ResourceEntry& entry)
{
  ZilchFragmentManager* manager = ZilchFragmentManager::GetInstance();
  ZilchFragment* fragment = new ZilchFragment();
  fragment->DocumentSetup(entry);
  fragment->mText = ReadFileIntoString(entry.FullPath);
  manager->AddResource(entry, fragment);
  return fragment;
}

//**************************************************************************************************
HandleOf<Resource> ZilchFragmentLoader::LoadFromBlock(ResourceEntry& entry)
{
  ZilchFragmentManager* manager = ZilchFragmentManager::GetInstance();
  ZilchFragment* fragment = new ZilchFragment();
  fragment->DocumentSetup(entry);
  fragment->mText = String((cstr)entry.Block.Data, entry.Block.Size);
  manager->AddResource(entry, fragment);
  return fragment;
}

//**************************************************************************************************
void ZilchFragmentLoader::ReloadFromFile(Resource* resource, ResourceEntry& entry)
{
  ((ZilchFragment*)resource)->ReloadData(ReadFileIntoString(entry.FullPath));
}

ImplementResourceManager(ZilchFragmentManager, ZilchFragment);

//**************************************************************************************************
ZilchFragmentManager::ZilchFragmentManager(BoundType* resourceType)
  : ResourceManager(resourceType),
    mLastExceptionVersion(-1)
{
  mCategory = "Code";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("Zilch Fragment (*.zilchfrag)", "*.zilchfrag"));
  mNoFallbackNeeded = true;
  mCanCreateNew = true;
  mCanDuplicate = true;
  mSearchable = true;
  mExtension = FileExtensionManager::GetZilchFragmentTypeEntry()->GetDefaultExtensionNoDot();
  mCanReload = true;

  AddLoader("ZilchFragment", new ZilchFragmentLoader());
}

//**************************************************************************************************
ZilchFragmentManager::~ZilchFragmentManager()
{
}

//**************************************************************************************************
void ZilchFragmentManager::ValidateNewName(Status& status, StringParam name, BoundType* optionalType)
{
  // Check all shader types
  GraphicsEngine* graphicsEngine = Z::gEngine->has(GraphicsEngine);
  ZilchShaderLibrary* lastFragmentLibrary = graphicsEngine->mShaderGenerator->GetCurrentInternalProjectLibrary();
  if (lastFragmentLibrary)
  {
    ShaderType* shaderType = lastFragmentLibrary->FindType(name, true);
    if (shaderType != nullptr)
    {
      status.SetFailed(String::Format("Type '%s' is already a fragment type", name.c_str()));
      return;
    }
  }

  ZilchDocumentResource::ValidateNewScriptName(status, name);
}

//**************************************************************************************************
void ZilchFragmentManager::ValidateRawName(Status& status, StringParam name, BoundType* optionalType)
{
  ZilchDocumentResource::ValidateRawScriptName(status, name);
}

//**************************************************************************************************
String ZilchFragmentManager::GetTemplateSourceFile(ResourceAdd& resourceAdd)
{
  ZilchFragment* fragmentTemplate = Type::DynamicCast<ZilchFragment*, Resource*>(resourceAdd.Template);

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
  if(config && config->TabWidth == TabWidth::TwoSpaces)
    tabReplacement.ReplaceString = "  ";

  String fileData = Replace(replacements, fragmentTemplate->mText);

  // Get template data off of resource
  String sourceFile = FilePath::Combine(GetTemporaryDirectory(), resourceAdd.FileName);
  WriteStringRangeToFile(sourceFile, fileData);
  return sourceFile;
}

//**************************************************************************************************
void ZilchFragmentManager::DispatchScriptError(StringParam eventId, StringParam shortMessage,
                                               StringParam fullMessage, const Zilch::CodeLocation& location)
{
  // This should only happen when a composite has a zilch error. Figure out how to report later?
  if(location.CodeUserData == nullptr)
    return;

  ZilchDocumentResource* resource = (ZilchDocumentResource*)location.CodeUserData;

  if (mLastExceptionVersion != ZilchManager::GetInstance()->mVersion)
  {
    mLastExceptionVersion = ZilchManager::GetInstance()->mVersion;
    mDuplicateExceptions.Clear();
  }

  bool isDuplicate = mDuplicateExceptions.Contains(fullMessage);
  mDuplicateExceptions.Insert(fullMessage);

  if (!isDuplicate)
  {
    DebugEngineEvent e;
    e.Handled = false;
    e.Script = resource;
    e.Message = shortMessage;
    e.Location = location;
    Z::gResources->DispatchEvent(eventId, &e);
  }

  Console::Print(Filter::DefaultFilter, "%s", fullMessage.c_str());
}

} // namespace Zero
