///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

// Straight out of box2d...work on my own full derivation later...
// Pulley:
// length1 = norm(p1 - s1)
// length2 = norm(p2 - s2)
// C0 = (length1 + ratio * length2)_initial
// C = C0 - (length1 + ratio * length2)
// u1 = (p1 - s1) / norm(p1 - s1)
// u2 = (p2 - s2) / norm(p2 - s2)
// Cdot = -dot(u1, v1 + cross(w1, r1)) - ratio * dot(u2, v2 + cross(w2, r2))
// J = -[u1 cross(r1, u1) ratio * u2  ratio * cross(r2, u2)]
// K = J * invM * JT
//   = invMass1 + invI1 * cross(r1, u1)^2 + ratio^2 * (invMass2 + invI2 * cross(r2, u2)^2)

namespace Zero
{
  
namespace Physics
{

JointInfo PulleyJoint::sInfo = JointInfo(1, 0);

/// The PulleyJoint's policy for Atom updating as well as Molecule computing.
struct PulleyPolicy : public DefaultFragmentPolicy<PulleyJoint>
{
  void AxisValue(MoleculeData& data, int atomIndex, PulleyJoint* joint)
  {
    ErrorIf(atomIndex >= 1, "PulleyJoint only has one atom. Cannot compute atom number %d.", atomIndex);
    PulleyAxisValue(joint, joint->mAtoms[atomIndex], 0);
  }

  // Returns baumgarte
  real AxisFragment(MoleculeData& data, int atomIndex, PulleyJoint* joint, ConstraintMolecule& mol)
  {
    ErrorIf(atomIndex >= 1, "PulleyJoint only has one atom. Cannot compute atom number %d.", atomIndex);
    PulleyAxisFragment(joint, mol);

    return joint->GetLinearBaumgarte();
  }
};

ImplementJointType(PulleyJoint);

ZilchDefineType(PulleyJoint, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(Ratio);

  ZilchBindGetterSetterProperty(JointAPath);
  ZilchBindGetterSetterProperty(JointBPath);
  ZilchBindGetterSetter(JointA);
  ZilchBindGetterSetter(JointB);
}

PulleyJoint::PulleyJoint()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
}

void PulleyJoint::Serialize(Serializer& stream)
{
  Joint::Serialize(stream);

  SerializeNameDefault(mRatio, real(1.0));

  stream.SerializeFieldDefault("JointAPath", mJoints[0].mCogPath, CogPath());
  stream.SerializeFieldDefault("JointBPath", mJoints[1].mCogPath, CogPath());
}

void PulleyJoint::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void PulleyJoint::OnAllObjectsCreated(CogInitializer& initializer)
{
  Joint::OnAllObjectsCreated(initializer);

  bool dynamicallyCreated = (initializer.Flags & CreationFlags::DynamicallyAdded) != 0;

  if(!dynamicallyCreated)
  {
    mJoints[0].mCogPath.RestoreLink(initializer, this, "JointAPath");
    mJoints[1].mCogPath.RestoreLink(initializer, this, "JointBPath");

    for(uint i = 0; i < 2; ++i)
    {
      // If the id we recovered is valid, retrieve the joint
      Cog* cog = mJoints[i].mCogPath.GetCog();
      if(cog == nullptr)
      {
        SetValid(false);
        continue;
      }

      mJoints[i].mJoint = cog->has(StickJoint);
      if(mJoints[i].mJoint == nullptr)
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

  // All of our joints are now valid. However, we don't know if we are connected
  // to obj0 or obj1 on each joint. This is needed for the proper solving
  // of the jacobian.
  for(uint i = 0; i < 2; ++i)
  {
    if(GetCollider(i) == mJoints[i].mJoint->GetCollider(0))
      mJoints[i].mObjId = 0;
    else
      mJoints[i].mObjId = 1;
  }

  // Don't do any final active or valid checks if we're in the editor because we don't
  // need to modify further state on ourself and we definitely don't
  // want to modify the other joints.
  if(GetSpace()->IsEditorMode())
    return;

  // If either joint was invalid, then we are also invalid
  if(mJoints[0].mJoint->GetValid() == false || mJoints[1].mJoint->GetValid() == false)
  {
    SetValid(false);
    return;
  }

  // Pulley only works on "inactive" stick joints. The stick joint serves as
  // defining the pulley, but if the stick is on then the pulley would have to
  // overpower the stick in order to work.
  mJoints[0].mJoint->SetActive(false);
  mJoints[1].mJoint->SetActive(false);
}

void PulleyJoint::OnDestroy(uint flags)
{
  Joint::OnDestroy(flags);

  // If there are any stick joints still alive then set
  // them to active so that they resume solving
  for(uint i = 0; i < 2; ++i)
  {
    Cog* cog = mJoints[i].mCogPath.GetCog();
    if(cog == nullptr)
      continue;

    StickJoint* joint = cog->has(StickJoint);
    if(joint != nullptr)
      joint->SetActive(true);
  }
}

void PulleyJoint::ComputeMoleculeData(MoleculeData& moleculeData)
{
  moleculeData.SetUp(nullptr, nullptr, this);
  moleculeData.SetAngularIdentity();
  moleculeData.SetLinearIdentity();
}

void PulleyJoint::UpdateAtoms()
{
  ValidateJoints();

  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData, PulleyPolicy());
}

uint PulleyJoint::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void PulleyJoint::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, PulleyPolicy());
}

