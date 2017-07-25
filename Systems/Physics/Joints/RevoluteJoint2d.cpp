///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

/*
Linear Constraint:
  Let i correspond to the x,y,z axes
  Ci   : dot(p2 - p1,i) = 0
  Ci   : dot(c2 + r2 - c1 - r1,i) = 0
  cDoti: dot(v2,i) + dot(cross(w2,r2),i) - dot(v1,i) - dot(cross(w1,r1),i) = 0
  cDoti: dot(v2,i) + dot(w2,cross(r2,i)) - dot(v1,i) - dot(w1,cross(r1,i)) = 0
  Ji   : [-i, -cross(r1,i), i, cross(r2,i)]

  Angular Constraint:
  Let i correspond the 3 basis vectors formed from the motor axis and two
  orthonormal vectors (t1,t2) to the motor axis where 
  i = 0 -> t1, i = 1 -> t2, i = 2 -> motor axis
  Let i correspond to the x,y,z axes
  Ci   : theta2_i - theta1_i = 0
  cDoti: dot(w2,i) - dot(w1,i) = 0
  Ji   : [0, -i, 0, i]

  normally i = 2 is only solved when it's limit is breached, when a motor is
  present, or is just not solved at all
*/

namespace Zero
{

namespace Physics
{

JointInfo RevoluteJoint2d::sInfo = JointInfo(3, 0x4);
typedef DefaultAngularLimitPolicy2d<RevoluteJoint2d> FragmentPolicy;

struct Revolute2dPolicy : public DefaultFragmentPolicy2d<RevoluteJoint2d>
{
  // Returns baumgarte
  real AxisFragment(MoleculeData& data, int atomIndex, RevoluteJoint2d* joint, ConstraintMolecule& mol)
  {
    real baumgarte;
    uint axisIndex = atomIndex % 2;
    real desiredConstraintValue = 0;
    uint filter = joint->GetAtomIndexFilter(atomIndex, desiredConstraintValue);

    // Compute the linear or angular fragment Jacobian
    if(filter & RevoluteJoint2d::LinearAxis)
    {
      LinearAxisFragment2d(data.mAnchors, data.LinearAxes[axisIndex], mol);
      baumgarte = joint->GetLinearBaumgarte(); 
    }
    else if(filter & RevoluteJoint2d::AngularAxis)
    {
      AngularAxisFragment2d(data.mRefAngle, mol);
      baumgarte = joint->GetAngularBaumgarte();
    }
    else
      ErrorIf(true, "Joint %s of index %d returned an invalid index filter.", joint->GetJointName(), atomIndex);

    return baumgarte;
  }
};

ImplementJointType(RevoluteJoint2d);

ImplementAnchorAccessors(RevoluteJoint2d, mAnchors);
ImplementAngleAccessors(RevoluteJoint2d, mReferenceAngle);

ZilchDefineType(RevoluteJoint2d, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  BindAnchorAccessors(Vec3(1));
}

RevoluteJoint2d::RevoluteJoint2d()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
}

void RevoluteJoint2d::Serialize(Serializer& stream)
{
  Joint::Serialize(stream);

  SerializeAnchors(stream, mAnchors, Vec3(1));
}

void RevoluteJoint2d::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void RevoluteJoint2d::ComputeInitialConfiguration()
{
  // Grab the body points from the object link
  ComputeCurrentAnchors(mAnchors);
}

void RevoluteJoint2d::ComponentAdded(BoundType* typeId, Component* component)
{
  Joint::ComponentAdded(typeId,component);
  if(typeId == ZilchTypeId(JointLimit))
  {
    JointLimit* limit = static_cast<JointLimit*>(component);
    limit->mMinErr = -Math::cPi * real(0.25);
    limit->mMaxErr = Math::cPi * real(0.25);
  }
}

void RevoluteJoint2d::ComputeMoleculeData(MoleculeData& moleculeData)
{
  moleculeData.SetUp(&mAnchors, nullptr, nullptr, this);
  moleculeData.SetLinearIdentity();
  moleculeData.AngularAxes[0] = Vec3::cZAxis;
  moleculeData.Set2dBases(this);
}

void RevoluteJoint2d::UpdateAtoms()
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData, FragmentPolicy());
}

uint RevoluteJoint2d::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void RevoluteJoint2d::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, FragmentPolicy());
}

void RevoluteJoint2d::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void RevoluteJoint2d::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void RevoluteJoint2d::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint RevoluteJoint2d::PositionMoleculeCount() const
{
  return GetPositionMoleculeCount(this);
}

void RevoluteJoint2d::ComputePositionMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputePositionMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, FragmentPolicy());
}

void RevoluteJoint2d::DebugDraw()
{
  if(!GetValid())
    return;
  DrawAnchorAtomFragment(mAnchors, GetCollider(0), GetCollider(1));
  DrawAngleAtomFragment(mReferenceAngle, GetCollider(0), GetCollider(1));
}

uint RevoluteJoint2d::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = 0;
  if(atomIndex < 2)
    return LinearAxis;
  else if(atomIndex < 3)
    return AngularAxis;
  return 0;
}

void RevoluteJoint2d::BatchEvents()
{
  BatchEventsFragment(this, sInfo.mAtomCount);
}

uint RevoluteJoint2d::GetDefaultLimitIds() const
{
  return 0x4;
}

uint RevoluteJoint2d::GetDefaultMotorIds() const
{
  return 0x4;
}

uint RevoluteJoint2d::GetDefaultSpringIds() const
{
  return 0x4;
}

real RevoluteJoint2d::GetJointAngle() const
{
  return 0.0f;
  //return mAtoms[5].mError;
  /*Quat referenceRotation = mReferenceAngle.GetReferenceAngle();
  return JointHelpers::GetRelativeAngle(referenceRotation,
    GetCollider(0)->GetWorldRotation(), GetCollider(1)->GetWorldRotation(),mAxes[0]);*/
}

Vec3 RevoluteJoint2d::GetWorldAxis() const
{
  return Vec3::cZAxis;
}

}//namespace Physics

}//namespace Zero
