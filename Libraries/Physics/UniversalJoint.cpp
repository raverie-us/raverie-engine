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

/*
Linear Constraint:
  Let i correspond to the x, y, z axes.
  Ci   : dot(p2 - p1,i) = 0
  Ci   : dot(c2 + r2 - c1 - r1,i) = 0
  cDoti: dot(v2,i) + dot(cross(w2,r2),i) - dot(v1,i) - dot(cross(w1,r1),i) = 0
  cDoti: dot(v2,i) + dot(w2,cross(r2,i)) - dot(v1,i) - dot(w1,cross(r1,i)) = 0
  Ji   : [-i, -cross(r1,i), i, cross(r2,i)]

  Angular Constraint:
  Let i correspond the 3 basis vectors formed from an object's angle atom
  brought to world space. Also let iA refer to axis i on object A and iB
  refer to axis i on object B.
  where i = 2 is the hinge axis and i of 0 and 1 form a basis for determining
  angle errors.
  We start with the velocity constraint instead of the position constraint
  which ignores some 2nd order terms but works well enough for now.
  The rough idea is thetaB - thetaA = 0
 
  cDoti: dot(w2,iA) - dot(w1,iB) = 0
  Ji   : [0, -iA, 0, iB]


  normally i = 0 and i = 1 are not solved. The are only solved when limits are present
  (not currently solved because I haven't done limits with this yet)
*/

/// The GearJoint's policy for Atom updating as well as Molecule computing.
struct ConeTwistPolicy : public DefaultFragmentPolicy<UniversalJoint>
{
  void ErrorFragment(int atomIndex, UniversalJoint* joint, ImpulseLimitAtom& molLimit)
  {
    uint flag = 1 << atomIndex;
    real desiredConstraintValue = 0;
    uint filter = joint->GetAtomIndexFilter(atomIndex, desiredConstraintValue);
    ConstraintAtom& atom = joint->mAtoms[atomIndex];

    // Compute the error of this constraint. Have to compute the error at this time
    // so that the limit values are known
    if(filter & UniversalJoint::LinearAxis)
      ComputeError(atom, molLimit,joint->mNode->mLimit, desiredConstraintValue, flag);
    else
      atom.mConstraintValue = real(0.0);

    ModifyErrorWithSlop(joint, atom);
  }

  // Returns baumgarte
  real AxisFragment(MoleculeData& data, int atomIndex, UniversalJoint* joint, ConstraintMolecule& mol)
  {
    real baumgarte;
    uint axisIndex = atomIndex % 3;
    real desiredConstraintValue = 0;
    uint filter = joint->GetAtomIndexFilter(atomIndex, desiredConstraintValue);

    // Compute the linear or angular fragment Jacobian
    if(filter & UniversalJoint::LinearAxis)
    {
      LinearAxisFragment(data.mAnchors, data.LinearAxes[axisIndex],mol);
      baumgarte = joint->GetLinearBaumgarte(); 
    }
    else if(filter & UniversalJoint::AngularAxis)
    {
      baumgarte = joint->GetAngularBaumgarte(); 
      // Compute the linear axis fragment Jacobian
      Mat3 rot0 = joint->GetCollider(0)->GetWorldRotation();
      Mat3 rot1 = joint->GetCollider(1)->GetWorldRotation();

      // Bring object 1's frame to world space
      Vec3 worldB0A0 = Math::Transform(rot0, joint->mBody0Axis0);
      Vec3 worldB0A1 = Math::Transform(rot0, joint->mBody0Axis1);
      Vec3 body0Axis = Math::Cross(worldB0A0, worldB0A1).AttemptNormalized();

      // Bring object 2's frame to world space
      Vec3 worldB1A0 = Math::Transform(rot1, joint->mBody1Axis0);
      Vec3 worldB1A1 = Math::Transform(rot1, joint->mBody1Axis1);
      Vec3 body1Axis = Math::Cross(worldB1A0, worldB1A1).AttemptNormalized();

      // Build the jacobian
      mol.mJacobian.Linear[0].ZeroOut();
      mol.mJacobian.Angular[0] = -body0Axis;
      mol.mJacobian.Linear[1].ZeroOut();
      mol.mJacobian.Angular[1] = body1Axis;
    }

    return baumgarte;
  }
};

