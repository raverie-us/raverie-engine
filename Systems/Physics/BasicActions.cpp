///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

IntegrationAction::IntegrationAction(Collider* collider)
{
  mTransformAction.QueueState(TransformAction::WorldTransform);
  mMassAction.QueueState(MassAction::WorldInertiaTensor);
  mBroadPhaseAction.UpdateAction(collider);

  //make sure we are in the queue for update
  collider->mPhysicsNode->QueueSelf();
  //queue all of the actions
  collider->mPhysicsNode->Queue(mTransformAction);
  collider->mPhysicsNode->Queue(mMassAction);
  collider->mPhysicsNode->Queue(mBroadPhaseAction);
}

MovementAction::MovementAction(Collider* collider)
{
  mTransformAction.QueueState(TransformAction::WorldTransform);
  mMassAction.QueueState(MassAction::WorldInertiaTensor);
  mMassAction.QueueState(MassAction::CenterMassUpdate);
  mBroadPhaseAction.UpdateAction(collider);

  //make sure we are in the queue for update
  collider->mPhysicsNode->QueueSelf();
  //queue all of the actions
  collider->mPhysicsNode->Queue(mTransformAction);
  collider->mPhysicsNode->Queue(mMassAction);
  collider->mPhysicsNode->Queue(mBroadPhaseAction);
}

KinematicMovementAction::KinematicMovementAction(RigidBody* body)
{
  mTransformAction.QueueState(TransformAction::WorldTransform);
  mTransformAction.QueueState(TransformAction::KinematicVelocity);

  //make sure we are in the queue for update
  body->mPhysicsNode->QueueSelf();
  //queue all of the actions
  body->mPhysicsNode->Queue(mTransformAction);
}

ColliderCreationAction::ColliderCreationAction(Collider* collider)
{
  mTransformAction.QueueState(TransformAction::WorldTransform);
  mTransformAction.QueueState(TransformAction::OverrideOldTransform);
  mBroadPhaseAction.InsertAction(collider);

  PhysicsNode* node = collider->mPhysicsNode;
  //make sure we are in the queue for update
  node->QueueSelf();
  //queue all of the actions
  node->Queue(mTransformAction);
  node->Queue(mBroadPhaseAction);
}

RigidBodyCreationAction::RigidBodyCreationAction(RigidBody* body)
{
  mMassAction.QueueState(MassAction::RecomputeCenterMass);
  mMassAction.QueueState(MassAction::RecomputeInertiaTensor);
  mMassAction.QueueState(MassAction::WorldInertiaTensor);

  //make sure we are in the queue for update
  body->mPhysicsNode->QueueSelf();
  //queue all of the actions
  body->mPhysicsNode->Queue(mMassAction);
}

FullTransformAction::FullTransformAction(Collider* collider)
{
  mTransformAction.QueueState(TransformAction::ReadTransform);
  mTransformAction.QueueState(TransformAction::WorldTransform);

  //make sure we are in the queue for update
  collider->mPhysicsNode->QueueSelf();
  //queue all of the actions
  collider->mPhysicsNode->Queue(mTransformAction);
}

FullTransformAction::FullTransformAction(PhysicsNode* node)
{
  mTransformAction.QueueState(TransformAction::ReadTransform);
  mTransformAction.QueueState(TransformAction::WorldTransform);

  //make sure we are in the queue for update
  node->QueueSelf();
  //queue all of the actions
  node->Queue(mTransformAction);
}

WorldTransformationAction::WorldTransformationAction(Collider* collider)
{
  mTransformAction.QueueState(TransformAction::WorldTransform);

  //make sure we are in the queue for update
  collider->mPhysicsNode->QueueSelf();
  //queue all of the actions
  collider->mPhysicsNode->Queue(mTransformAction);
}

InsertionAction::InsertionAction(Collider* collider)
{
  mTransformAction.QueueState(TransformAction::WorldTransform);
  mBroadPhaseAction.InsertAction(collider);

  Queue(collider);
}

InsertionAction::InsertionAction(Collider* collider, byte state)
{
  mTransformAction.QueueState(TransformAction::WorldTransform);
  mBroadPhaseAction.QueueState(state);

  Queue(collider);
}

void InsertionAction::Queue(Collider* collider)
{
  //make sure we are in the queue for update
  collider->mPhysicsNode->QueueSelf();
  //queue all of the actions
  collider->mPhysicsNode->Queue(mTransformAction);
  collider->mPhysicsNode->Queue(mBroadPhaseAction);
}

