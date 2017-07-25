///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Physics
{

bool ShouldSolvePosition(Contact* contact)
{
  return true;
}

bool ShouldSolvePosition(Joint* joint)
{
  PhysicsSolverConfig* config = joint->mSolver->mSolverConfig;
  JointConfigOverride* configOverride = joint->mNode->mConfigOverride;
  uint jointType = joint->GetJointType();

  // Hack! Normally we check the config values for joints to determine if it
  // should apply post stabilization or baumgarte. Custom joints determine this
  // via each constraint's SolvePosition bool which is set from script so just
  // check how many position constraints there are.
  if(jointType == CustomJoint::StaticGetJointType())
    return joint->PositionMoleculeCount() != 0;

  if(configOverride != nullptr)
  {
    if(configOverride->mPositionCorrectionType == ConstraintPositionCorrection::Baumgarte)
      return false;
    else if(configOverride->mPositionCorrectionType == ConstraintPositionCorrection::PostStabilization)
      return true;
  }

  if(config->mJointBlocks[jointType].mPositionCorrectionType == ConstraintPositionCorrection::Baumgarte)
    return false;
  if(config->mJointBlocks[jointType].mPositionCorrectionType == ConstraintPositionCorrection::PostStabilization)
    return true;

  return config->mPositionCorrectionType == PhysicsSolverPositionCorrection::PostStabilization;
}

void CollectContactsToSolve(IConstraintSolver::ContactList& inputList, IConstraintSolver::ContactList& contactsToSolve,
  PhysicsSolverConfig* config)
{
  //if the global + contact config say to use position correction then
  //solve all contacts with position correction
  PhysicsSolverPositionCorrection::Enum globalCorrection = config->mPositionCorrectionType;
  ConstraintPositionCorrection::Enum contactCorrection = config->mContactBlock.GetPositionCorrectionType();
  if((globalCorrection == PhysicsSolverPositionCorrection::PostStabilization && contactCorrection != ConstraintPositionCorrection::Baumgarte) ||
    (contactCorrection == ConstraintPositionCorrection::PostStabilization))
    contactsToSolve.Swap(inputList);
}

void ContactUpdate(Contact* contact, Collider* c0, Collider* c1)
{
  ProfileScopeTree("Contacts", "SolvePositions", Color::BlueViolet);

  Manifold* manifold = contact->mManifold;
  for(uint i = 0; i < manifold->ContactCount; ++i)
  {
    ManifoldPoint& point = manifold->Contacts[i];
    point.WorldPoints[0] = JointHelpers::BodyRToWorldPoint(c0, point.BodyPoints[0]);
    point.WorldPoints[1] = JointHelpers::BodyRToWorldPoint(c1, point.BodyPoints[1]);
    point.Penetration = Math::Dot(point.WorldPoints[0] - point.WorldPoints[1], point.Normal);
  }
}

void UpdateHierarchyTransform(RigidBody* body)
{
  {
    PhysicsNode* node = body->mPhysicsNode;
    //figure out if we need to include our parent's transform
    WorldTransformation* parentTransformation = nullptr;
    if(node->mParent)
      parentTransformation = node->mParent->GetTransform();

    WorldTransformation* nodeTransform = node->GetTransform();
    nodeTransform->ComputeTransformation(parentTransformation, node);

    body->mRotationQuat = Math::ToQuaternion(nodeTransform->GetWorldRotation());
  }

  RigidBody::CompositeColliderRange childColliders = body->mColliders.All();
  for(; !childColliders.Empty(); childColliders.PopFront())
  {
    Collider& collider = childColliders.Front();

    PhysicsNode* node = collider.mPhysicsNode;
    //figure out if we need to include our parent's transform
    WorldTransformation* parentTransformation = nullptr;
    if(node->mParent)
      parentTransformation = node->mParent->GetTransform();

    WorldTransformation* transform = node->GetTransform();
    transform->ComputeTransformation(parentTransformation, node);

    collider.CacheWorldValues();
  }

  RigidBody::BodyRange childBodies = body->mChildBodies.All();
  for(; !childBodies.Empty(); childBodies.PopFront())
  {
    RigidBody& childBody = childBodies.Front();
    UpdateHierarchyTransform(&childBody);

    childBody.mRotationQuat = Math::ToQuaternion(childBody.mPhysicsNode->GetTransform()->GetWorldRotation());
  }
}

void ApplyPositionCorrection(RigidBody* body, Vec3Param linearOffset, Vec3Param angularOffset)
{
  //We need to use the kinematic body for velocity correction
  //(since we need its velocity), but we don't want to updated it during
  //position correction (we don't want to update based upon its center of mass).
  //This is also convenient because there's no reason to position correct kinematics anyways.
  if(body != nullptr && body->GetKinematic() == false)
  {
    //translation is very simple to update, just offset by the linear offset
    body->UpdateCenterMass(linearOffset);
    //orientation is a bit trickier, we treat the angular offset like an
    //angular velocity that we're applying with a timestep of 1 and then we
    //integrate that angular velocity to get a new orientation
    Quat rot = body->GetWorldRotationQuat();
    Quat w = Quat(angularOffset.x, angularOffset.y, angularOffset.z, 0);
    rot = (w * rot) * real(0.5);
    body->UpdateOrientation(rot);

    // Below is no longer needed because the transforms need to be updated before solving
    // (since they're out of date on the first iteration due to position correction).
    // No extra work needs to be done afterwards either because publish will grab the updated
    // values on the body and the individual children values aren't important.

    //Unfortunately, position correction needs up-to-date positions which
    //is a bit trickier with hierarchies. Each node in the hierarchy stores a
    //cached body-to-world transform that needs to be updated after each position correction.
    //For now just directly update the hierarchy and maybe later worry
    //about a more efficient way (post transforms?)
    //ProfileScopeTree("Hierarchy", "SolvePositions", Color::PeachPuff);
    //UpdateHierarchyTransform(body);
  }
}

void ApplyPositionCorrection(RigidBody* b0, RigidBody* b1,
                             Vec3Param linearOffset0, Vec3Param angularOffset0,
                             Vec3Param linearOffset1, Vec3Param angularOffset1)
{
  ApplyPositionCorrection(b0, linearOffset0, angularOffset0);
  ApplyPositionCorrection(b1, linearOffset1, angularOffset1);
}

}//namespace Physics

}//namespace Zero
