///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

real CalculateObjectMass(Vec3Param dir, RigidBody* body, Vec3Param contactPoint)
{
  Vec3 r = contactPoint - body->GetWorldCenterOfMass();
  Vec3 RxDir = Math::Cross(r, dir);
  Mat3 invInertiaWorld = body->mInvInertia.GetInvWorldTensor();

  real linear = Math::Dot(dir, body->mInvMass.Apply(dir));
  real angular = Math::Dot(RxDir, Math::Transform(invInertiaWorld, RxDir));
  return linear + angular;
}

ZilchDefineType(PhysicsCarWheel, builder, type)
{
  ZeroBindComponent();
  ZeroBindDependency(Transform);

  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(Active);
  ZilchBindGetterSetterProperty(PhysicsCarPath);
  ZilchBindGetterSetterProperty(SpringMinLength);
  ZilchBindGetterSetterProperty(SpringStartLength);
  ZilchBindGetterSetterProperty(SpringMaxLength);
  ZilchBindFieldProperty(mSpringRestLength);
  ZilchBindFieldProperty(mDriveFactor);
  ZilchBindGetterSetterProperty(SteerFactor);
  ZilchBindGetterSetterProperty(Radius);

  ZilchBindFieldProperty(mFrequencyHz);
  ZilchBindFieldProperty(mDampingCompressionRatio);
  ZilchBindFieldProperty(mDampingRelaxationRatio);
  ZilchBindGetterSetterProperty(MaxSpringForce);
  ZilchBindGetterSetterProperty(MaxBrakeStrength);
  ZilchBindGetterSetterProperty(IsDriveWheel);
  ZilchBindGetterSetterProperty(Is2DWheel);
  ZilchBindFieldProperty(mForwardStaticFriction);
  ZilchBindFieldProperty(mForwardDynamicFriction);
  ZilchBindFieldProperty(mSideStaticFriction);
  ZilchBindFieldProperty(mSideDynamicFriction);
  ZilchBindFieldProperty(mGripScalar);
  ZilchBindFieldProperty(mMaxSpringCompressionDistance);
  ZilchBindFieldProperty(mMaxSpringRelaxationDistance);

  ZilchBindGetterSetterProperty(WheelLocalStartPosition);
  ZilchBindGetterSetterProperty(PreRotation);

  ZilchBindGetterSetterProperty(WorldWheelBasis)->Add(new EditorRotationBasis("PhysicsCarWheelBasisGizmo"));
  
  ZilchBindGetter(IsInContact);
  ZilchBindGetter(IsSliding);
  ZilchBindGetter(Rotation);
  ZilchBindGetter(RotationalVelocity);
  ZilchBindGetter(Grip);
  ZilchBindGetter(NormalImpulse);
  ZilchBindGetter(ForwardImpulse);
  ZilchBindGetter(SideImpulse);
  ZilchBindGetter(SpringLength);
  ZilchBindGetter(ContactedObject);
  ZilchBindGetter(ContactPoint);
  ZilchBindGetter(ContactNormal);
  ZilchBindGetter(WorldAxleAxis);
  ZilchBindGetter(WorldForwardAxis);
  ZilchBindGetter(WorldSpringAxis);
  ZilchBindGetter(WorldLinearVelocity);
  ZilchBindGetter(WorldAngularVelocity);

  ZeroBindTag(Tags::Physics);
}

PhysicsCarWheel::PhysicsCarWheel()
{
  mContactedObject = nullptr;
  mCarBody = nullptr;
  mCurrRotation = real(0.0);
  mRotationalVelocity = real(0.0);

  mSpringForce = real(0.0);
  mOldSpringLength = mSpringLength = real(0.0);

  mTotalForwardImpulse = mTotalSideImpulse = real(0.0);
}

void PhysicsCarWheel::Serialize(Serializer& stream)
{
  SerializeNameDefault(mWheelLocalStartPosition, Vec3(0, -1.0, 0));
  SerializeNameDefault(mBodyWheelSpringDir, Vec3(0, 1, 0));
  SerializeNameDefault(mBodyWheelForwardDir, Vec3(1, 0, 0));
  SerializeNameDefault(mBodyWheelAxleDir, Vec3(0, 0, 1));
  SerializeNameDefault(mPreRotation, Math::ToQuaternion(Vec3(1, 0, 0), real(3.14159f * .5f)));

  SerializeNameDefault(mSpringMinLength, real(0.0));
  SerializeNameDefault(mSpringStartLength, real(0.0));
  SerializeNameDefault(mSpringMaxLength, real(1.0));
  SerializeNameDefault(mSpringRestLength, real(0.5));
  SerializeNameDefault(mDriveFactor, real(1.0));
  SerializeNameDefault(mSteerFactor, real(0.5));
  SerializeNameDefault(mRadius, real(0.5));

  SerializeNameDefault(mFrequencyHz, real(20.0));
  SerializeNameDefault(mDampingCompressionRatio, real(.7));
  SerializeNameDefault(mDampingRelaxationRatio, real(.7));
  SerializeNameDefault(mMaxSpringForce, real(800.0));
  SerializeNameDefault(mMaxBrakeStrength, real(400.0));

  SerializeNameDefault(mMaxSpringCompressionDistance, real(.1));
  SerializeNameDefault(mMaxSpringRelaxationDistance, real(.1));

  SerializeNameDefault(mForwardStaticFriction, real(.7));
  SerializeNameDefault(mSideStaticFriction, real(4.0));
  SerializeNameDefault(mForwardDynamicFriction, real(.3));
  SerializeNameDefault(mSideDynamicFriction, real(1.0));
  SerializeNameDefault(mGripScalar, real(1.0));

  stream.SerializeFieldDefault("PhysicsCarPath", mPhysicsCarPath, CogPath());

  u32 mask = CarWheelFlags::IsInContact | CarWheelFlags::InEditor | 
             CarWheelFlags::ChildedWheel | CarWheelFlags::IsSliding;
  SerializeBits(stream, mFlags, CarWheelFlags::Names, mask, ~CarWheelFlags::Is2DWheel);
}

