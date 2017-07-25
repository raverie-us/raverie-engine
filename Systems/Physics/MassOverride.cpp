///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(MassOverride, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZeroBindDependency(Cog);
  ZeroBindDependency(RigidBody);

  ZilchBindGetterSetterProperty(Active)->ZeroSerialize(true);
  ZilchBindGetterSetter(InverseMass)->ZeroSerialize(real(1));
  // @MetaSerialization: Property needs to not cause rescans
  ZilchBindGetterSetterProperty(Mass);
  ZilchBindGetterSetter(LocalInverseInertiaTensor)->ZeroSerialize(Mat3::cIdentity);
  ZilchBindGetterSetter(LocalCenterOfMass)->ZeroSerialize(Vec3::cZero);

  // Runtime modifications (not serialized)
  ZilchBindGetterSetter(AutoComputeInertia);
  ZilchBindGetterSetter(AutoComputeCenterOfMass);
  ZilchBindMethodProperty(RecomputeMass);

  ZeroBindTag(Tags::Physics);
}

MassOverride::MassOverride()
{
  mFlags.SetFlag(MassOverrideStates::AutoComputeInertia | MassOverrideStates::AutoComputeCenterOfMass);
  mLocalCenterOfMass.ZeroOut();
}

void MassOverride::Serialize(Serializer& stream)
{
  MetaSerializeProperties(this, stream);
}
 
void MassOverride::Initialize(CogInitializer& initializer)
{
  // Mark that we've now been serialized so setters can properly update
  mFlags.SetFlag(MassOverrideStates::Serialized);

  if(initializer.Flags & CreationFlags::DynamicallyAdded)
    RecomputeMass();
}

bool MassOverride::IsSerialized()
{
  return mFlags.IsSet(MassOverrideStates::Serialized);
}

bool MassOverride::GetActive()
{
  return mFlags.IsSet(MassOverrideStates::Active);
}

void MassOverride::SetActive(bool state)
{
  mFlags.SetState(MassOverrideStates::Active, state);
  QueueUpdate();
}

real MassOverride::GetInverseMass()
{
  return mMassOverride.GetScalarInvMass();
}

void MassOverride::SetInverseMass(real invMass)
{
  // Just always mark the inertia and center of mass as being changed for now
  if(OperationQueue::IsListeningForSideEffects())
  {
    OperationQueue::RegisterSideEffect(this, "LocalInverseInertiaTensor", GetLocalInverseInertiaTensor());
    OperationQueue::RegisterSideEffect(this, "LocalCenterOfMass", GetLocalCenterOfMass());
  }

  // If we auto compute inertia and this is a run-time set (not serialization)
  // then update the inertia as well as the mass.
  if(GetAutoComputeInertia() && IsSerialized())
    UpdateMassAndInertia(invMass);
  else
    SetInverseMassInternal(invMass);
  
  QueueUpdate();
}

void MassOverride::SetInverseMassInternal(real invMass)
{
  mMassOverride.SetInvMass(invMass);
}

real MassOverride::GetMass()
{
  real invMass = GetInverseMass();
  if(invMass != 0)
    invMass = real(1.0) / invMass;
  return invMass;
}

void MassOverride::SetMass(real mass)
{
  // Mark all side effects
  if(OperationQueue::IsListeningForSideEffects())
  {
    OperationQueue::RegisterSideEffect(this, "LocalInverseInertiaTensor", GetLocalInverseInertiaTensor());
    OperationQueue::RegisterSideEffect(this, "LocalCenterOfMass", GetLocalCenterOfMass());
    OperationQueue::RegisterSideEffect(this, "InverseMass", GetInverseMass());
  }

  // Clamp the mass (ensures this isn't zero)
  mass = ClampMassTerm(mass);
  real invMass = real(1.0) / mass;
  SetInverseMass(invMass);
}

Mat3 MassOverride::GetLocalInverseInertiaTensor()
{
  return mInertiaOverride.GetInvModelTensor();
}

