///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Collider;
class JointSpring;
class PhysicsSpace;
struct JointMotor;
struct JointLimit;
class ObjectLinkEvent;
class ObjectLinkPointChangedEvent;

namespace Physics
{
class IConstraintSolver;
struct MoleculeWalker;
struct MoleculeData;
struct AngleAtom;
struct AnchorAtom;
}

namespace Tags
{
DeclareTag(Joint);
}

DeclareBitField8(JointFlags, OnIsland, Ghost, Valid, Active,
                             SendsEvents, AutoSnaps, CollideConnected, Initialized);

/// Joints connect two objects together with one or more constraints. A constraint
/// is a mathematical rule that restricts object movement, typically defined in
/// terms of the position and velocities of the objects involved.
struct Joint : public Component
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef JointEdge EdgeType;

  Joint();
  virtual ~Joint();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void OnDestroy(uint flags) override;

  /// Allows the individual joint to configure itself on dynamic addition
  virtual void ComputeInitialConfiguration() {};
  virtual void ObjectLinkPointUpdated(size_t edgeIndex, Vec3Param localPoint) {};
  /// Used to detect when motors/limits/springs are added dynamically.
  void ComponentAdded(BoundType* typeId, Component* component) override;
  /// Used to detect when motors/limits/springs are removed dynamically.
  void ComponentRemoved(BoundType* typeId, Component* component) override;

  // Joint Interface

  /// Update atoms is the step where a constraint computes the values (position
  /// difference, angle difference, etc...) of all of it's atoms. This info is
  /// then used to determine which atoms are actually active (dealing with limits).
  virtual void UpdateAtomsVirtual() {};
  /// Returns the number of fragments for the solver. A solver will often accumulate
  /// the total count of all atoms to efficiently create its molecule "array".
  virtual uint MoleculeCountVirtual() const { return 0; };
  /// Fills out the fragments for the solver. Must increment the walker.
  /// Compute molecules actually fills out the fragments for the solver. This
  /// step will actually compute the jacobian, mass terms, error values, etc...
  /// to be used during the solving step.
  virtual void ComputeMoleculesVirtual(Physics::MoleculeWalker& molecules) {};
  /// Allows the joint to run custom warm-string logic.
  virtual void WarmStartVirtual(Physics::MoleculeWalker& molecules) {};
  /// Allows the joint to run custom solving code (such as block solving).
  virtual void SolveVirtual(Physics::MoleculeWalker& molecules) {};
  /// Returns the number of fragments for position correction. Not all
  /// molecules may have position correction applied to them (e.g. springs).
  virtual uint PositionMoleculeCountVirtual() const { return 0; };
  /// Fills out the molecules with fragment data for all constraints requiring position correction.
  virtual void ComputePositionMoleculesVirtual(Physics::MoleculeWalker& molecules) {};
  /// Gives each joint a chance to copy and data from the temp molecule buffer back into
  /// its internal storage (such as the total impulse for each constraint atom).
  virtual void CommitVirtual(Physics::MoleculeWalker& molecules) {};
  virtual void DebugDrawVirtual() {};
  /// Given an atom's index, get the desired constraint value (solution to the position constraint).
  /// Also returns the atom type (AtomFilter: Linear or Angular).
  virtual uint GetAtomIndexFilterVirtual(uint atomIndex, real& desiredConstraintValue) const;
  virtual void BatchEventsVirtual() {};
  // Type identification
  virtual JointEnums::JointTypes GetJointType() const { return JointEnums::JointCount; }
  virtual cstr GetJointName() const { return "Joint"; }
  // @JoshD: Clean up the 255 to use AllAxes
  virtual uint GetDefaultLimitIds() const { return 255; };
  virtual uint GetDefaultMotorIds() const { return 255; };
  virtual uint GetDefaultSpringIds() const { return 255; };
  

  // Used so that the templated functions can still work with the base type...
  void UpdateAtoms() { UpdateAtomsVirtual(); };
  uint MoleculeCount() const { return MoleculeCountVirtual(); };
  void ComputeMolecules(Physics::MoleculeWalker& molecules) { ComputeMoleculesVirtual(molecules); };
  void WarmStart(Physics::MoleculeWalker& molecules) { WarmStartVirtual(molecules); };
  void Solve(Physics::MoleculeWalker& molecules) { SolveVirtual(molecules); };
  void Commit(Physics::MoleculeWalker& molecules) { CommitVirtual(molecules); };
  uint PositionMoleculeCount() const { return PositionMoleculeCountVirtual(); };
  void ComputePositionMolecules(Physics::MoleculeWalker& molecules) { ComputePositionMoleculesVirtual(molecules); };
  void DebugDraw() { DebugDrawVirtual(); };
  uint GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const { return GetAtomIndexFilterVirtual(atomIndex, desiredConstraintValue); };
  void BatchEvents() { BatchEventsVirtual(); };
  
  void SetPair(Physics::ColliderPair& pair);
  void LinkPair();
  void UnLinkPair();
  void Relink(uint index, Cog* cog);
  /// Used for a specific joint type (e.g. PulleyJoint) to reject a relink.
  /// This is the last step that happens when relinking, after the joint has set valid to true/false.
  virtual void SpecificJointRelink(uint index, Collider* collider) {};

  void OnObjectLinkChanged(ObjectLinkEvent* event);
  void OnObjectLinkPointChanged(ObjectLinkPointChangedEvent* e);

  uint GetActiveFilter() const;
  uint GetDefaultFilter() const;
  void ResetFilter();
  Collider* GetCollider(uint index) const;
  /// Returns the cog associated with an index. Index of 0 is ObjectA,
  /// index 1 is ObjectB. Used to write more streamline functions
  /// where you index into the objects in a loop.
  Cog* GetCog(uint index);
  /// If the passed in object is ObjectA, returns ObjectB.
  /// Provides easier logic for traversing across joints.
  Cog* GetOtherObject(Cog* cog);

  // Common helpers for joint atoms
  //------------------------------------------------------------------- AnchorAtom helpers
  /// Helper to get/set a local point on an anchor atom.
  Vec3 GetLocalPointHelper(const Physics::AnchorAtom& anchor, uint index) const;
  void SetLocalPointHelper(Physics::AnchorAtom& anchor, uint index, Vec3Param localPoint);
  /// Helpers to get/set world points on an anchor atom. The set for A/B are
  /// kept separate for marking side-effect properties.
  Vec3 GetWorldPointHelper(const Physics::AnchorAtom& anchor, uint index);
  void SetWorldPointAHelper(Physics::AnchorAtom& anchor, Vec3Param worldPoint);
  void SetWorldPointBHelper(Physics::AnchorAtom& anchor, Vec3Param worldPoint);
  void SetWorldPointHelper(Physics::AnchorAtom& anchor, Vec3Param worldPoint, uint index);
  /// Sets the position of the anchor on object A and B given a position in world space.
  void SetWorldPointsHelper(Physics::AnchorAtom& anchor, Vec3Param point);
  /// Helper function for when an object link point changes.
  void ObjectLinkPointUpdatedHelper(Physics::AnchorAtom& anchor, size_t edgeIndex, Vec3Param localPoint);
  
  //------------------------------------------------------------------- AxisAtom helpers
  /// Helpers to get/set a local space axis for an axis atom
  Vec3 GetLocalAxisHelper(const Physics::AxisAtom& axisAtom, uint index) const;
  void SetLocalAxisHelper(Physics::AxisAtom& axisAtom, uint index, Vec3Param localAxis);
  /// Helpers to get/set both local space axes from one world
  /// direction. Properly deals with marking side effects on both local axes.
  Vec3 GetWorldAxisHelper(const Physics::AxisAtom& axisAtom) const;
  void SetWorldAxisHelper(Physics::AxisAtom& axisAtom, Vec3Param worldAxis);

  //------------------------------------------------------------------- AngleAtom helpers
  /// Helpers to get/set local space reference frames for an angle atom.
  Quat GetLocalAngleHelper(const Physics::AngleAtom& angleAtom, uint index) const;
  void SetLocalAngleHelper(Physics::AngleAtom& angleAtom, uint index, QuatParam localReferenceFrame);

  /// Should baumgarte correction be used for the given joint type.
  bool GetShouldBaumgarteBeUsed(uint type) const;
  /// Get the linear baumgarte term for a joint type.
  real GetLinearBaumgarte(uint type) const;
  /// Get the angular baumgarte term for a joint type.
  real GetAngularBaumgarte(uint type) const;
  /// Get the linear error correction term (post stabilization) for a joint type.
  real GetLinearErrorCorrection(uint type) const;
  /// This function is so that the base Joint can have this
  /// called on it without knowing what type the joint is.
  real GetLinearErrorCorrection() const;
  /// Get the angular error correction term (post stabilization) for a joint type.
  real GetAngularErrorCorrection(uint type) const;
  real GetAngularErrorCorrection() const;
  real GetSlop() const;

  /// Helper function that forces the collider's cached
  /// body-to-world transforms to be up-to-date.
  void UpdateColliderCachedTransforms();

  /// Helper to grab the current anchors from the object link.
  void ComputeCurrentAnchors(Physics::AnchorAtom& anchors);
  /// Helper to compute a reference angle from the current object's rotations.
  void ComputeCurrentReferenceAngle(Physics::AngleAtom& referenceAngle);

  // Member accessors
  /// Is this joint currently on an island? Primarily used during island building.
  bool GetOnIsland() const;
  void SetOnIsland(bool onIsland);
  /// @JoshD: Unused?
  bool GetGhost() const;
  void SetGhost(bool ghost);
  // A joint is valid if it has all of the data needed to solve (typically if it's connected to two colliders)
  bool GetValid() const;
  void SetValid(bool valid);

  /// Determines if this joint is currently active.
  /// Used for runtime enabling/disabling of joints.
  bool GetActive() const;
  void SetActive(bool active);

  /// Determines if this joint will send any events.
  /// Used for a small efficiency boost and for reducing the number of events.
  bool GetSendsEvents() const;
  void SetSendsEvents(bool sendsEvents);

  /// Determines if this joint will automatically delete itself if any of its constraints
  /// reach the max impulse value. This will still send an event if it snaps.
  bool GetAutoSnaps() const;
  void SetAutoSnaps(bool autoSnaps);

  /// Determines if the two objects connected by this joint can collide.
  /// If any joint between this pair does not collide, then the
  /// pair does not collide. All joints have to be set to true in
  /// order to have the objects collide.
  bool GetCollideConnected() const;
  void SetCollideConnected(bool collideConnected);

  /// The maximum impulse (instantaneous force) that this joint can apply to correct itself.
  real GetMaxImpulse() const;
  void SetMaxImpulse(real maxImpulse);

  enum Filter
  {
    FilterFlag = 0xFF,
    ActiveOffset = 0,
    DefaultOffset = 8
  };

  enum AtomFilter
  {
    LinearAxis = 1 << 0,
    AngularAxis = 1 << 1
  };

  enum FragmentCountFilter
  {
    OneFragment = 0x1,
    TwoFragments = 0x3,
    ThreeFragments = 0x7,
    FourFragments = 0xF,
    FiveFragments = 0x1F,
    SixFragments = 0x3F
  };

  enum FragmentFilter
  {
    SingleFragment = 0x1,
    FreeLinearFragment = 0x3B,
    FreeAngularFragment = 0x1F,
    NoFreeAxesFragment = 0x3F
  };

  enum DefaultAddOnFilters
  {
    SingleAxis = 0x1,
    SingleLinearAxis = 0x4,
    SingleAngularAxis = 0x20,
    AllLinearAxes = 0x7,
    AllAngularAxes = 0x38,
    AllAxes = 0x3f
  };

  JointNode* mNode;
  uint mConstraintFilter;
  real mMaxImpulse;
  BitField<JointFlags::Enum> mFlags;

  EdgeType mEdges[2];
  Link<Joint> SolverLink;
  Link<Joint> SpaceLink;
  PhysicsSpace* mSpace;
  Physics::IConstraintSolver* mSolver;
};

