///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const real cAngularSleepEpsilon = real(0.16);//should be about (2/180)*pi
const real cLinearSleepEpsilon = real(0.16); //should be about .016
const real cTimeToSleep = real(1); // 1 second
const real RigidBody::mMaxVelocity = real(1e+10);

ZilchDefineType(RigidBody, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
  ZeroBindTag(Tags::Physics);

  ZeroBindDependency(Cog);
  ZeroBindDependency(Transform);
  ZeroBindEvent(Events::RigidBodySlept, ObjectEvent);
  ZeroBindEvent(Events::RigidBodyAwoke, ObjectEvent);

  // Bind velocity methods
  ZilchBindGetterSetterProperty(Velocity)->ZeroSerialize(Vec3::cZero);
  ZilchBindGetterSetterProperty(AngularVelocity)->ZeroSerialize(Vec3::cZero);
  ZilchBindMethod(ApplyLinearVelocity);
  ZilchBindMethod(ApplyAngularVelocity);
  ZilchBindMethod(ApplyVelocityAtPoint);
  ZilchBindMethod(GetPointVelocity);
  // Bind force methods
  ZilchBindGetterSetter(Force);
  ZilchBindGetterSetter(Torque);
  ZilchBindMethod(ApplyForce);
  ZilchBindMethod(ApplyTorque);
  ZilchBindMethod(ApplyForceAtOffsetVector);
  ZilchBindMethod(ApplyForceAtPoint);
  // Bind impulse methods
  ZilchBindMethod(ApplyLinearImpulse);
  ZilchBindMethod(ApplyAngularImpulse);
  ZilchBindMethod(ApplyImpulseAtOffsetVector);
  ZilchBindMethod(ApplyImpulseAtPoint);

  ZilchBindGetterSetterProperty(DynamicState)->ZeroSerialize(RigidBodyDynamicState::Dynamic);
  ZilchBindGetterSetterProperty(AllowSleep)->ZeroSerialize(true);
  ZilchBindGetterSetter(Asleep)->ZeroSerialize(true);
  ZilchBindMethod(ForceAwake);
  ZilchBindMethod(ForceAsleep);
  ZilchBindGetterSetterProperty(RotationLocked)->ZeroSerialize(false);
  ZilchBindGetterSetterProperty(Mode2D)->ZeroSerialize(Mode2DStates::InheritFromSpace);
  ZilchBindGetterProperty(Mass);
  ZilchBindGetter(LocalInverseInertiaTensor);
  ZilchBindGetter(WorldInverseInertiaTensor);
  ZilchBindGetter(WorldCenterOfMass);
  ZilchBindGetter(ActiveBody);
}

RigidBody::RigidBody()
{
  mVelocity.ZeroOut();
  mVelocityOld.ZeroOut();
  mAngularVelocity.ZeroOut();
  mAngularVelocityOld.ZeroOut();
  mForceAccumulator.ZeroOut();
  mTorqueAccumulator.ZeroOut();
  mCenterOfMass.ZeroOut();
  mPositionOffset.ZeroOut();
  mRotationQuat = Quat::cIdentity;
  mSleepTimer = real(0);
  mState.SetFlag(RigidBodyStates::AllowSleep);
  mState.SetFlag(RigidBodyStates::Inherit2DMode);
  mState.ClearFlag(RigidBodyStates::SleepAccumulated);
  SetAxisLock(false, false, false);
  mParentBody = nullptr;
  mPhysicsNode = nullptr;
  mSpaceEffectsToIgnore = nullptr;
  mSpace = nullptr;
}

void RigidBody::Serialize(Serializer& stream)
{
  MetaSerializeProperties(this, stream);
}

void RigidBody::Initialize(CogInitializer& initializer)
{
  mSpace = initializer.mSpace->has(PhysicsSpace);
  ErrorIf(!mSpace, "Rigid Body's parent has no physics space.");
  
  mSpace->AddComponent(this);

  // Just default center of mass to the translation of the object (shouldn't matter)
  Transform* transform = GetOwner()->has(Transform);
  mCenterOfMass = transform->GetTranslation();

  bool dynamicallyCreated = (initializer.Flags & CreationFlags::DynamicallyAdded) != 0;
  BodyInitialize(this, dynamicallyCreated);

  // We've set the 3 states available from serialization (inherit, 2d, 3d) but we haven't
  // actually been set-up yet. To do this call the internal set2d function with our actual
  // 2d state (checking the space if necessary)
  bool is2D = mState.IsSet(RigidBodyStates::Mode2D);
  if(mState.IsSet(RigidBodyStates::Inherit2DMode))
    is2D = mSpace->GetMode2D();
  Set2DInternal(is2D);
 
  SetRotationLockedInternal(mState.IsSet(RigidBodyStates::RotationLocked));
  
  // If we're kinematic then override the old transform values so that the kinematic
  // values do not get overridden to weird values on initialization
  if(GetKinematic())
    QueueOverrideOldTransform(this);
}

