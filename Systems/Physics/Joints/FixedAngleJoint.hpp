///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Collider;
class JointSpring;
class PhysicsSpace;

namespace Physics
{

/// Locks the orientation of two objects together. Used when two objects should
/// always rotate in unison. Typically not used by itself, but useful as a proof of
/// concept for joints that use this functionality internally.
/// Limits, motors, and springs should most likely not be used on this.
/// Add on definitions:
/// Limit: A limit will provide a min/max angle on every
///   axis that the objects must be between.
/// Motor: A motor will attempt to drive the rotation on every axis forward.
/// Spring: A spring will make the rotations on every axis at the bounds springy.
struct FixedAngleJoint : public Joint
{
  DeclareJointType(FixedAngleJoint);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAngleAccessors(FixedAngleJoint, mReferenceAngle);

  FixedAngleJoint();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void ComputeInitialConfiguration() override;

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

  AngleAtom mReferenceAngle;
  ConstraintAtom mAtoms[3];
  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::FixedAngleJoint FixedAngleJoint;

}//namespace Zero
