///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{

/// Legacy. A position joint that is designed to manipulate one object.
/// The only difference between this and the position joint is that the
/// manipulator always draws itself, draws differently, and configures
/// the max impulse differently.
struct ManipulatorJoint : public Joint
{
  DeclareJointType(ManipulatorJoint);

  ManipulatorJoint();

  // Component Interface
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
  void BatchEvents() {};

  /// The point in world space that the object's point is being moved towards.
  Vec3 GetTargetPoint();
  void SetTargetPoint(Vec3Param target);

  /// The local space point on the object that the joint is connected to.
  Vec3 GetLocalPoint();
  void SetLocalPoint(Vec3Param bodyPoint);

  /// The world space point on the object that the joint is connected to.
  Vec3 GetWorldPoint();
  void SetWorldPoint(Vec3Param worldPoint);

  Physics::AnchorAtom mAnchors;
  Physics::ConstraintAtom mAtoms[3];

  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::ManipulatorJoint ManipulatorJoint;

}//namespace Zero