void PhysicsCarWheel::OnAllObjectsCreated(CogInitializer& initializer)
{
  ConnectThisTo(&mPhysicsCarPath, Events::CogPathCogChanged, OnCogPathCogChanged);
  mPhysicsCarPath.SetRelativeTo(GetOwner());
  mPhysicsCarPath.RestoreLink(initializer, GetOwner(), "PhysicsCarPath");

  // Used to update the wheel positions based upon the car
  if(initializer.mSpace->IsEditorMode())
    mFlags.SetFlag(CarWheelFlags::InEditor);
}

void PhysicsCarWheel::OnDestroy(uint flags)
{
  // Remove this wheel from its owning car.
  if(mCarBody)
    mCarBody->RemoveWheelCog(GetOwner());
}

void PhysicsCarWheel::DebugDraw()
{
  if(mCarBody == nullptr)
    return;

  // Recompute this to make sure the debug drawing is correct
  UpdateWheelData();

  Vec3 pos = mWorldWheelStartPosition;

  // Draw the bases
  gDebugDraw->Add(Debug::Line(pos, pos + mWorldWheelSpringDir  * real(2.0)).HeadSize(real(0.2)).Color(Color::Red));
  gDebugDraw->Add(Debug::Line(pos, pos + mWorldWheelForwardDir * real(2.0)).HeadSize(real(0.2)).Color(Color::Blue));
  gDebugDraw->Add(Debug::Line(pos, pos + mWorldWheelAxleDir    * real(2.0)).HeadSize(real(0.2)).Color(Color::Green));
  gDebugDraw->Add(Debug::Cylinder(pos, mWorldWheelAxleDir, real(.01),mRadius));

  Vec3 start = mWorldWheelStartPosition - mWorldWheelSpringDir * mSpringStartLength;
  Vec3 min = mWorldWheelStartPosition - mWorldWheelSpringDir * mSpringMinLength;
  Vec3 max = mWorldWheelStartPosition - mWorldWheelSpringDir * mSpringMaxLength;
  Vec3 rest = mWorldWheelStartPosition - mWorldWheelSpringDir * mSpringRestLength;
  gDebugDraw->Add(Debug::Circle(start, mWorldWheelSpringDir, real(.01)).Color(Color::Red).OnTop(true));
  gDebugDraw->Add(Debug::Circle(min, mWorldWheelSpringDir, real(.01)).Color(Color::Green).OnTop(true));
  gDebugDraw->Add(Debug::Circle(max, mWorldWheelSpringDir, real(.01)).Color(Color::Blue).OnTop(true));
  gDebugDraw->Add(Debug::Circle(rest, mWorldWheelSpringDir, real(.01)).Color(Color::Gold).OnTop(true));
  gDebugDraw->Add(Debug::Line(start, max));

  // Draw the normal force
  real dt = mCarBody->mSpace->mIterationDt;
  Vec3 impulse = mWorldWheelSpringDir * mSpringForce * dt;
  gDebugDraw->Add(Debug::Line(pos, pos + impulse).OnTop(true).Color(Color::Green));

  // Calculate the actual apply position for the frictions

  // Applying the friction impulses at the wheel position is likely to cause
  // the car to flip, so use a coefficient to apply the impulses closer to
  // the center of mass (0 = center of mass, 1 = wheel position)
  Vec3 centerMass = mCarBody->mBody->GetWorldCenterOfMass();
  Vec3 r = mWorldContactPosition - centerMass;
  // Compute the projected vector of the relative position along the contact normal
  Vec3 rPosUp = Math::Dot(r, mWorldContactNormal) * mWorldContactNormal;

  // Compute the r vector for the side friction coef. and apply the impulse
  Vec3 newR = r + (mCarBody->mWheelFrictionSideRollCoef - real(1.0)) * rPosUp;
  Vec3 dir = mWorldAxleTangent;
  pos = newR + centerMass;
  gDebugDraw->Add(Debug::Line(pos, pos + mTotalSideImpulse * dir).OnTop(true).Color(Color::Red));
  // Compute the r vector for the side friction coef. and apply the impulse
  newR = r + (mCarBody->mWheelFrictionFrontRollCoef - real(1.0)) * rPosUp;
  dir = mWorldForwardTangent;
  pos = newR + centerMass;
  gDebugDraw->Add(Debug::Line(pos, pos + mTotalForwardImpulse * dir).OnTop(true).Color(Color::Blue));
}