// Helper macros to declare template code. This code is here so that the virtual
// interface can still exist, but there is a non-virtual interface that is expected.
// This interface is called by certain solvers to avoid as much virtual function
// overhead as possible.
#define DeclareJointType(jointType)                                                            \
  ZilchDeclareType(TypeCopyMode::ReferenceType);                                               \
  static cstr JointName;                                                                       \
  static const JointEnums::JointTypes mJointType;                                              \
  static cstr StaticGetJointName();                                                            \
  cstr GetJointName() const override;                                                          \
  static JointEnums::JointTypes StaticGetJointType();                                          \
  JointEnums::JointTypes GetJointType() const override;                                        \
  void UpdateAtomsVirtual() override;                                                          \
  uint MoleculeCountVirtual() const override;                                                  \
  void ComputeMoleculesVirtual(Physics::MoleculeWalker& molecules) override;                   \
  void WarmStartVirtual(Physics::MoleculeWalker& molecules) override;                          \
  void SolveVirtual(Physics::MoleculeWalker& molecules) override;                              \
  void CommitVirtual(Physics::MoleculeWalker& molecules) override;                             \
  uint PositionMoleculeCountVirtual() const override;                                          \
  void ComputePositionMoleculesVirtual(Physics::MoleculeWalker& molecules) override;           \
  void DebugDrawVirtual() override;                                                            \
  uint GetAtomIndexFilterVirtual(uint atomIndex, real& desiredConstraintValue) const override; \
  void BatchEventsVirtual() override;                                                          \
  real GetLinearBaumgarte() const;                                                             \
  real GetAngularBaumgarte() const;                                                            \
  real GetLinearErrorCorrection() const;                                                       \
  real GetAngularErrorCorrection() const;

