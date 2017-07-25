///////////////////////////////////////////////////////////////////////////////
///
/// \file Material.cpp
/// Implementation of the Material class and Manager.
///
/// Authors: Chris Peters, Nathan Carlson
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(CompositionLabelExtension, builder, type)
{
}

ZilchDefineType(Material, builder, type)
{
  type->Add(new MaterialFactory());

  ZeroBindDocumented();

  ZilchBindMethod(RuntimeClone);

  ZilchBindFieldGetterProperty(mSerializedList);
  ZilchBindFieldGetterProperty(mReferencedByList);

  ZilchBindGetterProperty(CompositionLabel)->Add(new CompositionLabelExtension());
}

Material::Material()
  : mRenderData(nullptr)
  , mCompositionChanged(false)
  , mPropertiesChanged(true)
  , mInputRangeVersion(-1)
  , mSerializedList(this)
  , mReferencedByList(this)
{
  mSerializedList.mDisplayName = "RenderGroups";
  mReferencedByList.mDisplayName = "ReferencedBy";
  mReferencedByList.mReadOnly = true;

  mSerializedList.mExpanded = &GraphicsResourceList::mMaterialSerializedExpanded;
  mReferencedByList.mExpanded = &GraphicsResourceList::mMaterialRuntimeExpanded;
}

Material::~Material()
{
}

void Material::Serialize(Serializer& stream)
{
  // Must serialize before polymorphic data
  stream.SerializeFieldDefault("RenderGroups", mSerializedList.mResourceIdNames, Array<String>());

  SerializeMaterialBlocks(stream);
}

void Material::SerializeMaterialBlocks(Serializer& stream)
{
  if(stream.GetMode() == SerializerMode::Saving)
  {
    forRange(MaterialBlockHandle block, mMaterialBlocks.All())
    {
      stream.StartPolymorphic(block.StoredType);
      block->Serialize(stream);
      stream.EndPolymorphic();
    }
  }
  else
  {
    PolymorphicNode materialNode;
    BoundType* materialBlockType = ZilchTypeId(MaterialBlock);
    while(stream.GetPolymorphic(materialNode))
    {
      MaterialBlockHandle block;

      BoundType* materialNodeType = MetaDatabase::FindType(materialNode.TypeName);

      // Notify them of data loss if we cannot proxy
      if(materialNodeType && materialNodeType->IsA(materialBlockType) == false)
      {
        String message = String::Format("A Fragment with the type name %s could not be proxied "
          "because a type with that name already exists. Data will be lost where this Fragment is "
          "used if the project is saved", materialNode.TypeName.mBegin);
        DoNotifyError("Could not proxy Material Fragment", message);
        stream.EndPolymorphic();
        continue;
      }

      // If the block couldn't be created then we need to create a proxy meta for it
      // (for zilch fragments that fail to compile and because resources can be loaded before compiling)
      if(materialNodeType == nullptr)
      {
        // Create a proxy component
        BoundType* proxyType = ProxyObject<MaterialBlock>::CreateProxyMetaFromFile(materialNode.TypeName, ProxyReason::TypeDidntExist);
        block = ZilchAllocate(MaterialBlock, proxyType);

        EngineLibraryExtensions::FindProxiedTypeOrigin(proxyType);
      }
      else
      {
        block = MaterialFactory::GetInstance()->MakeObject(materialNodeType);
      }

      if(block.IsNotNull())
      {
        // Must set owner before serialization, used by property setter
        block->mOwner = this;
        block->Serialize(stream);
        mMaterialBlocks.PushBack(block);
      }

      stream.EndPolymorphic();
    }
  }
}

void Material::Initialize()
{
  ConnectThisTo(&mSerializedList, Events::ResourceListItemAdded, OnResourceListItemAdded);
  ConnectThisTo(&mSerializedList, Events::ResourceListItemRemoved, OnResourceListItemRemoved);
}

void Material::Unload()
{
  mMaterialBlocks.Clear();
}

void Material::ReInitialize()
{
  TextSaver saver;
  saver.OpenBuffer();
  {
    saver.StartPolymorphic("Dummy");
    SerializeMaterialBlocks(saver);
    saver.EndPolymorphic();
  }
  
  // MaterialBlocks are reference counted, so we don't need to explicitly delete them
  mMaterialBlocks.Clear();

  DataTreeLoader loader;
  Status status;
  loader.OpenBuffer(status, saver.GetString());
  {
    PolymorphicNode dummyNode;
    loader.GetPolymorphic(dummyNode);

    SerializeMaterialBlocks(loader);
  }
}

void Material::ResourceModified()
{
  mPropertiesChanged = true;
}

HandleOf<Material> Material::RuntimeClone()
{
  HandleOf<Material> resourceHandle = DataResource::Clone();
  Material* resource = resourceHandle;

  ReturnIf(resource == nullptr, nullptr, "Failed to clone the material, returning null");

  resource->mCompositeName = mCompositeName;

  // Add the runtime list to the clone's list so that library resources don't have to be modified
  // and all effective connections are preserved
  resource->mSerializedList.mResourceIdNames.Append(mReferencedByList.mResourceIdNames.All());

  ResourceEvent event(mManager, resource);
  mManager->DispatchEvent(Events::ResourceAdded, &event);
  Z::gResources->DispatchEvent(Events::ResourcesLoaded, &event);

  return resourceHandle;
}

uint Material::GetSize() const
{
  return mMaterialBlocks.Size();
}

MaterialBlockHandle Material::GetBlockAt(uint index)
{
  if (index >= mMaterialBlocks.Size())
    return nullptr;
  else
    return mMaterialBlocks[index];
}

