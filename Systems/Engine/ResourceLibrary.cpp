///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys, Trevor Sundberg
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(PreScriptCompile);
DefineEvent(PreScriptSetCompile);
DefineEvent(CompileZilchFragments);
}//namespace Events

//--------------------------------------------------------------------- Zilch Compile Fragment Event
ZilchDefineType(ZilchPreCompilationEvent, builder, type)
{

}

ZilchDefineType(ZilchCompiledEvent, builder, type)
{

}

ZilchDefineType(ZilchCompileFragmentEvent, builder, type)
{

}

//----------------------------------------------------------------------------------- Resource Entry
//**************************************************************************************************
ZilchCompileFragmentEvent::ZilchCompileFragmentEvent(Module& dependencies,
  Array<ZilchDocumentResource*>& fragments, ResourceLibrary* owningLibrary) :
  mDependencies(dependencies),
  mFragments(fragments),
  mOwningLibrary(owningLibrary)
{
}

//**************************************************************************************************
ResourceEntry::ResourceEntry()
{
  mLibrarySource = nullptr;
  LoadOrder = 0;
  mBuilder = nullptr;
  mLibrary = nullptr;
}

//**************************************************************************************************
ResourceEntry::ResourceEntry(uint order, StringParam type, StringParam name,
  StringParam location, ResourceId id, 
  ContentItem* library, BuilderComponent* builder)
  : LoadOrder(order), Type(type), Name(name), Location(location), 
  mResourceId(id), mLibrarySource(library), mBuilder(builder),
  mLibrary(nullptr)
{

}

String ResourceEntry::ToString(bool shortFormat) const
{
  String idString = Zero::ToString(this->mResourceId, shortFormat);
  return String::Format("Resource %s:%s Loader '%s'", Name.c_str(), idString.c_str(), Type.c_str());
}

void ResourceEntry::Serialize(Serializer& stream)
{
  SerializeName(mResourceId);
  SerializeName(Type);
  SerializeName(Name);
  SerializeName(Location);
}

Zero::ResourceTemplate* ResourceEntry::GetResourceTemplate()
{
  if (mLibrarySource)
    return mLibrarySource->has(ResourceTemplate);
  return nullptr;
}

String ResourcePackageToString(const BoundType* type, const byte* value)
{
  ResourcePackage* resourcePackage = (ResourcePackage*)value;
  return String::Format("Resource Package %s", resourcePackage->Name.c_str());
}

ZilchDefineType(ResourcePackageDisplay, builder, type)
{
}

String ResourcePackageDisplay::GetName(HandleParam object)
{
  ResourcePackage* resourcePackage = object.Get<ResourcePackage*>(GetOptions::AssertOnNull);
  return String::Format("Resource Package %s", resourcePackage->Name.c_str());
}

String ResourcePackageDisplay::GetDebugText(HandleParam object)
{
  return GetName(object);
}

ZilchDefineType(ResourcePackage, builder, type)
{
  type->ToStringFunction = ResourcePackageToString;
  type->Add(new ResourcePackageDisplay());
  type->HandleManager = ZilchManagerId(PointerManager);
}

ResourcePackage::ResourcePackage()
{
}

void ResourcePackage::Save(StringParam filename)
{
  SaveToDataFile(*this, filename);
}

void ResourcePackage::Load(StringParam filename)
{
  LoadFromDataFile(*this, filename);
}

void ResourcePackage::Serialize(Serializer& stream)
{
  SerializeName(Name);
  stream.SerializeField("Resources", Resources);
}

//--------------------------------------------------------------------------------- Resource Library
BoundType* ResourceLibrary::sScriptType = nullptr;
BoundType* ResourceLibrary::sFragmentType = nullptr;
bool ResourceLibrary::sLibraryUnloading = false;

//**************************************************************************************************
ZilchDefineType(ResourceLibrary, builder, type)
{
}

//**************************************************************************************************
ResourceLibrary::ResourceLibrary() :
  mScriptCompileStatus(ZilchCompileStatus::Modified),
  mFragmentCompileStatus(ZilchCompileStatus::Modified)
{
  Resources.Reserve(256);
}

