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

PhysicsNodeManager::PhysicsNodeManager()
{

}

void PhysicsNodeManager::AddNode(PhysicsNode* node)
{
  mObjects.PushBack(node);
}

void PhysicsNodeManager::CommitChanges(BroadPhasePackage* package)
{
  //update transforms
  TransformUpdate();

  //update mass terms
  MassUpdate();

  //send out the BroadPhase commands
  BroadPhaseUpdate(package);
}

void PhysicsNodeManager::CommitChangesProfiled(BroadPhasePackage* package)
{
  //update transforms
  {
    ProfileScopeTree("Transform Updates", "Physics", Color::LightPink);
    TransformUpdate();
  }

  //update mass terms
  {
    ProfileScopeTree("Mass Recomputation", "Physics", Color::Gainsboro);
    MassUpdate();
  }

  //send out the BroadPhase commands
  {
    ProfileScopeTree("BatchedColliders", "Physics", Color::Goldenrod);
    BroadPhaseUpdate(package);
  }
}

void PhysicsNodeManager::UpdateNodeTree(PhysicsNode* treeNode)
{
  if(!treeNode->IsTransformOrMassQueued())
    return;

  //we have two lists, one for actually iterating through,
  //the other for searching for nodes throughout the tree
  ObjectList tempList;
  ObjectList stackList;

  //to update this entire tree, we need to have our objects in the correct order,
  //so sort them such that we always get a parent before a child

  //find the root
  PhysicsNode* parentNode = treeNode;
  while(parentNode->mParent)
    parentNode = parentNode->mParent;

  //if we were queued make sure to unqueue to prevent an error
  if(parentNode->GetQueue()->IsQueued())
    ObjectList::Unlink(parentNode);
  stackList.PushBack(parentNode);

  //as long as there are nodes to search through
  while(!stackList.Empty())
  {
    PhysicsNode* node = &stackList.Front();
    stackList.PopFront();

    //add all children nodes
    PhysicsNode::ChildrenRange children = node->mChildren.All();
    for(; !children.Empty(); children.PopFront())
    {
      PhysicsNode* childNode = &children.Front();
      //make sure to unqueue the child from the list if it was queued
      if(childNode->GetQueue()->IsQueued())
        ObjectList::Unlink(childNode);
      stackList.PushBack(childNode);
    }

    //if this node was previously queued then we have to update it, if it
    //wasn't queued we were just using it to search through the tree and
    //ignore it. We could have a queued parent and child and not be queued
    //ourself, that's why this check exists.
    if(node->GetQueue()->IsQueued())
      tempList.PushBack(node);
  }

  //iterate top to bottom and update the cached transform
  ObjectList::range objects = tempList.All();
  for(; !objects.Empty(); objects.PopFront())
  {
    PhysicsNode* node = &objects.Front();
    PhysicsQueue* queue = node->GetQueue();
    queue->mTransformAction.CommitState(node);
    queue->mTransformAction.EmptyState();
  }

  //iterate bottom to top and update the mass (I think the order doesn't
  //matter now, but in the future it might because the parent can use the
  //child's computed mass without recompute each sub-item's mass)
  objects = tempList.All();
  for(; !objects.Empty(); objects.PopBack())
  {
    PhysicsNode* node = &objects.Back();
    PhysicsQueue* queue = node->GetQueue();
    if(node->mBody)
      queue->mMassAction.CommitState(node->mBody);
    queue->mMassAction.EmptyState();
  }

  //we didn't update broadphase, so make sure to add these back into the list (add the end)
  mObjects.Splice(mObjects.End(), tempList);
}

void PhysicsNodeManager::TransformUpdate()
{
  static Array<PhysicsNode*> stack;
  ObjectList tempList;
  stack.Reserve(8);

  //What we are doing here is making sure that a parent is always
  //updated before its child. The way we do this is to take any child
  //we encounter and if its parent is not already taken care of, put
  //it and all of its parent's before us. This will ensure that a parent
  //comes before (although not immediately) its child.
  while(!mObjects.Empty())
  {
    PhysicsNode* node = &(mObjects.Front());
    PhysicsQueue* queue = node->GetQueue();

    //need to mark this item as not being on the queue so that
    //children will know when they iterate
    queue->MarkUnQueued();
    ObjectList::Unlink(node);
    stack.PushBack(node);

    //iterate through our parent's until we reach the top or until
    //we reach a parent that was already accounted for.
    while(node->IsParentQueued())
    {
      node = node->mParent;
      queue = node->GetQueue();
      queue->MarkUnQueued();
      ObjectList::Unlink(node);
      stack.PushBack(node);
    }

    //Take our previous results and put them in a new list in the correct
    //order. Also recompute their world transforms in the correct order
    while(!stack.Empty())
    {
      node = stack.Back();
      queue = node->GetQueue();
      stack.PopBack();

      queue->mTransformAction.CommitState(node);
      tempList.PushBack(node);
    }
  }

  //swap our temp list into our actual list
  mObjects.Swap(tempList);
}

void PhysicsNodeManager::MassUpdate()
{
  ObjectList::range range = mObjects.All();
  while(!range.Empty())
  {
    PhysicsNode& node = range.Back();
    range.PopBack();

    if(node.mBody)
      node.GetQueue()->mMassAction.CommitState(node.mBody);
  }
}

void PhysicsNodeManager::BroadPhaseUpdate(BroadPhasePackage* package)
{
  if(mObjects.Empty())
    return;

  BroadPhaseBatch staticBatch, dynamicBatch;

  ObjectList nodesToDelete;

  //iterate until the list is empty instead of using a range so that
  //we don't run into issues when we delete queues of deleted objects
  while(!mObjects.Empty())
  {
    PhysicsNode* node = &(mObjects.Front());
    PhysicsQueue* queue = node->GetQueue();
    ObjectList::Unlink(node);
    
    if(queue->mBroadPhaseAction.IsActionQueued())
      queue->ProcessQueue(staticBatch, dynamicBatch, node->mCollider);
    queue->Empty();

    //If the owner is null, that implies that this object has been deleted this
    //frame, but we needed the queue to perform the removal. We cannot
    //delete it now though because the queue holds the proxy, so queue it.
    if(node->IsDying())
      nodesToDelete.PushBack(node);
  }

  if(!staticBatch.removals.Empty())
    package->RemoveProxies(BroadPhase::Static, staticBatch.removals);
  if(!dynamicBatch.removals.Empty())
    package->RemoveProxies(BroadPhase::Dynamic, dynamicBatch.removals);

  if(!staticBatch.updates.Empty())
    package->UpdateProxies(BroadPhase::Static, staticBatch.updates);
  if(!dynamicBatch.updates.Empty())
    package->UpdateProxies(BroadPhase::Dynamic, dynamicBatch.updates);

  if(!staticBatch.inserts.Empty())
    package->CreateProxies(BroadPhase::Static, staticBatch.inserts);
  if(!dynamicBatch.inserts.Empty())
    package->CreateProxies(BroadPhase::Dynamic, dynamicBatch.inserts);

  //if any static changes happened, reconstruct the static BroadPhase
  if(!staticBatch.inserts.Empty() || !staticBatch.removals.Empty() || 
     !staticBatch.updates.Empty())
  {
    package->Construct();
  }

  //delete all of the nodes that now have no owners
  while(!nodesToDelete.Empty())
  {
    PhysicsNode* node = &(nodesToDelete.Front());
    ObjectList::Unlink(node);

    delete node;
  }

  mObjects.Clear();
}


}//namespace Physics

}//namespace Zero
