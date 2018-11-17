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

/// A weld joint is used to lock the position and orientation of two objects
/// together. Welds should generally not be used to make two objects rigid as they
/// are computationally more expensive and less rigid than using composites. The
/// primary uses for welds are for quick connections or connections that are desired
/// to not be fully rigid. Also, while it can be done with composites too, welds
/// can be used to model object breaking beyond some max force.
/// Limits, motors and springs should most likely not be used on this.
/// Add on definitions:
/// Limit: A limit will provide a min/max translation on the x, y, and z axes.
/// Motor: A motor will attempt to drive the rotation on the x, y, and z axes.
/// Spring: A spring will make the x, y, and z axis springy.
struct WeldJoint : public Joint
{
  DeclareJointType(WeldJoint);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAnchorAccessors(WeldJoint, mAnchors);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAngleAccessors(WeldJoint, mReferenceAngle);

  WeldJoint();
  
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
  AngleAtom mReferenceAngle;
  ConstraintAtom mAtoms[6];
  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::WeldJoint WeldJoint;

}//namespace Zero
