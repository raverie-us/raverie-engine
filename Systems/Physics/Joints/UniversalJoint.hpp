///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{

/// A universal joint locks positional movement of two points together as well
/// as locking rotation about one axis. This means it is a joint that constrains four
/// axes and leaves two free rotational axes. This joint is most useful to model
/// something like a arm or leg that has a large range of rotational movement.
struct UniversalJoint : public Joint
{
  DeclareJointType(UniversalJoint);
  /// <macro location="ConstraintAtomDefines.hpp" />
  DeclareAnchorAccessors(UniversalJoint, mAnchors);

  UniversalJoint();

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

  /// One of the two axes in local space of object A that
  /// the objects are allowed to rotate about.
  Vec3 GetLocalAxis0OfBodyA();
  void SetLocalAxis0OfBodyA(Vec3Param axis);
  /// One of the two axes in local space of object A that
  /// the objects are allowed to rotate about.
  Vec3 GetLocalAxis1OfBodyA();
  void SetLocalAxis1OfBodyA(Vec3Param axis);
  /// One of the two axes in local space of object B that
  /// the objects are allowed to rotate about.
  Vec3 GetLocalAxis0OfBodyB();
  void SetLocalAxis0OfBodyB(Vec3Param axis);
  /// One of the two axes in local space of object B that
  /// the objects are allowed to rotate about.
  Vec3 GetLocalAxis1OfBodyB();
  void SetLocalAxis1OfBodyB(Vec3Param axis);

  AnchorAtom mAnchors;

  Vec3 mBody0Axis0;
  Vec3 mBody0Axis1;
  Vec3 mBody1Axis0;
  Vec3 mBody1Axis1;

  ConstraintAtom mAtoms[6];
  static JointInfo sInfo;
};

}//namespace Physics

typedef Physics::UniversalJoint UniversalJoint;

}//namespace Zero
