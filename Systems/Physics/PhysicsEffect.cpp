///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Tags
{
DefineTag(PhysicsEffect);
}

ZilchDefineType(PhysicsEffect, builder, type)
{
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZilchBindMethod(Toggle);
  ZilchBindGetterSetterProperty(Active)->ZeroSerialize(true);
  ZilchBindGetterSetterProperty(WakeUpOnChange)->ZeroSerialize(true);
  ZilchBindGetterSetterProperty(DebugDrawEffect)->ZeroSerialize(true);
  ZilchBindGetter(EffectType);

  ZeroBindTag(Tags::Physics);
  ZeroBindTag(Tags::PhysicsEffect);
}

PhysicsEffect::PhysicsEffect()
{
  mFlags.SetFlag(EffectFlags::Active);
  mEffectType = PhysicsEffectType::Invalid;
}

void PhysicsEffect::Serialize(Serializer& stream)
{
  Error("Physics Effect should never have serialize called, meta serialization is assumed");
}

void PhysicsEffect::Initialize(CogInitializer& initializer)
{
  // To make sure we never break, we actually keep track of all
  // of the region/body/space. They internally deal with order, but this
  // is needed since 'has' returns objects that have already been deleted.

  // Deal with being on level settings (basically a space effect)
  if(GetOwner() == GetOwner()->GetLevelSettings())
    mFlags.SetFlag(EffectFlags::LevelEffect);

  // Check for a region
  Region* region = GetOwner()->has(Region);
  if(region)
    mFlags.SetFlag(EffectFlags::RegionEffect);

  // Check for a single Rigid Body
  RigidBody* body = GetOwner()->has(RigidBody);
  if(body)
    mFlags.SetFlag(EffectFlags::BodyEffect);

  Collider* collider = GetOwner()->has(Collider);
  if(collider)
    mFlags.SetFlag(EffectFlags::ColliderEffect);

  // If there is no region, check for a physics space
  PhysicsSpace* space = GetOwner()->has(PhysicsSpace);
  if(space)
    mFlags.SetFlag(EffectFlags::SpaceEffect);

  // All spaces need to know what effects are in them so try to find our
  // physics space (could be our owner or space) and add ourself to it
  if(space == nullptr && initializer.mSpace != nullptr)
    space = initializer.mSpace->has(PhysicsSpace);
  if(space != nullptr)
    space->mEffects.PushBack(this);

  AddEffect();
}

void PhysicsEffect::OnDestroy(uint flags)
{
  // If we have a physics space then remove ourself from it
  Space* space = GetSpace();
  if(space != nullptr)
  {
    PhysicsSpace* physicsSpace = space->has(PhysicsSpace);
    if(physicsSpace != nullptr)
      InList<PhysicsEffect, &PhysicsEffect::SpaceLink>::Unlink(this);
  }

  RemoveEffect();
}

void PhysicsEffect::ComponentAdded(BoundType* typeId, Component* component)
{
  if(typeId == ZilchTypeId(Region))
  {
    RemoveEffect();
    mFlags.SetFlag(EffectFlags::RegionEffect);
    AddEffect();
  }
  else if(typeId == ZilchTypeId(RigidBody))
  {
    RemoveEffect();
    mFlags.SetFlag(EffectFlags::BodyEffect);
    AddEffect();
  }
  else if(typeId == ZilchTypeId(PhysicsSpace))
  {
    RemoveEffect();
    mFlags.SetFlag(EffectFlags::SpaceEffect);
    AddEffect();
  }
  else
  {
    if(typeId->IsA(ZilchTypeId(Collider)))
    {
      RemoveEffect();
      mFlags.SetFlag(EffectFlags::ColliderEffect);
      AddEffect();
    }
  }
}

void PhysicsEffect::ComponentRemoved(BoundType* typeId, Component* component)
{
  if(typeId == ZilchTypeId(Region))
  {
    RemoveEffect();
    mFlags.ClearFlag(EffectFlags::RegionEffect);
    AddEffect();
  }
  else if(typeId == ZilchTypeId(RigidBody))
  {
    RemoveEffect();
    mFlags.ClearFlag(EffectFlags::BodyEffect);
    AddEffect();
  }
  else if(typeId == ZilchTypeId(PhysicsSpace))
  {
    RemoveEffect();
    mFlags.ClearFlag(EffectFlags::SpaceEffect);
    AddEffect();
  }
  else
  {
    if(typeId->IsA(ZilchTypeId(Collider)))
    {
      RemoveEffect();
      mFlags.ClearFlag(EffectFlags::ColliderEffect);
      AddEffect();
    }
  }
}

