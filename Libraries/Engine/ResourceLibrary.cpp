// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(PreScriptCompile);
DefineEvent(CompileZilchFragments);
DefineEvent(ResourceLibraryConstructed);
} // namespace Events

ZilchDefineType(ZilchCompiledEvent, builder, type)
{
}

ZilchDefineType(ZilchCompileFragmentEvent, builder, type)
{
}

// Resource Entry
ZilchCompileFragmentEvent::ZilchCompileFragmentEvent(Module& dependencies,
                                                     Array<ZilchDocumentResource*>& fragments,
                                                     ResourceLibrary* owningLibrary) :
    mDependencies(dependencies),
    mFragments(fragments),
    mOwningLibrary(owningLibrary)
{
}

ResourceEntry::ResourceEntry()
{
  mLibrarySource = nullptr;
  LoadOrder = 0;
  mBuilder = nullptr;
  mLibrary = nullptr;
}

ResourceEntry::ResourceEntry(uint order,
                             StringParam type,
                             StringParam name,
                             StringParam location,
                             ResourceId id,
                             ContentItem* library,
                             BuilderComponent* builder) :
    LoadOrder(order),
    Type(type),
    Name(name),
    Location(location),
    mResourceId(id),
    mLibrarySource(library),
    mBuilder(builder),
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

// Compiled Library
SwapLibrary::SwapLibrary() : mCompileStatus(ZilchCompileStatus::Modified)
{
}

LibraryRef SwapLibrary::GetNewestLibrary()
{
  if (mPendingLibrary)
    return mPendingLibrary;
  return mCurrentLibrary;
}

bool SwapLibrary::HasPendingLibrary()
{
  return (mPendingLibrary != nullptr);
}

void SwapLibrary::Commit()
{
  if (mPendingLibrary)
  {
    ErrorIf(mCompileStatus != ZilchCompileStatus::Compiled,
            "When committing and we have a pending library the compile status "
            "should have already been set to Compiled");

    // Verify mCurrentLibrary was unloaded.
    ErrorIf(mCurrentLibrary != nullptr,
            "The current library must be unloaded before committing a pending "
            "library.");

    MetaDatabase::GetInstance()->AddLibrary(mPendingLibrary, true);
    mCurrentLibrary = mPendingLibrary;
    mPendingLibrary = nullptr;

    if (mCurrentLibrary->Plugin)
      mCurrentLibrary->Plugin->InitializeSafe();
  }
}

void SwapLibrary::Unload()
{
  // This can be null if it's the first compilation
  if (mCurrentLibrary != nullptr)
  {
    MetaDatabase::GetInstance()->RemoveLibrary(mCurrentLibrary);
    if (mCurrentLibrary->Plugin)
      mCurrentLibrary->Plugin->UninitializeSafe();
  }

  mCurrentLibrary = nullptr;
}

// Resource Library
BoundType* ResourceLibrary::sScriptType = nullptr;
BoundType* ResourceLibrary::sFragmentType = nullptr;
bool ResourceLibrary::sLibraryUnloading = false;

ZilchDefineType(ResourceLibrary, builder, type)
{
}

ResourceLibrary::ResourceLibrary()
{
  Resources.Reserve(256);

  // When the project is compiled, we want to add extensions to it
  EventConnect(&mScriptProject, Zilch::Events::PreParser, &ResourceLibrary::OnScriptProjectPreParser, this);
  EventConnect(&mScriptProject, Zilch::Events::PostSyntaxer, &ResourceLibrary::OnScriptProjectPostSyntaxer, this);
  EventConnect(&mScriptProject, Zilch::Events::TypeParsed, &EngineLibraryExtensions::TypeParsedCallback);

  ObjectEvent toSend(this);
  Z::gResources->DispatchEvent(Events::ResourceLibraryConstructed, &toSend);
}

ResourceLibrary::~ResourceLibrary()
{
}

