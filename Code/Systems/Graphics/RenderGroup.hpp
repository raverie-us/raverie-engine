// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{

/// How Materials are categorized, determines which graphicals are drawn in a
/// render pass.
class RenderGroup : public DataResource
{
public:
  RaverieDeclareType(RenderGroup, TypeCopyMode::ReferenceType);

  RenderGroup();

  void Serialize(Serializer& stream) override;

  void OnResourceListItemAdded(ResourceListEvent* event);
  void OnResourceListItemRemoved(ResourceListEvent* event);

  void OnChildListItemAdded(ResourceListEvent* event);
  void OnChildListItemRemoved(ResourceListEvent* event);

  // Needed for sending resource modified event when properties are modified.
  void OnObjectModified(ObjectEvent* event);

  // Appends to set all associated Materials of this RenderGroup and all its sub
  // groups.
  void GetMaterials(HashSet<Material*>& materials);
  // Adds to array this RenderGroup and all sub groups.
  void GetRenderGroups(Array<RenderGroup*>& renderGroups);

  /// Returns whether or not the given RenderGroup is a sub group of this.
  bool IsSubRenderGroup(RenderGroup* renderGroup);
  /// Returns whether or not this is a sub group of the given RenderGroup.
  bool IsSubRenderGroupOf(RenderGroup* renderGroup);

  // List of resources to be associated with that are saved by this resource
  MaterialList mSerializedList;

  // List of resources that have this resource in their serialized list
  // Populated at runtime, used to easily see associations from the property
  // grid
  MaterialList mReferencedByList;

  /// Determines the order that graphicals will be drawn when processed as this
  /// RenderGroup.
  GraphicalSortMethod::Enum mGraphicalSortMethod;

  /// RenderGroup that this is a sub group of. Also a sub group of all of its
  /// parents.
  HandleOf<RenderGroup> GetParentRenderGroup();
  void SetParentRenderGroup(HandleOf<RenderGroup> renderGroup);
  String mParentRenderGroup;

  /// For assigning child RenderGroups, making this a parent group of everything
  /// in the list.
  ChildRenderGroupList mChildRenderGroups;

  // List used to get associated resources, need to know every associated
  // Material to collect shader inputs Should have one entry of other resource
  // if either or both reference each other Pointers are never kept around if a
  // resource is removed, cannot be handles or runtime resources will never get
  // removed
  Array<Material*> mActiveResources;

  // Managed at runtime to able to access the hierarchy quickly without
  // having to traverse all RenderGroups to find established connections.
  RenderGroup* mParentInternal;
  Array<RenderGroup*> mChildrenInternal;
  // The single helper method that must be used to set the internal pointers.
  void SetParentInternal(RenderGroup* renderGroup);

  // For internal assignment and usage by the graphics engine.
  int mSortId;
};

class RenderGroupManager : public ResourceManager
{
public:
  DeclareResourceManager(RenderGroupManager, RenderGroup);
  RenderGroupManager(BoundType* resourceType);
};

// Filter for allowable parent RenderGroups.
bool ParentRenderGroupFilter(HandleParam object, Property* property, HandleParam result, Status& status);
// Callback for resource list property editor.
void ListItemValidChild(GraphicsResourceList* resourceList, String entryIdName, Status& status);

} // namespace Raverie
