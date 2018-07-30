///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{

///A queue for a Cog. Is associated with a set of actions for
///both the collider and rigid body on a cog. Will automatically be
///delete when the associated collider and body remove ownership.
struct PhysicsQueue
{
  PhysicsQueue();

  void Queue(TransformAction& action);
  void Queue(MassAction& action);
  void Queue(BroadPhaseAction& action);

  bool IsQueued() const;
  BroadPhaseAction mBroadPhaseAction;

  friend struct PhysicsNodeManager;
  friend class PhysicsNode;

  void Validate();
  ///Empty the actions
  void Empty();

  ///Helper for the BroadPhase action. Fills out the batch structures for what
  ///needs to be changed in BroadPhase.
  void ProcessQueue(BroadPhaseBatch& staticBatch, BroadPhaseBatch& dynamicBatch, Collider* collider);
  void ColliderToBroadPhaseData(Collider* collider, BroadPhaseData& data);

  void MarkUnQueued();
  

  TransformAction mTransformAction;
  MassAction mMassAction;
  
  ///The proxy needs to be stored on the queue since the object will
  ///already be deleted by the time the queue is processed and we need proxy to
  ///remove the object from BroadPhase.
  BroadPhaseProxy mProxy;
};

}//namespace Physics

}//namespace Zero
