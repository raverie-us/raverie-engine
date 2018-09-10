///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------- Helpers

void SetStateTimeout(ExecutableState* state)
{
#if ZeroDebug
  state->SetTimeout(300);
#else
  state->SetTimeout(5);
#endif
}

void ZeroZilchExceptionCallback(ExceptionEvent* e)
{
  // Get the first non-native stack for debugging
  Exception* exception = e->ThrownException;
  CodeLocation location = exception->Trace.GetMostRecentNonNativeLocation();

  String shortMessage = exception->Message;
  String fullMessage = exception->GetFormattedMessage(MessageFormat::Python);
  ZilchScriptManager::DispatchScriptError(Events::UnhandledException, shortMessage, fullMessage, location);
}

void ZeroZilchFatalErrorCallback(FatalErrorEvent* e)
{
  if(e->ErrorCode == FatalError::OutOfMemory)
    FatalEngineError("Zilch Fatal Error: Out of Memory");
  else if(e->ErrorCode == FatalError::StackReserveOverflow)
    FatalEngineError("Zilch Fatal Error: Stack Reserve Overflow");
  else
    FatalEngineError("Zilch Fatal Error: Error Code '%d'", (int)e->ErrorCode);
}

// Called when an error occurs in compilation
void ZeroZilchErrorCallback(Zilch::ErrorEvent* e)
{
  // If plugins are currently compiling, let the user know that the error *might* be because of that
  ZilchPluginSourceManager* manager = ZilchPluginSourceManager::GetInstance();
  if(manager->IsCompilingPlugins())
  {
    e->ExactError = BuildString(e->ExactError,
      "\nThis may be because we're currently compiling Zilch"
      " plugins (once finished, scripts will recompile)");
  }

  String shortMessage = e->ExactError;
  String fullMessage = e->GetFormattedMessage(MessageFormat::Python);

  ZilchScriptManager::DispatchScriptError(Events::SyntaxError, shortMessage, fullMessage, e->Location);
}

void OnDebuggerPauseUpdate(DebuggerEvent* event)
{
  Event toSend;
  Z::gEngine->DispatchEvent(Events::DebuggerPauseUpdate, &toSend);

  // We assume the graphical rendering will not change the state of the program, so its safe to do during a breakpoint
  // This also generally draws some sort of 'debugging' overlay
  //Z::gGraphics->PerformRenderTasks(0.0f);
}

void OnDebuggerPause(DebuggerEvent* event)
{
  Event toSend;
  Z::gEngine->DispatchEvent(Events::DebuggerPause, &toSend);
}

void OnDebuggerResume(DebuggerEvent* event)
{
  Event toSend;
  Z::gEngine->DispatchEvent(Events::DebuggerResume, &toSend);
}

//----------------------------------------------------------------------------- ZilchScript Resource
//**************************************************************************************************
ZilchDefineType(ZilchScript, builder, type)
{
  ZeroBindDocumented();
}

//**************************************************************************************************
void ZilchScript::ReloadData(StringRange data)
{
  ZilchDocumentResource::ReloadData(data);

  mResourceLibrary->ScriptsModified();
}

//**************************************************************************************************
void ZilchScript::GetKeywords(Array<Completion>& keywordsOut)
{
  ZilchBase::GetKeywords(keywordsOut);

  ZilchScriptManager* manager = ZilchScriptManager::GetInstance();
  keywordsOut.Append(Grammar::GetUsedKeywords().All());
  keywordsOut.Append(Grammar::GetSpecialKeywords().All());

  AttributeExtensions* attributeExtensions = AttributeExtensions::GetInstance();
  keywordsOut.Append(attributeExtensions->mClassExtensions.Keys());
  keywordsOut.Append(attributeExtensions->mPropertyExtensions.Keys());
  keywordsOut.Append(attributeExtensions->mFunctionExtensions.Keys());
}

//**************************************************************************************************
void ZilchScript::GetLibraries(Array<LibraryRef>& libraries)
{
  MetaDatabase* metaDatabase = MetaDatabase::GetInstance();
  libraries.Insert(libraries.End(), metaDatabase->mNativeLibraries.All());

  // Add the core library so we get auto-completion on things like Console
  Zilch::Core& core = Core::GetInstance();
  libraries.Append(core.GetLibrary());

  GetLibrariesRecursive(libraries, mResourceLibrary);
}

