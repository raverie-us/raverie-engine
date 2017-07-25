///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

/*
  Linear Constraint:
  Let i correspond to the x, y, z axes.
  Ci   : dot(p2 - p1,i) = 0
  Ci   : dot(c2 + r2 - c1 - r1,i) = 0
  cDoti: dot(v2,i) + dot(cross(w2,r2),i) - dot(v1,i) - dot(cross(w1,r1),i) = 0
  cDoti: dot(v2,i) + dot(w2,cross(r2,i)) - dot(v1,i) - dot(w1,cross(r1,i)) = 0
  Ji   : [-i, -cross(r1,i), i, cross(r2,i)]

  Angular Constraint:
  Let i correspond to the x, y, z axes.
  Ci   : theta2_i - theta1_i = 0
  cDoti: dot(w2,i) - dot(w1,i) = 0
  Ji   : [0, -i, 0, i]
*/

namespace Zero
{

namespace Physics
{

JointInfo WeldJoint::sInfo = JointInfo(6, 0);

ImplementJointType(WeldJoint);

ImplementAnchorAccessors(WeldJoint, mAnchors);
ImplementAngleAccessors(WeldJoint, mReferenceAngle);

ZilchDefineType(WeldJoint, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  BindAnchorAccessors(Vec3(1));
  BindAngleAccessors();
}

WeldJoint::WeldJoint()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
  mAnchors.mBodyR[0] = Vec3(0,1,0);
  mAnchors.mBodyR[1] = Vec3(0,-1,0);
}

void WeldJoint::Serialize(Serializer& stream)
{
  Joint::Serialize(stream);

  SerializeAnchors(stream, mAnchors);
  SerializeAngles(stream, mReferenceAngle);
}

void WeldJoint::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void WeldJoint::ComputeInitialConfiguration()
{
  // First just grab the body points from the object link
  ComputeCurrentAnchors(mAnchors);

  // Then compute the reference angle that'll preserve
  // the current rotation between the objects
  ComputeCurrentReferenceAngle(mReferenceAngle);
}

void WeldJoint::ComputeMoleculeData(MoleculeData& moleculeData)
{
  moleculeData.SetUp(&mAnchors, &mReferenceAngle, this);
  moleculeData.SetLinearIdentity();
  moleculeData.SetAngularIdentity();
}

void WeldJoint::UpdateAtoms()
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData);
}

uint WeldJoint::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void WeldJoint::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData);
}

void WeldJoint::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void WeldJoint::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void WeldJoint::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint WeldJoint::PositionMoleculeCount() const
{
  return GetPositionMoleculeCount(this);
}

void WeldJoint::ComputePositionMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputePositionMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData);
}

void WeldJoint::DebugDraw()
{
  if(!GetValid())
    return;
  DrawAnchorAtomFragment(mAnchors, GetCollider(0), GetCollider(1));
  DrawAngleAtomFragment(mReferenceAngle, GetCollider(0), GetCollider(1));
}

uint WeldJoint::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = 0;
  if(atomIndex < 3)
    return LinearAxis;
  else if(atomIndex < 6)
    return AngularAxis;
  return 0;
}

void WeldJoint::BatchEvents()
{
  BatchEventsFragment(this, sInfo.mAtomCount);
}

uint WeldJoint::GetDefaultLimitIds() const
{
  return AllLinearAxes;
}

uint WeldJoint::GetDefaultMotorIds() const
{
  return AllAngularAxes;
}

uint WeldJoint::GetDefaultSpringIds() const
{
  return AllLinearAxes;
}

}//namespace Physics

}//namespace Zero