//**************************************************************************************************
void ResourceLibrary::Add(Resource* resource, bool isNew)
{
  resource->mResourceLibrary = this;
  Resources.PushBack(resource);

  bool codeModified = false;

  // Filter zilch resources into their appropriate containers
  ErrorIf(sScriptType == nullptr || sFragmentType == nullptr, "Script and Fragment types must be set");
  BoundType* resourceType = ZilchVirtualTypeId(resource);
  if(resourceType->IsA(sScriptType))
  {
    mScripts.PushBack((ZilchDocumentResource*)resource);
    ScriptsModified();
    codeModified = true;
  }
  else if(resourceType->IsA(sFragmentType))
  {
    mFragments.PushBack((ZilchDocumentResource*)resource);
    FragmentsModified();
    codeModified = true;
  }
  else if(resourceType->IsA(ZilchTypeId(ZilchLibraryResource)))
  {
    mPlugins.PushBack((ZilchLibraryResource*)resource);
    PluginsModified();
    codeModified = true;
  }

  // If this is a newly added resource and it's one
  // of the special ones that will invoke a recompile
  if (isNew && codeModified)
    ZilchManager::GetInstance()->Compile();
  
  // Resource was added
  ResourceEvent e;
  e.EventResource = resource;
  e.EventResourceLibrary = this;
  e.Manager = resource->GetManager();
  DispatchEvent(Events::ResourceAdded, &e);
}

//**************************************************************************************************
void ResourceLibrary::Remove(Resource* resource)
{
  // Store a reference so we can remove from the array, then delete after (just in case the handle
  // in the array was the last reference)
  HandleOf<Resource> resourceHandle = resource;

  Resources.EraseValueError(resourceHandle);

  bool codeModified = false;

  // Remove zilch resources from their containers
  BoundType* resourceType = ZilchVirtualTypeId(resource);
  if(resourceType->IsA(sScriptType))
  {
    mScripts.EraseValue((ZilchDocumentResource*)resource);
    ScriptsModified();
    codeModified = true;
  }
  else if(resourceType->IsA(sFragmentType))
  {
    mFragments.EraseValue((ZilchDocumentResource*)resource);
    FragmentsModified();
    codeModified = true;
  }
  else if(resourceType->IsA(ZilchTypeId(ZilchLibraryResource)))
  {
    mPlugins.EraseValue((ZilchLibraryResource*)resource);
    PluginsModified();
    codeModified = true;
  }

  // If we're removing a resource that affects compilation,
  // then we need to perform a recompile here
  if (codeModified)
  {
    ZilchManager::GetInstance()->Compile();
  }

  delete resource;
}

//**************************************************************************************************
void ResourceLibrary::AddDependency(ResourceLibrary* dependency)
{
  Dependencies.PushBack(dependency);
  dependency->Dependents.PushBack(this);
}

//**************************************************************************************************
bool ResourceLibrary::CanReference(ResourceLibrary* library)
{
  // Make sure to check ourself before dependencies
  if(library == this)
    return true;

  forRange(ResourceLibrary* dependentLibrary, Dependencies.All())
  {
    if(dependentLibrary == library)
      return true;
  }
  return false;
}

//**************************************************************************************************
void ResourceLibrary::Unload()
{
  ErrorIf(!Dependents.Empty(), "Cannot unload a Resource Library when other libraries depend on us");

  // Call unload on every resource so that references to other resources within the library can be cleared.
  forRange(Resource* resource, Resources.All())
  {
    if (resource != nullptr)
      resource->Unload();
    else
      DoNotifyError("Error", String::Format("A resource owned by library '%s' was incorrectly removed.", Name.c_str()));
  }

  // Validate all reference counts are now 1, otherwise there is likely a leaking handle somewhere.
  forRange(Resource* resource, Resources.All())
  {
    if (resource != nullptr && resource->GetReferenceCount() != 1)
    {
      // Report error and manually delete the resource.
      String resourceType = resource->ZilchGetDerivedType()->Name;
      String error = String::Format("%s resource '%s' is being referenced while unloading library '%s'.", resourceType.c_str(), resource->Name.c_str(), Name.c_str());
      DoNotifyError("Error", error);
      resource->GetManager()->Remove(resource, RemoveMode::Unloading);
      delete resource;
    }
  }

  // Resources use this flag to detect incorrect removal of non-runtime resources.
  // Changing this flag and removing library resources should only ever happen here.
  sLibraryUnloading = true;
  Resources.Clear();
  sLibraryUnloading = false;

  // Remove ourself as dependents on our dependencies
  forRange(ResourceLibrary* dependency, Dependencies.All())
    dependency->Dependents.EraseValue(this);

  // Remove ourself from the resource system
  Z::gResources->LoadedResourceLibraries.Erase(Name);

  MetaDatabase* instance = MetaDatabase::GetInstance();
  if(mCurrentFragmentLibrary != nullptr)
    instance ->RemoveLibrary(mCurrentFragmentLibrary);
  if(mCurrentScriptLibrary != nullptr)
    instance ->RemoveLibrary(mCurrentScriptLibrary);

  ResourceEvent event;
  Z::gResources->DispatchEvent(Events::ResourcesUnloaded, &event);

  delete this;
}

