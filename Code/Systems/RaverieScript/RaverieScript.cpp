// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

void ZeroRaverieExceptionCallback(ExceptionEvent* e)
{
  // Get the first non-native stack for debugging
  Exception* exception = e->ThrownException;
  CodeLocation location = exception->Trace.GetMostRecentNonNativeLocation();

  String shortMessage = exception->Message;
  String fullMessage = exception->GetFormattedMessage(MessageFormat::Python);
  RaverieScriptManager::DispatchScriptError(Events::UnhandledException, shortMessage, fullMessage, location);
}

void ZeroRaverieFatalErrorCallback(FatalErrorEvent* e)
{
  if (e->ErrorCode == FatalError::OutOfMemory)
    FatalEngineError("Raverie Fatal Error: Out of Memory");
  else if (e->ErrorCode == FatalError::StackReserveOverflow)
    FatalEngineError("Raverie Fatal Error: Stack Reserve Overflow");
  else
    FatalEngineError("Raverie Fatal Error: Error Code '%d'", (int)e->ErrorCode);
}

// Called when an error occurs in compilation
void ZeroRaverieErrorCallback(Raverie::ErrorEvent* e)
{
  String shortMessage = e->ExactError;
  String fullMessage = e->GetFormattedMessage(MessageFormat::Python);

  RaverieScriptManager::DispatchScriptError(Events::SyntaxError, shortMessage, fullMessage, e->Location);
}

void OnDebuggerPauseUpdate(DebuggerEvent* event)
{
  Event toSend;
  Z::gEngine->DispatchEvent(Events::DebuggerPauseUpdate, &toSend);

  // We assume the graphical rendering will not change the state of the program,
  // so its safe to do during a breakpoint This also generally draws some sort
  // of 'debugging' overlay
  // Z::gGraphics->PerformRenderTasks(0.0f);
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

// RaverieScript Resource
RaverieDefineType(RaverieScript, builder, type)
{
  RaverieBindDocumented();
}

void RaverieScript::ReloadData(StringRange data)
{
  RaverieDocumentResource::ReloadData(data);

  mResourceLibrary->ScriptsModified();
}

void RaverieScript::GetKeywords(Array<Completion>& keywordsOut)
{
  RaverieBase::GetKeywords(keywordsOut);

  RaverieScriptManager* manager = RaverieScriptManager::GetInstance();
  keywordsOut.Append(Grammar::GetUsedKeywords().All());
  keywordsOut.Append(Grammar::GetSpecialKeywords().All());

  AttributeExtensions* attributeExtensions = AttributeExtensions::GetInstance();
  keywordsOut.Append(attributeExtensions->mClassExtensions.Keys());
  keywordsOut.Append(attributeExtensions->mPropertyExtensions.Keys());
  keywordsOut.Append(attributeExtensions->mFunctionExtensions.Keys());
}

void RaverieScript::GetLibraries(Array<LibraryRef>& libraries)
{
  MetaDatabase* metaDatabase = MetaDatabase::GetInstance();
  libraries.Insert(libraries.End(), metaDatabase->mNativeLibraries.All());

  // Add the core library so we get auto-completion on things like Console
  Raverie::Core& core = Core::GetInstance();
  libraries.Append(core.GetLibrary());

  GetLibrariesRecursive(libraries, mResourceLibrary);
}

// @TrevorS: Isn't this the same logic as AddDependencies on
// ResourceLibrary/RaverieManager?
void RaverieScript::GetLibrariesRecursive(Array<LibraryRef>& libraries, ResourceLibrary* library)
{
  forRange (ResourceLibrary* dependency, library->Dependencies.All())
    GetLibrariesRecursive(libraries, dependency);

  forRange (SwapLibrary& swapPlugin, library->mSwapPlugins.Values())
  {
    if (swapPlugin.mCurrentLibrary != nullptr)
      libraries.PushBack(swapPlugin.mCurrentLibrary);
  }

  if (library->mSwapScript.mCurrentLibrary != nullptr)
    libraries.PushBack(library->mSwapScript.mCurrentLibrary);
  if (library->mSwapFragment.mCurrentLibrary != nullptr)
    libraries.PushBack(library->mSwapFragment.mCurrentLibrary);
}

// RaverieScriptLoader
HandleOf<Resource> RaverieScriptLoader::LoadFromFile(ResourceEntry& entry)
{
  RaverieScript* script = new RaverieScript();
  script->DocumentSetup(entry);
  RaverieScriptManager::GetInstance()->AddResource(entry, script);
  script->mText = ReadFileIntoString(entry.FullPath);
  return script;
}

HandleOf<Resource> RaverieScriptLoader::LoadFromBlock(ResourceEntry& entry)
{
  RaverieScript* script = new RaverieScript();
  script->DocumentSetup(entry);
  RaverieScriptManager::GetInstance()->AddResource(entry, script);
  script->mText = String((cstr)entry.Block.Data, entry.Block.Size);
  return script;
}

void RaverieScriptLoader::ReloadFromFile(Resource* resource, ResourceEntry& entry)
{
  ((RaverieScript*)resource)->ReloadData(ReadFileIntoString(entry.FullPath));
}

// RaverieScriptManager
ImplementResourceManager(RaverieScriptManager, RaverieScript);

RaverieScriptManager::RaverieScriptManager(BoundType* resourceType) :
    ResourceManager(resourceType),
    mLastExceptionVersion(-1)
{
  mCategory = "Code";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("Raverie Scripts", "*.raveriescript"));
  // We want RaverieScript to be the first thing that shows up in the "Code"
  // category in the add window
  mAddSortWeight = 0;
  mNoFallbackNeeded = true;
  mCanCreateNew = true;
  mSearchable = true;
  mExtension = FileExtensionManager::GetRaverieScriptTypeEntry()->GetDefaultExtensionNoDot();
  mCanReload = true;

  AddLoader("RaverieScript", new RaverieScriptLoader());

  // listen for when we should compile
  Raverie::EventConnect(ExecutableState::CallingState, Raverie::Events::PreUnhandledException, ZeroRaverieExceptionCallback);
  Raverie::EventConnect(ExecutableState::CallingState, Raverie::Events::FatalError, ZeroRaverieFatalErrorCallback);

  ConnectThisTo(Z::gResources, Events::ResourceLibraryConstructed, OnResourceLibraryConstructed);
}

