///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

/*
  Let i correspond to the x, y, z axes.
  Ci   : theta2_i - theta1_i = 0
  cDoti: dot(w2,i) - dot(w1,i) = 0
  Ji   : [0, -i, 0, i]
*/

namespace Zero
{

namespace Physics
{

JointInfo FixedAngleJoint::sInfo = JointInfo(3, 0);

ImplementJointType(FixedAngleJoint);

ImplementAngleAccessors(FixedAngleJoint, mReferenceAngle);

ZilchDefineType(FixedAngleJoint, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  BindAngleAccessors();
}

FixedAngleJoint::FixedAngleJoint()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
}

void FixedAngleJoint::Serialize(Serializer& stream)
{
  Joint::Serialize(stream);

  SerializeAngles(stream, mReferenceAngle);
}

void FixedAngleJoint::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void FixedAngleJoint::ComputeInitialConfiguration()
{
  // Compute the reference angle that'll preserve the current rotation between the objects
  ComputeCurrentReferenceAngle(mReferenceAngle);
}

void FixedAngleJoint::ComputeMoleculeData(MoleculeData& moleculeData)
{  
  moleculeData.SetUp(nullptr, &mReferenceAngle, this);
  moleculeData.SetLinearIdentity();
  moleculeData.SetAngularIdentity();
}

void FixedAngleJoint::UpdateAtoms()
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData);
}

uint FixedAngleJoint::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask); 
}

void FixedAngleJoint::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData);
}

void FixedAngleJoint::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void FixedAngleJoint::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void FixedAngleJoint::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint FixedAngleJoint::PositionMoleculeCount() const
{
  return GetPositionMoleculeCount(this);
}

void FixedAngleJoint::ComputePositionMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputePositionMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData);
}

void FixedAngleJoint::DebugDraw()
{
  if(!GetValid())
    return;
  DrawAngleAtomFragment(mReferenceAngle, GetCollider(0), GetCollider(1));
}

uint FixedAngleJoint::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = 0;
  return AngularAxis;
}

void FixedAngleJoint::BatchEvents()
{
  BatchEventsFragment(this, sInfo.mAtomCount);
}

uint FixedAngleJoint::GetDefaultLimitIds() const
{
  return AllAxes;
}

uint FixedAngleJoint::GetDefaultMotorIds() const
{
  return AllAxes;
}

uint FixedAngleJoint::GetDefaultSpringIds() const
{
  return AllAxes;
}

}//namespace Physics

}//namespace Zero
