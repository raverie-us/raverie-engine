///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareBitField1(TorqueFlags, LocalTorque);

/// Applies a torque to the center of mass of a body.
class TorqueEffect : public PhysicsEffect
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  TorqueEffect();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void DebugDraw() override;

  // Physics Effect Interface
  void PreCalculate(real dt) override;
  void ApplyEffect(RigidBody* obj, real dt) override;

  // Properties

  /// Determines if the torque is applied in local or world space.
  bool GetLocalTorque() const;
  void SetLocalTorque(bool state);
  /// The strength of the torque being applied.
  float GetTorqueStrength() const;
  void SetTorqueStrength(float strength);
  /// The axis that the torque is being applied about.
  Vec3 GetTorqueAxis() const;
  void SetTorqueAxis(Vec3Param axis);
  /// The axis of the torque in world space (can be used to manually add torque to a RigidBody).
  Vec3 GetWorldTorqueAxis() const;

private:
  // Whether or not the torque is local.
  BitField<TorqueFlags::Enum> mTorqueStates;
  // The strength of torque.
  float mTorqueStrength;

  // The axis of the torque.
  Vec3 mTorqueAxis;
  Vec3 mWorldTorqueAxis;
};

}//namespace Zero
