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

void LinearAxisValue(WorldAnchorAtom& anchor, Vec3Param axis, ConstraintAtom& atom, real constant)
{
  atom.mConstraintValue = Math::Dot(anchor.GetPointDifference(), axis) - constant;
}

void AngularAxisValue(WorldAngleAtom& angle, Vec3Param axis, ConstraintAtom& atom, real constant)
{
  // Old way with quaternions (2-1 mapping issue)
  // atom.mConstraintValue = real(2.0) * Math::Dot(angle.mWorldReferenceAngle.V3(),axis) - constant;
  atom.mConstraintValue = Math::Dot(angle.mEulerAngles, axis);
}

void LimitedAngularAxisValue(MoleculeData& data, uint axis, ConstraintAtom& atom)
{
  uint indexX = (axis + 1) % 3;
  uint indexY = (axis + 2) % 3;
  Vec3 basisX = data.mWorldBases[0].GetBasis(indexX);
  Vec3 basisY = data.mWorldBases[0].GetBasis(indexY);
  Vec3 testAxis = data.mWorldBases[1].GetBasis(indexX);

  real xProj = Math::Dot(testAxis, basisX);
  real yProj = Math::Dot(testAxis, basisY);
  atom.mConstraintValue = Math::ArcTan2(yProj, xProj);
}

void GearAxisValue(GearJoint* joint, ConstraintAtom& atom, real constant)
{
  if(!joint->GetActive() || !joint->GetValid())
    return;

  real coordinates[2];
  for(uint i = 0; i < 2; ++i)
  {
    if(joint->mJoints[i].mBoundType == GearJoint::RevJoint)
      coordinates[i] = joint->mJoints[i].mRevolute->GetJointAngle();
    else if(joint->mJoints[i].mBoundType == GearJoint::RevJoint2d)
      coordinates[i] = joint->mJoints[i].mRevolute2d->GetJointAngle();
    else if(joint->mJoints[i].mBoundType == GearJoint::PrismJoint)
      coordinates[i] = joint->mJoints[i].mPrismatic->GetJointTranslation();
    else if(joint->mJoints[i].mBoundType == GearJoint::PrismJoint2d)
      coordinates[i] = joint->mJoints[i].mPrismatic2d->GetJointTranslation();
  }
  joint->mAtoms[0].mConstraintValue = coordinates[0] + coordinates[1] * joint->mRatio;
  //temporary until i can get correct error values (JoshD questions)
  joint->mAtoms[0].mConstraintValue = real(0.0);
}

void PulleyAxisValue(PulleyJoint* joint, ConstraintAtom& atom, real constant)
{
  if(!joint->GetActive() || !joint->GetValid())
    return;

  StickJoint* joint0 = joint->mJoints[0].mJoint;
  StickJoint* joint1 = joint->mJoints[1].mJoint;
  WorldAnchorAtom anchors0(joint0->mAnchors, joint0);
  WorldAnchorAtom anchors1(joint1->mAnchors, joint1);
  real ratio = joint->mRatio;
  uint objId0 = joint->mJoints[0].mObjId;
  uint objId1 = joint->mJoints[1].mObjId;

  Vec3 u0 = anchors0.mWorldPoints[objId0] - anchors0.mWorldPoints[(objId0 + 1) % 2];
  Vec3 u1 = anchors1.mWorldPoints[objId1] - anchors1.mWorldPoints[(objId1 + 1) % 2];

  real length0 = u0.Length();
  real length1 = u1.Length();
  real length0Init = joint->mJoints[0].mJoint->GetLength();
  real length1Init = joint->mJoints[1].mJoint->GetLength();
  real c0 = length0Init + length1Init * joint->mRatio;
  real c = length0 + length1 * joint->mRatio;

  joint->mAtoms[0].mConstraintValue = c0 - c;
}