MaterialBlockHandle Material::GetById(BoundType* typeId)
{
  for (uint i = 0; i < mMaterialBlocks.Size(); ++i)
  {
    if (mMaterialBlocks[i].StoredType == typeId)
      return mMaterialBlocks[i];
  }

  return nullptr;
}

uint Material::GetBlockIndex(BoundType* typeId)
{
  for (uint i = 0; i < mMaterialBlocks.Size(); ++i)
  {
    if (mMaterialBlocks[i].StoredType == typeId)
      return i;
  }

  return (uint)-1;
}

void Material::Add(MaterialBlockHandle blockHandle, int index)
{
  MaterialBlock* block = blockHandle.Get<MaterialBlock*>();
  ReturnIf(block == nullptr, , "Null block given.");

  if (index < 0 || index > (int)mMaterialBlocks.Size())
    mMaterialBlocks.PushBack(block);
  else
    mMaterialBlocks.InsertAt(index, HandleOf<MaterialBlock>(block));

  block->mOwner = this;
  mCompositionChanged = true;
  SendModified();
}

bool Material::Remove(MaterialBlockHandle block)
{
  uint index = mMaterialBlocks.FindIndex(block);
  if (index >= mMaterialBlocks.Size())
    return false;

  mMaterialBlocks.EraseAt(index);
  mCompositionChanged = true;
  SendModified();
  return true;
}

void Material::OnResourceListItemAdded(ResourceListEvent* event)
{
  RenderGroup* renderGroup = RenderGroupManager::FindOrNull(event->mResourceIdName);
  if (renderGroup != nullptr)
    ResourceListEntryAdded(this, renderGroup);
}

void Material::OnResourceListItemRemoved(ResourceListEvent* event)
{
  RenderGroup* renderGroup = RenderGroupManager::FindOrNull(event->mResourceIdName);
  if (renderGroup != nullptr)
    ResourceListEntryRemoved(this, renderGroup);
}

IndexRange Material::AddShaderInputs(Array<ShaderInput>& shaderInputs, uint version)
{
  if (mPropertiesChanged == false && mInputRangeVersion == version)
    return mCachedInputRange;

  mCachedInputRange.start = shaderInputs.Size();

  forRange (MaterialBlock* block, mMaterialBlocks.All())
    block->AddShaderInputs(shaderInputs);

  mCachedInputRange.end = shaderInputs.Size();

  mPropertiesChanged = false;
  mInputRangeVersion = version;
  return mCachedInputRange;
}

void Material::UpdateCompositeName()
{
  MaterialFactory* factory = MaterialFactory::GetInstance();
  StringBuilder compositeNameBuilder;
  mFragmentNames.Clear();

  bool hasGeometry = false;

  // Material fragments
  forRange (MaterialBlockHandle block, mMaterialBlocks.All())
  {
    // Check if fragment had a restricted attribute added
    if (factory->mRestrictedComponents.Contains(block.StoredType))
    {
      String warning = String::Format("Fragment '%s' cannot be added to Materials and will have no effect on '%s'.",
        block.StoredType->Name.c_str(), Name.c_str());
      DoNotifyWarning("Restricted Fragment", warning);
      continue;
    }

    // Check for multiple geometry fragments
    if (factory->mGeometryComponents.Contains(block.StoredType))
    {
      if (hasGeometry)
      {
        String warning = String::Format("Geometry Fragment '%s' cannot be added to Material '%s', a Geometry Fragment is already present.",
          block.StoredType->Name.c_str(), Name.c_str());
        DoNotifyWarning("Multiple Geometry Fragments", warning);
        continue;
      }
      else
      {
        // Set true on first one found
        hasGeometry = true;
      }
    }

    if (block.StoredType->HasAttribute(ObjectAttributes::cProxy))
      continue;

    String fragmentName = block.StoredType->Name;
    compositeNameBuilder.Append(fragmentName);
    mFragmentNames.PushBack(fragmentName);
  }

  mCompositeName = compositeNameBuilder.ToString();
}

ImplementResourceManager(MaterialManager, Material);

MaterialManager::MaterialManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  AddLoader("Material", new ObjectInheritanceFileLoader<MaterialManager>());

  mCategory = "Graphics";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.Material.data"));

  DefaultResourceName = "ZeroMaterial";
  mCanCreateNew = true;
  mCanDuplicate = true;
  mCanReload = false;
  mExtension = DataResourceExtension;
}

void MaterialManager::ResourceDuplicated(Resource* resource, Resource* duplicate)
{
  Material* material = (Material*)resource;
  Material* dupMaterial = (Material*)duplicate;

  // If no RenderGroups reference the duplicated Material then nothing needs to be done
  if (material->mReferencedByList.mResourceIdNames.Empty())
    return;

  // If any resources are not writable then they are added to the duplicate instead
  // so the content item needs to be re-saved
  forRange (StringParam resourceIdName, material->mReferencedByList.All())
  {
    RenderGroup* renderGroup = RenderGroupManager::FindOrNull(resourceIdName);

    // Do not add duplicate to runtime resources
    if (renderGroup != nullptr && renderGroup->IsRuntime() == false)
    {
      if (renderGroup->IsWritable())
      {
        renderGroup->mSerializedList.AddResource(dupMaterial->ResourceIdName);
        if (Z::gRuntimeEditor != nullptr)
          MetaOperations::NotifyObjectModified(renderGroup);
      }
      else if (dupMaterial->mSerializedList.mResourceIdNames.Contains(resourceIdName) == false)
      {
        dupMaterial->mSerializedList.AddResource(resourceIdName);
      }
    }
  }

  dupMaterial->mContentItem->SaveContent();
}

} // namespace Zero