void ResourceLibrary::Add(Resource* resource, bool isNew)
{
  resource->mResourceLibrary = this;
  Resources.PushBack(resource);

  // Filter zilch resources into their appropriate containers
  ErrorIf(sScriptType == nullptr || sFragmentType == nullptr, "Script and Fragment types must be set");
  BoundType* resourceType = ZilchVirtualTypeId(resource);
  if (resourceType->IsA(sScriptType))
  {
    mScripts.PushBack((ZilchDocumentResource*)resource);
    ScriptsModified();
  }
  else if (resourceType->IsA(sFragmentType))
  {
    mFragments.PushBack((ZilchDocumentResource*)resource);
    FragmentsModified();
  }
  else if (resourceType->IsA(ZilchTypeId(ZilchLibraryResource)))
  {
    mPlugins.PushBack((ZilchLibraryResource*)resource);
    PluginsModified();
  }

  // Resource was added
  ResourceEvent e;
  e.EventResource = resource;
  e.EventResourceLibrary = this;
  e.Manager = resource->GetManager();
  DispatchEvent(Events::ResourceAdded, &e);
}

void ResourceLibrary::Remove(Resource* resource)
{
  // Store a reference so we can remove from the array, then delete after (just
  // in case the handle in the array was the last reference)
  HandleOf<Resource> resourceHandle = resource;

  // Allow resources to do any required cleanup before being removed, same as
  // when a library is unloaded.
  resource->Unload();

  Resources.EraseValueError(resourceHandle);

  // Remove zilch resources from their containers
  BoundType* resourceType = ZilchVirtualTypeId(resource);
  if (resourceType->IsA(sScriptType))
  {
    mScripts.EraseValue((ZilchDocumentResource*)resource);
    ScriptsModified();
  }
  else if (resourceType->IsA(sFragmentType))
  {
    mFragments.EraseValue((ZilchDocumentResource*)resource);
    FragmentsModified();
  }
  else if (resourceType->IsA(ZilchTypeId(ZilchLibraryResource)))
  {
    ZilchLibraryResource* libraryResource = (ZilchLibraryResource*)resource;
    mPlugins.EraseValue(libraryResource);
    mSwapPlugins[libraryResource].Unload();
    mSwapPlugins.Erase(libraryResource);
    PluginsModified();
  }

  delete resource;
}

void ResourceLibrary::AddDependency(ResourceLibrary* dependency)
{
  Dependencies.PushBack(dependency);
  dependency->Dependents.PushBack(this);
}

bool ResourceLibrary::BuiltType(BoundType* type)
{
  LibraryRef sourceLib = type->SourceLibrary;

  if (mSwapScript.mCurrentLibrary == sourceLib)
    return true;

  if (mSwapFragment.mCurrentLibrary == sourceLib)
    return true;

  forRange (SwapLibrary& swapPlugin, mSwapPlugins.Values())
  {
    if (swapPlugin.mCurrentLibrary == sourceLib)
      return true;
  }

  return false;
}

BoundType* GetReplacingTypeFromSwapLibrary(SwapLibrary& swapLibrary, BoundType* oldType)
{
  LibraryRef sourceLib = oldType->SourceLibrary;

  if (swapLibrary.mCurrentLibrary == sourceLib)
  {
    if (Library* pendingLibrary = swapLibrary.mPendingLibrary)
      return pendingLibrary->BoundTypes.FindValue(oldType->Name, nullptr);
  }

  return nullptr;
}

BoundType* ResourceLibrary::GetReplacingType(BoundType* oldType)
{
  if (BoundType* newType = GetReplacingTypeFromSwapLibrary(mSwapScript, oldType))
    return newType;

  if (BoundType* newType = GetReplacingTypeFromSwapLibrary(mSwapFragment, oldType))
    return newType;

  forRange (SwapLibrary& swapPlugin, mSwapPlugins.Values())
  {
    if (BoundType* newType = GetReplacingTypeFromSwapLibrary(swapPlugin, oldType))
      return newType;
  }

  return nullptr;
}