bool ComputeError(ConstraintAtom& atom, ImpulseLimitAtom& molLimit, JointLimit* limit, real constant, uint flag)
{
  //if there is no limit or the limit is not active on this atom, the error is just
  //the value minus the constant passed in
  if(limit == nullptr || limit->GetActive() == false || (limit->mAtomIds & flag) == 0)
  {
    atom.mError = atom.mConstraintValue - constant;
    return false;
  }

  //otherwise, check to see if the value is beyond the limit values and change the
  //error and clamp bounds accordingly
  if(atom.mConstraintValue < limit->mMinErr)
  {
    atom.mError = atom.mConstraintValue - limit->mMinErr;
    molLimit.mMinImpulse = real(0.0);
    return true;
  }
  else if(atom.mConstraintValue > limit->mMaxErr)
  {
    atom.mError = atom.mConstraintValue - limit->mMaxErr;
    molLimit.mMaxImpulse = real(0.0);
    return true;
  }
  else
    atom.mError = real(0.0);

  return false;
}

void ComputeActiveAtoms(ConstraintAtom* atoms, uint count, JointLimit* limit, uint& bitFlag)
{
  //if there is no limit to apply, return
  if(limit == nullptr || limit->GetActive() == false )
    return;

  for(uint i = 0; i < count; ++i)
  {
    uint flag = 1 << i;
    //if the limit is not acting on the atom, continue
    if((limit->mAtomIds & flag) == 0)
      continue;

    //check if the upper or lower limit is hit
    if(atoms[i].mError < limit->mMinErr)
    {
      bitFlag |= flag;
      limit->SetAtLowerLimit(true);
    }
    else if(atoms[i].mError > limit->mMaxErr)
    {
      bitFlag |= flag;
      limit->SetAtUpperLimit(true);
    }
    //otherwise this atom is not active
    else
    {
      bitFlag &= ~flag;
      limit->SetAtLowerLimit(false);
      limit->SetAtUpperLimit(false);
    }
  }
}

void LinearAxisFragment(WorldAnchorAtom& anchor, Vec3Param axis, ConstraintMolecule& mol)
{
  mol.mJacobian.Set(-axis, -Math::Cross(anchor[0], axis),
                     axis,  Math::Cross(anchor[1], axis));
}

void LinearAxisFragment2d(WorldAnchorAtom& anchor, Vec3Param axis, ConstraintMolecule& mol)
{
  mol.mJacobian.Set(-axis, -Math::Cross2d(anchor[0], axis),
                     axis,  Math::Cross2d(anchor[1], axis));
  /*Vec3 d = anchor[1] - anchor[0];
  Vec3 c = real(.5) * Cross2d(axis, d);
  mol.mJacobian.Set(-axis, -Math::Cross2d(anchor[0], axis) + c,
                     axis,  Math::Cross2d(anchor[1], axis) + c);*/
}

void PrismaticAxisFragment2d(WorldAnchorAtom& anchor, Vec3Param axis, ConstraintMolecule& mol)
{
  //Vec3 d = anchor.mWorldPoints[1] - anchor.mWorldPoints[0];
  //This d makes takes into account the rotational energy of the axis. However,
  //this axis makes the jacobian non-symmetric which also makes the primary
  //object 1 (so object 1 has more control over the pair, and should be heavier)
  //mol.mJacobian.Set(-axis, Math::Cross2d(d + anchor[0], axis),
  //                   axis, Math::Cross2d(anchor[1], axis));

  //This is the symmetric jacobian for a prismatic that depends upon w1 == w2.
  //This will work for the prismatic, but not for the wheel.
  Vec3 c = anchor.mWorldR[1] - anchor.mWorldR[0];
  mol.mJacobian.Set(-axis, real(.5) * Math::Cross(axis,c),
                     axis, real(.5) * Math::Cross(axis,c));
}

void AngularAxisFragment(WorldAngleAtom& angle, Vec3Param axis, ConstraintMolecule& mol)
{
  mol.mJacobian.Set(Vec3::cZero, -axis, Vec3::cZero, axis);
}

