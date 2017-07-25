///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2012-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

void AddChildBody(RigidBody* parent, RigidBody* child)
{
  ErrorIf(child->mParentBody != nullptr, "Child already had a parent body.");

  parent->mChildBodies.PushBack(child);
  child->mParentBody = parent;
}

void RemoveChildBody(RigidBody* parent, RigidBody* child)
{
  ErrorIf(child->mParentBody != parent,
    "Child had a different parent that what was passed in.");

  child->mParentBody = nullptr;
  parent->mChildBodies.Unlink(child);
}

void AddDirectChildCollider(RigidBody* directBody, Collider* collider)
{
  ErrorIf(collider->mActiveRigidBody != nullptr, "Collider already had a body.");

  collider->mDirectRigidBody = directBody;
  directBody->mColliders.PushBack(collider);
}

void RemoveDirectChildCollider(RigidBody* directBody, Collider* collider)
{
  ErrorIf(collider->mActiveRigidBody != directBody, "Collider did not have a body.");

  collider->mDirectRigidBody = nullptr;
  directBody->mColliders.Unlink(collider);
}

PhysicsNode* GetTreeRoot(PhysicsNode* node)
{
  PhysicsNode* root = node;
  while(root->mParent)
    root = root->mParent;
  return root;
}

void ValidateNode(PhysicsNode* node)
{
  node->RecomputeWorldTransform();

  PhysicsNode* parentNode = node->mParent;
  Cog* parentNodeCog = nullptr;
  if(parentNode)
    parentNodeCog = parentNode->GetTransform()->mTransform->GetOwner();

  Cog* origCog = node->GetTransform()->mTransform->GetOwner();
  Cog* cog = origCog;

  while(cog)
  {
    if(cog == parentNodeCog)
      return;

    cog = cog->GetParent();
  }

  if(cog == parentNodeCog)
    return;

  ZERO_DEBUG_BREAK;
  ErrorIf(true, "Not valid link");
}

void ValidateTree(PhysicsNode* testNode)
{
  //ValidateNode(testNode);
  PhysicsNode* root = GetTreeRoot(testNode);

  //do a depth first traversal of the physics nodes and call the
  //passed in functor on all of them
  Array<PhysicsNode*> stack;
  stack.Reserve(64);
  stack.PushBack(root);

  while(!stack.Empty())
  {
    PhysicsNode* node = stack.Back();
    stack.PopBack();

    ValidateNode(node);

    PhysicsNode::ChildrenList::range r = node->mChildren.All();
    for(; !r.Empty(); r.PopFront())
      stack.PushBack(&r.Front());
  }
}

PhysicsNode* FindParentNode(Cog* owner)
{
  //loop up over all of the cog's in the hierarchy,
  //once we find one with physics, we can return the node at that level
  Cog* parent = owner;
  while(parent)
  {
    Collider* collider = parent->has(Collider);
    if(collider)
      return collider->mPhysicsNode;

    RigidBody* body = parent->has(RigidBody);
    if(body)
      return body->mPhysicsNode;

    parent = parent->GetParent();
  }
  return nullptr;
}

//when a new node comes into existence, our parent may have children
//nodes that actually belong to us we need to figure out which of those
//nodes have us as a parent before their old parent
void RelinkNodes(Cog* owner, Cog* parentOwner, PhysicsNode* node, PhysicsNode* parentNode)
{
  PhysicsNode::ChildrenRange range = parentNode->mChildren.All();
  while(!range.Empty())
  {
    PhysicsNode* childNode = &range.Front();
    range.PopFront();

    Cog* childCog = childNode->GetCogOwner();
    Cog* parentCog = childCog->GetParent();
    //loop up the hierarchy until we either come across the old parent or the new node
    while(parentCog != parentOwner)
    {
      //if we come across the new node, we need to relink to that node
      if(parentCog == owner)
      {
        childNode->RemoveParent(parentNode);
        childNode->AddParent(node);
        break;
      }
      parentCog = parentCog->GetParent();
    }
  }
}