JointInfo UniversalJoint::sInfo = JointInfo(6, 0x18);

ImplementJointType(UniversalJoint);
ImplementAnchorAccessors(UniversalJoint, mAnchors);

ZilchDefineType(UniversalJoint, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();

  BindAnchorAccessors(Vec3(1));
  ZilchBindGetterSetterProperty(LocalAxis0OfBodyA);
  ZilchBindGetterSetterProperty(LocalAxis1OfBodyA);
  ZilchBindGetterSetterProperty(LocalAxis0OfBodyB);
  ZilchBindGetterSetterProperty(LocalAxis1OfBodyB);
}

UniversalJoint::UniversalJoint()
{
  mConstraintFilter = sInfo.mOnAtomMask << DefaultOffset;
}

void UniversalJoint::Serialize(Serializer& stream)
{
  Joint::Serialize(stream);

  SerializeAnchors(stream, mAnchors, Vec3(1));

  stream.SerializeFieldDefault("LocalAxis0OfBodyA", mBody0Axis0, Vec3(1, 0, 0));
  stream.SerializeFieldDefault("LocalAxis1OfBodyA", mBody0Axis1, Vec3(0, 1, 0));
  stream.SerializeFieldDefault("LocalAxis0OfBodyB", mBody1Axis0, Vec3(1, 0, 0));
  stream.SerializeFieldDefault("LocalAxis1OfBodyB", mBody1Axis1, Vec3(0, 1, 0));
}

void UniversalJoint::Initialize(CogInitializer& initializer)
{
  Joint::Initialize(initializer);
}

void UniversalJoint::ComputeInitialConfiguration()
{
  // First just grab the body points from the object link
  ComputeCurrentAnchors(mAnchors);

  // Next compute the axis of the prismatic as the axis between the two points
  Vec3 p0 = GetWorldPointA();
  Vec3 p1 = GetWorldPointB();
  Vec3 axis = p1 - p0;
  real length = axis.AttemptNormalize();
  // If we got an invalid axis then just use the y axis...
  if(length == real(0.0))
    axis = Vec3::cYAxis;

  Vec3 worldAxis0, worldAxis1;
  Math::GenerateOrthonormalBasis(axis, &worldAxis0, &worldAxis1);

  Collider* collider0 = GetCollider(0);
  if(collider0 != nullptr)
  {
    mBody0Axis0 = Physics::JointHelpers::WorldToBodyR(collider0, worldAxis0);
    mBody0Axis1 = Physics::JointHelpers::WorldToBodyR(collider0, worldAxis1);
  }
  Collider* collider1 = GetCollider(1);
  if(collider1 != nullptr)
  {
    mBody1Axis0 = Physics::JointHelpers::WorldToBodyR(collider1, worldAxis0);
    mBody1Axis1 = Physics::JointHelpers::WorldToBodyR(collider1, worldAxis1);
  }
}

void UniversalJoint::ComputeMoleculeData(MoleculeData& moleculeData)
{
  moleculeData.SetUp(&mAnchors, nullptr, nullptr, this);
  moleculeData.SetLinearIdentity();
  moleculeData.SetAngularIdentity();
}

void UniversalJoint::UpdateAtoms()
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);
  
  UpdateAtomsFragment(this, sInfo.mAtomCount, moleculeData, ConeTwistPolicy());
}

uint UniversalJoint::MoleculeCount() const
{
  return GetMoleculeCount(this, sInfo.mAtomCountMask);
}

void UniversalJoint::ComputeMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputeMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, ConeTwistPolicy());
}

void UniversalJoint::WarmStart(MoleculeWalker& molecules)
{
  WarmStartFragment(this, molecules, MoleculeCount());
}