Vec3 PhysicsEffect::TransformLocalDirectionToWorld(Vec3Param localDir) const
{
  Vec3 worldDir = localDir;
  // If we have a collider then use the cached body-to-world transform to get our world space values
  Collider* collider = GetOwner()->has(Collider);
  if(collider != nullptr)
  {
    WorldTransformation* transform = collider->GetWorldTransform();
    worldDir = transform->TransformNormal(worldDir);
    return worldDir;
  }

  // Otherwise fall-back on using the transform
  Transform* transform = GetOwner()->has(Transform);
  if(transform != nullptr)
  {
    Quat worldRotation = transform->GetWorldRotation();
    worldDir = transform->TransformNormal(worldDir);
    return worldDir;
  }
  return worldDir;
}

Vec3 PhysicsEffect::TransformLocalPointToWorld(Vec3Param localPoint) const
{
  Vec3 worldPoint = localPoint;
  // If we have a collider then use the cached body-to-world transform to get our world space values
  Collider* collider = GetOwner()->has(Collider);
  if(collider != nullptr)
  {
    WorldTransformation* transform = collider->GetWorldTransform();
    worldPoint = transform->TransformPoint(localPoint);
    return worldPoint;
  }

  // Otherwise fall-back on using the transform
  Transform* transform = GetOwner()->has(Transform);
  if(transform != nullptr)
  {
    Vec3 worldPoint = transform->TransformPoint(localPoint);
    return worldPoint;
  }
  return worldPoint;
}

void PhysicsEffect::TransformLocalDirectionAndPointToWorld(Vec3& localPoint, Vec3& localDir) const
{
  // If we have a collider then use the cached body-to-world transform to get our world space values
  Collider* collider = GetOwner()->has(Collider);
  if(collider != nullptr)
  {
    WorldTransformation* transform = collider->GetWorldTransform();
    localDir = transform->TransformNormal(localDir);
    localPoint = transform->TransformPoint(localPoint);
    return;
  }

  // Otherwise fall-back on using the transform
  Transform* transform = GetOwner()->has(Transform);
  if(transform != nullptr)
  {
    localDir = transform->TransformNormal(localDir);
    localPoint = transform->TransformPoint(localPoint);
    return;
  }
}

void PhysicsEffect::AddEffect()
{
  if(mFlags.IsSet(EffectFlags::LevelEffect))
    LevelAdd(GetSpace()->has(PhysicsSpace));
  else if(mFlags.IsSet(EffectFlags::RegionEffect))
    RegionAdd(GetOwner()->has(Region));
  else if(mFlags.IsSet(EffectFlags::BodyEffect))
    BodyAdd(GetOwner()->has(RigidBody));
  else if(mFlags.IsSet(EffectFlags::ColliderEffect))
    ColliderAdd(GetOwner()->has(Collider));
  else if(mFlags.IsSet(EffectFlags::SpaceEffect))
    SpaceAdd(GetOwner()->has(PhysicsSpace));
  else if(GetSpace() != nullptr)
    HierarchyAdd();
}

void PhysicsEffect::RemoveEffect()
{
  if(mFlags.IsSet(EffectFlags::LevelEffect))
    LevelRemove(GetSpace()->has(PhysicsSpace));
  else if(mFlags.IsSet(EffectFlags::RegionEffect))
    RegionRemove(GetOwner()->has(Region));
  else if(mFlags.IsSet(EffectFlags::BodyEffect))
    BodyRemove(GetOwner()->has(RigidBody));
  else if(mFlags.IsSet(EffectFlags::ColliderEffect))
    ColliderRemove(GetOwner()->has(Collider));
  else if(mFlags.IsSet(EffectFlags::SpaceEffect))
    SpaceRemove(GetOwner()->has(PhysicsSpace));
  else if(GetSpace() != nullptr)
    HierarchyRemove();
}