//when a new node comes into existence, if it has no parent node then we are stuck
//with an unfortunate situation. We might have nodes under us that we need to be
//linked. The only way to find them is do a full traversal of all hierarchies
//until we find a cog with physics where we can extract the node.
void LinkNewParentNode(PhysicsNode* node, Cog* owner)
{
  Array<Cog*> stack;
  stack.Reserve(64);
  stack.PushBack(owner);

  while(!stack.Empty())
  {
    Cog* cog = stack.Back();
    stack.PopBack();

    //if there is no hierarchy, then there is no point in continuing
    Hierarchy* hierarchy = cog->has(Hierarchy);
    if(hierarchy == nullptr)
      continue;

    //loop through all of the children of this object, if we can find a
    //child with physics on it, then we can stop iterating through that
    //branch of the tree. Otherwise, we have to continue down that
    //tree either until we reach the end or until we find physics.
    HierarchyList::range children = hierarchy->Children.All();
    for(; !children.Empty(); children.PopFront())
    {
      Cog* child = &(children.Front());
      Collider* collider = child->has(Collider);
      if(collider)
      {
        collider->mPhysicsNode->AddParent(node);
        continue;
      }
      RigidBody* body = child->has(RigidBody);
      if(body)
      {
        body->mPhysicsNode->AddParent(node);
        continue;
      }
      stack.PushBack(&(children.Front()));
    }
  }
}

//Clear all of the body/collider linkings to each other.
//This is assumed to run on the root node
struct ClearFunctor
{
  void Action(PhysicsNode* node)
  {
    RigidBody* body = node->mBody;
    Collider* collider = node->mCollider;
    if(body)
    {
      body->mParentBody = nullptr;
      body->mChildBodies.Clear();
      body->mColliders.Clear();
    }
    if(collider)
    {
      collider->mActiveRigidBody = nullptr;
      collider->mDirectRigidBody = nullptr;
    }
  }
};

//For any given node, determine if it is in world space or not, find if there
//are parent bodies, link the colliders to their correct bodies.
void LinkTreeNode(PhysicsNode* node)
{
  RigidBody* body = node->mBody;
  Collider* collider = node->mCollider;

  //don't update the in-world state here because we need to update that bottom-up
  //not top-down. This is safe to delay till later because everything here that
  //matters is queuing up an action to do later.

  RigidBody* directBody = nullptr;
  RigidBody* activeBody = nullptr;

  PhysicsNode* parentNode = node->mParent;
  //find our direct body from our parent collider or body
  if(parentNode)
  {
    //in the collider case, we can also grab the active body
    if(parentNode->mCollider)
    {
      activeBody = parentNode->mCollider->mActiveRigidBody;
      directBody = parentNode->mCollider->mDirectRigidBody;
    }
    else
      directBody = parentNode->mBody;
  }
  
  //hook up the body list
  if(body && directBody)
    AddChildBody(directBody, body);
  if(collider)
  {
    //set up the direct body of the collider (if the node has a
    //body then it takes over as the direct body)
    if(body)
      AddDirectChildCollider(body, collider);
    else if(directBody)
      AddDirectChildCollider(directBody, collider);

    //if this body is dynamic or kinematic, then it is the active body for this collider
    if(body && (body->IsDynamic() || body->GetKinematic()))
      collider->mActiveRigidBody = body;
    //no direct body means there is no body above us
    else if(directBody)
    {
      //if we do not have active body set, either we went into the body case above (no collider parent)
      //or the active body of our parent is null, either way, recurse the direct body to find the active body
      //(could speed up i guess by keeping another flag for if we took the collider branch
      //above since we'd know that active body is null)
      if(!activeBody)
      {
        activeBody = directBody;
        while(activeBody && activeBody->GetStatic())
          activeBody = activeBody->mParentBody;
      }
      collider->mActiveRigidBody = activeBody;
    }
  }
}

struct BuildFunctor
{
  void Action(PhysicsNode* node)
  {
    LinkTreeNode(node);
  }
};

