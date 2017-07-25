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

DeclareBitField5(JointLimitState, Active, AtLowerLimit, WasAtLowerLimit, AtUpperLimit, WasAtUpperLimit);

/// Defines limit properties for a joint. Used to add a min/max bounds to a joint. When the
/// joint is in between the min/max bounds, the "limited" portion will be
/// ignored (The stick will not solve when it is in between the bounds,
/// making it a rope).
/// See each joint for a description of how it reacts to a limit.
struct JointLimit : public Component
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  JointLimit();
  virtual ~JointLimit();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  //Helpers
  bool GetAtomIndexActive(uint atomIndexMask) const;
  /// Has this joint been initialized with a valid joint.
  /// Also used to deal with calling sets in serialize.
  bool IsValid();

  // Properties

  /// Determines if this limit is currently active.
  bool GetActive() const;
  void SetActive(bool active);
  /// Signifies what atoms on the joint this affects. For internal use.
  uint GetAtomIds() const;
  void SetAtomIds(uint atomIds);
  /// The lower bound for this limit.
  real GetLowerLimit() const;
  void SetLowerLimit(real limit);
  /// The upper bound for this limit.
  real GetUpperLimit() const;
  void SetUpperLimit(real limit);


  // Runtime helpers for event sending
  
  /// Are we currently at the lower limit?
  bool GetAtLowerLimit();
  void SetAtLowerLimit(bool state);
  /// Were we at the lower limit last frame?
  /// Used to only send out a "start" event for hitting the limit.
  bool GetWasAtLowerLimit();
  void SetWasAtLowerLimit(bool state);
  /// Are we currently at the upper limit?
  bool GetAtUpperLimit();
  void SetAtUpperLimit(bool state);
  /// Were we at the upper limit last frame?
  /// Used to only send out a "start" event for hitting the limit.
  bool GetWasAtUpperLimit();
  void SetWasAtUpperLimit(bool state);

  BitField<JointLimitState::Enum> mState;

  real mMinErr;
  real mMaxErr;

  uint mAtomIds;
  JointNode* mNode;
};

}//namespace Zero
