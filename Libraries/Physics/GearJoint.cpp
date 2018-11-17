///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

/*
  c_init: (coordinate1 + ratio * coordinate2)_init = 0
  C     : c_init - (coordinate1 + ratio * coordinate2) = 0
  cDot  : cDot1 - ratio * cDot2 = 0
  J     : [J1, -ratio * J2]

  if a connection is a revolute:
  C   : angle of the motor axis
  cDot: angular velocity about that axis
  J   : [0, axis]

  if a connection is a prismatic:
  C   : dot(axis, p2 - p1)
  cDot: dot(axis, v2 - v1)
  J   : [axis, 0]
  (we remove the torque because it is taken out by
  the prismatic, so we don't want to add it back in)
*/

namespace Zero
{
  
namespace Physics
{

JointInfo GearJoint::sInfo = JointInfo(1, 0);

///The GearJoint's policy for Atom updating as well as Molecule computing.
struct GearPolicy : public DefaultFragmentPolicy<GearJoint>
{
  void AxisValue(MoleculeData& data, int atomIndex, GearJoint* joint)
  {
    ErrorIf(atomIndex >= 1, "LinearAxisJoint only has one atom. Cannot compute atom number %d.", atomIndex);
    GearAxisValue(joint, joint->mAtoms[atomIndex], 0);
  }

  // Returns baumgarte
  real AxisFragment(MoleculeData& data, int atomIndex, GearJoint* joint, ConstraintMolecule& mol)
  {
    ErrorIf(atomIndex >= 1, "LinearAxisJoint only has one atom. Cannot compute atom number %d.", atomIndex);
    GearAxisFragment(joint, mol);

    return joint->GetLinearBaumgarte();
  }
};

ImplementJointType(GearJoint);

ZilchDefineType(GearJoint, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(Ratio);
  ZilchBindGetterSetterProperty(Constant);

  ZilchBindGetterSetterProperty(JointAPath);
  ZilchBindGetterSetterProperty(JointBPath);
  ZilchBindGetterSetter(JointA);
  ZilchBindGetterSetter(JointB);
}

GearJoint::GearJoint()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
}

void GearJoint::Serialize(Serializer& stream)
{
  Joint::Serialize(stream);

  SerializeNameDefault(mRatio, real(1.0));
  SerializeNameDefault(mConstant, real(1.0));

  stream.SerializeFieldDefault("JointAPath", mJoints[0].mCogPath, CogPath());
  stream.SerializeFieldDefault("JointBPath", mJoints[1].mCogPath, CogPath());
}

void GearJoint::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void GearJoint::OnAllObjectsCreated(CogInitializer& initializer)
{
  Joint::OnAllObjectsCreated(initializer);

  bool dynamicallyCreated = (initializer.Flags & CreationFlags::DynamicallyAdded) != 0;
  if(!dynamicallyCreated)
  {
    mJoints[0].mCogPath.RestoreLink(initializer, this, "JointAPath");
    mJoints[1].mCogPath.RestoreLink(initializer, this, "JointBPath");
    for(uint i = 0; i < 2; ++i)
    {
      if(ValidateJoint(i) == false)
        SetValid(false);
    }
  }
  else
  {
    // We were dynamically added, find joints on the objects we were connected to
    for(uint i = 0; i < 2; ++i)
    {
      Collider* collider = GetCollider(i);
      if(collider == nullptr)
      {
        SetValid(false);
        continue;
      }

      bool success = FindAndSetJoint(collider, i);
      if(success == false)
        SetValid(false);
    }
  }

  if(GetActive() == false || GetValid() == false)
    return;

  // Make sure both joints get OnAllObjectsCreated called on them
  // (object link and joint deals with a double call)
  for(uint i = 0; i < 2; ++i)
  {
    Cog* cog = mJoints[i].mJoint->GetOwner();
    ObjectLink* link = cog->has(ObjectLink);
    Joint* joint = mJoints[i].mJoint;
    link->OnAllObjectsCreated(initializer);
    joint->OnAllObjectsCreated(initializer);
  }

  // All of our joints are now valid, however, we don't know if we are connected
  // to obj0 or obj1 on each joint. This is needed for the proper solving
  // of the jacobian.
  for(uint i = 0; i < 2; ++i)
  {
    if(GetCollider(i) == mJoints[i].mJoint->GetCollider(0))
      mJoints[i].mObjIndex = 0;
    else
      mJoints[i].mObjIndex = 1;
  }

  // If either joint was invalid, then we are also invalid
  if(mJoints[0].mJoint->GetValid() == false || mJoints[1].mJoint->GetValid() == false)
  {
    SetValid(false);
    return;
  }
}

void GearJoint::ComputeMoleculeData(MoleculeData& moleculeData)
{
  moleculeData.SetUp(nullptr, nullptr, this);
  moleculeData.SetAngularIdentity();
  moleculeData.SetLinearIdentity();
}

void GearJoint::UpdateAtoms()
{
  ValidateJoints();

  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData, GearPolicy());
}

