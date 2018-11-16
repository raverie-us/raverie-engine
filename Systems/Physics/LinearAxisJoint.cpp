///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Physics
{

JointInfo LinearAxisJoint::sInfo = JointInfo(1, 0);

/// The linearAxisJoint's policy for Atom updating as well as Molecule computing.
struct LinearAxisPolicy : public DefaultFragmentPolicy<LinearAxisJoint>
{
  void AxisValue(MoleculeData& data, int atomIndex, LinearAxisJoint* joint)
  {
    ErrorIf(atomIndex >= 1, "LinearAxisJoint only has one atom. Cannot compute atom number %d.", atomIndex);
    joint->mAtoms[atomIndex].mError = real(0.0);
  }

  void ErrorFragment(int atomIndex, LinearAxisJoint* joint, ImpulseLimitAtom& molLimit)
  {
    ErrorIf(atomIndex >= 1, "LinearAxisJoint only has one atom. Cannot compute atom number %d.", atomIndex);
    uint flag = 1 << atomIndex;
    ConstraintAtom& atom = joint->mAtoms[atomIndex];

    // Compute the error of this constraint. have to compute the error at this time
    // so that the limit values are known
    ComputeError(atom, molLimit, joint->mNode->mLimit, 0, flag);
    ModifyErrorWithSlop(joint, atom);
  }

  // Returns baumgarte
  real AxisFragment(MoleculeData& data, int atomIndex, LinearAxisJoint* joint, ConstraintMolecule& mol)
  {
    ErrorIf(atomIndex >= 1, "LinearAxisJoint only has one atom. Cannot compute atom number %d.", atomIndex);
    mol.mJacobian.Set(data.LinearAxes[0], Vec3::cZero, Vec3::cZero, Vec3::cZero);

    return joint->GetLinearBaumgarte();
  }
};

ImplementJointType(LinearAxisJoint);

ZilchDefineType(LinearAxisJoint, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(WorldAxis);
}

LinearAxisJoint::LinearAxisJoint()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
}

void LinearAxisJoint::Serialize(Serializer& stream)
{
  Joint::Serialize(stream);

  SerializeNameDefault(mWorldAxis, Vec3::cXAxis);
}

void LinearAxisJoint::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void LinearAxisJoint::OnAllObjectsCreated(CogInitializer& initializer)
{
  Joint::OnAllObjectsCreated(initializer);

  // If we are not dynamically created, we don't need to do some extra logic
  bool dynamicallyCreated = (initializer.Flags & CreationFlags::DynamicallyAdded) != 0;
  if(!dynamicallyCreated)
    return;

  // By default this joint is for dynamic character controller which
  // does not want this joint to be active, just the motor.
  SetActive(false);
  // If we are dynamically created then add a joint motor assuming we
  // don't already have one (shouldn't be possible but oh well)
  JointMotor* motor = GetOwner()->has(JointMotor);
  if(motor == nullptr)
  {
    motor = new JointMotor();
    GetOwner()->AddComponent(motor);
    motor->mMaxImpulse = real(2);
    motor->mSpeed = 5;
  }

  motor->mAtomIds = GetDefaultMotorIds();
}

void LinearAxisJoint::ComputeMoleculeData(MoleculeData& moleculeData)
{
  moleculeData.SetUp(nullptr, nullptr, this);
  moleculeData.SetAngularIdentity();
  moleculeData.LinearAxes[0] = mWorldAxis;
}

void LinearAxisJoint::UpdateAtoms()
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData, LinearAxisPolicy());
}

uint LinearAxisJoint::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void LinearAxisJoint::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, LinearAxisPolicy());
}

void LinearAxisJoint::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void LinearAxisJoint::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void LinearAxisJoint::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint LinearAxisJoint::PositionMoleculeCount() const
{
  return GetPositionMoleculeCount(this);
}

void LinearAxisJoint::ComputePositionMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputePositionMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, LinearAxisPolicy());
}

void LinearAxisJoint::DebugDraw()
{
  if(!GetValid())
    return;
  Vec3 obj1Pos = GetCollider(0)->GetWorldTranslation();
  gDebugDraw->Add(Debug::Line(obj1Pos, obj1Pos + mWorldAxis));
}

uint LinearAxisJoint::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = 0;
  return LinearAxis;
}

void LinearAxisJoint::BatchEvents()
{
  BatchEventsFragment(this, sInfo.mAtomCount);
}

uint LinearAxisJoint::GetDefaultLimitIds() const
{
  return SingleAxis;
}

uint LinearAxisJoint::GetDefaultMotorIds() const
{
  return SingleAxis;
}

uint LinearAxisJoint::GetDefaultSpringIds() const
{
  return SingleAxis;
}

Vec3 LinearAxisJoint::GetWorldAxis()
{
  return mWorldAxis;
}

void LinearAxisJoint::SetWorldAxis(Vec3Param axis)
{
  mWorldAxis = axis;
}

}//namespace Physics

}//namespace Zero
