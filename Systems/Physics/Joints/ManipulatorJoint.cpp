///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

/*
  Let i correspond to the x,y,z axes
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

JointInfo ManipulatorJoint::sInfo = JointInfo(3,0);

ImplementJointType(ManipulatorJoint);

ZilchDefineType(ManipulatorJoint, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  ZilchBindGetterSetter(TargetPoint);
  ZilchBindGetterSetter(LocalPoint);
  ZilchBindGetterSetter(WorldPoint);
}

ManipulatorJoint::ManipulatorJoint()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
  mMaxImpulse = real(1000);
}

void ManipulatorJoint::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void ManipulatorJoint::OnAllObjectsCreated(CogInitializer& initializer)
{
  Joint::OnAllObjectsCreated(initializer);

  Collider* obj = GetCollider(0);
  if(obj == nullptr)
    return;

  RigidBody* body = obj->GetActiveBody();
  if(body)
    mMaxImpulse *= body->GetMass();
}

void ManipulatorJoint::ComputeMoleculeData(MoleculeData& moleculeData)
{
  moleculeData.SetUp(&mAnchors, nullptr, this);
  moleculeData.SetLinearIdentity();
  moleculeData.SetAngularIdentity();
}

void ManipulatorJoint::UpdateAtoms()
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData);
}

uint ManipulatorJoint::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void ManipulatorJoint::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData);
}

void ManipulatorJoint::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void ManipulatorJoint::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void ManipulatorJoint::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
  DebugDraw();
}

uint ManipulatorJoint::PositionMoleculeCount() const
{
  return GetPositionMoleculeCount(this);
}

void ManipulatorJoint::ComputePositionMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputePositionMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData);
}

void ManipulatorJoint::DebugDraw()
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
}

uint ManipulatorJoint::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = 0;
  return LinearAxis;
}

Vec3 ManipulatorJoint::GetTargetPoint()
{
  return mAnchors.mBodyR[1];
}

void ManipulatorJoint::SetTargetPoint(Vec3Param target)
{
  mAnchors.mBodyR[1] = target;
}

Vec3 ManipulatorJoint::GetLocalPoint()
{
  return mAnchors.mBodyR[0];
}

void ManipulatorJoint::SetLocalPoint(Vec3Param bodyPoint)
{
  mAnchors.mBodyR[0] = bodyPoint;
}

Vec3 ManipulatorJoint::GetWorldPoint()
{
  Collider* collider = GetCollider(0);
  if(collider == nullptr)
    return mAnchors.mBodyR[0];
  return JointHelpers::BodyRToWorldPoint(GetCollider(0), mAnchors.mBodyR[0]);
}

void ManipulatorJoint::SetWorldPoint(Vec3Param worldPoint)
{
  Collider* collider = GetCollider(0);
  if(collider != nullptr)
    mAnchors.mBodyR[0] = JointHelpers::WorldPointToBodyR(GetCollider(0), worldPoint);
}

}//namespace Physics

}//namespace Zero
