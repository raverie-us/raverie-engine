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

/// Defines spring properties for a joint. Used to make a joint soft and therefore behave
/// spring-like. A joint spring has a frequency in hertz at which to oscillate
/// as well as a damping ratio. The ratio should vary from 0 to 1 where 0 is
/// no damping and 1 is critical damping.
/// See each joint for a description of how it reacts to a spring.
class JointSpring : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  JointSpring();
  ~JointSpring();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  // Helpers
  bool GetAtomIndexActive(uint atomIndexMask);
  /// Has this joint been initialized with a valid joint.
  /// Also used to deal with calling sets in serialize.
  bool IsValid();

  // Properties

  /// Determines if this spring is active.
  bool GetActive() const;
  void SetActive(bool active);
  /// Signifies what atoms on the joint this affects. For internal use.
  uint GetAtomIds() const;
  void SetAtomIds(uint atomIds);
  /// The oscillation frequency of the spring in Hertz (cycles per second).
  real GetFrequencyHz() const;
  void SetFrequencyHz(real frequency);
  /// The damping ratio of this spring. The value should range from 0 to 1
  /// where 0 is no damping and 1 is critical damping.
  real GetDampingRatio() const;
  void SetDampingRatio(real dampRatio);

  //-------------------------------------------------------------------Internal
  Physics::SpringAtom mSpringAtom;
  JointNode* mNode;
  uint mAtomIds;
  bool mActive;
};

}//namespace Zero