#define ImplementJointType(jointType)                                                                                          \
  cstr jointType::JointName = #jointType;                                                                                      \
  const JointEnums::JointTypes jointType::mJointType = JointEnums::jointType##Type;                                            \
  cstr jointType::StaticGetJointName() { return JointName; }                                                                   \
  cstr jointType::GetJointName() const { return JointName; }                                                                   \
  JointEnums::JointTypes jointType::StaticGetJointType() { return mJointType; };                                               \
  JointEnums::JointTypes jointType::GetJointType() const { return mJointType; };                                               \
  void jointType::UpdateAtomsVirtual() { UpdateAtoms(); }                                                                      \
  uint jointType::MoleculeCountVirtual() const { return MoleculeCount(); }                                                     \
  void jointType::ComputeMoleculesVirtual(Physics::MoleculeWalker& molecules) { ComputeMolecules(molecules); }                 \
  void jointType::WarmStartVirtual(Physics::MoleculeWalker& molecules) { WarmStart(molecules); }                               \
  void jointType::SolveVirtual(Physics::MoleculeWalker& molecules) { Solve(molecules); }                                       \
  void jointType::CommitVirtual(Physics::MoleculeWalker& molecules) { Commit(molecules); }                                     \
  uint jointType::PositionMoleculeCountVirtual() const { return PositionMoleculeCount(); };                                    \
  void jointType::ComputePositionMoleculesVirtual(Physics::MoleculeWalker& molecules) { ComputePositionMolecules(molecules); } \
  void jointType::DebugDrawVirtual() { DebugDraw(); }                                                                          \
  uint jointType::GetAtomIndexFilterVirtual(uint atomIndex, real& desiredConstraintValue) const                                \
  {                                                                                                                            \
    return GetAtomIndexFilter(atomIndex, desiredConstraintValue);                                                              \
  }                                                                                                                            \
  void jointType::BatchEventsVirtual() { BatchEvents(); }                                                                      \
  real jointType::GetLinearBaumgarte() const                                                                                   \
  {                                                                                                                            \
    return Joint::GetLinearBaumgarte(StaticGetJointType());                                                                    \
  }                                                                                                                            \
  real jointType::GetAngularBaumgarte() const                                                                                  \
  {                                                                                                                            \
    return Joint::GetAngularBaumgarte(StaticGetJointType());                                                                   \
  };                                                                                                                           \
  real jointType::GetLinearErrorCorrection() const                                                                             \
  {                                                                                                                            \
    return Joint::GetLinearErrorCorrection(StaticGetJointType());                                                              \
  }                                                                                                                            \
  real jointType::GetAngularErrorCorrection() const                                                                            \
  {                                                                                                                            \
    return Joint::GetAngularErrorCorrection(StaticGetJointType());                                                             \
  }

}//namespace Zero
