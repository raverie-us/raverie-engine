// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class ContentItem;
class ContentLibrary;
class ResourcePackage;
class BuilderComponent;
class Resource;
typedef Array<ContentItem*> ContentItemArray;

namespace Events
{
// Sent out before scripts are compiled
DeclareEvent(PreScriptCompile);
DeclareEvent(CompileZilchFragments);
// Sent out on the Z::gResources in the constructor of ResourceLibrary (no
// resources added/loaded yet).
DeclareEvent(ResourceLibraryConstructed);
} // namespace Events

// Status of Zilch Script Compile
DeclareEnum2(ZilchCompileStatus,
             Modified, // Scripts have been modified and will not match what is
                       // in library
             Compiled  // All scripts compiled and project library is up to date
);

// Zilch Compiled Event
class ZilchCompiledEvent : public Event
{
public:
  ZilchDeclareType(ZilchCompiledEvent, TypeCopyMode::ReferenceType);
  LibraryRef mLibrary;
};

// Compile Fragment Event
class ZilchCompileFragmentEvent : public Event
{
public:
  ZilchDeclareType(ZilchCompileFragmentEvent, TypeCopyMode::ReferenceType);
  ZilchCompileFragmentEvent(Module& dependencies,
                            Array<ZilchDocumentResource*>& fragments,
                            ResourceLibrary* owningLibrary);

  Module& mDependencies;
  Array<ZilchDocumentResource*>& mFragments;
  LibraryRef mReturnedLibrary;
  ResourceLibrary* mOwningLibrary;
};

// Resource Entry
/// ResourceEntry built by a builder stored in ResourcePackage.
class ResourceEntry
{
public:
  ResourceEntry();
  ResourceEntry(uint order,
                StringParam type,
                StringParam name,
                StringParam location,
                ResourceId id,
                ContentItem* libraryResource,
                BuilderComponent* builder);

  uint LoadOrder;
  String Type;
  String Name;
  ResourceId mResourceId;
  ResourceLibrary* mLibrary;

  // For loading from files.
  String Location;
  String FullPath;

  // Loading from Data.
  DataBlock Block;

  // Only available when the editor is active.
  ContentItem* mLibrarySource;
  BuilderComponent* mBuilder;

  String ToString(bool shortFormat = false) const;
  void Serialize(Serializer& stream);
  ResourceTemplate* GetResourceTemplate();
};

typedef Array<ResourceEntry> ResourceListing;

// Resource Package
/// A Resource package is collection of resources
/// to be loaded. Built from a Content Library or
/// dynamically.
class ResourcePackage : public ReferenceCountedThreadSafeId32EventObject
{
public:
  ResourcePackage();
  ZilchDeclareType(ResourcePackage, TypeCopyMode::ReferenceType);

  void Save(StringParam filename);
  void Load(StringParam filename);
  void Serialize(Serializer& stream);

  String Name;
  String Location;
  ResourceListing Resources;

  ContentItemArray EditorProcessing;
};

class ResourcePackageDisplay : public MetaDisplay
{
  ZilchDeclareType(ResourcePackageDisplay, TypeCopyMode::ReferenceType);

  String GetName(HandleParam object) override;
  String GetDebugText(HandleParam object) override;
};

// Keeps the current library that we have already built and one that is pending
// with newer code.
class SwapLibrary
{
public:
  SwapLibrary();

  LibraryRef GetNewestLibrary();
  bool HasPendingLibrary();
  void Commit();
  void Unload();

  LibraryRef mCurrentLibrary;
  LibraryRef mPendingLibrary;

  // Is the library up to date or are there modifications?
  ZilchCompileStatus::Enum mCompileStatus;
};

// Resource Library
/// A Resource Library is a set of resources loaded from a
/// resource package. Used to manage resource lifetimes.
/// EventHandler is Zilch's EventObject and is needed for compilation events.
/// This should be removed once we combine the two
class ResourceLibrary : public SafeId32EventObject, public EventHandler
{
public:
  ZilchDeclareDerivedTypeExplicit(ResourceLibrary, SafeId32EventObject, TypeCopyMode::ReferenceType);

