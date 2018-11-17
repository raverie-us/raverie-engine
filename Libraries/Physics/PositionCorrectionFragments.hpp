///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{

struct PostionCorrectionConstants
{
  static const size_t mMoleculeCount = 15;
};

struct PartialMass
{
  Vec3 PartialLinear0;
  Vec3 PartialLinear1;
  Vec3 PartialAngular0;
  Vec3 PartialAngular1;
};

struct PositionSolverData
{
  Math::FixedMatrix<6, 6> mEffectiveMasses;
  Math::FixedVector<real, 6> mErrors;
  Math::FixedVector<real, 6> mLambdas;
  Math::FixedVector<PartialMass, 6> mPartialMasses;
};

//Checks to see if this object should do position correction
bool ShouldSolvePosition(Contact* contact);
bool ShouldSolvePosition(Joint* joint);

//Collect the joints/contacts that do need to be solved into a separate list so
//we don't have to check in the inner loop if it needs to be position corrected.
template <typename JointList>
inline void CollectJointsToSolve(JointList& inputList, JointList& jointsToSolve)
{
  //go through all of the joints and figure out which ones are position corrected
  typename JointList::range joints = inputList.All();
  while(!joints.Empty())
  {
    Joint& joint = joints.Front();
    joints.PopFront();

    if(ShouldSolvePosition(&joint))
    {
      inputList.Erase(&joint);
      jointsToSolve.PushBack(&joint);
    }
  }
}

void CollectContactsToSolve(IConstraintSolver::ContactList& inputList, IConstraintSolver::ContactList& contactsToSolve,
  PhysicsSolverConfig* config);

//All other joints don't need a special update so use this
//empty function (should be optimized out in release mode).
template<typename JointType>
inline void EmptyUpdate(JointType* joint, Collider* c0, Collider* c1)
{

}

//Before solving a contact, the world points (and penetration) need to be updated from
//the body points. The normal should actually be updated, but this requires re-running
//parts of collision detection and for now it is assumed that this answer is good enough.
void ContactUpdate(Contact* contact, Collider* c0, Collider* c1);

//Forcibly update the world transformation in the entire hierarchy contained by this rigid body.
void UpdateHierarchyTransform(RigidBody* body);

//Helper to apply the position correction to a body
void ApplyPositionCorrection(RigidBody* body, Vec3Param linearOffset, Vec3Param angularOffset);

void ApplyPositionCorrection(RigidBody* b0, RigidBody* b1,
                             Vec3Param linearOffset0, Vec3Param angularOffset0,
                             Vec3Param linearOffset1, Vec3Param angularOffset1);

