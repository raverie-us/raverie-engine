///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

PhysicsNode::PhysicsNode()
{
  mCollider = nullptr;
  mBody = nullptr;
  mParent = nullptr;
}

PhysicsNode::~PhysicsNode()
{

}

WorldTransformation* PhysicsNode::GetTransform()
{
  return &mTransform;
}

Physics::PhysicsQueue* PhysicsNode::GetQueue()
{
  return &mQueue;
}

RigidBody* PhysicsNode::GetActiveBody()
{
  if(mCollider != nullptr)
    return mCollider->GetActiveBody();
  return nullptr;
}

Cog* PhysicsNode::GetCogOwner()
{
  if(mCollider)
    return mCollider->GetOwner();
  if(mBody)
    return mBody->GetOwner();
  return nullptr;
}

void PhysicsNode::AddParent(PhysicsNode* node)
{
  mParent = node;
  node->mChildren.PushBack(this);
}

void PhysicsNode::RemoveParent(PhysicsNode* node)
{
  ErrorIf(mParent != node, "Removing a node that is not the parent.");
  mParent->mChildren.Unlink(this);
  mParent = nullptr;
}

void PhysicsNode::SetOwner(Collider* collider)
{
  ErrorIf(mCollider != nullptr,
    "Physics Queue already had a collider owner. Did you forget to remove the previous owner?");

  mCollider = collider;
  mCollider->mPhysicsNode = this;
}

void PhysicsNode::SetOwner(RigidBody* body)
{
  ErrorIf(mBody != nullptr,
    "Physics Queue already had a rigid body owner. Did you forget to remove the previous owner?");

  mBody = body;
  mBody->mPhysicsNode = this;
}

void PhysicsNode::RemoveOwner(Collider* collider)
{
  //(don't do this because during a component removal, it triggers a transform
  //update which can re-enter and blow up when the node on the object is null)
  //mCollider->mPhysicsNode = nullptr;
  mCollider = nullptr;
}

void PhysicsNode::RemoveOwner(RigidBody* body)
{
  //(don't do this because during a component removal, it triggers a transform
  //update which can re-enter and blow up when the node on the object is null)
  //mBody->mPhysicsNode = nullptr;
  mBody = nullptr;
}

void PhysicsNode::Queue(Physics::TransformAction& action)
{
  mQueue.Queue(action);
}

void PhysicsNode::Queue(Physics::MassAction& action)
{
  mQueue.Queue(action);
}

void PhysicsNode::Queue(Physics::BroadPhaseAction& action)
{
  mQueue.Queue(action);
}

bool PhysicsNode::IsTransformOrMassQueued()
{
  //if there is any action queued up then the state will not be empty
  return mQueue.mTransformAction.mState != Physics::TransformAction::Empty ||
         mQueue.mMassAction.mState != Physics::MassAction::Empty;
}

void PhysicsNode::UpdateTransformAndMass(PhysicsSpace* space)
{
  //Make sure that there is something to do first.
  //The update call below walks through the entire tree and is therefore expensive
  //to be calling constantly if there is no change. Whenever any action is queued up,
  //it should propagate to all children in the tree, meaning that if there is a change
  //required higher up the tree that affects us then our state will have been set.
  if(!IsTransformOrMassQueued())
    return;

  space->UpdateTransformAndMassOfTree(this);
}

bool PhysicsNode::IsInBroadPhase()
{
  return mQueue.mBroadPhaseAction.IsInBroadPhase();
}

bool PhysicsNode::IsInDynamicBroadPhase()
{
  return mQueue.mBroadPhaseAction.IsInDynamicBroadPhase();
}

uint PhysicsNode::BroadPhaseToRemoveFrom()
{
  return mQueue.mBroadPhaseAction.BroadPhaseToRemoveFrom();
}

void PhysicsNode::QueueSelf()
{
  if(!mQueue.IsQueued())
  {
    //need to get to the space somehow, go through the collider
    //or body depending on which one we have
    if(mCollider)
      mCollider->mSpace->QueuePhysicsNode(this);
    else if(mBody)
      mBody->mSpace->QueuePhysicsNode(this);
    else
      ErrorIf(true, "Action was queued on a dead queue. Make sure all changes are done before removing the owner.");

    mQueue.mBroadPhaseAction.SetState(Physics::BroadPhaseAction::CurrStateQueued);
  }
}

bool PhysicsNode::IsParentQueued() const
{
  if(mParent)
    return mParent->GetQueue()->IsQueued();
  return false;
}

bool PhysicsNode::IsDying() const
{
  return mCollider == nullptr && mBody == nullptr;
}

bool PhysicsNode::IsBodyRoot()
{
  //could in theory changes this to be checking if our body pointer is null.
  //if that is the case that would mean we are not a root.
  return mParent && (GetActiveBody() != mParent->GetActiveBody());
}

void PhysicsNode::ReadTransform()
{
  mTransform.ReadTransform(this);
}

void PhysicsNode::RecomputeWorldTransform()
{
  //figure out if we need to include our parent's transform
  WorldTransformation* parentTransformation = nullptr;
  if(mParent)
    parentTransformation = mParent->GetTransform();

  mTransform.ComputeTransformation(parentTransformation,this);

  if(mBody)
    mBody->mRotationQuat = Math::ToQuaternion(mTransform.GetWorldRotation());

  if(mCollider)
    mCollider->ComputeWorldBoundingVolumes();

  //Due to transform updates being propagated, we do not have to do anything
  //to the children. If a parent is added to the queue, then all of its
  //children will also be added due to either updates being propagated
  //from parent to child, or due to integration queuing up all children.
}

}//namespace Zero