void UniversalJoint::Solve(MoleculeWalker& molecules)
{
  SolveFragment(this, molecules, MoleculeCount());
}

void UniversalJoint::Commit(MoleculeWalker& molecules)
{
  CommitFragment(this, molecules, MoleculeCount());
}

uint UniversalJoint::PositionMoleculeCount() const
{
  return GetPositionMoleculeCount(this);
}

void UniversalJoint::ComputePositionMolecules(MoleculeWalker& molecules)
{
  MoleculeData moleculeData;
  ComputeMoleculeData(moleculeData);

  ComputePositionMoleculesFragment(this, molecules, sInfo.mAtomCount, moleculeData, ConeTwistPolicy());
}

void UniversalJoint::DebugDraw()
{
  if(!GetValid())
    return;

  WorldAnchorAtom anchors(mAnchors, GetCollider(0), GetCollider(1));
  Mat3 rot0 = GetCollider(0)->GetWorldRotation();
  Mat3 rot1 = GetCollider(1)->GetWorldRotation();

  Vec3 worldB0A0 = Math::Transform(rot0, mBody0Axis0);
  Vec3 worldB0A1 = Math::Transform(rot0, mBody0Axis1);
  Vec3 body0Axis = Math::Cross(worldB0A0, worldB0A1).AttemptNormalized();
  Vec3 point0 = anchors.mWorldPoints[0];
  gDebugDraw->Add(Debug::Line(point0, point0 + body0Axis));
  gDebugDraw->Add(Debug::Line(point0, point0 + worldB0A0).Color(Color::Red));
  gDebugDraw->Add(Debug::Line(point0, point0 + worldB0A1).Color(Color::Blue));

  Vec3 worldB1A0 = Math::Transform(rot1, mBody1Axis0);
  Vec3 worldB1A1 = Math::Transform(rot1, mBody1Axis1);
  Vec3 body1Axis = Math::Cross(worldB1A0, worldB1A1).AttemptNormalized();
  Vec3 point1 = anchors.mWorldPoints[1];
  gDebugDraw->Add(Debug::Line(point1, point1 + body1Axis));
  gDebugDraw->Add(Debug::Line(point1, point1 + worldB1A0).Color(Color::Red));
  gDebugDraw->Add(Debug::Line(point1, point1 + worldB1A1).Color(Color::Blue));
}

uint UniversalJoint::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  desiredConstraintValue = 0;
  if(atomIndex < 3)
    return LinearAxis;
  else if(atomIndex < 6)
    return AngularAxis;
  return 0;
}

void UniversalJoint::BatchEvents()
{
  BatchEventsFragment(this, sInfo.mAtomCount);
}

uint UniversalJoint::GetDefaultLimitIds() const
{
  return SingleAngularAxis;
}

uint UniversalJoint::GetDefaultMotorIds() const
{
  return SingleAngularAxis;
}

uint UniversalJoint::GetDefaultSpringIds() const
{
  return SingleAngularAxis;
}

Vec3 UniversalJoint::GetLocalAxis0OfBodyA()
{
  return mBody0Axis0;
}

void UniversalJoint::SetLocalAxis0OfBodyA(Vec3Param axis)
{
  mBody0Axis0 = axis;
}

Vec3 UniversalJoint::GetLocalAxis1OfBodyA()
{
  return mBody0Axis1;
}

void UniversalJoint::SetLocalAxis1OfBodyA(Vec3Param axis)
{
  mBody0Axis1 = axis;
}

Vec3 UniversalJoint::GetLocalAxis0OfBodyB()
{
  return mBody1Axis0;
}

void UniversalJoint::SetLocalAxis0OfBodyB(Vec3Param axis)
{
  mBody1Axis0 = axis;
}

Vec3 UniversalJoint::GetLocalAxis1OfBodyB()
{
  return mBody1Axis1;
}

void UniversalJoint::SetLocalAxis1OfBodyB(Vec3Param axis)
{
  mBody1Axis1 = axis;
}

}//namespace Physics

}//namespace Zero
