///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Collider;
class RigidBody;
class JointEvent;
struct Joint;

namespace Physics
{

struct Jacobian;
struct JointVelocity;
struct JointMass;
struct JointNode;

namespace JointHelpers
{

///Helper to fetch the velocities for the colliders in each edge.
///Fills a value with zero if no rigid body exists.
void GetVelocities(RigidBody* body0, RigidBody* body1, JointVelocity& velocities);
void GetVelocities(Collider* obj0, Collider* obj1, JointVelocity& velocities);
void GetVelocities(Joint* joint, JointVelocity& velocities);
///Helper to fetch the mass and inertia for the colliders in each edge.
///Fills a value with zero if no rigid body exists.
void GetMasses(RigidBody* body0, RigidBody* body1, JointMass& masses);
void GetMasses(Collider* obj0, Collider* obj1, JointMass& masses);
void GetMasses(Joint* joint, JointMass& masses);

void ForceAwakeJoint(Joint* joint);

void ApplyConstraintImpulse(RigidBody* body0, RigidBody* body1, const Jacobian& jacobian, real lambda);
void ApplyConstraintImpulse(JointMass& masses, JointVelocity& velocities, const Jacobian& jacobian, real lambda);
void CommitVelocities(Collider* obj0, Collider* obj1, JointVelocity& velocities);

Vec3 WorldToBodyR(Collider* collider, Vec3Param worldVector);
Vec3 BodyToWorldR(Collider* collider, Vec3Param bodyVector);
Vec3 WorldPointToBodyR(Collider* collider, Vec3Param worldPoint);
Vec3 BodyRToWorldPoint(Collider* collider, Vec3Param bodyR);
Vec3 Body1ToBody2R(Collider* body0, Collider* body1, Vec3Param body1R);

Vec3 BodyRToCenterMassR(Collider* collider, Vec3Param bodyVector);

void UnlinkJointsFromSolver(Collider* collider);
real GetRelativeAngle(QuatParam rotation, Vec3 localAxis);
real GetRelativeAngle(QuatParam initial, Mat3Param obj1, Mat3Param obj2, Vec3 localAxis);
real GetRelativeAngle(QuatParam initial, QuatParam obj1, QuatParam obj2, Vec3 localAxis);

JointEvent* CreateJointEvent(Joint* joint, StringParam eventName);
JointEvent* CreateJointEvent(JointMotor* motor, StringParam eventName);

template <typename EdgeListType>
void WakeUpConnected(EdgeListType& edgeList)
{
  typename EdgeListType::range range = edgeList.All();
  for(; !range.Empty(); range.PopFront())
    range.Front().mOther->ForceAwake();
}

}//namespace JointHelpers

#define DeclareJointAccessors(type, name) \
  type Get##Name() const { return m##name; } \
  void Set##Name(const type& var) { m##name = var; JointHelpers::ForceAwakeJoint(this); }

}//namespace Physics

}//namespace Zero
