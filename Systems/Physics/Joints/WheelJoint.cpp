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

JointInfo WheelJoint::sInfo = JointInfo(6, 0x20);

ImplementJointType(WheelJoint);

ImplementAnchorAccessors(WheelJoint, mAnchors);
ImplementAxisAccessors(WheelJoint, mAxes);
ImplementAngleAccessors(WheelJoint, mReferenceAngle);

ZilchDefineType(WheelJoint, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  BindAnchorAccessors(Vec3(1));
  BindAngleAccessors();
  BindAxisAccessors(Vec3::cYAxis);
  ZilchBindGetterSetterProperty(ShockAxis)->ZeroSerialize(Vec3::cYAxis);
  // @MetaSerialization: Property needs to cause rescans
  ZilchBindGetterSetterProperty(WorldShockAxis);
}

WheelJoint::WheelJoint()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
}

void WheelJoint::Serialize(Serializer& stream)
{
  Joint::Serialize(stream);

  SerializeAnchors(stream, mAnchors);
  SerializeAngles(stream, mReferenceAngle);
  SerializeAxes(stream, mAxes, Vec3::cYAxis);
  stream.SerializeFieldDefault("ShockAxis", mShockAxes[0], Vec3(0, 1, 0));
}

void WheelJoint::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void WheelJoint::OnAllObjectsCreated(CogInitializer& initializer)
{
  Joint::OnAllObjectsCreated(initializer);
  JointSpring* spring = HasOrAdd<JointSpring>(GetOwner());
  spring->mAtomIds = 1 << 2;
}

void WheelJoint::ComputeInitialConfiguration()
{
  // First just grab the body points from the object link
  ComputeCurrentAnchors(mAnchors);

  // Next compute the axis of the prismatic as the axis between the two points
  Vec3 p0 = GetWorldPointA();
  Vec3 p1 = GetWorldPointB();
  Vec3 axis = p1 - p0;
  // The axis can't do anything on the z axis, so make sure to remove that
  real length = axis.AttemptNormalize();
  // If we got an invalid axis then just use the y axis...
  if(length == real(0.0))
    axis = Vec3::cYAxis;
  SetWorldShockAxis(axis);
  SetWorldAxis(axis);
}

void WheelJoint::ComponentAdded(BoundType* typeId, Component* component)
{
  Joint::ComponentAdded(typeId,component);
  if(typeId == ZilchTypeId(JointLimit))
  {
    JointLimit* limit = static_cast<JointLimit*>(component);
    limit->mMinErr = -Math::cPi * real(0.25);
    limit->mMaxErr = Math::cPi * real(0.25);
  }
}

void WheelJoint::ComputeMoleculeData(MoleculeData& moleculeData)
{
  WorldAxisAtom worldAxes(mAxes, GetCollider(0), GetCollider(1));
  WorldAxisAtom worldShockAxes(mShockAxes, GetCollider(0), GetCollider(1));
  moleculeData.SetUp(&mAnchors, nullptr, &mAxes, this);
  
  moleculeData.SetLinearBasis(worldShockAxes.mWorldAxes[0]);
  moleculeData.SetAngularBasis(moleculeData.mAxes[0]);
}

void WheelJoint::UpdateAtoms()
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData, DefaultAngularLimitPolicy<WheelJoint>());
}

uint WheelJoint::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void WheelJoint::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, DefaultAngularLimitPolicy<WheelJoint>());
}

void WheelJoint::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void WheelJoint::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void WheelJoint::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint WheelJoint::PositionMoleculeCount() const
{
  return GetPositionMoleculeCount(this);
}

void WheelJoint::ComputePositionMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputePositionMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, DefaultAngularLimitPolicy<WheelJoint>());
}

void WheelJoint::DebugDraw()
{
  if(!GetValid())
    return;
  DrawAnchorAtomFragment(mAnchors, GetCollider(0), GetCollider(1));
  DrawAngleAtomFragment(mReferenceAngle, GetCollider(0), GetCollider(1));
  DrawAxisAtomFragment(mAxes, mAnchors, GetCollider(0), GetCollider(1));

  /*Mat3 basis1 = GetCollider(0)->GetWorldRotation();
  Mat3 basis2 = GetCollider(1)->GetWorldRotation();
  DrawHinge(this, mAnchors, basis1, basis2, mAxes[0], mAxes[1]);*/

  DrawAxisAtomFragment(mShockAxes, mAnchors, GetCollider(0), GetCollider(1));
}

uint WheelJoint::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = 0;
  if(atomIndex < 3)
    return LinearAxis;
  else if(atomIndex < 6)
    return AngularAxis;
  return 0;
}

void WheelJoint::BatchEvents()
{
  BatchEventsFragment(this, sInfo.mAtomCount);
}

uint WheelJoint::GetDefaultLimitIds() const
{
  return SingleAngularAxis;
}

uint WheelJoint::GetDefaultMotorIds() const
{
  return SingleAngularAxis;
}

uint WheelJoint::GetDefaultSpringIds() const
{
  return SingleLinearAxis;
}

Vec3 WheelJoint::GetShockAxis() const
{
  return mShockAxes[0];
}

void WheelJoint::SetShockAxis(Vec3Param axis)
{
  mShockAxes[0] = axis;
}

Vec3 WheelJoint::GetWorldShockAxis() const
{
  Collider* collider = GetCollider(0);
  if(collider == nullptr)
    return Vec3::cZero;

  return JointHelpers::BodyToWorldR(collider, mShockAxes.mBodyAxes[0]);
}

void WheelJoint::SetWorldShockAxis(Vec3Param axis)
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
