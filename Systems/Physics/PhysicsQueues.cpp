///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
  
namespace Physics
{

PhysicsQueue::PhysicsQueue()
{

}

void PhysicsQueue::Queue(TransformAction& action)
{
  mTransformAction.QueueState(action.mState);
}

void PhysicsQueue::Queue(MassAction& action)
{
  mMassAction.QueueState(action.mState);
}

void PhysicsQueue::Queue(BroadPhaseAction& action)
{
  mBroadPhaseAction.QueueState(action.mState);
}

void PhysicsQueue::Validate()
{
  mTransformAction.Validate();
  mMassAction.Validate();
  mBroadPhaseAction.Validate();
}

void PhysicsQueue::Empty()
{
  mTransformAction.EmptyState();
  mMassAction.EmptyState();
  mBroadPhaseAction.EmptyState();
}

void PhysicsQueue::ProcessQueue(BroadPhaseBatch& staticBatch, BroadPhaseBatch& dynamicBatch, Collider* collider)
{
  typedef BroadPhaseAction BPAction;
  BPAction& action = mBroadPhaseAction;

  byte& state = mBroadPhaseAction.mState;

  //deal with dynamic states
  if(action.IsSet(BPAction::DynamicRemoval))
    dynamicBatch.removals.PushBack(&mProxy);
  else if(action.IsSet(BPAction::DynamicInsert))
  {
    BroadPhaseData bpData;
    ColliderToBroadPhaseData(collider, bpData);
    BroadPhaseObject bpObject(&mProxy, bpData);
    dynamicBatch.inserts.PushBack(bpObject);
  }

  //deal with static states
  if(action.IsSet(BPAction::StaticRemoval))
    staticBatch.removals.PushBack(&mProxy);
  else if(action.IsSet(BPAction::StaticInsert))
  {
    BroadPhaseData bpData;
    ColliderToBroadPhaseData(collider, bpData);
    BroadPhaseObject bpObject(&mProxy, bpData);
    staticBatch.inserts.PushBack(bpObject);
  }

  //deal with updates
  if(!action.IsSet(BPAction::Static | BPAction::Dynamic) && 
     action.IsSet(BPAction::Update))
  {
    BroadPhaseData bpData;
    ColliderToBroadPhaseData(collider, bpData);
    BroadPhaseObject bpObject(&mProxy, bpData);

    if(action.IsSet(BPAction::CurrStateStatic))
      staticBatch.updates.PushBack(bpObject);
    else if(action.IsSet(BPAction::CurrStateDynamic))
      dynamicBatch.updates.PushBack(bpObject);
    else
      ErrorIf(true, "No states were set for this object's broadphase queue.");
  }
}

void PhysicsQueue::ColliderToBroadPhaseData(Collider* collider, BroadPhaseData& data)
{
  data.mAabb = collider->mAabb;
  data.mClientData = (void*)collider;
  data.mBoundingSphere = collider->mBoundingSphere;
}

void PhysicsQueue::MarkUnQueued()
{
  mBroadPhaseAction.ClearState(BroadPhaseAction::CurrStateQueued);
}

bool PhysicsQueue::IsQueued() const
{
  return mBroadPhaseAction.IsSet(BroadPhaseAction::CurrStateQueued);
}

}//namespace Physics

}//namespace Zero