void PhysicsCarWheel::TransformUpdate(TransformUpdateInfo& info)
{
  // Update the relative points based upon the car position
  // Only do this if they are in editor
  if(!mFlags.IsSet(CarWheelFlags::InEditor))
    return;

  if(info.TransformFlags & TransformUpdateFlags::Translation)
    UpdateLocalPointOnCar();
  if(info.TransformFlags & TransformUpdateFlags::Rotation)
    UpdatePreRotationOnCar();
}

void PhysicsCarWheel::AttachTo(AttachmentInfo& info)
{
  FixChildState(info.Parent);
}

void PhysicsCarWheel::Detached(AttachmentInfo& info)
{
  FixChildState(info.Parent);
}

void PhysicsCarWheel::FixChildState(Cog* parent)
{
  // Remove ourself from the old car if we had one
  if(mCarBody != nullptr)
    mCarBody->RemoveWheelCog(GetOwner());

  // Cache the reference to the new car
  UpdateCachedCarBody();
  // Add ourself to the new car if it exists
  if(mCarBody != nullptr)
    mCarBody->AddWheelCog(GetOwner());
}

PhysicsCar* PhysicsCarWheel::GetCarBody()
{
  Cog* cog = mPhysicsCarPath.GetCog();
  if(cog != nullptr)
    return cog->has(PhysicsCar);
  return nullptr;
}

void PhysicsCarWheel::UpdateCachedCarBody()
{
  mCarBody = GetCarBody();
}

CogPath PhysicsCarWheel::GetPhysicsCarPath()
{
  return mPhysicsCarPath;
}

void PhysicsCarWheel::SetPhysicsCarPath(CogPath& carPath)
{
  mPhysicsCarPath = carPath;
  UpdateCarBodyConnections();
}

void PhysicsCarWheel::OnCogPathCogChanged(Event* e)
{
  UpdateCarBodyConnections();
}

void PhysicsCarWheel::UpdateCarBodyConnections()
{
  // Remove ourself from any old car body if we had one
  if(mCarBody != nullptr)
    mCarBody->RemoveWheelCog(GetOwner());
  
  // Get the new cog we're pointing at
  Cog* cog = mPhysicsCarPath.GetCog();
  // If we have a new car with a PhysicsCar component then add
  // ourself to it's internal list (for runtime) of wheels
  if(cog != nullptr)
  {
    mCarBody = cog->has(PhysicsCar);
    if(mCarBody != nullptr)
      mCarBody->AddWheelCog(GetOwner());
  }
}

void PhysicsCarWheel::UpdateLocalPointOnCar(bool forcedUpdate)
{
  // Only if we have a valid car
  if(mCarBody == nullptr)
    return;

  if(!forcedUpdate && !mFlags.IsSet(CarWheelFlags::InEditor))
    return;

  Transform* transform = GetOwner()->has(Transform);
  Vec3 pos = transform->GetWorldTranslation();
  
  // Have to do this from the transform instead of the cached matrix on the
  // body because hierarchies are currently not guaranteed to be last so our
  // transform update might be called before the car's transform update...
  Mat4 worldTransform = mCarBody->GetOwner()->has(Transform)->GetWorldMatrix();
  mWheelLocalStartPosition = Math::TransformPoint(worldTransform.Inverted(), pos);

  // This is what I'd like to do...
  //mBodyWheelStartPos = mCarBody->mWorldTransform.InverseTransformPoint(pos);
}

void PhysicsCarWheel::UpdatePreRotationOnCar(bool forcedUpdate)
{
  // Only if we have a valid car
  if(mCarBody == nullptr)
    return;

  if(!forcedUpdate && !mFlags.IsSet(CarWheelFlags::InEditor))
    return;

  // Recalculate the pre-rotation of the wheel
  Transform* transform = GetOwner()->has(Transform);
  Quat rot = mCarBody->mBody->GetWorldRotationQuat().Inverted() * transform->GetWorldRotation();

  mPreRotation = rot;
}

void PhysicsCarWheel::UpdateTransformRelativeToParent()
{
  // Only if we have a valid car
  if(mCarBody == nullptr)
    return;

  Transform* transform = GetOwner()->has(Transform);
  Math::DecomposedMatrix4& worldTransform = mCarBody->mWorldTransform;

  transform->SetWorldTranslation(worldTransform.TransformPoint(mWheelLocalStartPosition));
  transform->SetWorldRotation(mCarBody->mBody->GetWorldRotationQuat() * mPreRotation);
  transform->UpdateAll();
}

real PhysicsCarWheel::CalculateRotationalVelocity(Vec3Param dir, Vec3Param relativeVelocity)
{
  // Project the velocity of the wheel onto the direction to determine how fast the wheel is turning
  real projVelCoef = Math::Dot(dir, relativeVelocity);
  // Now use the speed and the radius to compute the actual rotational speed
  return projVelCoef / mRadius;
}

