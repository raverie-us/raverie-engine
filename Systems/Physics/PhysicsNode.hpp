///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

///Base class created to have a PhysicsNode InList on itself.
class BasePhysicsNode
{
public:
  Link<BasePhysicsNode> NodeLink;
};

///A node that Contains the shared data between Collider and RigidBody.
///Keeps track of the physics queue, world transform and the hierarchy
///data for a physics cog.
class PhysicsNode : public BasePhysicsNode
{
public:

  PhysicsNode();
  ~PhysicsNode();

  WorldTransformation* GetTransform();
  Physics::PhysicsQueue* GetQueue();

  RigidBody* GetActiveBody();
  Cog* GetCogOwner();

  ///Add the node that is this object's parent
  void AddParent(PhysicsNode* node);
  ///Remove the passed in parent node.
  void RemoveParent(PhysicsNode* node);

  void SetOwner(Collider* collider);
  void SetOwner(RigidBody* body);
  void RemoveOwner(Collider* collider);
  void RemoveOwner(RigidBody* body);

  void Queue(Physics::TransformAction& action);
  void Queue(Physics::MassAction& action);
  void Queue(Physics::BroadPhaseAction& action);

  bool IsTransformOrMassQueued();
  void UpdateTransformAndMass(PhysicsSpace* space);

  bool IsInBroadPhase();
  bool IsInDynamicBroadPhase();
  uint BroadPhaseToRemoveFrom();

  ///Takes care of queuing to the manager and keeping track of queue state.
  void QueueSelf();

  bool IsParentQueued() const;
  bool IsDying() const;

  bool IsBodyRoot();

  void ReadTransform();
  void RecomputeWorldTransform();

  ///When these two are nullptr, the manager should delete this.
  Collider* mCollider;
  RigidBody* mBody;

  typedef BaseInList<BasePhysicsNode, PhysicsNode, &BasePhysicsNode::NodeLink> ChildrenList;
  typedef ChildrenList::range ChildrenRange;
  ChildrenList mChildren;
  PhysicsNode* mParent;

  ///Link for the physics node queue.
  IntrusiveLink(PhysicsNode, QueueLink);
private:

  Physics::PhysicsQueue mQueue;
  WorldTransformation mTransform;
};

}//namespace Zero
