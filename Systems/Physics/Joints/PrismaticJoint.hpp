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

/// A prismatic joint sometimes called a slider) is used to create something similar
/// to a piston. This joint fixes all degrees of rotation and leaves one linear axis free.
/// Therefore, the bodies will rotate with each other and move with each other, except for
/// one axis where they can move freely.
/// Add on definitions:
/// Limit: A limit will provide a min/max translational distance for the two
///   objects on the slider axis.
/// Motor: A motor will push/pull the objects on the slider axis.
/// Spring: A spring will make the slider axis springy at its limits.
struct PrismaticJoint : public Joint
{
  DeclareJointType(PrismaticJoint);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAnchorAccessors(PrismaticJoint, mAnchors);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAxisAccessors(PrismaticJoint, mAxes);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAngleAccessors(PrismaticJoint, mReferenceAngle);

  PrismaticJoint();
  
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

  uint PositionMoleculeCount() const;
  void ComputePositionMolecules(MoleculeWalker& molecules);

  void Commit(MoleculeWalker& molecules);
  void DebugDraw();
  uint GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const;
  void BatchEvents();

  // Joint Interface
  uint GetDefaultLimitIds() const override;
  uint GetDefaultMotorIds() const override;
  uint GetDefaultSpringIds() const override;

  real GetJointTranslation() const;

  AnchorAtom mAnchors;
  AxisAtom mAxes;
  AngleAtom mReferenceAngle;
  ConstraintAtom mAtoms[6];
  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::PrismaticJoint PrismaticJoint;

}//namespace Zero