bool ResourceLibrary::CanReference(ResourceLibrary* library)
{
  // Make sure to check ourself before dependencies
  if (library == this)
    return true;

  forRange (ResourceLibrary* dependentLibrary, Dependencies.All())
  {
    if (dependentLibrary == library)
      return true;
  }
  return false;
}

void ResourceLibrary::Unload()
{
  ErrorIf(!Dependents.Empty(), "Cannot unload a Resource Library when other libraries depend on us");

  // Call unload on every resource so that references to other resources within
  // the library can be cleared.
  forRange (Resource* resource, Resources.All())
  {
    if (resource != nullptr)
      resource->Unload();
    else
      DoNotifyError("Error", String::Format("A resource owned by library '%s' was incorrectly removed.", Name.c_str()));
  }

  // Validate all reference counts are now 1, otherwise there is likely a
  // leaking handle somewhere.
  forRange (Resource* resource, Resources.All())
  {
    if (resource != nullptr && resource->GetReferenceCount() != 1)
    {
      // Report error and manually delete the resource.
      String resourceType = resource->ZilchGetDerivedType()->Name;
      String error = String::Format("%s resource '%s' is being referenced while unloading library '%s'.",
                                    resourceType.c_str(),
                                    resource->Name.c_str(),
                                    Name.c_str());
      DoNotifyError("Error", error);
      resource->GetManager()->Remove(resource, RemoveMode::Unloading);
      delete resource;
    }
  }

  // Resources use this flag to detect incorrect removal of non-runtime
  // resources. Changing this flag and removing library resources should only
  // ever happen here.
  sLibraryUnloading = true;
  Resources.Clear();
  sLibraryUnloading = false;

  // Remove ourself as dependents on our dependencies
  forRange (ResourceLibrary* dependency, Dependencies.All())
    dependency->Dependents.EraseValue(this);

  // Remove ourself from the resource system
  Z::gResources->LoadedResourceLibraries.Erase(Name);

  mSwapFragment.Unload();
  mSwapScript.Unload();

  forRange (SwapLibrary& swapPlugin, mSwapPlugins.Values())
  {
    swapPlugin.Unload();
  }

  ResourceEvent event;
  Z::gResources->DispatchEvent(Events::ResourcesUnloaded, &event);

  delete this;
}

void ResourceLibrary::ScriptsModified()
{
  mSwapScript.mCompileStatus = ZilchCompileStatus::Modified;
  ZilchManager::GetInstance()->mShouldAttemptCompile = true;

  // All dependents must be recompiled, so mark them as modified
  forRange (ResourceLibrary* dependent, Dependents.All())
    dependent->ScriptsModified();
}

void ResourceLibrary::FragmentsModified()
{
  mSwapScript.mCompileStatus = ZilchCompileStatus::Modified;
  mSwapFragment.mCompileStatus = ZilchCompileStatus::Modified;
  ZilchManager::GetInstance()->mShouldAttemptCompile = true;

  // All dependents must be recompiled, so mark them as modified
  forRange (ResourceLibrary* dependent, Dependents.All())
    dependent->FragmentsModified();
}

void ResourceLibrary::PluginsModified()
{
  // Plugins are always fully compiled so they don't need to have a 'modified'
  // state However, since we decided that scripts in the same resource library
  // (and in dependent libraries) are always dependent upon the plugins, then we
  // must mark all scripts as being modified
  ScriptsModified();
}

bool AddDependencies(Module& module,
                     ResourceLibrary* library,
                     HashSet<ResourceLibrary*>& modifiedLibrariesOut,
                     bool onlyFragments)
{
  forRange (ResourceLibrary* dependency, library->Dependencies)
  {
    // Blindly call compile and force all our dependencies to be compiled
    // (will return instantly if it's already compiled!)
    // If the library did not compile, we cannot possibly compile ourselves
    if (dependency->CompileScripts(modifiedLibrariesOut) == false)
      return false;

    // Depth first
    if (AddDependencies(module, dependency, modifiedLibrariesOut, onlyFragments) == false)
      return false;

    if (!onlyFragments)
    {
      forRange (SwapLibrary& swapPlugin, dependency->mSwapPlugins.Values())
      {
        LibraryRef pluginLibrary = swapPlugin.GetNewestLibrary();

        // This will only ever be null in the case of templates since
        // we should have successfully compiled all the plugins above in
        // CompileScripts
        if (pluginLibrary != nullptr)
          module.Append(pluginLibrary);
      }

      module.Append(dependency->mSwapScript.GetNewestLibrary());
    }
    module.Append(dependency->mSwapFragment.GetNewestLibrary());
  }

  return true;
}

