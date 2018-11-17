///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------CollisionGroup
ZilchDefineType(CollisionGroup, builder, type)
{
  ZeroBindDocumented();

  ZeroBindTag(Tags::Physics);

  ZilchBindMethod(CreateRuntime);
  ZilchBindMethod(RuntimeClone);
}

CollisionGroup::CollisionGroup()
{

}

CollisionGroup::~CollisionGroup()
{

}

HandleOf<CollisionGroup> CollisionGroup::CreateRuntime()
{
  return CollisionGroupManager::CreateRuntime();
}

HandleOf<Resource> CollisionGroup::Clone()
{
  return RuntimeClone();
}

HandleOf<CollisionGroup> CollisionGroup::RuntimeClone()
{
  return CollisionGroupManager::CreateRuntime();
}

CollisionGroupInstance* CollisionGroup::GetNewInstance()
{
  // Create a new collision group that points at us
  CollisionGroupInstance* instance = new CollisionGroupInstance();
  instance->mResource = this;
  instance->mGroupId = 0;
  instance->mDetectionMask = u32(-1);
  instance->mResolutionMask = u32(-1);
  return instance;
}

//-------------------------------------------------------------------CollisionGroupInstance
CollisionGroupInstance::CollisionGroupInstance()
{
  mGroupId = 0;
  mDetectionMask = u32(-1);
  mResolutionMask = u32(-1);
  mTable = nullptr;
}

bool CollisionGroupInstance::SkipDetection(const CollisionGroupInstance& rhs) const
{
  return !((mGroupId & rhs.mDetectionMask) || (rhs.mGroupId & mDetectionMask));
}

bool CollisionGroupInstance::SkipResolution(const CollisionGroupInstance& rhs) const
{
  return !((mGroupId & rhs.mResolutionMask) || (rhs.mGroupId & mResolutionMask));
}

String CollisionGroupInstance::GetGroupName() const
{
  return mResource->Name;
}

//-------------------------------------------------------------------CollisionGroupManager
ImplementResourceManager(CollisionGroupManager, CollisionGroup);

CollisionGroupManager::CollisionGroupManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  AddLoader("CollisionGroup", new TextDataFileLoader<CollisionGroupManager>());
  mCategory = "Physics";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.CollisionGroup.data"));
  DefaultResourceName = "DefaultGroup";
  mCanCreateNew = true;
  mCanDuplicate = true;
  mExtension = DataResourceExtension;
}

}//namespace Zero
