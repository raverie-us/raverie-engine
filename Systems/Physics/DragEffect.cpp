///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(DragEffect, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(LinearDamping)->ZeroSerialize(real(0.2));
  ZilchBindGetterSetterProperty(AngularDamping)->ZeroSerialize(real(0.1));
  ZilchBindGetterSetterProperty(LinearDrag)->ZeroSerialize(real(0));
  ZilchBindGetterSetterProperty(AngularDrag)->ZeroSerialize(real(0));
}

DragEffect::DragEffect()
{
  mEffectType = PhysicsEffectType::Drag;
}

void DragEffect::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void DragEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if(!GetActive())
    return;

  // Deal with dt being 0 (timescale of 0 for instance)
  real invDt = 0;
  if(dt != 0)
    invDt = 1 / dt;

  // Clamp the max linear and angular damping values so that they can't reverse
  // the object's direction (and explode) but will instead exactly stop the object.
  real linearDamping = Math::Min(mLinearDamping, invDt);
  real angularDamping = Math::Min(mAngularDamping, invDt);

  // We don't have an acceleration accumulator, so we have to apply damping
  // as a force. To turn our acceleration into a force we have to multiply by
  // the mass so it will divide out when the force is integrated.

  Vec3 force = -obj->mVelocity * mLinearDrag;
  if(linearDamping != 0.0f)
  {
    Vec3 linearAcceleration = -obj->mVelocity * linearDamping;
    force += obj->mInvMass.ApplyInverted(linearAcceleration);
  }
  obj->ApplyForce(force);
  
  Vec3 torque = -obj->mAngularVelocity * mAngularDrag;
  if(angularDamping != 0.0f)
  {
    Vec3 angularAcceleration = -obj->mAngularVelocity * angularDamping;
    torque = obj->mInvInertia.ApplyInverted(angularAcceleration);
  }
  
  obj->ApplyTorque(torque);
}

real DragEffect::GetLinearDamping() const
{
  return mLinearDamping;
}

void DragEffect::SetLinearDamping(real linearDamping)
{
  mLinearDamping = Math::Max(linearDamping, real(0));
  CheckWakeUp();
}

real DragEffect::GetAngularDamping() const
{
  return mAngularDamping;
}

void DragEffect::SetAngularDamping(real angularDamping)
{
  mAngularDamping = Math::Max(angularDamping, real(0));
  CheckWakeUp();
}

real DragEffect::GetLinearDrag() const
{
  return mLinearDrag;
}

void DragEffect::SetLinearDrag(real drag)
{
  mLinearDrag = Math::Max(drag, real(0));
  CheckWakeUp();
}

real DragEffect::GetAngularDrag()
{
  return mAngularDrag;
}

void DragEffect::SetAngularDrag(real angularDrag)
{
  mAngularDrag = Math::Max(angularDrag, real(0));
  CheckWakeUp();
}

}//namespace Zero
