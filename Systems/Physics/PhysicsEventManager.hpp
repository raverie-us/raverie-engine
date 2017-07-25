///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2016-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class CollisionEvent;
class CollisionGroupEvent;
class JointEvent;
class PhysicsSpace;

namespace Physics
{

/// A structure to handle the creation, storing and sending of all physics events.
/// Mainly used to centralize all logic to this file.
struct PhysicsEventManager
{
  PhysicsEventManager();
  ~PhysicsEventManager();

  void SetAllocator(Memory::Heap* heap);

  // Event Batching
  void BatchCollisionStartedEvent(Manifold* manifold, PhysicsSpace* space);
  void BatchCollisionPersistedEvent(Manifold* manifold, PhysicsSpace* space);
  void BatchCollisionEndedEvent(Manifold* manifold, PhysicsSpace* space, bool immediateSend = false);
  void BatchPreSolveEvent(Manifold* manifold, PhysicsSpace* space);
  JointEvent* BatchJointEvent(Joint* joint, StringParam eventName);
  
  void DispatchEvents(PhysicsSpace* space);
  void DispatchPreSolveEvents(PhysicsSpace* space);

private:
  CollisionFilter* GetFilter(Manifold* manifold, PhysicsSpace* space);

  void CreateEvent(Physics::Manifold* manifold, PhysicsSpace* space, uint collisionType, uint filterFlag, bool immediateSend = false);
  void CreateCollisionEvent(Manifold* manifold, uint contactId, uint eventType, StringParam collisionType, bool immediateSend = false);
  CollisionEvent* CreateCollisionEventInternal(Manifold* manifold, ManifoldPoint& point, uint eventType, StringParam collisionType);
  
  void AddEvent(CollisionEvent* event);
  void AddEvent(CollisionGroupEvent* event);
  void AddEvent(JointEvent* event);

  void DispatchEvent(CollisionEvent* toSend);
  void DispatchEvent(PhysicsSpace* space, CollisionGroupEvent* toSend);
  void DispatchGroupEvent(Cog* obj, CollisionGroupEvent* eventObj, uint filterFlag);
  
  typedef Array<CollisionEvent*> CollisionEvents;
  typedef Array<CollisionGroupEvent*> CollisionGroupEvents;
  typedef InList<JointEvent> JointEventList;
  typedef InList<PreSolveEvent, &PreSolveEvent::mEventLink> PreSolveEventList;

  CollisionEvents mCollisionEvents;
  CollisionGroupEvents mCollisionGroupEvents;
  JointEventList mJointEvents;
  PreSolveEventList mPreSolveEvents;
};

}//namespace Physics

}//namespace Zero