Vec3 PhysicsCarWheel::GetRelativeVelocity()
{
  Vec3 rVel = mCarBody->mBody->GetPointVelocityInternal(mWorldContactPosition);
  if(mContactedObject)
    rVel -= mContactedObject->ComputePointVelocityInternal(mWorldContactPosition);
  return rVel;
}

real PhysicsCarWheel::CalculateImpulse(Vec3Param dir, Vec3Param relativeVelocity)
{
  real top = -Math::Dot(relativeVelocity, dir);
  // We assume that the object we are in contact with has infinite mass
  real bot = CalculateObjectMass(dir, mCarBody->mBody, mWorldContactPosition);

  return top / bot;
}

Quat PhysicsCarWheel::GetWorldWheelBasis()
{
  // Construct the basis from our local directions
  Mat3 localBasisMat3;
  localBasisMat3.SetBasis(0, mBodyWheelSpringDir);
  localBasisMat3.SetBasis(1, mBodyWheelForwardDir);
  localBasisMat3.SetBasis(2, mBodyWheelAxleDir);

  // Bring the basis into world space as a quaternion
  Quat localBasis = Math::ToQuaternion(localBasisMat3);
  Quat worldBasis = localBasis;
  if(mCarBody)
  {
    Quat localToWorld = mCarBody->mBody->GetWorldRotationQuat();
    worldBasis = localToWorld * localBasis;
  }
  return worldBasis;
}

void PhysicsCarWheel::SetWorldWheelBasis(QuatParam worldBasis)
{
  // Convert the basis back into local space
  Quat localBasis = worldBasis;
  if(mCarBody)
  {
    Quat localToWorld = mCarBody->mBody->GetWorldRotationQuat();
    localBasis = localToWorld.Inverted() * localBasis;
  }

  // Extract the basis vectors
  Mat3 localBasisMat3 = Math::ToMatrix3(localBasis);
  mBodyWheelSpringDir = localBasisMat3.GetBasis(0);
  mBodyWheelForwardDir = localBasisMat3.GetBasis(1);
  mBodyWheelAxleDir = localBasisMat3.GetBasis(2);
}

void PhysicsCarWheel::UpdateWheelData()
{
  // Bring the local wheel positions into world space
  Mat3 carWorldRot = mCarBody->mBody->GetWorldRotationMat3();
  Mat3 rot = Math::ToMatrix3(mBodyWheelSpringDir, -mCarBody->mSteerInput * mSteerFactor);
  rot = carWorldRot * rot;

  mWorldWheelSpringDir = Math::Transform(carWorldRot, mBodyWheelSpringDir);
  // Fix up later to deal with the different default rotations
  mWorldWheelForwardDir = Math::Transform(rot, mBodyWheelForwardDir);
  mWorldWheelAxleDir = Math::Transform(rot, mBodyWheelAxleDir);
  
  mWorldWheelStartPosition = mCarBody->mWorldTransform.TransformPoint(mWheelLocalStartPosition);
}

void PhysicsCarWheel::CastWheelPosition()
{
  SetIsInContact(false);
  mContactedObject = nullptr;
  mSpringLength = mSpringMaxLength;

  // Cast out into the world
  CastFilter filter;
  filter.Set(BaseCastFilterFlags::IgnoreGhost);
  CastResults results(5, filter);
  
  // Convert the raycast start, min and max to account for the start t value
  Vec3 rayDir = -mWorldWheelSpringDir;
  Vec3 rayStart = mWorldWheelStartPosition + rayDir * mSpringStartLength;
  real minCast = mSpringMinLength - mSpringStartLength;
  real maxCast = mSpringMaxLength + mRadius - mSpringStartLength;

  Ray worldRay(rayStart, rayDir);
  mCarBody->mSpace->CastRay(worldRay, results);

  // Find anything that we hit withing the max spring distance,
  // plus our radius, that is not us our are parent
  

  mSpringLength = mSpringMaxLength;
  Cog* wheelRoot = GetOwner()->FindRoot();
  Cog* carRoot = GetOwner()->FindRoot();
  uint index = 0;
  for(uint i = 0; i < results.Size(); ++i)
  {
    Cog* resultRoot = results[i].GetObjectHit()->FindRoot();
    // Reject ourself and our car
    if(wheelRoot == resultRoot || carRoot == resultRoot)
      continue;

    // Check to see if this is closer than the previous object
    real t = results[i].GetDistance();
    if(t < maxCast)
    {
      // Clamp the wheel to never go before the min distance, but find the
      // closest object to collide with even if it is closer than min. This
      // is done so that you can set a arbitrary min but not miss collision
      // if there is an object in between the start and min.
      real minDistance = Math::Max(minCast, t - mRadius);
      // Now add back in the start t value since the spring length is expected to
      // start at the wheel start position, not the spring start position
      mSpringLength = minDistance + mSpringStartLength;

      mContactedObject = results[i].GetCollider();
      maxCast = t;
      index = i;
    }
  }

  // If we hit an object, save where we hit it
  if(mContactedObject != nullptr)
  {
    mWorldContactPosition = results[index].GetWorldPosition();
    mWorldContactNormal = results[index].GetNormal();
    SetIsInContact(true);
  }
  else
  {
    mWorldContactPosition.ZeroOut();
    mWorldContactNormal.ZeroOut();
  }
}

