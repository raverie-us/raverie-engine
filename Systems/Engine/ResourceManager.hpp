///////////////////////////////////////////////////////////////////////////////
///
/// \file ResourceManager.hpp
/// Declaration of the Resource Manager.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum3(RemoveMode,
  // This means the resource is not being removed. This is odd, and should probably be removed,
  // but it's needed until we split ResourceEvent into a ResourceRemovedEvent.
  None,
  // Resource is being unloaded
  Unloading,
  // Resource is being removed from project (editor only)
  Deleted);

// Forward Declarations
class Archetype;
class ResourceManager;
class DocumentResource;
class ResourceLoader;
struct ResourceAdd;

// Events
namespace Events
{
  DeclareEvent(ResourceAdded);
  DeclareEvent(ResourceModified);
  DeclareEvent(ResourceRemoved);
  DeclareEvent(ResourceReload);
}

/// Event structure sent out in the above events.
class ResourceEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  ResourceEvent();
  ResourceEvent(ResourceManager* manager, Resource* resource);
  String Name;
  String Path;
  ResourceManager* Manager;
  Resource* EventResource;
  ResourceLibrary* EventResourceLibrary;
  RemoveMode::Enum RemoveMode;
};

/// Resource managers stores resources of particular types. Derived resource
/// managers inherit from this class to implement resource managers of different
/// types.
class ResourceManager : public EventObject
{
public:
  OverloadedNew();

  // Typedefs.
  typedef HashMap<ResourceId, Resource*> ResourceIdMapType;
  typedef ResourceIdMapType::valuerange ResourceRange;

  // Constructor / Destructor.
  ResourceManager(BoundType* metaType);
  ~ResourceManager();

  BoundType* GetResourceType();

  // Allocates and initializes a new resource. Should be the only method that uses AllocateDefaultConstructed().
  virtual Resource* CreateNewResourceInternal(StringParam name);
  // Calls CreateNewResourceInternal and does required setup for runtime resources
  virtual Resource* CreateRuntimeInternal(StringParam name = String());

  // Get a resource by NameId string
  Resource* GetResource(StringParam name, ResourceNotFound::Enum notFound);

  // Get a resource by resourceId
  Resource* GetResource(ResourceId resourceId, ResourceNotFound::Enum notFound);

  // Get the resource to used for default resource values
  Resource* GetDefaultResource();

  // Get the resource to use when resource can not be found to indicate an error
  Resource* GetFallbackResource();

  //Remove the resource from the resource manager.
  void Remove(Resource* resource, RemoveMode::Enum removeMode);

  //Validate that a name of a resource is ok to add
  //Before calling this, we already check for valid file names,
  // and that it won't conflict with another resource of the same name
  virtual void ValidateName(Status& status, StringParam name);

  //List all available resources
  void EnumerateResources(Array<String>& values);
  //Gets all available resources, values should not be held onto after used.
  void EnumerateResources(Array<Resource*>& values);

  //Add a new resource with a new name.
  virtual String GetTemplateSourceFile(ResourceAdd& resourceAdd);

  ResourceRange AllResources();

  // Called after a resource has been duplicated for any custom resource logic
  virtual void ResourceDuplicated(Resource* resource, Resource* duplicate) {}

  // Name of the resource type managed by this resource manager.
  String mResourceTypeName;
  // Resource Type managed by this resource manager. Storing this BoundType* is safe because 
  // we only have native Resource types. If we ever support non-native Resource types, this will
  // have to be changed to a handle.
  BoundType* mResourceType;
  // Name of the resource to use when defaulting a resource field.
  String DefaultResourceName;
  // Name of the resource to use when a resource can not be found.
  // Usually the same as the Default
  String FallbackResourceName;

  // Can new resources be created in the editor with CreateNewResource?
  bool mCanCreateNew;
  // Can resources be added from file?
  bool mCanAddFile;
  // Can resources be reloaded?
  bool mCanReload;
  // Can resources be duplicated?
  bool mCanDuplicate;
  // Ignore the missing fallback check.
  bool mNoFallbackNeeded;
  // Resource is searchable
  bool mSearchable;
  // Need Preview for editing?
  bool mPreview;
  // Is this resource type shown?
  bool mHidden;
  // The extension for the resource. If empty defaulted to .data
  String mExtension;
  // Used to place this type in a category in the Resource Add window.
  String mCategory;
  // Sort weight in the add resource window.
  uint mAddSortWeight;

  // This is currently only used for the import dialog in the add menu. Some of the extension
  // in this array are not the actual extensions used by the engine, but are filters for use
  // in the file dialog.
  Array<FileDialogFilter> mOpenFileFilters;

  friend class ResourceSystem;
  friend class ResourceLoader;

//Internals
  //Saves/Loads a resource from a stream object. If saving the resourcePtr
  //is saved, if loading the new resource is returned.
  Resource* ResolveResourceStream(cstr fieldName, Serializer& stream, Resource* resourcePtr, 
                                  ResourceNotFound::Enum ifNotFound, cstr defaultResourceName = nullptr);

//protected:
  //Map of ResourceIds to Resources.
  ResourceIdMapType ResourceIdMap;

  //Map of ResourceName strings to Resources.
  typedef HashMap<String, ResourceId> ResourceNameMapType;
  ResourceNameMapType ResourceNameMap;

  void DestroyResources();
  void AddLoader(StringParam name, ResourceLoader* resourceLoader);
  void AddResource(ResourceEntry& entry, Resource* resource);

private:
  Resource* GetResourceNameOrId(StringRange name, ResourceId resourceId);
  Resource* GetResourceById(ResourceId id);
  Resource* GetResourceByName(StringParam name);
  void MissingResource(StringParam resourceName, ResourceId resourceId);