void RigidBody::OnAllObjectsCreated(CogInitializer& initializer)
{
  bool dynamicallyCreated = (initializer.Flags & CreationFlags::DynamicallyAdded) != 0;
  BodyOnAllObjectsCreated(this, dynamicallyCreated);

  QueueOverrideOldTransform(this);
}

void RigidBody::AttachTo(AttachmentInfo& info)
{
  PhysicsAttachTo(mPhysicsNode, info);
}

void RigidBody::Detached(AttachmentInfo& info)
{
  PhysicsDetach(mPhysicsNode, info);
}

void RigidBody::TransformUpdate(TransformUpdateInfo& info)
{
  // If physics caused the transform update then don't do anything
  if((info.TransformFlags & TransformUpdateFlags::Physics) != 0)
    return;

  // This should ideally never happen, but there are cases where someone
  // triggers a transform update before we ever get Initialize.
  // This will just safeguard that real quick.
  if(mPhysicsNode == nullptr || mPhysicsNode->IsDying())
    return;

  // If we ever get a transform update but have no collider, that means we have
  // to handle queuing the transform update for this node
  if(mPhysicsNode->mCollider == nullptr)
    FullTransformAction transformAction(mPhysicsNode);

  // No matter what transform action happened, we need to queue up mass updates
  // (might be able to optimize this in certain cases (root), but this is just queuing
  // us up, so it is fairly cheap)
  Collider* collider = mPhysicsNode->mCollider;
  if(collider == nullptr || !collider->mState.IsSet(ColliderFlags::MasslessCollider))
    QueueMassUpdate();

  // Deal with kinematic state
  if(mState.IsSet(RigidBodyStates::Kinematic))
  {
    KinematicMovementAction action(this);
    mSpace->ActivateKinematic(this);
  }
}

void RigidBody::OnDestroy(uint flags)
{
  bool dynamicallyRemoved = (flags & DestroyFlags::DynamicallyDestroyed) != 0;
  BodyOnDestroy(this, dynamicallyRemoved);

  // Remove the space's reference to us (important to do after the children are removed in the above function).
  mSpace->RemoveComponent(this);
}

Vec3 RigidBody::GetVelocity()
{
  // If we're kinematic, make sure that we flush the queue if there are any outstanding changes.
  // If we don't do this and the user manually moves the object and immediately call GetVelocity
  // then they won't get the correct kinematic velocity.
  UpdateKinematicVelocities();
  return mVelocity;
}

void RigidBody::SetVelocity(Vec3Param velocity)
{
  // Clamp velocity to attempt to deal with bad floating point values
  mVelocity = ClampVelocityValue(velocity, "velocity");
}

Vec3 RigidBody::GetAngularVelocity()
{
  // See the comment in GetVelocity()
  UpdateKinematicVelocities();
  return mAngularVelocity;
}

void RigidBody::SetAngularVelocity(Vec3Param angularVelocity)
{
  mAngularVelocity = ClampVelocityValue(angularVelocity, "angular velocity");
}

Vec3 RigidBody::GetForce()
{
  return mForceAccumulator;
}

void RigidBody::SetForce(Vec3Param force)
{
  mForceAccumulator = force;
}

Vec3 RigidBody::GetTorque()
{
  return mTorqueAccumulator;
}

void RigidBody::SetTorque(Vec3Param torque)
{
  mTorqueAccumulator = torque;
}

void RigidBody::ApplyLinearVelocity(Vec3Param linear)
{
  mVelocity += linear;
}

void RigidBody::ApplyAngularVelocity(Vec3Param angular)
{
  mAngularVelocity += angular;
}

void RigidBody::ApplyVelocityAtPoint(Vec3Param velocity, Vec3Param worldPoint)
{
  // Check for changes to the center of mass
  UpdateResourcesAndQueue();

  Vec3 worldR = worldPoint - mCenterOfMass;
  Vec3 angularVelocity = Math::Cross(worldR, velocity);
  ApplyLinearVelocity(velocity);
  ApplyAngularVelocity(angularVelocity);
}

