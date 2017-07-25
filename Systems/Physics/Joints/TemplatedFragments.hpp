///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

const uint cMotorId = 0xC0FFEE;

namespace Zero
{

namespace Physics
{

//returns the number of bits set in a uint
inline uint GetNumberOfBitsSet(uint bitFlag)
{
  uint count = 0;
  while(bitFlag != 0)
  {
    bitFlag = (bitFlag - 1) & bitFlag;
    ++count;
  }
  return count;
}

template <typename JointType>
struct DefaultFragmentPolicy
{
  void AxisValue(MoleculeData& data, int atomIndex, JointType* joint)
  {
    real desiredConstraintValue = 0;
    uint filter = joint->GetAtomIndexFilter(atomIndex, desiredConstraintValue);
    uint axisIndex = atomIndex % 3;
    ConstraintAtom& atom = joint->mAtoms[atomIndex];

    //if the filter said this was a linear axis, compute a linear value
    if(filter & JointType::LinearAxis)
      LinearAxisValue(data.mAnchors, data.LinearAxes[axisIndex], atom);
    //otherwise compute an angular value
    else if(filter & JointType::AngularAxis)
    {
      if(joint->mNode->LimitIndexActive(atomIndex))
        LimitedAngularAxisValue(data, axisIndex, atom);
      else
        AngularAxisValue(data.mRefAngle, data.AngularAxes[axisIndex], atom);
    }
  }

  void ModifyErrorWithSlop(JointType* joint, ConstraintAtom& atom)
  {
    real slop = joint->GetSlop();
    //slop reduces the error towards zero, so we have to determine which side of
    //zero we're on and apply to slop to get us to zero (but not past zero)
    if(atom.mError > 0)
      atom.mError = Math::Max(atom.mError - slop, real(0.0));
    else
      atom.mError = Math::Min(atom.mError + slop, real(0.0));
  }

  void ErrorFragment(int atomIndex, JointType* joint, ImpulseLimitAtom& molLimit)
  {
    uint flag = 1 << atomIndex;
    real desiredConstraintValue = 0;
    uint filter = joint->GetAtomIndexFilter(atomIndex, desiredConstraintValue);
    ConstraintAtom& atom = joint->mAtoms[atomIndex];

    //compute the error of this constraint. have to compute the error at this time
    //so that the limit values are known
    bool wasLimited = ComputeError(atom, molLimit, joint->mNode->mLimit, desiredConstraintValue, flag);
    //apply slop to the error
    if(wasLimited)
      ModifyErrorWithSlop(joint, atom);
  }

  //returns baumgarte
  real AxisFragment(MoleculeData& data, int atomIndex, JointType* joint, ConstraintMolecule& mol)
  {
    real baumgarte;
    uint axisIndex = atomIndex % 3;
    real desiredConstraintValue = 0;
    uint filter = joint->GetAtomIndexFilter(atomIndex, desiredConstraintValue);

    //compute the linear or angular fragment Jacobian
    if(filter & JointType::LinearAxis)
    {
      LinearAxisFragment(data.mAnchors, data.LinearAxes[axisIndex], mol);
      baumgarte = joint->GetLinearBaumgarte(); 
    }
    else if(filter & JointType::AngularAxis)
    {
      AngularAxisFragment(data.mRefAngle, data.AngularAxes[axisIndex], mol);
      baumgarte = joint->GetAngularBaumgarte();
    }
    else
      ErrorIf(true, "Joint %s of index %d returned an invalid index filter.", joint->GetJointName(), atomIndex);

    return baumgarte;
  }
};

template <typename JointType>
struct DefaultAngularLimitPolicy : public DefaultFragmentPolicy<JointType>
{
  void AxisValue(MoleculeData& data, int atomIndex, JointType* joint)
  {
    real desiredConstraintValue = 0;
    uint filter = joint->GetAtomIndexFilter(atomIndex, desiredConstraintValue);
    uint axisIndex = atomIndex % 3;
    ConstraintAtom& atom = joint->mAtoms[atomIndex];

    //if the filter said this was a linear axis, compute a linear value
    if(filter & JointType::LinearAxis)
      LinearAxisValue(data.mAnchors, data.LinearAxes[axisIndex], atom);
    //otherwise compute an angular value
    else if(filter & JointType::AngularAxis)
    {
      if(joint->mNode->LimitIndexActive(atomIndex))
        LimitedAngularAxisValue(data, axisIndex, atom);
      else
      {
        Vec3 err = Math::Cross(data.mAxes[0], data.mAxes[1]);
        atom.mConstraintValue = 2 * Math::Dot(data.AngularAxes[axisIndex], err);
      }
    }
  }
};

template <typename JointType>
struct DefaultFragmentPolicy2d : public DefaultFragmentPolicy<JointType>
{
  void AxisValue(MoleculeData& data, int atomIndex, JointType* joint)
  {
    real desiredConstraintValue = 0;
    uint filter = joint->GetAtomIndexFilter(atomIndex, desiredConstraintValue);
    uint axisIndex = atomIndex % 2;
    ConstraintAtom& atom = joint->mAtoms[atomIndex];

    //if the filter said this was a linear axis, compute a linear value
    if(filter & JointType::LinearAxis)
      LinearAxisValue(data.mAnchors, data.LinearAxes[axisIndex], atom);
    //otherwise compute an angular value
    else if(filter & JointType::AngularAxis)
    {
      if(joint->mNode->LimitIndexActive(atomIndex))
        LimitedAngularAxisValue(data, axisIndex, atom);
      else
        AngularAxisValue(data.mRefAngle, data.AngularAxes[axisIndex], atom);
    }
  }