  ResourceLibrary();
  ~ResourceLibrary();

  // Add a resource to this set
  void Add(Resource* resource, bool isNew);

  // Remove from this resource library
  // only used when resources are deleted not unloaded.
  void Remove(Resource* resource);

  void AddDependency(ResourceLibrary* dependency);

  /// Checks the current libraries to see if the given type was built from our
  /// scripts, fragments, or plugins.
  bool BuiltType(BoundType* type);

  /// If the given type was built in this library, it will attempt to find the
  /// type replacing it in the pending libraries. It's usually only valid to
  /// call this function in response to the ZilchManager compile events before
  /// libraries are committed.
  BoundType* GetReplacingType(BoundType* oldType);

  /// Can this library hold a reference to a resource in the given library.
  /// Checks library dependencies to see if the given library is in a dependent
  /// (parent).
  bool CanReference(ResourceLibrary* library);

  /// Deletes all Resources in this library.
  void Unload();

  // When resources are modified, we want to mark our compile status as modified
  void ScriptsModified();
  void FragmentsModified();
  void PluginsModified();

  // Attempts to compile this resource library if it was modified
  // Note that this function will call Compile if a dependent resource library
  // requires it Also remember that this function does not handle things that
  // need to happen globally
  bool CompileScripts(HashSet<ResourceLibrary*>& modifiedLibrariesOut);
  bool CompileFragments(HashSet<ResourceLibrary*>& modifiedLibrariesOut);
  bool CompilePlugins(HashSet<ResourceLibrary*>& modifiedLibrariesOut);

  void OnScriptProjectPreParser(ParseEvent* e);
  void OnScriptProjectPostSyntaxer(ParseEvent* e);

  // Only called once all libraries within Zero are fully compiled
  // Turns each pending library into the current library
  void PreCommitUnload();
  void Commit();

  /// Name of the resource package from which this set was loaded.
  String Name;
  String Location;

  // All the resource libraries that we are dependent upon
  // This must form a DAG (cycles are not allowed) where the core set is the
  // root
  Array<ResourceLibrary*> Dependencies;

  // All libraries that depend on us.
  Array<ResourceLibrary*> Dependents;

  // All scripts and plugins that exist within this resource library
  Array<ZilchDocumentResource*> mScripts;
  Array<ZilchDocumentResource*> mFragments;
  Array<ZilchLibraryResource*> mPlugins;

  // The plugins that this resource library has built (may be empty if none
  // exist or haven't been compiled yet)
  OrderedHashMap<ZilchLibraryResource*, SwapLibrary> mSwapPlugins;

  // The library that this resource library has built (may be null if it hasn't
  // compiled yet) If this is set to 'Compiled', it also implies that the
  // Fragment status is compiled as well
  SwapLibrary mSwapScript;

  // The fragment library that this resource library has built (may be null if
  // it hasn't compiled yet)
  SwapLibrary mSwapFragment;

  // A project we use for the scripts (we clear it and re-add all code files)
  // We need this to stick around for the Zilch debugger
  Project mScriptProject;

  // All loaded resources. These handles are the ones in charge of keeping the
  // Resources in this library alive.
  Array<HandleOf<Resource>> Resources;

  // When a Resource is added, we want to do something special for scripts and
  // fragments. Unfortunately, we don't know those types in the Engine project.
  // These should be set during the Graphics and ZilchScript project
  // initialization.
  static BoundType* sScriptType;
  static BoundType* sFragmentType;
  // Getter for the unloading flag
  static bool IsLibraryUnloading()
  {
    return sLibraryUnloading;
  }

private:
  // Used to detect incorrect removal of non-runtime resources
  static bool sLibraryUnloading;
};

} // namespace Zero
