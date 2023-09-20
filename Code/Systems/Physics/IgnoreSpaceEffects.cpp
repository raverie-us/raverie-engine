// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(IgnoreSpaceEffects, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();

  RaverieBindDependency(RigidBody);

  RaverieBindGetterSetterProperty(IgnoreDrag)->RaverieSerialize(true);
  RaverieBindGetterSetterProperty(IgnoreFlow)->RaverieSerialize(true);
  RaverieBindGetterSetterProperty(IgnoreForce)->RaverieSerialize(true);
  RaverieBindGetterSetterProperty(IgnoreGravity)->RaverieSerialize(true);
  RaverieBindGetterSetterProperty(IgnorePointForce)->RaverieSerialize(true);
  RaverieBindGetterSetterProperty(IgnorePointGravity)->RaverieSerialize(true);
  RaverieBindGetterSetterProperty(IgnoreThrust)->RaverieSerialize(true);
  RaverieBindGetterSetterProperty(IgnoreVortex)->RaverieSerialize(true);
  RaverieBindGetterSetterProperty(IgnoreWind)->RaverieSerialize(true);
  RaverieBindGetterSetterProperty(IgnoreTorque)->RaverieSerialize(true);
  RaverieBindGetterSetterProperty(IgnoreBuoyancy)->RaverieSerialize(true);
  RaverieBindGetterSetterProperty(IgnoreCustom)->RaverieSerialize(true);

  RaverieBindMethod(GetIgnoreState);
  RaverieBindMethod(SetIgnoreState);
}

void IgnoreSpaceEffects::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void IgnoreSpaceEffects::Initialize(CogInitializer& initializer)
{
  RigidBody* body = GetOwner()->has(RigidBody);
  if (body != nullptr)
    body->mSpaceEffectsToIgnore = this;
}

void IgnoreSpaceEffects::OnDestroy(uint flags)
{
  RigidBody* body = GetOwner()->has(RigidBody);
  if (body != nullptr)
    body->mSpaceEffectsToIgnore = nullptr;
}

bool IgnoreSpaceEffects::IsIgnored(PhysicsEffect* effect)
{
  return mFlags.IsSet(effect->GetEffectType());
}

bool IgnoreSpaceEffects::GetIgnoreDrag()
{
  return mFlags.IsSet(PhysicsEffectType::Drag);
}

void IgnoreSpaceEffects::SetIgnoreDrag(bool ignore)
{
  mFlags.SetState(PhysicsEffectType::Drag, ignore);
}

bool IgnoreSpaceEffects::GetIgnoreFlow()
{
  return mFlags.IsSet(PhysicsEffectType::Flow);
}

void IgnoreSpaceEffects::SetIgnoreFlow(bool ignore)
{
  mFlags.SetState(PhysicsEffectType::Flow, ignore);
}

bool IgnoreSpaceEffects::GetIgnoreForce()
{
  return mFlags.IsSet(PhysicsEffectType::Force);
}

void IgnoreSpaceEffects::SetIgnoreForce(bool ignore)
{
  mFlags.SetState(PhysicsEffectType::Force, ignore);
}

bool IgnoreSpaceEffects::GetIgnoreGravity()
{
  return mFlags.IsSet(PhysicsEffectType::Gravity);
}

void IgnoreSpaceEffects::SetIgnoreGravity(bool ignore)
{
  mFlags.SetState(PhysicsEffectType::Gravity, ignore);
}

bool IgnoreSpaceEffects::GetIgnorePointForce()
{
  return mFlags.IsSet(PhysicsEffectType::PointForce);
}

void IgnoreSpaceEffects::SetIgnorePointForce(bool ignore)
{
  return mFlags.SetState(PhysicsEffectType::PointForce, ignore);
}

bool IgnoreSpaceEffects::GetIgnorePointGravity()
{
  return mFlags.IsSet(PhysicsEffectType::PointGravity);
}

void IgnoreSpaceEffects::SetIgnorePointGravity(bool ignore)
{
  return mFlags.SetState(PhysicsEffectType::PointGravity, ignore);
}

bool IgnoreSpaceEffects::GetIgnoreThrust()
{
  return mFlags.IsSet(PhysicsEffectType::Thrust);
}

void IgnoreSpaceEffects::SetIgnoreThrust(bool ignore)
{
  mFlags.SetState(PhysicsEffectType::Thrust, ignore);
}

bool IgnoreSpaceEffects::GetIgnoreVortex()
{
  return mFlags.IsSet(PhysicsEffectType::Vortex);
}

void IgnoreSpaceEffects::SetIgnoreVortex(bool ignore)
{
  mFlags.SetState(PhysicsEffectType::Vortex, ignore);
}

bool IgnoreSpaceEffects::GetIgnoreWind()
{
  return mFlags.IsSet(PhysicsEffectType::Wind);
}

void IgnoreSpaceEffects::SetIgnoreWind(bool ignore)
{
  mFlags.SetState(PhysicsEffectType::Wind, ignore);
}

bool IgnoreSpaceEffects::GetIgnoreTorque()
{
  return mFlags.IsSet(PhysicsEffectType::Torque);
}

void IgnoreSpaceEffects::SetIgnoreTorque(bool ignore)
{
  mFlags.SetState(PhysicsEffectType::Torque, ignore);
}

bool IgnoreSpaceEffects::GetIgnoreBuoyancy()
{
  return mFlags.IsSet(PhysicsEffectType::Buoyancy);
}

void IgnoreSpaceEffects::SetIgnoreBuoyancy(bool ignore)
{
  mFlags.SetState(PhysicsEffectType::Buoyancy, ignore);
}

bool IgnoreSpaceEffects::GetIgnoreCustom()
{
  return mFlags.IsSet(PhysicsEffectType::Custom);
}

void IgnoreSpaceEffects::SetIgnoreCustom(bool ignore)
{
  mFlags.SetState(PhysicsEffectType::Custom, ignore);
}

bool IgnoreSpaceEffects::GetIgnoreState(PhysicsEffectType::Enum effectType)
{
  return mFlags.IsSet(effectType);
}

void IgnoreSpaceEffects::SetIgnoreState(PhysicsEffectType::Enum effectType, bool ignore)
{
  mFlags.SetState(effectType, ignore);
}

} // namespace Raverie
