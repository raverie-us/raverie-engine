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
Memory::Pool* sContactPool = nullptr;

ContactManager::ContactManager()
{
  if(sContactPool == nullptr)
    sContactPool = new Memory::Pool("Contacts", Memory::GetNamedHeap("Physics"), sizeof(Contact), 1000);
  mContactPool = sContactPool;
  mSpace = nullptr;
}

ContactManager::~ContactManager()
{
  // Make sure to destroy all queued up contacts
  DestroyContacts();
}

Contact* ContactManager::AddManifold(Manifold& manifold)
{
  // Correct this manifold for 2d if it needs to be. If this returns
  // false then there are no points left in the manifold and
  // therefore there is no point in adding it except for events.
  bool valid = manifold.CorrectFor2D();

  PhysicsEventManager* eventManager = mSpace->mEventManager;

  Contact* contact = ContactAlreadyExistsNew(&manifold);
  // If the contact didn't already exist, create it
  if(!contact)
  {
    contact = mContactPool->AllocateType<Contact>();

    contact->mContactManager = this;
    contact->SetPair(manifold.Objects);
    contact->SetManifold(new Manifold(manifold));
    ++contact->GetCollider(0)->mContactCount;
    ++contact->GetCollider(1)->mContactCount;

    // If it was not valid, then make it not active so it doesn't resolve,
    // but it will persist so collision events will work properly.
    if(!valid)
      contact->SetActive(false);

    // Can't change state in the middle of looping, otherwise broadphase
    // can get false positives due to order dependency on contact adding.
    
    eventManager->BatchCollisionStartedEvent(contact->mManifold, mSpace);
  }
  else
  {
    contact->UpdateManifold(&manifold);
    eventManager->BatchCollisionPersistedEvent(contact->mManifold, mSpace);
  }
  // Whether or not this is a start or persisted collision,
  // we should batch up pre-solve events
  eventManager->BatchPreSolveEvent(contact->mManifold, mSpace);

  return contact;
}

void ContactManager::RemoveManifold(Manifold* manifold)
{
  Contact* contact = ContactAlreadyExistsNew(manifold);
  // If the contact existed for this manifold, remove it
  if(contact)
    Remove(contact, false);
}

void ContactManager::Remove(Contact* contact, bool sendImmediately)
{
  // Unlink the contact from the collider's and the constraint solvers
  --contact->GetCollider(0)->mContactCount;
  --contact->GetCollider(1)->mContactCount;
  contact->UnLinkPair();

  // Wake up both objects.
  Manifold* manifold = contact->GetManifold();

  //if(manifold->ContactCount > 0)
  {
    manifold->Objects.A->ForceAwake();
    manifold->Objects.B->ForceAwake();
  }

  mSpace->mEventManager->BatchCollisionEndedEvent(manifold, mSpace, sendImmediately);

  // We want the CollisionEnded event to have access to the manifold, but the contact
  // owns the manifold and would delete it, so delay destruct the contacts
  mContactsToDestroy.PushBack(contact);
}

void ContactManager::DestroyContacts()
{
  // Actually delete the memory of all the contacts
  while(!mContactsToDestroy.Empty())
  {
    Contact* contact = &mContactsToDestroy.Front();
    mContactsToDestroy.PopFront();

    mContactPool->DeallocateType(contact);
  }
}

Contact* ContactAlreadyExistsNew(Manifold* manifold)
{
  Collider* collider1 = manifold->Objects[0];
  Collider* collider2 = manifold->Objects[1];
  Collider* otherCollider = nullptr;
  Contact* contact = nullptr;

  Collider::ContactEdgeList::range range;
  if(collider1->mContactCount < collider2->mContactCount)
  {
    range = collider1->mContactEdges.All();
    otherCollider = collider2;
  }
  else
  {
    range = collider2->mContactEdges.All();
    otherCollider = collider1;
  }

  // Loop through all of the edges on object 1 and check to see if any of them
  // are connected to the other object in the manifold.
  while(!range.Empty())
  {
    ContactEdge& edge = range.Front();
    range.PopFront();

    // Constraint is not between the two objects of the manifold, don't care
    if(edge.mOther != otherCollider)
      continue;

    if(edge.mContact->GetManifold()->ContactId != manifold->ContactId)
      continue;

    // The objects on this constraint match the manifold, we want this one.
    contact = edge.mContact;
    break;
  }
  return contact;
}

Contact* ContactAlreadyExistsDebug(Manifold* manifold)
{
  Collider* collider1 = manifold->Objects[0];
  Collider* collider2 = manifold->Objects[1];
  Contact* contact1 = nullptr,
         * contact2 = nullptr;

  Collider::ContactEdgeList::range range = collider1->mContactEdges.All();
  // Loop through all of the edges on object 1 and check to see if any of them
  // are connected to the other object in the manifold.
  while(!range.Empty())
  {
    ContactEdge& edge = range.Front();
    range.PopFront();

    // Constraint is not between the two objects of the manifold, don't care
    if(edge.mOther != collider2)
      continue;

    if(edge.mContact->GetManifold()->ContactId != manifold->ContactId)
      continue;

    // The objects on this constraint match the manifold, we want this one.
    contact1 = edge.mContact;
    break;
  }

  range = collider2->mContactEdges.All();
  // Loop through all of the edges on object 1 and check to see if any of them
  // are connected to the other object in the manifold.
  while(!range.Empty())
  {
    ContactEdge& edge = range.Front();
    range.PopFront();

    // Constraint is not between the two objects of the manifold, don't care
    if(edge.mOther != collider1)
      continue;

    if(edge.mContact->GetManifold()->ContactId != manifold->ContactId)
      continue;

    // The objects on this constraint match the manifold, we want this one.
    contact2 = edge.mContact;
    break;
  }

  ErrorIf(contact1 != contact2, "Contact was on one collider but not the other!");

  return contact1;
}

}//namespace Physics

}//namespace Zero
