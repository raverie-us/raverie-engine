///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(RigidBodySlept);
DefineEvent(RigidBodyAwoke);
DefineEvent(CollisionStarted);
DefineEvent(CollisionPersisted);
DefineEvent(CollisionEnded);
DefineEvent(GroupCollisionStarted);
DefineEvent(GroupCollisionPersisted);
DefineEvent(GroupCollisionEnded);
DefineEvent(GroupCollisionPreSolve);
DefineEvent(PhysicsUpdateFinished);
}//namespace Events


//-------------------------------------------------------------------BaseCollisionEvent
ZilchDefineType(BaseCollisionEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterProperty(Object);
  ZilchBindGetterProperty(OtherObject);

  ZilchBindGetterProperty(ContactPointCount);
  ZilchBindGetterProperty(IsGhost);

  ZilchBindGetterProperty(ContactPoints);
}

BaseCollisionEvent::BaseCollisionEvent()
{
  mManifold = nullptr;
  mObjectIndex = 0;
}

void BaseCollisionEvent::Set(const Physics::Manifold* manifold, StringParam eventType)
{
  mManifold = manifold;
  mEventType = eventType;
}

Cog* BaseCollisionEvent::GetObject()
{
  return mManifold->Objects[mObjectIndex]->GetOwner();
}

Cog* BaseCollisionEvent::GetOtherObject()
{
  return mManifold->Objects[(mObjectIndex + 1) % 2]->GetOwner();
}

bool BaseCollisionEvent::GetIsGhost()
{
  bool aGhost = false;
  bool bGhost = false;
  aGhost = GetCollider()->NotCollideable();
  bGhost = GetOtherCollider()->NotCollideable();
  return aGhost || bGhost;
}

uint BaseCollisionEvent::GetContactPointCount()
{
  return mManifold->ContactCount;
}

ContactPointRange BaseCollisionEvent::GetContactPoints()
{
  ContactPointRange range;
  range.Set(mManifold, mObjectIndex);
  return range;
}

ContactPoint BaseCollisionEvent::GetFirstPoint()
{
  ContactPoint point;
  point.Set(mManifold, &mManifold->Contacts[0], mObjectIndex);
  return point;
}

Collider* BaseCollisionEvent::GetCollider()
{
  return mManifold->Objects[mObjectIndex];
}

Collider* BaseCollisionEvent::GetOtherCollider()
{
  return mManifold->Objects[(mObjectIndex + 1) % 2];
}

void BaseCollisionEvent::MatchCollisionFilterOrder(CollisionFilter* filter)
{
  // To help make SendToA and SendToB make more sense, flip the event's order to match the same as the filter.
  // This means SendToA will send to the collider with the same collision group as the first one in the filter. 
  // If both collision groups are the same then no flip happens but there's no logical order then anyways.
  Collider* firstCollider = GetCollider();
  ResourceId firstGroupId = firstCollider->mCollisionGroupInstance->mResource->mResourceId;
  if(firstGroupId != filter->first())
    mObjectIndex = !mObjectIndex;
}

const Physics::ManifoldPoint& BaseCollisionEvent::GetPoint(uint index)
{
  ErrorIf(index >= GetContactPointCount(), "Point index is greater than the number of contact points.");
  return mManifold->Contacts[index];
}

//-------------------------------------------------------------------CollisionEvent
ZilchDefineType(CollisionEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterProperty(FirstPoint);

  ZeroBindTag(Tags::Physics);
}

CollisionEvent::CollisionEvent()
{
  mContactIndex = 0;
}

void CollisionEvent::Set(const Physics::Manifold* manifold, const Physics::ManifoldPoint& point, StringParam eventType)
{
  BaseCollisionEvent::Set(manifold, eventType);
  mContactPoint = point;
}

ContactPoint CollisionEvent::GetFirstPoint()
{
  ContactPoint point;
  point.Set(mManifold, &mContactPoint, mObjectIndex);
  return point;
}

void CollisionEvent::UpdatePoint()
{
  if(mCollisionType == CollisionStarted || mCollisionType == CollisionPersisted)
    mContactPoint = mManifold->Contacts[mContactIndex];
}

String CollisionEvent::GetEventName(BaseCollisionEvent::CollisionType type)
{
  if(type == BaseCollisionEvent::CollisionStarted)
    return Events::CollisionStarted;
  else if(type == BaseCollisionEvent::CollisionPersisted)
    return Events::CollisionPersisted;
  else
    return Events::CollisionEnded;
}

//-------------------------------------------------------------------CollisionGroupEvent
ZilchDefineType(CollisionGroupEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterProperty(TypeAName);
  ZilchBindGetterProperty(TypeBName);
  ZilchBindGetterProperty(FirstPoint);

  ZeroBindTag(Tags::Physics);
}

CollisionGroupEvent::CollisionGroupEvent()
{
  
}

void CollisionGroupEvent::Set(const Physics::Manifold* manifold, const CollisionFilter& pair, CollisionFilterBlock* block, StringParam eventType)
{
  BaseCollisionEvent::Set(manifold,eventType);
  mBlock = block;

  mTypeAName = pair.GetTypeAName();
  mTypeBName = pair.GetTypeBName();
  
  // Put the objects in the same ordering as the pair
  if(manifold->Objects[0]->mCollisionGroupInstance->mResource->mResourceId != pair.TypeA)
    mObjectIndex = !mObjectIndex;
}

String CollisionGroupEvent::GetTypeAName()
{
  return mTypeAName;
}

String CollisionGroupEvent::GetTypeBName()
{
  return mTypeBName;
}

String CollisionGroupEvent::GetEventName(BaseCollisionEvent::CollisionType type)
{
  if(type == BaseCollisionEvent::CollisionStarted)
    return Events::GroupCollisionStarted;
  else if(type == BaseCollisionEvent::CollisionPersisted)
    return Events::GroupCollisionPersisted;
  else
    return Events::GroupCollisionEnded;
}

//-------------------------------------------------------------------PreSolveEvent
ZilchDefineType(PreSolveEvent, builder, type)
{
  ZeroBindDocumented();

  ZeroBindTag(Tags::Physics);
}

PreSolveEvent::PreSolveEvent()
{
  mBlock = nullptr;
}

void PreSolveEvent::Set(const Physics::Manifold* manifold, CollisionFilterBlock* preSolveBlock)
{
  BaseCollisionEvent::Set(manifold, String());
  mBlock = preSolveBlock;

  if(!mBlock->mEventOverride.Empty())
    EventId = mBlock->mEventOverride;
  else
    EventId = Events::GroupCollisionPreSolve;
}

}//namespace Zero
