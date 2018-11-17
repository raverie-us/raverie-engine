///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class RigidBody;

namespace Physics
{

struct ColliderPair;
struct MoleculeWalker;
struct Manifold;
class Contact;
class ContactManager;
class IConstraintSolver;

DeclareBitField6(ContactFlags,OnIsland, Ghost, SkipsResolution, Valid, NewContact, Active);

///A constraint specifically for solving a non-penetration constraint.
///This should not be created anywhere but in the constraint solver.
class Contact
{
public:
  typedef ContactEdge EdgeType;

  Contact();
  virtual ~Contact();

  bool GetOnIsland() const;
  void SetOnIsland(bool onIsland);
  bool GetGhost() const;
  void SetGhost(bool ghost);
  bool GetSkipResolution() const;
  void SetSkipResolution(bool skip);
  bool GetValid() const;
  void SetValid(bool valid);
  /// Not to be confused with Valid. This marks the contact to not resolve
  /// collision but still be a valid contact (and not get deleted immediately).
  /// Used currently for z axis contact in 2d.
  bool GetActive() const;
  void SetActive(bool active);
  bool GetIsNew() const;
  bool GetSendsEvents() const;

  void UnLinkPair();
  virtual void Destroy(bool sendImmediately = false);

  void SetPair(ColliderPair& pair);
  void SetManifold(Manifold* manifold);
  void UpdateManifold(Manifold* manifold);
  void UpdateManifoldInternal(Manifold* manifold);
  Manifold* GetManifold();

  ///Returns how many contacts this constraint is over.
  uint GetContactCount() const;


  virtual void UpdateAtoms();
  virtual uint MoleculeCount() const;
  virtual void ComputeMolecules(MoleculeWalker& fragments);
  virtual void WarmStart(MoleculeWalker& fragments);
  virtual void Solve(MoleculeWalker& fragments);
  void SolveSse(MoleculeWalker& fragments);
  virtual void Commit(MoleculeWalker& fragments);
  uint PositionMoleculeCount() const;
  void ComputePositionMolecules(MoleculeWalker& fragments);

  virtual void DebugDraw();
  virtual uint GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const;

  bool GetShouldBaumgarteBeUsed() const;
  real GetLinearBaumgarte() const;
  real GetLinearErrorCorrection() const;
  //Not used, but needed for an interface
  real GetAngularErrorCorrection() const {return 0;};

  Collider* GetCollider(uint index) const;

  
  Link<Contact> SolverLink;

  EdgeType mEdges[2];

  ContactManager* mContactManager;
  IConstraintSolver* mSolver;

//private:

  Manifold* mManifold;

  BitField<ContactFlags::Enum> mFlags;
};

}//namespace Physics

}//namespace Zero
