///////////////////////////////////////////////////////////////////////////////
///
/// \file Material.hpp
/// Declaration of the Material resource class and manager.
///
/// Authors: Chris Peters, Nathan Carlson
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// A composition of shader fragments that defines a shader program that is used when rendering Graphicals.
class Material : public DataResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Material();
  ~Material();

  HandleOf<Resource> Clone() override {return RuntimeClone();}
  /// Creates an anonymous copy that can be independently modified, destroyed when all references are gone.
  HandleOf<Material> RuntimeClone();

  void Serialize(Serializer& stream) override;
  void SerializeMaterialBlocks(Serializer& stream);
  void Initialize() override;
  void Unload() override;

  void ReInitialize();

  void ResourceModified() override;

  // Ui for label divider
  uint GetCompositionLabel() { return 0; }

  // Factory interface
  uint GetSize() const;
  MaterialBlockHandle GetBlockAt(uint index);
  MaterialBlockHandle GetById(BoundType* typeId);
  uint GetBlockIndex(BoundType* typeId);
  void Add(MaterialBlockHandle block, int index);
  bool Remove(MaterialBlockHandle block);

  void OnResourceListItemAdded(ResourceListEvent* event);
  void OnResourceListItemRemoved(ResourceListEvent* event);

  // Adds inputs to the passed in array
  IndexRange AddShaderInputs(Array<ShaderInput>& shaderInputs, uint version);

  void UpdateCompositeName();

  MaterialRenderData* mRenderData;

  // List of resources to be associated with that are saved by this resource
  RenderGroupList mSerializedList;
  // List of resources that have this resource in their serialized list
  // Populated at runtime, used to easily see associations from the property grid
  RenderGroupList mReferencedByList;
  // List used to get associated resources, need to know every associated RenderGroup to add Graphical entries
  // Should have one entry of other resource if either or both reference each other
  // Pointers are never kept around if a resource is removed, cannot be handles or runtime resources will never get removed
  Array<RenderGroup*> mActiveResources;

  typedef Array<HandleOf<MaterialBlock>> MaterialBlockArray;
  MaterialBlockArray mMaterialBlocks;

  String mCompositeName;
  Array<String> mFragmentNames;

  bool mCompositionChanged;
  bool mPropertiesChanged;
  uint mInputRangeVersion;
  IndexRange mCachedInputRange;
};

//------------------------------------------------------------- Material Manager
class MaterialManager : public ResourceManager
{
public:
  DeclareResourceManager(MaterialManager, Material);
  MaterialManager(BoundType* resourceType);

  void ResourceDuplicated(Resource* resource, Resource* duplicate) override;
};

class CompositionLabelExtension : public EditorPropertyExtension
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
};

} // namespace Zero
