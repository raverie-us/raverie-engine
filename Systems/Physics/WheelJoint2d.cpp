///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

/*
  Linear Constraint:
  Let i correspond the 3 basis vectors formed from the shock axis and two
  orthonormal vectors (t1,t2) to the shock axis where 
  i = 0 -> t1, i = 1 -> t2, i = 2 -> shock axis
  Ci   : dot(p2 - p1,i) = 0
  Ci   : dot(c2 + r2 - c1 - r1,i) = 0
  cDoti: dot(v2,i) + dot(cross(w2,r2),i) - dot(v1,i) - dot(cross(w1,r1),i) = 0
  cDoti: dot(v2,i) + dot(w2,cross(r2,i)) - dot(v1,i) - dot(w1,cross(r1,i)) = 0
  Ji   : [-i, -cross(r1,i), i, cross(r2,i)]

  a spring atom is added to i = 2 as the shocks of the wheel

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

JointInfo WheelJoint2d::sInfo = JointInfo(3, 0x4);
typedef DefaultFragmentPolicy2d<WheelJoint2d> FragmentPolicy;

ImplementJointType(WheelJoint2d);

ImplementAnchorAccessors(WheelJoint2d, mAnchors);
ImplementAngleAccessors(WheelJoint2d, mReferenceAngle);

ZilchDefineType(WheelJoint2d, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  BindAnchorAccessors(Vec3(1));
  BindAngleAccessors();
  ZilchBindGetterSetterProperty(ShockAxis)->ZeroSerialize(Vec3::cYAxis);
  // @MetaSerialization: Property needs to cause rescans
  ZilchBindGetterSetterProperty(WorldShockAxis);
}

WheelJoint2d::WheelJoint2d()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
}

void WheelJoint2d::Serialize(Serializer& stream)
{
  Joint::Serialize(stream);

  SerializeAnchors(stream, mAnchors);
  SerializeAngles(stream, mReferenceAngle);
  stream.SerializeFieldDefault("ShockAxis", mShockAxes[0], Vec3::cYAxis);
  mShockAxes[1] = mShockAxes[0];
}

void WheelJoint2d::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void WheelJoint2d::OnAllObjectsCreated(CogInitializer& initializer)
{
  Joint::OnAllObjectsCreated(initializer);

  JointSpring* spring = GetOwner()->has(JointSpring);
  if(spring == nullptr)
  {
    spring = new JointSpring();
    GetOwner()->AddComponent(spring);
  }

  spring->mAtomIds = 1 << 1;
}

void WheelJoint2d::ComputeInitialConfiguration()
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
  SetWorldShockAxis(axis);
}

void WheelJoint2d::ComputeMoleculeData(MoleculeData& moleculeData)
{
  WorldAxisAtom worldShockAxes(mShockAxes, GetCollider(0), GetCollider(1));
  moleculeData.SetUp(&mAnchors, nullptr, nullptr, this);
  worldShockAxes.mWorldAxes[0].z = 0;
  worldShockAxes.mWorldAxes[0].AttemptNormalize();
  moleculeData.LinearAxes[1] = worldShockAxes.mWorldAxes[0];
  moleculeData.LinearAxes[0] = Math::Cross(moleculeData.LinearAxes[1], Vec3::cZAxis);
  moleculeData.AngularAxes[0] = Vec3::cZAxis;
  moleculeData.Set2dBases(this);
}

void WheelJoint2d::ComponentAdded(BoundType* typeId, Component* component)
{
  Joint::ComponentAdded(typeId,component);
  if(typeId == ZilchTypeId(JointLimit))
  {
    JointLimit* limit = static_cast<JointLimit*>(component);
    limit->mMinErr = -Math::cPi * real(0.25);
    limit->mMaxErr = Math::cPi * real(0.25);
  }
}

void WheelJoint2d::UpdateAtoms()
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData, FragmentPolicy());
}

uint WheelJoint2d::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void WheelJoint2d::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, FragmentPolicy());
}

void WheelJoint2d::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void WheelJoint2d::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void WheelJoint2d::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint WheelJoint2d::PositionMoleculeCount() const
{
  return GetPositionMoleculeCount(this);
}

void WheelJoint2d::ComputePositionMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputePositionMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, FragmentPolicy());
}

void WheelJoint2d::DebugDraw()
{
  if(!GetValid())
    return;
  DrawAnchorAtomFragment(mAnchors, GetCollider(0), GetCollider(1));
  DrawAngleAtomFragment(mReferenceAngle, GetCollider(0), GetCollider(1));
  DrawAxisAtomFragment(mShockAxes, mAnchors, GetCollider(0), GetCollider(1));
}

uint WheelJoint2d::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = 0;
  if(atomIndex < 2)
    return LinearAxis;
  else if(atomIndex < 3)
    return AngularAxis;
  return 0;
}

void WheelJoint2d::BatchEvents()
{
  BatchEventsFragment(this, sInfo.mAtomCount);
}

uint WheelJoint2d::GetDefaultLimitIds() const
{
  return 0x4;
}

uint WheelJoint2d::GetDefaultMotorIds() const
{
  return 0x4;
}

uint WheelJoint2d::GetDefaultSpringIds() const
{
  return 0x2;
}

Vec3 WheelJoint2d::GetShockAxis() const
{
  return mShockAxes[0];
}

void WheelJoint2d::SetShockAxis(Vec3Param axis)
{
  mShockAxes[0] = axis;
}

Vec3 WheelJoint2d::GetWorldShockAxis() const
{
  Collider* collider = GetCollider(0);
  if(collider == nullptr)
    return Vec3::cZero;

  return JointHelpers::BodyToWorldR(collider, mShockAxes.mBodyAxes[0]);
}

void WheelJoint2d::SetWorldShockAxis(Vec3Param axis)
{
  // Register side-effect properties
  if(OperationQueue::IsListeningForSideEffects())
    OperationQueue::RegisterSideEffect(this, "ShockAxis", GetShockAxis());

  if(axis == Vec3::cZero)
    return;

  Collider* collider = GetCollider(0);
  if(collider == nullptr)
    return;

  mShockAxes.mBodyAxes[0] = JointHelpers::WorldToBodyR(collider, axis);
}

}//namespace Physics

}//namespace Zero
