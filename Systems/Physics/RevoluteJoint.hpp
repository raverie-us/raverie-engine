///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum2(JointFrameOfReference, ObjectA, ObjectB);

namespace Physics
{

/// A revolute joint is used to create a wheel or a hinge.
/// A revolute joint bring the two locally defined axes together and allows
/// free rotation only on that axis. This axis is also where the motor is
/// applied. The two axes that are orthogonal to the motor axis have their
/// rotation locked (objects rotate together unless on the motor axis).
/// Add on definitions:
/// Limit: A limit will provide a min/max angle on the motor axis. Zero is defined by the
///        location of the primary axis on the FrameOfreference object.
/// Motor: A motor will drive the objects about the motor axis.
/// Spring: A spring will make the motor axis springy at the limits.
struct RevoluteJoint : public Joint
{
  DeclareJointType(RevoluteJoint);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAnchorAccessors(RevoluteJoint, mAnchors);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAngleAccessors(RevoluteJoint, mReferenceAngle);

  RevoluteJoint();
  
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
  
  /// The angle error of the joint. Needed for GearJoint.
  real GetJointAngle() const;

  // @JoshD: Remove later
  /// The axis of freedom for the joint in world space. This is the axis that
  /// rotational movement is allowed on. Which object's axis is used to compute
  /// this is determined by UseFrameA.
  Vec3 GetWorldAxis() const;
  void SetWorldAxis(Vec3Param axis);
  // @JoshD: Remove later
  /// Legacy. Used to set the entire frame in world space for this joint. The x and y
  /// axes are used as a basis for limiting the joint. The x axis is at angle 0
  /// and the y axis is at angle 90. The z axis is the axis of rotational freedom.
  void SetWorldFrame(QuatParam rot);

  /// The basis of the joint in world-space. This basis will come object specified by
  /// FrameOfReference. The basis is constructed such that the x-axis is the primary
  /// axis while the z-axis is the hinge axis.
  Quat GetWorldBasis();
  void SetWorldBasis(QuatParam basis);

  /// Should the default basis of the constraint be object A or B?
  /// This determines which object's world axis is used when constructing the
  /// basis for the constraint. In the case of a dynamic and static object, the
  /// static object is generally the better choice. As a general rule of thumb,
  /// it should be the heavier/most important object.
  JointFrameOfReference::Enum GetFrameOfReference() const;
  void SetFrameOfReference(JointFrameOfReference::Enum objectFrame);

  Quat BuildFrameFromAxis(QuatParam oldWorldFrame, Vec3Param axis);
  void SetBodyAxisInternal(uint objIndex, Vec3Param axis);

  AnchorAtom mAnchors;
  AngleAtom mReferenceAngle;
  uint mPrimaryFrameIndex;

  ConstraintAtom mAtoms[6];
  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::RevoluteJoint RevoluteJoint;

}//namespace Zero
