///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(DynamicMotor, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZeroBindDependency(Collider);
  ZeroBindDependency(RigidBody);

  ZilchBindMethod(MoveInDirection);
  ZilchBindMethod(SetReferenceFrameToWorld);
  ZilchBindMethod(SetReferenceFrameToObject);
  ZilchBindGetterSetter(Active);
  ZilchBindGetterSetter(MaxMoveImpulse);
}

DynamicMotor::DynamicMotor()
{
  mActive = true;
  mMaxMoveImpulse = real(5.0);
}

DynamicMotor::~DynamicMotor()
{
  // Delete the primary joint
  Cog* primary = mVelJointCog;
  if(primary)
    primary->ForceDestroy();
}

void DynamicMotor::Serialize(Serializer& stream)
{
  
}

void DynamicMotor::Initialize(CogInitializer& initializer)
{
  mSpace = initializer.mSpace->has(PhysicsSpace);
  mTransform = GetOwner()->has(Transform);
  mBody = GetOwner()->has(RigidBody);

  CreateJoint();
}

RelativeVelocityJoint* DynamicMotor::CreateJoint()
{
  // Create the velocity joint
  JointCreator jointCreator;
  Cog* jointCog = jointCreator.CreateWorldPoints(mSpace->mWorldCollider->GetOwner(), GetOwner(), CoreArchetypes::ObjectLink, Vec3::cZero);
  if(jointCog == nullptr)
    return nullptr;

  jointCog->SetEditorOnly();
  jointCog->AddComponentByName("RelativeVelocityJoint");
  jointCog->SetName(String::Format("%s%s", GetOwner()->GetName().c_str(), "MotorJoint"));
  // Store the CogId to the joint
  mVelJointCog = jointCog->GetId();

  // Initialize properties on the joint
  RelativeVelocityJoint* velJoint = jointCog->has(RelativeVelocityJoint);
  velJoint->SetSpeed(0, real(0.0));
  velJoint->SetSpeed(1, real(0.0));
  velJoint->SetSpeed(2, real(0.0));
  velJoint->SetAxisActive(0, true);
  velJoint->SetAxisActive(1, false);
  velJoint->SetAxisActive(2, true);
  SetMaxMoveImpulse(mMaxMoveImpulse);
  velJoint->SetAxis(0, Vec3::cZero);
  velJoint->SetAxis(1, Vec3::cZero);
  velJoint->SetAxis(2, Vec3::cZero);

  MoveInDirection(Vec3::cXAxis, Vec3::cYAxis);
  MoveInDirection(Vec3::cZero, Vec3::cYAxis);

  SetReferenceFrameToWorld();
  // Return the new joint
  return velJoint;
}

RelativeVelocityJoint* DynamicMotor::GetJoint()
{
  // Check to see if we have a valid cog with a relative velocity joint
  RelativeVelocityJoint* joint = mVelJointCog.has(RelativeVelocityJoint);
  if(joint == nullptr)
  {
    // Destroy the old cog and create a new joint cog (there's some issues with
    // recently dynamically deleted components still getting events that means we
    // can't just re-add the component, but rather we need a new composition)
    mVelJointCog.SafeDestroy();
    joint = CreateJoint();
  }
  return joint;
}

void DynamicMotor::SetReferenceFrameToWorld()
{
  SetReferenceFrameToObject(mSpace->mWorldCollider->GetOwner());
}

void DynamicMotor::SetReferenceFrameToObject(Cog* object)
{
  // Get the object link from the joint
  RelativeVelocityJoint* joint = GetJoint();
  ObjectLink* link = joint->GetOwner()->has(ObjectLink);
  // Set the object we're connected to
  link->SetObjectA(object);
}

void DynamicMotor::MoveInDirection(Vec3Param direction, Vec3Param up)
{
  if(mActive == false)
    return;

  RelativeVelocityJoint* velJoint = GetJoint();

  // Set the up vector
  velJoint->SetAxis(1, up);

  Vec3 dir = direction;
  real speed = dir.AttemptNormalize();
  if(speed != real(0.0))
  {
    // Set the primary axis
    velJoint->SetAxis(0, dir);

    // Set the right axis
    Vec3 right = Math::Cross(direction, up);
    velJoint->SetAxis(2, right);
  }
  else
  {
    Vec3 normalizedUp = up.AttemptNormalized();

    // If the old right or forward had any direction in the new up, this
    // could prevent gravity from affecting us. Because of this project
    // out the new up from the old right and forward.
    Vec3 oldForward = velJoint->GetAxis(0);
    Vec3 forward = oldForward - Math::Dot(normalizedUp, oldForward) * normalizedUp;
    forward.AttemptNormalize();
    velJoint->SetAxis(0, forward);

    // Set the right axis
    Vec3 right = Math::Cross(direction, up);
    velJoint->SetAxis(2, right);    
  }
  mBody->ForceAwake();
  velJoint->SetSpeed(0, speed);
}

bool DynamicMotor::GetActive() const
{
  return mActive;
}

void DynamicMotor::SetActive(bool active)
{
  RelativeVelocityJoint* velJoint = GetJoint();
  mActive = active;

  if(velJoint == nullptr)
    return;

  velJoint->SetAxisActive(0, active);
  velJoint->SetAxisActive(2, active);
}

float DynamicMotor::GetMaxMoveImpulse()
{
  return mMaxMoveImpulse;
}

void DynamicMotor::SetMaxMoveImpulse(float val)
{
  mMaxMoveImpulse = val;
  RelativeVelocityJoint* velJoint = GetJoint();
  velJoint->SetMaxImpulse(0, val);
  velJoint->SetMaxImpulse(1, val);
  velJoint->SetMaxImpulse(2, val);
}

}//namespace Zero
