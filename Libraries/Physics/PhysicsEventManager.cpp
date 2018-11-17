///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2016-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Physics
{

PhysicsEventManager::PhysicsEventManager()
{

}

PhysicsEventManager::~PhysicsEventManager()
{
  // Delete all of the collision events so that we don't have a memory
  // leak when they kill a space

  CollisionEvents::range range = mCollisionEvents.All();
  while(!range.Empty())
  {
    CollisionEvent* e = range.Front();
    range.PopFront();
    delete e;
  }
  mCollisionEvents.Clear();

  JointEventList::range jointRange = mJointEvents.All();
  while(!jointRange.Empty())
  {
    JointEvent* event = &(jointRange.Front());
    jointRange.PopFront();
    delete event;
  }
  mJointEvents.Clear();

  CollisionGroupEvents::range groupRange = mCollisionGroupEvents.All();
  while(!groupRange.Empty())
  {
    CollisionGroupEvent* e = groupRange.Front();
    groupRange.PopFront();
    delete e;
  }
  mCollisionGroupEvents.Clear();
}

void PhysicsEventManager::SetAllocator(Memory::Heap* heap)
{
  mCollisionEvents.SetAllocator(HeapAllocator(heap));
  mCollisionGroupEvents.SetAllocator(HeapAllocator(heap));
}

void PhysicsEventManager::BatchCollisionStartedEvent(Manifold* manifold, PhysicsSpace* space)
{
  CreateEvent(manifold, space, BaseCollisionEvent::CollisionStarted, FilterFlags::StartEvent);
}

void PhysicsEventManager::BatchCollisionPersistedEvent(Manifold* manifold, PhysicsSpace* space)
{
  CreateEvent(manifold, space, BaseCollisionEvent::CollisionPersisted, FilterFlags::PersistedEvent);
}

void PhysicsEventManager::BatchCollisionEndedEvent(Manifold* manifold, PhysicsSpace* space, bool immediateSend)
{
  CreateEvent(manifold, space, BaseCollisionEvent::CollisionEnded, FilterFlags::EndEvent, immediateSend);
}

void PhysicsEventManager::BatchPreSolveEvent(Manifold* manifold, PhysicsSpace* space)
{
  // If the filter doesn't exist, there's nothing to do
  CollisionFilter* f = GetFilter(manifold, space);
  if(f == nullptr)
    return;

  // If the flag is not set, don't bother searching
  if(!f->mFilterFlags.IsSet(FilterFlags::PreSolveEvent))
    return;

  // Find the appropriate block (n search of 4 items..., could pre-sort if I care later)
  CollisionFilterBlock* block = nullptr;
  for(uint i = 0; i < f->mBlocks.Size(); ++i)
  {
    if(f->mBlocks[i]->mBlockType == FilterFlags::PreSolveEvent)
    {
      block = f->mBlocks[i];
      break;
    }
  }
  // If we didn't find it bail
  if(block == nullptr)
    return;

  // Now batch the event to be sent
  PreSolveEvent* e = new PreSolveEvent();
  e->Set(manifold, block);
  // Fix the order to match the filter
  e->MatchCollisionFilterOrder(f);
  mPreSolveEvents.PushBack(e);
}

JointEvent* PhysicsEventManager::BatchJointEvent(Joint* joint, StringParam eventName)
{
  JointEvent* event = new JointEvent();
  event->mJoint = joint;
  event->mEventType = eventName;
  event->mColliderA = joint->GetCollider(0);
  event->mColliderB = joint->GetCollider(1);
  AddEvent(event);
  return event;
}

void PhysicsEventManager::DispatchEvents(PhysicsSpace* space)
{
  CollisionEvents::range range = mCollisionEvents.All();
  for(; !range.Empty(); range.PopFront())
  {
    CollisionEvent* eventObj = range.Front();
    DispatchEvent(eventObj);
  }
  mCollisionEvents.Clear();

  // Safe as long as we still have events
  while(!mJointEvents.Empty())
  {
    JointEvent* eventObj = &(mJointEvents.Front());
    mJointEvents.PopFront();

    EventDispatcher* dispatcher = eventObj->mJoint->GetOwner()->GetDispatcher();
    dispatcher->Dispatch(eventObj->mEventType, eventObj);
    delete eventObj;
  }

  // Send the group events to objectA, B or the space if they want it
  CollisionGroupEvents::range groupRange = mCollisionGroupEvents.All();
  for(; !groupRange.Empty(); groupRange.PopFront())
  {
    CollisionGroupEvent* eventObj = groupRange.Front();
    DispatchEvent(space, eventObj);
  }
  mCollisionGroupEvents.Clear();
}

void PhysicsEventManager::DispatchPreSolveEvents(PhysicsSpace* space)
{
  while(!mPreSolveEvents.Empty())
  {
    PreSolveEvent* e = &mPreSolveEvents.Front();
    mPreSolveEvents.PopFront();

    // Cache the cogs we might send events to (cleans up the object index swapping)
    Cog* objA = e->GetObject();
    Cog* objB = e->GetOtherObject();

    // Send to A if the block allows us to
    if(e->mBlock->mStates.IsSet(CollisionBlockStates::SendEventsToA))
      objA->GetDispatcher()->Dispatch(e->EventId, e);
    // Send to B (make sure to swap the index first)
    if(e->mBlock->mStates.IsSet(CollisionBlockStates::SendEventsToB))
    {
      e->mObjectIndex = !e->mObjectIndex;
      objB->GetDispatcher()->Dispatch(e->EventId, e);
    }
    //send to the space (it gets B's ordering, but that shouldn't really matter)
    if(e->mBlock->mStates.IsSet(CollisionBlockStates::SendEventsToSpace))
      space->GetOwner()->GetDispatcher()->Dispatch(e->EventId, e);

    delete e;
  }
}

CollisionFilter* PhysicsEventManager::GetFilter(Manifold* manifold, PhysicsSpace* space)
{
  CollisionGroupInstance* instance1 = manifold->Objects[0]->mCollisionGroupInstance;
  CollisionGroupInstance* instance2 = manifold->Objects[1]->mCollisionGroupInstance;

  CollisionFilter pair(instance1->mResource->mResourceId, instance2->mResource->mResourceId);
  // Find the filter for this pair
  return space->mCollisionTable->FindFilter(pair);
}

void PhysicsEventManager::CreateEvent(Physics::Manifold* manifold, PhysicsSpace* space,
                                      uint collisionType, uint blockType, bool immediateSend)
{
  // Always send the normal collision event (whether or not this is
  // correct behavior I'll worry about later, right now it should never be blocked though)
  BaseCollisionEvent::CollisionType typedCollision = (BaseCollisionEvent::CollisionType)collisionType;
  this->CreateCollisionEvent(manifold, 0, collisionType, CollisionEvent::GetEventName(typedCollision), immediateSend);

  // Grab the filter for the two objects in the manifold
  CollisionFilter* f = GetFilter(manifold, space);
  
  // If there was no filter, we can't do anything
  if(f == nullptr)
    return;

  // Otherwise, first check to see if the filter has the block type we're
  // looking for, if it doesn't there's no point in continuing
  CollisionFilter& filter = *f;
  if(!filter.mFilterFlags.IsSet(blockType))
    return;
  
  // Now find the appropriate block by searching for it
  // (n search of 4 items..., could pre-sort if I care later)
  CollisionFilterBlock* block = nullptr;
  for(uint i = 0; i < filter.mBlocks.Size(); ++i)
  {
    if(filter.mBlocks[i]->mBlockType == blockType)
    {
      block = filter.mBlocks[i];
      break;
    }
  }

  // If we didn't find the block then there's no events to send
  if(block == nullptr)
    return;

  // See if the block sends out the normal event name or
  // if it has a custom event name being sent
  String eventName;
  if(!block->mEventOverride.Empty())
    eventName = block->mEventOverride;
  else
    eventName = CollisionGroupEvent::GetEventName(typedCollision);

  // Batch the group event now
  CollisionGroupEvent* eventObj = new CollisionGroupEvent();
  eventObj->Set(manifold, filter, block, eventName);
  // Fix the order to match the filter
  eventObj->MatchCollisionFilterOrder(f);
  eventObj->mCollisionType = typedCollision;

  // If we need to send the even right away (object destruction) then do so,
  // otherwise queue up the event to be batch sent with all the others
  if(immediateSend)
    DispatchEvent(space, eventObj);
  else
    AddEvent(eventObj);
}

void PhysicsEventManager::CreateCollisionEvent(Manifold* manifold, uint contactId, uint eventType, StringParam collisionType, bool immediateSend)
{
  ManifoldPoint& point = manifold->Contacts[contactId];
  CollisionEvent* eventObj;
  eventObj = CreateCollisionEventInternal(manifold, point, eventType, collisionType);
  if(eventObj)
  {
    eventObj->mContactIndex = contactId;

    // If we immediate send then dispatch now (object destruction) otherwise batch it up
    if(immediateSend)
      DispatchEvent(eventObj);
    else
      AddEvent(eventObj);
  }
}

CollisionEvent* PhysicsEventManager::CreateCollisionEventInternal(Manifold* manifold, ManifoldPoint& point, uint eventType, StringParam collisionType)
{
  // Make sure that the objects send events (optimization) and events of type
  if(!manifold->GetSendsMessages())
    return nullptr;

  CollisionEvent* eventObj = new CollisionEvent();
  eventObj->mCollisionType = (BaseCollisionEvent::CollisionType)eventType;
  eventObj->Set(manifold, point, collisionType);
  return eventObj;
}

void PhysicsEventManager::AddEvent(CollisionEvent* eventObj)
{
  mCollisionEvents.PushBack(eventObj);
}

void PhysicsEventManager::AddEvent(CollisionGroupEvent* eventObj)
{
  mCollisionGroupEvents.PushBack(eventObj);
}

void PhysicsEventManager::AddEvent(JointEvent* eventObj)
{
  mJointEvents.PushBack(eventObj);
}

void PhysicsEventManager::DispatchEvent(CollisionEvent* toSend)
{
  // Update the cached point that triggered the event (to grab impulse info mainly)
  toSend->UpdatePoint();

  // Cache the two objects we might send events to
  // (this cleans up the event object index swapping)
  Collider* objA = toSend->GetCollider();
  Collider* objB = toSend->GetOtherCollider();

  // Dispatch to A if it sends events
  if(objA->mState.IsSet(ColliderFlags::SendsEvents))
    objA->GetDispatcher()->Dispatch(toSend->mEventType, toSend);
  // Same for B, however first swap the object index
  // (effectively swaps the internal data such that A is now B and B is now A)
  if(objB->mState.IsSet(ColliderFlags::SendsEvents))
  {
    toSend->mObjectIndex = !toSend->mObjectIndex;
    objB->GetDispatcher()->Dispatch(toSend->mEventType, toSend);
  }

  delete toSend;
}

void PhysicsEventManager::DispatchEvent(PhysicsSpace* space, CollisionGroupEvent* toSend)
{
  // Cache the cogs we might send events to (cleans up the object index swapping)
  Cog* objA = toSend->GetObject();
  Cog* objB = toSend->GetOtherObject();

  // Try to send events to A
  DispatchGroupEvent(objA, toSend, CollisionBlockStates::SendEventsToA);
  // Try to send events to B (swap the info such that B now becomes A)
  toSend->mObjectIndex = !toSend->mObjectIndex;
  DispatchGroupEvent(objB, toSend, CollisionBlockStates::SendEventsToB);
  // Try to send to the space (the space will get the swapped
  // event, but no order is really guaranteed here...)
  DispatchGroupEvent(space->GetOwner(), toSend, CollisionBlockStates::SendEventsToSpace);

  delete toSend;
}

void PhysicsEventManager::DispatchGroupEvent(Cog* obj, CollisionGroupEvent* eventObj, uint filterFlag)
{
  if(!eventObj->mBlock->mStates.IsSet(filterFlag))
    return;

  EventDispatcher* dispatcher = obj->GetDispatcher();
  dispatcher->Dispatch(eventObj->mEventType, eventObj);
}

}//namespace Physics

}//namespace Zero