//**************************************************************************************************
// @TrevorS: Isn't this the same logic as AddDependencies on ResourceLibrary/ZilchManager?
void ZilchScript::GetLibrariesRecursive(Array<LibraryRef>& libraries, ResourceLibrary* library)
{
  forRange(ResourceLibrary* dependency, library->Dependencies.All())
    GetLibrariesRecursive(libraries, dependency);

  forRange(SwapLibrary& swapPlugin, library->mSwapPlugins.Values())
  {
    if (swapPlugin.mCurrentLibrary != nullptr)
      libraries.PushBack(swapPlugin.mCurrentLibrary);
  }

  if (library->mSwapScript.mCurrentLibrary != nullptr)
    libraries.PushBack(library->mSwapScript.mCurrentLibrary);
  if (library->mSwapFragment.mCurrentLibrary != nullptr)
    libraries.PushBack(library->mSwapFragment.mCurrentLibrary);
}

//------------------------------------------------------------- ZilchScriptLoader
HandleOf<Resource> ZilchScriptLoader::LoadFromFile(ResourceEntry& entry)
{
  ZilchScript* script = new ZilchScript();
  script->DocumentSetup(entry);
  ZilchScriptManager::GetInstance()->AddResource(entry, script);
  script->mText = ReadFileIntoString(entry.FullPath);
  return script;
}

HandleOf<Resource> ZilchScriptLoader::LoadFromBlock(ResourceEntry& entry)
{
  ZilchScript* script = new ZilchScript();
  script->DocumentSetup(entry);
  ZilchScriptManager::GetInstance()->AddResource(entry, script);
  script->mText = String((cstr)entry.Block.Data, entry.Block.Size);
  return script;
}

void ZilchScriptLoader::ReloadFromFile(Resource* resource, ResourceEntry& entry)
{
  ((ZilchScript*)resource)->ReloadData(ReadFileIntoString(entry.FullPath));
}

//------------------------------------------------------------- ZilchScriptManager
ImplementResourceManager(ZilchScriptManager, ZilchScript);

ZilchScriptManager::ZilchScriptManager(BoundType* resourceType)
  : ResourceManager(resourceType),
    mLastExceptionVersion(-1)
{
  mCategory = "Code";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("All Zilch Scripts", "*.zilchscript;*.z"));
  mOpenFileFilters.PushBack(FileDialogFilter("*.zilchscript"));
  mOpenFileFilters.PushBack(FileDialogFilter("*.z"));
  // We want ZilchScript to be the first thing that shows up in the "Code" category in the add window
  mAddSortWeight = 0;
  mNoFallbackNeeded = true;
  mCanCreateNew = true;
  mSearchable = true;
  mExtension = FileExtensionManager::GetZilchScriptTypeEntry()->GetDefaultExtensionNoDot();
  mCanReload = true;

  AddLoader("ZilchScript", new ZilchScriptLoader());

  //listen for when we should compile
  Zilch::EventConnect(ExecutableState::CallingState, Zilch::Events::UnhandledException, ZeroZilchExceptionCallback);
  Zilch::EventConnect(ExecutableState::CallingState, Zilch::Events::FatalError, ZeroZilchFatalErrorCallback);

  ConnectThisTo(Z::gResources, Events::ResourceLibraryConstructed, OnResourceLibraryConstructed);
}

void ZilchScriptManager::ValidateNewName(Status& status, StringParam name, BoundType* optionalType)
{
  ZilchDocumentResource::ValidateNewScriptName(status, name);
}