Vec3 RigidBody::GetPointVelocity(Vec3Param worldPoint)
{
  // Check for changes to the center of mass
  UpdateResourcesAndQueue();

  // Find the active body to calculate velocity from
  RigidBody* activeBody = GetActiveBody();

  // Calculate the point velocity from this rigid body
  // (if the active body ends up being static this will just result in zero)
  return activeBody->GetPointVelocityInternal(worldPoint);
}

void RigidBody::ApplyForce(Vec3Param force)
{
  mForceAccumulator += force;
}

void RigidBody::ApplyTorque(Vec3Param torque)
{
  mTorqueAccumulator += torque;
}

void RigidBody::ApplyForceAtOffsetVector(Vec3Param force, Vec3Param worldOffset)
{
  Vec3 torque = Math::Cross(worldOffset, force);
  ApplyForce(force);
  ApplyTorque(torque);
}

void RigidBody::ApplyForceAtPoint(Vec3Param force, Vec3Param worldPoint)
{
  // Check for changes to the center of mass
  UpdateResourcesAndQueue();

  Vec3 worldR = worldPoint - mCenterOfMass;
  ApplyForceAtOffsetVector(force, worldR);
}

void RigidBody::ApplyLinearImpulse(Vec3Param linear)
{
  // Check for changes to the mass
  UpdateResourcesAndQueue();
  mVelocity += mInvMass.Apply(linear);
}

void RigidBody::ApplyAngularImpulse(Vec3Param angular)
{
  // Check for changes to the inertia
  UpdateResourcesAndQueue();
  mAngularVelocity += mInvInertia.Apply(angular);
}

void RigidBody::ApplyConstraintImpulse(Vec3Param linear, Vec3Param angular)
{
  ApplyLinearImpulse(linear);
  ApplyAngularImpulse(angular);
}

void RigidBody::ApplyImpulseAtOffsetVector(Vec3Param impulse, Vec3Param worldOffset)
{
  Vec3 angularMomentum = Math::Cross(worldOffset, impulse);
  ApplyConstraintImpulse(impulse, angularMomentum);
}

void RigidBody::ApplyImpulseAtPoint(Vec3Param impulse, Vec3Param worldPoint)
{
  // Check for changes to the center of mass (also mass and inertia)
  UpdateResourcesAndQueue();

  Vec3 worldR = worldPoint - mCenterOfMass;
  ApplyImpulseAtOffsetVector(impulse, worldR);
}

RigidBodyDynamicState::Enum RigidBody::GetDynamicState()
{
  // Convert the bits to the correct enum state
  if(mState.IsSet(RigidBodyStates::Static))
    return RigidBodyDynamicState::Static;
  if(mState.IsSet(RigidBodyStates::Kinematic))
    return RigidBodyDynamicState::Kinematic;
  return RigidBodyDynamicState::Dynamic;
}

void RigidBody::SetDynamicState(RigidBodyDynamicState::Enum state)
{
  // If we're not initialized then just set the correct flag states and exit
  if(!IsInitialized())
  {
    // Always start with being dynamic and then set the correct static/kinematic bit if necessary
    mState.ClearFlag(RigidBodyStates::Static | RigidBodyStates::Kinematic);
    if(state == RigidBodyDynamicState::Static)
      mState.SetFlag(RigidBodyStates::Static);
    else if(state == RigidBodyDynamicState::Kinematic)
      mState.SetFlag(RigidBodyStates::Kinematic);
    return;
  }

  // If we're changing to the same state don't do anything. This is not only a performance win
  // (rebuilding trees, recomputing terms, etc...) but this prevents accidental behavior such 
  // as keeping the rigid body awake by constantly setting it to dynamic.
  if(state == GetDynamicState())
    return;

  // Otherwise, we need to set the correct flags and update all correct terms.
  // The mass and velocities could change if we freeze this rigid body, but the active rigid body
  // in the hierarchy could also change so the tree needs to be updated.

  // It's easiest to start with a dynamic state and then set individual bits afterwards
  mState.ClearFlag(RigidBodyStates::Kinematic | RigidBodyStates::Static);
  if(state == RigidBodyDynamicState::Static)
  {
    mState.SetFlag(RigidBodyStates::Static);
    ClearMassAndVelocities();
  }
  else if(state == RigidBodyDynamicState::Kinematic)
  {
    mState.SetFlag(RigidBodyStates::Kinematic);
    ClearMassAndVelocities();

    // Since we switched to kinematic, we have to override our old
    // transform values so we won't register a teleportation
    QueueOverrideOldTransform(this);
  }
  else
  {
    QueueMassUpdate();
  }

  // Tell the physics space that this body has changed state
  // (so the space knows which list this body should be in)
  BodyStateChanged(this);
}

bool RigidBody::GetStatic()
{
  return mState.IsSet(RigidBodyStates::Static);
}

