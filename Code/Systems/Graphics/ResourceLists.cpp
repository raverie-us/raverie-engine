// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
DefineEvent(ResourceListItemAdded);
DefineEvent(ResourceListItemRemoved);
} // namespace Events

RaverieDefineType(ResourceListEvent, builder, type)
{
  type->AddAttribute(ObjectAttributes::cHidden);
}

bool GraphicsResourceList::mMaterialSerializedExpanded = false;
bool GraphicsResourceList::mMaterialRuntimeExpanded = false;
bool GraphicsResourceList::mRenderGroupSerializedExpanded = false;
bool GraphicsResourceList::mRenderGroupRuntimeExpanded = false;
bool GraphicsResourceList::mChildRenderGroupListExpanded = false;

GraphicsResourceList::GraphicsResourceList(Resource* owner) : mReadOnly(false), mOwner(owner), mExpanded(nullptr), mListItemCallback(nullptr)
{
}

GraphicsResourceList::GraphicsResourceList(const GraphicsResourceList& rhs)
{
  mResourceIdNames = rhs.mResourceIdNames;
}

GraphicsResourceList& GraphicsResourceList::operator=(const GraphicsResourceList& graphicsResourceList)
{
  mResourceIdNames = graphicsResourceList.mResourceIdNames;
  return *this;
}

Array<String>::range GraphicsResourceList::GetIdNames()
{
  return mResourceIdNames.All();
}

uint GraphicsResourceList::GetResourceIndex(StringParam resourceIdName)
{
  return mResourceIdNames.FindIndex(resourceIdName);
}

void GraphicsResourceList::CheckForAddition(Status& status, Resource* resource)
{
  // If id part is not up to date this will query incorrectly
  if (mResourceIdNames.Contains(resource->ResourceIdName))
    status.SetFailed("Resource already added.");
}

void GraphicsResourceList::AddResource(StringParam resourceIdName, uint index)
{
  if (index <= mResourceIdNames.Size())
    mResourceIdNames.InsertAt(index, resourceIdName);
  else
    mResourceIdNames.PushBack(resourceIdName);

  ResourceListEvent event;
  event.mResourceIdName = resourceIdName;
  DispatchEvent(Events::ResourceListItemAdded, &event);
}

void GraphicsResourceList::RemoveResource(StringParam resourceIdName)
{
  if (mResourceIdNames.EraseValue(resourceIdName))
  {
    ResourceListEvent event;
    event.mResourceIdName = resourceIdName;
    DispatchEvent(Events::ResourceListItemRemoved, &event);
  }
}

RaverieDefineType(RenderGroupList, builder, type)
{
  RaverieBindDocumented();

  RaverieBindMethod(Add);
  RaverieBindMethod(Remove);
  RaverieBindMethod(All);
}

RenderGroupList::RenderGroupList(Resource* owner) : GraphicsResourceList(owner)
{
}

String RenderGroupList::GetResourceTypeName()
{
  return "RenderGroup";
}

void RenderGroupList::Add(RenderGroup& renderGroup)
{
  if (mReadOnly)
    return DoNotifyException("Error", "Cannot modify ReferencedBy list.");

  if (!mOwner->IsRuntime())
    return DoNotifyException("Error", "Cannot modify non-runtime resource.");

  Status status;
  CheckForAddition(status, &renderGroup);
  if (status.Failed())
    return DoNotifyException("Error", status.Message);

  AddResource(renderGroup.ResourceIdName);
}

void RenderGroupList::Remove(RenderGroup& renderGroup)
{
  if (mReadOnly)
    return DoNotifyException("Error", "Cannot modify ReferencedBy list.");

  if (!mOwner->IsRuntime())
    return DoNotifyException("Error", "Cannot modify non-runtime resource.");

  if (!mResourceIdNames.Contains(renderGroup.ResourceIdName))
    return;

  RemoveResource(renderGroup.ResourceIdName);
}

Array<HandleOf<RenderGroup>> RenderGroupList::All()
{
  Array<HandleOf<RenderGroup>> resources;
  forRange (String idName, mResourceIdNames.All())
  {
    RenderGroup* renderGroup = RenderGroupManager::Instance->FindOrNull(idName);
    if (renderGroup != nullptr)
      resources.PushBack(renderGroup);
  }

  return resources;
}

RaverieDefineType(ChildRenderGroupList, builder, type)
{
}

ChildRenderGroupList::ChildRenderGroupList(Resource* owner) : RenderGroupList(owner)
{
}

void ChildRenderGroupList::CheckForAddition(Status& status, Resource* resource)
{
  GraphicsResourceList::CheckForAddition(status, resource);
  if (status.Failed())
    return;

  RenderGroup* owner = (RenderGroup*)mOwner;
  RenderGroup* renderGroup = (RenderGroup*)resource;

  // Self check.
  if (renderGroup == owner)
  {
    status.SetFailed("RenderGroup cannot be a parent of itself.");
    return;
  }

  // Already assigned a parent.
  if (RenderGroup* parentGroup = renderGroup->GetParentRenderGroup())
  {
    status.SetFailed(String::Format("RenderGroup is already a child of '%s'.", parentGroup->Name.c_str()));
    return;
  }

  // Self is already a sub group.
  if (owner->IsSubRenderGroupOf(renderGroup))
  {
    status.SetFailed("Cannot set a parent RenderGroup as a child.");
    return;
  }
}

RaverieDefineType(MaterialList, builder, type)
{
  RaverieBindDocumented();

  RaverieBindMethod(Add);
  RaverieBindMethod(Remove);
  RaverieBindMethod(All);
}