void PhysicsCarWheel::CalculateSpringForce()
{
  // If we are not in contact, no spring forces to calculate
  if(!GetIsInContact())
    return;

  // Compute the mass that this spring sees (basically the mass of the impulse for the wheel)
  real effectiveMass = real(1.0) / CalculateObjectMass(mWorldWheelSpringDir, mCarBody->mBody, mWorldContactPosition);
  // Convert the frequency to the spring coefficient k (k = mass*frequency^2)
  real springCoef = effectiveMass * mFrequencyHz * mFrequencyHz;
  // Calculate how far away we are from the rest length of the spring
  // (this is -x since x would be length - restLength)
  real lengthOffset = mSpringRestLength - mSpringLength;
  // Calculate the force of the spring (F = -kx)
  mSpringForce = springCoef * lengthOffset;

  // Calculate the dampening for the spring (F = -cv)

  // Not sure what exactly this is doing, but it attempts to get the
  // relative velocity of the wheel relative to the spring
  real normDotSpring = Math::Dot(mWorldContactNormal, mWorldWheelSpringDir);
  real velRelativeToSpring;
  // In the case that the wheel and the ground are almost perpendicular,
  // just set the relative velocity to 0 to avoid a zero division
  if(normDotSpring <= real(.1))
    velRelativeToSpring = real(0.0);
  else
  {
    Vec3 relVel = GetRelativeVelocity();
    real projVelCoef = Math::Dot(mWorldContactNormal, relVel);
    velRelativeToSpring = projVelCoef / normDotSpring;
  }

  real dampeningRatio;
  // Determine if the spring is relaxing or compressing
  if(velRelativeToSpring < real(0.0))
    dampeningRatio = mDampingCompressionRatio;
  else
    dampeningRatio = mDampingRelaxationRatio;
  // Convert the dampening ratio to the dampening coefficient c (c = 2*mass*ratio*frequency)
  real dampeningCoef = real(2.0) * effectiveMass * dampeningRatio * mFrequencyHz;
  // Calculate the dampening force (F = -cv)
  mSpringForce -= dampeningCoef * velRelativeToSpring;

  // The spring can only push apart, not pull together (like a contact)
  // so a wheel won't stick to the surface
  mSpringForce = Math::Clamp(mSpringForce, real(0.0), mMaxSpringForce);
}

void PhysicsCarWheel::ApplySpringForce(real dt)
{
  if(!GetIsInContact())
    return;

  // Calculate the impulse along the spring direction
  Vec3 impulse = mWorldWheelSpringDir * mSpringForce * dt;
  // Apply the impulse at the wheel position
  mCarBody->mBody->ApplyImpulseAtPoint(impulse, mWorldContactPosition);
}

/// Helper struct to make it easier to pass around a lot more data
struct ConstraintInfo
{
  ConstraintInfo(PhysicsCarWheel* wheel, real dt)
  {
    mNormalForce = wheel->mSpringForce;
    mDt = dt;
    mClampToStatic = false;
    mCurrentGrip = nullptr;
    mGripScalar = wheel->mGripScalar * wheel->mCarBody->mGripScalar;
  }

  void SetRs(PhysicsCarWheel* wheel, real rollCoef)
  {
    Vec3 centerMass = wheel->mCarBody->mBody->GetWorldCenterOfMass();
    Vec3 r = wheel->mWorldContactPosition - centerMass;
    // Compute the projected vector of the relative position along the contact normal
    Vec3 rPosUp = Math::Dot(r, wheel->mWorldContactNormal) * wheel->mWorldContactNormal;
    mR1 = wheel->mContactedObject->GetRigidBodyCenterOfMass() - wheel->mWorldContactPosition;
    mR2 = r + (rollCoef - real(1.0)) * rPosUp;
  }

  real GetStaticFrictionImpulse()
  {
    return mNormalForce * mStaticFriction * mGripScalar * mDt;
  }

  real GetDynamicFrictionImpulse()
  {
    return mNormalForce * mDynamicFriction * mGripScalar * mDt;
  }

  real mTotalImpulse;
  real mMaxForce;
  real mTargetSpeed;
  real mDt;
  real mStaticFriction;
  real mDynamicFriction;
  real mNormalForce;
  real mGripScalar;
  real mClampToStatic;
  real* mCurrentGrip;
  Vec3 mR1;
  Vec3 mR2;
};

void PhysicsCarWheel::BeginIteration()
{
  // We're starting over, so clear out the total impulses
  // (could warm start this, but not important to have that convergence)
  mTotalForwardImpulse = mTotalSideImpulse = real(0.0f);
  // Clear out the grips as well
  mForwardGrip = mSideGrip = real(0.0);

  // Project the forward and axle direction onto the contact plane
  mWorldForwardTangent = Math::ProjectOnPlane(mWorldWheelForwardDir, mWorldContactNormal);
  mWorldForwardTangent.AttemptNormalize();
  mWorldAxleTangent = Math::Cross(mWorldForwardTangent, mWorldContactNormal);
}

