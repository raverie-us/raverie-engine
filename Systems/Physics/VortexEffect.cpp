///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(VortexEffect, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindGetterSetterProperty(LocalAxis)->ZeroSerialize(true);
  ZilchBindGetterSetterProperty(MinDistance)->ZeroSerialize(real(1));
  ZilchBindGetterSetterProperty(MaxDistance)->ZeroSerialize(real(5));
  ZilchBindGetterSetterProperty(TwistStrengthAtMinDistance)->ZeroSerialize(real(10));
  ZilchBindGetterSetterProperty(TwistStrengthAtMaxDistance)->ZeroSerialize(real(2));
  ZilchBindGetterSetterProperty(InwardStrengthAtMinDistance)->ZeroSerialize(real(2));
  ZilchBindGetterSetterProperty(InwardStrengthAtMaxDistance)->ZeroSerialize(real(10));
  ZilchBindGetterSetterProperty(VortexAxis)->ZeroSerialize(Vec3(0, 1, 0));
  ZilchBindGetterProperty(WorldVortexAxis);
  ZilchBindGetterSetterProperty(InterpolationType)->ZeroSerialize(PhysicsEffectInterpolationType::Linear);
  ZilchBindGetterSetterProperty(EndCondition)->ZeroSerialize(PhysicsEffectEndCondition::ClampToMax);
}

VortexEffect::VortexEffect()
{
  mEffectType = PhysicsEffectType::Vortex;
  // Min/Max need to be initialized to the same values as serialization to avoid any 
  // clamping logic happening during serialization since properties are individually set instead of batch set.
  mMinDistance = 1;
  mMaxDistance = 5;
}

void VortexEffect::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void VortexEffect::DebugDraw()
{
  if(!GetDebugDrawEffect())
    return;

  // Update the world space values
  PreCalculate(0);

  Vec3 point = mWorldVortexCenter;
  Vec3 axis = mWorldVortexAxis;
  // Prevent a zero division from debug drawing to generate an orthonormal basis
  if(axis == Vec3::cZero)
    return;

  // Get an animating time
  real t = GetAnimationTime(GetOwner());
  // Compute the axis and t-values for the min/max distances. To properly animate we have to keep the
  // t-values positive and flip the axis if the strength is negative.
  Vec3 axisMin = axis * (real)Math::Sign(mTwistStrengthAtMinDistance);
  Vec3 axisMax = axis * (real)Math::Sign(mTwistStrengthAtMaxDistance);
  // Scale the inner and outer t-values based upon the speed. Normalize the speed so
  // it's moving at the given linear speed (not a rotational speed)
  real tMin = t * Math::Abs(mTwistStrengthAtMinDistance) / mMinDistance;
  real tMax = t * Math::Abs(mTwistStrengthAtMaxDistance) / mMaxDistance;

  // Draw the inner and outer distance rings
  DrawRing(point, axisMin, mMinDistance, 3, tMin, Color::Red);
  DrawRing(point, axisMax, mMaxDistance, 3, tMax, Color::Green);
  gDebugDraw->Add(Debug::Line(point, point + axis).Color(Color::Black).HeadSize(real(0.1f)));

  // Draw the inward forces

  // Get the scaled min/max force length taking into account distance between the two values
  real minForce = -mInwardStrengthAtMinDistance;
  real maxForce = -mInwardStrengthAtMaxDistance;
  GetPenumbraDebugDrawValues(mMinDistance, mMaxDistance, minForce, maxForce);
  // Animate the inward force vectors (I don't like how this looks so commented out for now)
  //float inwardT = Math::Fractional(t);
  //minForce *= inwardT;
  //maxForce *= inwardT;

  Vec3 axis0, axis1;
  Math::GenerateOrthonormalBasis(mWorldVortexAxis, &axis0, &axis1);

  size_t subDivisions = 4;
  for(size_t i = 0; i < subDivisions; ++i)
  {
    float theta = (i * Math::cTwoPi) / subDivisions;
    Vec3 offset = axis0 * Math::Cos(theta) + axis1 * Math::Sin(theta);

    Vec3 p0 = point + offset * mMinDistance;
    Vec3 p1 = point + offset * mMaxDistance;
    gDebugDraw->Add(Debug::Line(p0, p0 + offset * minForce).HeadSize(0.1f).Color(Color::Red));
    gDebugDraw->Add(Debug::Line(p1, p1 + offset * maxForce).HeadSize(0.1f).Color(Color::Green));
  }
}

