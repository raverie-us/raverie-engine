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

  keywordsOut.Append(manager->mAllowedClassAttributes.All());
  keywordsOut.Append(manager->mAllowedFunctionAttributes.All());
  keywordsOut.Append(manager->mAllowedGetSetAttributes.All());
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
void ZilchScript::GetLibrariesRecursive(Array<LibraryRef>& libraries, ResourceLibrary* library)
{
  forRange(ResourceLibrary* dependency, library->Dependencies.All())
    GetLibrariesRecursive(libraries, dependency);

  if (library->mCurrentScriptLibrary != nullptr)
    libraries.PushBack(library->mCurrentScriptLibrary);
  if (library->mCurrentFragmentLibrary != nullptr)
    libraries.PushBack(library->mCurrentFragmentLibrary);
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
  : ResourceManager(resourceType)
{
  //mDebugger.AddProject(&mProject);
  //EventConnect(&mDebugger, Events::DebuggerPause, OnDebuggerPause);
  //EventConnect(&mDebugger, Events::DebuggerResume, OnDebuggerResume);
  //EventConnect(&mDebugger, Events::DebuggerPauseUpdate, OnDebuggerPauseUpdate);

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

  //ConnectThisTo(Z::gResources, Events::ResourcesLoaded, OnResourcesLoaded);
  //listen for when we should compile
  ConnectThisTo(Z::gEngine, Events::EngineUpdate, OnEngineUpdate);
  Zilch::EventConnect(ExecutableState::CallingState, Zilch::Events::UnhandledException, ZeroZilchExceptionCallback);
  Zilch::EventConnect(ExecutableState::CallingState, Zilch::Events::FatalError, ZeroZilchFatalErrorCallback);

  mAllowedClassAttributes.Insert("RunInEditor");
  mAllowedClassAttributes.Insert("Tool");
  mAllowedClassAttributes.Insert("Command");
  mAllowedClassAttributes.Insert(ObjectAttributes::cGizmo);
  mAllowedClassAttributes.Insert(ObjectAttributes::cComponentInterface);
  
  mAllowedFunctionAttributes.Insert("Static");
  mAllowedFunctionAttributes.Insert("Virtual");
  mAllowedFunctionAttributes.Insert("Override");
  mAllowedFunctionAttributes.Insert("Hidden");
  mAllowedFunctionAttributes.Insert("Property");

  mAllowedGetSetAttributes.Insert("Static");
  mAllowedGetSetAttributes.Insert("Virtual");
  mAllowedGetSetAttributes.Insert("Override");
  mAllowedGetSetAttributes.Insert("Hidden");
  mAllowedGetSetAttributes.Insert("Serialized");
  mAllowedGetSetAttributes.Insert("Editable");
  mAllowedGetSetAttributes.Insert("Property");
  mAllowedGetSetAttributes.Insert("Dependency");
  mAllowedGetSetAttributes.Insert("NetProperty");
  mAllowedGetSetAttributes.Insert("NetPeerId");
  mAllowedGetSetAttributes.Insert("RuntimeClone");
  mAllowedGetSetAttributes.Insert("ShaderInput");
  mAllowedGetSetAttributes.Insert("ResourceProperty");
  mAllowedGetSetAttributes.Insert("RenamedFrom");
  mAllowedGetSetAttributes.Insert(PropertyAttributes::cLocalModificationOverride);

  ConnectThisTo(Z::gResources, Events::PreScriptSetCompile, OnPreZilchProjectCompilation);
}

void ZilchScriptManager::ValidateName(Status& status, StringParam name)
{
  ZilchDocumentResource::ValidateScriptName(status, name);
}

String ZilchScriptManager::GetTemplateSourceFile(ResourceAdd& resourceAdd)
{
  ZilchScript* scriptTemplate = Type::DynamicCast<ZilchScript*, Resource*>(resourceAdd.Template);

  ReturnIf(scriptTemplate == nullptr, String(), "Invalid resource given to create template.");

  String templateFile = BuildString("TemplateZilch", scriptTemplate->Name);

  Replacements replacements;

  // Replace the component name
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

  String fileData = Replace(replacements, scriptTemplate->mText);

  // Get template data off of resource
  String sourceFile = FilePath::Combine(GetTemporaryDirectory(), resourceAdd.FileName);
  WriteStringRangeToFile(sourceFile, fileData);
  return sourceFile;
}

void ZilchScriptManager::OnResourcesLoaded(ResourceEvent* event)
{
  // If we don't have scripts or plugins, then don't bother compiling
  // This prevents compiling early (twice) and having missing meta
  bool noScripts = ResourceIdMap.Empty();
  bool noPlugins = ZilchPluginLibraryManager::GetInstance()->ResourceIdMap.Empty();
  if(noScripts && noPlugins)
    return;

  ZilchManager::GetInstance()->Compile();
}

void ZilchScriptManager::OnPreZilchProjectCompilation(ZilchPreCompilationEvent* e)
{
  EventConnect(e->mProject, Zilch::Events::CompilationError, ZeroZilchErrorCallback);
  EventConnect(e->mProject, Zilch::Events::TypeParsed, &ZilchScriptManager::TypeParsedCallback, this);
}

void ZilchScriptManager::OnEngineUpdate(Event* event)
{
  //METAREFACTOR
  //mDebugger.Update();
}

void ZilchScriptManager::TypeParsedCallback(Zilch::ParseEvent* e, void* userData)
{
  LibraryBuilder* builder = e->Builder;
  BoundType* type = e->Type;
  ZilchScriptManager* self = (ZilchScriptManager*)userData;

  ValidateAttributes(type->Attributes, *e->Location, self->mAllowedClassAttributes, "class", e->BuildingProject);
  forRange(Function* zilchFunction, type->AllFunctions.All())
    ValidateAttributes(zilchFunction->Attributes, zilchFunction->Location, self->mAllowedFunctionAttributes, "function", e->BuildingProject);
  forRange(Property* zilchProperty, type->AllProperties.All())
  {
    ValidateAttributes(zilchProperty->Attributes, zilchProperty->Location, self->mAllowedGetSetAttributes, "property", e->BuildingProject);
    CheckDependencies(type, zilchProperty, e->BuildingProject);
  }

  // Check if an object has a gizmo attribute.
  Attribute* gizmoAttribute = type->HasAttributeInherited(ObjectAttributes::cGizmo);
  if(gizmoAttribute)
  {
    // Get the archetype's name if it exists
    Zilch::AttributeParameter* param = gizmoAttribute->HasAttributeParameter("archetype");
    if(param != nullptr)
    {
      MetaEditorGizmo* editorGizmo = new MetaEditorGizmo();
      editorGizmo->mGizmoArchetype = param->StringValue;
      type->Add(editorGizmo);
    }
  }

  // Check for this having a base that has component interface
  Attribute* componentInterfaceAttribute = type->HasAttributeInherited(ObjectAttributes::cComponentInterface);
  if(componentInterfaceAttribute)
  {
    BoundType* baseType = type->BaseType;
    // Keep walking up the hierarchy to find what base type had the attribute. Make
    // sure we skip the base type itself though (i.e. Collider can't add Collider as an interface)
    while(baseType != nullptr && type != baseType)
    {
      if(baseType->HasAttribute(ObjectAttributes::cComponentInterface))
      {
        CogComponentMeta* componentMeta = type->HasOrAdd<CogComponentMeta>(type);
        // The default constructor will set this type to FromDataOnly so we have to manually set this for now
        componentMeta->mSetupMode = SetupMode::DefaultSerialization;
        componentMeta->AddInterface(baseType);
        break;
      }
      
      baseType = baseType->BaseType;
    }
  }
}

void ZilchScriptManager::ValidateAttribute(Attribute& attribute, CodeLocation& location, HashSet<String>& allowedAttributes, StringParam attributeClassification, Project* buildingProject)
{
  if(allowedAttributes.Contains(attribute.Name) == false)
  {
    String msg = String::Format("Attribute '%s' is not valid on a %s\n\n", attribute.Name.c_str(), attributeClassification.c_str());
    DispatchZeroZilchError(location, msg, buildingProject);
  }
}

void ZilchScriptManager::ValidateAttributes(Array<Attribute>& attributes, CodeLocation& location, HashSet<String>& allowedAttributes, StringParam attributeClassification, Project* buildingProject)
{
  for(size_t i = 0; i < attributes.Size(); ++i)
  {
    Attribute& attribute = attributes[i];
    ValidateAttribute(attribute, location, allowedAttributes, attributeClassification, buildingProject);
  }
}

void ZilchScriptManager::CheckDependencies(BoundType* classType, Property* property, Project* buildingProject)
{
  if(property->HasAttribute(PropertyAttributes::cDependency))
  {
    BoundType* componentType = ZilchTypeId(Component);

    // It's only valid for Components to have dependencies
    if(!classType->IsA(componentType))
    {
      String message = "Dependency properties can only be on Component Types";
      DispatchZeroZilchError(property->Location, message, buildingProject);
      return;
    }

    // Make sure the property type is a Component
    BoundType* propertyType = Type::GetBoundType(property->PropertyType);
    if(!propertyType->IsA(componentType))
    {
      // The extra spaces in this message is to force word wrap in our text editor. This is silly, but
      // the word wrap is arbitrarily short. Should be fixed at some point
      String message = "Dependency properties must be of type Component (e.g. Transform)";
      DispatchZeroZilchError(property->Location, message, buildingProject);
      return;
    }

    CogComponentMeta* metaComponent = classType->HasOrAdd<CogComponentMeta>(classType);
    metaComponent->mDependencies.Insert(propertyType);
    metaComponent->mSetupMode = SetupMode::DefaultConstructor;
  }
}

void ZilchScriptManager::DispatchScriptError(StringParam eventId, StringParam shortMessage, StringParam fullMessage, const CodeLocation& location)
{
  Resource* resource = (Resource*)location.CodeUserData;

  DebugEngineEvent e;
  e.Handled = false;
  e.Script = Type::DynamicCast<DocumentResource*>(resource);
  e.Message = shortMessage;
  e.Location = location;
  Z::gResources->DispatchEvent(eventId, &e);

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