void AngularAxisFragment2d(WorldAngleAtom& angle, ConstraintMolecule& mol)
{
  mol.mJacobian.Set(Vec3::cZero, -Vec3::cZAxis, Vec3::cZero, Vec3::cZAxis);
}

void GearAxisFragment(GearJoint* joint, ConstraintMolecule& mol)
{
  //clean up later (deal with joints being active but
  //need values computed for other things) (JoshD Questions)
  if(!joint->GetActive() || !joint->GetValid())
    return;

  for(uint i = 0; i < 2; ++i)
  {
    if(joint->mJoints[i].mBoundType == GearJoint::RevJoint)
    {
      mol.mJacobian.Linear[i] = Vec3::cZero;
      mol.mJacobian.Angular[i] = joint->mJoints[i].mRevolute->GetWorldAxis();
    }
    else if(joint->mJoints[i].mBoundType == GearJoint::RevJoint2d)
    {
      mol.mJacobian.Linear[i] = Vec3::cZero;
      mol.mJacobian.Angular[i] = joint->mJoints[i].mRevolute2d->GetWorldAxis();
    }
    else if(joint->mJoints[i].mBoundType == GearJoint::PrismJoint)
    {
      mol.mJacobian.Linear[i] = joint->mJoints[i].mPrismatic->GetWorldAxis();
      mol.mJacobian.Angular[i] = Vec3::cZero;
    }
    else if(joint->mJoints[i].mBoundType == GearJoint::PrismJoint2d)
    {
      mol.mJacobian.Linear[i] = joint->mJoints[i].mPrismatic2d->GetWorldAxis();
      mol.mJacobian.Angular[i] = Vec3::cZero;
    }
  }
  mol.mJacobian.Linear[1] *= joint->mRatio;
  mol.mJacobian.Angular[1] *= joint->mRatio;
  mol.mJacobian = -mol.mJacobian;
}

void PulleyAxisFragment(PulleyJoint* joint, ConstraintMolecule& mol)
{
  //clean up later (deal with joints being active but
  //need values computed for other things) (JoshD Questions)
  if(!joint->GetActive() || !joint->GetValid())
    return;

  // u1 = (p1 - s1) / norm(p1 - s1)
  // u2 = (p2 - s2) / norm(p2 - s2)
  //J = -[u1 cross(r1, u1) ratio * u2  ratio * cross(r2, u2)]
  StickJoint* joint0 = joint->mJoints[0].mJoint;
  StickJoint* joint1 = joint->mJoints[1].mJoint;
  WorldAnchorAtom anchors0(joint0->mAnchors, joint0);
  WorldAnchorAtom anchors1(joint1->mAnchors, joint1);
  real ratio = joint->mRatio;
  uint objId0 = joint->mJoints[0].mObjId;
  uint objId1 = joint->mJoints[1].mObjId;

  Vec3 r0 = anchors0.mWorldR[objId0];
  Vec3 r1 = anchors1.mWorldR[objId1];
  
  Vec3 u0 = anchors0.mWorldPoints[objId0] - anchors0.mWorldPoints[(objId0 + 1) % 2];
  Vec3 u1 = anchors1.mWorldPoints[objId1] - anchors1.mWorldPoints[(objId1 + 1) % 2];
  u0.Normalize();
  u1.Normalize();

  mol.mJacobian.Set(-u0, -Math::Cross(r0, u0), -u1 * ratio, -Math::Cross(r1, u1) * ratio);
}

void ComputeMoleculeFragment(ImpulseLimitAtom& limit, ConstraintAtom& atom, ConstraintMolecule& mol, JointMass& masses)
{
  //compute the mass term
  real effectiveMass = mol.mJacobian.ComputeMass(masses);
  if(effectiveMass < Math::PositiveMin())
    mol.mMass = real(0.0);
  else
    mol.mMass = real(1.0) / effectiveMass;
  //copy over the impulse, error, and limits
  mol.mImpulse = atom.mImpulse;
  mol.mError = atom.mError;
  mol.SetLimit(limit.mMinImpulse, limit.mMaxImpulse);
}