//**************************************************************************************************
void ResourceLibrary::ScriptsModified()
{
  mScriptCompileStatus = ZilchCompileStatus::Modified;

  // All dependents must be recompiled, so mark them as modified
  forRange(ResourceLibrary* dependent, Dependents.All())
    dependent->ScriptsModified();
}

//**************************************************************************************************
void ResourceLibrary::FragmentsModified()
{
  mScriptCompileStatus = ZilchCompileStatus::Modified;
  mFragmentCompileStatus = ZilchCompileStatus::Modified;

  // All dependents must be recompiled, so mark them as modified
  forRange(ResourceLibrary* dependent, Dependents.All())
    dependent->FragmentsModified();
}

//**************************************************************************************************
void ResourceLibrary::PluginsModified()
{
  // Plugins are always fully compiled so they don't need to have a 'modified' state
  // However, since we decided that scripts in the same resource library (and in dependent libraries)
  // are always dependent upon the plugins, then we must mark all scripts as being modified
  ScriptsModified();
}

//**************************************************************************************************
bool AddDependencies(Module& module, ResourceLibrary* library,
                     HashSet<ResourceLibrary*>& modifiedLibraries, bool onlyFragments)
{
  forRange(ResourceLibrary* dependency, library->Dependencies)
  {
    // Blindly call compile and force all our dependencies to be compiled
    // (will return instantly if it's already compiled!)
    // If the library did not compile, we cannot possibly compile ourselves
    if(dependency->CompileScripts(modifiedLibraries) == false)
      return false;

    // Depth first
    if(AddDependencies(module, dependency, modifiedLibraries, onlyFragments) == false)
      return false;

    if(!onlyFragments)
      module.Append(dependency->GetNewestScriptLibrary());
    module.Append(dependency->GetNewestFragmentLibrary());
  }

  return true;
}

//**************************************************************************************************
void ResourceLibrary::OnScriptProjectPreParser(ParseEvent* e)
{
  EngineLibraryExtensions::AddExtensionsPreCompilation(*e->Builder, this);
}

//**************************************************************************************************
void OnScriptProjectPostSyntaxer(ParseEvent* e)
{
  EngineLibraryExtensions::AddExtensionsPostCompilation(*e->Builder);
}

//**************************************************************************************************
bool ResourceLibrary::CompileScripts(HashSet<ResourceLibrary*>& modifiedLibraries)
{
  // If we already compiled, then we know that all dependent libraries must have been good
  // This means that if we ever referenced 
  if(mScriptCompileStatus == ZilchCompileStatus::Compiled)
    return true;

  // Scripts cannot compile if fragments do not compile
  if(CompileFragments(modifiedLibraries) == false)
    return false;

  Module dependencies;
  
  // Remove the core library from dependencies as we're going to add it from native libraries
  dependencies.Clear();

  // Add all native libraries
  forRange(LibraryRef library, MetaDatabase::GetInstance()->mNativeLibraries)
    dependencies.Append(library);

  // Add all resource library dependencies
  AddDependencies(dependencies, this, modifiedLibraries, false);

  // Add our fragment library (it may be null if we didn't have any fragments)
  if(LibraryRef newestFragment = GetNewestFragmentLibrary())
    dependencies.Append(newestFragment);

  // By this point, we've already compiled all our dependencies
  ZPrint("  Compiling %s Scripts\n", this->Name.c_str());

  Project project;

  // When the project is compiled, we want to add extensions to it
  EventConnect(&project, Zilch::Events::PreParser, &ResourceLibrary::OnScriptProjectPreParser, this);
  EventConnect(&project, Zilch::Events::PostSyntaxer, &OnScriptProjectPostSyntaxer);
  EventConnect(&project, Zilch::Events::TypeParsed, &EngineLibraryExtensions::TypeParsedCallback);

  // Dispatch an event allowing specific resource managers to do whatever they want with the project
  // This was added so that ZilchPlugins can listen for compilation errors 
  ZilchPreCompilationEvent e;
  e.mProject = &project;
  Z::gResources->DispatchEvent(Events::PreScriptSetCompile, &e);

  // Add all plugins
  forRange(ZilchLibraryResource* pluginLibrary, mPlugins)
    pluginLibrary->AddToProject(project);

  // Add all scripts
  forRange(ZilchDocumentResource* script, mScripts)
  {
    // Templates shouldn't be compiled. They contain invalid strings (%RESOURCENAME%) that are
    // replaced when a new resource is created from the template
    if(script->GetResourceTemplate() == nullptr)
      project.AddCodeFromString(script->mText, script->GetNameOrFilePath(), script);
  }

  mPendingScriptLibrary = project.Compile(this->Name, dependencies, EvaluationMode::Project);

  if(mPendingScriptLibrary != nullptr)
  {
    modifiedLibraries.Insert(this);
    ZilchManager::GetInstance()->mPendingScriptProjectLibrary = mPendingScriptLibrary;
    mScriptCompileStatus = ZilchCompileStatus::Compiled;
    return true;
  }

  return false;
}

