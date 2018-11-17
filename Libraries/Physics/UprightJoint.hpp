///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{

/// A joint to keep an object upright. Locks two axes of the objects together
/// but allows free rotation on the plane defined by that axis. This constraint
/// is useful for keeping any object upright. This could also be used to auto
/// correct an object slowly by lowering the max impulse value of the constraint.
class UprightJoint : public Joint
{
public:
  DeclareJointType(UprightJoint);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAxisAccessors(UprightJoint, mAxes);

  UprightJoint();

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

  ConstraintAtom mAtoms[3];

  AxisAtom mAxes;

  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::UprightJoint UprightJoint;

}//namespace Zero
