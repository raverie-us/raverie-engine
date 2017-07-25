///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

JointEdge::JointEdge()
{
  mJoint = nullptr;
  mCollider = mOther = nullptr;
}

namespace Physics
{

ContactEdge::ContactEdge()
{
  mJoint = nullptr;
  mCollider = mOther = nullptr;
}

JointVelocity::JointVelocity()
{
  Linear[0].ZeroOut();
  Angular[0].ZeroOut();
  Linear[1].ZeroOut();
  Angular[1].ZeroOut();
}

JointVelocity::JointVelocity(Joint* joint)
{
  JointHelpers::GetVelocities(joint,*this);
}

JointVelocity::JointVelocity(Vec3Param v0, Vec3Param w0, Vec3Param v1, Vec3Param w1)
{
  Linear[0] = v0;
  Angular[0] = w0;
  Linear[1] = v1;
  Angular[1] = w1;
}

JointMass::JointMass()
{
  InverseInertia[0].ZeroOut();
  InverseInertia[1].ZeroOut();
}

JointMass::JointMass(Joint* joint)
{
  JointHelpers::GetMasses(joint,*this);
}

Jacobian::Jacobian()
{
  Linear[0].ZeroOut();
  Angular[0].ZeroOut();
  Linear[1].ZeroOut();
  Angular[1].ZeroOut();
};

Jacobian::Jacobian(Vec3Param linear0, Vec3Param angular0, Vec3Param linear1, Vec3Param angular1)
{
  Linear[0] = linear0;
  Angular[0] = angular0;
  Linear[1] = linear1;
  Angular[1] = angular1;
}

void Jacobian::Set(Vec3Param linear0, Vec3Param angular0, Vec3Param linear1, Vec3Param angular1)
{
  Linear[0] = linear0;
  Angular[0] = angular0;
  Linear[1] = linear1;
  Angular[1] = angular1;
}

real Jacobian::ComputeMass(Mass M0, Mat3Param I0, Mass M1, Mat3Param I1) const
{

  real m1term = Math::Dot(Linear[0], M0.Apply(Linear[0]));
  real m2term = Math::Dot(Linear[1], M1.Apply(Linear[1]));
  real i1term = Math::Dot(Angular[0], Math::Transform(I0, Angular[0]));
  real i2term = Math::Dot(Angular[1], Math::Transform(I1, Angular[1]));
  return m1term + m2term + i1term + i2term;
}

real Jacobian::ComputeMass(JointMass& masses)
{
  return ComputeMass(masses.mInvMass[0], masses.InverseInertia[0], masses.mInvMass[1], masses.InverseInertia[1]);
}

real Jacobian::ComputeJV(Vec3Param v0, Vec3Param w0, Vec3Param v1, Vec3Param w1) const
{
  return Math::Dot(Linear[0], v0) + Math::Dot(Angular[0], w0) + 
         Math::Dot(Linear[1], v1) + Math::Dot(Angular[1], w1);
}

real Jacobian::ComputeJV(const JointVelocity& velocities)
{
  return ComputeJV(velocities.Linear[0], velocities.Angular[0], velocities.Linear[1], velocities.Angular[1]);
}

Jacobian Jacobian::operator-() const
{
  return Jacobian(-Linear[0], -Angular[0], -Linear[1], -Angular[1]);
}

}//namespace Physics

JointNode::JointNode()
{
  mJoint = nullptr;
  mLimit = nullptr;
  mMotor = nullptr;
  mSpring = nullptr;
  mConfigOverride = nullptr;
}

JointNode::~JointNode()
{
  if(mLimit)
    mLimit->mNode = nullptr;
  if(mMotor)
    mMotor->mNode = nullptr;
  if(mSpring)
    mSpring->mNode = nullptr;
  if(mConfigOverride)
    mConfigOverride->mNode = nullptr;
}

void JointNode::Destroy()
{
  if(mLimit)
    mLimit->GetOwner()->RemoveComponent(mLimit);
  if(mMotor)
    mMotor->GetOwner()->RemoveComponent(mMotor);
  if(mSpring)
    mSpring->GetOwner()->RemoveComponent(mSpring);
  if(mConfigOverride)
    mConfigOverride->GetOwner()->RemoveComponent(mConfigOverride);
}

bool JointNode::LimitIndexActive(uint atomIndex)
{
  uint flag = 1 << atomIndex;
  return mLimit != nullptr && mLimit->GetAtomIndexActive(flag);
}

bool JointNode::MotorIndexActive(uint atomIndex)
{
  uint flag = 1 << atomIndex;
  return mMotor != nullptr && mMotor->GetAtomIndexActive(flag);
}

bool JointNode::SpringIndexActive(uint atomIndex)
{
  uint flag = 1 << atomIndex;
  return mSpring != nullptr && mSpring->GetAtomIndexActive(flag);
}

}//namespace Zero
