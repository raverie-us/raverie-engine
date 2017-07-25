/////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010, DigiPen Institute of Technology
///
/////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Physics
{

GenericBasicSolver::GenericBasicSolver()
{
  mEntryCount = 0;
}

GenericBasicSolver::~GenericBasicSolver()
{

}

void GenericBasicSolver::AddJoint(Joint* joint)
{
  joint->mSolver = this;
  joint->UpdateAtomsVirtual();
  mEntryCount += joint->MoleculeCountVirtual();
  mJoints.PushBack(joint);
}

void GenericBasicSolver::AddContact(Contact* contact)
{
  contact->mSolver = this;
  contact->UpdateAtoms();
  mEntryCount += contact->MoleculeCount();
  mContacts.PushBack(contact);
}

void GenericBasicSolver::AddJoints(JointList& joints)
{
  JointList::range range = joints.All();
  for(; !range.Empty(); range.PopFront())
  {
    Joint* joint = &(range.Front());
    joint->mSolver = this;
    joint->UpdateAtomsVirtual();
    mEntryCount += joint->MoleculeCountVirtual();
  }
  mJoints.Splice(mJoints.End(),joints.All());
}

void GenericBasicSolver::AddContacts(ContactList& contacts)
{
  ContactList::range range = contacts.All();
  for(; !range.Empty(); range.PopFront())
  {
    Contact* contact = &(range.Front());
    contact->mSolver = this;
    contact->UpdateAtoms();
    mEntryCount += contact->MoleculeCount();
  }
  mContacts.Splice(mContacts.End(),contacts.All());
}

void GenericBasicSolver::Solve(real dt)
{
  GenericBasicSolver::UpdateData();
  GenericBasicSolver::WarmStart();
  GenericBasicSolver::SolveVelocities();
  GenericBasicSolver::Commit();
  GenericBasicSolver::BatchEvents();
}

void GenericBasicSolver::DebugDraw(uint debugFlags)
{
  if(!(debugFlags & PhysicsSpaceDebugDrawFlags::DrawConstraints))
    return;

  DrawJointsFragmentList(mJoints);
  DrawJointsFragmentList(mContacts);
}

void GenericBasicSolver::Clear()
{
  ClearFragmentList(mJoints);
  ClearFragmentList(mContacts);

  mEntries.Clear();
  mObjects.Clear();
  mLookupMap.Clear();
}

void GenericBasicSolver::UpdateData()
{
  mEntries.Resize(mEntryCount);

  CreateEntriesandObjects();
}

void GenericBasicSolver::WarmStart()
{
  if(mSolverConfig->mWarmStart == false)
    return;

  //initialize all of the constraint entries with the old accumulated impulse
  //and the new Jacobian
  for(uint i = 0; i < mEntries.Size(); ++i)
  {
    ConstraintEntry& entry = mEntries[i];
    ConstraintObjectData& obj1 = mObjects[entry.Obj1Index];
    ConstraintObjectData& obj2 = mObjects[entry.Obj2Index];
  
    Vec3 lineaerImpulse1 = entry.Fragment.mJacobian.Linear[0] * entry.Fragment.mImpulse;
    Vec3 lineaerImpulse2 = entry.Fragment.mJacobian.Linear[1] * entry.Fragment.mImpulse;
    Vec3 angularImpulse1 = entry.Fragment.mJacobian.Angular[0] * entry.Fragment.mImpulse;
    Vec3 angularImpulse2 = entry.Fragment.mJacobian.Angular[1] * entry.Fragment.mImpulse;
    obj1.ApplyImpulse(lineaerImpulse1,angularImpulse1);
    obj2.ApplyImpulse(lineaerImpulse2,angularImpulse2);
  }
}

void GenericBasicSolver::SolveVelocities()
{
  for(uint i = 0; i < GetSolverIterationCount(); ++i)
    IterateVelocities(i);
}

void GenericBasicSolver::IterateVelocities(uint iteration)
{
  for(uint i = 0; i < mEntries.Size(); ++i)
  {
    ConstraintEntry& entry = mEntries[i];
    
    ConstraintObjectData& obj1 = mObjects[entry.Obj1Index];
    ConstraintObjectData& obj2 = mObjects[entry.Obj2Index];

    JointVelocity velocity(obj1.Velocity,obj1.AngularVelocity,obj2.Velocity,obj2.AngularVelocity);
    real lambda = ComputeLambda(entry.Fragment,velocity);
    
    //apply the constraint
    Vec3 lineaerImpulse1 = entry.Fragment.mJacobian.Linear[0] * lambda;
    Vec3 lineaerImpulse2 = entry.Fragment.mJacobian.Linear[1] * lambda;
    Vec3 angularImpulse1 = entry.Fragment.mJacobian.Angular[0] * lambda;
    Vec3 angularImpulse2 = entry.Fragment.mJacobian.Angular[1] * lambda;
    obj1.ApplyImpulse(lineaerImpulse1,angularImpulse1);
    obj2.ApplyImpulse(lineaerImpulse2,angularImpulse2);
  }
}

void GenericBasicSolver::SolvePositions()
{
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

void GenericBasicSolver::Commit()
{
  //commit the new velocities back to the objects
  for(uint i = 0; i < mObjects.Size(); ++i)
  {
    mObjects[i].CommitVelocities();
  }
  
  MoleculeWalker fragments(mEntries.Data(),sizeof(ConstraintEntry),0);


  //commit the entry back to the constraint, this is done by giving the
  //entries to the constraint and having it extract the necessary data from
  //the entry. The reason for this is that there are multiple entries per
  //constraint and it is easier to have the constraint handle that unmapping
  //than the generic solver.
  JointList::range jointRange = mJoints.All();
  while(!jointRange.Empty())
  {
    Joint& joint = jointRange.Front();
    jointRange.PopFront();
    joint.CommitVirtual(fragments);
  }

  ContactList::range contactRange = mContacts.All();
  while(!contactRange.Empty())
  {
    Contact& contact = contactRange.Front();
    contactRange.PopFront();
    contact.Commit(fragments);
  }
}

void GenericBasicSolver::BatchEvents()
{
  BatchEventsFragmentList(mJoints);
}

GenericBasicSolver::ConstraintEntry::ConstraintEntry()
{
  Obj1Index = (uint)-1;
  Obj2Index = (uint)-1;
}

GenericBasicSolver::ConstraintObjectData::ConstraintObjectData()
{
  Body = nullptr;
}

void GenericBasicSolver::ConstraintObjectData::SetBody(RigidBody* body)
{
  Body = body;
  if(body == nullptr)
  {
    Velocity.ZeroOut();
    AngularVelocity.ZeroOut();
    mInvMass.SetInvMass(0);
    InverseInertiaTensor.ZeroOut();
    return;
  }

  Velocity = body->mVelocity;
  AngularVelocity = body->mAngularVelocity;
  mInvMass = body->mInvMass;
  InverseInertiaTensor = body->mInvInertia.GetInvWorldTensor();
}

void GenericBasicSolver::ConstraintObjectData::ApplyImpulse(Vec3Param linear, Vec3Param angular)
{
  Velocity += mInvMass.Apply(linear);
  AngularVelocity += Math::Transform(InverseInertiaTensor,angular);
}

void GenericBasicSolver::ConstraintObjectData::CommitVelocities()
{
  if(Body)
  {
    Body->mVelocity = Velocity;
    Body->mAngularVelocity = AngularVelocity;
  }
}

template <typename JointType> 
void GenericBasicSolver::CreateEntry(JointType& joint, MoleculeWalker& fragments)
{
  uint fragmentCount = joint.MoleculeCountVirtual();

  //map the constraint entry to its corresponding constraint object data.
  //this should be made better than the n^2 algorithm it currently is
  uint obj1Index = FindRigidBodyIndex(joint.GetCollider(0));
  uint obj2Index = FindRigidBodyIndex(joint.GetCollider(1));
  for(uint i = 0; i < fragmentCount; ++i)
  {
    ConstraintEntry& entry = *reinterpret_cast<ConstraintEntry*>(&fragments[i]);
    entry.Obj1Index = obj1Index;
    entry.Obj2Index = obj2Index;
  }

  //get the entries from the given constraint
  joint.ComputeMoleculesVirtual(fragments);
}

void GenericBasicSolver::CreateEntriesandObjects()
{
  uint startIndex = 0;
  mEntries.Resize(mEntryCount);

  MoleculeWalker fragments(mEntries.Data(),sizeof(ConstraintEntry),0);

  //have to loop specially here since contacts do 
  //not have a computefragmentsvirtual function...

  JointList::range jointRange = mJoints.All();
  for(; !jointRange.Empty(); jointRange.PopFront())
  {
    Joint& joint = jointRange.Front();
    uint fragmentCount = joint.MoleculeCountVirtual();
    //map the constraint entry to its corresponding constraint object data.
    //this should be made better than the n^2 algorithm it currently is
    uint obj1Index = FindRigidBodyIndex(joint.GetCollider(0));
    uint obj2Index = FindRigidBodyIndex(joint.GetCollider(1));
    for(uint i = 0; i < fragmentCount; ++i)
    {
      ConstraintEntry& entry = *reinterpret_cast<ConstraintEntry*>(&fragments[i]);
      entry.Obj1Index = obj1Index;
      entry.Obj2Index = obj2Index;
    }

    //get the entries from the given constraint
    joint.ComputeMoleculesVirtual(fragments);
  }

  ContactList::range contactRange = mContacts.All();
  for(; !contactRange.Empty(); contactRange.PopFront())
  {
    Contact& contact = contactRange.Front();
    uint fragmentCount = contact.MoleculeCount();
    //map the constraint entry to its corresponding constraint object data.
    //this should be made better than the n^2 algorithm it currently is
    uint obj1Index = FindRigidBodyIndex(contact.GetCollider(0));
    uint obj2Index = FindRigidBodyIndex(contact.GetCollider(1));
    for(uint i = 0; i < fragmentCount; ++i)
    {
      ConstraintEntry& entry = *reinterpret_cast<ConstraintEntry*>(&fragments[i]);
      entry.Obj1Index = obj1Index;
      entry.Obj2Index = obj2Index;
    }

    //get the entries from the given constraint
    contact.ComputeMolecules(fragments);
  }
}

uint GenericBasicSolver::FindRigidBodyIndex(Collider* collider)
{
  uint notFoundVal = uint(-1);
  RigidBody* body = collider->GetActiveBody();
  uint index = mLookupMap.FindValue(body,notFoundVal);
  uint objectCount = mObjects.Size();
  if(index == notFoundVal)
  {
    ConstraintObjectData objData;
    objData.SetBody(body);
    mObjects.PushBack(objData);
    //form the map reference
    mLookupMap[body] = objectCount;
    index = objectCount;
  }
  return index;

  

  //RigidBody* body = collider->GetBody();
  ////loop over the current constraint objects
  //uint objectCount = mObjects.Size();
  //for(uint i = 0; i < objectCount; ++i)
  //{
  //  ConstraintObjectData& cObject = mObjects[i];
  //  if(cObject.Body == body)
  //  {
  //    //if there actually was a body, then this is the correct index
  //    //of the object
  //    if(body)
  //      return i;
  //  }
  //}

  ////if we made it through the loop, then there was no match, so add a new 
  ////constraint object and return that index
  //ConstraintObjectData objData;
  //objData.SetBody(body);
  //mObjects.PushBack(objData);
  //return objectCount;
}

}//namespace Physics

}//namespace Zero
