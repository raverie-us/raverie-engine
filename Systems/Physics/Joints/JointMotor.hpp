///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct JointNode;

DeclareBitField2(JointMotorFlags, Active, Reverse);

/// Defines motor properties for a joint. Used to add energy to a joint.
/// A motor defines a desired speed to move at as well as a max impulse
/// that can be applied to reach that speed in a timestep.
/// See each joint for a description of how it reacts to a motor.
struct JointMotor : public Component
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  JointMotor();
  virtual ~JointMotor();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  // Helpers
  bool GetAtomIndexActive(uint atomIndexMask);
  /// Has this joint been initialized with a valid joint.
  /// Also used to deal with calling sets in serialize.
  bool IsValid();

  // Properties

  /// Determines if this motor is currently active.
  bool GetActive() const;
  void SetActive(bool active);
  /// Determines if this motor should move in reverse.
  /// This is a convenient way to reverse a motor without having to negate the speed.
  bool GetReverse() const;
  void SetReverse(bool reverse);
  /// Signifies what atoms on the joint this affects. For internal use.
  uint GetAtomIds() const;
  void SetAtomIds(uint atomIds);
  /// The maximum impulse that the motor can apply
  /// each frame to reach the target speed.
  real GetMaxImpulse() const;
  void SetMaxImpulse(real force);
  /// The desired speed for this motor.
  real GetSpeed() const;
  void SetSpeed(real speed);

  BitField<JointMotorFlags::Enum> mFlags;
  real mMaxImpulse;
  real mSpeed;
  real mImpulse;

  uint mAtomIds;

  JointNode* mNode;
};

}//namespace Zero
