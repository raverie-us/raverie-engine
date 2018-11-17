///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class PhysicsSpace;
namespace Memory{class Pool;}

namespace Physics
{

class IConstraintSolver;

///A set of objects and constraints that are dependent on each other.
///Used to solve constraint systems independently and to check for sleeping.
class Island
{
public:
  static Memory::Pool* sPool;
  static void* operator new(size_t size);
  static void operator delete(void* pMem, size_t size);

  Island();
  ~Island();

  void SetSolver(IConstraintSolver* solver, bool ownsSolver);
  void MergeIsland(Island& island);

  void Add(Collider* collider);
  void Add(Contact* contact);
  void Add(Joint* joint);

  void IntegrateVelocity(real dt);
  void IntegratePosition(real dt);
  void CommitConstraints();
  void Solve(real dt, bool allowSleeping, uint debugFlags);
  void SolvePositions(real dt);
  void UpdateSleep(real dt, bool allowSleeping, uint debugFlags);
  ///Helper function to mark everything as not on an island.
  void ClearIslandFlags(Collider& collider);
  void Clear();

  ///Returns if the provided collider is in the island.
  bool ContainsCollider(const Collider* collider);


  Link<Island> ManagerLink;
  typedef InList<Collider,&Collider::mIslandLink> Colliders;
  Colliders mColliders;
  IConstraintSolver* mSolver;

  typedef InList<Contact,&Contact::SolverLink> ContactList;
  ContactList mContacts;

  typedef InList<Joint,&Joint::SolverLink> JointList;
  JointList mJoints;
  JointList mUnSolvableJoints;

  bool mOwnsSolver;

  uint ContactCount;
  uint JointCount;
  uint ColliderCount;
};

}//namespace Physics

}//namespace Zero
