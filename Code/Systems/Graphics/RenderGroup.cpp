// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(RenderGroup, builder, type)
{
  RaverieBindDocumented();

  RaverieBindMethod(IsSubRenderGroup);
  RaverieBindMethod(IsSubRenderGroupOf);

  RaverieBindFieldGetterPropertyAs(mSerializedList, "Materials");
  RaverieBindFieldGetterProperty(mReferencedByList);

  RaverieBindFieldProperty(mGraphicalSortMethod);

  MetaEditorResource* metaEditor = new MetaEditorResource(true, true, String(), false, false);
  metaEditor->Filter = ParentRenderGroupFilter;
  RaverieBindGetterSetterProperty(ParentRenderGroup)->Add(metaEditor);
  RaverieBindFieldGetterProperty(mChildRenderGroups);
}

RenderGroup::RenderGroup() : mSerializedList(this), mReferencedByList(this), mChildRenderGroups(this), mParentInternal(nullptr), mSortId(-1)
{
  mSerializedList.mDisplayName = "Materials";
  mReferencedByList.mDisplayName = "ReferencedBy";
  mReferencedByList.mReadOnly = true;

  mSerializedList.mExpanded = &GraphicsResourceList::mRenderGroupSerializedExpanded;
  mReferencedByList.mExpanded = &GraphicsResourceList::mRenderGroupRuntimeExpanded;

  mChildRenderGroups.mDisplayName = "ChildRenderGroups";
  mChildRenderGroups.mExpanded = &GraphicsResourceList::mChildRenderGroupListExpanded;
  mChildRenderGroups.mListItemCallback = ListItemValidChild;

  ConnectThisTo(&mSerializedList, Events::ResourceListItemAdded, OnResourceListItemAdded);
  ConnectThisTo(&mSerializedList, Events::ResourceListItemRemoved, OnResourceListItemRemoved);

  ConnectThisTo(&mChildRenderGroups, Events::ResourceListItemAdded, OnChildListItemAdded);
  ConnectThisTo(&mChildRenderGroups, Events::ResourceListItemRemoved, OnChildListItemRemoved);

  ConnectThisTo(this, Events::ObjectModified, OnObjectModified);
}

void RenderGroup::Serialize(Serializer& stream)
{
  stream.SerializeFieldDefault("Materials", mSerializedList.mResourceIdNames, Array<String>());

  SerializeEnumNameDefault(GraphicalSortMethod, mGraphicalSortMethod, GraphicalSortMethod::None);
  SerializeNameDefault(mParentRenderGroup, String());

  stream.SerializeFieldDefault("ChildRenderGroups", mChildRenderGroups.mResourceIdNames, Array<String>());
}

void RenderGroup::OnResourceListItemAdded(ResourceListEvent* event)
{
  Material* material = MaterialManager::FindOrNull(event->mResourceIdName);
  if (material != nullptr)
    ResourceListEntryAdded(this, material);
}

void RenderGroup::OnResourceListItemRemoved(ResourceListEvent* event)
{
  Material* material = MaterialManager::FindOrNull(event->mResourceIdName);
  if (material != nullptr)
    ResourceListEntryRemoved(this, material);
}

void RenderGroup::OnChildListItemAdded(ResourceListEvent* event)
{
  RenderGroup* renderGroup = RenderGroupManager::Instance->FindOrNull(event->mResourceIdName);
  if (renderGroup != nullptr && !IsSubRenderGroupOf(renderGroup))
    renderGroup->SetParentInternal(this);
}

void RenderGroup::OnChildListItemRemoved(ResourceListEvent* event)
{
  RenderGroup* renderGroup = RenderGroupManager::Instance->FindOrNull(event->mResourceIdName);
  if (renderGroup != nullptr && IsSubRenderGroup(renderGroup))
    renderGroup->SetParentInternal(nullptr);
}

void RenderGroup::OnObjectModified(ObjectEvent* event)
{
  SendModified();
}

void RenderGroup::GetMaterials(HashSet<Material*>& materials)
{
  Array<RenderGroup*> renderGroups;
  GetRenderGroups(renderGroups);

  forRange (RenderGroup* renderGroup, renderGroups.All())
    materials.Append(renderGroup->mActiveResources.All());
}

void RenderGroup::GetRenderGroups(Array<RenderGroup*>& renderGroups)
{
  renderGroups.PushBack(this);
  forRange (RenderGroup* childGroup, mChildrenInternal.All())
    childGroup->GetRenderGroups(renderGroups);
}

bool RenderGroup::IsSubRenderGroup(RenderGroup* renderGroup)
{
  while (renderGroup != nullptr)
  {
    if (renderGroup == this)
      return true;

    renderGroup = renderGroup->GetParentRenderGroup();
  }

  return false;
}

bool RenderGroup::IsSubRenderGroupOf(RenderGroup* renderGroup)
{
  if (renderGroup == nullptr)
    return false;

  return renderGroup->IsSubRenderGroup(this);
}

HandleOf<RenderGroup> RenderGroup::GetParentRenderGroup()
{
  return mParentInternal;
}

void RenderGroup::SetParentRenderGroup(HandleOf<RenderGroup> renderGroup)
{
  // If parent is established by this being in its child list, do not allow
  // changing this property. This limitation is for simplicity's sake in
  // managing the RenderGroup hierarchies.
  if (mParentInternal != nullptr && mParentInternal->mChildRenderGroups.mResourceIdNames.Contains(ResourceIdName))
    return DoNotifyWarning("Cannot modify parent",
                           "RenderGroup is in the child list of the other and "
                           "can only be removed from there.");

  // Null will not be a sub group and is valid to set as the parent.
  RenderGroup* group = renderGroup;
  if (!IsSubRenderGroup(group))
  {
    mParentRenderGroup = group != nullptr ? group->ResourceIdName : String();
    SetParentInternal(group);
  }
}

void RenderGroup::SetParentInternal(RenderGroup* renderGroup)
{
  // This method should never modify any of the serialized data,
  // any cases where that is needed must be handled independently.

  if (renderGroup == mParentInternal)
    return;

  // Unhook from previous parent.
  if (mParentInternal != nullptr)
  {
    mParentInternal->mChildrenInternal.EraseValue(this);
    mParentInternal = nullptr;
  }

  // Hook to new parent.
  mParentInternal = renderGroup;
  if (mParentInternal != nullptr)
    mParentInternal->mChildrenInternal.PushBack(this);
}

ImplementResourceManager(RenderGroupManager, RenderGroup);

RenderGroupManager::RenderGroupManager(BoundType* resourceType) : ResourceManager(resourceType)
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

bool ParentRenderGroupFilter(HandleParam object, Property* property, HandleParam result, Status& status)
{
  RenderGroup* owner = object.Get<RenderGroup*>();
  RenderGroup* renderGroup = result.Get<RenderGroup*>();

  if (renderGroup == owner)
    status.SetFailed("RenderGroup cannot be a parent of itself.");
  else if (owner != nullptr && owner->IsSubRenderGroup(renderGroup))
    status.SetFailed("RenderGroup is already a sub RenderGroup.");

  return true;
}

void ListItemValidChild(GraphicsResourceList* resourceList, String entryIdName, Status& status)
{
  RenderGroup* owner = (RenderGroup*)resourceList->mOwner;
  RenderGroup* renderGroup = RenderGroupManager::Instance->FindOrNull(entryIdName);
  if (!owner->mChildrenInternal.Contains(renderGroup))
    status.SetFailed("RenderGroup cannot be a child with the current configuration.");
}

} // namespace Raverie
