///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{
struct Manifold;
}//namespace Physics

/// Allows a user to send out collision events that do not originate from the physics system.
/// This includes determining when to send start/persisted/end events. This is primarily
/// intended for use with a swept character controller where you never actually come into
/// contact with objects. The objects you would have hit can be added to this so that the
/// same logic can be used for swept and non-swept collision.
class CustomCollisionEventTracker : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Add a collision for this frame between ourself and the passed in collider.
  void AddCollision(Collider* otherCollider, Vec3Param worldPoint, Vec3Param worldNormalTowardsOther);
  /// Send out all of the events that have been queued up. This includes determining what
  /// should be started/persisted/ended events. These events are sent with the prefix
  /// that is passed in, e.g. if you pass in 'Collision' then
  /// CollisionStarted, CollisionPersisted, and CollisionEnded will be the event names.
  void SendEvents(StringParam eventPrefix);

  struct CollisionData
  {
    Collider* mOtherCollider;
    CogId mOtherCog;
    Vec3 mWorldPoint;
    Vec3 mWorldNormalTowardsOther;
  };

  void FilloutEvent(CollisionData& data, Physics::Manifold* manifold, CollisionEvent* toSend);
  void SendEventList(Array<CollisionData>& events, StringParam eventName);

  typedef HashMap<CogId, CollisionData> CollisionMap;
  CollisionMap mCollisions;
  CollisionMap mPreviousCollisions;
};

}//namespace Zero