bool RigidBody::GetKinematic() const
{
  return mState.IsSet(RigidBodyStates::Kinematic);
}

bool RigidBody::IsDynamic()
{
  return !(mState.IsSet(RigidBodyStates::Kinematic | RigidBodyStates::Static));
}

bool RigidBody::GetAllowSleep() const
{
  return mState.IsSet(RigidBodyStates::AllowSleep);
}

void RigidBody::SetAllowSleep(bool state)
{
  mState.SetState(RigidBodyStates::AllowSleep, state);
  if(!IsInitialized())
    return;

  if(!state)
    ForceAwake();
}

bool RigidBody::GetAsleep()
{
  return IsAsleep();
}

void RigidBody::SetAsleep(bool asleep)
{
  // We're being initialized, just set the flag and exit
  if(!IsInitialized())
  {
    mState.SetState(RigidBodyStates::Asleep, asleep);
    return;
  }

  // Force the object awake or asleep as necessary
  if(asleep)
    ForceAsleep();
  else
    ForceAwake();
}

bool RigidBody::IsAsleep()
{
  return mState.IsSet(RigidBodyStates::Asleep);
}

void RigidBody::ForceAwake()
{
  // Reset the sleep timer.
  mSleepTimer = real(0);

  // Deal with state changes and event sending.
  InternalWakeUp();

  // Force all of our static/kinematic bodies awake. This doesn't propagate
  // to child rigid bodies as they move independently
  BodyRange bodies = mChildBodies.All();
  for(; !bodies.Empty(); bodies.PopFront())
  {
    RigidBody& body = bodies.Front();
    if(!body.IsDynamic())
      body.ForceAwake();
  }
}

void RigidBody::ForceAsleep()
{
  // Set the sleep timer to the max value so that we won't be woken up next frame
  mSleepTimer = cTimeToSleep;
  PutToSleep();
}

bool RigidBody::GetRotationLocked() const
{
  return mState.IsSet(RigidBodyStates::RotationLocked);
}

void RigidBody::SetRotationLocked(bool state)
{
  // If we aren't initialized yet then just set the flag
  if(!IsInitialized())
  {
    mState.SetState(RigidBodyStates::RotationLocked, state);
    return;
  }

  SetRotationLockedInternal(state);
  // If we were rotation locked and now we aren't then we need to force
  // ourself awake (so we could start falling if we need to). @JoshD: should I do this
  // even if you go to rotation locked? Does the optimization matter?
  if(!state)
    ForceAwake();
}

Mode2DStates::Enum RigidBody::GetMode2D() const
{
  // Convert our bits to the enum representation
  if(mState.IsSet(RigidBodyStates::Inherit2DMode))
    return Mode2DStates::InheritFromSpace;
  else if(mState.IsSet(RigidBodyStates::Mode2D))
    return Mode2DStates::Mode2D;
  else
    return Mode2DStates::Mode3D;
}

void RigidBody::SetMode2D(Mode2DStates::Enum state)
{
  // If we aren't initialized yet then just set the correct flags, we'll take care of properly
  // updating the flags (such as inherit) and everything else in initialize.
  if(!IsInitialized())
  {
    mState.ClearFlag(RigidBodyStates::Inherit2DMode | RigidBodyStates::Mode2D);
    if(state == Mode2DStates::InheritFromSpace)
      mState.SetFlag(RigidBodyStates::Inherit2DMode);
    else if(state == Mode2DStates::Mode2D)
      mState.SetFlag(RigidBodyStates::Mode2D);
    return;
  }

  // We need to set bits from the enum, but we also need to determine who's mode 2d we're using (ours or the space's)
  bool mode2D = true;
  // If we inherit from the space then set that state and get the space's mode2d state
  if(state == Mode2DStates::InheritFromSpace)
  {
    mState.SetFlag(RigidBodyStates::Inherit2DMode);
    mode2D = mSpace->GetMode2D();
  }
  // Otherwise we need to clear the inherit from space flag and mark whether we're going to 2d
  else
  {
    mState.ClearFlag(RigidBodyStates::Inherit2DMode);
    mode2D = (state == Mode2DStates::Mode2D);
  }

  // Once we know our desired state (2d or not, it doesn't matter if
  // this is inherited) then we need to update our state
  Set2DInternal(mode2D);
}

real RigidBody::GetMass()
{
  UpdateResourcesAndQueue();

  real invMass = mInvMass.GetScalarInvMass();
  if(invMass != real(0))
    return real(1) / invMass;
  return real(0);
}

