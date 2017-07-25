///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Nathan Carlson
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------ZilchFragment
ZilchDefineType(ZilchFragment, builder, type)
{
}

ZilchFragment::ZilchFragment()
{
}

void ZilchFragment::ReloadData(StringRange data)
{
  ZilchDocumentResource::ReloadData(data);

  mResourceLibrary->FragmentsModified();

  ResourceEvent event;
  event.Manager = ZilchFragmentManager::GetInstance();
  event.EventResource = this;
  event.Manager->DispatchEvent(Events::ResourceModified, &event);
}

void AddResourceLibraries(Array<Zilch::LibraryRef>& libraries, ResourceLibrary* library)
{
  forRange(ResourceLibrary* dependency, library->Dependencies.All())
    AddResourceLibraries(libraries, dependency);

  libraries.PushBack(library->mCurrentFragmentLibrary);
}

void ZilchFragment::GetLibraries(Array<Zilch::LibraryRef>& libraries)
{
  // Add the core library so we get auto-completion on things like Console
  Zilch::Core& core = Core::GetInstance();
  libraries.Append(core.GetLibrary());
  // Also add the intrinsics library (to get the 'Shader' type).
  libraries.Append(ShaderIntrinsicsLibrary::GetInstance().GetLibrary());

  GetLibrariesRecursive(libraries, mResourceLibrary);
}

void ZilchFragment::GetLibrariesRecursive(Array<LibraryRef>& libraries, ResourceLibrary* library)
{
  forRange(ResourceLibrary* dependency, library->Dependencies.All())
    GetLibrariesRecursive(libraries, dependency);

  if(library->mCurrentFragmentLibrary != nullptr)
  {
    GraphicsEngine* graphicsEngine = Z::gEngine->has(GraphicsEngine);
    Zilch::LibraryRef wrapperLibrary = library->mCurrentFragmentLibrary;
    ZilchShaderLibrary* internalFragmentLibrary = graphicsEngine->mShaderGenerator->GetInternalLibrary(wrapperLibrary);
    ErrorIf(internalFragmentLibrary == nullptr, "Didn't find an internal library for a wrapper library");
    libraries.PushBack(internalFragmentLibrary->mZilchLibrary);
  }
}

void ZilchFragment::AttemptGetDefinition(ICodeEditor* editor, size_t cursorPosition, CodeDefinition& definition)
{
  ZilchDocumentResource::AttemptGetDefinition(editor, cursorPosition, definition);
}

//-------------------------------------------------------------------ZilchFragmentLoader
HandleOf<Resource> ZilchFragmentLoader::LoadFromFile(ResourceEntry& entry)
{
  ZilchFragmentManager* manager = ZilchFragmentManager::GetInstance();
  ZilchFragment* fragment = new ZilchFragment();
  fragment->DocumentSetup(entry);
  fragment->mText = ReadFileIntoString(entry.FullPath);
  manager->AddResource(entry, fragment);
  return fragment;
}

HandleOf<Resource> ZilchFragmentLoader::LoadFromBlock(ResourceEntry& entry)
{
  ZilchFragmentManager* manager = ZilchFragmentManager::GetInstance();
  ZilchFragment* fragment = new ZilchFragment();
  fragment->DocumentSetup(entry);
  fragment->mText = String((cstr)entry.Block.Data, entry.Block.Size);
  manager->AddResource(entry, fragment);
  return fragment;
}

void ZilchFragmentLoader::ReloadFromFile(Resource* resource, ResourceEntry& entry)
{
  ((ZilchFragment*)resource)->ReloadData(ReadFileIntoString(entry.FullPath));
}

//-------------------------------------------------------------------ZilchFragmentManager
ImplementResourceManager(ZilchFragmentManager, ZilchFragment);

ZilchFragmentManager::ZilchFragmentManager(BoundType* resourceType)
  : ResourceManager(resourceType)
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

ZilchFragmentManager::~ZilchFragmentManager()
{

}

void ZilchFragmentManager::ValidateName(Status& status, StringParam name)
{
  ZilchDocumentResource::ValidateScriptName(status, name);
  if(status.Failed())
    return;

  // Check all shader types
  GraphicsEngine* graphicsEngine = Z::gEngine->has(GraphicsEngine);
  ZilchShaderLibrary* lastFragmentLibrary = graphicsEngine->mShaderGenerator->GetCurrentInternalProjectLibrary();
  ShaderType* shaderType = lastFragmentLibrary->FindType(name, true);
  if(shaderType != nullptr)
  {
    status.SetFailed(String::Format("Type '%s' is already a fragment type", name.c_str()));
  }
}

String ZilchFragmentManager::GetTemplateSourceFile(ResourceAdd& resourceAdd)
{
  ZilchFragment* fragmentTemplate = Type::DynamicCast<ZilchFragment*, Resource*>(resourceAdd.Template);

  ReturnIf(fragmentTemplate == nullptr, String(), "Invalid resource given to create template.");

  // Get the correct template file name
  String templateFile = BuildString("Template", fragmentTemplate->Name);

  // Replace the fragment name where needed
  Replacements replacements;
  Replacement& nameReplacement = replacements.PushBack();
  nameReplacement.MatchString = "%RESOURCENAME%";
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

void ZilchFragmentManager::DispatchScriptError(StringParam eventId, StringParam shortMessage,
                                               StringParam fullMessage, const Zilch::CodeLocation& location)
{
  // This should only happen when a composite has a zilch error. Figure out how to report later?
  if(location.CodeUserData == nullptr)
    return;

  ZilchDocumentResource* resource = (ZilchDocumentResource*)location.CodeUserData;

  DebugEngineEvent e;
  e.Handled = false;
  e.Script = resource;
  e.Message = shortMessage;
  e.Location = location;
  Z::gResources->DispatchEvent(eventId, &e);

  Console::Print(Filter::DefaultFilter, "%s", fullMessage.c_str());
}

}//namespace Zero