RemovalAction::RemovalAction(Collider* collider)
{
  mBroadPhaseAction.RemoveAction(collider);

  //make sure we are in the queue for update
  collider->mPhysicsNode->QueueSelf();
  //queue all of the actions
  collider->mPhysicsNode->Queue(mBroadPhaseAction);
}

MassRecomputationAction::MassRecomputationAction(RigidBody* body)
{
  mMassAction.QueueState(MassAction::RecomputeCenterMass);
  mMassAction.QueueState(MassAction::RecomputeInertiaTensor);
  mMassAction.QueueState(MassAction::WorldInertiaTensor);

  //make sure we are in the queue for update
  body->mPhysicsNode->QueueSelf();
  //queue all of the actions
  body->mPhysicsNode->Queue(mMassAction);
}

void QueueFullMassRecompuation(PhysicsNode* node)
{
  MassAction massAction;
  massAction.QueueState(MassAction::RecomputeCenterMass);
  massAction.QueueState(MassAction::RecomputeInertiaTensor);
  massAction.QueueState(MassAction::WorldInertiaTensor);

  //make sure we are in the queue for update
  node->QueueSelf();
  //queue all of the actions
  node->Queue(massAction);
}

void QueueInertiaRecompuation(PhysicsNode* node)
{
  MassAction massAction;
  massAction.QueueState(MassAction::RecomputeInertiaTensor);
  massAction.QueueState(MassAction::WorldInertiaTensor);

  //make sure we are in the queue for update
  node->QueueSelf();
  //queue all of the actions
  node->Queue(massAction);
}

void QueueTransformRead(PhysicsNode* node)
{
  TransformAction transformAction;
  transformAction.QueueState(TransformAction::WorldTransform);
  transformAction.QueueState(TransformAction::ReadTransform);
  transformAction.QueueState(TransformAction::KinematicVelocity);

  node->QueueSelf();
  node->Queue(transformAction);
}

void QueueBodyIntegration(PhysicsNode* node)
{
  TransformAction transformAction;
  transformAction.QueueState(TransformAction::WorldTransform);
  transformAction.QueueState(TransformAction::KinematicVelocity);
  
  MassAction massAction;
  massAction.QueueState(MassAction::WorldInertiaTensor);
  
  //make sure we are in the queue for update
  node->QueueSelf();

  //queue all of the actions
  node->Queue(transformAction);
  node->Queue(massAction);
}

void ChangeBroadPhase(Collider* collider, bool toDynamicBroadphase)
{
  PhysicsNode* node = collider->mPhysicsNode;

  BroadPhaseAction bpAction;
  bpAction.RemoveAction(collider);
  if(toDynamicBroadphase)
    bpAction.QueueState(BroadPhaseAction::DynamicInsert);
  else
    bpAction.QueueState(BroadPhaseAction::StaticInsert);

  node->QueueSelf();
  node->Queue(bpAction);
}

void InsertIntoBroadPhase(Collider* collider)
{
  PhysicsNode* node = collider->mPhysicsNode;

  BroadPhaseAction bpAction;
  //if we have a direct body then we go into dynamic
  if(collider->mDirectRigidBody)
    bpAction.QueueState(BroadPhaseAction::DynamicInsert);
  else
    bpAction.QueueState(BroadPhaseAction::StaticInsert);

  node->QueueSelf();
  node->Queue(bpAction);
}

bool RemoveFromBroadPhase(Collider* collider)
{
  PhysicsNode* node = collider->mPhysicsNode;
  uint id = node->BroadPhaseToRemoveFrom();
  if(id == 0)
    return false;

  BroadPhaseAction bpAction;
  bpAction.PushAction(id);

  node->QueueSelf();
  node->Queue(bpAction);
  return true;
}

void UpdateInBroadPhase(Collider* collider)
{
  PhysicsNode* node = collider->mPhysicsNode;

  BroadPhaseAction bpAction;
  bpAction.UpdateAction(collider);

  node->QueueSelf();
  node->Queue(bpAction);
}

void QueueOverrideOldTransform(RigidBody* body)
{
  PhysicsNode* node = body->mPhysicsNode;

  TransformAction transformAction;
  transformAction.QueueState(TransformAction::OverrideOldTransform);

  //make sure we are in the queue for update
  node->QueueSelf();

  //queue all of the actions
  node->Queue(transformAction);
}

}//namespace Zero
