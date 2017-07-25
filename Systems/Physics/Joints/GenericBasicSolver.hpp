/////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010, DigiPen Institute of Technology
///
/////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{

///A solver that copies data into an array for quick solving. This does not allow
///joints to change during each iteration. Since it is a basic solver, there
///are virtual function calls during the set up phase, but during actual solving
///there are none. Also copies RigidBody data to a small array to help cache.
class GenericBasicSolver : public IConstraintSolver
{
public:
  GenericBasicSolver();
  ~GenericBasicSolver();

  // IConstraintSolver Interface
  void AddJoint(Joint* joint) override;
  void AddContact(Contact* contact) override;
  void AddJoints(JointList& joints) override;
  void AddContacts(ContactList& contacts) override;
  // Solving functions
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

  ///An entry for a constraint. Contains the fragment and the
  ///id of each object in the object data array.
  struct ConstraintEntry
  {
    ConstraintEntry();

    ConstraintMolecule Fragment;
    uint Obj1Index,Obj2Index;
  };
  typedef Array<ConstraintEntry> EntryArray;

  ///The basics of a RigidBody needed to solve a constraint.
  struct ConstraintObjectData
  {
    ConstraintObjectData();
    void SetBody(RigidBody* body);
    void ApplyImpulse(Vec3Param linear, Vec3Param angular);
    void CommitVelocities();

    Vec3 Velocity;
    Vec3 AngularVelocity;

    Physics::Mass mInvMass;
    real InverseMass;
    Mat3 InverseInertiaTensor;
    RigidBody* Body;
  };
  typedef Array<ConstraintObjectData> ObjectArray;

  template <typename JointType> void CreateEntry(JointType& joint, MoleculeWalker& fragments);
  void CreateEntriesandObjects();
  uint FindRigidBodyIndex(Collider* collider);


  typedef InList<Joint,&Joint::SolverLink> JointList;
  typedef InList<Contact,&Contact::SolverLink> ContactList;
  typedef Array<ConstraintMolecule> MoleculeList;
  JointList mJoints;
  ContactList mContacts;

  EntryArray mEntries;
  ObjectArray mObjects;
  HashMap<RigidBody*,uint> mLookupMap;

  uint mEntryCount;
};
  
}//namespace Physics

}//namespace Zero
