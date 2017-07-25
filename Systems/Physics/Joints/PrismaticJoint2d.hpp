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

/// A prismatic joint is used to create something similar to a piston.
/// This is the 2d version of PrismaticJoint. This joint is used
/// in 2d mode for increased performance and stability. The slider axis is
/// projected onto the z axis (the z component is ignored) so that the
/// objects can be arbitrarily far apart. The x and y rotations are also
/// ignored since the objects are only allowed to rotate about the z axis.
/// Add on definitions:
/// Limit: A limit will provide a min/max translational distance for the two
///   objects on the slider axis.
/// Motor: A motor will push/pull the objects on the slider axis.
/// Spring: A spring will make the slider axis springy at its limits.
struct PrismaticJoint2d : public Joint
{
  DeclareJointType(PrismaticJoint2d);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAnchorAccessors(PrismaticJoint2d, mAnchors);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAxisAccessors(PrismaticJoint2d, mAxes);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAngleAccessors(PrismaticJoint2d, mReferenceAngle);

  PrismaticJoint2d();

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

  real GetJointTranslation() const;

  AnchorAtom mAnchors;
  AxisAtom mAxes;
  AngleAtom mReferenceAngle;
  ConstraintAtom mAtoms[3];
  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::PrismaticJoint2d PrismaticJoint2d;

}//namespace Zero
