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
struct MoleculeWalker;

///A solver that allows each object to solve itself. This allows joints and 
///contacts to change each iteration (such as contacts changing friction bounds).
///The normal solver stores all in a separate list so it can avoid calling
///virtual functions and call each class' implementation directly.
class NormalSolver : public IConstraintSolver
{
public:
  NormalSolver();
  ~NormalSolver();

  // IConstraintSolver Interface
  void AddJoint(Joint* joint) override;
  void AddContact(Contact* contact) override;
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

private:
  typedef InList<Joint,&Joint::SolverLink> JointList;
  typedef InList<Contact,&Contact::SolverLink> ContactList;
  typedef Array<ConstraintMolecule> MoleculeList;

  //Declare a InList for each joint type.
#define JointType(type) \
  typedef BaseInList<Joint,type,&Joint::SolverLink> type##List; \
  type##List m##type##List;

#include "Physics/Joints/JointList.hpp"

#undef JointType

  uint mConstraintCount;

  ContactList mContacts;
  MoleculeList mMolecules;
};

}//namespace Physics

}//namespace Zero

