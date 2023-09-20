// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{

bool Material::sNotifyModified = true;

RaverieDefineType(CompositionLabelExtension, builder, type)
{
}

RaverieDefineType(Material, builder, type)
{
  type->Add(new MaterialFactory());

  RaverieBindDocumented();

  RaverieBindMethod(RuntimeClone);

  RaverieBindFieldGetterPropertyAs(mSerializedList, "RenderGroups");
  RaverieBindFieldGetterProperty(mReferencedByList);

  RaverieBindGetterProperty(CompositionLabel)->Add(new CompositionLabelExtension());
}

Material::Material() :
    mRenderData(nullptr),
    mSerializedList(this),
    mReferencedByList(this),
    mCompositionChanged(false),
    mPropertiesChanged(true),
    mInputRangeVersion(-1)
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
  if (stream.GetMode() == SerializerMode::Saving)
  {
    forRange (MaterialBlockHandle block, mMaterialBlocks.All())
    {
      stream.StartPolymorphic(block);
      block->Serialize(stream);
      stream.EndPolymorphic();
    }
  }
  else
  {
    PolymorphicNode materialNode;
    BoundType* materialBlockType = RaverieTypeId(MaterialBlock);
    while (stream.GetPolymorphic(materialNode))
    {
      MaterialBlockHandle block;

      BoundType* materialNodeType = MetaDatabase::FindType(materialNode.TypeName);

      // Notify them of data loss if we cannot proxy
      if (materialNodeType && materialNodeType->IsA(materialBlockType) == false)
      {
        String message = String::Format("A Fragment with the type name %s could not be proxied "
                                        "because a type with that name already exists. Data will be lost "
                                        "where this Fragment is "
                                        "used if the project is saved",
                                        materialNode.TypeName.mBegin);
        DoNotifyError("Could not proxy Material Fragment", message);
        stream.EndPolymorphic();
        continue;
      }

      // If the block couldn't be created then we need to create a proxy meta
      // for it (for raverie fragments that fail to compile and because resources
      // can be loaded before compiling)
      if (materialNodeType == nullptr)
      {
        // Create a proxy component
        BoundType* proxyType =
            ProxyObject<MaterialBlock>::CreateProxyType(materialNode.TypeName, ProxyReason::TypeDidntExist);
        block = RaverieAllocate(MaterialBlock, proxyType);

        EngineLibraryExtensions::FindProxiedTypeOrigin(proxyType);
      }
      else
      {
        block = MaterialFactory::GetInstance()->MakeObject(materialNodeType);
      }

      if (block.IsNotNull())
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
  forRange (HandleOf<MaterialBlock> handle, mMaterialBlocks.All())
    handle.Delete();

  mMaterialBlocks.Clear();
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

  // Add the runtime list to the clone's list so that library resources don't
  // have to be modified and all effective connections are preserved
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
  if (sNotifyModified)
    SendModified();
}

bool Material::Remove(MaterialBlockHandle block)
{
  uint index = mMaterialBlocks.FindIndex(block);
  if (index >= mMaterialBlocks.Size())
    return false;

  mMaterialBlocks[index].Delete();
  mMaterialBlocks.EraseAt(index);
  mCompositionChanged = true;
  if (sNotifyModified)
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
      String warning = String::Format("Fragment '%s' cannot be added to Materials and will "
                                      "have no effect on '%s'.",
                                      block.StoredType->Name.c_str(),
                                      Name.c_str());
      DoNotifyWarning("Restricted Fragment", warning);
      continue;
    }

    // Check for multiple geometry fragments
    if (factory->mGeometryComponents.Contains(block.StoredType))
    {
      if (hasGeometry)
      {
        String warning = String::Format("Geometry Fragment '%s' cannot be added to Material "
                                        "'%s', a Geometry Fragment is already present.",
                                        block.StoredType->Name.c_str(),
                                        Name.c_str());
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

MaterialManager::MaterialManager(BoundType* resourceType) : ResourceManager(resourceType)
{
  AddLoader("Material", new ObjectInheritanceFileLoader<MaterialManager>());

  mCategory = "Graphics";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.Material.data"));

  DefaultResourceName = "RaverieMaterial";
  mCanCreateNew = true;
  mCanDuplicate = true;
  mCanReload = false;
  mExtension = DataResourceExtension;
}

void MaterialManager::ResourceDuplicated(Resource* resource, Resource* duplicate)
{
  Material* material = (Material*)resource;
  Material* dupMaterial = (Material*)duplicate;

  // If no RenderGroups reference the duplicated Material then nothing needs to
  // be done
  if (material->mReferencedByList.mResourceIdNames.Empty())
    return;

  // If any resources are not writable then they are added to the duplicate
  // instead so the content item needs to be re-saved
  forRange (StringParam resourceIdName, material->mReferencedByList.GetIdNames())
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

void MaterialManager::ReInitializeRemoveComponents()
{
  Material::sNotifyModified = false;

  mReInitializeQueue.BeginBatch();

  forRange (Resource* resource, AllResources())
  {
    Material* material = (Material*)resource;
    Material::MaterialBlockArray::range range = material->mMaterialBlocks.All();
    while (!range.Empty())
    {
      MaterialBlock* materialBlock = range.Back();
      BoundType* blockType = RaverieVirtualTypeId(materialBlock);

      AddRemoveComponentOperation* op =
          new AddRemoveComponentOperation(material, blockType, ComponentOperation::Remove);
      op->mNotifyModified = false;
      op->Redo();
      mReInitializeQueue.Queue(op);

      range.PopBack();
    }
  }

  mReInitializeQueue.EndBatch();
}

void MaterialManager::ReInitializeAddComponents()
{
  mReInitializeQueue.Undo();
  mReInitializeQueue.ClearAll();
  Material::sNotifyModified = true;
}

} // namespace Raverie
