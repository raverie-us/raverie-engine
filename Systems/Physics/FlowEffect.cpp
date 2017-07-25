///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
  
ZilchDefineType(FlowEffect, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
  
  ZilchBindGetterSetterProperty(LocalForce)->ZeroSerialize(true);
  ZilchBindGetterSetterProperty(AttractToFlowCenter)->ZeroSerialize(false);
  ZilchBindGetterSetterProperty(FlowSpeed)->ZeroSerialize(real(10));
  ZilchBindGetterSetterProperty(MaxFlowForce)->ZeroSerialize(real(10));
  ZilchBindGetterSetterProperty(FlowDirection)->ZeroSerialize(Vec3(0, 1, 0));
  ZilchBindGetterProperty(WorldFlowDirection);
  ZilchBindGetterSetterProperty(AttractSpeed)->ZeroSerialize(real(5));
  ZilchBindGetterSetterProperty(MaxAttractForce)->ZeroSerialize(real(50));
}

FlowEffect::FlowEffect()
{
  mEffectType = PhysicsEffectType::Flow;
}

void FlowEffect::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

void FlowEffect::Initialize(CogInitializer& data)
{
  PhysicsEffect::Initialize(data);
}

void FlowEffect::DebugDraw()
{
  if(!GetDebugDrawEffect())
    return;

  PreCalculate(0);
  mWorldFlowCenter;
  mWorldFlowDirection;

  // Get the total time passed in this space for the animation of the effect
  real totalTime = GetAnimationTime(GetOwner());

  // Get a cylinder approximation of the current cog
  Cylinder cylinder = GetCogCylinder(GetOwner(), mWorldFlowDirection);
  
  // Offset the cylinder so its center is at the flow center. The cylinder
  // might be too big with this, but it more accurately shows the flow center.
  Vec3 cylinderCenter;
  cylinder.GetCenter(cylinderCenter);
  cylinder.PointA += mWorldFlowCenter - cylinderCenter;
  cylinder.PointB += mWorldFlowCenter - cylinderCenter;

  real cylinderHeight = Math::Distance(cylinder.PointA, cylinder.PointB);
  real radius = cylinder.Radius;
  Vec3 topPoint = cylinder.PointB;
  Vec3 botPoint = cylinder.PointA;
  
  // Get two tangential directions to the flow direction to draw the ring arrows
  Vec3 axis0, axis1;
  Math::GenerateOrthonormalBasis(mWorldFlowDirection, &axis0, &axis1);
  
  // Draw the flow direction
  gDebugDraw->Add(Debug::Line(mWorldFlowCenter, mWorldFlowCenter + mWorldFlowDirection).HeadSize(0.1f));  

  // Pick some arbitrary scaling on the inward force vectors based upon the ring's radius
  real inwardForceDisplaySize = Math::Min(0.5f, radius * 0.5f);
  real headSize = Math::Clamp(radius * 0.1f, 0.01f, 0.1f);
  // Clamp the animating speed. Beyond a certain point it's too hard to visualize.
  real flowAnimationSpeed = Math::Clamp(mFlowSpeed, 0.0f, 10.0f);
  
  real flowSign = (real)Math::Sign(mFlowSpeed);
  real attractSign = (real)Math::Sign(mAttractSpeed);

  // Draw animating rings from the bottom of the cylinder to the top. Determine how many rings to draw from the height
  // of the cylinder. We can draw anywhere from 1 to 5, depending on the height of the cylinder.
  size_t ringCount = Math::Clamp((int)Math::Ceil(cylinderHeight), 1, 5);
  for(size_t i = 0; i < ringCount; ++i)
  {
    // Normalize the t-value so that it moves at 1 unit per second
    real totalT = (totalTime / cylinderHeight);
    // Now scale the t-value so it's moving at the flow speed
    totalT *= flowAnimationSpeed;
    // Offset the t-value for this ring
    totalT += real(i) / ringCount;
    // Now convert this to a 0-1 value
    real t = Math::Fractional(totalT);
    // If the flow speed is negated, the "invert" the interpolation value so we go from top to bottom
    if(flowSign < 0)
      t = (1 - t);

    // Compute the center of this animating disc
    Vec3 discCenter = Math::Lerp(botPoint, topPoint, t);
    gDebugDraw->Add(Debug::Circle(discCenter, mWorldFlowDirection, radius));

    // If we attract to the flow center, also draw the "inward" force vectors
    if(GetAttractToFlowCenter())
    {
      // Draw 9 arrows evenly spaced about the ring
      size_t subDivisions = 9;
      for(size_t i = 0; i < subDivisions; ++i)
      {
        // Compute the angle along the circle and use that the get the position on the ring
        real theta = (Math::cTwoPi * i) / subDivisions;
        Vec3 offset = axis0 * Math::Cos(theta) + axis1 * Math::Sin(theta);
        Vec3 radialPoint = discCenter + offset * radius;
        // Compute the "inward vector" based upon the direction of the attract force
        Vec3 dir = attractSign * offset * inwardForceDisplaySize;
        gDebugDraw->Add(Debug::Line(radialPoint, radialPoint - dir).HeadSize(headSize));
      }
    }
  }
}

void FlowEffect::PreCalculate(real dt)
{
  UpdateFlowInformation();
}

