///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

/*
  Linear Constraint:
  Let i correspond to the x, y, z axes
  Ci   : dot(p2 - p1,i) = 0
  Ci   : dot(c2 + r2 - c1 - r1,i) = 0
  cDoti: dot(v2,i) + dot(cross(w2,r2),i) - dot(v1,i) - dot(cross(w1,r1),i) = 0
  cDoti: dot(v2,i) + dot(w2,cross(r2,i)) - dot(v1,i) - dot(w1,cross(r1,i)) = 0
  Ji   : [-i, -cross(r1,i), i, cross(r2,i)]

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

JointInfo PhyGunJoint::sInfo = JointInfo(6,0);

ImplementJointType(PhyGunJoint);

ZilchDefineType(PhyGunJoint, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  ZilchBindGetterSetter(TargetPoint);
  ZilchBindGetterSetter(LocalPoint);
  ZilchBindGetterSetter(WorldPoint);
  ZilchBindGetterSetter(TargetRotation);
  ZilchBindGetterSetter(WorldRotation);
}

PhyGunJoint::PhyGunJoint()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
  mMaxImpulse = real(1000);
}

void PhyGunJoint::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void PhyGunJoint::ComputeMoleculeData(MoleculeData& moleculeData)
{
  moleculeData.SetUp(&mAnchors, &mReferenceAngle, this);
  moleculeData.SetLinearIdentity();
  moleculeData.SetAngularIdentity();
}

void PhyGunJoint::UpdateAtoms()
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData);
}

uint PhyGunJoint::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void PhyGunJoint::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData);
}

void PhyGunJoint::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void PhyGunJoint::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void PhyGunJoint::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint PhyGunJoint::PositionMoleculeCount() const
{
  return GetPositionMoleculeCount(this);
}

void PhyGunJoint::ComputePositionMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputePositionMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData);
}

void PhyGunJoint::DebugDraw()
{
  if(!GetValid())
    return;

  // Get the world anchor values
  Collider* obj0 = GetCollider(0);
  WorldAnchorAtom anchors(mAnchors, obj0, GetCollider(1));

  // Get the object position
  Vec3 obj0Pos = obj0->GetWorldTranslation();

  // Draw lines from each object's center to its respective anchor
  gDebugDraw->Add(Debug::Line(obj0Pos, anchors.mWorldPoints[0]).Color(Color::White));
  // Draw a line between the anchors
  gDebugDraw->Add(Debug::Line(anchors.mWorldPoints[0], anchors.mWorldPoints[1]).Color(Color::Gray));

  DrawAngleAtomFragment(mReferenceAngle, GetCollider(0),GetCollider(1));

  // Get each object's current rotation
  Quat obj0CurrRot = Math::ToQuaternion(obj0->GetWorldRotation());

  // Bring the local rotations into world space
  Mat3 obj0Rot = Math::ToMatrix3(obj0CurrRot * mReferenceAngle.mLocalAngles[0]);

  // Draw the bases of each angle
  DrawBasisFragment(obj0, obj0Rot);
}

uint PhyGunJoint::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = 0;
  if(atomIndex < 3)
    return LinearAxis;
  else if(atomIndex < 6)
    return AngularAxis;
  return 0;
}

Vec3 PhyGunJoint::GetTargetPoint()
{
  return mAnchors.mBodyR[1];
}

void PhyGunJoint::SetTargetPoint(Vec3Param target)
{
  mAnchors.mBodyR[1] = target;
}

Vec3 PhyGunJoint::GetLocalPoint()
{
  return mAnchors.mBodyR[0];
}

void PhyGunJoint::SetLocalPoint(Vec3Param bodyPoint)
{
  mAnchors.mBodyR[0] = bodyPoint;
}

Vec3 PhyGunJoint::GetWorldPoint()
{
  Collider* collider0 = GetCollider(0);
  if(collider0 == nullptr)
    return Vec3::cZero;
  return JointHelpers::BodyRToWorldPoint(collider0, mAnchors.mBodyR[0]);
}

void PhyGunJoint::SetWorldPoint(Vec3Param worldPoint)
{
  Collider* collider0 = GetCollider(0);
  if(collider0 != nullptr)
    mAnchors.mBodyR[0] = JointHelpers::WorldPointToBodyR(collider0, worldPoint);
}

Quat PhyGunJoint::GetTargetRotation()
{
  return mReferenceAngle[1];
}

void PhyGunJoint::SetTargetRotation(QuatParam target)
{
  mReferenceAngle[1] = target;
}

Quat PhyGunJoint::GetWorldRotation()
{
  return mReferenceAngle[0].Conjugated();
}

void PhyGunJoint::SetWorldRotation(QuatParam worldRotation)
{
  mReferenceAngle[0] = worldRotation.Conjugated();
}

}//namespace Physics

}//namespace Zero
