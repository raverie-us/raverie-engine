///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(RigidBodySlept);
  DeclareEvent(RigidBodyAwoke);
  DeclareEvent(CollisionStarted);
  DeclareEvent(CollisionPersisted);
  DeclareEvent(CollisionEnded);
  DeclareEvent(GroupCollisionStarted);
  DeclareEvent(GroupCollisionPersisted);
  DeclareEvent(GroupCollisionEnded);
  DeclareEvent(GroupCollisionPreSolve);
  DeclareEvent(PhysicsUpdateFinished);
}//namespace Events

struct CollisionFilter;
struct CollisionFilterBlock;

namespace Physics
{
  struct Manifold;
  struct ManifoldPoint;
}//namespace Physics

//--------------------------------------------------------- Base Collision Event
/// Common interface for all collision events. Contains shared methods
/// to access contact information for a collision.
class BaseCollisionEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  BaseCollisionEvent();
  void Set(const Physics::Manifold* manifold, StringParam eventType);
  
  /// The object that this event was sent to.
  Cog* GetObject();
  /// The other object in this collision.
  Cog* GetOtherObject();

  /// If this was a ghost collision (detected but not resolved).
  bool GetIsGhost();
  /// The number of contact points in this collision.
  uint GetContactPointCount();
  /// A range for iterating through all contact points.
  ContactPointRange GetContactPoints();

  /// Convenience function to return the first ContactPoint. Some logic only cares about
  /// one point of information. In a more general case all points should be iterated over.
  ContactPoint GetFirstPoint();

  // C++ helpers for those who want to efficiently fetch the collider
  // (we already store the collider internally, so this avoids a
  // collider->GetOwner()->has(Collider) and just returns collider).
  Collider* GetCollider();
  Collider* GetOtherCollider();

  // Update the collider order (who is "A" and who is "B") to match the filter's order.
  void MatchCollisionFilterOrder(CollisionFilter* filter);
  
  /// Returns the manifold point at the given index. This is
  /// for C++ use and only for those who know what they are doing.
  const Physics::ManifoldPoint& GetPoint(uint index);

  /// Pointer to the manifold for the collision to pull out extra data.
  const Physics::Manifold* mManifold;
  /// Used to determine what kind of collision this is during event sending.
  enum CollisionType{CollisionStarted, CollisionPersisted, CollisionEnded};
  CollisionType mCollisionType;
  /// The event name that will be sent (CollisionStarted, etc...)
  String mEventType;
  /// Internal index used to "flip the manifold data" when sending to the second
  /// object. This effectively reverses all the data (aka, the normal is negated for object B).
  uint mObjectIndex;
};

//-------------------------------------------------------------- Collision Event
/// Information about a collision in physics. Sent when collisions start, persist, or end.
class CollisionEvent : public BaseCollisionEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CollisionEvent();
  void Set(const Physics::Manifold* manifold, const Physics::ManifoldPoint& point, StringParam eventType);

  /// Convenience function to return the first ContactPoint. Some logic only cares about
  /// one point of information. In a more general case all points should be iterated over.
  ContactPoint GetFirstPoint();

  // @JoshD: Legacy?
  /// Used to update the values in the event for the incident point before
  /// we send the event (some values might have changed after solving).
  void UpdatePoint();

  // @JoshD: Legacy?
  /// The point that triggered the Start/End event
  Physics::ManifoldPoint mContactPoint;
  /// The index of the point. Used to update the point before
  /// sending (since we now know the impulse magnitude)
  uint mContactIndex;

  static String GetEventName(BaseCollisionEvent::CollisionType type);
};

//-------------------------------------------------------- CollisionGroupEvent
/// An event sent out when specified by a CollisionFilter in a CollisionTable.
/// Used to hook up events based upon certain CollisionGroup types colliding.
class CollisionGroupEvent : public BaseCollisionEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CollisionGroupEvent();

  /// Sets the two colliders with the given pair. Takes care of making sure the
  /// collider ordering matches the filter order.
  void Set(const Physics::Manifold* manifold, const CollisionFilter& pair, CollisionFilterBlock* block, StringParam eventType);

  /// Returns the CollisionGroup name of object A
  String GetTypeAName();
  /// Returns the CollisionGroup name of object B
  String GetTypeBName();

  String mTypeAName;
  String mTypeBName;
  CollisionFilterBlock* mBlock;

  static String GetEventName(BaseCollisionEvent::CollisionType type);

private:
  friend class PhysicsSpace;
};

//-------------------------------------------------------------------PreSolveEvent
/// Event sent out when a CollisionFilter contains a PreSolveBlock. This
/// event is sent out after collision detection but before collision resolution.
/// This can be used to alter the state of the two objects in a collision
/// before they're resolved (e.g. turn one from static to dynamic). Warning: Do not
/// delete objects or do any other significant changes as this may destabilize the system.
class PreSolveEvent : public BaseCollisionEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

   PreSolveEvent();
   void Set(const Physics::Manifold* manifold, CollisionFilterBlock* preSolveBlock);

   CollisionFilterBlock* mBlock;
   Link<PreSolveEvent> mEventLink;
};

}//namespace Zero
