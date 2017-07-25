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

/// A stick joint is used to simulate a rope or a stick. This joint forces
/// a constant distance between the anchor points on the objects. If there
/// is no limit, this behaves as a stick. If there is a limit, then this
/// behaves as a rope. Motors and springs are also applied to the axis of
/// the rope.
/// Add on definitions:
/// Limit: A limit will provide a min/max distance that
///   the anchors can be between.
/// Motor: A motor will push/pull the objects in the direction of the rope.
///   The motor will not have any effect unless a limit or spring is present.
/// Spring: A spring will make the rope behave spring-like at its boundaries.
struct StickJoint : public Joint
{
  DeclareJointType(StickJoint);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAnchorAccessors(StickJoint, mAnchors);

  StickJoint();
  
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

  /// The desired length between the anchor points of object A and B.
  real GetLength() const;
  void SetLength(real length);

  AnchorAtom mAnchors;
  ConstraintAtom mAtoms[1];
  real mLength;
  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::StickJoint StickJoint;

}//namespace Zero
