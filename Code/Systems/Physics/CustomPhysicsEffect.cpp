// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
DefineEvent(CustomPhysicsEffectPrecalculatePhase);
DefineEvent(ApplyCustomPhysicsEffect);
} // namespace Events

RaverieDefineType(CustomPhysicsEffectEvent, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();

  RaverieBindFieldProperty(mDt);
  RaverieBindFieldProperty(mEffect);
  RaverieBindFieldProperty(mRigidBody);
}

CustomPhysicsEffectEvent::CustomPhysicsEffectEvent()
{
  mRigidBody = nullptr;
  mEffect = nullptr;
  mDt = 0;
}

RaverieDefineType(CustomPhysicsEffect, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();

  RaverieBindEvent(Events::CustomPhysicsEffectPrecalculatePhase, CustomPhysicsEffectEvent);
  RaverieBindEvent(Events::ApplyCustomPhysicsEffect, CustomPhysicsEffectEvent);
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
  if (!GetActive())
    return;

  CustomPhysicsEffectEvent toSend;
  toSend.mEffect = this;
  toSend.mRigidBody = nullptr;
  toSend.mDt = dt;
  GetOwner()->DispatchEvent(Events::CustomPhysicsEffectPrecalculatePhase, &toSend);
}

void CustomPhysicsEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if (!GetActive())
    return;

  CustomPhysicsEffectEvent toSend;
  toSend.mEffect = this;
  toSend.mRigidBody = obj;
  toSend.mDt = dt;
  GetOwner()->DispatchEvent(Events::ApplyCustomPhysicsEffect, &toSend);
}

} // namespace Raverie
