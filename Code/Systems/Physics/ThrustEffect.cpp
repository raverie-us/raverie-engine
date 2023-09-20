// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(ThrustEffect, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();

  RaverieBindGetterSetterProperty(LocalSpaceDirection)->RaverieSerialize(true);
  RaverieBindGetterSetterProperty(ForceStrength)->RaverieSerialize(real(10));
  RaverieBindGetterSetterProperty(ForceDirection)->RaverieSerialize(Vec3(0, 1, 0));
  RaverieBindGetterProperty(WorldForceDirection);
}

ThrustEffect::ThrustEffect()
{
  mEffectType = PhysicsEffectType::Thrust;
}

void ThrustEffect::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void ThrustEffect::DebugDraw()
{
  if (!GetDebugDrawEffect())
    return;

  PreCalculate(0);
  Vec3 pos = mWorldThrustCenter;
  Vec3 dir = mWorldThrustDirection * mForceStrength;
  gDebugDraw->Add(Debug::Line(pos, pos + dir).HeadSize(0.1f));
}

void ThrustEffect::PreCalculate(real dt)
{
  if (!GetActive())
    return;

  // Update the cached center and direction
  mWorldThrustCenter = TransformLocalPointToWorld(Vec3::cZero);
  mWorldThrustDirection = GetWorldForceDirection();
}

void ThrustEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if (!GetActive())
    return;

  // Apply the force to the rigid body at our thrust center
  Vec3 force = mWorldThrustDirection * mForceStrength;
  obj->ApplyForceAtPointNoWakeUp(force, mWorldThrustCenter);
}

bool ThrustEffect::GetLocalSpaceDirection() const
{
  return mThrustFlags.IsSet(ThrustFlags::LocalSpaceDirection);
}

void ThrustEffect::SetLocalSpaceDirection(bool state)
{
  mThrustFlags.SetState(ThrustFlags::LocalSpaceDirection, state);
  CheckWakeUp();
}

real ThrustEffect::GetForceStrength() const
{
  return mForceStrength;
}

void ThrustEffect::SetForceStrength(real strength)
{
  mForceStrength = strength;
  CheckWakeUp();
}

Vec3 ThrustEffect::GetForceDirection() const
{
  return mForceDirection;
}

void ThrustEffect::SetForceDirection(Vec3Param force)
{
  mForceDirection = force;
  CheckWakeUp();
}

Vec3 ThrustEffect::GetWorldForceDirection() const
{
  Vec3 worldThrustDirection = mForceDirection;
  if (mThrustFlags.IsSet(ThrustFlags::LocalSpaceDirection))
    worldThrustDirection = TransformLocalDirectionToWorld(worldThrustDirection);
  // Always re-normalize the world axis
  return worldThrustDirection.AttemptNormalized();
}

} // namespace Raverie
