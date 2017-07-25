#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(ResourceListItemAdded);
  DefineEvent(ResourceListItemRemoved);
}

ZilchDefineType(ResourceListEvent, builder, type)
{
  type->AddAttribute(ObjectAttributes::cHidden);
}

bool GraphicsResourceList::mMaterialSerializedExpanded = false;
bool GraphicsResourceList::mMaterialRuntimeExpanded = false;
bool GraphicsResourceList::mRenderGroupSerializedExpanded = false;
bool GraphicsResourceList::mRenderGroupRuntimeExpanded = false;

GraphicsResourceList::GraphicsResourceList(Resource* owner)
  : mReadOnly(false)
  , mExpanded(nullptr)
  , mOwner(owner)
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

Array<String>::range GraphicsResourceList::All()
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
    status.SetFailed("Resource already added");
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

ZilchDefineType(RenderGroupList, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
  type->AddAttribute(ObjectAttributes::cHidden);
}

RenderGroupList::RenderGroupList(Resource* owner)
  : GraphicsResourceList(owner)
{
}

String RenderGroupList::GetResourceTypeName()
{
  return "RenderGroup";
}

ZilchDefineType(MaterialList, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
  type->AddAttribute(ObjectAttributes::cHidden);
}

MaterialList::MaterialList(Resource* owner)
  : GraphicsResourceList(owner)
{
}

String MaterialList::GetResourceTypeName()
{
  return "Material";
}

void ResourceListAdd(Material* material)
{
  forRange (String& resourceIdName, material->mSerializedList.All())
  {
    RenderGroup* renderGroup = RenderGroupManager::FindOrNull(resourceIdName);
    if (renderGroup != nullptr)
      ResourceListResourceAdded(material, renderGroup, &resourceIdName);
  }
}

void ResourceListAdd(RenderGroup* renderGroup)
{
  forRange (String& resourceIdName, renderGroup->mSerializedList.All())
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
  forRange (String& resourceIdName, material->mSerializedList.All())
  {
    RenderGroup* renderGroup = RenderGroupManager::FindOrNull(resourceIdName);
    if (renderGroup != nullptr)
    {
      // If this resource is not in the runtime list of the found resource then this is a newly added resource
      if (renderGroup->mReferencedByList.mResourceIdNames.Contains(material->ResourceIdName) == false)
        ResourceListResourceAdded(material, renderGroup, &resourceIdName);
    }
  }
}

void ResourceListResolveReferences(RenderGroup* renderGroup)
{
  // Find any entries in serialized list that previously didn't exist
  forRange (String& resourceIdName, renderGroup->mSerializedList.All())
  {
    Material* material = MaterialManager::FindOrNull(resourceIdName);
    if (material != nullptr)
    {
      // If this resource is not in the runtime list of the found resource then this is a newly added resource
      if (material->mReferencedByList.mResourceIdNames.Contains(renderGroup->ResourceIdName) == false)
        ResourceListResourceAdded(renderGroup, material, &resourceIdName);
    }
  }
}

} // namespace Zero
