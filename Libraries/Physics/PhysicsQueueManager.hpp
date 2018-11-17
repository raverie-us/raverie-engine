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

///Manages the collection of PhysicsQueues. Updates the transform, mass and
///BroadPhase properties for all objects in the queue in the correct order.
///Outstanding changes should be pushed before this is deleted.
struct PhysicsNodeManager
{
  PhysicsNodeManager();

  ///Adds a PhysicsQueue to be pushed during the next commit phase.
  void AddNode(PhysicsNode* node);
  ///Commits all outstanding PhysicsQueue changes and clears the pending list.
  void CommitChanges(BroadPhasePackage* package);
  ///Same as above but with profile blocks.
  void CommitChangesProfiled(BroadPhasePackage* package);

  void UpdateNodeTree(PhysicsNode* node);

private:
  ///Take care of the TransformActions.
  void TransformUpdate();
  ///Take care of the MassActions.
  void MassUpdate();
  ///Take care of the BroadPhaseActions.
  void BroadPhaseUpdate(BroadPhasePackage* package);

  typedef InList<PhysicsNode, &PhysicsNode::QueueLink> ObjectList;
  ObjectList mObjects;
};

}//namespace Physics

}//namespace Zero