  //returns baumgarte
  real AxisFragment(MoleculeData& data, int atomIndex, JointType* joint, ConstraintMolecule& mol)
  {
    real baumgarte;
    uint axisIndex = atomIndex % 2;
    real desiredConstraintValue = 0;
    uint filter = joint->GetAtomIndexFilter(atomIndex, desiredConstraintValue);

    //compute the linear or angular fragment Jacobian
    if(filter & JointType::LinearAxis)
    {
      LinearAxisFragment2d(data.mAnchors, data.LinearAxes[axisIndex], mol);
      baumgarte = joint->GetLinearBaumgarte(); 
    }
    else if(filter & JointType::AngularAxis)
    {
      AngularAxisFragment2d(data.mRefAngle, mol);
      baumgarte = joint->GetAngularBaumgarte();
    }
    else
      ErrorIf(true, "Joint %s of index %d returned an invalid index filter.", joint->GetJointName(), atomIndex);

    return baumgarte;
  }
};

template <typename JointType>
struct DefaultAngularLimitPolicy2d : public DefaultFragmentPolicy2d<JointType>
{
  void AxisValue(MoleculeData& data, int atomIndex, JointType* joint)
  {
    real desiredConstraintValue = 0;
    uint filter = joint->GetAtomIndexFilter(atomIndex, desiredConstraintValue);
    uint axisIndex = atomIndex % 2;
    ConstraintAtom& atom = joint->mAtoms[atomIndex];

    //if the filter said this was a linear axis, compute a linear value
    if(filter & JointType::LinearAxis)
      LinearAxisValue(data.mAnchors, data.LinearAxes[axisIndex], atom);
    //otherwise compute an angular value
    else if(filter & JointType::AngularAxis)
    {
      if(joint->mNode->LimitIndexActive(atomIndex))
        LimitedAngularAxisValue(data, axisIndex, atom);
      else
      {
        Vec3 err = Math::Cross(data.mAxes[0], data.mAxes[1]);
        atom.mConstraintValue = 2 * Math::Dot(data.AngularAxes[axisIndex], err);
      }
    }
  }
};

template <typename JointType, typename PolicyType>
void UpdateAtomsFragment(JointType* joint, uint atomCount, MoleculeData& data, PolicyType policy)
{
  //reset the filter for the constraint to be unaffected by limits
  joint->ResetFilter();

  for(uint i = 0; i < atomCount; ++i)
    policy.AxisValue(data, i, joint);

  if(!joint->GetActive())
  {
    //mark all atoms as not active (clear bits 0 through atomCount - 1)
    joint->mConstraintFilter &= ~((1 << atomCount) - 1);
  }
  else
  {
    //determine which of these constraints are active after limits take affect
    ComputeActiveAtoms(joint->mAtoms, atomCount, joint->mNode->mLimit, joint->mConstraintFilter);
  }
}

template <typename JointType>
void UpdateAtomsFragment(JointType* joint, uint atomCount, MoleculeData& data)
{
  UpdateAtomsFragment(joint, atomCount, data, DefaultFragmentPolicy<JointType>());
}

template <typename JointType>
uint GetMoleculeCount(JointType* joint, uint motorFilter)
{
  //get how many constraints are active after limits
  uint count = GetNumberOfBitsSet(joint->GetActiveFilter());
  //get how many motors are active
  uint motorCount = 0;
  JointMotor* motor = joint->mNode->mMotor;
  if(motor && motor->GetActive())
    motorCount = GetNumberOfBitsSet(joint->mNode->mMotor->mAtomIds & motorFilter);
  //the number of fragments is the sum
  return count + motorCount;
}

template <typename JointType>
uint GetPositionMoleculeCount(JointType* joint)
{
  uint index = 0;
  uint count = 0;

  uint filter = joint->GetActiveFilter();
  uint mask = 1;

  //count how many bits are set where the axis is not a spring
  while(mask <= filter)
  {
    if((mask & filter) != 0)
    {
      if(joint->mNode->SpringIndexActive(index) == false)
        ++count;
    }
      
    ++index;
    mask = mask << 1;
  }

  //the number of fragments is the sum
  return count;
}

template <typename JointType, typename PolicyType>
void ComputeMoleculesFragment(JointType* joint, MoleculeWalker& mols, uint atomCount, MoleculeData& data, PolicyType policy)
{
  //cache off the mass terms
  JointMass masses;
  JointHelpers::GetMasses(joint->GetCollider(0), joint->GetCollider(1), masses);
  ImpulseLimitAtom jointLimit(joint->mMaxImpulse);
  real baumgarte = real(4.5);
  real dt = joint->mSpace->mIterationDt;

  bool useBaumgarte = joint->GetShouldBaumgarteBeUsed(JointType::mJointType);

  for(uint i = 0; i < atomCount; ++i)
  {
    uint flag = 1 << i;
    ConstraintMolecule mol;
    mol.mAtomIndex = i;
    //copy the limit values so we can change them but keep the original max bounds
    ImpulseLimitAtom molLimit = jointLimit;

    //get the error for this atom
    policy.ErrorFragment(i, joint, molLimit);

    //get the axis terms for our mol such as the jacobian 
    baumgarte = policy.AxisFragment(data, i, joint, mol);

    //compute the leftover mass, impulse, and limit values
    ComputeMoleculeFragment(molLimit, joint->mAtoms[i], mol, masses);

    //if the motor is active on this atom, add in the motor fragment
    JointMotor* motor = joint->mNode->mMotor;
    if(joint->mNode->MotorIndexActive(i))
    {
      ConstraintMolecule motorMol;
      MotorFragment(mol, motorMol, *motor);
      motorMol.mAtomIndex = cMotorId;
      motorMol.mImpulse = motor->mImpulse;

      *mols = motorMol;
      ++mols;
    }

    //make the fragment rigid or soft (do after the motor so soft constraints
    //don't make the motor behave odd)
    JointSpring* spring = joint->mNode->mSpring;
    if(joint->mNode->SpringIndexActive(i))
      SoftConstraintFragment(mol, spring->mSpringAtom, baumgarte,dt);
    else
    {
      if(useBaumgarte)
        RigidConstraintFragment(mol,baumgarte);
      else
        RigidConstraintFragment(mol, real(0.0));
    }

    //check to see if the limit is active on this constraint.
    //we have to check the limit after computing everything so that the
    //motor can be active on an inactive constraint
    if((joint->mConstraintFilter & flag) != 0)
    {
      *mols = mol;
      ++mols;
    }
  }
}

template <typename JointType>
void ComputeMoleculesFragment(JointType* joint, MoleculeWalker& mols, uint atomCount, MoleculeData& data)
{
  ComputeMoleculesFragment(joint, mols, atomCount, data, DefaultFragmentPolicy<JointType>());
}


template <typename JointType, typename PolicyType>
void ComputePositionMoleculesFragment(JointType* joint, MoleculeWalker& mols, uint atomCount, MoleculeData& data, PolicyType policy)
{
  //cache off the mass terms
  JointMass masses;
  JointHelpers::GetMasses(joint->GetCollider(0), joint->GetCollider(1), masses);
  ImpulseLimitAtom jointLimit(joint->mMaxImpulse);
  real baumgarte = real(4.5);
  real dt = joint->mSpace->mIterationDt;

  for(uint i = 0; i < atomCount; ++i)
  {
    uint flag = 1 << i;
    ConstraintMolecule mol;
    mol.mAtomIndex = i;

    //don't position correct springs
    JointSpring* spring = joint->mNode->mSpring;
    if(joint->mNode->SpringIndexActive(i))
      continue;

    if((joint->mConstraintFilter & flag) == 0)
      continue;

    //copy the limit values so we can change them but keep the original max bounds
    ImpulseLimitAtom molLimit = jointLimit;

    //get the error for this atom
    policy.ErrorFragment(i, joint, molLimit);

    //get the axis terms for our mol such as the jacobian 
    baumgarte = policy.AxisFragment(data, i, joint, mol);

    //compute the leftover mass, impulse, and limit values
    ComputeMoleculeFragment(molLimit, joint->mAtoms[i], mol, masses);

    RigidConstraintFragment(mol, baumgarte);

    *mols = mol;
    ++mols;
  }
}

template <typename JointType>
void ComputePositionMoleculesFragment(JointType* joint, MoleculeWalker& mols, uint atomCount, MoleculeData& data)
{
  ComputePositionMoleculesFragment(joint, mols, atomCount, data, DefaultFragmentPolicy<JointType>());
}

template <typename JointType>
void WarmStartFragment(JointType* joint, MoleculeWalker& mols, uint molCount)
{
  if(molCount == 0)
    return;

  //cache the velocities and masses
  JointVelocity velocities;
  JointHelpers::GetVelocities(joint->GetCollider(0), joint->GetCollider(1), velocities);
  JointMass masses;
  JointHelpers::GetMasses(joint->GetCollider(0), joint->GetCollider(1), masses);

  //apply each fragment to the velocities
  for(uint i = 0; i < molCount; ++i)
  {
    ConstraintMolecule& mol = mols[i];
    JointHelpers::ApplyConstraintImpulse(masses, velocities, mol.mJacobian, mol.mImpulse);
  }

  mols += molCount;

  //copy the velocities back out to the objects
  JointHelpers::CommitVelocities(joint->GetCollider(0), joint->GetCollider(1), velocities);
}

template <typename JointType>
void SolveFragment(JointType* joint, MoleculeWalker& mols, uint molCount)
{
  if(molCount == 0)
    return;

  //cache the velocities and masses
  JointVelocity velocities;
  JointHelpers::GetVelocities(joint->GetCollider(0), joint->GetCollider(1), velocities);
  JointMass masses;
  JointHelpers::GetMasses(joint->GetCollider(0), joint->GetCollider(1), masses);

  //apply each fragment to the velocities
  for(uint i = 0; i < molCount; ++i)
  {
    ConstraintMolecule& mol = mols[i];
    real lambda = ComputeLambda(mol, velocities);
    JointHelpers::ApplyConstraintImpulse(masses, velocities, mol.mJacobian, lambda);
  }

  mols += molCount;

  //copy the velocities back out to the objects
  JointHelpers::CommitVelocities(joint->GetCollider(0), joint->GetCollider(1), velocities);
}

template <typename JointType>
void CommitFragment(JointType* joint, MoleculeWalker& mols, uint molCount)
{
  for(uint i = 0; i < molCount; ++i)
  {
    ConstraintMolecule& mol = mols[i];
    uint atomIndex = mol.mAtomIndex;
    //right now the last impulse for the motor wins, should I do the correct thing by having
    //an impulse value for each molecule? (JoshD questions)
    if(atomIndex == cMotorId)
    {
      joint->mNode->mMotor->mImpulse = mol.mImpulse;
      continue;
    }

    //save the error and impulse back out to the correct atom (does error need to be copied here?)
    joint->mAtoms[atomIndex].mImpulse = mol.mImpulse;
    joint->mAtoms[atomIndex].mError = mol.mError;
  }

  mols += molCount;
}

inline void SnapJoint(Joint* joint)
{
  joint->GetOwner()->Destroy();
}

inline void SendJointEvent(Joint* joint)
{
  JointHelpers::CreateJointEvent(joint, Events::JointExceedImpulseLimit);
}

template <typename JointType, typename Functor>
void CheckJointEvents(JointType* joint, uint atomCount, Functor func)
{
  for(uint i = 0; i < atomCount; ++i)
  {
    //skip inactive atoms
    uint flag = 1 << i;
    if(!(joint->mConstraintFilter & flag))
      continue;

    real absImpulse = Math::Abs(joint->mAtoms[i].mImpulse);
    real maxImpulse = joint->mMaxImpulse;
    if(absImpulse >= maxImpulse)
    {
      func(joint);
      break;
    }
  }
}

template <typename JointType>
void CheckJointLimitEvents(JointType* joint)
{
  JointLimit* limit = joint->mNode->mLimit;
  //if we don't have an active limit then there's nothing to send
  if(limit == nullptr || limit->GetActive() == false)
    return;

  //Check to see if we just hit the lower limit
  if(limit->GetAtLowerLimit() == true && limit->GetWasAtLowerLimit() == false)
  {
    JointHelpers::CreateJointEvent(joint, Events::JointLowerLimitReached);
    limit->SetWasAtLowerLimit(true);
  }
  //Check to see if we left the lower limit
  else if(limit->GetAtLowerLimit() == false && limit->GetWasAtLowerLimit() == true)
    limit->SetWasAtLowerLimit(false);

  //same now for the upper limit
  if(limit->GetAtUpperLimit() == true && limit->GetWasAtUpperLimit() == false)
  {
    JointHelpers::CreateJointEvent(joint, Events::JointUpperLimitReached);
    limit->SetWasAtUpperLimit(true);
  }
  else if(limit->GetAtUpperLimit() == false && limit->GetWasAtUpperLimit() == true)
    limit->SetWasAtUpperLimit(false);
}

template <typename JointType>
void BatchEventsFragment(JointType* joint, uint atomCount)
{
  if(joint->GetSendsEvents())
    CheckJointEvents(joint, atomCount, SendJointEvent);

  if(joint->GetAutoSnaps())
    CheckJointEvents(joint, atomCount, SnapJoint);

  CheckJointLimitEvents(joint);
}

//Debug draw an anchor fragment (Colors are fixed)
inline void DrawAnchorAtomFragment(const AnchorAtom& localAnchors, Collider* obj0, Collider* obj1)
{
  //get the world anchor values
  WorldAnchorAtom anchors(localAnchors, obj0, obj1);

  //get the object positions
  Vec3 obj0Pos = obj0->GetWorldTranslation();
  Vec3 obj1Pos = obj1->GetWorldTranslation();

  //draw lines from each object's center to its respective anchor
  gDebugDraw->Add(Debug::Line(obj0Pos, anchors.mWorldPoints[0]).Color(Color::White));
  gDebugDraw->Add(Debug::Line(obj1Pos, anchors.mWorldPoints[1]).Color(Color::Black));
  //draw a line between the anchors
  gDebugDraw->Add(Debug::Line(anchors.mWorldPoints[0], anchors.mWorldPoints[1]).Color(Color::Gray));
}

//Debug draw an axis fragment at the passed in anchor (Colors fixed)
inline void DrawAxisAtomFragment(const AxisAtom& localAxes, const AnchorAtom& localAnchors, Collider* obj0, Collider* obj1)
{
  //get the world anchor and axis values
  WorldAxisAtom axes(localAxes, obj0, obj1);
  WorldAnchorAtom worldAnchors(localAnchors, obj0, obj1);

  //get the object positions
  Vec3 obj0Pos = worldAnchors.mWorldPoints[0];
  Vec3 obj1Pos = worldAnchors.mWorldPoints[1];

  //draw each axis at the object's anchor position
  gDebugDraw->Add(Debug::Line(obj0Pos, obj0Pos + axes.mWorldAxes[0]).Color(Color::Red));
  gDebugDraw->Add(Debug::Line(obj1Pos, obj1Pos + axes.mWorldAxes[1]).Color(Color::Blue));
}

//Debug draw a the bases of a matrix (Color fixed)
inline void DrawBasisFragment(Collider* obj, Mat3Param objRot)
{
  Vec3 objPos = obj->GetWorldTranslation();

  //draw the bases at the object's position
  gDebugDraw->Add(Debug::Line(objPos, objPos + objRot.GetBasis(0)).Color(Color::Red));
  gDebugDraw->Add(Debug::Line(objPos, objPos + objRot.GetBasis(1)).Color(Color::Green));
  gDebugDraw->Add(Debug::Line(objPos, objPos + objRot.GetBasis(2)).Color(Color::Blue));
}

//Debug draw an angle fragment (Color fixed)
inline void DrawAngleAtomFragment(const AngleAtom& localAngle, Collider* obj0, Collider* obj1)
{
  //get each object's current rotation
  Quat obj0CurrRot = Math::ToQuaternion(obj0->GetWorldRotation());
  Quat obj1CurrRot = Math::ToQuaternion(obj1->GetWorldRotation());

  //bring the local rotations into world space
  Mat3 obj0Rot = Math::ToMatrix3(obj0CurrRot * localAngle.mLocalAngles[0]);
  Mat3 obj1Rot = Math::ToMatrix3(obj1CurrRot * localAngle.mLocalAngles[1]);

  //draw the bases of each angle
  DrawBasisFragment(obj0, obj0Rot);
  DrawBasisFragment(obj1, obj1Rot);
}

//Debug draw an object's basis (Color fixed)
inline void DrawBasisFragment(Collider* obj)
{
  Vec3 objPos = obj->GetWorldTranslation();
  Mat3 objRot = obj->GetWorldRotation();

  gDebugDraw->Add(Debug::Line(objPos, objPos + objRot.GetBasis(0)).Color(Color::Red));
  gDebugDraw->Add(Debug::Line(objPos, objPos + objRot.GetBasis(1)).Color(Color::Green));
  gDebugDraw->Add(Debug::Line(objPos, objPos + objRot.GetBasis(2)).Color(Color::Blue));
}

}//namespace Physics

}//namespace Zero
