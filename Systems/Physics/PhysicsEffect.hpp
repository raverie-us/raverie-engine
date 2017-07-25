///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Most of these flags are internally used to store what the effect is operating on.
/// The Active and WakeUpOnChange flags are exposed as base class properties though.
DeclareBitField8(EffectFlags, Active, WakeUpOnChange, DebugDraw, SpaceEffect, RegionEffect,
                 BodyEffect, ColliderEffect, LevelEffect);

/// All physics effect types. Exposed on each physics effect to denote
/// what kind of effect it is. Also used in IgnoreSpaceEffects.
DeclareBitField13(PhysicsEffectType, Drag, Flow, Force, Gravity, Thrust, Vortex, Wind,
                  Torque, PointGravity, PointForce, Buoyancy, Custom, Invalid);

/// Describes how force values are interpolated between min/max values.
/// <param name="Linear">Linearly interpolate between values.</param>
/// <param name="Quadratic">Quadratically interpolate between values (uses t^2).</param>
DeclareEnum2(PhysicsEffectInterpolationType, Linear, Quadratic);
/// Describes how interpolation is performed (if at all) outside of a physics effect's max distance.
/// <param name="ClampToMax">Clamps to the max force value</param>
/// <param name="ContinueFalloff">Continue the regular interpolation method</param>
/// <param name="NoEffect">Don't apply a force past the max distance</param>
DeclareEnum3(PhysicsEffectEndCondition, ClampToMax, ContinueFalloff, NoEffect);

/// A common interface for all effects in physics. An effect is something that typically
/// applies a force and can be attached to a collider, rigid body, region, or even a space.
/// This effect is applied every frame according to the rules of the object it is attached to.
class PhysicsEffect : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PhysicsEffect();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnDestroy(uint flags) override;
  void ComponentAdded(BoundType* typeId, Component* component) override;
  void ComponentRemoved(BoundType* typeId, Component* component) override;

  //-------------------------------------------------------------------PhysicsEffect Interface
  /// Some values can be pre-calculated once per frame instead
  /// of per body/object (local to world space directions, etc...)
  virtual void PreCalculate(real dt) {};
  /// Apply the desired forces to the given body. Dt is provided for
  /// effects that actually want to apply an acceleration (masked as a force).
  virtual void ApplyEffect(RigidBody* obj, real dt) = 0;
  virtual void ApplyEffect(SpringSystem* obj, real dt) {};

  // Helpers
  Vec3 TransformLocalDirectionToWorld(Vec3Param localDir) const;
  Vec3 TransformLocalPointToWorld(Vec3Param localPoint) const;
  void TransformLocalDirectionAndPointToWorld(Vec3& localPoint, Vec3& localDir) const;

  void AddEffect();
  void RemoveEffect();

  void LevelAdd(PhysicsSpace* space);
  void LevelRemove(PhysicsSpace* space);
  void RegionAdd(Region* region);
  void RegionRemove(Region* region);
  void BodyAdd(RigidBody* body);
  void BodyRemove(RigidBody* body);
  void ColliderAdd(Collider* collider);
  void ColliderRemove(Collider* collider);
  void SpaceAdd(PhysicsSpace* space);
  void SpaceRemove(PhysicsSpace* space);
  void HierarchyAdd();
  void HierarchyRemove();

  /// If WakeUpOnChange is set, tell the associated object (RigidBody, Region, etc...) to wake up.
  void CheckWakeUp();

  /// Toggles whether or not this effect is active.
  void Toggle();

  /// What kind of effect this is (e.g. ForceEffect, GravityEffect, etc...).
  PhysicsEffectType::Enum GetEffectType();

  //Properties

  /// Enable/disable this effect.
  bool GetActive();
  void SetActive(bool state);

  /// Whether the object associated with this is woken up when any property is changed
  bool GetWakeUpOnChange();
  void SetWakeUpOnChange(bool state);

  /// Should the effect debug draw
  bool GetDebugDrawEffect();
  void SetDebugDrawEffect(bool state);

  Link<PhysicsEffect> link;
  Link<PhysicsEffect> SpaceLink;

protected:
  BitField<EffectFlags::Enum> mFlags;
  PhysicsEffectType::Enum mEffectType;
};

typedef InList<PhysicsEffect>                            PhysicsEffectList;
typedef InList<PhysicsEffect, &PhysicsEffect::SpaceLink> SpaceEffectList;

}//namespace Zero
