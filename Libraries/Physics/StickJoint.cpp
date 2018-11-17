///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

/*
  C   : c5(p2 - p1)^2 -L^2 = 0
  cDot: (p2 - p1) * (v2 + cross(w2,r2) - v1 - cross(w1,r1) = 0
  Let : d = p2 - p1
  cDot: dot(d,v2) + dot(d,cross(w2,r2)) - dot(d,v1) - dot(d,cross(w1,r1)) = 0
  cDot: dot(d,v2) + dot(w2,cross(r2,d)) - dot(d,v1) - dot(w1,cross(r1,d)) = 0
  J   : [-d,-cross(r1,d),d,cross(r2,d)]
  Identity used:    a x b * c =     a * b x c     = c x a * b
            dot(cross(a,b),c) = dot(a,cross(b,c)) = dot(cross(c,a),b)
*/

namespace Zero
{

namespace Physics
{

JointInfo StickJoint::sInfo = JointInfo(1, 0);

ImplementJointType(StickJoint);

ImplementAnchorAccessors(StickJoint, mAnchors);

ZilchDefineType(StickJoint, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(Length);
  BindAnchorAccessors(Vec3::cZero);
}

StickJoint::StickJoint()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
}

void StickJoint::Serialize(Serializer& stream)
{
  Joint::Serialize(stream);

  SerializeNameDefault(mLength, real(2.0));
  SerializeAnchors(stream, mAnchors, Vec3::cZero);
}

void StickJoint::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void StickJoint::ComputeInitialConfiguration()
{
  // First just grab the body points from the object link
  ComputeCurrentAnchors(mAnchors);

  // Compute the current length
  Vec3 p0 = GetWorldPointA();
  Vec3 p1 = GetWorldPointB();
  mLength = (p1 - p0).Length();
}

void StickJoint::ComponentAdded(BoundType* typeId, Component* component)
{
  Joint::ComponentAdded(typeId,component);
  if(typeId == ZilchTypeId(JointLimit))
  {
    JointLimit* limit = static_cast<JointLimit*>(component);
    limit->mMinErr = 0;
    limit->mMaxErr = mLength;
  }
}

void StickJoint::ComputeMoleculeData(MoleculeData& moleculeData)
{  
  moleculeData.SetUp(&mAnchors, nullptr, this);
  moleculeData.LinearAxes[0] = moleculeData.mAnchors.GetPointDifference();
  moleculeData.LinearAxes[0].AttemptNormalize();
}

void StickJoint::UpdateAtoms()
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData);
}

uint StickJoint::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void StickJoint::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules,sInfo.mAtomCount, moleculeData);
}

void StickJoint::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void StickJoint::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void StickJoint::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint StickJoint::PositionMoleculeCount() const
{
  return GetPositionMoleculeCount(this);
}

void StickJoint::ComputePositionMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputePositionMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData);
}

void StickJoint::DebugDraw()
{
  if(!GetValid())
    return;
  DrawAnchorAtomFragment(mAnchors, GetCollider(0), GetCollider(1));
}

uint StickJoint::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = mLength;
  return LinearAxis;
}

void StickJoint::BatchEvents()
{
  BatchEventsFragment(this, sInfo.mAtomCount);
}

uint StickJoint::GetDefaultLimitIds() const
{
  return SingleAxis;
}

uint StickJoint::GetDefaultMotorIds() const
{
  return SingleAxis;
}

uint StickJoint::GetDefaultSpringIds() const
{
  return SingleAxis;
}

real StickJoint::GetLength() const
{
  return mLength;
}

void StickJoint::SetLength(real length)
{
  mLength = length;
}

}//namespace Physics

}//namespace Zero