template <typename JointType, typename UpdateFunctor>
void JointBlockSolvePositions(JointType& joint, PositionSolverData& data, UpdateFunctor functor)
{
  ConstraintMolecule moleculeList[PostionCorrectionConstants::mMoleculeCount];

  Collider* c0 = joint.GetCollider(0);
  Collider* c1 = joint.GetCollider(1);
  RigidBody* b0 = c0->GetActiveBody();
  RigidBody* b1 = c1->GetActiveBody();

  //Make sure that the cached transforms are up to date for these objects.
  //This needs to happen at the beginning of solving because this happens after
  //position integration (hence they start out invalid). This function updates the
  //entire hierarchy, however this should probably be optimized by only updating the
  //chain of this collider (since this doesn't happen at the end anymore we
  //only need to update ourself, not the entire hierarchy).
  if(b0 != nullptr)
    UpdateHierarchyTransform(b0);
  if(b1 != nullptr)
    UpdateHierarchyTransform(b1);

  //Update atoms can alter the molecule count so it needs to be called
  //before getting the count. Call the functor first too because of contacts.
  functor(&joint, c0, c1);
  joint.UpdateAtoms();

  uint activeConstraints = joint.PositionMoleculeCount();
  if(activeConstraints == 0)
    return;

  MoleculeWalker molecules(moleculeList, sizeof(ConstraintMolecule), 0);
  joint.ComputePositionMolecules(molecules);

  //One of the best optimizations is to detect that there's nothing
  //to do and then don't do anything. If there's no error to correct
  //then just bail and don't correct anything.
  bool shouldSolve = false;
  for(uint i = 0; i < activeConstraints; ++i)
  {
    if(moleculeList[i].mError != real(0.0))
      shouldSolve = true;
  }
  if(shouldSolve == false)
    return;

  //find any constraints that can't be solved (mass term of zero) and remove
  //them from the list (this will fix things such as kinematic objects and axes
  //that can't be solved for some reason, such as 2d mode)
  uint writePosition = 0;
  uint readPosition = 0;
  uint constraintsToCheck = activeConstraints;
  //do a safe iteration by overriding any values we don't care about
  //with valid values and then resizing afterwards
  for(; readPosition < constraintsToCheck; ++readPosition)
  {
    if(moleculeList[readPosition].mMass == real(0.0))
    {
      --activeConstraints;
      continue;
    }

    //don't copy over if there's no reason to do it
    if(writePosition != readPosition)
      moleculeList[writePosition] = moleculeList[readPosition];
    ++writePosition;
  }

  //if we have nothing to solve then don't bother to do any work
  if(activeConstraints == 0)
    return;
  
  data.mErrors.Resize(activeConstraints);
  data.mLambdas.Resize(activeConstraints);
  data.mPartialMasses.Resize(activeConstraints);

  if(b0 != nullptr)
    b0->UpdateWorldInertiaTensor();
  if(b1 != nullptr)
    b1->UpdateWorldInertiaTensor();
  WorldTransformation* t0 = c0->GetWorldTransform();
  WorldTransformation* t1 = c1->GetWorldTransform();
  JointMass masses;
  JointHelpers::GetMasses(c0, c1, masses);
  
  for(uint i = 0; i < activeConstraints; ++i)
  {
    PartialMass& partialMass = data.mPartialMasses[i];
    Physics::Jacobian j = moleculeList[i].mJacobian;
  
    partialMass.PartialLinear0 = masses.mInvMass[0].Apply(j.Linear[0]);
    partialMass.PartialLinear1 = masses.mInvMass[1].Apply(j.Linear[1]);
    partialMass.PartialAngular0 = Math::Transform(masses.InverseInertia[0], j.Angular[0]);
    partialMass.PartialAngular1 = Math::Transform(masses.InverseInertia[1], j.Angular[1]);
  }
  
  //get the max linear and angular error correction that we can use
  real maxLinearError = joint.GetLinearErrorCorrection();
  real maxAngularError = joint.GetAngularErrorCorrection();

  for(uint y = 0; y < activeConstraints; ++y)
  {
    for(uint x = y; x < activeConstraints; ++x)
    {
      if(x == y)
        data.mEffectiveMasses(y, x) = real(1.0) / moleculeList[x].mMass;
      else
      {
        Physics::Jacobian& j1 = moleculeList[x].mJacobian;
        PartialMass& pm2 = data.mPartialMasses[y];
  
        real m1term = Math::Dot(j1.Linear[0], pm2.PartialLinear0);
        real m2term = Math::Dot(j1.Linear[1], pm2.PartialLinear1);
        real i1term = Math::Dot(j1.Angular[0], pm2.PartialAngular0);
        real i2term = Math::Dot(j1.Angular[1], pm2.PartialAngular1);
  
        real effectiveMass = m1term + m2term + i1term + i2term;
        data.mEffectiveMasses(y, x) = effectiveMass;
        data.mEffectiveMasses(x, y) = effectiveMass;
      }
    }

    real maxError;
    //have to pass in a real by reference, just pass a dummy in for now
    real dummy;
    //determine if we use the linear or angular error for this constraint
    if(joint.GetAtomIndexFilter(moleculeList[y].mAtomIndex, dummy) == Joint::LinearAxis)
      maxError = maxLinearError;
    else
      maxError = maxAngularError;

    data.mLambdas[y] = real(0.0);
    data.mErrors[y] = -Math::Clamp(moleculeList[y].mError, -maxError, maxError);
  }

  Math::GaussSeidelSolver solver;
  solver.mMaxIterations = 5;
  Math::GenericDimIndexPolicy policy;
  solver.Solve(data.mEffectiveMasses, data.mErrors, data.mLambdas, policy);

  Vec3 body0PosOffset = Vec3::cZero;
  Vec3 body1PosOffset = Vec3::cZero;
  Vec3 body0AngularOffset = Vec3::cZero;
  Vec3 body1AngularOffset = Vec3::cZero;
  for(uint i = 0; i < activeConstraints; ++i)
  {
    real lambda = data.mLambdas[i];
    Jacobian& jacobian = moleculeList[i].mJacobian;
    PartialMass& partialMass = data.mPartialMasses[i];

    if(b0 != nullptr)
    {
      Vec3 pos0 = lambda * partialMass.PartialLinear0;
      Vec3 angular0 = lambda * partialMass.PartialAngular0;

      body0PosOffset += pos0;
      body0AngularOffset += angular0;
    }
    if(b1 != nullptr)
    {
      Vec3 pos1 = lambda * partialMass.PartialLinear1;
      Vec3 angular1 = lambda * partialMass.PartialAngular1;

      body1PosOffset += pos1;
      body1AngularOffset += angular1;
    }
  }

  ApplyPositionCorrection(b0, b1, body0PosOffset, body0AngularOffset, body1PosOffset, body1AngularOffset);
}

