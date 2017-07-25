///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Physics
{

size_t GetMaxSolverSize()
{
  size_t basicSolver = sizeof(BasicSolver);
  size_t normalSolver = sizeof(NormalSolver);
  size_t basicGenericSolver = sizeof(GenericBasicSolver);
  
  return Math::Max(basicSolver, Math::Max(normalSolver,basicGenericSolver));
}

Memory::Pool* IConstraintSolver::sPool = 
  new Memory::Pool("Solvers", Memory::GetNamedHeap("Physics"), GetMaxSolverSize() , 512 );

ImplementOverloadedNewWithAllocator(IConstraintSolver, IConstraintSolver::sPool);

void IConstraintSolver::AddJoints(JointList& joints)
{
  while(!joints.Empty())
  {
    Joint* joint = &(joints.Front());
    joints.Unlink(joint);
    AddJoint(joint);
  }
}

void IConstraintSolver::AddContacts(ContactList& contacts)
{
  while(!contacts.Empty())
  {
    Contact* contact = &(contacts.Front());
    contacts.Unlink(contact);
    AddContact(contact);
  }
}

uint IConstraintSolver::GetSolverIterationCount() const
{
  return mSolverConfig->mSolverIterationCount;
}

uint IConstraintSolver::GetSolverPositionIterationCount() const
{
  return mSolverConfig->mPositionIterationCount;
}

}//namespace Physics

}//namespace Zero
