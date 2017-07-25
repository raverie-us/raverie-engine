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

BasicSolver::BasicSolver()
{
  SetConfiguration(nullptr);
  mConstraintCount = 0;
}

BasicSolver::~BasicSolver()
{
  Clear();
}

void BasicSolver::AddJoint(Joint* joint)
{
  joint->mSolver = this;
  joint->UpdateAtomsVirtual();
  mConstraintCount += joint->MoleculeCountVirtual();
  mJoints.PushBack(joint);
}

void BasicSolver::AddContact(Contact* contact)
{
  contact->mSolver = this;
  contact->UpdateAtoms();
  mConstraintCount += contact->MoleculeCount();
  mContacts.PushBack(contact);
}

void BasicSolver::AddJoints(JointList& joints)
{
  JointList::range range = joints.All();
  for(; !range.Empty(); range.PopFront())
  {
    Joint* joint = &(range.Front());
    joint->mSolver = this;
    joint->UpdateAtomsVirtual();
    mConstraintCount += joint->MoleculeCountVirtual();
  }
  mJoints.Splice(mJoints.End(),joints.All());
}

void BasicSolver::AddContacts(ContactList& contacts)
{
  ContactList::range range = contacts.All();
  for(; !range.Empty(); range.PopFront())
  {
    Contact* contact = &(range.Front());
    contact->mSolver = this;
    contact->UpdateAtoms();
    mConstraintCount += contact->MoleculeCount();
  }
  mContacts.Splice(mContacts.End(),contacts.All());
}

///Necessary solving functions
void BasicSolver::Solve(real dt)
{
  BasicSolver::UpdateData();
  BasicSolver::WarmStart();
  BasicSolver::SolveVelocities();
  BasicSolver::Commit();
  BasicSolver::BatchEvents();

  //can't solve positions here as this step needs to run after position integration
  //BasicSolver::SolvePositions();
}

void BasicSolver::DebugDraw(uint debugFlags)
{
  if(debugFlags & PhysicsSpaceDebugDrawFlags::DrawConstraints)
    DrawJoints(debugFlags);
}

void BasicSolver::Clear()
{
  ClearFragmentList(mJoints);
  ClearFragmentList(mContacts);
}

void BasicSolver::UpdateData()
{
  mMolecules.Resize(mConstraintCount);

  MoleculeWalker molecules(mMolecules.Data(),sizeof(ConstraintMolecule),0);

  UpdateDataFragmentList(mJoints,molecules);
  UpdateDataFragmentList(mContacts,molecules);
}

void BasicSolver::WarmStart()
{
  if(mSolverConfig->mWarmStart == false)
    return;

  MoleculeWalker molecules(mMolecules.Data(),sizeof(ConstraintMolecule),0);

  WarmStartFragmentList(mJoints,molecules);
  WarmStartFragmentList(mContacts,molecules);
}

void BasicSolver::SolveVelocities()
{
  ProfileScopeTree("SolveVelocities", "ResolutionPhase", Color::DarkMagenta);

  //solve all of the velocity constraints the given number of times
  for(uint i = 0; i < GetSolverIterationCount(); ++i)
    IterateVelocities(i);
}

void BasicSolver::IterateVelocities(uint iteration)
{
  MoleculeWalker molecules(mMolecules.Data(),sizeof(ConstraintMolecule),0);

  IterateVelocitiesFragmentList(mJoints,molecules,iteration);
  IterateVelocitiesFragmentList(mContacts,molecules,iteration);
}

void BasicSolver::SolvePositions()
{
  //first do a pre-processing step to figure out which joints/contacts actually
  //need position correction (so we're not doing the check during the inner loop)
  JointList jointsToSolve;
  ContactList contactsToSolve;

  CollectJointsToSolve(mJoints, jointsToSolve);
  CollectContactsToSolve(mContacts, contactsToSolve, mSolverConfig);

  for(uint iterationCount = 0; iterationCount < GetSolverPositionIterationCount(); ++iterationCount)
  {
    if(mSolverConfig->mSubType == PhysicsSolverSubType::BasicSolving)
    {
      SolveConstraintPosition(jointsToSolve, EmptyUpdate<Joint>);
      SolveConstraintPosition(contactsToSolve, ContactUpdate);
    }
    else
    {
      BlockSolvePositions(jointsToSolve, EmptyUpdate<Joint>);
      BlockSolvePositions(contactsToSolve, ContactUpdate);
    }
  }

  //make sure to put the joints and contacts back into the main
  //list so we'll visit them again next frame
  if(!jointsToSolve.Empty())
    mJoints.Splice(mJoints.End(), jointsToSolve.All());
  if(!contactsToSolve.Empty())
    mContacts.Splice(mContacts.End(), contactsToSolve.All());
}

void BasicSolver::Commit()
{
  MoleculeWalker molecules(mMolecules.Data(),sizeof(ConstraintMolecule),0);

  CommitFragmentList(mJoints,molecules);
  CommitFragmentList(mContacts,molecules);
}

void BasicSolver::BatchEvents()
{
  BatchEventsFragmentList(mJoints);
}

void BasicSolver::DrawJoints(uint debugFlag)
{
  DrawJointsFragmentList(mJoints);
  DrawJointsFragmentList(mContacts);
}

}//namespace Physics

}//namespace Zero
