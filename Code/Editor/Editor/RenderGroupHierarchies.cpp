// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Zero
{

RenderGroupSource::RenderGroupSource()
{
  mOperationQueue = Z::gEditor->GetOperationQueue();
}

DataEntry* RenderGroupSource::GetRoot()
{
  return this;
}

DataEntry* RenderGroupSource::ToEntry(DataIndex index)
{
  if (index.Id == 0)
    return this;
  else
    return RenderGroupManager::Instance->GetResource(index.Id, ResourceNotFound::ReturnNull);
}

DataIndex RenderGroupSource::ToIndex(DataEntry* dataEntry)
{
  if (dataEntry == this)
  {
    return DataIndex(0);
  }
  else
  {
    RenderGroup* renderGroup = (RenderGroup*)dataEntry;
    return DataIndex((u64)renderGroup->mResourceId);
  }
}

DataEntry* RenderGroupSource::Parent(DataEntry* dataEntry)
{
  if (dataEntry == this)
    return nullptr;

  // This class acts as the parent for all root RenderGroups for the tree
  // interface.
  RenderGroup* renderGroup = (RenderGroup*)dataEntry;
  if (renderGroup->mParentInternal != nullptr)
    return renderGroup->mParentInternal;
  else
    return this;
}

uint RenderGroupSource::ChildCount(DataEntry* dataEntry)
{
  if (dataEntry == this)
  {
    mRootGroups.Clear();
    Array<Resource*> resources;
    RenderGroupManager::Instance->EnumerateResources(resources);
    forRange (Resource* resource, resources.All())
    {
      // Get all child groups of this object (root RenderGroups with no parent).
      RenderGroup* renderGroup = (RenderGroup*)resource;
      if (renderGroup->mParentInternal == nullptr)
        mRootGroups.PushBack(renderGroup);
    }
    return mRootGroups.Size();
  }

  RenderGroup* renderGroup = (RenderGroup*)dataEntry;
  return renderGroup->mChildrenInternal.Size();
}

DataEntry* RenderGroupSource::GetChild(DataEntry* dataEntry, uint index, DataEntry* prev)
{
  if (dataEntry == this)
  {
    if (index < mRootGroups.Size())
      return mRootGroups[index];
    else
      return nullptr;
  }

  RenderGroup* renderGroup = (RenderGroup*)dataEntry;
  if (index < renderGroup->mChildrenInternal.Size())
    return renderGroup->mChildrenInternal[index];
  else
    return nullptr;
}

bool RenderGroupSource::IsExpandable(DataEntry* dataEntry)
{
  if (dataEntry == this)
    return true;

  RenderGroup* renderGroup = (RenderGroup*)dataEntry;
  return !renderGroup->mChildrenInternal.Empty();
}

void RenderGroupSource::GetData(DataEntry* dataEntry, Any& variant, StringParam column)
{
  if (dataEntry == this)
    return;

  if (column == "Name")
  {
    RenderGroup* renderGroup = (RenderGroup*)dataEntry;
    variant = renderGroup->Name;
  }
}

bool RenderGroupSource::SetData(DataEntry* dataEntry, const Any& variant, StringParam column)
{
  return false;
}

void RenderGroupSource::CanMove(Status& status, DataEntry* source, DataEntry* destination, InsertMode::Type insertMode)
{
  // Do nothing if a data entry is null.
  if (destination == nullptr || source == nullptr)
  {
    status.Context = InsertError::NotSupported;
    return;
  }

  // No rearranging supported because it doesn't mean anything.
  if (insertMode != InsertMode::On)
  {
    status.Context = InsertError::NotSupported;
    return;
  }

  RenderGroup* moving = (RenderGroup*)source;

  // If destination is this data source object then the operation is removing
  // the assigned parent. Do nothing if no parent to remove.
  if (destination == this && moving->mParentInternal == nullptr)
  {
    status.Context = InsertError::NotSupported;
    return;
  }

  // Validate if parent can be modified.
  if (moving->mParentInternal != nullptr)
  {
    bool inParentsChildList = InParentsChildList(moving);
    // If it's the parent property but it's read only.
    if (!inParentsChildList && !moving->IsWritable())
    {
      status.SetFailed("Resource is read only, cannot modify parent.");
      status.Context = InsertError::Invalid;
      return;
    }
    // If it's the child list but parent is read only.
    else if (inParentsChildList && !moving->mParentInternal->IsWritable())
    {
      status.SetFailed("Parent is read only, cannot remove from child list.");
      status.Context = InsertError::Invalid;
      return;
    }
  }

  if (destination == this)
  {
    status.Message = "Remove parent.";
  }
  else
  {
    RenderGroup* dest = (RenderGroup*)destination;

    // Cannot assign a parent if both resources are read only.
    // Have to be able to modify either a parent or child list properties.
    if (!moving->IsWritable() && !dest->IsWritable())
    {
      status.SetFailed("Both Resources are read only.");
      status.Context = InsertError::Invalid;
      return;
    }

    // Don't allowing moving to a child RenderGroup.
    if (moving->IsSubRenderGroup(dest))
    {
      status.SetFailed("Cannot assign a child as parent.");
      status.Context = InsertError::Invalid;
      return;
    }

    // Don't reassign same parent.
    if ((RenderGroup*)moving->GetParentRenderGroup() == dest)
    {
      status.SetFailed("Already parent.");
      status.Context = InsertError::Invalid;
      return;
    }

    status.Message = String::Format("Assign '%s' as parent of '%s'.", dest->Name.c_str(), moving->Name.c_str());
  }
}

void RenderGroupSource::BeginBatchMove()
{
  mOperationQueue->BeginBatch("RenderGroup_BatchMove");
}

bool RenderGroupSource::Move(DataEntry* destinationEntry, DataEntry* movingEntry, InsertMode::Type insertMode)
{
  RenderGroup* moving = (RenderGroup*)movingEntry;

  // Removing the assigned parent when destination is this object.
  if (destinationEntry == this)
  {
    mOperationQueue->BeginBatch("RenderGroup_Move");

    RemoveFromParent(moving);
    ChangeAndQueueProperty(mOperationQueue, moving, PropertyPath("ParentRenderGroup"), Any(ZilchTypeId(RenderGroup)));

    mOperationQueue->EndBatch();
  }
  else
  {
    RenderGroup* dest = (RenderGroup*)destinationEntry;

    // If the moving RenderGroup is writable then use the parent property to
    // assign relationship.
    if (moving->IsWritable())
    {
      mOperationQueue->BeginBatch("RenderGroup_Move");

      RemoveFromParent(moving);
      ChangeAndQueueProperty(mOperationQueue, moving, PropertyPath("ParentRenderGroup"), dest);

      mOperationQueue->EndBatch();
    }
    // Otherwise use the child list property of the other RenderGroup.
    else
    {
      mOperationQueue->BeginBatch("RenderGroup_Move");

      RemoveFromParent(moving);
      Operation* operation =
          new ResourceListOperation<RenderGroupList>(dest->mChildRenderGroups, moving->ResourceIdName);
      mOperationQueue->Queue(operation);
      operation->Redo();

      mOperationQueue->EndBatch();
    }
  }

  return true;
}

void RenderGroupSource::EndBatchMove()
{
  mOperationQueue->EndBatch();
}

bool RenderGroupSource::InParentsChildList(RenderGroup* renderGroup)
{
  if (renderGroup->mParentInternal != nullptr)
    return renderGroup->mParentInternal->mChildRenderGroups.mResourceIdNames.Contains(renderGroup->ResourceIdName);

  return false;
}

void RenderGroupSource::RemoveFromParent(RenderGroup* renderGroup)
{
  if (InParentsChildList(renderGroup))
  {
    Operation* operation = new ResourceListOperation<RenderGroupList>(
        renderGroup->mParentInternal->mChildRenderGroups, renderGroup->ResourceIdName, -1, false);
    mOperationQueue->Queue(operation);
    operation->Redo();
  }
}

ZilchDefineType(RenderGroupHierarchies, builder, type)
{
}

RenderGroupHierarchies::RenderGroupHierarchies(Composite* parent) : Composite(parent)
{
  SetLayout(CreateStackLayout());

  mTree = new TreeView(this);
  mTree->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);

  TreeFormatting formatting;
  ColumnFormat column;
  column.Name = "Name";
  column.ColumnType = ColumnType::Flex;
  formatting.Columns.PushBack(column);
  mTree->SetFormat(formatting);

  mSource = new RenderGroupSource();
  mTree->SetDataSource(mSource);

  ConnectThisTo(RenderGroupManager::GetInstance(), Events::ResourceAdded, OnResourceModified);
  ConnectThisTo(RenderGroupManager::GetInstance(), Events::ResourceModified, OnResourceModified);
  ConnectThisTo(RenderGroupManager::GetInstance(), Events::ResourceRemoved, OnResourceModified);
}

RenderGroupHierarchies::~RenderGroupHierarchies()
{
  SafeDelete(mSource);
}

void RenderGroupHierarchies::OnResourceModified(ResourceEvent* event)
{
  mTree->SetDataSource(mSource);
}

} // namespace Zero
