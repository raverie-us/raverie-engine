// Authors: Nathan Carlson
// Copyright 2016, DigiPen Institute of Technology

#include "Precompiled.hpp"

namespace Zero
{

//**************************************************************************************************
ZilchDefineType(RenderGroup, builder, type)
{
  ZeroBindDocumented();

  ZilchBindFieldGetterPropertyAs(mSerializedList, "Materials");
  ZilchBindFieldGetterProperty(mReferencedByList);

  ZilchBindFieldProperty(mGraphicalSortMethod);
}

//**************************************************************************************************
RenderGroup::RenderGroup()
  : mSerializedList(this)
  , mReferencedByList(this)
  , mSortId(-1)
{
  mSerializedList.mDisplayName = "Materials";
  mReferencedByList.mDisplayName = "ReferencedBy";
  mReferencedByList.mReadOnly = true;

  mSerializedList.mExpanded = &GraphicsResourceList::mRenderGroupSerializedExpanded;
  mReferencedByList.mExpanded = &GraphicsResourceList::mRenderGroupRuntimeExpanded;

  ConnectThisTo(&mSerializedList, Events::ResourceListItemAdded, OnResourceListItemAdded);
  ConnectThisTo(&mSerializedList, Events::ResourceListItemRemoved, OnResourceListItemRemoved);
}

//**************************************************************************************************
void RenderGroup::Serialize(Serializer& stream)
{
  stream.SerializeFieldDefault("Materials", mSerializedList.mResourceIdNames, Array<String>());

  SerializeEnumNameDefault(GraphicalSortMethod, mGraphicalSortMethod, GraphicalSortMethod::None);
}

//**************************************************************************************************
void RenderGroup::OnResourceListItemAdded(ResourceListEvent* event)
{
  Material* material = MaterialManager::FindOrNull(event->mResourceIdName);
  if (material != nullptr)
    ResourceListEntryAdded(this, material);
}

//**************************************************************************************************
void RenderGroup::OnResourceListItemRemoved(ResourceListEvent* event)
{
  Material* material = MaterialManager::FindOrNull(event->mResourceIdName);
  if (material != nullptr)
    ResourceListEntryRemoved(this, material);
}

ImplementResourceManager(RenderGroupManager, RenderGroup);

//**************************************************************************************************
RenderGroupManager::RenderGroupManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  AddLoader("RenderGroup", new TextDataFileLoader<RenderGroupManager>());

  DefaultResourceName = "Opaque";

  mCategory = "Graphics";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.RenderGroup.data"));

  mCanCreateNew = true;
  mNoFallbackNeeded = true;
  mExtension = DataResourceExtension;
}

} // namespace Zero
