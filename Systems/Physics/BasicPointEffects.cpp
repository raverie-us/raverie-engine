///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2013-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------PointForceEffect
ZilchDefineType(BasicPointEffect, builder, type)
{
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(MinDistance)->ZeroSerialize(real(1));
  ZilchBindGetterSetterProperty(MaxDistance)->ZeroSerialize(real(5));
  ZilchBindGetterSetterProperty(StrengthAtMin)->ZeroSerialize(real(1));
  ZilchBindGetterSetterProperty(StrengthAtMax)->ZeroSerialize(real(5));
  ZilchBindGetterSetterProperty(LocalPositionOffset)->ZeroSerialize(Vec3::cZero);
  ZilchBindGetterSetterProperty(InterpolationType)->ZeroSerialize(PhysicsEffectInterpolationType::Linear);
  ZilchBindGetterSetterProperty(EndCondition)->ZeroSerialize(PhysicsEffectEndCondition::ClampToMax);
}

BasicPointEffect::BasicPointEffect()
{
  // Min/Max need to be initialized to the same values as serialization to avoid any 
  // clamping logic happening during serialization since properties are individually set instead of batch set.
  mMinDistance = 1;
  mMaxDistance = 5;
  mMinStrength = 0;
  mMaxStrength = 0;
  mOutwardDirectionScalar = -1;
  mLocalPositionOffset = Vec3::cZero;
  mWorldPoint = Vec3::cZero;
}

void BasicPointEffect::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void BasicPointEffect::DebugDraw()
{
  if(!GetDebugDrawEffect())
    return;

  // Update the world-space application point. This could be called via script
  // so to make sure this is accurate we need to recompute it.
  ComputeApplicationPoint();

  // Draw the inner and outer distance values
  real minDistance = GetMinDistance();
  real maxDistance = GetMaxDistance();
  gDebugDraw->Add(Debug::Sphere(mWorldPoint, minDistance).Color(Color::Red));
  gDebugDraw->Add(Debug::Sphere(mWorldPoint, maxDistance).Color(Color::Green));

  // Get a signed normalize force for both the min and max distances
  real minForce = GetForceStrenthAtDistance(mMinDistance);
  real maxForce = GetForceStrenthAtDistance(mMaxDistance);
  // Get the scaled min/max force length taking into account distance between the two values
  GetPenumbraDebugDrawValues(minDistance, maxDistance, minForce, maxForce);

  // Draw arrows given spherical coordinates
  real phiDegrees[] = {0, 45, 90, 135, 180};
  real thetaDegrees[] = {0, 90, 180, 270};

  size_t phiCount = sizeof(phiDegrees) / sizeof(real);
  size_t thetaCount = sizeof(thetaDegrees) / sizeof(real);
  for(size_t phiIndex = 0; phiIndex < phiCount; ++phiIndex)
  {
    real phi = Math::DegToRad(phiDegrees[phiIndex]);
    real sinPhi = Math::Sin(phi);
    real cosPhi = Math::Cos(phi);

    for(size_t thetaIndex = 0; thetaIndex < thetaCount; ++thetaIndex)
    {
      real theta = Math::DegToRad(thetaDegrees[thetaIndex]);
      real sinTheta = Math::Sin(theta);
      real cosTheta = Math::Cos(theta);

      Vec3 dir(sinPhi * cosTheta, cosPhi, sinPhi * sinTheta);

      Vec3 minPos = mWorldPoint + dir* minDistance;
      Vec3 maxPos = mWorldPoint + dir* maxDistance;
      gDebugDraw->Add(Debug::Line(minPos, minPos + dir* minForce).HeadSize(0.1f).Color(Color::Red));
      gDebugDraw->Add(Debug::Line(maxPos, maxPos + dir* maxForce).HeadSize(0.1f).Color(Color::Green));
    }
  }
}