void MassOverride::SetLocalInverseInertiaTensor(Mat3Param invInertia)
{
  SetLocalInverseInertiaTensorInternal(invInertia);
  QueueUpdate();
}

void MassOverride::SetLocalInverseInertiaTensorInternal(Mat3Param invInertia)
{
  mInertiaOverride.SetInvTensorModel(invInertia);
}

Vec3 MassOverride::GetLocalCenterOfMass()
{
  return mLocalCenterOfMass;
}

void MassOverride::SetLocalCenterOfMass(Vec3Param localCenterOfMass)
{
  mLocalCenterOfMass = localCenterOfMass;
  mFlags.ClearFlag(MassOverrideStates::AutoComputeCenterOfMass);
  QueueUpdate();
}

bool MassOverride::GetAutoComputeCenterOfMass()
{
  return mFlags.IsSet(MassOverrideStates::AutoComputeCenterOfMass);
}

void MassOverride::SetAutoComputeCenterOfMass(bool autoCompute)
{
  mFlags.SetState(MassOverrideStates::AutoComputeCenterOfMass, autoCompute);
  QueueUpdate();
}

bool MassOverride::GetAutoComputeInertia()
{
  return mFlags.IsSet(MassOverrideStates::AutoComputeInertia);
}

void MassOverride::SetAutoComputeInertia(bool autoCompute)
{
  mFlags.SetState(MassOverrideStates::AutoComputeInertia, autoCompute);
}

void MassOverride::RecomputeMass()
{
  // Mark all side effects
  if(OperationQueue::IsListeningForSideEffects())
  {
    OperationQueue::RegisterSideEffect(this, "Mass", GetMass());
    OperationQueue::RegisterSideEffect(this, "LocalInverseInertiaTensor", GetLocalInverseInertiaTensor());
    OperationQueue::RegisterSideEffect(this, "LocalCenterOfMass", GetLocalCenterOfMass());
    OperationQueue::RegisterSideEffect(this, "InverseMass", GetInverseMass());
  }

  RigidBody* body = GetOwner()->has(RigidBody);
  // Avoid recomputing the mass computing a zero mass because the object is static or kinematic
  if(body->GetStatic() || body->GetKinematic())
  {
    DoNotifyWarning("Can't update mass", "The rigid body is either kinematic or static. "
      "Please make the body dynamic in order recompute the current overridden mass.");
    return;
  }

  body->GetSpace()->MarkModified();

  // Recomputing mass will notice the mass override and use our current values,
  // to get around this we need to temporarily mark ourself as not active
  uint tempFlags = mFlags.Field;
  mFlags.Clear();

  body->RecomputeAllMassTerms();
  mMassOverride = body->mInvMass;
  mInertiaOverride = body->mInvInertia;

  mFlags.U32Field = tempFlags;
}

real MassOverride::ClampMassTerm(real value)
{
  // Check for too large of values, without this we can get #INF which
  // will cause several issues during serialization
  if(value > 9e10f)
  {
    DoNotifyWarning("Invalid mass", "Mass is too large");
    value = 9e10;
  }
  else if(value < 9e-10f)
  {
    DoNotifyWarning("Invalid mass", "Mass is too small");
    value = 1e-10;
  }
  return value;
}

void MassOverride::UpdateMassAndInertia(real invMass)
{
  // Clamp extreme mass terms
  invMass = ClampMassTerm(invMass);

  real oldInvMass = GetInverseMass();
  real ratio = invMass;
  if(oldInvMass != real(0.0))
    ratio /= oldInvMass;

  Mat3 oldInvInertiaTensor = GetLocalInverseInertiaTensor();

  SetInverseMassInternal(invMass);
  SetLocalInverseInertiaTensorInternal(ratio * oldInvInertiaTensor);
}

void MassOverride::QueueUpdate()
{
  // Deal with meta serialization being called as opposed to run-time setters
  if(!IsSerialized())
    return;

  RigidBody* body = GetOwner()->has(RigidBody);
  body->QueueMassUpdate();
}

}//namespace Zero