void RigidBody::SetMass(real mass)
{
  ErrorIf(mass == real(0), "Cannot have 0 mass.");
  SetInverseMass(real(1) / mass);
}

void RigidBody::SetInverseMass(real invMass)
{
  mInvMass.SetInvMass(invMass);
}

Mat3 RigidBody::GetLocalInverseInertiaTensor()
{
  UpdateResourcesAndQueue();
  return mInvInertia.GetInvModelTensor();
}

Mat3 RigidBody::GetWorldInverseInertiaTensor()
{
  UpdateResourcesAndQueue();
  return mInvInertia.GetInvWorldTensor();
}

Vec3 RigidBody::GetWorldCenterOfMass()
{
  UpdateResourcesAndQueue();

  return InternalGetWorldCenterOfMass();
}

Vec3 RigidBody::GetWorldTranslation() const
{
  return mPhysicsNode->GetTransform()->GetWorldTranslation();
}

Mat3 RigidBody::GetWorldRotationMat3() const
{
  return mPhysicsNode->GetTransform()->GetWorldRotation();
}

Quat RigidBody::GetWorldRotationQuat() const
{
  return mRotationQuat;
}

Mat4 RigidBody::GetWorldMatrix() const
{
  return mPhysicsNode->GetTransform()->GetWorldMatrix();
}

RigidBody* RigidBody::GetActiveBody()
{
  // Find the closest non-static parent rigid body up the hierarchy
  RigidBody* activeBody = this;
  while(activeBody != nullptr)
  {
    if(!activeBody->GetStatic())
      break;

    activeBody = activeBody->mParentBody;
  }
  if(activeBody == nullptr)
    activeBody = this;
  return activeBody;
}

RigidBody::CompositeColliderList::range RigidBody::GetColliders()
{
  return mColliders.All();
}

Vec3 RigidBody::ClampVelocityValue(Vec3Param value, StringParam varName)
{
  // If we haven't been initialized yet then don't notify anyone about invalid
  // values and just clamp with our hard-coded max (instead of the space's)
  if(!IsInitialized())
  {
    Vec3 maxVel = Vec3(RigidBody::mMaxVelocity);
    return Math::Clamp(value, -maxVel, maxVel);
  }

  // Clamp to a max velocity value
  real maxVel = mSpace->mMaxVelocity;
  bool wasClamped = false;
  Vec3 val = Math::DebugClamp(value, Vec3(-maxVel), Vec3(maxVel), wasClamped);

  // If anything was clamped and we haven't already had an
  // invalid velocity then display an error message
  if(wasClamped && mSpace->mInvalidVelocityOccurred == false)
  {
    // During a game we only want to display an error message once, however if
    // we're in editor we want to display the error message every time.
    if(!GetSpace()->IsEditorMode())
      mSpace->mInvalidVelocityOccurred = true;

    String objName = GetOwner()->GetDescription();
    String errStr = String::Format("%s was set beyond the max range of [%g, %g] on object %s"
      "The %s will be clamped to this range.", varName.c_str(),
      -maxVel, maxVel, objName.c_str(), varName.c_str());

    String errTitle = String::Format("Setting invalid %s", varName.c_str());
    DoNotifyWarning(errTitle, errStr);
  }
  return val;
}

bool RigidBody::IsInitialized()
{
  return mSpace != nullptr;
}

Vec3 RigidBody::GetPointVelocityInternal(Vec3Param worldPoint)
{
  Vec3 worldR = worldPoint - mCenterOfMass;
  return mVelocity + Cross(mAngularVelocity, worldR);
}

void RigidBody::PutToSleep()
{
  mState.SetFlag(RigidBodyStates::Asleep);
  // Clear velocity and force
  mVelocity.ZeroOut();
  mVelocityOld.ZeroOut();
  mAngularVelocity.ZeroOut();
  mAngularVelocityOld.ZeroOut();
  mForceAccumulator.ZeroOut();
  mTorqueAccumulator.ZeroOut();
  // Notify everyone we fell asleep
  ObjectEvent toSend(this);
  GetOwner()->DispatchEvent(Events::RigidBodySlept, &toSend);
}

void RigidBody::WakeUp()
{
  // Deal with state changes and event sending
  InternalWakeUp();
  
  // Wake up all of our static/kinematic bodies
  BodyRange bodies = mChildBodies.All();
  for(; !bodies.Empty(); bodies.PopFront())
  {
    RigidBody* body = &bodies.Front();
    if(!body->IsDynamic())
      body->WakeUp();
  }
}

