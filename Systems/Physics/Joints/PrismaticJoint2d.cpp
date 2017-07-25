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
  orthonormal vectors (t1,t2) to the motor axis where 
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

JointInfo PrismaticJoint2d::sInfo = JointInfo(3, 0x2);

struct Prismatic2dPolicy : public DefaultFragmentPolicy2d<PrismaticJoint2d>
{
  // Returns baumgarte
  real AxisFragment(MoleculeData& data, int atomIndex, PrismaticJoint2d* joint, ConstraintMolecule& mol)
  {
    real baumgarte;
    uint axisIndex = atomIndex % 2;
    real desiredConstraintValue = 0;
    uint filter = joint->GetAtomIndexFilter(atomIndex, desiredConstraintValue);

    // Compute the linear or angular fragment Jacobian
    if(filter & PrismaticJoint2d::LinearAxis)
    {
      PrismaticAxisFragment2d(data.mAnchors, data.LinearAxes[axisIndex], mol);
      baumgarte = joint->GetLinearBaumgarte(); 
    }
    else if(filter & PrismaticJoint2d::AngularAxis)
    {
      AngularAxisFragment2d(data.mRefAngle, mol);
      baumgarte = joint->GetAngularBaumgarte();
    }
    else
      ErrorIf(true, "Joint %s of index %d returned an invalid index filter.", joint->GetJointName(), atomIndex);

    return baumgarte;
  }
};

ImplementJointType(PrismaticJoint2d);

ImplementAnchorAccessors(PrismaticJoint2d, mAnchors);
ImplementAxisAccessors(PrismaticJoint2d, mAxes);
ImplementAngleAccessors(PrismaticJoint2d, mReferenceAngle);

ZilchDefineType(PrismaticJoint2d, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  BindAnchorAccessors(Vec3(1));
  BindAngleAccessors();
  BindAxisAccessors(Vec3::cYAxis);
}

PrismaticJoint2d::PrismaticJoint2d()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
}

void PrismaticJoint2d::Serialize(Serializer& stream)
{
  Joint::Serialize(stream);

  SerializeAnchors(stream, mAnchors);
  SerializeAngles(stream, mReferenceAngle);
  SerializeAxes(stream, mAxes, Vec3::cYAxis);
}

void PrismaticJoint2d::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void PrismaticJoint2d::ComputeInitialConfiguration()
{
  // First just grab the body points from the object link
  ComputeCurrentAnchors(mAnchors);

  // Next compute the axis of the prismatic as the axis between the two points
  Vec3 p0 = GetWorldPointA();
  Vec3 p1 = GetWorldPointB();
  Vec3 axis = p1 - p0;
  // The axis can't do anything on the z axis, so make sure to remove that
  axis.z = real(0.0);
  real length = axis.AttemptNormalize();
  // If we got an invalid axis then just use the y axis...
  if(length == real(0.0))
    axis = Vec3::cYAxis;
  SetWorldAxis(axis);

  // Then compute the reference angle that'll preserve
  // the current rotation between the objects
  ComputeCurrentReferenceAngle(mReferenceAngle);
}

void PrismaticJoint2d::ComputeMoleculeData(MoleculeData& moleculeData)
{
  WorldAxisAtom worldAxes(mAxes, GetCollider(0), GetCollider(1));
  moleculeData.SetUp(&mAnchors, &mReferenceAngle, this);
  moleculeData.LinearAxes[1] = worldAxes.mWorldAxes[0];
  moleculeData.LinearAxes[0] = Math::Cross(moleculeData.LinearAxes[1], Vec3::cZAxis);
  moleculeData.AngularAxes[0] = Vec3::cZAxis;

  moleculeData.LinearAxes[1].Normalize();
  moleculeData.LinearAxes[0].Normalize();
}

void PrismaticJoint2d::UpdateAtoms()
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData, Prismatic2dPolicy());
}

uint PrismaticJoint2d::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void PrismaticJoint2d::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, Prismatic2dPolicy());
}

void PrismaticJoint2d::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void PrismaticJoint2d::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void PrismaticJoint2d::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint PrismaticJoint2d::PositionMoleculeCount() const
{
  return GetPositionMoleculeCount(this);
}

void PrismaticJoint2d::ComputePositionMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputePositionMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, Prismatic2dPolicy());
}

void PrismaticJoint2d::DebugDraw()
{
  if(!GetValid())
    return;
  DrawAnchorAtomFragment(mAnchors, GetCollider(0), GetCollider(1));
  DrawAngleAtomFragment(mReferenceAngle, GetCollider(0), GetCollider(1));
  DrawAxisAtomFragment(mAxes, mAnchors, GetCollider(0), GetCollider(1));
}

uint PrismaticJoint2d::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = 0;
  if(atomIndex < 2)
    return LinearAxis;
  else if(atomIndex < 3)
    return AngularAxis;
  return 0;
}

void PrismaticJoint2d::BatchEvents()
{
  BatchEventsFragment(this, sInfo.mAtomCount);
}

uint PrismaticJoint2d::GetDefaultLimitIds() const
{
  return 0x2;
}

uint PrismaticJoint2d::GetDefaultMotorIds() const
{
  return 0x2;
}

uint PrismaticJoint2d::GetDefaultSpringIds() const
{
  return 0x2;
}

real PrismaticJoint2d::GetJointTranslation() const
{
  WorldAnchorAtom anchors(mAnchors, GetCollider(0), GetCollider(1));
  WorldAxisAtom axes(mAxes, GetCollider(0), GetCollider(1));

  return Math::Dot(anchors.GetPointDifference(), axes[0]);
}

}//namespace Physics

}//namespace Zero
