///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
DeclareEvent(ComputeCustomJointInfo);
}//namespace Events

namespace Physics
{

struct CustomJoint;

//-------------------------------------------------------------------CustomJointEvent
/// Sent by CustomJoint before solving constraints. Used to configure
/// constraints before the physics system begins solving.
class CustomJointEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// The joint that sent this event.
  CustomJoint* mOwner;
  /// The current frame's delta time. Use to setup the constraint if necessary.
  real mDt;
};

//-------------------------------------------------------------------CustomConstraintInfo
/// Information to represent a constraint to be solved. The main information that needs to
/// be set here is the Jacobian and error. A constraint will enforce that the relative
/// velocities along the Jacobian are equal to zero (ignoring error correction or motors).
struct CustomConstraintInfo : public ReferenceCountedEventObject
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CustomConstraintInfo();

  /// Set the Jacobian of this constraint (and the effective mass).
  void SetJacobian(Vec3Param linear0, Vec3Param angular0, Vec3Param linear1, Vec3Param angular1);
  /// Set the position error of the constraint. This also sets the bias of the constraint
  /// (used to actually correct the error). If you want to set this constraint as a motor
  /// you should not call this function (or call it first). If you want to set this as a
  /// spring then make sure you call this first.
  void SetErrorAndBias(real error);
  /// Given the currently set mass and error, fix the constraint to be soft (i.e.
  /// solve the constraint like a spring). The spring fixes the
  /// constraint using the given frequency (oscillations per second)
  /// and damping ratio (0 is no damping, 1 is critical damping).
  void ComputeSpring(float frequencyHz, float dampRatio);
  /// Sets this constraint as a motor (i.e. a constraint that drives movement
  /// along the Jacobian direction at a certain speed). The motor has a min and max impulse
  /// value that can be solved (typically -value, +value). If you set this as a motor, you should do so last.
  /// Motors should typically be their own constraint unlike springs.
  void ComputeMotor(float targetSpeed, float minImpulse, float maxImpulse);

  /// Reset secondary terms that should be computed each frame.
  void Reset();
  /// Remove this constraint from whatever joint owns it.
  /// This is equivalent to "this.Owner.RemoveConstraint(this)".
  void DetachFromOwner();
  /// Is this constraint currently owned by a joint? (Equivalent to "this.Owner != null").
  bool IsOwned();
  /// What joint currently owns this constraint.
  CustomJoint* GetOwner();

  /// Linear portion of objectA's Jacobian.
  Vec3 mLinear0;
  /// Angular portion of objectA's Jacobian.
  Vec3 mAngular0;
  /// Linear portion of objectB's Jacobian.
  Vec3 mLinear1;
  /// Angular portion of objectB's Jacobian.
  Vec3 mAngular1;

  /// The effective mass of the constraint. This is typically set by calling SetJacobian.
  real mEffectiveMass;
  /// The error of the constraint. This should typically be set via the SetError function.
  real mError;
  /// The baumgarte term used to correct error. This should typically be set in initialization
  /// (per constraint) and then left alone. Default value is 5.
  real mBaumgarte;

  /// The bias is used to apply energy into the system. Typically, bias is combined with Error
  /// and Baumgarte to fix error. Bias is also used for motors and springs to drive the constraint.
  real mBias;
  /// The min impulse magnitude allowed for the constraint.
  real mMinImpulse;
  /// The max impulse magnitude allowed for the constraint.
  real mMaxImpulse;
  /// The total accumulated impulse of this constraint. If you want to not use warm-starting then clear this value every frame.
  real mImpulse;
  /// Gamma is used to soften constraints. This should typically never be manually set.
  /// Instead, it is set when configuring the constraint to act like a spring.
  real mGamma;
  
  /// Is this constraint currently active?
  bool mActive;
  /// Should this constraint solve position directly or use baumgarte correction?
  /// Toggling SolvePosition should be done before setting any other values (ideally in initialization).
  /// Setting a constraint to be a motor or a spring will turn off position correction
  /// as an error bias must be used to solve those scenarios.
  bool mSolvePosition;

  HandleOf<CustomJoint> mOwner;
};

//-------------------------------------------------------------------CustomJoint
/// A customizable joint that can be configured in script. The user can create constraints belonging
/// to this joint and set the required values to solve them. Some basic constraint understanding is required.
/// To compute constraints you should listen to Events.ComputeCustomJointInfo.
struct CustomJoint : public Joint
{
  DeclareJointType(CustomJoint);

  CustomJoint();

  // Joint Interface Implementation
  void UpdateAtoms();
  uint MoleculeCount() const;
  void ComputeMolecules(MoleculeWalker& molecules);
  void WarmStart(MoleculeWalker& molecules);
  void Solve(MoleculeWalker& molecules);
  void Commit(MoleculeWalker& molecules);
  uint PositionMoleculeCount() const;
  void ComputePositionMolecules(MoleculeWalker& molecules);
  void DebugDraw() {  };
  uint GetAtomIndexFilter(uint atomIndex, real& desiredConstraintValue) const;
  void BatchEvents();


  /// Create a constraint that is attached to this joint.
  CustomConstraintInfo* CreateConstraint();
  /// Add a constraint to this joint. This will assert if a joint already owns this constraint.
  void AddConstraint(CustomConstraintInfo* constraint);
  /// If the given constraint belongs to this joint then remove it from the constraints to solve.
  void RemoveConstraint(CustomConstraintInfo* constraint);
  /// Clear all constraints from this joint (so none will solve).
  void ClearConstraints();

  /// Returns how many constraints this joint owns.
  size_t GetConstraintCount();
  /// Returns the constraint at the given index. Will assert if the index is outside the constraint count range.
  CustomConstraintInfo* GetConstraint(size_t index);

  void ConstraintInfoToMolecule(CustomConstraintInfo* constraint, ConstraintMolecule& molecule);
  void UpdateTransform(int colliderIndex);

  typedef HandleOf<CustomConstraintInfo> ConstraintInfoReference;
  Array<ConstraintInfoReference> mConstraints;
};

}//namespace Physics

typedef Physics::CustomJoint CustomJoint;
typedef Physics::CustomJointEvent CustomJointEvent;
typedef Physics::CustomConstraintInfo CustomConstraintInfo;

}//namespace Zero
