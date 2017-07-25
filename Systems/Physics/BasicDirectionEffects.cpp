///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2013-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------BasicDirectionEffect
ZilchDefineType(BasicDirectionEffect, builder, type)
{
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(LocalSpaceDirection)->ZeroSerialize(true);
  ZilchBindGetterSetterProperty(Strength)->ZeroSerialize(real(10));
  ZilchBindGetterSetterProperty(Direction)->ZeroSerialize(Vec3(0, -1, 0));
  ZilchBindGetterProperty(WorldDirection);
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
  if(!GetDebugDrawEffect())
    return;

  // Draw the world space direction vector (normalized)
  PreCalculate(0);
  Vec3 pos = TransformLocalPointToWorld(Vec3::cZero); 
  // Should this reflect the actual magnitude of the force?
  gDebugDraw->Add(Debug::Line(pos, pos + mWorldForce).HeadSize(0.1f));
}

void BasicDirectionEffect::PreCalculate(real dt)
{
  if(!GetActive())
    return;

  // The world space force is constant for all objects passed in so cache this once per iteration
  mWorldForce = GetWorldDirection() * mStrength;
}

bool BasicDirectionEffect::GetLocalSpaceDirection() const
{
  return mForceStates.IsSet(BasicForceFlags::LocalSpaceDirection);
}

void BasicDirectionEffect::SetLocalSpaceDirection(bool state)
{
  mForceStates.SetState(BasicForceFlags::LocalSpaceDirection,state);
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
  if(mForceStates.IsSet(BasicForceFlags::LocalSpaceDirection))
    worldForceDirection = TransformLocalDirectionToWorld(worldForceDirection);
  // Always re-normalize the world direction
  return worldForceDirection.AttemptNormalized();
}

//-------------------------------------------------------------------ForceEffect
ZilchDefineType(ForceEffect, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
}

ForceEffect::ForceEffect()
{
  mEffectType = PhysicsEffectType::Force;
}

void ForceEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if(!GetActive())
    return;

  // Apply the force as normal
  Vec3 force = mWorldForce;
  obj->ApplyForce(force);
}

//-------------------------------------------------------------------GravityEffect
ZilchDefineType(GravityEffect, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
}

GravityEffect::GravityEffect()
{
  mEffectType = PhysicsEffectType::Gravity;
}

void GravityEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if(!GetActive())
    return;

  Vec3 force = mWorldForce;
  // We're applying this as a constant acceleration, so convert the
  // force to an acceleration by dividing out the mass
  force = obj->mInvMass.ApplyInverted(force);
  obj->ApplyForce(force);
}

void GravityEffect::ApplyEffect(SpringSystem* obj, real dt)
{
  if(!GetActive())
    return;

  Vec3 force = mWorldForce;
  for(uint i = 0; i < obj->mPointMasses.Size(); ++i)
  {
    SpringSystem::PointMass& pointMass = obj->mPointMasses[i];

    if(pointMass.mInvMass != real(0.0))
      pointMass.mForce += force / pointMass.mInvMass;
  }
}

}//namespace Zero
