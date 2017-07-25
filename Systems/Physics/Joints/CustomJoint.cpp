///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(ComputeCustomJointInfo);
}//namespace Events

namespace Physics
{

//-------------------------------------------------------------------CustomConstraintInfo
ZilchDefineType(CustomJointEvent, builder, type)
{
  ZeroBindDocumented();
  ZilchBindFieldProperty(mOwner);
  ZilchBindFieldProperty(mDt);
}

//-------------------------------------------------------------------CustomConstraintInfo
ZilchDefineType(CustomConstraintInfo, builder, type)
{
  ZeroBindDocumented();
  ZilchBindFieldProperty(mLinear0);
  ZilchBindFieldProperty(mAngular0);
  ZilchBindFieldProperty(mLinear1);
  ZilchBindFieldProperty(mAngular1);

  ZilchBindFieldProperty(mEffectiveMass);
  ZilchBindFieldProperty(mGamma);
  ZilchBindFieldProperty(mBias);
  ZilchBindFieldProperty(mMinImpulse);
  ZilchBindFieldProperty(mMaxImpulse);
  ZilchBindFieldProperty(mImpulse);
  ZilchBindFieldProperty(mError);
  ZilchBindFieldProperty(mBaumgarte);
  ZilchBindFieldProperty(mActive);
  ZilchBindFieldProperty(mSolvePosition);

  ZilchBindMethod(SetJacobian);
  ZilchBindMethod(SetErrorAndBias);
  ZilchBindMethod(ComputeMotor);
  ZilchBindMethod(ComputeSpring);
  ZilchBindMethod(DetachFromOwner);
  ZilchBindMethod(IsOwned);
  ZilchBindGetterProperty(Owner);
}

CustomConstraintInfo::CustomConstraintInfo()
{
  mLinear0 = mLinear1 = mAngular0 = mAngular1 = Vec3::cZero;

  mEffectiveMass = 0;
  mGamma = 0;
  mBias = 0;
  mMinImpulse = -Math::PositiveMax();
  mMaxImpulse = Math::PositiveMax();
  mImpulse = 0;
  mError = 0;
  mBaumgarte = 5;
  mOwner = nullptr;
  mActive = true;
  mSolvePosition = false;
}

void CustomConstraintInfo::SetJacobian(Vec3Param linear0, Vec3Param angular0, Vec3Param linear1, Vec3Param angular1)
{
  mLinear0 = linear0;
  mAngular0 = angular0;
  mLinear1 = linear1;
  mAngular1 = angular1;

  // Try to get the owning joint
  CustomJoint* joint = GetOwner();
  if(joint == nullptr)
    return;

  // Get the masses of the colliders
  JointMass masses;
  JointHelpers::GetMasses(joint->GetCollider(0), joint->GetCollider(1), masses);
  
  // Compute the effective mass if the constraint (J^T * M * J)
  Jacobian jacobian;
  jacobian.Set(mLinear0, mAngular0, mLinear1, mAngular1);
  mEffectiveMass = jacobian.ComputeMass(masses);
}

void CustomConstraintInfo::SetErrorAndBias(real error)
{
  mError = error;
  mBias = mError * mBaumgarte;
}

void CustomConstraintInfo::ComputeSpring(float frequencyHz, float dampRatio)
{
  // Default dt to 60 fps
  float dt = 1.0f / 60.0f;
  CustomJoint* joint = mOwner;
  // Get dt from the space if we have it
  if(joint == nullptr)
    dt = joint->mSpace->mIterationDt;

  // Update the mass, bias, and gamma
  SoftConstraintFragment(mEffectiveMass, mError, mBias, mGamma, frequencyHz, dampRatio, mBaumgarte, dt);

  // We can't solve position anymore
  mSolvePosition = false;
}
void CustomConstraintInfo::ComputeMotor(float targetSpeed, float minImpulse, float maxImpulse)
{
  mMinImpulse = minImpulse;
  mMaxImpulse = maxImpulse;
  mBias = -targetSpeed;
  mGamma = real(0.0);

  // We can't solve position anymore
  mSolvePosition = false;
}

void CustomConstraintInfo::Reset()
{
  mGamma = 0.0f;
  mEffectiveMass = 0.0f;
  mBias = 0.0f;
}

void CustomConstraintInfo::DetachFromOwner()
{
  // If we have a joint then remove ourself from it
  CustomJoint* joint = mOwner;
  if(joint == nullptr)
    return;

  joint->RemoveConstraint(this);
  mOwner = nullptr;
}

bool CustomConstraintInfo::IsOwned()
{
  CustomJoint* joint = mOwner;
  return joint != nullptr;
}

CustomJoint* CustomConstraintInfo::GetOwner()
{
  return mOwner;
}

//-------------------------------------------------------------------CustomJoint
ImplementJointType(CustomJoint);
ZilchDefineType(CustomJoint, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Joint);
  ZeroBindDocumented();
  ZeroBindEvent(Events::ComputeCustomJointInfo, CustomJointEvent);
  