uint GearJoint::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void GearJoint::ComputeMolecules(MoleculeWalker& molecules)
{
  if(!GetActive())
    return;

  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, GearPolicy());
}

void GearJoint::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void GearJoint::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void GearJoint::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint GearJoint::PositionMoleculeCount() const
{
  // Implement later!
  return 0;
}

void GearJoint::ComputePositionMolecules(MoleculeWalker& molecules)
{
  if(!GetActive())
    return;
}

void GearJoint::DebugDraw()
{
  if(!GetValid())
    return;

  // Draw a line between the center of the two bound objects
  Vec3 obj0Pos = GetCollider(0)->GetWorldTranslation();
  Vec3 obj1Pos = GetCollider(1)->GetWorldTranslation();

  gDebugDraw->Add(Debug::Line(obj0Pos, obj1Pos).Color(Color::Black));
}

uint GearJoint::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = 0;
  return LinearAxis;
}

void GearJoint::BatchEvents()
{
  BatchEventsFragment(this, sInfo.mAtomCount);
}

uint GearJoint::GetDefaultLimitIds() const
{
  return SingleAxis;
}

uint GearJoint::GetDefaultMotorIds() const
{
  return SingleAxis;
}

uint GearJoint::GetDefaultSpringIds() const
{
  return SingleAxis;
}

real GearJoint::GetRatio() const
{
  return mRatio;
}

void GearJoint::SetRatio(real ratio)
{
  mRatio = ratio;
  JointHelpers::ForceAwakeJoint(this);
}

real GearJoint::GetConstant() const
{
  return mConstant;
}

void GearJoint::SetConstant(real constant)
{
  mConstant = constant;
  JointHelpers::ForceAwakeJoint(this);
}

CogPath GearJoint::GetJointAPath()
{
  return mJoints[0].mCogPath;
}

void GearJoint::SetJointAPath(CogPath& cogPath)
{
  mJoints[0].mCogPath = cogPath;
}

CogPath GearJoint::GetJointBPath()
{
  return mJoints[1].mCogPath;
}

void GearJoint::SetJointBPath(CogPath& cogPath)
{
  mJoints[1].mCogPath = cogPath;
}

Cog* GearJoint::GetJointA()
{
  return mJoints[0].mCogPath.GetCog();
}

void GearJoint::SetJointA(Cog* cog)
{
  RelinkJoint(0, cog);
}

Cog* GearJoint::GetJointB()
{
  return mJoints[1].mCogPath.GetCog();
}

void GearJoint::SetJointB(Cog* cog)
{
  RelinkJoint(1, cog);
}

void GearJoint::SpecificJointRelink(uint index, Collider* collider)
{
  bool success = FindAndSetJoint(collider, index);
  if(!success)
    SetValid(false);
}

