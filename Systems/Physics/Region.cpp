///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(Region, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZeroBindDependency(Cog);
  ZeroBindDependency(Collider);

  ZilchBindFieldProperty(mWakeUpOnEffectChange);

  ZilchBindMethod(DispatchEvent);
  ZeroBindTag(Tags::Physics);
}

Region::Region()
{
  mCollider = nullptr;
}

void Region::Serialize(Serializer& stream)
{
  SerializeNameDefault(mWakeUpOnEffectChange, false);
}

void Region::Initialize(CogInitializer& initializer)
{
  mSpace = initializer.mSpace->has(PhysicsSpace);
  mCollider = GetOwner()->has(Collider);

  ErrorIf(mSpace == nullptr, "Region's parent has no physics space.");
  ErrorIf(mCollider == nullptr, "Regions must have a Collider Component.");

  mSpace->AddComponent(this);
}

void Region::OnDestroy(uint flags /* = 0 */)
{
  mSpace->RemoveComponent(this);
}

void Region::DispatchEvent(StringParam eventId, Event* toSend)
{
  // Dispatch an event on all objects in contact with the region
  RegionContactRange r = All();
  while(!r.Empty())
  {
    Collider* collider = r.Front();
    r.PopFront();

    Cog* cog = collider->GetOwner();
    cog->DispatchEvent(eventId, toSend);
  }
}

void Region::AddEffect(PhysicsEffect* effect)
{
  mEffects.PushBack(effect);

  WakeUpAll();
}

void Region::RemoveEffect(PhysicsEffect* effect)
{
  mEffects.Erase(effect);

  WakeUpAll();
}

void Region::Update(real dt)
{
  // Break early if there are no effects
  if(mEffects.Empty())
    return;

  // For each object in the region
  ContactBodyRange bodyRange(mCollider->mContactEdges.All());
  for(; !bodyRange.Empty(); bodyRange.PopFront())
  {
    ContactGraphEdge holder = bodyRange.Front();
    RigidBody* body = holder.GetOtherBody();
    // Only apply region effects to non-static bodies (yes this will apply to kinematic objects.
    // This is a bug feature which allows things like kinematic swept controllers to
    // still get forces applied, just not integrated).
    if(body->GetStatic() == false)
    {
      // Wake up any bodies that just came into contact with the region (otherwise the forces won't do anything)
      if(holder.GetIsNew())
        body->ForceAwake();
      if(body->IsAsleep() == false && !holder.GetSkipsResolution())
        ApplyEffects(body, dt);
    }
  }
}

void Region::ApplyEffects(RigidBody* body, real dt)
{
  PhysicsEffectList::range effects = mEffects.All();
  for(; !effects.Empty(); effects.PopFront())
  {
    PhysicsEffect& effect = effects.Front();
    if(effect.GetActive())
      effect.ApplyEffect(body, dt);
  }
}

void Region::WakeUpAll()
{
  // If we don't wake the bodies up when we change our effects then exit early
  if(!mWakeUpOnEffectChange || mCollider == nullptr)
    return;

  // Effects have been changed, so we have to wake up all bodies otherwise new
  // effects will not be properly applied.
  ContactBodyRange bodyRange(mCollider->mContactEdges.All());
  for(; !bodyRange.Empty(); bodyRange.PopFront())
  {
    RigidBody* body = bodyRange.Front().GetOtherBody();
    if(body != nullptr && body->GetStatic() == false)
      body->ForceAwake();
  }
}

//-------------------------------------------------------------------Region::RegionContactRange
Region::RegionContactRange::RegionContactRange(const ContactRange& range)
  : mRange(range)
{
  
}

Collider* Region::RegionContactRange::Front()
{
  return mRange.Front().GetOtherCollider();
}

void Region::RegionContactRange::PopFront()
{
  mRange.PopFront();
}

bool Region::RegionContactRange::Empty()
{
  return mRange.Empty();
}

Region::RegionContactRange Region::All()
{
  return RegionContactRange(FilterContactRange(mCollider));
}

}//namespace Zero