void VortexEffect::PreCalculate(real dt)
{
  if(!GetActive())
    return;

  // Update the cached world information
  ComputeVortexInformation();
}

void VortexEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if(!GetActive())
    return;

  Vec3 worldTwistAxis = mWorldVortexAxis;

  Vec3 objPos = obj->GetWorldCenterOfMass();
  Vec3 selfPos = mWorldVortexCenter;

  // Get the vector from the center of the vortex to the center of the rigid
  // body and apply a force perpendicular to that vector and the twist axis.
  Vec3 toBody = objPos - selfPos;

  // We calculate how far away we are from the twist disc's center by projecting
  // the vector from the twist center to rigid body's center onto the vortex's plane
  // (project out the twist axis). The length of this vector is the distance from the vortex center.
  Vec3 projToBody = Math::ProjectOnPlane(toBody, worldTwistAxis);
  real projDistance = projToBody.AttemptNormalize();
  // Compute where in between the two distances we are
  real t = (projDistance - mMinDistance) / (mMaxDistance - mMinDistance);

  // Deal with clamping t based upon our end condition flag.
  // If we are past the max there's a few different things we could do
  if(t > 1)
  {
    // If we're set to not do anything then just return
    if(mVortexStates.IsSet(VortexFlags::NoEffect))
      return;

    // If we clamp to the max distance's values then just clamp the t to pretend we're at the max
    else if(mVortexStates.IsSet(VortexFlags::ClampToMax))
      t = 1;
    // If we're set to continue falloff then don't do anything, it'll happen naturally
  }
  // If we're below the min then just use the min's value.
  else if(t < 0)
    t = 0;
  
  // Calculate the force strengths based upon the computed t-values
  real forceStrength, inwardStrength;
  GetStrengthValues(t, forceStrength, inwardStrength);

  // Since the twist axis and the projected offset vector are perpendicular
  // (by the nature of how we compute the projected offset), there is no
  // need to normalize this vector since it will be unit length
  Vec3 forceAxis = Math::Cross(worldTwistAxis, projToBody);

  // Compute and apply the total force
  Vec3 inwardForce = projToBody * inwardStrength;
  Vec3 sideForce = forceAxis * forceStrength;
  Vec3 totalForce = sideForce - inwardForce;
  obj->ApplyForce(totalForce);
}

void VortexEffect::ComputeVortexInformation()
{
  mWorldVortexCenter = Vec3::cZero;
  mWorldVortexAxis = mVortexAxis;

  // If we use a local axis then transform both the center and axis at the same time (more efficient)
  if(mVortexStates.IsSet(VortexFlags::LocalAxis))
    TransformLocalDirectionAndPointToWorld(mWorldVortexCenter, mWorldVortexAxis);
  // Otherwise only transform the center
  else
    mWorldVortexCenter = TransformLocalPointToWorld(mWorldVortexCenter);
  // Always re-normalize the world axis
  mWorldVortexAxis.AttemptNormalize();
}

void VortexEffect::GetStrengthValues(real t, real& twistForce, real& inwardForce)
{
  real alteredT = t;
  // Update t if we use quadratic interpolation
  if(mInterpolationType == PhysicsEffectInterpolationType::Quadratic)
  {
    alteredT *= t;  
  }

  // Compute the interpolated forces
  twistForce = Math::Lerp(mTwistStrengthAtMinDistance, mTwistStrengthAtMaxDistance, alteredT);
  inwardForce = Math::Lerp(mInwardStrengthAtMinDistance, mInwardStrengthAtMaxDistance, alteredT);
}

bool VortexEffect::GetLocalAxis()
{
  return mVortexStates.IsSet(VortexFlags::LocalAxis);
}

void VortexEffect::SetLocalAxis(bool isLocal)
{
  mVortexStates.SetState(VortexFlags::LocalAxis, isLocal);
  CheckWakeUp();
}

real VortexEffect::GetMinDistance()
{
  return mMinDistance;
}

