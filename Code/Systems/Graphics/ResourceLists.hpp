// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{

namespace Events
{
DeclareEvent(ResourceListItemAdded);
DeclareEvent(ResourceListItemRemoved);
} // namespace Events

class ResourceListEvent : public Event
{
public:
  RaverieDeclareType(ResourceListEvent, TypeCopyMode::ReferenceType);
  String mResourceIdName;
};

class GraphicsResourceList : public SafeId32EventObject
{
public:
  GraphicsResourceList(Resource* owner);
  GraphicsResourceList(const GraphicsResourceList& rhs);
  // Required by property interface, default doesn't exist because of
  // EventObject
  GraphicsResourceList& operator=(const GraphicsResourceList& graphicsResourceList);

  // Interface required by ResourceListEditor
  /// Returns the IdName of all resources in the list.
  Array<String>::range GetIdNames();
  uint GetResourceIndex(StringParam resourceIdName);
  virtual void CheckForAddition(Status& status, Resource* resource);
  void AddResource(StringParam resourceIdName, uint index = -1);
  void RemoveResource(StringParam resourceIdName);
  String GetDisplayName()
  {
    return mDisplayName;
  }
  bool GetReadOnly()
  {
    return mReadOnly;
  }
  bool GetExpanded()
  {
    return mExpanded ? *mExpanded : true;
  }
  void SetExpanded(bool expanded)
  {
    if (mExpanded)
      *mExpanded = expanded;
  }
  typedef void (*ListItemCallback)(GraphicsResourceList* resourceList, String entryIdName, Status& status);
  ListItemCallback mListItemCallback;

  Array<String> mResourceIdNames;
  String mDisplayName;
  bool mReadOnly;

  // The resource that owns this list
  Resource* mOwner;

  bool* mExpanded;
  static bool mMaterialSerializedExpanded;
  static bool mMaterialRuntimeExpanded;
  static bool mRenderGroupSerializedExpanded;
  static bool mRenderGroupRuntimeExpanded;
  static bool mChildRenderGroupListExpanded;
};

class RenderGroupList : public GraphicsResourceList
{
public:
  RaverieDeclareType(RenderGroupList, TypeCopyMode::ReferenceType);

  RenderGroupList(Resource* owner);

  // Interface required by ResourceListEditor
  typedef RenderGroupManager ManagerType;
  String GetResourceTypeName();

  // For script to modify runtime resources
  /// Adds the RenderGroup to this Material's list. Runtime resources only.
  void Add(RenderGroup& renderGroup);
  /// Removes the RenderGroup from this Material's list. Runtime resources only.
  void Remove(RenderGroup& renderGroup);
  /// Range of all resources in the list.
  Array<HandleOf<RenderGroup>> All();
};

class ChildRenderGroupList : public RenderGroupList
{
public:
  RaverieDeclareType(ChildRenderGroupList, TypeCopyMode::ReferenceType);

  ChildRenderGroupList(Resource* owner);

  void CheckForAddition(Status& status, Resource* resource) override;
};

class MaterialList : public GraphicsResourceList
{
public:
  RaverieDeclareType(MaterialList, TypeCopyMode::ReferenceType);

  MaterialList(Resource* owner);

  // Interface required by ResourceListEditor
  typedef MaterialManager ManagerType;
  String GetResourceTypeName();

  // For script to modify runtime resources
  /// Adds the Material to this RenderGroups's list. Runtime resources only.
  void Add(Material& material);
  /// Removes the Material from this RenderGroups's list. Runtime resources
  /// only.
  void Remove(Material& material);
  /// Range of all resources in the list.
  Array<HandleOf<Material>> All();
};

template <typename SelfResource, typename OtherResource>
void ResourceListResourceAdded(SelfResource* selfResource, OtherResource* otherResource, String* listName = nullptr)
{
  // If the found resource has a matching name for an old entry then the id part
  // of this entry will be out of date Reassign the current resource's idName so
  // that addition queries made against the resource list are valid
  if (listName != nullptr)
    *listName = otherResource->ResourceIdName;

  // Function potentially called more than once if entry already exists
  ErrorIf(otherResource->mReferencedByList.mResourceIdNames.Contains(selfResource->ResourceIdName),
          "Resource already in list.");
  // Add to the runtime list of the other resource
  otherResource->mReferencedByList.mResourceIdNames.PushBack(selfResource->ResourceIdName);

  bool activeHandle = selfResource->mActiveResources.Contains(otherResource);
  // There is an error in our management of cross references if these ever don't
  // match
  ErrorIf(activeHandle != otherResource->mActiveResources.Contains(selfResource),
          "Material/RenderGroup cross reference mismatch.");

  // Do not add another entry if resources are already referencing each other
  if (!activeHandle)
  {
    // Add to both active lists
    otherResource->mActiveResources.PushBack(selfResource);
    selfResource->mActiveResources.PushBack(otherResource);
  }
}

