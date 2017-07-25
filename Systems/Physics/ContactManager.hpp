///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
namespace Memory{class Pool;}

namespace Physics
{

/// Manages memory allocation and deallocation for contacts.
/// Used when a manifold is to be reported to the constraint solver
/// or when a contact is removing itself from the solver.
class ContactManager
{
public:
  ContactManager();
  ~ContactManager();

  /// Gets the existing contact for this manifold or creates a new one if none exists.
  /// Used when a collision has been detected.
  Contact* AddManifold(Manifold& manifold);
  /// Used when a collision no longer should exist.
  void RemoveManifold(Manifold* manifold);
  /// Used when a contact should be removed, maybe due to object deletion.
  void Remove(Contact* contact, bool sendImmediately = false);

  /// Delete contacts that had been queued for delay destruction.
  void DestroyContacts();

  PhysicsSpace* mSpace;
private:
  
  typedef InList<Contact, &Contact::SolverLink> ContactList;
  ContactList mContactsToDestroy;

  Memory::Pool* mContactPool;
};

/// Checks if a contact already exists for a given manifold. Returns nullptr
/// if none existed.
Contact* ContactAlreadyExistsNew(Manifold* manifold);
Contact* ContactAlreadyExistsDebug(Manifold* manifold);

}//namespace Physics

}//namespace Zero