void VortexEffect::SetMinDistance(real distance)
{
  // Make sure the min distance is not negative
  if(distance < 0)
  {
    DoNotifyWarning("Invalid distance", "Cannot set the min distance to less than zero"
      " or greater than or equal to the max distance");
    distance = real(0);
  }

  // Check to make sure that min < max. If so, clamp the distance to the max it can be.
  if(distance >= mMaxDistance)
  {
    DoNotifyWarning("Invalid distance", "Cannot set the min distance to "
      "greater than or equal to the max distance");
    distance = mMaxDistance - real(.001f);
  }
  mMinDistance = distance;
  // Make sure to wake-up objects if we do that on effect change
  CheckWakeUp();
}

real VortexEffect::GetMaxDistance()
{
  return mMaxDistance;
}

void VortexEffect::SetMaxDistance(real distance)
{
  // Check to make sure that min < max. If so, clamp the distance to the min it can be.
  if(distance <= mMinDistance)
  {
    DoNotifyWarning("Invalid distance", "Cannot set the max distance to "
      "less than or equal to the min distance");
    distance = mMinDistance + real(.001f);
  }
  mMaxDistance = Math::Max(distance, real(0.001f));
  CheckWakeUp();
}

real VortexEffect::GetTwistStrengthAtMinDistance()
{
  return mTwistStrengthAtMinDistance;
}

void VortexEffect::SetTwistStrengthAtMinDistance(real forceStrenth)
{
  mTwistStrengthAtMinDistance = forceStrenth;
  CheckWakeUp();
}

real VortexEffect::GetTwistStrengthAtMaxDistance()
{
  return mTwistStrengthAtMaxDistance;
}

void VortexEffect::SetTwistStrengthAtMaxDistance(real forceStrenth)
{
  mTwistStrengthAtMaxDistance = forceStrenth;
  CheckWakeUp();
}

real VortexEffect::GetInwardStrengthAtMinDistance()
{
  return mInwardStrengthAtMinDistance;
}

void VortexEffect::SetInwardStrengthAtMinDistance(real inwardStrenth)
{
  mInwardStrengthAtMinDistance = inwardStrenth;
  CheckWakeUp();
}

real VortexEffect::GetInwardStrengthAtMaxDistance()
{
  return mInwardStrengthAtMaxDistance;
}

void VortexEffect::SetInwardStrengthAtMaxDistance(real inwardStrenth)
{
  mInwardStrengthAtMaxDistance = inwardStrenth;
  CheckWakeUp();
}

Vec3 VortexEffect::GetVortexAxis()
{
  return mVortexAxis;
}

void VortexEffect::SetVortexAxis(Vec3 axis)
{
  mVortexAxis = axis;
  CheckWakeUp();
}

Vec3 VortexEffect::GetWorldVortexAxis()
{
  ComputeVortexInformation();
  return mWorldVortexAxis;
}

PhysicsEffectEndCondition::Enum VortexEffect::GetEndCondition()
{
  // Convert the bitfield representation of our end condition to the enum value
  PhysicsEffectEndCondition::Enum state;
  if(mVortexStates.IsSet(VortexFlags::ContinueFalloff))
    state = PhysicsEffectEndCondition::ContinueFalloff;
  else if(mVortexStates.IsSet(VortexFlags::NoEffect))
    state = PhysicsEffectEndCondition::NoEffect;
  else if(mVortexStates.IsSet(VortexFlags::ClampToMax))
    state = PhysicsEffectEndCondition::ClampToMax;
  return state;
}

void VortexEffect::SetEndCondition(PhysicsEffectEndCondition::Enum condition)
{
  // Convert the enum representation of our end condition to the bitfield
  mVortexStates.ClearFlag(VortexFlags::ContinueFalloff | VortexFlags::NoEffect | VortexFlags::ClampToMax);
  if(condition == PhysicsEffectEndCondition::NoEffect)
    mVortexStates.SetFlag(VortexFlags::NoEffect);
  else if(condition == PhysicsEffectEndCondition::ContinueFalloff)
    mVortexStates.SetFlag(VortexFlags::ContinueFalloff);
  else if(condition == PhysicsEffectEndCondition::ClampToMax)
    mVortexStates.SetFlag(VortexFlags::ClampToMax);
  // Make sure to wake-up objects if we do that on effect change
  CheckWakeUp();
}

PhysicsEffectInterpolationType::Enum VortexEffect::GetInterpolationType()
{
  return mInterpolationType;
}

void VortexEffect::SetInterpolationType(PhysicsEffectInterpolationType::Enum type)
{
  mInterpolationType = type;
  // Make sure to wake-up objects if we do that on effect change
  CheckWakeUp();
}

}//namespace Zero