void RaverieScriptManager::ValidateNewName(Status& status, StringParam name, BoundType* optionalType)
{
  RaverieDocumentResource::ValidateNewScriptName(status, name);
}

void RaverieScriptManager::ValidateRawName(Status& status, StringParam name, BoundType* optionalType)
{
  if (!optionalType || optionalType->IsA(RaverieTypeId(Component)))
  {
    // Because we do component access off of Cogs using the . operator, then it
    // might conflict with an actual member of Cog (name a component 'Destroy',
    // what is Owner.Destroy?) We must do this for Space and GameSession also
    // (technically GameSession and Space doubly hit Cog, but that's fine).
    bool hasMember = RaverieTypeId(Cog)->GetMember(name) || RaverieTypeId(GameSession)->GetMember(name) ||
                     RaverieTypeId(Space)->GetMember(name) || RaverieTypeId(CogPath)->GetMember(name);

    if (hasMember)
    {
      String message = String::Format("Components cannot have the same name as a property/method on "
                                      "Cog/Space/GameSession (this.Owner.%s would conflict)",
                                      name.c_str());
      status.SetFailed(message);
      return;
    }
  }

  RaverieDocumentResource::ValidateRawScriptName(status, name);
}

String RaverieScriptManager::GetTemplateSourceFile(ResourceAdd& resourceAdd)
{
  RaverieScript* scriptTemplate = Type::DynamicCast<RaverieScript*, Resource*>(resourceAdd.Template);

  ReturnIf(scriptTemplate == nullptr, String(), "Invalid resource given to create template.");

  String templateFile = BuildString("TemplateRaverie", scriptTemplate->Name);

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
  if (config && config->TabWidth == TabWidth::TwoSpaces)
    tabReplacement.ReplaceString = "  ";

  String fileData = Replace(replacements, scriptTemplate->mText);

  // Get template data off of resource
  String sourceFile = FilePath::Combine(GetTemporaryDirectory(), resourceAdd.FileName);
  WriteStringRangeToFile(sourceFile, fileData);
  return sourceFile;
}

void RaverieScriptManager::OnResourceLibraryConstructed(ObjectEvent* e)
{
  ResourceLibrary* library = (ResourceLibrary*)e->Source;
  EventConnect(&library->mScriptProject, Raverie::Events::CompilationError, ZeroRaverieErrorCallback);
}

void RaverieScriptManager::DispatchScriptError(StringParam eventId,
                                             StringParam shortMessage,
                                             StringParam fullMessage,
                                             const CodeLocation& location)
{
  RaverieScriptManager* instance = RaverieScriptManager::GetInstance();
  Resource* resource = (Resource*)location.CodeUserData;

  if (instance->mLastExceptionVersion != RaverieManager::GetInstance()->mVersion)
  {
    instance->mLastExceptionVersion = RaverieManager::GetInstance()->mVersion;
    instance->mDuplicateExceptions.Clear();
  }

  bool isDuplicate = instance->mDuplicateExceptions.Contains(fullMessage);
  instance->mDuplicateExceptions.Insert(fullMessage);

  if (!isDuplicate)
  {
    ScriptEvent e;
    e.Script = Type::DynamicCast<DocumentResource*>(resource);
    e.Message = shortMessage;
    e.Location = location;
    Z::gResources->DispatchEvent(eventId, &e);
  }

  Console::Print(Filter::DefaultFilter, "%s", fullMessage.c_str());
}

void RaverieScriptManager::DispatchZeroRaverieError(const CodeLocation& location,
                                                StringParam message,
                                                Project* buildingProject)
{
  String shortMessage = BuildString("Error: ", message);
  String fullMessage = location.GetFormattedStringWithMessage(MessageFormat::Python, shortMessage);
  buildingProject->Raise(location, ErrorCode::GenericError, message.c_str());
}

void RaverieScriptManager::OnMemoryLeak(MemoryLeakEvent* event)
{
  static const String UnknownType("<UnkownType>");
  static const String NullDump("null");

  String typeName = UnknownType;
  String dump = NullDump;
  bool isTypeNative = true;
  Handle* leakedObject = event->LeakedObject;
  if (leakedObject != nullptr)
  {
    BoundType* type = leakedObject->StoredType;
    typeName = type->ToString();

    StringBuilderExtended builder;
    ScriptConsole::DumpValue(builder, type, (const byte*)leakedObject, 5, 0);
    dump = builder.ToString();

    isTypeNative = type->IsTypeOrBaseNative();
  }

  String message = String::Format("* A memory leak was detected with the type %s. Make sure "
                                  "to avoid cycles "
                                  "of references, or explicitly invoke delete (typically "
                                  "within a destructor).\n* Memory Dump:\n%s",
                                  typeName.c_str(),
                                  dump.c_str());

  WarnIf(isTypeNative, "%s", message.c_str());
  ZPrint("%s", message.c_str());
}

} // namespace Raverie