void MotorFragment(ConstraintMolecule& constraintMol, ConstraintMolecule& motorMol, JointMotor& motor)
{
  //a motor is just the normal molecule with different limits and bias
  motorMol = constraintMol;
  motorMol.mMinImpulse = -motor.mMaxImpulse;
  motorMol.mMaxImpulse = motor.mMaxImpulse;
  motorMol.mBias = -motor.mSpeed;
  motorMol.mGamma = real(0.0);

  if(motor.GetReverse())
    motorMol.mBias *= real(-1.0);
}

void RigidConstraintFragment(real& molError, real& molBias, real& molGamma, real baumgarte)
{
  molBias = molError * baumgarte;
  molGamma = real(0.0);
}

void RigidConstraintFragment(ConstraintMolecule& mol, real baumgarte)
{
  RigidConstraintFragment(mol.mError, mol.mBias, mol.mGamma, baumgarte);
}

void SoftConstraintFragment(real& molMass, real& molError, real& molBias, real& molGamma, real springFrequencyHz, real springDampRatio, real baumgarte, real dt)
{
  //if the spring has a frequency of 0, that means it is rigid.
  //Also, if the mass is too small then we either have a mass of 0 or a bad jacobian, 
  //just run the rigid constraint fragment which will handle this correctly
  if(springFrequencyHz == real(0.0) || molMass < Math::PositiveMin())
  {
    RigidConstraintFragment(molError, molBias, molGamma, baumgarte);
    return;
  }

  //taken from Erin Catto's GDC 2011 Soft Constraints lecture
  real mass = molMass;
  real invMass = real(1.0) / mass;
  real error = molError;

  real omega = Math::cTwoPi * springFrequencyHz;
  real k = mass * omega * omega;
  real d = real(2.0) * mass * springDampRatio * omega;

  real gamma = dt * (d + dt * k);
  gamma = gamma != real(0.0) ? real(1.0) / gamma : real(0.0);
  //making a constraint soft changes the effective mass
  mass = invMass + gamma;
  mass = mass != real(0.0) ? real(1.0) / mass : real(0.0);

  //compute the final bias, mass, and gamma
  molBias = baumgarte * error * gamma * dt * k;
  molMass = mass;
  molGamma = gamma;
}

void SoftConstraintFragment(ConstraintMolecule& mol, SpringAtom& spring, real baumgarte, real dt)
{
  SoftConstraintFragment(mol.mMass, mol.mError, mol.mBias, mol.mGamma, spring.mFrequencyHz, spring.mDampingRatio, baumgarte, dt);
}

void ContactNormalFragment(ConstraintMolecule& mol, real baumgarte, real restitutionBias)
{
  mol.mBias = mol.mError * baumgarte - restitutionBias;
  mol.mGamma = real(0.0);
}

real ComputeLambda(ConstraintMolecule& mol, JointVelocity& velocites)
{
  //compute JV
  real cDot = mol.mJacobian.ComputeJV(velocites);
  real impulse = mol.mImpulse;
  // add in the bias and gamma
  cDot += mol.mBias;
  cDot += mol.mGamma * impulse;
  //then get the mass weighted lambda
  real lambda = -mol.mMass * cDot;

  //clamp lambda within the limit bounds to get the new impulse
  real oldImpulse = impulse;
  impulse = Math::Clamp(oldImpulse + lambda, mol.mMinImpulse, mol.mMaxImpulse);
  lambda = impulse - oldImpulse;

  //save the accumulated impulse
  mol.mImpulse = impulse;
  //return the lambda to apply
  return lambda;
}

}//namespace Physics

}//namespace Zero