void RigidBody::InternalWakeUp()
{
  // If we're already awake then don't do anything
  if(!IsAsleep())
    return;

  // We could try to do this only if we're a dynamic object, but kinematic objects
  // can actually fall asleep. While it seems like this might not make sense it is
  // a performance gain since kinematics then do not have to check against
  // static/kinematics for detection. Because of these things, don't do any extra
  // check other than if it's asleep.
  
  
  // Clear the sleep bit.
  mState.ClearFlag(RigidBodyStates::Asleep);
  // Put ourselves in the correct list.
  mSpace->ComponentStateChange(this);
  
  // If the object was asleep and we told it to wake up we need to reset the timer.
  // If this is not done then the object will immediately fall asleep
  // again if its velocity (linear or angular) is not large enough.
  mSleepTimer = real(0);

  // Tell everyone that we woke up
  ObjectEvent toSend(this);
  GetOwner()->DispatchEvent(Events::RigidBodyAwoke, &toSend);
}

bool RigidBody::UpdateSleepTimer(real dt)
{
  // If we've already been updated, we still need to run the below logic to
  // determine whether to return true or false but we don't want to doubly
  // increment the timer by dt. As a quick fix zero out dt so we run the
  // correct logic we just don't doubly accumulate.
  if(mState.IsSet(RigidBodyStates::SleepAccumulated))
    dt = real(0);
  mState.SetFlag(RigidBodyStates::SleepAccumulated);

  // If these values are left in the if statement, debug and release can behave
  // differently when it comes to floating point exceptions (Release will trip
  // while debug won't). For consistency, they are pulled out of the loop.
  real velLenSq = mVelocity.LengthSq();
  real angVelLenSq = mAngularVelocity.LengthSq();
  // If we are able to sleep, increment our sleep timer
  if(GetAllowSleep() && 
     velLenSq <= cLinearSleepEpsilon * cLinearSleepEpsilon &&
     angVelLenSq <= cAngularSleepEpsilon * cAngularSleepEpsilon)
  {
    mSleepTimer += dt;
    return true;
  }
  
  // Otherwise we aren't able to sleep so reset the sleep timer
  mSleepTimer = real(0);
  return false;
}

void RigidBody::SetRotationLockedInternal(bool state)
{
  // If no state is changing, do nothing
  if(state == mState.IsSet(RigidBodyStates::RotationLocked))
    return;

  if(state)
  {
    mState.SetFlag(RigidBodyStates::RotationLocked);
    SetAxisLock(true, true, true);
    mAngularVelocity.ZeroOut();
  }
  else
  {
    mState.ClearFlag(RigidBodyStates::RotationLocked);
    // At the moment, body states (2d, rotation locked, etc) are not taken
    // into account on child bodies when computing mass and inertia. Change
    // this later to work (not sure what exactly the correct behavior is...)
    // but until then, we only need to compute inertia when we stop being
    // rotation locked since we might be a root body
    QueueMassUpdate();
  }
}

void RigidBody::Set2DInternal(bool state)
{
  if(state)
  {
    mState.SetFlag(RigidBodyStates::Mode2D);
    // Clear out any old velocity on the z-axis
    mVelocity[2] = real(0);
    // Lock linear motion on the z-axis
    mInvMass.SetAxisLock(true, 2);
    // In mode-2d, inertia is updated every frame (so we lock world axes) so nothing needs to happen here.
  }
  else
  {
    mState.ClearFlag(RigidBodyStates::Mode2D);
    mInvMass.SetAxisLock(false, 2);
  }
}

void RigidBody::UpdateMode2D()
{
  if(mState.IsSet(RigidBodyStates::Inherit2DMode))
  {
    bool mode2D = mSpace->GetMode2D();
    Set2DInternal(mode2D);
  }
}

void RigidBody::ClearMassAndVelocities()
{
  // Clear mass, inertia, and velocities
  mInvMass.SetInvMass(0);
  mInvInertia.ClearTensors();
  mVelocity.ZeroOut();
  mAngularVelocity.ZeroOut();
}

void RigidBody::UpdateKinematicVelocities()
{
  // Check if a kinematic velocity computation is queued up
  TransformAction& action = mPhysicsNode->GetQueue()->mTransformAction;
  if(!(action.mState & TransformAction::KinematicVelocity))
    return;
  
  WorldTransformation* transform = mPhysicsNode->GetTransform();
  Vec3 oldTranslation = transform->GetOldTranslation();
  Mat3 oldRotation = transform->GetOldRotation();

  // We have to read the new transform and recompute the
  // world transform since the physics queue hasn't update these yet
  mPhysicsNode->ReadTransform();
  mPhysicsNode->RecomputeWorldTransform();

  // Since this was called from a Get function we want to use the dt in the user's scope.
  // The only reasonable assumption here is that this is the time-space's dt (not the physics iteration dt)
  TimeSpace* timeSpace = mSpace->GetOwner()->has(TimeSpace);
  ReturnIf(timeSpace == nullptr, , "No time space.");
  real dt = timeSpace->GetDtOrZero();
  if(dt == 0)
    return;

  // Now we can compute the kinematic velocities
  ComputeVelocities(oldTranslation, oldRotation, dt);
  // Make sure to clear that we need a kinematic velocity update so if a
  // velocity getter is called again then we don't recompute stuff pointlessly
  // (could in theory clear the transform reads too...)
  action.mState &= ~TransformAction::KinematicVelocity;
}

