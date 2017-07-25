///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Physics
{

JointInfo RelativeVelocityJoint::sInfo = JointInfo(3,0);

/// The linearAxisJoint's policy for Atom updating as well as Molecule computing.
struct RelativeVelocityPolicy : public DefaultFragmentPolicy<RelativeVelocityJoint>
{
  void AxisValue(MoleculeData& data, int atomIndex, RelativeVelocityJoint* joint)
  {
    ErrorIf(atomIndex >= (int)joint->sInfo.mAtomCount, "RelativeVelocityJoint has only %d atom. Cannot compute atom number %d.",
           joint->sInfo.mAtomCount,atomIndex);

    joint->mAtoms[atomIndex].mError = -joint->mSpeeds[atomIndex];
  }

  void ErrorFragment(int atomIndex, RelativeVelocityJoint* joint, ImpulseLimitAtom& molLimit)
  {
    ErrorIf(atomIndex >= (int)joint->sInfo.mAtomCount, "RelativeVelocityJoint has only %d atom. Cannot compute atom number %d.",
            joint->sInfo.mAtomCount, atomIndex);

    molLimit = ImpulseLimitAtom(joint->mMaxImpulses[atomIndex]);
    uint flag = 1 << atomIndex;
    ConstraintAtom& atom = joint->mAtoms[atomIndex];

    // Compute the error of this constraint. have to compute the error at this time
    // so that the limit values are known
    ComputeError(atom, molLimit, joint->mNode->mLimit, 0, flag);
    ModifyErrorWithSlop(joint, atom);
  }

  // Returns baumgarte
  real AxisFragment(MoleculeData& data, int atomIndex, RelativeVelocityJoint* joint, ConstraintMolecule& mol)
  {
    ErrorIf(atomIndex >= (int)joint->sInfo.mAtomCount, "RelativeVelocityJoint has only %d atom. Cannot compute atom number %d.",
            joint->sInfo.mAtomCount, atomIndex);

    // Compute the linear axis fragment Jacobian
    Vec3 axis = data.LinearAxes[atomIndex];
    // There's no easy way to disable 1 atom within a joint, just make it do nothing by
    // setting the jacobian to the zero axis (thus computing impulses that are 0)
    if(joint->GetAxisActive(atomIndex) == false)
      axis = Vec3::cZero;
    mol.mJacobian.Set(-axis, Vec3::cZero, axis, Vec3::cZero);

    return joint->GetLinearBaumgarte();
  }
};

ImplementJointType(RelativeVelocityJoint);

ZilchDefineType(RelativeVelocityJoint, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  ZilchBindMethod(GetAxis);
  ZilchBindMethod(SetAxis);
  ZilchBindMethod(GetSpeed);
  ZilchBindMethod(SetSpeed);
  ZilchBindMethod(GetMaxImpulse);
  ZilchBindMethod(SetMaxImpulse);
  ZilchBindMethod(GetAxisActive);
  ZilchBindMethod(SetAxisActive);
}

RelativeVelocityJoint::RelativeVelocityJoint()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
}

void RelativeVelocityJoint::Serialize(Serializer& stream)
{
  Joint::Serialize(stream);

  stream.SerializeFieldDefault("Axis1", mAxes[0], Vec3::cXAxis);
  stream.SerializeFieldDefault("Axis2", mAxes[1], Vec3::cYAxis);
  stream.SerializeFieldDefault("Axis3", mAxes[2], Vec3::cZAxis);
  SerializeNameDefault(mSpeeds, Vec3::cZero);
  SerializeNameDefault(mMaxImpulses, Vec3(10,0,10));
  SerializeBits(stream, mActiveFlags, RelativeVelocityJointActiveAxes::Names);
}

void RelativeVelocityJoint::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void RelativeVelocityJoint::ComputeMoleculeData(MoleculeData& moleculeData)
{
  moleculeData.SetUp(nullptr, nullptr, this);
  moleculeData.SetAngularIdentity();
  moleculeData.LinearAxes[0] = mAxes[0];
  moleculeData.LinearAxes[1] = mAxes[1];
  moleculeData.LinearAxes[2] = mAxes[2];
}

void RelativeVelocityJoint::UpdateAtoms()
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData, RelativeVelocityPolicy());
}

uint RelativeVelocityJoint::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void RelativeVelocityJoint::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, RelativeVelocityPolicy());
}

void RelativeVelocityJoint::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void RelativeVelocityJoint::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void RelativeVelocityJoint::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint RelativeVelocityJoint::PositionMoleculeCount() const
{
  return 0;
}

void RelativeVelocityJoint::ComputePositionMolecules(MoleculeWalker& molecules)
{
  // Position correction doesn't exist for a velocity constraint
}

void RelativeVelocityJoint::DebugDraw()
{

}

uint RelativeVelocityJoint::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = 0;
  return LinearAxis;
}

void RelativeVelocityJoint::BatchEvents()
{
  BatchEventsFragment(this, sInfo.mAtomCount);
}

uint RelativeVelocityJoint::GetDefaultLimitIds() const
{
  return AllLinearAxes;
}

uint RelativeVelocityJoint::GetDefaultMotorIds() const
{
  return AllLinearAxes;
}

uint RelativeVelocityJoint::GetDefaultSpringIds() const
{
  return AllLinearAxes;
}

Vec3 RelativeVelocityJoint::GetAxis(uint index)
{
  return mAxes[index];
}

void RelativeVelocityJoint::SetAxis(uint index, Vec3Param axis)
{
  mAxes[index] = axis;
}

real RelativeVelocityJoint::GetSpeed(uint index)
{
  return mSpeeds[index];
}

void RelativeVelocityJoint::SetSpeed(uint index, real speed)
{
  mSpeeds[index] = speed;
}

real RelativeVelocityJoint::GetMaxImpulse(uint index)
{
  return mMaxImpulses[index];
}

void RelativeVelocityJoint::SetMaxImpulse(uint index, real maxImpulse)
{
  mMaxImpulses[index] = maxImpulse;
}

bool RelativeVelocityJoint::GetAxisActive(uint index)
{
  return mActiveFlags.IsSet(1 << index);
}

void RelativeVelocityJoint::SetAxisActive(uint index, bool active)
{
  mActiveFlags.SetState(1 << index,active);
}

}//namespace Physics

}//namespace Zero