MaterialList::MaterialList(Resource* owner) : GraphicsResourceList(owner)
{
}

String MaterialList::GetResourceTypeName()
{
  return "Material";
}

void MaterialList::Add(Material& material)
{
  if (mReadOnly)
    return DoNotifyException("Error", "Cannot modify ReferencedBy list.");

  if (!mOwner->IsRuntime())
    return DoNotifyException("Error", "Cannot modify non-runtime resource.");

  Status status;
  CheckForAddition(status, &material);
  if (status.Failed())
    return DoNotifyException("Error", status.Message);

  AddResource(material.ResourceIdName);
}

void MaterialList::Remove(Material& material)
{
  if (mReadOnly)
    return DoNotifyException("Error", "Cannot modify ReferencedBy list.");

  if (!mOwner->IsRuntime())
    return DoNotifyException("Error", "Cannot modify non-runtime resource.");

  if (!mResourceIdNames.Contains(material.ResourceIdName))
    return;

  RemoveResource(material.ResourceIdName);
}

Array<HandleOf<Material>> MaterialList::All()
{
  Array<HandleOf<Material>> resources;
  forRange (String idName, mResourceIdNames.All())
  {
    Material* material = MaterialManager::Instance->FindOrNull(idName);
    if (material != nullptr)
      resources.PushBack(material);
  }

  return resources;
}

void ResourceListAdd(Material* material)
{
  forRange (String& resourceIdName, material->mSerializedList.GetIdNames())
  {
    RenderGroup* renderGroup = RenderGroupManager::FindOrNull(resourceIdName);
    if (renderGroup != nullptr)
      ResourceListResourceAdded(material, renderGroup, &resourceIdName);
  }
}

void ResourceListAdd(RenderGroup* renderGroup)
{
  forRange (String& resourceIdName, renderGroup->mSerializedList.GetIdNames())
  {
    Material* material = MaterialManager::FindOrNull(resourceIdName);
    if (material != nullptr)
      ResourceListResourceAdded(renderGroup, material, &resourceIdName);
  }
}

void ResourceListRemove(Material* material)
{
  forRange (RenderGroup* renderGroup, material->mActiveResources.All())
  {
    if (renderGroup != nullptr)
      ResourceListResourceRemoved(material, renderGroup);
  }
  material->mActiveResources.Clear();
}

void ResourceListRemove(RenderGroup* renderGroup)
{
  forRange (Material* material, renderGroup->mActiveResources.All())
  {
    if (material != nullptr)
      ResourceListResourceRemoved(renderGroup, material);
  }
  renderGroup->mActiveResources.Clear();
}

void ResourceListResolveReferences(Material* material)
{
  // Find any entries in serialized list that previously didn't exist
  forRange (String& resourceIdName, material->mSerializedList.GetIdNames())
  {
    RenderGroup* renderGroup = RenderGroupManager::FindOrNull(resourceIdName);
    if (renderGroup != nullptr)
    {
      // If this resource is not in the runtime list of the found resource then
      // this is a newly added resource
      if (renderGroup->mReferencedByList.mResourceIdNames.Contains(material->ResourceIdName) == false)
        ResourceListResourceAdded(material, renderGroup, &resourceIdName);
    }
  }
}

void ResourceListResolveReferences(RenderGroup* renderGroup)
{
  // Find any entries in serialized list that previously didn't exist
  forRange (String& resourceIdName, renderGroup->mSerializedList.GetIdNames())
  {
    Material* material = MaterialManager::FindOrNull(resourceIdName);
    if (material != nullptr)
    {
      // If this resource is not in the runtime list of the found resource then
      // this is a newly added resource
      if (material->mReferencedByList.mResourceIdNames.Contains(renderGroup->ResourceIdName) == false)
        ResourceListResourceAdded(renderGroup, material, &resourceIdName);
    }
  }
}

void ResolveRenderGroupHierarchy(RenderGroup* renderGroup)
{
  // For valid entries the idName is reassigned to resolve resource renames or
  // new resources matching an old entry by name. Also sets internal pointers in
  // case it's newly resolved. Redundantly setting is okay.

  // Resolve parent if valid.
  RenderGroup* parentGroup = RenderGroupManager::Instance->FindOrNull(renderGroup->mParentRenderGroup);
  if (parentGroup != nullptr && !renderGroup->IsSubRenderGroup(parentGroup))
  {
    renderGroup->mParentRenderGroup = parentGroup->ResourceIdName;
    renderGroup->SetParentInternal(parentGroup);
  }

  for (size_t i = 0; i < renderGroup->mChildRenderGroups.mResourceIdNames.Size(); ++i)
  {
    // Resolve child if valid.
    String& idName = renderGroup->mChildRenderGroups.mResourceIdNames[i];
    RenderGroup* childGroup = RenderGroupManager::Instance->FindOrNull(idName);
    if (childGroup != nullptr && !renderGroup->IsSubRenderGroupOf(childGroup))
    {
      idName = childGroup->ResourceIdName;
      childGroup->SetParentInternal(renderGroup);
    }
  }
}

void ResolveRenderGroupHierarchies()
{
  Array<Resource*> renderGroups;
  RenderGroupManager::Instance->EnumerateResources(renderGroups);
  forRange (Resource* resource, renderGroups.All())
  {
    RenderGroup* group = (RenderGroup*)resource;
    ResolveRenderGroupHierarchy(group);
  }
}

} // namespace Raverie