  ZilchBindMethod(CreateConstraint);
  ZilchBindMethod(AddConstraint);
  ZilchBindMethod(RemoveConstraint);
  ZilchBindMethod(ClearConstraints);

  ZilchBindGetter(ConstraintCount);
  ZilchBindMethod(GetConstraint);
}

CustomJoint::CustomJoint()
{
}

void CustomJoint::UpdateAtoms()
{
  // Unfortunately, during position correction the constraint info has to be updated each iteration.
  // Physics doesn't normally update the transform until the end of the frame
  // (it updates the internal cached transforms). In this case the old position values will exist
  // and a script won't be able to correctly compute the constraint info. Because of this we must
  // forcefully update each object's transform beforehand. Maybe clean up the interface later to
  // not do this when solving velocity instead of position?
  UpdateTransform(0);
  UpdateTransform(1);

  // Reset all the data on the constraint for the new frame
  for(size_t i = 0; i < mConstraints.Size(); ++i)
  {
    CustomConstraintInfo* constraint = mConstraints[i];
    if(constraint != nullptr)
      constraint->Reset();
  }

  // Send the event to let anyone handle updating constraints
  CustomJointEvent toSend;
  toSend.mOwner = this;
  toSend.mDt = mSpace->mIterationDt;
  GetOwner()->DispatchEvent(Events::ComputeCustomJointInfo, &toSend);
}

uint CustomJoint::MoleculeCount() const
{
  size_t activeCount = 0;
  if(!GetActive())
    return activeCount;

  // Count each active constraint
  for(size_t i = 0; i < mConstraints.Size(); ++i)
  {
    CustomConstraintInfo* constraint = mConstraints[i];
    if(constraint->mActive)
      ++activeCount;
  }
  return activeCount;
}

void CustomJoint::ComputeMolecules(MoleculeWalker& molecules)
{
  if(!GetActive())
    return;

  // Copy over each active constraint
  for(size_t i = 0; i < mConstraints.Size(); ++i)
  {
    CustomConstraintInfo* constraint = mConstraints[i];
    if(!constraint->mActive)
      continue;

    ConstraintMolecule& mol = *molecules;
    ConstraintInfoToMolecule(constraint, mol);

    ++molecules;
  }
}

void CustomJoint::WarmStart(MoleculeWalker& molecules)
{
  if(!GetActive())
    return;

  WarmStartFragment(this, molecules, MoleculeCount());
}

void CustomJoint::Solve(MoleculeWalker& molecules)
{
  if(!GetActive())
    return;

  SolveFragment(this, molecules, MoleculeCount());
}

void CustomJoint::Commit(MoleculeWalker& molecules)
{
  if(!GetActive())
    return;

  // For warm-starting we need to copy back out the total accumulated impulse for each constraint
  for(size_t i = 0; i < mConstraints.Size(); ++i)
  {
    CustomConstraintInfo* constraint = mConstraints[i];
    if(!constraint->mActive)
      continue;

    ConstraintMolecule& mol = *molecules;
    constraint->mImpulse = mol.mImpulse;
    ++molecules;
  }
}

uint CustomJoint::PositionMoleculeCount() const
{
  size_t activeCount = 0;
  // If this joint isn't active then there's nothing to solve
  if(!GetActive())
    return activeCount;

  // Find how many constraints are active that also solve position
  for(size_t i = 0; i < mConstraints.Size(); ++i)
  {
    CustomConstraintInfo* constraint = mConstraints[i];
    if(constraint->mActive && constraint->mSolvePosition)
      ++activeCount;
  }
  return activeCount;
}

