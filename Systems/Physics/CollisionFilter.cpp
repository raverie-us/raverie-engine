///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

String GroupFilterDisplay(CollisionFilter* filter)
{
  String typeAName = filter->GetTypeADisplayName();
  String typeBName = filter->GetTypeBDisplayName();
  return String::Format("Filter: (%s / %s)", 
    typeAName.c_str(), typeBName.c_str());
}

ZilchDefineType(CollisionFilter, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(CollisionFlag);
  ZilchBindGetterProperty(CollisionGroupA);
  ZilchBindGetterProperty(CollisionGroupB);
  type->Add(new CollisionFilterMetaComposition(true));
}

CollisionFilter::CollisionFilter(ResourceId first, ResourceId second)
{
  mFilterFlags.Clear();

  SetPair(first, second);
}

CollisionFilter::~CollisionFilter()
{
  DeleteObjectsInContainer(mBlocks);
}

void CollisionFilter::SetPair(ResourceId first, ResourceId second)
{
  if(first < second)
    mPair = ResourcePair(first, second);
  else
    mPair = ResourcePair(second, first);

  TypeA = first;
  TypeB = second;
}

void CollisionFilter::Serialize(Serializer& stream)
{
  // We're saving so save the resource's ids
  if(stream.GetMode() == SerializerMode::Saving)
  {
    HandleOf<CollisionGroup> group1;
    group1 = CollisionGroupManager::Find(TypeA);
    HandleOf<CollisionGroup> group2;
    group2 = CollisionGroupManager::Find(TypeB);
    SerializeResource("TypeA", group1, CollisionGroupManager);
    SerializeResource("TypeB", group2, CollisionGroupManager);
  }
  // We're loading so grab the resources ids
  else
  {
    HandleOf<CollisionGroup> group1;
    HandleOf<CollisionGroup> group2;
    SerializeResource("TypeA", group1, CollisionGroupManager);
    SerializeResource("TypeB", group2, CollisionGroupManager);

    SetPair(group1->mResourceId, group2->mResourceId);
  }

  // Now serialize the rest of the bit states and the override strings
  SerializeBits(stream, mFilterFlags, FilterFlags::Names, 0, 0);

  // Serialize our composition of constraint config blocks
  BoundType* selfBoundType = this->ZilchGetDerivedType();
  CollisionFilterMetaComposition* factory = selfBoundType->Has<CollisionFilterMetaComposition>();
  factory->SerializeArray(stream, mBlocks);
}

void CollisionFilter::SetDefaults()
{
  CollisionGroup* group = CollisionGroupManager::Find("DefaultGroup");
  SetPair(group->mResourceId, group->mResourceId);
}

CollisionFilterCollisionFlags::Enum CollisionFilter::GetCollisionFlag()
{  
  // Convert the bitfield to the corresponding enum value
  if(mFilterFlags.IsSet(FilterFlags::SkipResolution | FilterFlags::SkipDetectingCollision) == false)
    return CollisionFilterCollisionFlags::Resolve;
  else if(mFilterFlags.IsSet(FilterFlags::SkipDetectingCollision))
    return CollisionFilterCollisionFlags::SkipDetection;
  else
    return CollisionFilterCollisionFlags::SkipResolution;
}

void CollisionFilter::SetCollisionFlag(CollisionFilterCollisionFlags::Enum state)
{
  // Convert the enum to the corresponding bit field value
  mFilterFlags.ClearFlag(FilterFlags::SkipResolution | FilterFlags::SkipDetectingCollision);
  if(state == CollisionFilterCollisionFlags::SkipDetection)
    mFilterFlags.SetFlag(FilterFlags::SkipDetectingCollision);
  else if(state == CollisionFilterCollisionFlags::SkipResolution)
    mFilterFlags.SetFlag(FilterFlags::SkipResolution);

  // We've change the bitfield states, rebuild the cached data used during runtime
  mTable->ReconfigureGroups();
}

ResourceId CollisionFilter::first() const
{
  return mPair.first;
}

ResourceId CollisionFilter::second() const
{
  return mPair.second;
}