real PhysicsCarWheel::InternalFrictionCalculation(Vec3Param dir, ConstraintInfo& info)
{
  real dt = info.mDt;
  RigidBody* otherBody = mContactedObject->GetActiveBody();
  RigidBody* body = mCarBody->mBody;

  // The jacobian could also be computed only once (see comment below)
  Physics::Jacobian jacobian;
  jacobian.Set(-dir, -Math::Cross(info.mR1, dir), dir, Math::Cross(info.mR2, dir));

  // This could be optimized by taking this entire mass calculation and only
  // doing it once per logic update, however due to having to store the memory
  // somewhere and not wanting to permanently increase the size of this component
  // it is re-calculated. (Maybe later add some sort of molecule like joints
  // that the car holds onto in its stack)
  Physics::JointMass masses;
  // Pass in null instead of the other body because we're treating the other body
  // as "kinematic" so we don't want its mass since it's infinite
  Physics::JointHelpers::GetMasses(nullptr, body, masses);
  real constraintMass = jacobian.ComputeMass(masses);
  // Prevent zero divisions (most likely from 2d mode)
  if(constraintMass == real(0.0))
    return constraintMass;

  // Compute the impulse value
  Physics::JointVelocity velocities;
  Physics::JointHelpers::GetVelocities(otherBody, body, velocities);
  real jv = jacobian.ComputeJV(velocities);
  real b = -info.mTargetSpeed;
  real lambda = -(jv + b) / constraintMass;

  // Clamp the total impulse
  real maxImpulse = info.mMaxForce * dt;
  real oldImpulse = info.mTotalImpulse;
  info.mTotalImpulse = Math::Clamp(oldImpulse + lambda, -maxImpulse, maxImpulse);

  // Now clamp to the friction cone
  real maxStaticImpulse = info.GetStaticFrictionImpulse();

  // If the magnitude of our friction is greater than the max static
  // impulse, then we slip into dynamic friction
  real absTotalImpulse = Math::Abs(info.mTotalImpulse);
  if(absTotalImpulse > maxStaticImpulse && absTotalImpulse > real(0.01))
  {
    // Grab the dynamic friction bounds
    real maxDynamicImpulse = info.GetDynamicFrictionImpulse();
    // If we clamp to static for some reason (anti-lock brakes),
    // then change our max impulse to be static friction
    if(info.mClampToStatic)
      maxDynamicImpulse = maxStaticImpulse;
    // Compute how much grip we have (the ratio of how
    // much we have to cut back the actual impulse by)
    real currentGrip = maxDynamicImpulse / absTotalImpulse;
    // Normalize the total impulse to the max value
    info.mTotalImpulse *= currentGrip;

    // Store the grip, if we clamp to static though then we still have full grip
    *info.mCurrentGrip = currentGrip;
    if(info.mClampToStatic)
      *info.mCurrentGrip = real(1.0);
  }
  // If we didn't clamp, then we have full grip
  else
    *info.mCurrentGrip = real(1.0);

  // Now compute the actual impulse (the amount of
  // impulse we actually got to apply after clamping)
  real impulse = info.mTotalImpulse - oldImpulse;

  body->ApplyConstraintImpulse(jacobian.Linear[1] * impulse,
    jacobian.Angular[1] * impulse);

  // Now return the new total impulse so the caller can cache it
  return info.mTotalImpulse;
}

void PhysicsCarWheel::SolveFrictionImpulse(real dt)
{
  // If we weren't in contact then we can't have friction
  if(!GetIsInContact())
    return;

  ConstraintInfo info(this, dt);
  info.SetRs(this, mCarBody->mWheelFrictionFrontRollCoef);
  info.mMaxForce = mCarBody->mMaxTorque;
  info.mStaticFriction = mForwardStaticFriction;
  info.mDynamicFriction = mForwardDynamicFriction;
  info.mCurrentGrip = &mForwardGrip;

  // If our brakes have input (brakes take precedence over gas)
  if(mCarBody->mBrakeInput != real(0.0))
  {
    info.mClampToStatic = mCarBody->mAntiLockBrakes;
    info.mTargetSpeed = 0;
    info.mMaxForce = mMaxBrakeStrength * mCarBody->mBrakeInput;
    info.mTotalImpulse = mTotalForwardImpulse;

    mTotalForwardImpulse = InternalFrictionCalculation(mWorldForwardTangent, info);
  }
  else if(GetIsDriveWheel() && mCarBody->mGasInput != real(0.0))
  {
    info.mClampToStatic = mCarBody->mTorqueGovernor;
    info.mTargetSpeed = mCarBody->mMaxSpeed * mCarBody->mGasInput;
    info.mMaxForce = mCarBody->mMaxTorque;
    info.mTotalImpulse = mTotalForwardImpulse;

    mTotalForwardImpulse = InternalFrictionCalculation(mWorldForwardTangent, info);
  }
  // If there was no input, we're just coasting which for now gives us full grip
  else
    mForwardGrip = real(1.0);

  // If we're a 2d wheel, don't solve since there's nothing to do
  if(GetIs2DWheel())
  {
    mTotalSideImpulse = real(0.0);
    return;
  }

  info.SetRs(this, mCarBody->mWheelFrictionSideRollCoef);
  info.mClampToStatic = false;
  info.mMaxForce = Math::PositiveMax();
  info.mTargetSpeed = 0;
  info.mTotalImpulse = mTotalSideImpulse;
  info.mStaticFriction = mSideStaticFriction;
  info.mDynamicFriction = mSideDynamicFriction;
  info.mCurrentGrip = &mSideGrip;
  mTotalSideImpulse = InternalFrictionCalculation(mWorldAxleTangent, info);
}

