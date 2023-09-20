// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
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
} // namespace Events

RaverieDefineType(BaseCollisionEvent, builder, type)
{
  RaverieBindDocumented();

  RaverieBindGetterProperty(Object);
  RaverieBindGetterProperty(OtherObject);

  RaverieBindGetterProperty(ContactPointCount);
  RaverieBindGetterProperty(IsGhost);

  RaverieBindGetterProperty(ContactPoints);
}

BaseCollisionEvent::BaseCollisionEvent()
{
  mManifold = nullptr;
  mObjectIndex = 0;
}

void BaseCollisionEvent::Set(Physics::Manifold* manifold, StringParam eventType)
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
  // To help make SendToA and SendToB make more sense, flip the event's order to
  // match the same as the filter. This means SendToA will send to the collider
  // with the same collision group as the first one in the filter. If both
  // collision groups are the same then no flip happens but there's no logical
  // order then anyways.
  Collider* firstCollider = GetCollider();
  ResourceId firstGroupId = firstCollider->mCollisionGroupInstance->mResource->mResourceId;
  if (firstGroupId != filter->first())
    mObjectIndex = !mObjectIndex;
}

const Physics::ManifoldPoint& BaseCollisionEvent::GetPoint(uint index)
{
  ErrorIf(index >= GetContactPointCount(), "Point index is greater than the number of contact points.");
  return mManifold->Contacts[index];
}

RaverieDefineType(CollisionEvent, builder, type)
{
  RaverieBindDocumented();

  RaverieBindGetterProperty(FirstPoint);

  RaverieBindTag(Tags::Physics);
}

CollisionEvent::CollisionEvent()
{
  mContactIndex = 0;
}

void CollisionEvent::Set(Physics::Manifold* manifold, StringParam eventType)
{
  BaseCollisionEvent::Set(manifold, eventType);
  mContactIndex = 0;
  UpdatePoint();
}

void CollisionEvent::Set(Physics::Manifold* manifold, const Physics::ManifoldPoint& point, StringParam eventType)
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
  if (mCollisionType == CollisionStarted || mCollisionType == CollisionPersisted)
    mContactPoint = mManifold->Contacts[mContactIndex];
}

String CollisionEvent::GetEventName(BaseCollisionEvent::CollisionType type)
{
  if (type == BaseCollisionEvent::CollisionStarted)
    return Events::CollisionStarted;
  else if (type == BaseCollisionEvent::CollisionPersisted)
    return Events::CollisionPersisted;
  else
    return Events::CollisionEnded;
}

RaverieDefineType(CollisionGroupEvent, builder, type)
{
  RaverieBindDocumented();

  RaverieBindGetterProperty(TypeAName);
  RaverieBindGetterProperty(TypeBName);
  RaverieBindGetterProperty(FirstPoint);

  RaverieBindTag(Tags::Physics);
}

CollisionGroupEvent::CollisionGroupEvent()
{
}

void CollisionGroupEvent::Set(Physics::Manifold* manifold,
                              const CollisionFilter& pair,
                              CollisionFilterBlock* block,
                              StringParam eventType)
{
  BaseCollisionEvent::Set(manifold, eventType);
  mBlock = block;

  mTypeAName = pair.GetTypeAName();
  mTypeBName = pair.GetTypeBName();

  // Put the objects in the same ordering as the pair
  if (manifold->Objects[0]->mCollisionGroupInstance->mResource->mResourceId != pair.TypeA)
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
  if (type == BaseCollisionEvent::CollisionStarted)
    return Events::GroupCollisionStarted;
  else if (type == BaseCollisionEvent::CollisionPersisted)
    return Events::GroupCollisionPersisted;
  else
    return Events::GroupCollisionEnded;
}

RaverieDefineType(PreSolveEvent, builder, type)
{
  RaverieBindDocumented();

  RaverieBindGetterSetter(Restitution);
  RaverieBindGetterSetter(Friction);

  RaverieBindTag(Tags::Physics);
}

PreSolveEvent::PreSolveEvent()
{
  mBlock = nullptr;
}

void PreSolveEvent::Set(Physics::Manifold* manifold, CollisionFilterBlock* preSolveBlock)
{
  BaseCollisionEvent::Set(manifold, String());
  mBlock = preSolveBlock;

  if (!mBlock->mEventOverride.Empty())
    EventId = mBlock->mEventOverride;
  else
    EventId = Events::GroupCollisionPreSolve;
}

real PreSolveEvent::GetRestitution()
{
  return mManifold->Restitution;
}

void PreSolveEvent::SetRestitution(real restitution)
{
  mManifold->Restitution = restitution;
}

real PreSolveEvent::GetFriction()
{
  return mManifold->DynamicFriction;
}

void PreSolveEvent::SetFriction(real friction)
{
  mManifold->DynamicFriction = friction;
}

} // namespace Raverie