void RigidBody::ComputeVelocities(Vec3Param oldTranslation, Mat3Param oldRotation, real dt)
{
  // This only makes sense to do on kinematic objects
  if(!mState.IsSet(RigidBodyStates::Kinematic))
    return;

  // Get the current translation and orientation
  WorldTransformation* transform = mPhysicsNode->GetTransform();
  Vec3 worldTranslation = transform->GetWorldTranslation();
  Mat3 worldRotation = transform->GetWorldRotation();

  // Compute the velocities from our position updates
  mVelocity = Physics::Integration::VelocityApproximation(oldTranslation, worldTranslation, dt);
  mAngularVelocity = Physics::Integration::AngularVelocityApproximation(oldRotation, worldRotation, dt);
}

Vec3 RigidBody::InternalGetWorldCenterOfMass() const
{
  return mCenterOfMass;
}

void RigidBody::RecomputeAllMassTerms()
{
  RecomputeCenterMass();
  RecomputeInertiaTensor();
  UpdateWorldInertiaTensor();

  // If this body is not dynamic then clear the mass and inertia
  // (still have to update the center of mass though)
  if(!IsDynamic())
  {
    mInvMass.SetInvMass(0);
    mInvInertia.ClearTensors();
  }
}

void RigidBody::RecomputeCenterMass()
{
  ComputeMass(this);
}

void RigidBody::RecomputeInertiaTensor()
{
  ComputeInertia(this);
}

void RigidBody::UpdateCenterMassFromWorldPosition()
{
  // Recompute the offset of the translation from the center of mass in
  // world-space and use that to recompute the center of mass
  WorldTransformation* transform = mPhysicsNode->GetTransform();
  Vec3 worldPositionOffset = Math::Transform(transform->GetWorldRotation(), mPositionOffset);
  mCenterOfMass = transform->GetWorldTranslation() - worldPositionOffset;
}

void RigidBody::UpdateWorldInertiaTensor()
{
  WorldTransformation* transform = mPhysicsNode->GetTransform();

  Mat3 rotation = transform->GetWorldRotation();
  mInvInertia.ComputeWorldTensor(rotation);
  // If we're in 2d mode then lock the world-space 2d axes.
  // This has to be performed on the world-space inertia tensor since we're locking world-space axes.
  if(mState.IsSet(RigidBodyStates::Mode2D))
    mInvInertia.WorldLock2D();
}

void RigidBody::SetAxisLock(bool xAxis, bool yAxis, bool zAxis)
{
  if(xAxis)
    mInvInertia.LockLocalAxis(0);
  if(yAxis)
    mInvInertia.LockLocalAxis(1);
  if(zAxis)
    mInvInertia.LockLocalAxis(2);
}

void RigidBody::QueueMassUpdate()
{
  // Queuing an update is fine and all, but this body might not be where we
  // actually take into account mass properties. We need to loop up and queue
  // mass updates on all parent bodies until we reach the root. (Might only
  // have to do this on the root, look into it later JoshD questions)

  RigidBody* body = this;
  while(body)
  {
    MassRecomputationAction action(body);

    if(body->IsDynamic())
      return;

    body = body->mParentBody;
  }
}

void RigidBody::UpdateResourcesAndQueue()
{
  mSpace->UpdateModifiedResources();
  mPhysicsNode->UpdateTransformAndMass(mSpace);
}

void RigidBody::UpdateCenterMass(Vec3Param offset)
{
  mCenterOfMass += offset;
  // Clamp the center of mass to avoid getting to bad floating point positions
  mCenterOfMass = Transform::ClampTranslation(GetSpace(), GetOwner(), mCenterOfMass);

  // Bring the position offset into world space so we can update the cached world-space translation
  WorldTransformation* transform = mPhysicsNode->GetTransform();
  Vec3 worldPositionOffset = Math::Transform(transform->GetWorldRotation(), mPositionOffset);
  transform->SetTranslation(mCenterOfMass + worldPositionOffset);
}