  // Used internally to allocated the derived resource type, do not call this method.
  virtual Resource* AllocateDefaultConstructed() = 0;
};

//-------------------------------------------------------------- Resource Loader

/// Resource loader interface. Used to load resource of various formats.
/// A resource manager may provide multiple loaders.
class ResourceLoader
{
public:
  ResourceLoader(){};
  virtual ~ResourceLoader(){};
  virtual HandleOf<Resource> LoadFromFile(ResourceEntry& entry) = 0;
  virtual void ReloadFromFile(Resource* resource, ResourceEntry& entry) {}
  virtual HandleOf<Resource> LoadFromBlock(ResourceEntry& entry) { return nullptr; }
};

//------------------------------------------------------------ Native Resource Manager Setup

// Adds Resource Typed Static Functions for native resource types
// CreateRuntime should not be used directly, implement a static CreateRuntime method on the resource type
// and call the manager's CreateRuntime from there, along with any other required setup of the resource for runtime usage.
// If creation of a runtime resource in script is intended then bind the resource's CreateRuntime method.
#define DeclareResourceManager(ManagerType, ManagerResourceType)                                                            \
  typedef ResourceManager ZilchBase;                                                                                        \
  typedef ManagerType ZilchSelf;                                                                                            \
  typedef ManagerResourceType ResourceType;   typedef ManagerResourceType RT;    typedef ManagerType self_type;             \
  static void Initialize();                                                                                                 \
  static ManagerType* Instance;                                                                                             \
  static ManagerType* GetInstance(){return Instance;}                                                                       \
  static bool IsValid(){return Instance != NULL; }                                                                          \
  static RT* GetDefault(){return (RT*)Instance->GetDefaultResource();}                                                      \
  static RT* GetFallback(){return (RT*)Instance->GetFallbackResource();}                                                    \
  static RT* Find(ResourceId resourceId){return (RT*)Instance->GetResource(resourceId, ResourceNotFound::ErrorFallback);}   \
  static RT* Find(StringParam nameId){return (RT*)Instance->GetResource(nameId, ResourceNotFound::ErrorFallback);}            \
  static RT* FindOrNull(StringParam nameId){return (RT*)Instance->GetResource(nameId, ResourceNotFound::ReturnNull);}         \
  static RT* CreateNewResource(StringParam name){return (RT*)Instance->CreateNewResourceInternal(name);}                    \
  private:                                                                                                                  \
  friend class ManagerResourceType;                                                                                         \
  static RT* CreateRuntime(StringParam name = String()){return (RT*)Instance->CreateRuntimeInternal(name);}                 \
  Resource* AllocateDefaultConstructed() override {return new RT();}                                                        \
  public:

// Implement native resource managers
#define ImplementResourceManager(ManagerType, ManagerResourceType)                                \
  ManagerType* ManagerType::Instance = NULL;                                                      \
  void ManagerType::Initialize(){                                                                 \
  BoundType* metaType = ZilchTypeId(ManagerResourceType);                                         \
  ManagerType::Instance = new ManagerType(metaType);  }                                           \

// Initialize a ResourceManager it must be forward declared
#define InitializeResourceManager(ManagerType)                                                    \
  ManagerType::Initialize();

template<typename ManagerType>
void SerializeResourceImpl(cstr fieldName, Serializer& stream,
                           HandleOf<typename ManagerType::ResourceType>& resourceRef)
{
  resourceRef = (typename ManagerType::ResourceType*)
    ManagerType::GetInstance()->ResolveResourceStream(fieldName, stream, resourceRef, ResourceNotFound::ReturnDefault);
}

template<typename ManagerType>
void SerializeResourceImpl(cstr fieldName, Serializer& stream,
                           HandleOf<typename ManagerType::ResourceType>& resourceRef,
                           cstr defaultResourceName, bool canBeNull = false)
{
  resourceRef = (typename ManagerType::ResourceType*)
    ManagerType::GetInstance()->ResolveResourceStream(fieldName, stream, resourceRef, ResourceNotFound::ReturnNull, defaultResourceName);

  if (canBeNull == false && resourceRef.IsNull())
    resourceRef = ManagerType::GetInstance()->FindOrNull(defaultResourceName);
}

template<typename ManagerType>
void SerializeResourceImpl(cstr fieldName, Serializer& stream,
                           HandleOf<typename ManagerType::ResourceType>& resourceRef,
                           StringParam defaultResourceName, bool canBeNull = false)
{
  SerializeResourceImpl<ManagerType>(fieldName, stream, resourceRef, defaultResourceName.c_str(), canBeNull);
}

#define SerializeResourceName(name, managerName) \
        SerializeResourceImpl<managerName>(#name, stream, name);

#define SerializeResourceNameDirect(name, managerName) \
        SerializeResourceImpl<managerName>(#name, stream, name);

#define SerializeResource(fieldName, name, managerName) \
        SerializeResourceImpl<managerName>(fieldName, stream, name);

#define SerializeResourceNameManagerDefault(name, managerName) \
        SerializeResourceImpl<managerName>(#name, stream, name, managerName::GetInstance()->DefaultResourceName.c_str());

#define SerializeResourceNameDefault(name, managerName, defaultValue) \
        SerializeResourceImpl<managerName>(#name, stream, name, defaultValue);

#define SerializeNullableResourceNameDefault(name, managerName, defaultValue) \
        SerializeResourceImpl<managerName>(#name, stream, name, defaultValue, true);

}//namespace Zero
