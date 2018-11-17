///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Memory{class Pool;}

namespace Physics
{

struct Manifold;

///A class to manage and solve all constraints and joints. 
///This is used not only for constraints such as ropes and motors, 
///but is also used for collisions/contacts.
class IConstraintSolver
{
public:
  static Memory::Pool* sPool;

  OverloadedNew();

  typedef InList<Contact,&Contact::SolverLink> ContactList;
  typedef InList<Joint,&Joint::SolverLink> JointList;

  IConstraintSolver() { mHeap = nullptr; };
  virtual ~IConstraintSolver() {};

  void SetConfiguration(PhysicsSolverConfig* config) 
  {
    mSolverConfig = config;
  };

  void SetHeap(Memory::Heap* heap)
  {
    mHeap = heap;
  }

  ///Add functions for joints
  virtual void AddJoint(Joint* joint) {}
  virtual void AddContact(Contact* contact) {}
  ///Batch add functions
  virtual void AddJoints(JointList& joints);
  virtual void AddContacts(ContactList& contacts);

  ///Attempts to satisfy all of the joints for the given frame.
  virtual void Solve(real dt) = 0;
  ///Debug draws all joints and contacts.
  virtual void DebugDraw(uint debugFlags) = 0;
  ///Removes all joints.
  virtual void Clear() = 0;

  ///Intermediate solving functions
  virtual void UpdateData() {};
  virtual void WarmStart() {};
  virtual void SolveVelocities() {};
  virtual void IterateVelocities(uint iteration) {};
  virtual void SolvePositions() {};
  virtual void Commit() {};
  virtual void BatchEvents() {};

  /// Returns the number of iterations this solver should use (determined by the config).
  uint GetSolverIterationCount() const;
  uint GetSolverPositionIterationCount() const;


  PhysicsSolverConfig* mSolverConfig;
  Memory::Heap* mHeap;
};

}//namespace Physics

}//namespace Zero
