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

/// A joint that models a wheel with shocks. This is the 2d version of
/// WheelJoint. This joint is used in 2d mode for increased performance
/// and stability. The motor axis is automatically set to the z axis,
/// as that is the only axis objects can rotate about. Also, the translation
/// on the z axis is ignored so that objects can be arbitrarily far apart.
/// Add on definitions:
/// Limit: A limit will provide a min/max angle on the motor axis.
/// Motor: A motor will turn the objects about the motor axis.
/// Spring: A spring will make the shock axis springy.
///   A spring is attached by default to a wheel.
struct WheelJoint2d : public Joint
{
  DeclareJointType(WheelJoint2d);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAnchorAccessors(WheelJoint2d, mAnchors);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAngleAccessors(WheelJoint2d, mReferenceAngle);

  WheelJoint2d();
  
  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer);
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

  /// The shock axis of the wheel in object A's local space.
  Vec3 GetShockAxis() const;
  void SetShockAxis(Vec3Param axis);
  /// The shock axis of the wheel in world space.
  Vec3 GetWorldShockAxis() const;
  void SetWorldShockAxis(Vec3Param axis);

  AnchorAtom mAnchors;
  AxisAtom mShockAxes;
  AngleAtom mReferenceAngle;
  ConstraintAtom mAtoms[6];
  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::WheelJoint2d WheelJoint2d;

}//namespace Zero
