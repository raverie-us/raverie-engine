///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{

/// A constraint solver designed to thread the constraints
/// into as many threads as possible.
class ThreadedSolver : public IConstraintSolver
{
public:
  ThreadedSolver();
  ~ThreadedSolver();

  // IConstraintSolver Interface
  void AddJoint(Joint* joint) override;
  void AddContact(Contact* contact) override;
  void AddJoints(JointList& joints) override;
  void AddContacts(ContactList& contacts) override;
  // Solve Functions
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
  MoleculeList mMolecules;
  uint mConstraintCount;

  typedef ConstraintGroup<Contact> ContactGroup;
  typedef ConstraintGroup<Joint> JointGroup;
  ContactGroup mContactPhases;
  JointGroup mJointPhases;
};

}//namespace Physics

}//namespace Zero