void RigidBody::UpdateOrientation(QuatParam offset)
{
  // Use a small angle approximation to update the cached rotation quaternion
  mRotationQuat += offset;
  mRotationQuat.Normalize();

  InternalRecomputeOrientation();
}

void RigidBody::Rotate(QuatParam rotation)
{
  // Perform a full rotation (no small angle approximation)
  mRotationQuat *= rotation;
  mRotationQuat.Normalize();

  InternalRecomputeOrientation();

  GenerateIntegrationUpdate();
}

void RigidBody::InternalRecomputeOrientation()
{
  WorldTransformation* transform = mPhysicsNode->GetTransform();
  transform->SetRotation(Math::ToMatrix3(mRotationQuat));

  // We rotate about the center of mass, not the translation. Therefore, 
  // we have to rotate the position about the center of mass. To do this and
  // avoid numerical issues, we bring the local space offset to world-space with
  // the new orientation and use that to set the translation.
  Mat3 worldRotation = transform->GetWorldRotation();
  Vec3 worldPositionOffset = Math::Transform(worldRotation, mPositionOffset);
  transform->SetTranslation(worldPositionOffset + mCenterOfMass);

  /*//The way to update with deltas. A bit problematic and possible stability issues though.
  offsetQuat = mOrientation * offsetQuat;
  offsetQuat.Normalize();
  Vec3 offsetPos = mPosition - mCenterMass;
  mPosition = offsetQuat.RotatedVector(offsetPos);
  mPosition += mCenterMass;*/
}

void RigidBody::GenerateIntegrationUpdate()
{
  QueueBodyIntegration(mPhysicsNode);

  // Queue all of the colliders for an update (this will
  // try to queue myself twice if this node has a body and collider, but
  // that won't break, just a few extra calculations)
  CompositeColliderList::range range = mColliders.All();
  for(; !range.Empty(); range.PopFront())
  {
    Collider* collider = &range.Front();
    collider->GenerateIntegrationUpdate();
  }

  // If we are kinematic we have to make sure we are in the moving kinematic
  // list so our velocities will be dealt with correctly
  if(GetKinematic())
    mSpace->ActivateKinematic(this);

  // Lastly, we have to make sure to queue updates for all static and kinematic children bodies
  BodyRange bodies = mChildBodies.All();
  for(; !bodies.Empty(); bodies.PopFront())
  {
    RigidBody* childBody = &bodies.Front();
    if(!childBody->IsDynamic())
      childBody->GenerateIntegrationUpdate();
  }
}

void RigidBody::PublishTransform()
{
  Transform* transform = GetOwner()->has(Transform);
  // Always set world-space values as the rigid body has always marked this transform as being in world
  transform->SetWorldTranslationInternal(mPhysicsNode->GetTransform()->GetPublishedTranslation());
  transform->SetWorldRotationInternal(mRotationQuat);
  transform->UpdateAll(TransformUpdateFlags::Physics);
}

void RigidBody::AddBodyEffect(PhysicsEffect* effect)
{
  mEffects.PushBack(effect);
}

void RigidBody::UpdateBodyEffects(real dt, RigidBody* rootBody)
{
  // We might be a static child body who applies forces to our parent.
  // If the passed in body is set, we are applying to our parent, otherwise
  // this was called from the space so set ourself to be the parent
  if(rootBody == nullptr)
    rootBody = this;

  // Apply all of our body effects
  PhysicsEffectList::range bodyEffects = mEffects.All();
  for(; !bodyEffects.Empty(); bodyEffects.PopFront())
  {
    PhysicsEffect& effect = bodyEffects.Front();
    effect.ApplyEffect(rootBody, dt);
  }

  // Apply all of the effects of our composited colliders
  CompositeColliderList::range colliders = mColliders.All();
  for(; !colliders.Empty(); colliders.PopFront())
  {
    PhysicsEffectList::range colliderEffects = colliders.Front().mEffects.All();
    for(; !colliderEffects.Empty(); colliderEffects.PopFront())
    {
      PhysicsEffect& effect = colliderEffects.Front();
      effect.ApplyEffect(rootBody, dt);
    }
  }

  // Apply all effects on static children bodies
  BodyRange bodies = mChildBodies.All();
  for(; !bodies.Empty(); bodies.PopFront())
  {
    RigidBody* body = &bodies.Front();
    if(body->GetStatic())
      body->UpdateBodyEffects(dt, rootBody);
  }
}

void RigidBody::RemoveBodyEffect(PhysicsEffect* effect)
{
  mEffects.Erase(effect);
}

}//namespace Zero