//Re-link the collider/rigidbody of a node. Also queue up transform, mass and
//broadphase changes to account for everything. As a small final thing, make sure
//that each collider and rigidbody are in their correct list on the space.
struct CompleteBuildFunctor
{
  void Action(PhysicsNode* node)
  {
    LinkTreeNode(node);

    QueueTransformRead(node);
    RigidBody* body = node->mBody;
    Collider* collider = node->mCollider;
    if(body)
    {
      QueueFullMassRecompuation(body->mPhysicsNode);
      //wake up the body in case anything happened (JoshD questions) (needed?)
      body->ForceAwake();
      body->mSpace->ComponentStateChange(body);
    }
    if(collider)
    {
      //make sure we're in the right broadphase by removing/Insert
      //into it again (does nothing if we're in the same broadphase)
      bool inBroadphase = RemoveFromBroadPhase(collider);
      InsertIntoBroadPhase(collider);
      //since we may have also moved due to a hierarchy change,
      //update ourself in the broadphase just in case
      //(only do this if we were in a broadphase, otherwise
      //the update will crash because the proxy is invalid)
      if(inBroadphase)
        UpdateInBroadPhase(collider);

      collider->mSpace->ComponentStateChange(collider);
    }
  }
};

template <typename Functor>
void FullRecurseDown(PhysicsNode* root, Functor& functor)
{
  //do a depth first traversal of the physics nodes and call the
  //passed in functor on all of them
  Array<PhysicsNode*> stack;
  stack.Reserve(64);
  stack.PushBack(root);

  while(!stack.Empty())
  {
    PhysicsNode* node = stack.Back();
    stack.PopBack();

    functor.Action(node);

    PhysicsNode::ChildrenList::range r = node->mChildren.All();
    for(; !r.Empty(); r.PopFront())
      stack.PushBack(&r.Front());
  }
}

void ChangeInWorld(PhysicsNode* node)
{
  //recursively set each child
  PhysicsNode::ChildrenList::range r = node->mChildren.All();
  for(; !r.Empty(); r.PopFront())
    ChangeInWorld(&r.Front());

  //update the in world flag last (bottom-up)
  RigidBody* body = node->mBody;
  Collider* collider = node->mCollider;

  //make sure the in world state of the transform is set correctly
  if(body)
  {
    Transform* t = body->GetOwner()->has(Transform);
    bool inWorld = body->IsDynamic();
    if(!t->GetSpace()->IsEditorMode())
      t->SetInWorld(inWorld);
  }
  else
  {
    Transform* t = collider->GetOwner()->has(Transform);
    t->SetInWorld(false);
  }
}

void CompleteTreeRebuild(PhysicsNode* root)
{
  //do one pass to clear all the old links
  ClearFunctor clearFunctor;
  FullRecurseDown(root, clearFunctor);
  //then do a second pass to re-link and queue changes
  CompleteBuildFunctor buildFunctor;
  FullRecurseDown(root, buildFunctor);

  //we have to change the in world flag bottom-up instead of top down to help
  //make sure the transforms are as exact as possible (otherwise the decomposed
  //error will propagate moving down the tree and cause more errors that can be very visible)
  ChangeInWorld(root);
}

//when a physics node goes away, we need to relink the tree around it.
//This may however, create multiple new trees (a root node disappears).
void UnlinkNode(PhysicsNode* node)
{
  PhysicsNode* parentNode = node->mParent;
  PhysicsNode::ChildrenRange range = node->mChildren.All();

  //if we have a parent node, link all of our children to our parent
  //then unlink ourself
  if(parentNode != nullptr)
  {
    while(!range.Empty())
    {
      PhysicsNode* child = &(range.Front());
      range.PopFront();

      child->RemoveParent(node);
      child->AddParent(parentNode);
    }
    node->RemoveParent(parentNode);
  }
  //otherwise, unlink our children from ourself. One small extra is that since
  //we've unlinked our children, we have no way to rebuild their tree since we 
  //lose references to them. Therefore, rebuild their tree now.
  else
  {
    while(!range.Empty())
    {
      PhysicsNode* child = &(range.Front());
      range.PopFront();

      child->RemoveParent(node);
      //need to rebuild the tree for the child
      CompleteTreeRebuild(child);
    }
  }
}

