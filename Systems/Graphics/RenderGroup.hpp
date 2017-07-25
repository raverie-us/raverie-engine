////////////////////////////////////////////////////////////////////////////////
/// Authors: Nathan Carlson
/// Copyright 2016, DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// How Materials are categorized, determines which graphicals are drawn in a render pass.
class RenderGroup : public DataResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  RenderGroup();

  void Serialize(Serializer& stream) override;

  void OnResourceListItemAdded(ResourceListEvent* event);
  void OnResourceListItemRemoved(ResourceListEvent* event);

  /// Determines the order that graphicals will be drawn when processed as this RenderGroup.
  GraphicalSortMethod::Enum mGraphicalSortMethod;

  // List of resources to be associated with that are saved by this resource
  MaterialList mSerializedList;
  // List of resources that have this resource in their serialized list
  // Populated at runtime, used to easily see associations from the property grid
  MaterialList mReferencedByList;
  // List used to get associated resources, need to know every associated Material to collect shader inputs
  // Should have one entry of other resource if either or both reference each other
  // Pointers are never kept around if a resource is removed, cannot be handles or runtime resources will never get removed
  Array<Material*> mActiveResources;

  // For internal assignment and usage by the graphics engine.
  int mSortId;
};

class RenderGroupManager : public ResourceManager
{
public:
  DeclareResourceManager(RenderGroupManager, RenderGroup);
  RenderGroupManager(BoundType* resourceType);
};

} // namespace Zero
