// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace Events
{
DeclareEvent(CustomPhysicsEffectPrecalculatePhase);
DeclareEvent(ApplyCustomPhysicsEffect);
} // namespace Events

class CustomPhysicsEffect;
class RigidBody;

/// Event data for applying custom physics effects.
class CustomPhysicsEffectEvent : public Event
{
public:
  RaverieDeclareType(CustomPhysicsEffectEvent, TypeCopyMode::ReferenceType);

  CustomPhysicsEffectEvent();

  /// The effect that sent out this event.
  CustomPhysicsEffect* mEffect;
  /// The RigidBody to apply forces to.
  RigidBody* mRigidBody;
  /// The timestep of the current physics frame (in seconds).
  real mDt;
};

/// A physics effect that sends events out so users can apply custom logic for
/// forces.
class CustomPhysicsEffect : public PhysicsEffect
{
public:
  RaverieDeclareType(CustomPhysicsEffect, TypeCopyMode::ReferenceType);

  CustomPhysicsEffect();

  // Component Interface
  void Serialize(Serializer& stream) override;

  // Physics Effect Interface
  void PreCalculate(real dt) override;
  void ApplyEffect(RigidBody* obj, real dt) override;
};

} // namespace Raverie
