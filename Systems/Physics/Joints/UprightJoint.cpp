///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Physics
{

JointInfo UprightJoint::sInfo = JointInfo(3, 0x4);

/// The UprightJoint's policy for Atom updating as well as Molecule computing.
struct UprightPolicy : public DefaultFragmentPolicy<UprightJoint>
{
  void AxisValue(MoleculeData& data, int atomIndex, UprightJoint* joint)
  {
    ErrorIf(atomIndex >= (int)joint->sInfo.mAtomCount, "UprightJoint has only %d atom. Cannot compute atom number %d.",
            joint->sInfo.mAtomCount, atomIndex);

    uint axisIndex = atomIndex % 3;
    ConstraintAtom& atom = joint->mAtoms[atomIndex];

    // This error calculation has a fail case when the two axes are exactly
    // 180 degrees error. If the two vectors are basically in that case, just
    // set the error to pi and it'll auto correct out of this state in a random direction.
    if(Math::Dot(data.mAxes[0],data.mAxes[1]) < real(-.999))
    {
      atom.mConstraintValue = Math::cPi;
      return;
    }

    // Otherwise, use that fact that cross(u,v) ~= sin(theta) and use a small angle
    // approximation to say sin(theta) ~= theta. Take that error and project it onto
    // the correct axis to figure out the error. It's not exactly right, but it produces reasonable behavior.
    Vec3 axisError = Math::Cross(data.mAxes[0], data.mAxes[1]);
    atom.mConstraintValue = Math::Dot(axisError, data.AngularAxes[axisIndex]);
  }

  // Returns baumgarte
  real AxisFragment(MoleculeData& data, int atomIndex, UprightJoint* joint, ConstraintMolecule& mol)
  {
    ErrorIf(atomIndex >= (int)joint->sInfo.mAtomCount, "UprightJoint has only %d atom. Cannot compute atom number %d.",
            joint->sInfo.mAtomCount, atomIndex);

    uint axisIndex = atomIndex % 3;
    AngularAxisFragment(data.mRefAngle, data.AngularAxes[axisIndex], mol);

    return joint->GetAngularBaumgarte();
  }
};

ImplementJointType(UprightJoint);
ImplementAxisAccessors(UprightJoint, mAxes);

ZilchDefineType(UprightJoint, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();
  BindAxisAccessors(Vec3::cYAxis);
}

UprightJoint::UprightJoint()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
}

void UprightJoint::Serialize(Serializer& stream)
{
  Joint::Serialize(stream);
  SerializeAxes(stream, mAxes, Vec3::cYAxis);
}

void UprightJoint::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void UprightJoint::ComputeInitialConfiguration()
{
  SetWorldAxis(Vec3::cYAxis);
}

void UprightJoint::ComputeMoleculeData(MoleculeData& moleculeData)
{
  moleculeData.SetUp(nullptr, nullptr, &mAxes, this);
  moleculeData.SetAngularBasis(moleculeData.mAxes[0]);
}

void UprightJoint::UpdateAtoms()
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);
  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData, UprightPolicy());
}

uint UprightJoint::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void UprightJoint::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);
  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, UprightPolicy());
}

void UprightJoint::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void UprightJoint::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void UprightJoint::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint UprightJoint::PositionMoleculeCount() const
{
  return GetPositionMoleculeCount(this);
}

void UprightJoint::ComputePositionMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);
  ComputePositionMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, UprightPolicy());
}

void UprightJoint::DebugDraw()
{  
  Collider* collider0 = GetCollider(0);
  Collider* collider1 = GetCollider(1);

  // We must have two valid colliders
  if(collider0 == nullptr || collider1 == nullptr)
    return;

  WorldAxisAtom worldAxes(mAxes, collider0, collider1);

  Vec3 pos0 = collider0->GetWorldTranslation();
  gDebugDraw->Add(Debug::Line(pos0, pos0 + worldAxes[0]).Color(Color::Red));
  
  Vec3 pos1 = collider1->GetWorldTranslation();
  gDebugDraw->Add(Debug::Line(pos1, pos1 + worldAxes[1]).Color(Color::Blue));
}

uint UprightJoint::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  return AngularAxis;
}

void UprightJoint::BatchEvents()
{
  BatchEventsFragment(this, sInfo.mAtomCount);
}

uint UprightJoint::GetDefaultLimitIds() const
{
  return SingleAxis;
}

uint UprightJoint::GetDefaultMotorIds() const
{
  return 0;
}

uint UprightJoint::GetDefaultSpringIds() const
{
  return SingleAxis;
}

}//namespace Physics

}//namespace Zero
