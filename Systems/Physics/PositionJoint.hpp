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

/// Represents a ball and socket joint. A position joint constrains the two
/// anchor points on each object to be equal.
/// Limits, motors, and springs typically should not be used.
/// Add on definitions:
/// Limit: A limit will provide a min/max translation on every
///   axis (x,y,z) that the objects must be between.
/// Motor: A motor will attempt to drive the translation in the
/// positive direction on every axis.
/// Spring: A spring will make the translation on every
/// axis springy at the bounds.
struct PositionJoint : public Joint
{
  DeclareJointType(PositionJoint);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAnchorAccessors(PositionJoint, mAnchors);

  PositionJoint();
  
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

  AnchorAtom mAnchors;
  ConstraintAtom mAtoms[3];
  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::PositionJoint PositionJoint;

}//namespace Zero