void PhysicsCarWheel::FinishedIteration(real dt)
{
  // If we're not in contact, we don't want to compute the speed of the wheel the normal way
  if(!GetIsInContact())
  {
    // Normally, we want to target 0, but if we have gas target the max speed
    real targetSpeed = real(0.0);
    if(mCarBody->mGasInput != real(0.0))
      targetSpeed = mCarBody->mMaxSpeed * mCarBody->mGasInput;
    mRotationalVelocity = Math::Lerp(mRotationalVelocity, targetSpeed, real(.01));
    return;
  }

  // Now compute the rotation of the wheel
  Vec3 relVel = GetRelativeVelocity();

  // If the brakes are on and we don't have anti-lock brakes, just stop rotation
  if(mCarBody->mBrakeInput != real(0.0) && !mCarBody->mAntiLockBrakes)
    mRotationalVelocity = real(0.0);
  // Otherwise calculate the point velocity of the wheel to get the rotational velocity
  else
    mRotationalVelocity = CalculateRotationalVelocity(mWorldForwardTangent, relVel);
}

void PhysicsCarWheel::UpdateWheelTransform(real dt)
{
  mCurrRotation += mRotationalVelocity * dt;

  // Calculate the rotation of the wheel about the axle
  Quat rollRot = Math::ToQuaternion(mBodyWheelAxleDir, mCurrRotation);
  // Calculate the rotation of the wheel about the spring dir
  Mat3 rotMat;
  rotMat = Math::ToMatrix3(mBodyWheelSpringDir, -mCarBody->mSteerInput * mSteerFactor);
  Quat rotQuat = Math::ToQuaternion(rotMat);

  Transform* transform = GetOwner()->has(Transform);
  real springLength = mSpringLength;

  // If we're in world then use the graphical max distance
  // (no reason to not do this with children, I just don't care right now...)
  if(mFlags.IsSet(CarWheelFlags::ChildedWheel) == false)
  {
    real graphicalLength = mSpringLength;
    real moveLength = mSpringLength - mOldSpringLength;
    if(moveLength > real(0.0))
    {
      if(moveLength > mMaxSpringRelaxationDistance)
        graphicalLength = mOldSpringLength + mMaxSpringRelaxationDistance;
    }
    else
    {
      if(-moveLength > mMaxSpringRelaxationDistance)
        graphicalLength = mOldSpringLength - mMaxSpringRelaxationDistance;
    }
    mOldSpringLength = graphicalLength;
    springLength = graphicalLength;
  }

  // Need to update the start pos of the wheel because of integration
  mWorldWheelStartPosition = mCarBody->mWorldTransform.TransformPoint(mWheelLocalStartPosition);
  // Set the world translation (so children work as well, have to set world instead
  // of local in children case because we may be n levels deep in the hierarchy).
  Vec3 worldPos = mWorldWheelStartPosition + springLength * -mWorldWheelSpringDir;
  transform->SetWorldTranslation(worldPos);
  // Set world rotation for the same reason as listed above
  Quat carRot = mCarBody->mBody->GetWorldRotationQuat();
  //if(mFlags.IsSet(CarWheelFlags::ChildedWheel) == false)
  {
    rotQuat = carRot * rotQuat * rollRot * mPreRotation;
    transform->SetWorldRotation(rotQuat);
  }
  //else
  //{
  //  //if we're a child then don't include the car's rotation
  //  rotQuat = rotQuat * rollRot * mPreRotation;
  //  transform->SetRotation(rotQuat);
  //}
  

  // Send the update event for transform
  transform->UpdateAll();
}

bool PhysicsCarWheel::GetActive()
{
  return mFlags.IsSet(CarWheelFlags::Active);
}

void PhysicsCarWheel::SetActive(bool active)
{
  mFlags.SetState(CarWheelFlags::Active, active);
}

Vec3 PhysicsCarWheel::GetWheelLocalStartPosition()
{
  return mWheelLocalStartPosition;
}

void PhysicsCarWheel::SetWheelLocalStartPosition(Vec3Param localStartPosition)
{
  mWheelLocalStartPosition = localStartPosition;

  if(!mFlags.IsSet(CarWheelFlags::InEditor))
    return;

  if(mCarBody == nullptr)
    return;

  // Move the wheel to the correct position and orientation on the car
  UpdateWheelData();
  UpdateTransformRelativeToParent();
}

real PhysicsCarWheel::GetSteerFactor()
{
  return mSteerFactor;
}

void PhysicsCarWheel::SetSteerFactor(real steerFactor)
{
  mSteerFactor = steerFactor;
}

real PhysicsCarWheel::GetMaxBrakeStrength()
{
  return mMaxBrakeStrength;
}

