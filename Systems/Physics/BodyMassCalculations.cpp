///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//Functor for computing the total mass and center of mass on a rigid body hierarchy.
struct MassComputationFunctor
{
  //Formula for the new center of mass
  //R = (mi*ri) / (mi) (summation notation)
  //where i is the index of each object, 
  //m is the mass, 
  //r is the center of mass of the i'th object and
  //R is the new center of mass

  MassComputationFunctor()
  {
    mCenterMass.ZeroOut();
    mTotalMass = real(0.0);
    mNoColliders = true;
  }

  void operator()(Collider* collider, RigidBody* directBody)
  {
    float mass = collider->ComputeMass();

    mCenterMass += mass * collider->GetColliderWorldCenterOfMass();
    mTotalMass += mass;

    mNoColliders = false;
  }

  void GetMassTerms(Vec3Ref centerMass, real& invMass, bool& massless)
  {
    invMass = real(1.0);
    massless = true;
    //deal with having no mass (density of 0)
    if(mTotalMass != 0.0f)
    {
      invMass = real(1.0) / mTotalMass;
      massless = false;
    }

    centerMass = mCenterMass * invMass;
  }

  real mTotalMass;
  Vec3 mCenterMass;
  bool mNoColliders;
};

//Functor for setting the massless flag on all of
//the colliders in a rigid body hierarchy.
struct MasslessFunctor
{
  MasslessFunctor(bool& massless)
  {
    mMassless = massless;
  }

  void operator()(Collider* collider, RigidBody* directBody)
  {
    collider->mState.SetState(ColliderFlags::MasslessBody, mMassless);
  }

  bool mMassless;
};

DeclareBitField3(InertiaStates,Valid,TooSmall,TooLarge);

//Functor for computing the inertia tensor on a rigid body hierarchy.
struct InertiaComputationFunctor
{
  //Formula for computing the inertia tensor (generic form of the parallel axis theorem)
  //Jij = Iij + m(|r|^2 * (kronecker delta)ij - ri * rj)
  //where i and j are the indices, 
  //r is the vector from center of mass to the new axis, 
  //I is the inertia tensor about the center of mass and
  //J is the new inertia tensor about the new axis.
  //This formula can also be re-written as:
  //J = I + (Dot(r, r) * Identity) - outerproduct(r, r)) * mass
  //where outerproduct is define as Mat3(a.x * b, a.y * b, a.z * b);

  //unfortunately, this calculation is done with inertia tensors, 
  //not inverse inertia tensors. Because of this we have to invert lots of times.

  InertiaComputationFunctor(Vec3 centerMass)
  {
    mStates.Clear();
    mCenterMass = centerMass;
    mInertiaTensor.ZeroOut();
    mIdentity.SetIdentity();
  }

  void operator()(Collider* collider, RigidBody* directBody)
  {
    real mass = collider->ComputeMass();
    //if the mass is too small, don't add this for inertia
    if(mass < real(.000001))
    {
      mStates.SetFlag(InertiaStates::TooSmall);
      return;
    }
    //if the mass is too large, also don't add it for inertia
    else if(mass > real(9999999.0))
    {
      mStates.SetFlag(InertiaStates::TooLarge);
      return;
    }

    mStates.SetFlag(InertiaStates::Valid);
    Mat3 invInertiaTensor;
    collider->ComputeLocalInverseInertiaTensor(mass, invInertiaTensor);

    Mat3 worldOrientation = collider->GetWorldRotation();
    Mat3 worldOrientationInv = worldOrientation.Transposed();

    //get the world space inertia tensor
    Mat3 inertiaTensorM = invInertiaTensor.SafeInverted();
    Mat3 inertiaTensorW = worldOrientation * inertiaTensorM * worldOrientationInv;


    Vec3 worldCenterOfMass = collider->GetColliderWorldCenterOfMass();
    Geometry::CombineInertiaTensor(mInertiaTensor, mCenterMass, inertiaTensorW, worldCenterOfMass, mass);
  }

  bool GetResults(Mat3Ref invInertiaM, RigidBody* body)
  {
    //if we never got a valid tensor, then leave the tensor as the zero matrix
    //(should fix this for a very small object, but oh well)
    if(!mStates.IsSet(InertiaStates::Valid))
    {
      invInertiaM.ZeroOut();
      return false;
    }
    //if any inertia was large enough to be infinite,
    //set the whole thing to infinite
    else if(mStates.IsSet(InertiaStates::TooLarge))
    {
      invInertiaM.ZeroOut();
      return false;
    }

    //this was a world space calculation, have to bring it from world space to 
    //body space
    WorldTransformation* transform = body->mPhysicsNode->GetTransform();
    Mat3 rotation = transform->GetWorldRotation();
    invInertiaM = rotation.Transposed() * mInertiaTensor * rotation;
    //we also want the inverse tensor
    invInertiaM.SafeInvert();

    return true;
  }

  Vec3 mCenterMass;
  Mat3 mInertiaTensor;
  Mat3 mIdentity;
  BitField<InertiaStates::Enum> mStates;
};