template <typename ListType, typename UpdateFunctor>
inline void BlockSolvePositions(ListType& jointList, UpdateFunctor functor)
{
  PositionSolverData data;

  typename ListType::range jointRange = jointList.All();
  while(!jointRange.Empty())
  {
    typename ListType::sub_reference joint = jointRange.Front();
    jointRange.PopFront();

    JointBlockSolvePositions(joint, data, functor);
  }
}

template <typename ListType, typename UpdateFunctor>
inline void SolveConstraintPosition(ListType& jointList, UpdateFunctor functor)
{
  ConstraintMolecule moleculeList[PostionCorrectionConstants::mMoleculeCount];

  typename ListType::range jointRange = jointList.All();
  while(!jointRange.Empty())
  {
    typename ListType::sub_reference joint = jointRange.Front();
    jointRange.PopFront();

    //get the max error correction that this joint can solver
    real maxError = joint.GetLinearErrorCorrection();

    Collider* c0 = joint.GetCollider(0);
    Collider* c1 = joint.GetCollider(1);
    RigidBody* b0 = c0->GetActiveBody();
    RigidBody* b1 = c1->GetActiveBody();
    if(b0 != nullptr)
      UpdateHierarchyTransform(b0);
    if(b1 != nullptr)
      UpdateHierarchyTransform(b1);

    //have to update atoms once so we have the correct number of molecules (soooo bad!)
    functor(&joint, c0, c1);
    joint.UpdateAtoms();
    uint activeJoints = joint.PositionMoleculeCount();
    for(uint i = 0; i < activeJoints; ++i)
    {
      if(b0 != nullptr)
        b0->UpdateWorldInertiaTensor();
      if(b1 != nullptr)
        b1->UpdateWorldInertiaTensor();

      MoleculeWalker molecules(moleculeList,sizeof(ConstraintMolecule),0);
      functor(&joint, c0, c1);
      joint.UpdateAtoms();
      joint.ComputePositionMolecules(molecules);

      ConstraintMolecule& mol = moleculeList[i];
      JointMass masses;
      JointHelpers::GetMasses(c0, c1, masses);

      mol.mError = Math::Min(mol.mError, maxError);
      real lambda = -mol.mMass * mol.mError;

      Vec3 pos0 = Vec3::cZero;
      Vec3 angular0 = Vec3::cZero;
      Vec3 pos1 = Vec3::cZero;
      Vec3 angular1 = Vec3::cZero;
      if(b0 != nullptr)
      {
        pos0 = lambda * masses.mInvMass[0].Apply(mol.mJacobian.Linear[0]);
        angular0 = lambda * Math::Transform(masses.InverseInertia[0],mol.mJacobian.Angular[0]);
      }
      if(b1 != nullptr)
      {
        pos1 = lambda * masses.mInvMass[1].Apply(mol.mJacobian.Linear[1]);
        angular1 = lambda * Math::Transform(masses.InverseInertia[1],mol.mJacobian.Angular[1]);
      }

      ApplyPositionCorrection(b0, b1, pos0, angular0, pos1, angular1);
    }
  }
}

}//namespace Physics

}//namespace Zero