//**************************************************************************************************
bool ResourceLibrary::CompileFragments(HashSet<ResourceLibrary*>& modifiedLibraries)
{
  // If we already compiled, then we know that all dependent libraries must have been good
  if(mFragmentCompileStatus == ZilchCompileStatus::Compiled)
    return true;

  Module dependencies;

  // Remove the default core library
  dependencies.Clear();

  AddDependencies(dependencies, this, modifiedLibraries, true);

  // By this point, we've already compiled all our dependencies
  ZPrint("  Compiling %s Fragments\n", this->Name.c_str());

  ZilchCompileFragmentEvent e(dependencies, mFragments, this);
  ZilchManager::GetInstance()->DispatchEvent(Events::CompileZilchFragments, &e);
  mPendingFragmentLibrary = e.mReturnedLibrary;

  if(mPendingFragmentLibrary != nullptr)
  {
    modifiedLibraries.Insert(this);
    ZilchManager::GetInstance()->mPendingFragmentProjectLibrary = mPendingFragmentLibrary;
    mFragmentCompileStatus = ZilchCompileStatus::Compiled;
    return true;
  }

  return false;
}

//**************************************************************************************************
void ResourceLibrary::Commit()
{
  MetaDatabase* metaDatabase = MetaDatabase::GetInstance();

  ErrorIf(mPendingScriptLibrary == nullptr && mPendingFragmentLibrary == nullptr,
    "Commit should only be called if we had a pending library");

  ErrorIf(mScriptCompileStatus != ZilchCompileStatus::Compiled ||
    mFragmentCompileStatus != ZilchCompileStatus::Compiled,
    "Commit should only be called when the library was fully compiled (should have a pending above)");

  // Replace the script library
  if(mPendingScriptLibrary)
  {
    // This can be null if it's the first compilation
    if(mCurrentScriptLibrary != nullptr)
      metaDatabase->RemoveLibrary(mCurrentScriptLibrary);

    metaDatabase->AddLibrary(mPendingScriptLibrary);
    mCurrentScriptLibrary = mPendingScriptLibrary;
    mPendingScriptLibrary = nullptr;
  }

  // Replace the fragment library
  if(mPendingFragmentLibrary)
  {
    // This can be null if it's the first compilation
    if(mCurrentFragmentLibrary != nullptr)
      metaDatabase->RemoveLibrary(mCurrentFragmentLibrary);

    metaDatabase->AddLibrary(mPendingFragmentLibrary);
    mCurrentFragmentLibrary = mPendingFragmentLibrary;
    mPendingFragmentLibrary = nullptr;
  }
}

//**************************************************************************************************
LibraryRef ResourceLibrary::GetNewestScriptLibrary()
{
  if(mPendingScriptLibrary)
    return mPendingScriptLibrary;
  return mCurrentScriptLibrary;
}

//**************************************************************************************************
bool ResourceLibrary::HasPendingScriptLibrary()
{
  return (mPendingScriptLibrary != nullptr);
}

//**************************************************************************************************
LibraryRef ResourceLibrary::GetNewestFragmentLibrary()
{
  if(mPendingFragmentLibrary)
    return mPendingFragmentLibrary;
  return mCurrentFragmentLibrary;
}

//**************************************************************************************************
bool ResourceLibrary::HasPendingFragmentLibrary()
{
  return (mPendingFragmentLibrary != nullptr);
}


//**************************************************************************************************
const Array<LibraryRef>& ResourceLibrary::GetNewestScriptLibraries()
{
  if(!mPendingScriptLibraries.Empty())
    return mPendingScriptLibraries;
  return mCurrentScriptLibraries;
}

}//namespace Zero