void GearJoint::RelinkJoint(uint index, Cog* cog)
{
  if(cog == nullptr)
  {
    DoNotifyWarning("Invalid link", "Joint must be linked to an object.");
    return;
  }

  Joint* joint = GetValidJointOnCog(cog);
  if(joint == nullptr)
  {
    DoNotifyWarning("Invalid link", "Cannot link to an object that doesn't have a "
                    "RevoluteJoint, RevoluteJoint2d, PrismaticJoint, or PrismaticJoint2d.");
    return;
  }

  // Make sure that the joint we're linking to is connected to the collider we're connected to
  if(joint->GetCollider(0) == GetCollider(index))
    mJoints[index].mObjIndex = 0;
  else if(joint->GetCollider(1) == GetCollider(index))
    mJoints[index].mObjIndex = 1;
  else
  {
    DoNotifyWarning("Invalid link", "Can only connect to Joints that are attached to the same collider as the GearJoint.");
    return;
  }

  mJoints[index].mCogPath.SetCog(cog);
}

bool GearJoint::FindAndSetJoint(Collider* collider, uint index)
{
  RevoluteJointRange revolutes = FilterJointRange<RevoluteJoint>(collider);
  RevoluteJoint2dRange revolutes2d = FilterJointRange<RevoluteJoint2d>(collider);
  PrismaticJointRange prismatics = FilterJointRange<PrismaticJoint>(collider);
  PrismaticJoint2dRange prismatics2d = FilterJointRange<PrismaticJoint2d>(collider);

  // Try to find a joint type we can connect to
  // (just choose an arbitrary order of which joint type we pick first)
  Joint* joint = nullptr;
  if(!revolutes.Empty())
    joint = &revolutes.GetConstraint();
  else if(!revolutes2d.Empty())
    joint = &revolutes2d.GetConstraint();
  else if(!prismatics.Empty())
    joint = &prismatics.GetConstraint();
  else if(!prismatics2d.Empty())
    joint = &prismatics2d.GetConstraint();
  
  if(joint == nullptr)
    return false;
  
  if(OperationQueue::IsListeningForSideEffects())
  {
    if(index == 0)
      OperationQueue::RegisterSideEffect(this, "JointAPath", mJoints[index].mCogPath);
    else
      OperationQueue::RegisterSideEffect(this, "JointBPath", mJoints[index].mCogPath);
  }
  mJoints[index].mJoint = joint;
  mJoints[index].mCogPath.SetCog(joint->GetOwner());
  return true;
}

bool GearJoint::ValidateJoint(uint index)
{
  Cog* cog = mJoints[index].mCogPath.GetCog();
  if(cog == nullptr)
  {
    mJoints[index].mJoint = nullptr;
    return false;
  }

  Joint* joint = GetValidJointOnCog(cog);

  if(joint == nullptr)
  {
    mJoints[index].mJoint = nullptr;
    return false;
  }

  // Determine what kind of joint we got
  mJoints[index].mJoint = joint;
  if(joint->GetJointType() == RevoluteJoint::StaticGetJointType())
    mJoints[index].mBoundType = RevJoint;
  else if(joint->GetJointType() == RevoluteJoint2d::StaticGetJointType())
    mJoints[index].mBoundType = RevJoint2d;
  else if(joint->GetJointType() == PrismaticJoint::StaticGetJointType())
    mJoints[index].mBoundType = PrismJoint;
  else if(joint->GetJointType() == PrismaticJoint2d::StaticGetJointType())
    mJoints[index].mBoundType = PrismJoint2d;
  return true;
}

void GearJoint::ValidateJoints()
{
  bool joint0Valid = ValidateJoint(0);
  bool joint1Valid = ValidateJoint(1);
  if(joint0Valid == false || joint1Valid == false)
    SetValid(false);
}

Joint* GearJoint::GetValidJointOnCog(Cog* cog)
{
  Joint* joint = cog->has(RevoluteJoint);
  if(joint != nullptr)
    return joint;
  joint = cog->has(RevoluteJoint2d);
  if(joint != nullptr)
    return joint;
  joint = cog->has(PrismaticJoint);
  if(joint != nullptr)
    return joint;
  joint = cog->has(PrismaticJoint2d);
  if(joint != nullptr)
    return joint;

  return nullptr;
}

}//namespace Physics

}//namespace Zero
