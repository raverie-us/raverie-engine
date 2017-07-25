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

struct RevoluteJoint;
struct PrismaticJoint;
struct RevoluteJoint2d;
struct PrismaticJoint2d;

/// A gear connects two joints on two objects together.
/// Either joint can be a prismatic or a revolute.
/// A gear ratio is used to bind the two joints together.
/// Limits, motors, and springs should not be used on this.
struct GearJoint : public Joint
{
  DeclareJointType(GearJoint);

  GearJoint();

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

  /// The gear ratio that the two constraints are bound with.
  real GetRatio() const;
  void SetRatio(real ratio);
  /// The initial offset of the gear ratio.
  real GetConstant() const;
  void SetConstant(real constant);

  /// The joint connected to ObjectA that the gear operates on.
  CogPath GetJointAPath();
  void SetJointAPath(CogPath& cogPath);
  /// The joint connected to ObjectB that the gear operates on.
  CogPath GetJointBPath();
  void SetJointBPath(CogPath& cogPath);

  /// The joint connected to ObjectA that the gear operates on.
  Cog* GetJointA();
  void SetJointA(Cog* cog);
  /// The joint connected to ObjectB that the gear operates on.
  Cog* GetJointB();
  void SetJointB(Cog* cog);

  void SpecificJointRelink(uint index, Collider* collider) override;
  void RelinkJoint(uint index, Cog* cog);
  bool FindAndSetJoint(Collider* collider, uint index);
  bool ValidateJoint(uint index);
  void ValidateJoints();
  Joint* GetValidJointOnCog(Cog* cog);

  enum BoundType
  {
    RevJoint, RevJoint2d, PrismJoint, PrismJoint2d
  };

  struct JointUnion
  {
    union
    {
      Joint* mJoint;
      RevoluteJoint* mRevolute;
      RevoluteJoint2d* mRevolute2d;
      PrismaticJoint* mPrismatic;
      PrismaticJoint2d* mPrismatic2d;
    };
    CogPath mCogPath;
    uint mObjIndex;
    uint mBoundType;
  };

  JointUnion mJoints[2];
  real mRatio;
  real mConstant;
  Physics::ConstraintAtom mAtoms[1];

  uint mBindingFlags;

  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::GearJoint GearJoint;

}//namespace Zero