void FlowEffect::ApplyEffect(RigidBody* obj, real dt)
{
  if(!GetActive())
    return;

  // Get the inverse mass of the object, we need this to compute the acceleration values as forces
  real invMass = obj->mInvMass.GetScalarInvMass();
  if(invMass == real(0))
    return;

  Vec3 worldFlowDir = mWorldFlowDirection;
  Vec3 worldFlowCenter = mWorldFlowCenter;
  
  // Determine the relative velocity of the rigid body in the flow direction
  // (how much of an error the flow speed has)
  real currentFlow = Math::Dot(worldFlowDir, obj->mVelocity);
  real flowToApply = mFlowSpeed - currentFlow;

  // Calculate how much force is required to get to our target flow velocity
  real appliedForce = flowToApply / (invMass * dt);
  // Now clamp that force based upon the max flow strength
  // (so we will always target a set speed but have a max acceleration with which to get there)
  appliedForce = Math::Clamp(appliedForce, -mMaxFlowForce, mMaxFlowForce);
  obj->ApplyForce(appliedForce * worldFlowDir);

  // If we want to attract the object towards the center of the flow
  if(mForceStates.IsSet(FlowFlags::AttractToFlowCenter))
  {
    // Compute the translation offset of the rigid body perpendicular to the flow direction
    Vec3 objCenter = obj->GetWorldTranslation();
    Vec3 offset = objCenter - worldFlowCenter;
    Vec3 dirToCenter = -Math::ProjectOnPlane(offset, worldFlowDir);
    real distanceToCenter = dirToCenter.AttemptNormalize();
    
    // Compute the velocity in the perpendicular flow direction
    Vec3 sideVel = Math::ProjectOnPlane(obj->mVelocity, worldFlowDir);
    real sideSpeed = Math::Length(sideVel);
    

    Vec3 totalSideAttractionForce = Vec3::cZero;
    // If the object is in the center of the flow then we can't apply an attraction force
    // towards the center. In that case just compute a force to slow down the object.
    if(distanceToCenter == real(0))
    {
      Vec3 velocityDir = sideVel.AttemptNormalized();
      real stoppingForce = sideSpeed / (invMass * dt);
      stoppingForce = Math::Clamp(stoppingForce, -mMaxAttractForce, mMaxAttractForce);
      totalSideAttractionForce = -velocityDir * stoppingForce;
    }
    // Otherwise, compute an acceleration force towards the center of the flow (but avoiding overshoot)
    else
    {
      // Make sure our side speed is pointing towards the center of the flow
      if(Math::Dot(sideVel, dirToCenter) < 0)
        sideSpeed = -sideSpeed;

      // Calculate the acceleration needed to reach our target speed
      real accelerationToTargetSpeed = (mAttractSpeed - sideSpeed) / dt;
      // This acceleration could overshoot our target position, so also calculate
      // the acceleration that will get us to our target position next frame
      real accelerationToTargetPosition = (distanceToCenter - dt * sideSpeed) / (dt * dt);

      // We want the min of the two accelerations (so we'll stop at the center)
      real acceleration = Math::Min(accelerationToTargetSpeed, accelerationToTargetPosition);

      // Clamp the force to our force limits
      real appliedAttractForce = acceleration / invMass;
      appliedAttractForce = Math::Clamp(appliedAttractForce, -mMaxAttractForce, mMaxAttractForce);
      totalSideAttractionForce = dirToCenter * appliedAttractForce;
    }
    obj->ApplyForce(totalSideAttractionForce);
  }
}

void FlowEffect::UpdateFlowInformation()
{
  mWorldFlowCenter = Vec3::cZero;
  mWorldFlowDirection = mFlowDirection;

  // If this is a local force then transform the flow direction and position to world space
  if(mForceStates.IsSet(FlowFlags::LocalForce))
    TransformLocalDirectionAndPointToWorld(mWorldFlowCenter, mWorldFlowDirection);
  // Otherwise we still need the world space position
  else
    mWorldFlowCenter = TransformLocalPointToWorld(mWorldFlowCenter);

  // Always re-normalize the world axis
  mWorldFlowDirection.AttemptNormalize();
}

bool FlowEffect::GetLocalForce() const
{
  return mForceStates.IsSet(FlowFlags::LocalForce);
}

void FlowEffect::SetLocalForce(bool state)
{
  mForceStates.SetState(FlowFlags::LocalForce, state);
  CheckWakeUp();
}

bool FlowEffect::GetAttractToFlowCenter()
{
  return mForceStates.IsSet(FlowFlags::AttractToFlowCenter);
}

void FlowEffect::SetAttractToFlowCenter(bool state)
{
  mForceStates.SetState(FlowFlags::AttractToFlowCenter, state);
  CheckWakeUp();
}

real FlowEffect::GetFlowSpeed()
{
  return mFlowSpeed;
}

void FlowEffect::SetFlowSpeed(real speed)
{
  mFlowSpeed = speed;
  CheckWakeUp();
}

real FlowEffect::GetMaxFlowForce()
{
  return mMaxFlowForce;
}

void FlowEffect::SetMaxFlowForce(real strength)
{
  mMaxFlowForce = strength;
  CheckWakeUp();
}

Vec3 FlowEffect::GetFlowDirection()
{
  return mFlowDirection;
}

void FlowEffect::SetFlowDirection(Vec3Param dir)
{
  mFlowDirection = dir;
  CheckWakeUp();
}

Vec3 FlowEffect::GetWorldFlowDirection()
{
  UpdateFlowInformation();
  return mWorldFlowDirection;
}

real FlowEffect::GetMaxAttractForce()
{
  return mMaxAttractForce;
}

void FlowEffect::SetMaxAttractForce(real strength)
{
  mMaxAttractForce = strength;
  CheckWakeUp();
}

real FlowEffect::GetAttractSpeed()
{
  return mAttractSpeed;
}

void FlowEffect::SetAttractSpeed(real speed)
{
  mAttractSpeed = speed;
  CheckWakeUp();
}

}//namespace Zero