void PhysicsCarWheel::SetMaxBrakeStrength(real maxStrength)
{
  mMaxBrakeStrength = Math::Max(maxStrength, real(0.0));
}

Quat PhysicsCarWheel::GetPreRotation()
{
  return mPreRotation;
}

void PhysicsCarWheel::SetPreRotation(QuatParam quat)
{
  mPreRotation = quat;
  UpdateTransformRelativeToParent();
}

real PhysicsCarWheel::GetRadius()
{
  return mRadius;
}

void PhysicsCarWheel::SetRadius(real radius)
{
  mRadius = Math::Max(radius, real(0.001));
}

real PhysicsCarWheel::GetSpringMinLength()
{
  return mSpringMinLength;
}

void PhysicsCarWheel::SetSpringMinLength(real value)
{
  if(value > mSpringMaxLength)
  {
    DoNotifyException("Invalid spring min length.",
                    "The spring min length cannot be larger than the spring max length value.");
    mSpringMinLength = mSpringMaxLength;
  }
  else
    mSpringMinLength = value;
}

real PhysicsCarWheel::GetSpringStartLength()
{
  return mSpringStartLength;
}

void PhysicsCarWheel::SetSpringStartLength(real value)
{
  mSpringStartLength = value;
}

real PhysicsCarWheel::GetSpringMaxLength()
{
  return mSpringMaxLength;
}

void PhysicsCarWheel::SetSpringMaxLength(real value)
{
  if(value < mSpringMinLength)
  {
    DoNotifyException("Invalid spring max length.",
      "The spring max length cannot be smaller than the spring min length.");
    mSpringMaxLength = mSpringMinLength;
  }
  else
    mSpringMaxLength = value;
}

real PhysicsCarWheel::GetMaxSpringForce()
{
  return mMaxSpringForce;
}

void PhysicsCarWheel::SetMaxSpringForce(real maxForce)
{
  mMaxSpringForce = Math::Max(maxForce, real(0.0));
}

bool PhysicsCarWheel::GetIsInContact()
{
  return mFlags.IsSet(CarWheelFlags::IsInContact);
}

void PhysicsCarWheel::SetIsInContact(bool state)
{
  mFlags.SetState(CarWheelFlags::IsInContact, state);
}

bool PhysicsCarWheel::GetIsSliding()
{
  return mFlags.IsSet(CarWheelFlags::IsSliding);
}

void PhysicsCarWheel::SetIsSliding(bool state)
{
  mFlags.SetState(CarWheelFlags::IsSliding, state);
}

bool PhysicsCarWheel::GetIsDriveWheel()
{
  return mFlags.IsSet(CarWheelFlags::IsDriveWheel);
}

void PhysicsCarWheel::SetIsDriveWheel(bool state)
{
  mFlags.SetState(CarWheelFlags::IsDriveWheel, state);
}

bool PhysicsCarWheel::GetIs2DWheel()
{
  return mFlags.IsSet(CarWheelFlags::Is2DWheel);
}

void PhysicsCarWheel::SetIs2DWheel(bool state)
{
  mFlags.SetState(CarWheelFlags::Is2DWheel, state);
}

real PhysicsCarWheel::GetRotation()
{
  return mCurrRotation;
}

real PhysicsCarWheel::GetRotationalVelocity()
{
  return mRotationalVelocity;
}

real PhysicsCarWheel::GetGrip()
{
  return Math::Min(mForwardGrip, mSideGrip);
}

real PhysicsCarWheel::GetNormalImpulse()
{
  return mSpringForce;
}

real PhysicsCarWheel::GetForwardImpulse()
{
  return mTotalForwardImpulse;
}

real PhysicsCarWheel::GetSideImpulse()
{
  return mTotalSideImpulse;
}

real PhysicsCarWheel::GetSpringLength()
{
  return mSpringLength;
}

Cog* PhysicsCarWheel::GetContactedObject()
{
  if(mContactedObject)
    return mContactedObject->GetOwner();
  return nullptr;
}

Vec3 PhysicsCarWheel::GetContactPoint()
{
  return mWorldContactPosition;
}

Vec3 PhysicsCarWheel::GetContactNormal()
{
  return mWorldContactNormal;
}

Vec3 PhysicsCarWheel::GetWorldAxleAxis()
{
  return mWorldWheelAxleDir;
}

Vec3 PhysicsCarWheel::GetWorldForwardAxis()
{
  return mWorldWheelForwardDir;
}

Vec3 PhysicsCarWheel::GetWorldSpringAxis()
{
  return mWorldWheelSpringDir;
}

Vec3 PhysicsCarWheel::GetWorldLinearVelocity()
{
  if(mCarBody == nullptr)
    return Vec3::cZero;

  Transform* transform = GetOwner()->has(Transform);
  Vec3 worldPos = transform->GetWorldTranslation();

  RigidBody* body = mCarBody->mBody;
  return body->GetPointVelocity(worldPos) + body->mVelocity;
}

Vec3 PhysicsCarWheel::GetWorldAngularVelocity()
{
  return GetWorldAxleAxis() * GetRotationalVelocity();
}

}//namespace Zero