void PhysicsEffect::LevelAdd(PhysicsSpace* space)
{
  space->AddGlobalEffect(this);
}

void PhysicsEffect::LevelRemove(PhysicsSpace* space)
{
  space->RemoveGlobalEffect(this);
}

void PhysicsEffect::RegionAdd(Region* region)
{
  region->AddEffect(this);
}

void PhysicsEffect::RegionRemove(Region* region)
{
  region->RemoveEffect(this);
}

void PhysicsEffect::BodyAdd(RigidBody* body)
{
  body->AddBodyEffect(this);
}

void PhysicsEffect::BodyRemove(RigidBody* body)
{
  body->RemoveBodyEffect(this);
}

void PhysicsEffect::ColliderAdd(Collider* collider)
{
  collider->mEffects.PushBack(this);
}

void PhysicsEffect::ColliderRemove(Collider* collider)
{
  collider->mEffects.Erase(this);
}

void PhysicsEffect::SpaceAdd(PhysicsSpace* space)
{
  space->AddGlobalEffect(this);
}

void PhysicsEffect::SpaceRemove(PhysicsSpace* space)
{
  space->RemoveGlobalEffect(this);
}

void PhysicsEffect::HierarchyAdd()
{
  PhysicsSpace* physicsSpace = GetSpace()->has(PhysicsSpace);
  if(physicsSpace != nullptr)
    physicsSpace->mHierarchyEffects.PushBack(this);
}

void PhysicsEffect::HierarchyRemove()
{
  PhysicsSpace* physicsSpace = GetSpace()->has(PhysicsSpace);
  if(physicsSpace != nullptr)
    physicsSpace->mHierarchyEffects.Erase(this);
}

void PhysicsEffect::CheckWakeUp()
{
  // If we don't wake up the objects when a property is changed then don't do anything
  if(!mFlags.IsSet(EffectFlags::WakeUpOnChange) || !IsInitialized())
    return;

  // Otherwise tell the correct thing to wake everything up
  if(mFlags.IsSet(EffectFlags::RegionEffect))
  {
    Region* region = GetOwner()->has(Region);
    if(!region)
      return;
    region->WakeUpAll();
  }
  else if(mFlags.IsSet(EffectFlags::BodyEffect))
  {
    RigidBody* body = GetOwner()->has(RigidBody);
    while(body && !body->IsDynamic())
      body = body->mParentBody;
    if(body)
      body->ForceAwake();
  }
  else if(mFlags.IsSet(EffectFlags::ColliderEffect))
  {
    Collider* collider = GetOwner()->has(Collider);
    if(!collider)
      return;
    collider->ForceAwake();
  }
  else if(mFlags.IsSet(EffectFlags::SpaceEffect))
  {
    PhysicsSpace* space = GetOwner()->has(PhysicsSpace);
    if(!space)
      return;
    space->ForceAwakeRigidBodies();
  }
}

void PhysicsEffect::Toggle()
{
  mFlags.ToggleFlag(EffectFlags::Active);
  CheckWakeUp();
}

PhysicsEffectType::Enum PhysicsEffect::GetEffectType()
{
  return mEffectType;
}

bool PhysicsEffect::GetActive()
{
  return mFlags.IsSet(EffectFlags::Active);
}

void PhysicsEffect::SetActive(bool state)
{
  if(mFlags.IsSet(EffectFlags::Active) == state)
    return;

  mFlags.SetState(EffectFlags::Active, state);
  if(IsInitialized())
    CheckWakeUp();
}

bool PhysicsEffect::GetWakeUpOnChange()
{
  return mFlags.IsSet(EffectFlags::WakeUpOnChange);
}

void PhysicsEffect::SetWakeUpOnChange(bool state)
{
  mFlags.SetState(EffectFlags::WakeUpOnChange, state);
}

bool PhysicsEffect::GetDebugDrawEffect()
{
  return mFlags.IsSet(EffectFlags::DebugDraw);
}

void PhysicsEffect::SetDebugDrawEffect(bool state)
{
  mFlags.SetState(EffectFlags::DebugDraw, state);
}

}//namespace Zero
