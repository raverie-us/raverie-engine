///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(TorqueEffect, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(LocalTorque)->ZeroSerialize(true);
  ZilchBindGetterSetterProperty(TorqueStrength)->ZeroSerialize(real(1));
  ZilchBindGetterSetterProperty(TorqueAxis)->ZeroSerialize(Vec3::cYAxis);
  ZilchBindGetterProperty(WorldTorqueAxis);
}

TorqueEffect::TorqueEffect()
{
  mEffectType = PhysicsEffectType::Torque;
}

void TorqueEffect::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void TorqueEffect::DebugDraw()
{
  if(!GetDebugDrawEffect())
    return;

  PreCalculate(0);
  Vec3 torqueAxis = mWorldTorqueAxis;
  Vec3 torqueCenter = TransformLocalPointToWorld(Vec3::cZero);
  
  // Get an animating time
  real t = GetAnimationTime(GetOwner());

  // Include the strength? If the strength is include we have to only use positive strength values to update t.
  // Negative values should flip the axis (otherwise the arrow-heads will be backwards).
  torqueAxis *= (real)Math::Sign(mTorqueStrength);
  t *= Math::Abs(mTorqueStrength);
  
  // Hard-code the animation radius at 1 (could use the object's cylinder's radius?)
  real radius = 1;
  // Draw a ring that has 3 sub-divisions
  DrawRing(torqueCenter, torqueAxis, radius, 3, t, Color::White);
  gDebugDraw->Add(Debug::Line(torqueCenter, torqueCenter + torqueAxis).HeadSize(0.1f));
}

void TorqueEffect::PreCalculate(real dt)
{
  if(!GetActive())
    return;

  mWorldTorqueAxis = GetWorldTorqueAxis();
}

void TorqueEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if(!GetActive())
    return;

  Vec3 torque = mWorldTorqueAxis * mTorqueStrength;
  obj->ApplyTorque(torque);
}

bool TorqueEffect::GetLocalTorque() const
{
  return mTorqueStates.IsSet(TorqueFlags::LocalTorque);
}

void TorqueEffect::SetLocalTorque(bool state)
{
  mTorqueStates.SetState(TorqueFlags::LocalTorque,state);
  CheckWakeUp();
}

float TorqueEffect::GetTorqueStrength() const
{
  return mTorqueStrength;
}

void TorqueEffect::SetTorqueStrength(float strength)
{
  mTorqueStrength = strength;
  CheckWakeUp();
}

Vec3 TorqueEffect::GetTorqueAxis() const
{
  return mTorqueAxis;
}

void TorqueEffect::SetTorqueAxis(Vec3Param axis)
{
  mTorqueAxis = axis;
  CheckWakeUp();
}

Vec3 TorqueEffect::GetWorldTorqueAxis() const
{
  Vec3 worldTorqueAxis = mTorqueAxis;
  if(mTorqueStates.IsSet(TorqueFlags::LocalTorque))
    worldTorqueAxis = TransformLocalDirectionToWorld(worldTorqueAxis);
  // Always re-normalize the world axis
  return worldTorqueAxis.AttemptNormalized();
}

}//namespace Zero
