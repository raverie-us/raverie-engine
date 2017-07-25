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

/// A revolute joint is used to create a wheel or a hinge.
/// This is the 2d version of RevoluteJoint. This joint is used
/// in 2d mode for increased performance and stability. The motor axis is
/// automatically set to the z axis, as that is the only axis objects can
/// rotate about. Also, the translation on the z axis is ignored so that
/// objects can be arbitrarily far apart.
/// Add on definitions:
/// Limit: A limit will provide a min/max angle on the motor axis.
/// Motor: A motor will turn the objects about the motor axis.
/// Spring: A spring will make the motor axis springy at the limits.
struct RevoluteJoint2d : public Joint
{
  DeclareJointType(RevoluteJoint2d);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAnchorAccessors(RevoluteJoint2d, mAnchors);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAngleAccessors(RevoluteJoint2d, mReferenceAngle);

  RevoluteJoint2d();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void ComputeInitialConfiguration() override;
  void ComponentAdded(BoundType* typeId, Component* component) override;

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

  real GetJointAngle() const;
  Vec3 GetWorldAxis() const;

  AnchorAtom mAnchors;
  AngleAtom mReferenceAngle;
  ConstraintAtom mAtoms[6];
  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::RevoluteJoint2d RevoluteJoint2d;

}//namespace Zero