void BasicPointEffect::PreCalculate(real dt)
{
  if(!GetActive())
    return;

  ComputeApplicationPoint();
}

void BasicPointEffect::ComputeApplicationPoint()
{
  // Cache the world space point (common in all operations)
  mWorldPoint = TransformLocalPointToWorld(mLocalPositionOffset);
}

float BasicPointEffect::GetForceStrenthAtDistance(float distance)
{
  // If we're beyond the max distance and we're set to have no effect beyond it then
  // return 0 (maybe rework this so we can return false or something later)
  if(distance > mMaxDistance && mPointFlags.IsSet(PointFlags::NoEffect))
    return 0;

  // Otherwise, we will always apply a force, just have to figure out how much.
  // If we are below the min distance, we always use the min strength value.
  if(distance < mMinDistance)
    return mMinStrength * mOutwardDirectionScalar;

  // Otherwise, we want to interpolate between the min and the max with whatever interpolation type.
  
  // Calculate the interpolation time
  real t = (distance - mMinDistance) / (mMaxDistance - mMinDistance);
  // If we clamp to the max distance, make sure that we clamp the t value to
  // [0,1], otherwise we just interpolate as normal (this can go negative)
  if(mPointFlags.IsSet(PointFlags::ClampToMax))
    t = Math::Clamp(t, real(0), real(1));
  // Compute the force strength by interpolating between the min/max strength
  float forceStrength = GetStrengthValue(t);
  return forceStrength * mOutwardDirectionScalar;
}

Vec3 BasicPointEffect::GetForceAppliedAtPoint(Vec3Param point)
{
  // Calculate the normalized vector to the center point as well as the distance from the center point
  Vec3 toCenter = point - mWorldPoint;
  real distance = toCenter.AttemptNormalize();

  // Calculate the strength of the force given the radial distance
  float forceStrength = GetForceStrenthAtDistance(distance);
  return toCenter * forceStrength;
}

Vec3 BasicPointEffect::CalculateInwardForce(RigidBody* obj)
{
  return GetForceAppliedAtPoint(obj->GetWorldCenterOfMass());
}

real BasicPointEffect::GetStrengthValue(real t)
{
  // If we're quadratically interpolating, just square the interpolation value
  if(mPointFlags.IsSet(PointFlags::QuadraticInterpolation))
    t *= t;

  // Interpolate between the min/max strengths
  return Math::Lerp(mMinStrength, mMaxStrength, t);
}

real BasicPointEffect::GetMinDistance()
{
  return mMinDistance;
}

void BasicPointEffect::SetMinDistance(real distance)
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

real BasicPointEffect::GetMaxDistance()
{
  return mMaxDistance;
}

void BasicPointEffect::SetMaxDistance(real distance)
{
  // Check to make sure that min < max. If so, clamp the distance to the min it can be.
  if(distance <= mMinDistance)
  {
    DoNotifyWarning("Invalid distance", "Cannot set the max distance to "
      "less than or equal to the min distance");
    distance = mMinDistance + real(.001f);
  }
  mMaxDistance = distance;
  // Make sure to wake-up objects if we do that on effect change
  CheckWakeUp();
}

real BasicPointEffect::GetStrengthAtMin()
{
  return mMinStrength;
}

void BasicPointEffect::SetStrengthAtMin(real strength)
{
  mMinStrength = strength;
  // Make sure to wake-up objects if we do that on effect change
  CheckWakeUp();
}

real BasicPointEffect::GetStrengthAtMax()
{
  return mMaxStrength;
}

void BasicPointEffect::SetStrengthAtMax(real strength)
{
  mMaxStrength = strength;
  // Make sure to wake-up objects if we do that on effect change
  CheckWakeUp();
}

Vec3 BasicPointEffect::GetLocalPositionOffset()
{
  return mLocalPositionOffset;
}

void BasicPointEffect::SetLocalPositionOffset(Vec3Param offset)
{
  mLocalPositionOffset = offset;
  // Make sure to wake-up objects if we do that on effect change
  CheckWakeUp();
}