void ResourceLibrary::OnScriptProjectPreParser(ParseEvent* e)
{
  // This isnt' the best solution, however because we can't intercept the plugin
  // library before its done we need to add extensions for the plugins here
  // (such as .YourComponent).
  forRange (SwapLibrary& swapLibrary, mSwapPlugins.Values())
  {
    if (swapLibrary.GetNewestLibrary())
    {
      EngineLibraryExtensions::AddNativeExtensions(*e->Builder, swapLibrary.GetNewestLibrary()->BoundTypes);
    }
  }

  EngineLibraryExtensions::AddExtensionsPreCompilation(*e->Builder, this);
}

void ResourceLibrary::OnScriptProjectPostSyntaxer(ParseEvent* e)
{
  EngineLibraryExtensions::AddExtensionsPostCompilation(*e->Builder);
}

bool ResourceLibrary::CompileScripts(HashSet<ResourceLibrary*>& modifiedLibrariesOut)
{
  // If we already compiled, then we know that all dependent libraries must have
  // been good This means that if we ever referenced
  if (mSwapScript.mCompileStatus == ZilchCompileStatus::Compiled)
    return true;

  // Currently the version is used to detect duplicate errors
  // Since scripts are changing, we definitely want to show duplicate errors.
  ++ZilchManager::GetInstance()->mVersion;

  // Scripts cannot compile if fragments do not compile
  if (CompileFragments(modifiedLibrariesOut) == false)
    return false;

  // Scripts cannot compile if plugins do not compile
  if (CompilePlugins(modifiedLibrariesOut) == false)
    return false;

  Module dependencies;

  // Remove the core library from dependencies as we're going to add it from
  // native libraries
  dependencies.Clear();

  // Add all native libraries
  dependencies.Append(MetaDatabase::GetInstance()->mNativeLibraries.All());

  // Add all resource library dependencies
  AddDependencies(dependencies, this, modifiedLibrariesOut, false);

  // Add our fragment library (it may be null if we didn't have any fragments)
  if (LibraryRef newestFragment = mSwapFragment.GetNewestLibrary())
    dependencies.Append(newestFragment);

  forRange (SwapLibrary& swapPlugin, mSwapPlugins.Values())
  {
    LibraryRef pluginLibrary = swapPlugin.GetNewestLibrary();

    // This will only ever be null in the case of templates since
    // we should have successfully compiled all the plugins above
    if (pluginLibrary != nullptr)
      dependencies.Append(pluginLibrary);
  }

  // By this point, we've already compiled all our dependencies
  ZPrint("  Compiling %s Scripts\n", this->Name.c_str());

  // Clear the project out since it may have files from before
  mScriptProject.Clear();

  // Add all scripts
  forRange (ZilchDocumentResource* script, mScripts)
  {
    // Templates shouldn't be compiled. They contain potentially invalid code
    // and identifiers such as RESOURCE_NAME_ that are replaced when a new
    // resource is created from the template
    if (script->GetResourceTemplate() == nullptr)
      mScriptProject.AddCodeFromString(script->mText, script->GetOrigin(), script);
  }

  mSwapScript.mPendingLibrary = mScriptProject.Compile(this->Name, dependencies, EvaluationMode::Project);

  if (mSwapScript.mPendingLibrary != nullptr)
  {
    modifiedLibrariesOut.Insert(this);
    mSwapScript.mCompileStatus = ZilchCompileStatus::Compiled;
    return true;
  }

  return false;
}