template <typename ObjectType1, typename ObjectType2>
void GenericInitialize(ObjectType1* obj, ObjectType2* otherObj, bool dynamicallyCreated)
{
  PhysicsNode* node = nullptr;
  PhysicsNode* parentNode = nullptr;
  Cog* owner = obj->GetOwner();

  //if we don't have our other physics object (collider/body) then we are
  //the first physics object at this level and therefore have to make a node.
  //otherwise, we can just grab the node from our neighbor.
  if(otherObj && otherObj->mPhysicsNode)
  {
    node = otherObj->mPhysicsNode;
    parentNode = node->mParent;
  }
  else
  {
    node = new PhysicsNode();
    //have to give the world transform object a reference to it's transform
    WorldTransformation* transform = node->GetTransform();
    transform->SetTransform(owner->has(Transform));
    parentNode = FindParentNode(owner->GetParent());
    if(parentNode)
      node->AddParent(parentNode);
  }

  node->SetOwner(obj);
  obj->mPhysicsNode = node;

  //need to fix any children of our parent that actually have us as a parent
  //(don't need to do this if we already had our collider/body since it
  //would already have been linked correctly)
  if(dynamicallyCreated && otherObj == nullptr)
  {
    if(parentNode)
      RelinkNodes(owner, parentNode->GetCogOwner(), node, parentNode);
    //if we have no parent, we may still have children that need to find us...
    else
      LinkNewParentNode(node, owner);
  }
}

template <typename ObjectType>
void GenericOnAllObjectsCreated(ObjectType* obj, PhysicsNode* node, bool dynamicallyCreated)
{
  //when we're dynamically created, just rebuild everything
  if(dynamicallyCreated)
    CompleteTreeRebuild(GetTreeRoot(node));
  //when this is during an object's creation, be a tad bit smart
  //and only rebuild the tree if we are the root...
  else
  {
    if(node->mParent == nullptr)
    {
      CompleteTreeRebuild(node);
      //make sure we set the mass and transform properties for the first frame
      obj->mSpace->UpdateTransformAndMassOfTree(node);
    }
  }
}

template <typename ObjectType>
void GenericOnDestroy(ObjectType* obj, PhysicsNode* node, bool dynamicallyDestroyed)
{
  PhysicsNode* root = GetTreeRoot(node);
  //remove ourself from the physics node
  node->RemoveOwner(obj);

  bool nodeIsDying = node->IsDying();
  //if our node is going away, we need to relink our children, parent and ourself
  if(nodeIsDying)
  {
    UnlinkNode(node);
    //if we are dying from being dynamically destroyed, the in world flag will
    //not be properly reset (since we have no more physics components and
    //therefore this node will not be in the tree) so reset it here.
    if(dynamicallyDestroyed)
    {
      Cog* cog = obj->GetOwner();
      Transform* t = cog->has(Transform);
      t->SetInWorld(false);
    }
  }

  if(dynamicallyDestroyed)
  {
    //if the node is not dying, we need to rebuild the tree
    //however, if the node is dying, we only need to rebuild the tree
    //if we are not the root (otherwise the children have been rebuilt in unlink node)
    if(!nodeIsDying || root != node)
      CompleteTreeRebuild(root);
  }
  else
  {
    //nothing to do if we are the root (everyone is going away)
    //also no need to do anything if the root is dying
    if(node == root || root->IsDying())
      return;

    //in the case where a collider and body are being destroyed,
    //we want to only rebuild the tree once (and the second time at that).
    //If we only rebuild the tree once both the collider and body have been
    //removed, then we'll be fine
    if(nodeIsDying)
      CompleteTreeRebuild(root);
  }
}

void BodyInitialize(RigidBody* body, bool dynamicallyCreated)
{
  Collider* collider = body->GetOwner()->has(Collider);
  GenericInitialize(body, collider, dynamicallyCreated);
}