PhysicsEffectEndCondition::Enum BasicPointEffect::GetEndCondition()
{
  // Convert the bitfield representation of our end condition to the enum value
  PhysicsEffectEndCondition::Enum state;
  if(mPointFlags.IsSet(PointFlags::ContinueFalloff))
    state = PhysicsEffectEndCondition::ContinueFalloff;
  else if(mPointFlags.IsSet(PointFlags::NoEffect))
    state = PhysicsEffectEndCondition::NoEffect;
  else if(mPointFlags.IsSet(PointFlags::ClampToMax))
    state = PhysicsEffectEndCondition::ClampToMax;
  return state;
}

void BasicPointEffect::SetEndCondition(PhysicsEffectEndCondition::Enum condition)
{
  // Convert the enum representation of our end condition to the bitfield
  mPointFlags.ClearFlag(PointFlags::ContinueFalloff | PointFlags::NoEffect | PointFlags::ClampToMax);
  if(condition == PhysicsEffectEndCondition::NoEffect)
    mPointFlags.SetFlag(PointFlags::NoEffect);
  else if(condition == PhysicsEffectEndCondition::ContinueFalloff)
    mPointFlags.SetFlag(PointFlags::ContinueFalloff);
  else if(condition == PhysicsEffectEndCondition::ClampToMax)
    mPointFlags.SetFlag(PointFlags::ClampToMax);
  // Make sure to wake-up objects if we do that on effect change
  CheckWakeUp();
}

PhysicsEffectInterpolationType::Enum BasicPointEffect::GetInterpolationType()
{
  // Convert the bitfield representation of our interpolation to the enum value
  PhysicsEffectInterpolationType::Enum state;
  if(mPointFlags.IsSet(PointFlags::LinearInterpolation))
    state = PhysicsEffectInterpolationType::Linear;
  else if(mPointFlags.IsSet(PointFlags::QuadraticInterpolation))
    state = PhysicsEffectInterpolationType::Quadratic;
  return state;
}

void BasicPointEffect::SetInterpolationType(PhysicsEffectInterpolationType::Enum type)
{
  // Convert the enum representation of our interpolation to the bitfield
  mPointFlags.ClearFlag(PointFlags::LinearInterpolation);
  if(type == PhysicsEffectInterpolationType::Linear)
    mPointFlags.SetFlag(PointFlags::LinearInterpolation);
  else if(type == PhysicsEffectInterpolationType::Quadratic)
    mPointFlags.SetFlag(PointFlags::QuadraticInterpolation);
  // Make sure to wake-up objects if we do that on effect change
  CheckWakeUp();
}

//-------------------------------------------------------------------PointForceEffect
ZilchDefineType(PointForceEffect, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZeroBindDocumented();
}

PointForceEffect::PointForceEffect()
{
  mEffectType = PhysicsEffectType::PointForce;
  // Force effect points out with positive strength values
  mOutwardDirectionScalar = 1;
}

void PointForceEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if(GetActive() == false)
    return;

  // Calculate and apply the force
  Vec3 force = CalculateInwardForce(obj);
  obj->ApplyForce(force);
}

//-------------------------------------------------------------------PointGravityEffect
ZilchDefineType(PointGravityEffect, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZeroBindDocumented();
}

PointGravityEffect::PointGravityEffect()
{
  mEffectType = PhysicsEffectType::PointGravity;
  // Force effect points inward with positive strength values
  mOutwardDirectionScalar = -1;
}

void PointGravityEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if(GetActive() == false)
    return;

  Vec3 force = CalculateInwardForce(obj);
  // To apply this as an acceleration, divide out by the mass.
  Vec3 scaledForce = obj->mInvMass.ApplyInverted(force);
  obj->ApplyForce(scaledForce);
}

}//namespace Zero