bool ResourceLibrary::CompileFragments(HashSet<ResourceLibrary*>& modifiedLibraries)
{
  // If we already compiled, then we know that all dependent libraries must have
  // been good
  if (mSwapFragment.mCompileStatus == ZilchCompileStatus::Compiled)
    return true;

  Module dependencies;

  // Remove the default core library
  dependencies.Clear();

  AddDependencies(dependencies, this, modifiedLibraries, true);

  // By this point, we've already compiled all our dependencies
  ZPrint("  Compiling %s Fragments\n", this->Name.c_str());

  ZilchCompileFragmentEvent e(dependencies, mFragments, this);
  ZilchManager::GetInstance()->DispatchEvent(Events::CompileZilchFragments, &e);
  mSwapFragment.mPendingLibrary = e.mReturnedLibrary;

  if (mSwapFragment.mPendingLibrary != nullptr)
  {
    modifiedLibraries.Insert(this);
    ZilchManager::GetInstance()->mPendingFragmentProjectLibrary = mSwapFragment.mPendingLibrary;
    mSwapFragment.mCompileStatus = ZilchCompileStatus::Compiled;
    return true;
  }

  return false;
}

bool ResourceLibrary::CompilePlugins(HashSet<ResourceLibrary*>& modifiedLibrariesOut)
{
  // Plugins should only depend on the Core library and native Zero libraries
  // (the only libraries we generate headers for) In the future we could attempt
  // to generate other library headers (dependencies...)
  Module pluginDependencies;
  pluginDependencies.Append(MetaDatabase::GetInstance()->mNativeLibraries.All());

  bool allPluginsSucceeded = true;

  forRange (ZilchLibraryResource* libraryResource, mPlugins.All())
  {
    if (libraryResource->GetResourceTemplate() != nullptr)
      continue;

    SwapLibrary& swapPlugin = mSwapPlugins[libraryResource];
    if (swapPlugin.mCompileStatus == ZilchCompileStatus::Modified)
    {
      Status status;
      String pluginPath = libraryResource->GetSharedLibraryPath();
      Resource* origin = libraryResource->GetOriginResource();

      swapPlugin.mPendingLibrary = Plugin::LoadFromFile(status, pluginDependencies, pluginPath, origin);

      // If the status failed, it could just be because the plugin was empty
      // (we're in progress compiling it). Note: We'll get a separate error if
      // the plugin fails to compile or if we can't find the compiler.
      if (status.Failed() && status.Context != Plugin::StatusContextEmpty)
      {
        Console::Print(
            Filter::DefaultFilter, "Plugin Error (%s): %s\n", libraryResource->Name.c_str(), status.Message.c_str());
      }

      if (swapPlugin.mPendingLibrary != nullptr)
      {
        modifiedLibrariesOut.Insert(this);
        swapPlugin.mCompileStatus = ZilchCompileStatus::Compiled;
      }
      else
      {
        // We failed to compile one plugin, so we should stop all other script
        // compilation
        allPluginsSucceeded = false;
      }
    }
  }

  return allPluginsSucceeded;
}

void ResourceLibrary::PreCommitUnload()
{
  // These unload calls are for removing types from the meta database before
  // committing any pending types in order to prevent any type discrepancies or
  // issues. Do NOT unload types unless there is actually a new library pending
  // commit.
  if (mSwapFragment.mPendingLibrary)
    mSwapFragment.Unload();

  forRange (SwapLibrary& swapPlugin, mSwapPlugins.Values())
    if (swapPlugin.mPendingLibrary)
      swapPlugin.Unload();

  if (mSwapScript.mPendingLibrary)
    mSwapScript.Unload();
}

void ResourceLibrary::Commit()
{
  // Replace the fragment library
  mSwapFragment.Commit();

  forRange (SwapLibrary& swapPlugin, mSwapPlugins.Values())
  {
    swapPlugin.Commit();
  }

  // Replace the script library
  mSwapScript.Commit();
}

} // namespace Zero