void PulleyJoint::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void PulleyJoint::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void PulleyJoint::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint PulleyJoint::PositionMoleculeCount() const
{
  return GetPositionMoleculeCount(this);
}

void PulleyJoint::ComputePositionMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputePositionMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, PulleyPolicy());
}

void PulleyJoint::DebugDraw()
{
  if(!GetValid())
    return;
  // Draw a line between the center of the two bound objects
  Vec3 obj0Pos = GetCollider(0)->GetWorldTranslation();
  Vec3 obj1Pos = GetCollider(1)->GetWorldTranslation();

  gDebugDraw->Add(Debug::Line(obj0Pos, obj1Pos).Color(Color::Black));
}

uint PulleyJoint::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = 0;
  return LinearAxis;
}

void PulleyJoint::BatchEvents()
{
  BatchEventsFragment(this, sInfo.mAtomCount);
}

uint PulleyJoint::GetDefaultLimitIds() const
{
  return SingleAxis;
}

uint PulleyJoint::GetDefaultMotorIds() const
{
  return SingleAxis;
}

uint PulleyJoint::GetDefaultSpringIds() const
{
  return SingleAxis;
}

real PulleyJoint::GetRatio() const
{
  return mRatio;
}

void PulleyJoint::SetRatio(real ratio)
{
  mRatio = ratio;
  JointHelpers::ForceAwakeJoint(this);
}

CogPath PulleyJoint::GetJointAPath()
{
  return mJoints[0].mCogPath;
}

void PulleyJoint::SetJointAPath(CogPath& cogPath)
{
  mJoints[0].mCogPath = cogPath;
}

CogPath PulleyJoint::GetJointBPath()
{
  return mJoints[1].mCogPath;
}

void PulleyJoint::SetJointBPath(CogPath& cogPath)
{
  mJoints[1].mCogPath = cogPath;
}

Cog* PulleyJoint::GetJointA()
{
  return mJoints[0].mCogPath.GetCog();
}

void PulleyJoint::SetJointA(Cog* cog)
{
  RelinkJoint(0, cog);
}

Cog* PulleyJoint::GetJointB()
{
  return mJoints[1].mCogPath.GetCog();
}

void PulleyJoint::SetJointB(Cog* cog)
{
  RelinkJoint(1, cog);
}

void PulleyJoint::SpecificJointRelink(uint index, Collider* collider)
{
  bool success = FindAndSetJoint(collider, index);
  if(!success)
    SetValid(false);
}

void PulleyJoint::RelinkJoint(uint index, Cog* cog)
{
  if(cog == nullptr)
  {
    DoNotifyWarning("Invalid link", "Joint must be linked to an object.");
    return;
  }

  StickJoint* stick = cog->has(StickJoint);
  if(stick == nullptr)
  {
    DoNotifyWarning("Invalid link", "Cannot link to an object that doesn't have a StickJoint.");
    return;
  }

  // Make sure that the joint we're linking to is connected to the collider we're connected to
  if(stick->GetCollider(0) == GetCollider(index))
    mJoints[index].mObjId = 0;
  else if(stick->GetCollider(1) == GetCollider(index))
    mJoints[index].mObjId = 1;
  else
  {
    DoNotifyWarning("Invalid link", "Can only connect to StickJoints that are attached to the same collider as the PulleyJoint.");
    return;
  }

  mJoints[index].mCogPath.SetCog(cog);
}

bool PulleyJoint::FindAndSetJoint(Collider* collider, uint index)
{
  StickJointRange sticks = FilterJointRange<StickJoint>(collider);
  if(sticks.Empty())
    return false;

  StickJoint* joint = &sticks.GetConstraint();
  mJoints[index].mJoint = joint;
  mJoints[index].mCogPath.SetCog(joint->GetOwner());
  return true;
}

void PulleyJoint::ValidateJoints()
{
  Cog* jointCogs[2];
  jointCogs[0] = mJoints[0].mCogPath.GetCog();
  jointCogs[1] = mJoints[1].mCogPath.GetCog();
  // If either joint is destroyed or the actual joint component
  // is removed then we need to become invalidated
  for(uint i = 0; i < 2; ++i)
  {
    if(jointCogs[i] == nullptr)
    {
      mJoints[i].mJoint = nullptr;
      SetValid(false);
    }
    else
    {
      StickJoint* joint = jointCogs[i]->has(StickJoint);
      if(joint == nullptr)
      {
        mJoints[i].mJoint = nullptr;
        SetValid(false);
      }
    }
  }

  // If we became invalidate for whatever reason,
  // then we need to turn any valid joints on
  if(GetValid() == false)
  {
    for(uint i = 0; i < 2; ++i)
    {
      if(mJoints[i].mJoint != nullptr)
        mJoints[i].mJoint->SetActive(true);
    }
  }
}

}//namespace Physics

}//namespace Zero