void CustomJoint::ComputePositionMolecules(MoleculeWalker& molecules)
{
  // Don't solve if not active
  if(!GetActive())
    return;

  // Copy all active constraints that are set to solve position
  for(size_t i = 0; i < mConstraints.Size(); ++i)
  {
    CustomConstraintInfo* constraint = mConstraints[i];
    if(!constraint->mActive || !constraint->mSolvePosition)
      continue;

    ConstraintMolecule& mol = *molecules;
    ConstraintInfoToMolecule(constraint, mol);

    ++molecules;
  }
}

uint CustomJoint::GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const
{
  // Not used
  return 0;
}

void CustomJoint::BatchEvents()
{
  // Implement later! (have to check for impulse limits, snapping, and max impulse)
}

CustomConstraintInfo* CustomJoint::CreateConstraint()
{
  ConstraintInfoReference& ref = mConstraints.PushBack();
  CustomConstraintInfo* mol = new CustomConstraintInfo();
  mol->mOwner = this;
  ref = mol;
  return mol;
}

void CustomJoint::AddConstraint(CustomConstraintInfo* constraint)
{
  // If this constraint already has an owner then assert
  if(constraint->IsOwned())
  {
    String msg;
    CustomJoint* owner = constraint->mOwner;
    Cog* cogOwner = owner->GetOwner();
    
    String cogDescription = cogOwner->GetDescription();
    msg = String::Format("Constraint already is owned by cog '%s'", cogDescription.c_str());
    DoNotifyException("Invalid Constraint Add", msg);
    return;
  }

  // Otherwise add this to our list of constraints
  ConstraintInfoReference& ref = mConstraints.PushBack();
  ref = constraint;
}

void CustomJoint::RemoveConstraint(CustomConstraintInfo* constraint)
{
  // Find the constraint's index
  HandleOf<CustomConstraintInfo> constraintHandle = constraint;
  Array<ConstraintInfoReference>::size_type index = mConstraints.FindIndex(constraintHandle);
  if(index == Array<ConstraintInfoReference>::InvalidIndex)
    return;

  // Efficient swap remove
  if(index != mConstraints.Size() - 1)
    mConstraints[index] = mConstraints.Back();
  mConstraints.PopBack();
}

void CustomJoint::ClearConstraints()
{
  mConstraints.Clear();
}

size_t CustomJoint::GetConstraintCount()
{
  return mConstraints.Size();
}

CustomConstraintInfo* CustomJoint::GetConstraint(size_t index)
{
  // Verify the index
  if(index >= mConstraints.Size())
  {
    DoNotifyException("Invalid Index",
                      String::Format("Index '%d' is invalid. There are only '%d' constraints available.", index, mConstraints.Size()));
    return nullptr;
  }

  return mConstraints[index];
}

void CustomJoint::ConstraintInfoToMolecule(CustomConstraintInfo* constraint, ConstraintMolecule& molecule)
{
  molecule.mJacobian.Linear[0] = constraint->mLinear0;
  molecule.mJacobian.Linear[1] = constraint->mLinear1;
  molecule.mJacobian.Angular[0] = constraint->mAngular0;
  molecule.mJacobian.Angular[1] = constraint->mAngular1;

  molecule.mMass = constraint->mEffectiveMass;
  // For efficient computations store the effective mass as the inverse effective mass
  if(molecule.mMass < Math::PositiveMin())
    molecule.mMass = real(0.0);
  else
    molecule.mMass = real(1.0) / molecule.mMass;

  molecule.mGamma = constraint->mGamma;
  molecule.mBias = constraint->mBias;
  molecule.mMinImpulse = constraint->mMinImpulse;
  molecule.mMaxImpulse = constraint->mMaxImpulse;
  molecule.mImpulse = constraint->mImpulse;
  molecule.mError = constraint->mError;

  // If we solve position then we don't want to also apply error correction via
  // baumgarte so clear this (motors and springs clear solve position)
  if(constraint->mSolvePosition)
    molecule.mBias = 0.0f;
}

void CustomJoint::UpdateTransform(int colliderIndex)
{
  Collider* collider = GetCollider(colliderIndex);
  if(collider == nullptr)
    return;

  RigidBody* body = collider->GetActiveBody();
  if(body == nullptr)
    return;

  if(!body->IsDynamic())
    return;

  body->PublishTransform();
}

}//namespace Physics

}//namespace Zero
