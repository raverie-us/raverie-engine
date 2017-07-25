///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Physics
{

Memory::Pool* Island::sPool = new Memory::Pool("Islands", Memory::GetNamedHeap("Physics"), sizeof(Island), 4096 );

void* Island::operator new(size_t size)
{
  return sPool->Allocate(size);
}
void Island::operator delete(void* pMem, size_t size)
{
  return sPool->Deallocate(pMem, size);
}

Island::Island()
{
  mSolver = nullptr;

  ContactCount = 0;
  JointCount = 0;
  ColliderCount = 0;
  mOwnsSolver = true;
}

Island::~Island()
{
  Clear();
  if(mOwnsSolver)
    delete mSolver;
}

void Island::SetSolver(IConstraintSolver* solver, bool ownsSolver)
{
  mSolver = solver;
  mOwnsSolver = ownsSolver;
}

void Island::MergeIsland(Island& island)
{
  ColliderCount += island.ColliderCount;
  ContactCount += island.ContactCount;
  JointCount += island.JointCount;

  
  mColliders.Splice(mColliders.End(), island.mColliders);
  if(!island.mJoints.Empty())
    mJoints.Splice(mJoints.End(), island.mJoints.All());
  if(!island.mContacts.Empty())
    mContacts.Splice(mContacts.End(), island.mContacts.All());
}

void Island::Add(Collider* collider)
{
  ++ColliderCount;

  mColliders.PushBack(collider);
}

void Island::Add(Contact* contact)
{
  Collider* collider1 = contact->GetCollider(0);
  Collider* collider2 = contact->GetCollider(1);
  contact->SetOnIsland(true);
  contact->SetValid(false);
  
  //if any object is a ghost then don't resolve
  if(collider1->NotCollideable() ||
    collider2->NotCollideable())
  {
    contact->SetGhost(true);
    // we still want to check to see if resolution should be skipped for not
    // applying region effects
    if(collider1->mCollisionGroupInstance->SkipResolution((*collider2->mCollisionGroupInstance)))
      contact->SetSkipResolution(true);
    return;
  }

  //if the collision group says to skip this,
  //then mark the contact as ghost so we don't resolve it
  if(collider1->mCollisionGroupInstance->SkipResolution((*collider2->mCollisionGroupInstance)))
  {
    contact->SetGhost(true);
    contact->SetSkipResolution(true);
    return;
  }

  //if the contact is marked as not active for any reason, don't add it. This
  //is different from valid because the contact needs to keep existing for as
  //long as the objects are in contact, but they should not be resolved.
  //Currently used to resolve 2d contacts that are almost completely in the z axis.
  if(!contact->GetActive())
    return;

  //If the contact was a ghost last frame but if the object's it connects to
  //are not ghosts, that means the state of the objects changed. We need to
  //mark the contact as being a normal contact again.
  if(contact->GetGhost())
  {
    contact->SetGhost(false);
    contact->SetSkipResolution(false);
  }

  ++ContactCount;
  mContacts.PushBack(contact);
}

void Island::Add(Joint* joint)
{
  ++JointCount;
  joint->SetOnIsland(true);


  // Deal with trying to solve joints that can't be solved (parent connected to child, object connected to itself, etc...)
  Collider* collider0 = joint->GetCollider(0);
  Collider* collider1 = joint->GetCollider(1);
  if(collider0 != nullptr && collider1 != nullptr)
  {
    RigidBody* body0 = collider0->GetActiveBody();
    RigidBody* body1 = collider1->GetActiveBody();

    // Get whatever the top-level body is (skip kinematics since we want to find if a kinematic is attached to its parent)
    while(body0 != nullptr && body0->GetKinematic())
      body0 = body0->mParentBody;
    while(body1 != nullptr && body1->GetKinematic())
      body1 = body1->mParentBody;

    // If the two bodies are the same then we effectively have a static connection which likely can't be
    // solved, "skip" solving the joint (by just adding it to another list of joints we don't solve)
    if(body0 == body1)
    {
      mUnSolvableJoints.PushBack(joint);
      return;
    }
  }

  mJoints.PushBack(joint);
}

void Island::IntegrateVelocity(real dt)
{
  
}

void Island::IntegratePosition(real dt)
{

}

void Island::CommitConstraints()
{
  if(!mJoints.Empty())
    mSolver->AddJoints(mJoints);
  if(!mContacts.Empty())
    mSolver->AddContacts(mContacts);
}

void Island::Solve(real dt, bool allowSleeping, uint debugFlags)
{
  CommitConstraints();
  mSolver->Solve(dt);
  UpdateSleep(dt, allowSleeping, debugFlags);
}

void Island::SolvePositions(real dt)
{
  mSolver->SolvePositions();
}

void Island::UpdateSleep(real dt, bool allowSleeping, uint debugFlags)
{
  if(!allowSleeping)
    return;

  real minSleepTime = Math::PositiveMax();

  //Update the sleep timers of all objects
  Colliders::range range = mColliders.All();

  while(!range.Empty())
  {
    RigidBody* body = range.Front().GetActiveBody();
    range.PopFront();

    if(!body || body->GetStatic())
      continue;

    if(body->UpdateSleepTimer(dt))
    {
      minSleepTime = Math::Min(minSleepTime, body->mSleepTimer);
    }
    else
    {
      if(debugFlags & PhysicsSpaceDebugDrawFlags::DrawSleepPreventors)
      {
        Aabb aabb = range.Front().mAabb;
        gDebugDraw->Add(Debug::Obb(aabb).Color(Color::Aquamarine));
      }

      minSleepTime = real(0.0);
    }
  }

  if(minSleepTime < cTimeToSleep)
    return;

  range = mColliders.All();

  while(!range.Empty())
  {
    RigidBody* body = range.Front().GetActiveBody();
    range.PopFront();

    if(!body || body->IsAsleep())
      continue;
    body->PutToSleep();
  }
}

void Island::ClearIslandFlags(Collider& collider)
{
  collider.mState.ClearFlag(ColliderFlags::OnIsland);
  
  Collider::JointEdgeList::range jointRange = collider.mJointEdges.All();
  for(; !jointRange.Empty(); jointRange.PopFront())
    jointRange.Front().mJoint->SetOnIsland(false);

  Collider::ContactEdgeList::range contactNewRange = collider.mContactEdges.All();
  for(; !contactNewRange.Empty(); contactNewRange.PopFront())
    contactNewRange.Front().mContact->SetOnIsland(false);

  //also clear out the dirty bit for sleep having been accumulated.
  RigidBody* body = collider.GetActiveBody();
  if(body)
    body->mState.ClearFlag(RigidBodyStates::SleepAccumulated);
}

void Island::Clear()
{
  Colliders::range range = mColliders.All();
  for(; !range.Empty(); range.PopFront())
    ClearIslandFlags(range.Front());

  mJoints.Clear();
  mContacts.Clear();
  mColliders.Clear();

  mUnSolvableJoints.Clear();
  mSolver->Clear();

  ContactCount = 0;
  JointCount = 0;
  ColliderCount = 0;
}

bool Island::ContainsCollider(const Collider* collider)
{
  Colliders::range range = mColliders.All();
  for(; !range.Empty(); range.PopFront())
  {
    Collider* c = &range.Front();
    if(c == collider)
      return true;
  }
  return false;
}

}//namespace Physics

}//namespace Zero
