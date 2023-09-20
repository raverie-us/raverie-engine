// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(BasicDirectionEffect, builder, type)
{
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();

  RaverieBindGetterSetterProperty(LocalSpaceDirection)->RaverieSerialize(true);
  RaverieBindGetterSetterProperty(Strength)->RaverieSerialize(real(10));
  RaverieBindGetterSetterProperty(Direction)->RaverieSerialize(Vec3(0, -1, 0));
  RaverieBindGetterProperty(WorldDirection);
}

BasicDirectionEffect::BasicDirectionEffect()
{
  mStrength = 10;
  mDirection = Vec3(0, -1, 0);
  mWorldForce = mDirection * mStrength;
}

void BasicDirectionEffect::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void BasicDirectionEffect::DebugDraw()
{
  if (!GetDebugDrawEffect())
    return;

  // Draw the world space direction vector (normalized)
  PreCalculate(0);
  Vec3 pos = TransformLocalPointToWorld(Vec3::cZero);
  // Should this reflect the actual magnitude of the force?
  gDebugDraw->Add(Debug::Line(pos, pos + mWorldForce).HeadSize(0.1f));
}

void BasicDirectionEffect::PreCalculate(real dt)
{
  if (!GetActive())
    return;

  // The world space force is constant for all objects passed in so cache this
  // once per iteration
  mWorldForce = GetWorldDirection() * mStrength;
}

bool BasicDirectionEffect::GetLocalSpaceDirection() const
{
  return mForceStates.IsSet(BasicForceFlags::LocalSpaceDirection);
}

void BasicDirectionEffect::SetLocalSpaceDirection(bool state)
{
  mForceStates.SetState(BasicForceFlags::LocalSpaceDirection, state);
  // Make sure to check for waking objects up on effect change
  CheckWakeUp();
}

real BasicDirectionEffect::GetStrength() const
{
  return mStrength;
}

void BasicDirectionEffect::SetStrength(real strength)
{
  mStrength = strength;
  // Make sure to check for waking objects up on effect change
  CheckWakeUp();
}

Vec3 BasicDirectionEffect::GetDirection() const
{
  return mDirection;
}

void BasicDirectionEffect::SetDirection(Vec3Param force)
{
  mDirection = force;
  // Make sure to check for waking objects up on effect change
  CheckWakeUp();
}

Vec3 BasicDirectionEffect::GetWorldDirection() const
{
  Vec3 worldForceDirection = mDirection;
  // If we have a local space direction, calculate the world space direction
  if (mForceStates.IsSet(BasicForceFlags::LocalSpaceDirection))
    worldForceDirection = TransformLocalDirectionToWorld(worldForceDirection);
  // Always re-normalize the world direction
  return worldForceDirection.AttemptNormalized();
}

RaverieDefineType(ForceEffect, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();
}

ForceEffect::ForceEffect()
{
  mEffectType = PhysicsEffectType::Force;
}

void ForceEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if (!GetActive())
    return;

  // Apply the force as normal
  Vec3 force = mWorldForce;
  obj->ApplyForceNoWakeUp(force);
}

RaverieDefineType(GravityEffect, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();
}

GravityEffect::GravityEffect()
{
  mEffectType = PhysicsEffectType::Gravity;
}

void GravityEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if (!GetActive())
    return;

  Vec3 force = mWorldForce;
  // We're applying this as a constant acceleration, so convert the
  // force to an acceleration by dividing out the mass
  force = obj->mInvMass.ApplyInverted(force);
  obj->ApplyForceNoWakeUp(force);
}

void GravityEffect::ApplyEffect(SpringSystem* obj, real dt)
{
  if (!GetActive())
    return;

  Vec3 force = mWorldForce;
  for (uint i = 0; i < obj->mPointMasses.Size(); ++i)
  {
    SpringSystem::PointMass& pointMass = obj->mPointMasses[i];

    if (pointMass.mInvMass != real(0.0))
      pointMass.mForce += force / pointMass.mInvMass;
  }
}

} // namespace Raverie
