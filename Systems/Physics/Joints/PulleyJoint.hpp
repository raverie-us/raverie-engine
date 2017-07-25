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

struct StickJoint;

/// A PulleyJoint turns two StickJoints into a pulley via a pulley ratio.
/// A PulleyJoint connects the two free objects of two different stick joints.
/// These two objects will then be bound to move together via the formula
/// "length0 + ratio * length1 = 0". Limits, motors and springs should not
/// be used on a pulley.
struct PulleyJoint : public Joint
{
  DeclareJointType(PulleyJoint);

  PulleyJoint();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void OnDestroy(uint flags = 0) override;

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

  /// The ratio between the two stick joints.
  /// The ratio is used in the formula "length0 + ratio * length1 = 0".
  real GetRatio() const;
  void SetRatio(real ratio);

  /// The joint connected to ObjectA that the pulley operates on.
  CogPath GetJointAPath();
  void SetJointAPath(CogPath& cogPath);
  /// The joint connected to ObjectB that the pulley operates on.
  CogPath GetJointBPath();
  void SetJointBPath(CogPath& cogPath);

  /// The joint connected to ObjectA that the pulley operates on.
  Cog* GetJointA();
  void SetJointA(Cog* cog);
  /// The joint connected to ObjectB that the pulley operates on.
  Cog* GetJointB();
  void SetJointB(Cog* cog);

  void SpecificJointRelink(uint index, Collider* collider) override;
  void RelinkJoint(uint index, Cog* cog);
  bool FindAndSetJoint(Collider* collider, uint index);
  void ValidateJoints();

  struct JointUnion
  {
    StickJoint* mJoint;
    CogPath mCogPath;
    uint mObjId;
  };

  JointUnion mJoints[2];
  real mRatio;
  Physics::ConstraintAtom mAtoms[1];

  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::PulleyJoint PulleyJoint;

}//namespace Zero
