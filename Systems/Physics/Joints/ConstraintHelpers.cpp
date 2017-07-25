///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Physics
{

namespace JointHelpers
{

void GetVelocities(RigidBody* body0, RigidBody* body1, JointVelocity& velocities)
{
  if(body0)
  {
    velocities.Linear[0] = body0->mVelocity;
    velocities.Angular[0] = body0->mAngularVelocity;
  }
  else
  {
    velocities.Linear[0].ZeroOut();
    velocities.Angular[0].ZeroOut();
  }
  if(body1)
  {
    velocities.Linear[1] = body1->mVelocity;
    velocities.Angular[1] = body1->mAngularVelocity;
  }
  else
  {
    velocities.Linear[1].ZeroOut();
    velocities.Angular[1].ZeroOut();
  }
}

void GetVelocities(Collider* obj0, Collider* obj1, JointVelocity& velocities)
{
  RigidBody* body0 = obj0->GetActiveBody(),
           * body1 = obj1->GetActiveBody();

  GetVelocities(body0, body1, velocities);
}

void GetVelocities(Joint* joint, JointVelocity& velocities)
{
  GetVelocities(joint->GetCollider(0), joint->GetCollider(1), velocities);
}

void GetMasses(RigidBody* body0, RigidBody* body1, JointMass& masses)
{
  if(body0)
  {
    masses.mInvMass[0] = body0->mInvMass;
    masses.InverseInertia[0] = body0->mInvInertia.GetInvWorldTensor();
  }
  else
  {
    masses.mInvMass[0].SetInvMass(0);
    masses.InverseInertia[0].ZeroOut();
  }
  if(body1)
  {
    masses.mInvMass[1] = body1->mInvMass;
    masses.InverseInertia[1] = body1->mInvInertia.GetInvWorldTensor();
  }
  else
  {
    masses.mInvMass[1].SetInvMass(0);
    masses.InverseInertia[1].ZeroOut();
  }
}

void GetMasses(Collider* obj0, Collider* obj1, JointMass& masses)
{
  RigidBody* body0 = obj0->GetActiveBody();
  RigidBody* body1 = obj1->GetActiveBody();

  GetMasses(body0, body1, masses);
}

void GetMasses(Joint* joint, JointMass& masses)
{
  GetMasses(joint->GetCollider(0), joint->GetCollider(1), masses);
}

void ForceAwakeJoint(Joint* joint)
{
  if(!joint->GetValid())
    return;

  joint->GetCollider(0)->ForceAwake();
  joint->GetCollider(1)->ForceAwake();
}

void ApplyConstraintImpulse(RigidBody* body0, RigidBody* body1, const Jacobian& jacobian, real lambda)
{
  if(body0)
  {
    body0->ApplyConstraintImpulse(jacobian.Linear[0] * lambda,
                                  jacobian.Angular[0] * lambda);
  }
  if(body1)
  {
    body1->ApplyConstraintImpulse(jacobian.Linear[1] * lambda,
                                  jacobian.Angular[1] * lambda);
  }
}

void ApplyConstraintImpulse(JointMass& masses, JointVelocity& velocities, const Jacobian& jacobian, real lambda)
{
  velocities.Linear[0] += masses.mInvMass[0].Apply(jacobian.Linear[0]) * lambda;
  velocities.Angular[0] += Math::Transform(masses.InverseInertia[0], jacobian.Angular[0] * lambda);

  velocities.Linear[1] += masses.mInvMass[1].Apply(jacobian.Linear[1]) * lambda;
  velocities.Angular[1] += Math::Transform(masses.InverseInertia[1], jacobian.Angular[1] * lambda);
}

void CommitVelocities(Collider* obj0, Collider* obj1, JointVelocity& velocities)
{
  RigidBody* body0 = obj0->GetActiveBody();
  RigidBody* body1 = obj1->GetActiveBody();

  if(body0)
  {
    body0->mVelocity = velocities.Linear[0];
    body0->mAngularVelocity = velocities.Angular[0];
  }
  if(body1)
  {
    body1->mVelocity = velocities.Linear[1];
    body1->mAngularVelocity = velocities.Angular[1];
  }
}

Vec3 WorldToBodyR(Collider* collider, Vec3Param worldVector)
{
  return collider->GetWorldTransform()->InverseTransformNormal(worldVector);
}

Vec3 BodyToWorldR(Collider* collider, Vec3Param bodyVector)
{
  return collider->GetWorldTransform()->TransformNormal(bodyVector);
}

Vec3 WorldPointToBodyR(Collider* collider, Vec3Param worldPoint)
{
  return collider->GetWorldTransform()->InverseTransformPoint(worldPoint);
}

Vec3 BodyRToWorldPoint(Collider* collider, Vec3Param bodyR)
{
  return collider->GetWorldTransform()->TransformPoint(bodyR);
}

Vec3 Body1ToBody2R(Collider* body0, Collider* body1, Vec3Param body1R)
{
  Vec3 worldVector = BodyToWorldR(body0, body1R);
  return WorldToBodyR(body1, worldVector);
}

Vec3 BodyRToCenterMassR(Collider* collider, Vec3Param bodyVector)
{
  Vec3 worldPoint = BodyRToWorldPoint(collider, bodyVector);

  return worldPoint - collider->GetRigidBodyCenterOfMass();
}

void UnlinkJointsFromSolver(Collider* collider)
{
  typedef InList<Physics::Contact, &Physics::Contact::SolverLink> ContactList;
  typedef InList<Joint, &Joint::SolverLink> JointList;
  
  Collider::JointEdgeList::range jointRange = collider->mJointEdges.All();
  while(!jointRange.Empty())
  {
    Joint* joint = jointRange.Front().mJoint;
    jointRange.PopFront();
    if(!joint->GetOnIsland())
      JointList::Unlink(joint);
    joint->SetOnIsland(false);
  }

  Collider::ContactEdgeList::range contactNewRange = collider->mContactEdges.All();
  while(!contactNewRange.Empty())
  {
    Physics::Contact* contact = contactNewRange.Front().mContact;
    contactNewRange.PopFront();
    if(!contact->GetGhost())
      ContactList::Unlink(contact);
    contact->SetOnIsland(false);
  }
}

real GetRelativeAngle(QuatParam rotation, Vec3 localAxis)
{
  real arcCos = Math::ArcCos(rotation.w) * 2;
  //deal with larger than pi angles (due to the times 2)
  if(arcCos > Math::cPi)
    arcCos -= Math::cTwoPi;
  //deal with an angle that had a negated axis
  if(Math::Dot(rotation.V3(), localAxis) < 0)
    arcCos *= -1;
  return arcCos;
}

real GetRelativeAngle(QuatParam initial, Mat3Param obj0, Mat3Param obj1, Vec3 localAxis)
{
  return GetRelativeAngle(initial, Math::ToQuaternion(obj0), Math::ToQuaternion(obj1), localAxis);
}

real GetRelativeAngle(QuatParam initial, QuatParam obj0, QuatParam obj1, Vec3 localAxis)
{
  //get the delta rotation
  Quat deltaRotation = obj0.Conjugated() * obj1;
  //get the delta of the delta's (that Contains the angle error)
  Quat err = deltaRotation * initial.Conjugated();
  err.Normalize();

  return GetRelativeAngle(err, localAxis);
}

JointEvent* CreateJointEvent(Joint* joint, StringParam eventName)
{
  return joint->mSpace->mEventManager->BatchJointEvent(joint, eventName);
}

JointEvent* CreateJointEvent(JointMotor* motor, StringParam eventName)
{
  return CreateJointEvent(motor->mNode->mJoint, eventName);
}

}//namespace JointHelpers

}//namespace Physics

}//namespace Zero
