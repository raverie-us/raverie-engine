///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{

struct Manifold;

///A solver that allows each object to solve itself. This allows joints and 
///contacts to change each iteration (such as contacts changing friction bounds).
///The basic solver stores all joints in one list and must therefore call virtual
///functions on each joint.
class BasicSolver : public IConstraintSolver
{
public:
  BasicSolver();
  ~BasicSolver();

  // IConstraintSolver Interface
  void AddJoint(Joint* joint) override;
  void AddContact(Contact* contact) override;
  void AddJoints(JointList& joints) override;
  // Solve Functions
  void AddContacts(ContactList& contacts) override;
  void Solve(real dt) override;
  void DebugDraw(uint debugFlags) override;
  void Clear() override;
  // Iteration functions
  void UpdateData() override;
  void WarmStart() override;
  void SolveVelocities() override;
  void IterateVelocities(uint iteration) override;
  void SolvePositions() override;
  void Commit() override;
  void BatchEvents() override;

  void DrawJoints(uint debugFlags);

private:
  typedef InList<Joint,&Joint::SolverLink> JointList;
  typedef InList<Contact,&Contact::SolverLink> ContactList;
  typedef Array<ConstraintMolecule> MoleculeList;

  JointList mJoints;
  ContactList mContacts;
  uint mConstraintCount;
  MoleculeList mMolecules;
};

}//namespace Physics

}//namespace Zero