CollisionGroup* CollisionFilter::GetCollisionGroupA() const
{
  return CollisionGroupManager::Find(TypeA);
}

CollisionGroup* CollisionFilter::GetCollisionGroupB() const
{
  return CollisionGroupManager::Find(TypeB);
}

String CollisionFilter::GetTypeAName() const
{
  CollisionGroup* group = CollisionGroupManager::Find(TypeA);
  if(group != nullptr)
    return group->ResourceIdName;
  return String();
}

String CollisionFilter::GetTypeBName() const
{
  CollisionGroup* group = CollisionGroupManager::Find(TypeB);
  if(group != nullptr)
    return group->ResourceIdName;
  return String();
}

String CollisionFilter::GetTypeADisplayName() const
{
  CollisionGroup* group = CollisionGroupManager::Find(TypeA);
  if(group != nullptr)
    return group->Name;
  return String();
}

String CollisionFilter::GetTypeBDisplayName() const
{
  CollisionGroup* group = CollisionGroupManager::Find(TypeB);
  if(group != nullptr)
    return group->Name;
  return String();
}

CollisionFilter* CollisionFilter::Clone() const
{
  CollisionFilter* clone = new CollisionFilter(TypeA, TypeB);
  clone->mTable = nullptr;
  clone->mFilterFlags = mFilterFlags;

  // Copy all blocks
  for(size_t i = 0; i < mBlocks.Size(); ++i)
  {
    // Create a new copy of the same block type by getting the bound type then going
    // through the meta composition to allocate the block type
    BoundType* boundType = mBlocks[i]->ZilchGetDerivedType();
    CollisionFilterMetaComposition* metaComposition = boundType->HasInherited<CollisionFilterMetaComposition>();
    HandleOf<CollisionFilterBlock> newBlockHandle = metaComposition->AllocateBlock(boundType, false);
    CollisionFilterBlock* newBlock = newBlockHandle;
    // Deep copy the block
    *newBlock = *mBlocks[i];
    // Add the block to the cloned resource
    clone->Add(newBlockHandle, i);
  }
  return clone;
}

uint CollisionFilter::GetSize() const
{
  return mBlocks.Size();
}

HandleOf<CollisionFilterBlock> CollisionFilter::GetBlockAt(uint index)
{
  // Convert the instance to a filter and grab the block at the given index
  CollisionFilterBlock* block = mBlocks[index];
  return block;
}

HandleOf<CollisionFilterBlock> CollisionFilter::GetById(BoundType* typeId)
{
  // There's no easy way to turn the id to a block without a linear search,
  // luckily there aren't many block types so it doesn't matter now
  for(uint i = 0; i < mBlocks.Size(); ++i)
  {
    // See if this block has the type id we want
    CollisionFilterBlock* block = mBlocks[i];
    if(ZilchVirtualTypeId(block) == typeId)
      return block;
  }
  
  return Handle();
}

void CollisionFilter::Add(const HandleOf<CollisionFilterBlock>& blockHandle, int index)
{
  CollisionFilterBlock* block = blockHandle;
  // Add the block to the filter
  mBlocks.PushBack(block);
  // Set a flag on the filter so it knows if it has this block type or not.
  // This allows a quick check when sending events to see if
  // a search for the block is even needed.
  mFilterFlags.SetFlag(block->mBlockType);
}

bool CollisionFilter::Remove(const HandleOf<CollisionFilterBlock>& blockHandle)
{
  CollisionFilterBlock* block = blockHandle;
  // Try to find the index of this block
  uint index = mBlocks.FindIndex(block);
  if(index >= mBlocks.Size())
    return false;
  
  // If we found it then remove the block, also make sure to clear
  // the flag on the filter used for quick block checking
  mBlocks.EraseAt(index);
  mFilterFlags.ClearFlag(block->mBlockType);
  return true;
}

size_t CollisionFilter::Hash() const
{
  char* str = (char*)(&mPair.first);
  return HashString(str, sizeof(ResourceId) * 2);
}

bool CollisionFilter::operator==(const CollisionFilter& rhs) const
{
  return mPair == rhs.mPair;
}

}//namespace Zero
