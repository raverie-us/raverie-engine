///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

/*
  Let i correspond to the x,y,z axes.
  Ci   : dot(p2 - p1,i) = 0
  Ci   : dot(c2 + r2 - c1 - r1,i) = 0
  cDoti: dot(v2,i) + dot(cross(w2,r2),i) - dot(v1,i) - dot(cross(w1,r1),i) = 0
  cDoti: dot(v2,i) + dot(w2,cross(r2,i)) - dot(v1,i) - dot(w1,cross(r1,i)) = 0
  Ji   : [-i, -cross(r1,i), i, cross(r2,i)]
*/

namespace Zero
{

namespace Physics
{

JointInfo PositionJoint::sInfo = JointInfo(3, 0);

ImplementJointType(PositionJoint);

ImplementAnchorAccessors(PositionJoint, mAnchors);

ZilchDefineType(PositionJoint, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  BindAnchorAccessors(Vec3(1));
}

PositionJoint::PositionJoint()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
}

void PositionJoint::Serialize(Serializer& stream)
{
  Joint::Serialize(stream);

  SerializeAnchors(stream, mAnchors);
}

void PositionJoint::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void PositionJoint::ComputeInitialConfiguration()
{
  // Grab the body points from the object link
  ComputeCurrentAnchors(mAnchors);
}

void PositionJoint::ComputeMoleculeData(MoleculeData& moleculeData)
{
  moleculeData.SetUp(&mAnchors, nullptr, this);
  moleculeData.SetLinearIdentity();
  moleculeData.SetAngularIdentity();
}

void PositionJoint::UpdateAtoms()
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData);
}

uint PositionJoint::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void PositionJoint::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData);
}

void PositionJoint::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void PositionJoint::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void PositionJoint::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint PositionJoint::PositionMoleculeCount() const
{
  return GetPositionMoleculeCount(this);
}

void PositionJoint::ComputePositionMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputePositionMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData);
}

void PositionJoint::DebugDraw()
{
  if(!GetValid())
    return;
  DrawAnchorAtomFragment(mAnchors, GetCollider(0), GetCollider(1));
}

uint PositionJoint::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = 0;
  return LinearAxis;
}

void PositionJoint::BatchEvents()
{
  BatchEventsFragment(this, sInfo.mAtomCount);
}

uint PositionJoint::GetDefaultLimitIds() const
{
  return AllAxes;
}

uint PositionJoint::GetDefaultMotorIds() const
{
  return AllAxes;
}

uint PositionJoint::GetDefaultSpringIds() const
{
  return AllAxes;
}

}//namespace Physics

}//namespace Zero