template <typename SelfResource, typename OtherResource>
void ResourceListResourceRemoved(SelfResource* selfResource, OtherResource* otherResource)
{
  // If other resource was in the serialized list then entry from runtime list
  // needs to be removed
  if (selfResource->mSerializedList.mResourceIdNames.Contains(otherResource->ResourceIdName))
  {
    ErrorIf(!otherResource->mReferencedByList.mResourceIdNames.Contains(selfResource->ResourceIdName),
            "Resource not in list.");
    otherResource->mReferencedByList.mResourceIdNames.EraseValue(selfResource->ResourceIdName);
  }

  ErrorIf(!otherResource->mActiveResources.Contains(selfResource), "Resource not in list.");

  // Remove active entry from other resource but not self resource
  // This method is expected to be called while iterating over self resource's
  // active list and is cleared afterward
  otherResource->mActiveResources.EraseValue(selfResource);
}

template <typename SelfResource, typename OtherResource>
void ResourceListEntryAdded(SelfResource* selfResource, OtherResource* otherResource)
{
  ErrorIf(otherResource->mReferencedByList.mResourceIdNames.Contains(selfResource->ResourceIdName),
          "Resource already in list.");
  // Add to the runtime list of the other resource
  otherResource->mReferencedByList.mResourceIdNames.PushBack(selfResource->ResourceIdName);

  bool activeHandle = selfResource->mActiveResources.Contains(otherResource);
  // There is an error in our management of cross references if these ever don't
  // match
  ErrorIf(activeHandle != otherResource->mActiveResources.Contains(selfResource),
          "Material/RenderGroup cross reference mismatch.");

  // Do not add another entry if resources are already referencing each other
  if (!activeHandle)
  {
    // Add to both active lists
    otherResource->mActiveResources.PushBack(selfResource);
    selfResource->mActiveResources.PushBack(otherResource);
  }
}

template <typename SelfResource, typename OtherResource>
void ResourceListEntryRemoved(SelfResource* selfResource, OtherResource* otherResource)
{
  ErrorIf(!otherResource->mReferencedByList.mResourceIdNames.Contains(selfResource->ResourceIdName),
          "Resource not in list.");
  // Remove from the runtime list of the other resource
  otherResource->mReferencedByList.mResourceIdNames.EraseValue(selfResource->ResourceIdName);

  // There is an error in our management of cross references if these ever don't
  // match
  ErrorIf(selfResource->mActiveResources.Contains(otherResource) !=
              otherResource->mActiveResources.Contains(selfResource),
          "Material/RenderGroup cross reference mismatch.");

  // Do not remove entries if still in other resource's list
  if (!otherResource->mSerializedList.mResourceIdNames.Contains(selfResource->ResourceIdName))
  {
    otherResource->mActiveResources.EraseValue(selfResource);
    selfResource->mActiveResources.EraseValue(otherResource);
  }
}

template <typename ResourceManagerType>
void ResetIdName(String& idName)
{
  typename ResourceManagerType::ResourceType* resource = ResourceManagerType::FindOrNull(idName);
  // Serialized list can have entries for removed resources.
  if (resource != nullptr)
    idName = resource->ResourceIdName;
}

template <typename SelfResource, typename OtherManager>
void ResourceListResetIdNames(Array<Resource*>& resources)
{
  forRange (Resource* resource, resources.All())
  {
    SelfResource* selfResource = (SelfResource*)resource;

    forRange (String& idName, selfResource->mReferencedByList.mResourceIdNames.All())
      ResetIdName<OtherManager>(idName);

    if (selfResource->IsWritable())
    {
      forRange (String& idName, selfResource->mSerializedList.mResourceIdNames.All())
        ResetIdName<OtherManager>(idName);
    }
  }
}

// Should be called when a resource is added to the engine
void ResourceListAdd(Material* material);
void ResourceListAdd(RenderGroup* renderGroup);
// Should be called when a resource is removed from the engine
void ResourceListRemove(Material* material);
void ResourceListRemove(RenderGroup* renderGroup);
// Should be called on every Material after any RenderGroups are added to the
// engine
void ResourceListResolveReferences(Material* material);
// Should be called on every RenderGroup after any Materials are added to the
// engine
void ResourceListResolveReferences(RenderGroup* renderGroup);
// Should be called when a RenderGroup is added or renamed to resolve and update
// idNames.
void ResolveRenderGroupHierarchies();

} // namespace Raverie
