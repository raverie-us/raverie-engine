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

/// A joint that models a wheel with shocks. A wheel aligns the two local axes
/// together and allows free rotation about this axis. The specified shock axis
/// is turned into a soft constraint to model the shocks. Note: ObjectA should
/// be the root object as the shock axis rotates with this object. If ObjectA
/// is the wheel, then the shock axis will rotate with the wheel, causing the
/// shocks to not stay aligned.
/// Add on definitions:
/// Limit: A limit will provide a min/max angle on the motor axis.
/// Motor: A motor will turn the objects about the motor axis.
/// Spring: A spring will make the shock axis springy.
///   A spring is attached by default to a wheel.
struct WheelJoint : public Joint
{
  DeclareJointType(WheelJoint);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAnchorAccessors(WheelJoint, mAnchors);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAxisAccessors(WheelJoint, mAxes);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAngleAccessors(WheelJoint, mReferenceAngle);

  WheelJoint();
  
  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
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

  /// The shock axis in the local space of body A.
  Vec3 GetShockAxis() const;
  void SetShockAxis(Vec3Param axis);
  /// The shock axis after it has been translated into world space.
  Vec3 GetWorldShockAxis() const;
  void SetWorldShockAxis(Vec3Param axis);

  AnchorAtom mAnchors;
  AxisAtom mAxes;
  AxisAtom mShockAxes;
  AngleAtom mReferenceAngle;
  ConstraintAtom mAtoms[6];
  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::WheelJoint WheelJoint;

}//namespace Zero
