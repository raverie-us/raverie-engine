///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(CustomPhysicsEffectPrecalculatePhase);
  DefineEvent(ApplyCustomPhysicsEffect);
}//namespace Events

//-------------------------------------------------------------------CustomPhysicsEffectEvent
ZilchDefineType(CustomPhysicsEffectEvent, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZilchBindFieldProperty(mDt);
  ZilchBindFieldProperty(mEffect);
  ZilchBindFieldProperty(mRigidBody);
}

CustomPhysicsEffectEvent::CustomPhysicsEffectEvent()
{
  mRigidBody = nullptr;
  mEffect = nullptr;
  mDt = 0;
}

//-------------------------------------------------------------------CustomPhysicsEffect
ZilchDefineType(CustomPhysicsEffect, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZeroBindEvent(Events::CustomPhysicsEffectPrecalculatePhase, CustomPhysicsEffectEvent);
  ZeroBindEvent(Events::ApplyCustomPhysicsEffect, CustomPhysicsEffectEvent);
}

CustomPhysicsEffect::CustomPhysicsEffect()
{
  mEffectType = PhysicsEffectType::Custom;
}

void CustomPhysicsEffect::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void CustomPhysicsEffect::PreCalculate(real dt)
{
  if(!GetActive())
    return;

  CustomPhysicsEffectEvent toSend;
  toSend.mEffect = this;
  toSend.mRigidBody = nullptr;
  toSend.mDt = dt;
  GetOwner()->DispatchEvent(Events::CustomPhysicsEffectPrecalculatePhase, &toSend);
}

void CustomPhysicsEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if(!GetActive())
    return;

  CustomPhysicsEffectEvent toSend;
  toSend.mEffect = this;
  toSend.mRigidBody = obj;
  toSend.mDt = dt;
  GetOwner()->DispatchEvent(Events::ApplyCustomPhysicsEffect, &toSend);
}

}//namespace Zero
