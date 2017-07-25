///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

template <typename ListType>
void ThreadedDrawFragment(ListType& jointList, ByteColor& color)
{
  typename ListType::range jointRange = jointList.All();
  while(!jointRange.Empty())
  {
    typename ListType::sub_reference joint = jointRange.Front();
    jointRange.PopFront();

    Zero::Vec3 posA = joint.GetCollider(0)->GetWorldTranslation();
    Zero::Vec3 posB = joint.GetCollider(1)->GetWorldTranslation();
    Zero::gDebugDraw->Add(Zero::Debug::Line(posA,posB).Color(color));
  }

  color = (color << 8) ^ Color::Red;
}

namespace Zero
{

namespace Physics
{

template <typename JointList>
void ThreadSolveFunction(JointList& joints, MoleculeWalker molecules, uint startIndex)
{
  MoleculeWalker mols = molecules;
  mols += startIndex;

  IterateVelocitiesFragment(joints,mols,0);
}

ThreadedSolver::ThreadedSolver()
{
  mConstraintCount = 0;
}

ThreadedSolver::~ThreadedSolver()
{
  Clear();
}

void ThreadedSolver::AddJoint(Joint* joint)
{
  joint->mSolver = this;
  joint->UpdateAtomsVirtual();
  mConstraintCount += joint->MoleculeCountVirtual();
  mJoints.PushBack(joint);
}

void ThreadedSolver::AddContact(Contact* contact)
{
  contact->mSolver = this;
  contact->UpdateAtoms();
  mConstraintCount += contact->MoleculeCount();
  mContacts.PushBack(contact);
}

void ThreadedSolver::AddJoints(JointList& joints)
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

void ThreadedSolver::AddContacts(ContactList& contacts)
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

void ThreadedSolver::Solve(real dt)
{
  ThreadedSolver::UpdateData();
  ThreadedSolver::WarmStart();
  ThreadedSolver::SolveVelocities();
  ThreadedSolver::Commit();
  ThreadedSolver::BatchEvents();
}

void ThreadedSolver::DebugDraw(uint debugFlags)
{
  if(debugFlags & PhysicsSpaceDebugDrawFlags::DrawConstraints)
    DrawJoints(debugFlags);

  //ByteColor color = Color::Black;
  //GroupOperationParamFragment<JointList>(mJointPhases,color,ThreadedDrawFragment<JointList>);
}

void ThreadedSolver::Clear()
{
  ClearFragmentList(mJoints);
  ClearFragmentList(mContacts);

  GroupOperationFragment<ContactList>(mContactPhases,ClearFragmentList<ContactList>);
  GroupOperationFragment<JointList>(mJointPhases,ClearFragmentList<JointList>);

  mJointPhases.Clear();
  mContactPhases.Clear();
}

void ThreadedSolver::UpdateData()
{
  mMolecules.Resize(mConstraintCount);

  MoleculeWalker molecules(mMolecules.Data(),sizeof(ConstraintMolecule),0);

  SplitConstraints(mContacts,mContactPhases);
  SplitConstraints(mJoints,mJointPhases);

  GroupOperationParamFragment<ContactList>(mContactPhases,molecules,UpdateDataFragmentList<ContactList>);
  GroupOperationParamFragment<JointList>(mJointPhases,molecules,UpdateDataFragmentList<JointList>);
}

void ThreadedSolver::WarmStart()
{
  if(mSolverConfig->mWarmStart == false)
    return;

  MoleculeWalker molecules(mMolecules.Data(),sizeof(ConstraintMolecule),0);

  GroupOperationParamFragment<ContactList>(mContactPhases,molecules, WarmStartFragmentList<ContactList>);
  GroupOperationParamFragment<JointList>(mJointPhases,molecules, WarmStartFragmentList<JointList>);
}

void ThreadedSolver::SolveVelocities()
{
  //solve all of the velocity constraints the given number of times
  for(uint i = 0; i < GetSolverIterationCount(); ++i)
    IterateVelocities(i);
}

void ThreadedSolver::IterateVelocities(uint iteration)
{
  MoleculeWalker molecules(mMolecules.Data(),sizeof(ConstraintMolecule),0);

  GroupOperationTwoParamFragment<ContactList>(mContactPhases,molecules,iteration,IterateVelocitiesFragmentList<ContactList>);
  GroupOperationTwoParamFragment<JointList>(mJointPhases,molecules,iteration,IterateVelocitiesFragmentList<JointList>);
}

void ThreadedSolver::SolvePositions()
{
  //first have to re-collect all of the joints and contacts so we
  //can prune out the ones we don't solve positions on
  GroupOperationParamFragment<ContactList>(mContactPhases, mContacts, CollectJoints<ContactList>);
  GroupOperationParamFragment<JointList>(mJointPhases, mJoints, CollectJoints<JointList>);
  
  //Could in theory split back onto lists to thread, but that's a rather expensive operation.
  //Maybe just prune each list, but unfortunately then there'd be unbalanced lists.

  //first do a pre-processing step to figure out which joints/contacts actually
  //need position correction (so we're not doing the check during the inner loop)
  JointList jointsToSolve;
  ContactList contactsToSolve;

  CollectJointsToSolve(mJoints, jointsToSolve);
  CollectContactsToSolve(mContacts, contactsToSolve, mSolverConfig);

  for(uint iterationCount = 0; iterationCount < GetSolverPositionIterationCount(); ++iterationCount)
  {
    BlockSolvePositions(jointsToSolve, EmptyUpdate<Joint>);
    BlockSolvePositions(contactsToSolve, ContactUpdate);
  }

  //make sure to put the joints and contacts back into the main
  //list so we'll visit them again next frame
  if(!jointsToSolve.Empty())
    mJoints.Splice(mJoints.End(), jointsToSolve.All());
  if(!contactsToSolve.Empty())
    mContacts.Splice(mContacts.End(), contactsToSolve.All());
}

void ThreadedSolver::Commit()
{
  MoleculeWalker molecules(mMolecules.Data(),sizeof(ConstraintMolecule),0);

  GroupOperationParamFragment<ContactList>(mContactPhases,molecules,CommitFragmentList<ContactList>);
  GroupOperationParamFragment<JointList>(mJointPhases,molecules,CommitFragmentList<JointList>);
}

void ThreadedSolver::BatchEvents()
{
  GroupOperationFragment<JointList>(mJointPhases,BatchEventsFragmentList<JointList>);
}

void ThreadedSolver::DrawJoints(uint debugFlag)
{
  GroupOperationFragment<ContactList>(mContactPhases,DrawJointsFragmentList<ContactList>);
  GroupOperationFragment<JointList>(mJointPhases,DrawJointsFragmentList<JointList>);
}

}//namespace Physics

}//namespace Zero
