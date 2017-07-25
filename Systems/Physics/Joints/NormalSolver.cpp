///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Physics
{

NormalSolver::NormalSolver()
{
  SetConfiguration(nullptr);
  mConstraintCount = 0;
}

NormalSolver::~NormalSolver()
{
  Clear();
}

//define a case statement for each joint type
#define JointType(type) \
  case JointEnums::type##Type: \
  { \
  m##type##List.PushBack(joint); \
    break; \
  }
  

void NormalSolver::AddJoint(Joint* joint)
{
  joint->mSolver = this;
  joint->UpdateAtomsVirtual();
  mConstraintCount += joint->MoleculeCountVirtual();

  //switch to determine which list to add the joint to
  switch(joint->GetJointType())
  {
#include "Physics/Joints/JointList.hpp"
  }
}

#undef JointType

void NormalSolver::AddContact(Contact* contact)
{
  contact->mSolver = this;
  contact->UpdateAtoms();
  mConstraintCount += contact->MoleculeCount();
  mContacts.PushBack(contact);
}

///Necessary solving functions
void NormalSolver::Solve(real dt)
{
  NormalSolver::UpdateData();
  NormalSolver::WarmStart();
  NormalSolver::SolveVelocities();
  NormalSolver::Commit();
  NormalSolver::BatchEvents();
}

void NormalSolver::DebugDraw(uint debugFlags)
{
  if(!(debugFlags & PhysicsSpaceDebugDrawFlags::DrawConstraints))
    return;

#define JointType(type) \
  DrawJointsFragmentList(m##type##List);

#include "Physics/Joints/JointList.hpp"

#undef JointType
  DrawJointsFragmentList(mContacts);
  //if(debugFlags & Physics::DebugDraw::DrawConstraints)
  //  DrawJoints(debugFlags);
}

void NormalSolver::Clear()
{
#define JointType(type) \
  ClearFragmentList(m##type##List);

#include "Physics/Joints/JointList.hpp"

#undef JointType

  ClearFragmentList(mContacts);
}

void NormalSolver::UpdateData()
{
  mMolecules.Resize(mConstraintCount);
  
  MoleculeWalker molecules(mMolecules.Data(),sizeof(ConstraintMolecule),0);

#define JointType(type) \
  UpdateDataFragmentList(m##type##List,molecules);

#include "Physics/Joints/JointList.hpp"

#undef JointType

  UpdateDataFragmentList(mContacts,molecules);
}

void NormalSolver::WarmStart()
{
  if(mSolverConfig->mWarmStart == false)
    return;

  MoleculeWalker molecules(mMolecules.Data(),sizeof(ConstraintMolecule),0);

#define JointType(type) \
  WarmStartFragmentList(m##type##List,molecules);

#include "Physics/Joints/JointList.hpp"

#undef JointType

  WarmStartFragmentList(mContacts,molecules);
}

void NormalSolver::SolveVelocities()
{
  //solve all of the velocity constraints the given number of times
  for(uint i = 0; i < GetSolverIterationCount(); ++i)
    IterateVelocities(i);
}

void NormalSolver::IterateVelocities(uint iteration)
{
  MoleculeWalker molecules(mMolecules.Data(),sizeof(ConstraintMolecule),0);

#define JointType(type) \
  IterateVelocitiesFragmentList(m##type##List,molecules,iteration);

#include "Physics/Joints/JointList.hpp"

#undef JointType

  IterateVelocitiesFragmentList(mContacts,molecules,iteration);
}

void NormalSolver::SolvePositions()
{
  //get a list of all joints that actually need position correction
#define JointType(type)                                \
  type##List type##ToSolve;                            \
  CollectJointsToSolve(m##type##List, type##ToSolve);  

#include "Physics/Joints/JointList.hpp"
#undef JointType
  //get a list of all contacts that need position correction (currently all or nothing)
  ContactList contactsToSolve;
  CollectContactsToSolve(mContacts, contactsToSolve, mSolverConfig);

  for(uint iterationCount = 0; iterationCount < GetSolverPositionIterationCount(); ++iterationCount)
  {
    //solve each joint list
#define JointType(type)                                                   \
    BlockSolvePositions(type##ToSolve, EmptyUpdate<Joint>);

    #include "Physics/Joints/JointList.hpp"
#undef JointType

    BlockSolvePositions(contactsToSolve, ContactUpdate);
  }

  //splice each list of joints we solved back into the main list
#define JointType(type)                                                  \
  if(!type##ToSolve.Empty())                                             \
    m##type##List.Splice(m##type##List.End(), type##ToSolve.All());

  #include "Physics/Joints/JointList.hpp"
#undef JointType

  if(!contactsToSolve.Empty())
    mContacts.Splice(mContacts.End(), contactsToSolve.All());
}

void NormalSolver::Commit()
{
  MoleculeWalker molecules(mMolecules.Data(),sizeof(ConstraintMolecule),0);

#define JointType(type) \
  CommitFragmentList(m##type##List,molecules);

#include "Physics/Joints/JointList.hpp"

#undef JointType

  CommitFragmentList(mContacts,molecules);
}

void NormalSolver::BatchEvents()
{
#define JointType(type) \
  BatchEventsFragmentList(m##type##List);

#include "Physics/Joints/JointList.hpp"

#undef JointType
}

}//namespace Physics

}//namespace Zero