template <typename Functor>
void Recurse(RigidBody* body, Functor& functor)
{
  Array<RigidBody*> stack;
  stack.Reserve(10);
  stack.PushBack(body);

  while(!stack.Empty())
  {
    RigidBody* currentBody = stack.Back();
    stack.PopBack();

    //deal with all direct colliders of this body
    RigidBody::CompositeColliderList::range range = currentBody->mColliders.All();
    for(; !range.Empty(); range.PopFront())
      functor(&range.Front(), currentBody);

    //queue up child bodies as long as they are static or kinematic
    RigidBody::ChildBodyList::range bodies = currentBody->mChildBodies.All();
    for(; !bodies.Empty(); bodies.PopFront())
    {
      RigidBody* childBody = &bodies.Front();
      if(!childBody->IsDynamic())
        stack.PushBack(childBody);
    }
  }
}

void InternalComputeMass(RigidBody* body, Vec3Ref centerOfMass, real& invMass)
{
  WorldTransformation* transformation = body->mPhysicsNode->GetTransform();
  //Check to see if we have a mass override. If our mass override is active and
  //has the center of mass locked, there's no point in computing anything since
  //we're overriding it all anyways
  MassOverride* massOverride = body->GetOwner()->has(MassOverride);
  if(massOverride && massOverride->GetActive() && !massOverride->GetAutoComputeCenterOfMass())
  {
    //center of mass is in world space, so make sure to transform it
    centerOfMass = transformation->TransformPoint(massOverride->GetLocalCenterOfMass());
    invMass = massOverride->GetInverseMass();
    return;
  }

  //We now have to compute the mass and center of mass. Even if we
  //are static or massless we need to know where the center of mass is.
  MassComputationFunctor massComputation;
  Recurse(body, massComputation);

  //If there were no colliders then we are just a floating body.
  //Set up the mass and center of mass real quick and bail.
  if(massComputation.mNoColliders)
  {
    //If we had a mass override grab it's mass. Don't bother with the
    //center of mass because we would've bailed earlier if that was set.
    if(massOverride && massOverride->GetActive())
      invMass = massOverride->GetInverseMass();
    //Otherwise, just set the mass to 1
    else
      invMass = real(1.0);

    //We don't know where the center of mass is supposed to be,
    //so just put it at the translation of the root.
    Transform* transform = body->GetOwner()->has(Transform);
    centerOfMass = transform->GetTranslation();
    return;
  }

  //get the actual computed mass terms
  bool massless;
  massComputation.GetMassTerms(centerOfMass, invMass, massless);

  //however, if we have a mass override that's active, grab that
  //mass instead but use the computed center of mass
  if(massOverride && massOverride->GetActive())
    invMass = massOverride->GetInverseMass();

  //set the massless state on all child colliders
  MasslessFunctor masslessFunctor(massless);
  Recurse(body, masslessFunctor);
}

void ComputeMass(RigidBody* body)
{
  //compute the mass and center of mass
  //(this internal function deals with mass override and massless states)
  real invMass;
  Vec3 centerOfMass;
  InternalComputeMass(body, centerOfMass, invMass);

  body->SetInverseMass(invMass);

  //deal with 2d mode
  if(body->mState.IsSet(RigidBodyStates::Mode2D))
    body->mInvMass.SetAxisLock(true, 2);


  //fix the center of mass offset from the transform position

  //the center of mass doesn't care about 2d mode
  //(if mass override changed the center of mass then we overrode the variable earlier)
  body->mCenterOfMass = centerOfMass;

  WorldTransformation* transform = body->mPhysicsNode->GetTransform();
  //compute the position offset from the center of mass to the
  //position of the parent collider in it's body space (only rotation).
  body->mPositionOffset = transform->GetWorldTranslation() - centerOfMass;
  body->mPositionOffset = Math::TransposedTransform(transform->GetWorldRotation(), 
    body->mPositionOffset);

  //We need to have infinite mass to be kinematic but we needed to compute where
  //the center of mass was. This requires computing the inverse mass.
  if(body->IsDynamic() == false)
    body->mInvMass.SetInvMass(0);
}

void ComputeInertia(RigidBody* body)
{
  //deal real quick with locked rotations
  if(body->mState.IsSet(RigidBodyStates::RotationLocked) ||
     !body->IsDynamic())
  {
    body->mInvInertia.ClearTensors();
    return;
  }

  //if we have an override, there's no point in computing inertia since we'll
  //just replace it anyways
  MassOverride* massOverride = body->GetOwner()->has(MassOverride);
  if(massOverride && massOverride->GetActive())
  {
    body->mInvInertia.SetInvTensorModel(massOverride->GetLocalInverseInertiaTensor());
    return;
  }

  //compute the inertia
  InertiaComputationFunctor inertiaFunctor(body->InternalGetWorldCenterOfMass());
  Recurse(body, inertiaFunctor);

  Mat3 invInertiaM;
  inertiaFunctor.GetResults(invInertiaM, body);

  body->mInvInertia.SetInvTensorModel(invInertiaM);
}

}//namespace Zero
