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
class JointSpring;
struct JointMotor;
struct JointLimit;
struct JointConfigOverride;

struct JointNode;
struct Joint;

///An edge for a joint. Used to construct islands efficiently.
struct JointEdge
{
  JointEdge();

  Joint* mJoint;
  Collider* mCollider;
  Collider* mOther;
  Link<JointEdge> ColliderLink;
};

namespace Physics
{

class Contact;

///An edge for a contact. Used to construct islands efficiently.
struct ContactEdge
{
  ContactEdge();

  //clean up to not be a union later
  union
  {
    Contact* mContact;
    Contact* mJoint;
  };
  Collider* mCollider;
  Collider* mOther;
  Link<ContactEdge> ColliderLink;
};

///Defines the velocity for a joint. A simple helper object.
struct JointVelocity
{
  JointVelocity();
  JointVelocity(Joint* joint);
  JointVelocity(Vec3Param v0, Vec3Param w0, Vec3Param v1, Vec3Param w1);
  Vec3 Linear[2];
  Vec3 Angular[2];
};

///Defines the mass for a joint. A simple helper object.
struct JointMass
{
  JointMass();
  JointMass(Joint* joint);

  Mass mInvMass[2];
  Mat3 InverseInertia[2];
};

///A Jacobian structure. Used to specify how to solve a generic constraint.
struct Jacobian
{
  Jacobian();
  Jacobian(Vec3Param linear0, Vec3Param angular0, Vec3Param linear1, Vec3Param angular1);

  void Set(Vec3Param linear0, Vec3Param angular0, Vec3Param linear1, Vec3Param angular1);
  real ComputeMass(Mass M0, Mat3Param I0, Mass M1, Mat3Param I1) const;
  real ComputeMass(JointMass& masses);
  real ComputeJV(Vec3Param v0, Vec3Param w0, Vec3Param v1, Vec3Param w1) const;
  real ComputeJV(const JointVelocity& velocities);

  Jacobian operator-() const;

  Vec3 Linear[2];
  Vec3 Angular[2];
};

}//namespace Physics

///A node for a joint so that a limit can find its joint and vice versa. Same
///for motors and springs.
struct JointNode
{
  JointNode();
  ~JointNode();

  void Destroy();
  
  bool LimitIndexActive(uint atomIndex);
  bool MotorIndexActive(uint atomIndex);
  bool SpringIndexActive(uint atomIndex);

  Joint* mJoint;
  JointLimit* mLimit;
  JointMotor* mMotor;
  JointSpring* mSpring;
  JointConfigOverride* mConfigOverride;
};

struct JointInfo
{
  // This takes in the max number of atoms this constraint has (ignoring motors) and
  // then a mask of which ones of these are inactive by default.
  JointInfo(uint atomCount, uint offAtomMask)
  {
    mAtomCount = atomCount;
    mAtomCountMask = (1 << atomCount) - 1;
    mOffAtomMask = offAtomMask;
    mOnAtomMask = (~offAtomMask) & mAtomCountMask;
  }

  uint mAtomCount;
  uint mAtomCountMask;
  uint mOffAtomMask;
  uint mOnAtomMask;
};

}//namespace Zero
