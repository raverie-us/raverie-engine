///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(CustomCollisionEventTracker, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  // This works with physics, we must have a collider (otherwise collision events would need to change)
  ZeroBindDependency(Collider);
  ZeroBindDependency(Cog);

  ZilchBindMethod(AddCollision);
  ZilchBindMethod(SendEvents);
}

void CustomCollisionEventTracker::AddCollision(Collider* otherCollider, Vec3Param worldPoint, Vec3Param worldNormalTowardsOther)
{
  if(otherCollider == nullptr)
  {
    String msg = "CustomCollisionEventTracker expects a valid collider.";
    DoNotifyException("Invalid Collider", msg);
    return;
  }

  // Fill out the data to save for later
  CollisionData data;
  data.mOtherCollider = otherCollider;
  data.mOtherCog = otherCollider->GetOwner();
  data.mWorldPoint = worldPoint;
  data.mWorldNormalTowardsOther = worldNormalTowardsOther;
  // We only allow one collision per other cog for now, I'll consider more if someone wants that
  mCollisions[data.mOtherCog] = data;
}

void CustomCollisionEventTracker::SendEvents(StringParam eventPrefix)
{
  Array<CollisionData> collisionsStarted, collisionsEnded, collisionsPersisted;

  // Check our current collision to see whether they're collision started or persisted events
  CollisionMap::range range = mCollisions.All();
  for(; !range.Empty(); range.PopFront())
  {
    CollisionData& data = range.Front().second;

    // The other cog could have been destroyed between the user adding it and sending events
    Cog* otherCog = data.mOtherCog;
    if(otherCog == nullptr || otherCog->has(Collider) == nullptr)
      continue;

    // If we weren't in contact last frame then this is a collision started event
    CollisionData* previousData = mPreviousCollisions.FindPointer(data.mOtherCog);
    if(previousData == nullptr)
      collisionsStarted.PushBack(data);
    else
      collisionsPersisted.PushBack(data);
  }
  // Now we have to check for collision ended, this happens when we
  // were in contact last frame but no longer are this frame
  range = mPreviousCollisions.All();
  for(; !range.Empty(); range.PopFront())
  {
    CollisionData& data = range.Front().second;
    Cog* otherCog = data.mOtherCog;
    if(otherCog == nullptr || otherCog->has(Collider) == nullptr)
      continue;

    CollisionData* newData = mCollisions.FindPointer(data.mOtherCog);
    if(newData == nullptr)
      collisionsEnded.PushBack(data);
  }

  String startEventName = BuildString(eventPrefix, "Started");
  String persistEventName = BuildString(eventPrefix, "Persisted");
  String endEventName = BuildString(eventPrefix, "Ended");

  SendEventList(collisionsStarted, startEventName);
  SendEventList(collisionsPersisted, persistEventName);
  SendEventList(collisionsEnded, endEventName);

  mPreviousCollisions = mCollisions;
  mCollisions.Clear();
}

void CustomCollisionEventTracker::FilloutEvent(CollisionData& data, Physics::Manifold* manifold, CollisionEvent* toSend)
{
  // Set the other collider for the manifold (we were set already outside of this function)
  manifold->Objects[1] = data.mOtherCog.has(Collider);

  // Hardcoded to 1 point for now
  Physics::ManifoldPoint& point = manifold->Contacts[0];
  manifold->ContactCount = 1;

  point.WorldPoints[0] = point.WorldPoints[1] = data.mWorldPoint;
  // Have to update the body points from the world points
  point.BodyPoints[0] = Physics::JointHelpers::WorldPointToBodyR(manifold->Objects[0], point.WorldPoints[0]);
  point.BodyPoints[1] = Physics::JointHelpers::WorldPointToBodyR(manifold->Objects[1], point.WorldPoints[1]);
  point.Normal = data.mWorldNormalTowardsOther;
  // Hardcoded for now, should this be exposed?
  point.Penetration = 0;
  point.AccumulatedImpulse = Vec3::cZero;

  // Calling FirstPoint returns a contact point stored on the event, 
  // not the manifold. Make sure to set that to the correct value.
  toSend->mContactPoint = point;
}

void CustomCollisionEventTracker::SendEventList(Array<CollisionData>& events, StringParam eventName)
{
  Physics::Manifold tempManifold;
  tempManifold.Objects[0] = GetOwner()->has(Collider);
  CollisionEvent toSend;
  toSend.mEventType = eventName;
  toSend.mManifold = &tempManifold;

  for(uint i = 0; i < events.Size(); ++i)
  {
    FilloutEvent(events[i], &tempManifold, &toSend);

    // Send to ourself
    toSend.mObjectIndex = 0;
    GetOwner()->GetDispatcher()->Dispatch(eventName, &toSend);
    
    // Send to the other collider (making sure to flip the data)
    toSend.mObjectIndex = 1;
    events[i].mOtherCollider->GetDispatcher()->Dispatch(eventName, &toSend);
  }
}

}//namespace Zero
