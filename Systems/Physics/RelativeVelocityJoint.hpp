///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Collider;
class JointSpring;

namespace Physics
{

DeclareBitField3(RelativeVelocityJointActiveAxes, Axis1, Axis2, Axis3);

/// A relative velocity joint defines what the desired relative velocity on
/// three world axes should be between two objects. Relative velocity is defined
/// as v2 - v1. This joint has not been tested with motors or limits in any way.
struct RelativeVelocityJoint : public Joint
{
  DeclareJointType(RelativeVelocityJoint);

  RelativeVelocityJoint();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  // Joint Interface Implementation
  void ComputeMoleculeData(MoleculeData& moleculeData);
  void UpdateAtoms();
  uint MoleculeCount() const;
  void ComputeMolecules(MoleculeWalker& molecules);
  void WarmStart(MoleculeWalker& molecules);
  void Solve(MoleculeWalker& molecules);
  void Commit(MoleculeWalker& molecules);
  uint PositionMoleculeCount() const;
  void ComputePositionMolecules(MoleculeWalker& molecules);
  void DebugDraw();
  uint GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const;
  void BatchEvents();

  // Joint Interface
  uint GetDefaultLimitIds() const override;
  uint GetDefaultMotorIds() const override;
  uint GetDefaultSpringIds() const override;

  /// One of 3 axes to constrain movement on.
  Vec3 GetAxis(uint index);
  /// One of 3 axes to constrain movement on.
  void SetAxis(uint index, Vec3Param axis);

  /// The desired relative speed for the given axis index.
  real GetSpeed(uint index);
  /// The desired relative speed for the given axis index.
  void SetSpeed(uint index, real speed);

  /// The max impulse for the given axis index.
  real GetMaxImpulse(uint index);
  /// The max impulse for the given axis index.
  void SetMaxImpulse(uint index, real maxImpulse);

  /// Whether or not the given axis index is active.
  bool GetAxisActive(uint index);
  /// Whether or not the given axis index is active.
  void SetAxisActive(uint index, bool active);

  Vec3 mAxes[3];
  Vec3 mSpeeds;
  Vec3 mMaxImpulses;
  BitField<RelativeVelocityJointActiveAxes::Enum> mActiveFlags;

  ConstraintAtom mAtoms[3];
  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::RelativeVelocityJoint RelativeVelocityJoint;

}//namespace Zero