void ZilchScriptManager::ValidateRawName(Status& status, StringParam name, BoundType* optionalType)
{
  if (!optionalType || optionalType->IsA(ZilchTypeId(Component)))
  {
    // Because we do component access off of Cogs using the . operator, then it might
    // conflict with an actual member of Cog (name a component 'Destroy', what is Owner.Destroy?)
    // We must do this for Space and GameSession also (technically GameSession and Space doubly hit Cog, but that's fine).
    bool hasMember =
      ZilchTypeId(Cog)->GetMember(name) ||
      ZilchTypeId(GameSession)->GetMember(name) ||
      ZilchTypeId(Space)->GetMember(name) ||
      ZilchTypeId(CogPath)->GetMember(name);

    if (hasMember)
    {
      String message = String::Format(
        "Components cannot have the same name as a property/method on Cog/Space/GameSession (this.Owner.%s would conflict)",
        name.c_str());
      status.SetFailed(message);
      return;
    }
  }

  ZilchDocumentResource::ValidateRawScriptName(status, name);
}

String ZilchScriptManager::GetTemplateSourceFile(ResourceAdd& resourceAdd)
{
  ZilchScript* scriptTemplate = Type::DynamicCast<ZilchScript*, Resource*>(resourceAdd.Template);

  ReturnIf(scriptTemplate == nullptr, String(), "Invalid resource given to create template.");

  String templateFile = BuildString("TemplateZilch", scriptTemplate->Name);

  Replacements replacements;

  // Replace the component name
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

  String fileData = Replace(replacements, scriptTemplate->mText);

  // Get template data off of resource
  String sourceFile = FilePath::Combine(GetTemporaryDirectory(), resourceAdd.FileName);
  WriteStringRangeToFile(sourceFile, fileData);
  return sourceFile;
}

void ZilchScriptManager::OnResourceLibraryConstructed(ObjectEvent* e)
{
  ResourceLibrary* library = (ResourceLibrary*)e->Source;
  EventConnect(&library->mScriptProject, Zilch::Events::CompilationError, ZeroZilchErrorCallback);
}

void ZilchScriptManager::DispatchScriptError(StringParam eventId, StringParam shortMessage, StringParam fullMessage, const CodeLocation& location)
{
  ZilchScriptManager* instance = ZilchScriptManager::GetInstance();
  Resource* resource = (Resource*)location.CodeUserData;

  if (instance->mLastExceptionVersion != ZilchManager::GetInstance()->mVersion)
  {
    instance->mLastExceptionVersion = ZilchManager::GetInstance()->mVersion;
    instance->mDuplicateExceptions.Clear();
  }

  bool isDuplicate = instance->mDuplicateExceptions.Contains(fullMessage);
  instance->mDuplicateExceptions.Insert(fullMessage);

  if (!isDuplicate)
  {
    DebugEngineEvent e;
    e.Handled = false;
    e.Script = Type::DynamicCast<DocumentResource*>(resource);
    e.Message = shortMessage;
    e.Location = location;
    Z::gResources->DispatchEvent(eventId, &e);
  }

  Console::Print(Filter::DefaultFilter, "%s", fullMessage.c_str());
}

void ZilchScriptManager::DispatchZeroZilchError(const CodeLocation& location, StringParam message, Project* buildingProject)
{
  String shortMessage = BuildString("Zero Error: ", message);
  String fullMessage = location.GetFormattedStringWithMessage(MessageFormat::Python, shortMessage);
  buildingProject->Raise(location, ErrorCode::GenericError, message.c_str());
}

void ZilchScriptManager::OnMemoryLeak(MemoryLeakEvent* event)
{
  static const String UnknownType("<UnkownType>");
  static const String NullDump("null");

  String typeName = UnknownType;
  String dump = NullDump;
  bool isTypeNative = true;
  Handle* leakedObject = event->LeakedObject;
  if(leakedObject != nullptr)
  {
    BoundType* type = leakedObject->StoredType;
    typeName = type->ToString();

    StringBuilderExtended builder;
    Zilch::Console::DumpValue(builder, type, (const byte*)leakedObject, 5, 0);
    dump = builder.ToString();

    isTypeNative = type->IsTypeOrBaseNative();
  }

  String message = String::Format(
    "* A memory leak was detected with the type %s. Make sure to avoid cycles "
    "of references, or explicitly invoke delete (typically within a destructor).\n* Memory Dump:\n%s",
    typeName.c_str(), dump.c_str());

  WarnIf(isTypeNative, "%s", message.c_str());
  ZPrint("%s", message.c_str()); 
}

}//namespace Zero
