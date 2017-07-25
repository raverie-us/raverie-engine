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

namespace Physics
{

/// Legacy. A linear axis joint is used to keep an object locked on a plane
/// that is defined by a normal. This was made to help make a dynamic character
/// controller. Instead of locking translation along a plane, the constraint can
/// be turned off with a motor attached to it which will drive movement in the
/// direction of the plane normal. This can then be thought of as a "move in direction" constraint.
struct LinearAxisJoint : public Joint
{
  DeclareJointType(LinearAxisJoint);

  LinearAxisJoint();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;

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

  /// The axis in world space that is constrained.
  Vec3 GetWorldAxis();
  void SetWorldAxis(Vec3Param axis);

  Vec3 mWorldAxis;

  ConstraintAtom mAtoms[1];

  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::LinearAxisJoint LinearAxisJoint;

}//namespace Zero
