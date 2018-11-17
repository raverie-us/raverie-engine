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

TransformAction::TransformAction()
{
  EmptyState();
}

void TransformAction::QueueState(byte state)
{
  mState |= state;
}

void TransformAction::CommitState(PhysicsNode* node)
{
  Collider* collider = node->mCollider;
  RigidBody* body = node->mBody;

  if(collider == nullptr && body == nullptr)
    return;

  WorldTransformation* transform = node->GetTransform();

  Vec3 oldTranslation = transform->GetOldTranslation();
  Mat3 oldRotation = transform->GetOldRotation();

  if(mState & ReadTransform)
    node->ReadTransform();
  if(mState & WorldTransform)
    node->RecomputeWorldTransform();
  //this happens when a collider is first made, we need to update the old
  //values to be where it currently is so that the kinematic velocity
  //calculation for the first frame will be correct
  if(mState & OverrideOldTransform)
  {
    oldTranslation = transform->GetWorldTranslation();
    oldRotation = transform->GetWorldRotation();
    transform->mOldTranslation = oldTranslation;
    transform->mOldRotation = oldRotation;
  }

  //Update the kinematic velocity if requested. The partial velocity still needs to be
  //updated here even though it's updated in UpdateKinematicVelocities so that the user
  //can update positions mid-frame and get a new velocity on their kinematic objects.
  //However, don't update the old position and rotation as that should only be updated
  //once per frame. If the old position is updated multiple times then velocity calculation
  //will get partial values. To avoid that the positions are only updated once a
  //frame (see UpdateKinematicVelocities).
  if(body != nullptr && mState & KinematicVelocity)
    body->ComputeVelocities(oldTranslation, oldRotation, body->mSpace->mIterationDt);
}

void TransformAction::EmptyState()
{
  mState = Empty;
}

void TransformAction::Validate()
{

}


MassAction::MassAction()
{
  EmptyState();
}

void MassAction::QueueState(byte state)
{
  mState |= state;
}

void MassAction::CommitState(RigidBody* owner)
{
  if(mState & RecomputeCenterMass)
    owner->RecomputeCenterMass();
  if(mState & CenterMassUpdate)
    owner->UpdateCenterMassFromWorldPosition();
  if(mState & RecomputeInertiaTensor)
    owner->RecomputeInertiaTensor();
  if(mState & WorldInertiaTensor)
    owner->UpdateWorldInertiaTensor();
  //we've now committed all state, mark ourself as empty now
  EmptyState();
}

void MassAction::EmptyState()
{
  mState = Empty;
}

void MassAction::Validate()
{

}

BroadPhaseAction::BroadPhaseAction()
{
  mState = Empty;
}

void BroadPhaseAction::PushAction(byte state)
{
  QueueState(state);
}

void BroadPhaseAction::InsertAction(Collider* collider)
{
  if(collider->InDynamicBroadPhase())
    PushAction(DynamicInsert);
  else
    PushAction(StaticInsert);
}

void BroadPhaseAction::RemoveAction(Collider* collider)
{
  if(collider->InDynamicBroadPhase())
    PushAction(DynamicRemoval);
  else
    PushAction(StaticRemoval);
}

void BroadPhaseAction::UpdateAction(Collider* collider)
{
  PushAction(Update);
}

void BroadPhaseAction::QueueState(byte state)
{
  //deal with dynamic states
  if(state & DynamicInsert)
  {
    if(IsSet(DynamicRemoval))
      ClearState(Dynamic);
    else
    {
      ClearState(Dynamic);
      SetState(state & Dynamic);
    }
  }
  else if(state & DynamicRemoval)
  {
    if(IsSet(DynamicInsert))
      ClearState(Dynamic);
    else
    {
      ClearState(Dynamic);
      SetState(state & Dynamic);
    }
  }


  //deal with static states
  if(state & StaticInsert)
  {
    if(IsSet(StaticRemoval))
      ClearState(Static);
    else
    {
      ClearState(Static);
      SetState(state & Static);
    }
  }
  else if(state & StaticRemoval)
  {
    if(IsSet(StaticInsert))
      ClearState(Static);
    else
    {
      ClearState(Static);
      SetState(state & Static);
    }
  }

  //combine the old state (with update stripped out) with the new update state.
  mState = (mState & ~Update) | (state & Update);
}

void BroadPhaseAction::EmptyState()
{
  //determine if we are in a static or dynamic state
  if(mState & DynamicInsert)
    mState = CurrStateDynamic;
  else if(mState & StaticInsert)
    mState = CurrStateStatic;
  else if(mState & (DynamicRemoval | StaticRemoval))
    mState = mState & ~(CurrStateDynamic | CurrStateStatic);

  ClearState(CurrStateQueued | Update | Static | Dynamic);

  Validate();
}

void BroadPhaseAction::ClearAction()
{
  mState = Empty;
}

bool BroadPhaseAction::IsSet(byte state) const
{
  return (mState & state) != 0;
}

void BroadPhaseAction::SetState(byte state)
{
  mState |= state;
}

void BroadPhaseAction::ClearState(byte state)
{
  mState = mState & ~state;
}

bool BroadPhaseAction::IsActionQueued() const
{
  return (mState & (Update | Dynamic | Static)) != 0;
}

bool BroadPhaseAction::IsInBroadPhase() const
{
  return (mState & (CurrStateStatic | CurrStateDynamic)) != 0;
}

bool BroadPhaseAction::IsInDynamicBroadPhase() const
{
  return (mState & CurrStateDynamic) != 0;
}

uint BroadPhaseAction::BroadPhaseToRemoveFrom() const
{
  //we cannot just check the current broadphase state. We may be currently in static,
  //but with a static remove and a dynamic Insert queued. That would mean we'd
  //want to queue a dynamic remove. Therefore, if we have an Insert queued, queue
  //the corresponding remove. If we don't have an Insert, just use the current state.
  if(mState & StaticInsert)
    return StaticRemoval;
  if(mState & DynamicInsert)
    return DynamicRemoval;
  if(mState & CurrStateStatic)
    return StaticRemoval;
  if(mState & CurrStateDynamic)
    return DynamicRemoval;

  return 0;
}

void BroadPhaseAction::Validate()
{
  ErrorIf(IsSet(DynamicRemoval) && !IsSet(CurrStateDynamic),
    "Removing Dynamic when this object was never dynamic.");
  ErrorIf(IsSet(StaticRemoval) && !IsSet(CurrStateStatic),
    "Removing Static when this object was never static.");
  ErrorIf(IsSet(DynamicRemoval) && IsSet(StaticRemoval),
    "Removing from static and dynamic in same action.");
  ErrorIf(IsSet(DynamicInsert) && IsSet(StaticInsert),
    "Inserting to static and dynamic in same action.");

  ErrorIf(IsSet(DynamicRemoval) && IsSet(DynamicInsert),
    "Cannot remove and Insert an object simultaneously.");

  ErrorIf(IsSet(StaticRemoval) && IsSet(StaticInsert),
    "Cannot remove and Insert an object simultaneously.");
}

}//namespace Physics

}//namespace Zero
