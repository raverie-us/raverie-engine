///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

/*
  Linear Constraint:
  Let i correspond the 3 basis vectors formed from the motor axis and two
  orthonormal vectors (t1, t2) to the motor axis where :
  i = 0 -> t1, i = 1 -> t2, i = 2 -> motor axis
  Ci   : dot(p2 - p1,i) = 0
  Ci   : dot(c2 + r2 - c1 - r1,i) = 0
  cDoti: dot(v2,i) + dot(cross(w2,r2),i) - dot(v1,i) - dot(cross(w1,r1),i) = 0
  cDoti: dot(v2,i) + dot(w2,cross(r2,i)) - dot(v1,i) - dot(w1,cross(r1,i)) = 0
  Ji   : [-i, -cross(r1,i), i, cross(r2,i)]

  normally i = 2 is only solved when it's limit is breached, when a motor is
  present, or is just not solved at all

  Angular Constraint:
  Let i correspond to the x,y,z axes
  Ci   : theta2_i - theta1_i = 0
  cDoti: dot(w2,i) - dot(w1,i) = 0
  Ji   : [0, -i, 0, i]
*/

namespace Zero
{

namespace Physics
{

JointInfo PrismaticJoint::sInfo = JointInfo(6, 0x4);

ImplementJointType(PrismaticJoint);

ImplementAnchorAccessors(PrismaticJoint, mAnchors);
ImplementAxisAccessors(PrismaticJoint, mAxes);
ImplementAngleAccessors(PrismaticJoint, mReferenceAngle);

PrismaticJoint::PrismaticJoint()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
}

ZilchDefineType(PrismaticJoint, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  BindAnchorAccessors(Vec3(1));
  BindAngleAccessors();
  BindAxisAccessors(Vec3::cYAxis);
}

void PrismaticJoint::Serialize(Serializer& stream)
{
  Joint::Serialize(stream);

  SerializeAnchors(stream, mAnchors);
  SerializeAngles(stream, mReferenceAngle);
  SerializeAxes(stream, mAxes, Vec3::cYAxis);
}

void PrismaticJoint::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void PrismaticJoint::ComputeInitialConfiguration()
{
  // First grab the body points from the object link
  ComputeCurrentAnchors(mAnchors);

  // Next compute the axis of the prismatic as the axis between the two points
  Vec3 p0 = GetWorldPointA();
  Vec3 p1 = GetWorldPointB();
  Vec3 axis = p1 - p0;
  real length = axis.AttemptNormalize();
  // If we got an invalid axis then just use the y axis...
  if(length == real(0.0))
    axis = Vec3::cYAxis;
  SetWorldAxis(axis);

  // Then compute the reference angle that'll preserve
  // the current rotation between the objects
  ComputeCurrentReferenceAngle(mReferenceAngle);
}

void PrismaticJoint::ComputeMoleculeData(MoleculeData& moleculeData)
{
  WorldAxisAtom worldAxes(mAxes, GetCollider(0), GetCollider(1));
  moleculeData.SetUp(&mAnchors, &mReferenceAngle, this);
  moleculeData.SetLinearBasis(worldAxes.mWorldAxes[0]);
  moleculeData.SetAngularIdentity();
}

void PrismaticJoint::UpdateAtoms()
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData);
}

uint PrismaticJoint::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void PrismaticJoint::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData);
}

void PrismaticJoint::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void PrismaticJoint::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void PrismaticJoint::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint PrismaticJoint::PositionMoleculeCount() const
{
  return GetPositionMoleculeCount(this);
}

void PrismaticJoint::ComputePositionMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputePositionMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData);
}

void PrismaticJoint::DebugDraw()
{
  if(!GetValid())
    return;
  DrawAnchorAtomFragment(mAnchors, GetCollider(0), GetCollider(1));
  DrawAngleAtomFragment(mReferenceAngle, GetCollider(0), GetCollider(1));
  DrawAxisAtomFragment(mAxes, mAnchors, GetCollider(0), GetCollider(1));
}

uint PrismaticJoint::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = 0;
  if(atomIndex < 3)
    return LinearAxis;
  else if(atomIndex < 6)
    return AngularAxis;
  return 0;
}

void PrismaticJoint::BatchEvents()
{
  BatchEventsFragment(this, sInfo.mAtomCount);
}

uint PrismaticJoint::GetDefaultLimitIds() const
{
  return SingleLinearAxis;
}

uint PrismaticJoint::GetDefaultMotorIds() const
{
  return SingleLinearAxis;
}

uint PrismaticJoint::GetDefaultSpringIds() const
{
  return SingleLinearAxis;
}

real PrismaticJoint::GetJointTranslation() const
{
  WorldAnchorAtom anchors(mAnchors, GetCollider(0), GetCollider(1));
  WorldAxisAtom axes(mAxes, GetCollider(0), GetCollider(1));

  return Math::Dot(anchors.GetPointDifference(), axes[0]);
}

}//namespace Physics

}//namespace Zero