void BodyOnAllObjectsCreated(RigidBody* body, bool dynamicallyCreated)
{
  //only run on all objects created code once. We make the rigid body
  //the component who will not run if there is a collider.
  if(dynamicallyCreated || body->mPhysicsNode->mCollider == nullptr)
    GenericOnAllObjectsCreated(body, body->mPhysicsNode, dynamicallyCreated);
}

void BodyStateChanged(RigidBody* body)
{
  CompleteTreeRebuild(GetTreeRoot(body->mPhysicsNode));
}

void BodyOnDestroy(RigidBody* body, bool dynamicallyDestroyed)
{
  //fixing the tree will unlink and relink all of the colliders, but this
  //body won't be in the tree to do that, so unlink all of it's colliders real quick
  //but make sure that the collider's do not still point to the rigid body
  //(allowing this caused a crash at some point during a wake up, not sure
  //how it could ever happen, but putting this loop in to prevent that).
  RigidBody::CompositeColliderRange range = body->mColliders.All();
  for(; !range.Empty(); range.PopFront())
    range.Front().mActiveRigidBody = nullptr;

  body->mColliders.Clear();
  body->mChildBodies.Clear();
  GenericOnDestroy(body, body->mPhysicsNode, dynamicallyDestroyed);
}

void ColliderInitialize(Collider* collider, bool dynamicallyCreated)
{
  RigidBody* body = collider->GetOwner()->has(RigidBody);
  GenericInitialize(collider, body, dynamicallyCreated);
}

void ColliderOnAllObjectsCreated(Collider* collider, bool dynamicallyCreated)
{
  GenericOnAllObjectsCreated(collider, collider->mPhysicsNode, dynamicallyCreated);
}

void ColliderOnDestroy(Collider* collider, bool dynamicallyDestroyed)
{
  bool worldCollider = collider == collider->mSpace->mWorldCollider;
  PhysicsNode* node = collider->mPhysicsNode;
  //remove from broadphase (special case the world collider which is never in broadphase)
  if(!worldCollider)
    RemoveFromBroadPhase(collider);

  GenericOnDestroy(collider, collider->mPhysicsNode, dynamicallyDestroyed);

  collider->mActiveRigidBody = collider->mDirectRigidBody = nullptr;

  //also, the world collider's node is not managed by the space, so we have to manually destroy it
  if(worldCollider)
    delete node;
}

void PhysicsAttachTo(PhysicsNode* node, AttachmentInfo& info)
{
  //to make sure we don't do this twice, check to see if
  //we are already hooked up, if so don't do anything
  if(node->mParent)
    return;

  //look for any cog that we are attaching to that has physics
  PhysicsNode* parentNode = FindParentNode(info.Parent);

  //if we have no parent with physics, the tree structure hasn't changed, but
  //our transform positions have.
  //Lazy...could prob just queue up the transform updates instead of building the tree.
  if(parentNode == nullptr)
  {
    CompleteTreeRebuild(node);
    return;
  }

  //link up to our parent and fix the tree
  node->AddParent(parentNode);
  CompleteTreeRebuild(GetTreeRoot(parentNode));
}

void PhysicsDetach(PhysicsNode* node, AttachmentInfo& info)
{
  PhysicsNode* parentNode = node->mParent;

  //in the event our parent is already clear, this means we've already called this
  //(might happen with how both collider's and bodies are notified but the logic takes place the same either way)
  if(parentNode == nullptr)
    return;

  //from the parent passed in, find the closest node and get the cog from it
  PhysicsNode* attachParentNode = FindParentNode(info.Parent);
  Cog* attachParentCog = nullptr;
  if(attachParentNode)
    attachParentCog = attachParentNode->GetCogOwner();

  //if we are not the direct child of the parent node, then our parent handles this
  if(attachParentCog != parentNode->GetCogOwner())
    return;

  //unlink and fix the tree (we made 2 trees, so fix both)
  node->RemoveParent(parentNode);
  CompleteTreeRebuild(GetTreeRoot(parentNode));
  CompleteTreeRebuild(GetTreeRoot(node));
}

}//namespace Zero
