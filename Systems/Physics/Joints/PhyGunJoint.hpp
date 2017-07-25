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

/// Legacy. A physics gun joint is an experimental joint for picking up objects. This acts as a weld
/// between an object and the world. Primarily an experiment for picking up objects as a player.
/// Should be custom implemented in script with CustomJoint instead.
struct PhyGunJoint : public Joint
{
  DeclareJointType(PhyGunJoint);

  PhyGunJoint();

  // Component Interface
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
  void BatchEvents() {};

  /// The point in world space that the object's point should match.
  Vec3 GetTargetPoint();
  void SetTargetPoint(Vec3Param target);
  /// The local point on the object that should match the target point.
  Vec3 GetLocalPoint();
  void SetLocalPoint(Vec3Param bodyPoint);
  /// The world point on the object that should match the target point.
  Vec3 GetWorldPoint();
  void SetWorldPoint(Vec3Param worldPoint);
  /// The world space rotation that the basis of the object should match.
  /// Used to set the desired rotation of the object in world space.
  Quat GetTargetRotation();
  void SetTargetRotation(QuatParam target);
  /// Used to set the world rotation basis of the object that should be matched to the target rotation.
  Quat GetWorldRotation();
  void SetWorldRotation(QuatParam worldRotation);

  AnchorAtom mAnchors;
  AngleAtom mReferenceAngle;
  ConstraintAtom mAtoms[6];

  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::PhyGunJoint PhyGunJoint;

}//namespace Zero
