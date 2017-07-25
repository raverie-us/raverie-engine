///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

extern const real cAngularSleepEpsilon;
extern const real cLinearSleepEpsilon;
extern const real cTimeToSleep;

class PhysicsSpace;
class PhysicsNode;
class IgnoreSpaceEffects;

// Internal states of a rigid body.
DeclareBitField8(RigidBodyStates, Static, 
                                  Asleep, 
                                  Kinematic, 
                                  RotationLocked, 
                                  Mode2D, 
                                  AllowSleep,
                                  Inherit2DMode,
                                  SleepAccumulated);

/// What kind of dynamics this body should have. Determines if forces are
/// integrated and if collisions are resolved.
/// <param name="Dynamic">A regular body that can be affected by forces.</param>
/// <param name="Static">An infinite mass body. Will not move via dynamics.</param>
/// <param name="Kinematic">An infinite mass body that approximates velocity when moved.
/// Used for moving platforms and other movable infinite mass objects.</param>
DeclareEnum3(RigidBodyDynamicState, Dynamic, Static, Kinematic);

/// Represents what dimensions a RigidBody should operate in (how many degrees of freedom it has by default).
/// <param name="Mode2D">Restricts the object's movement through physics to 2D. This lowers
/// the object's degrees of freedom down to 3 (x-linear, y-linear, z-angular).</param>
/// <param name="InheritFromSpace">Use the Mode2D state on the PhysicsSpace.
/// This allows easy changing of the entire space between 2D/3D.</param>
/// <param name="Mode3D">A regular 3D object (6 degrees of freedom).</param>
DeclareEnum3(Mode2DStates, Mode2D,
                           InheritFromSpace,
                           Mode3D);

/// A Base for the RigidBody so that it can contain an in-list of itself.
class BaseRigidBody : public Component
{
public:
  Link<BaseRigidBody> RigidBodyHierarchyLink;
};

/// RigidBody defines the inertia (mass, velocity, etc...) of a rigid object. Any PhysicsEffects
/// attached to a RigidBody without a region will be applied to the center of mass of this body.
class RigidBody : public BaseRigidBody
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef InList<Collider, &Collider::mBodyLink> CompositeColliderList;
  typedef CompositeColliderList::range CompositeColliderRange;
  typedef BaseInList<BaseRigidBody, RigidBody, &BaseRigidBody::RigidBodyHierarchyLink> ChildBodyList;
  typedef ChildBodyList::range BodyRange;

  RigidBody();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void AttachTo(AttachmentInfo& info) override;
  void Detached(AttachmentInfo& info) override;
  void TransformUpdate(TransformUpdateInfo& info) override;
  void OnDestroy(uint flags) override;

  // Properties

  /// The velocity (units per second) of this body in world space. Max bounds are around 1e+10,
  /// after this the velocity will be capped.
  Vec3 GetVelocity();
  void SetVelocity(Vec3Param velocity);
  /// The angular velocity (radians per second) of this body in world space. Objects will rotate about
  /// this axis using the right-hand rule. Max bounds are around 1e+10, after this
  /// the angular velocity will be capped.
  Vec3 GetAngularVelocity();
  void SetAngularVelocity(Vec3Param angularVelocity);
  /// The total accumulated force (in world space) that is being applied
  /// to the center of mass of this rigid body this frame.
  Vec3 GetForce();
  void SetForce(Vec3Param force);
  /// The total accumulated torque (in world space) that is being applied
  /// to the center of mass of this rigid body this frame.
  Vec3 GetTorque();
  void SetTorque(Vec3Param torque);

  /// Adds to the current linear velocity. (world space)
  void ApplyLinearVelocity(Vec3Param linear);
  /// Adds to the current angular velocity. (world space)
  void ApplyAngularVelocity(Vec3Param angular);
  /// Applies a velocity at a world space point on the object.
  /// Results in a change to linear and angular velocity.
  void ApplyVelocityAtPoint(Vec3Param velocity, Vec3Param worldPoint);
  
  /// Computes the linear point velocity of world-space point.
  Vec3 GetPointVelocity(Vec3Param worldPoint);

  /// Applies a force through the center of mass of the body. (world space)
  void ApplyForce(Vec3Param force);
  /// Applies a torque through the center of mass of the body. (world space)
  void ApplyTorque(Vec3Param torque);
  /// Applies a force at an offset from the center of mass (world space). Results in a
  /// force and torque to the center of mass.
  void ApplyForceAtOffsetVector(Vec3Param force, Vec3Param worldOffset);
  /// Applies a force at a world point (world space). Results in a
  /// force and torque to the center of mass.
  void ApplyForceAtPoint(Vec3Param force, Vec3Param worldPoint);

  /// Applies a linear impulse through the center of mass (world space).
  /// Only results in a change of linear velocity.
  void ApplyLinearImpulse(Vec3Param linear);
  /// Applies an angular impulse through the center of mass (world space).
  /// Only results in a change of angular velocity.
  void ApplyAngularImpulse(Vec3Param angular);
  /// Applies a linear and angular impulse (world space). Used in solving constraints.
  void ApplyConstraintImpulse(Vec3Param linear, Vec3Param angular);
  /// Applies an impulse at on offset from the center of mass (world space).
  /// Results in a change in linear and angular velocity.
  void ApplyImpulseAtOffsetVector(Vec3Param impulse, Vec3Param worldOffset);
  /// Applies an impulse at a world point (world space).
  /// Results in a change to linear and angular velocity.
  void ApplyImpulseAtPoint(Vec3Param impulse, Vec3Param worldPoint);
  
  /// How this rigid body handles dynamics. Is it a regular rigid body (dynamic)?
  /// Does it not move (static)? Does the user want to manually move it and have
  /// objects properly collide with it (kinematic)? Note: Static vs. static does
  /// not check for collision.
  RigidBodyDynamicState::Enum GetDynamicState();
  void SetDynamicState(RigidBodyDynamicState::Enum state);
  /// Static objects cannot be moved by collisions, forces or velocity changes.
  /// This is a more efficient way of changing an object between static and
  /// dynamic than removing the RigidBody. Static objects should not be moved
  /// during runtime as they will not correctly account for their position change
  /// when colliding with objects.
  bool GetStatic();
  /// Kinematic objects are static objects that can be moved during runtime.
  /// Kinematics will correctly deal with position change during collisions
  /// by approximating the linear and angular velocity from the transform changes.
  bool GetKinematic() const;
  /// Returns if the object is dynamic (not static or kinematic)
  bool IsDynamic();

  /// Sleeping happens when an object has not "moved" (small enough linear + angular velocity)
  /// for long enough. If this happens then the physics engine stops updating this object
  /// (integration, collision detection, etc...) until an awake object hits it.
  /// Sleeping is purely an optimization. Sometimes it is not desirable for a body to ever
  /// fall asleep (such as a player).
  bool GetAllowSleep() const;
  void SetAllowSleep(bool state);
  /// Whether or not this object is currently asleep. Setting this to true will force the
  /// object asleep even if this causes invalid behavior (objects floating).
  bool GetAsleep();
  void SetAsleep(bool asleep);
  /// Checks if the object is currently asleep.
  bool IsAsleep();
  /// Forces the object awake. Will reset the sleep timer.
  void ForceAwake();
  /// Forces the object asleep. Warning: calling this function could
  /// create gameplay flaws if used incorrectly, use at your own risk (and sparingly).
  void ForceAsleep();

  /// Makes physics unable to rotate this object. Manual rotations can still be applied.
  bool GetRotationLocked() const;
  void SetRotationLocked(bool state);
  
  /// Used to make an object act as if it were 2D. This is done by locking
  /// it to the current z-plane and only allowing rotation about the
  /// world's z-axis. Objects can be set to always be 2D or 3D, or this can
  /// be deferred to the PhysicsSpace's Mode2D.
  Mode2DStates::Enum GetMode2D() const;
  void SetMode2D(Mode2DStates::Enum state);
  
  /// The current mass of the rigid body. This includes all child colliders belonging to this body.
  real GetMass();
  void SetMass(real mass);
  void SetInverseMass(real invMass);

  /// The inverse inertia tensor in local space of this body. The local space inertia
  /// tensor doesn't change when the object rotates but is not typically useful for any
  /// calculations other than computing the world-space inverse inertia tensor.
  Mat3 GetLocalInverseInertiaTensor();
  /// The inverse inertia tensor in world space of this body. Describes how hard it is to rotate an
  /// object about the world-space axes. Useful to convert any torque into an angular velocity.
  Mat3 GetWorldInverseInertiaTensor();
  
  /// The position of the center of mass in world space.
  Vec3 GetWorldCenterOfMass();
  /// The cached world-space translation of this rigid body
  Vec3 GetWorldTranslation() const;
  /// The cached local-to-world rotation matrix of this rigid body (even if static/kinematic).
  Mat3 GetWorldRotationMat3() const;
  /// The cached local-to-world rotation quaternion of this rigid body
  /// (even if static/kinematic). Mostly used for integration.
  Quat GetWorldRotationQuat() const;
  /// The full local-to-world transform of this rigid body.
  Mat4 GetWorldMatrix() const;

  /// If this is a static body then the active body (the one force/velocity should be
  /// applied to or calculated from) is the nearest parent body up the hierarchy that
  /// is not static. If one isn't found then this returns the current rigid body (this).
  RigidBody* GetActiveBody();
  /// Returns all colliders that are owned by this rigid body. This is all colliders at or below
  /// the rigid body's level in the hierarchy, including all colliders under static/kinematic bodies.
  /// This is basically the full list of colliders that affect this body's velocity/mass/etc... 
  CompositeColliderList::range GetColliders();


  //-------------------------------------------------------------------Internal
  /// Has this body already started initialization (aka, we have a physics space).
  /// Used to deal with setters during serialization that require updating the physics space.
  bool IsInitialized();
  Vec3 GetPointVelocityInternal(Vec3Param worldPoint);
  /// Wakes the object up.  Does not reset the sleep timer, 
  /// meaning it will go to sleep instantly if it is not moving.
  void WakeUp();
  /// Helper function to clamp linear/angular velocity
  Vec3 ClampVelocityValue(Vec3Param value, StringParam varName);
  /// Force this body asleep.
  void PutToSleep();
  /// The shared functionality between ForceAwake and WakeUp.
  void InternalWakeUp();
  /// Updates the object's sleep state.
  bool UpdateSleepTimer(real dt);
  void SetRotationLockedInternal(bool state);
  /// Internal helper that actually sets state associated with 2D mode and
  /// doesn't worry about the inheritance of the space's 2D mode.
  void Set2DInternal(bool state);
  void UpdateMode2D();

  /// Clear all mass and velocity terms to 0. Used when switching to static/kinematic states.
  void ClearMassAndVelocities();
  /// Recompute kinematic velocities from any translations that have happened.
  void UpdateKinematicVelocities();
  /// Compute the delta velocities to get from the passed translation and rotation to the current ones.
  void ComputeVelocities(Vec3Param oldTranslation, Mat3Param oldRotation, real dt);
  
  /// Recompute the mass, inertia, and world inertia of this body.
  /// Internal function that doesn't check the modified state (otherwise infinite loops can happen).
  Vec3 InternalGetWorldCenterOfMass() const;
  void RecomputeAllMassTerms();
  void RecomputeCenterMass();
  void RecomputeInertiaTensor();
  /// Use the stored offset from the center of mass to the translation of the cog to
  /// recompute where the center of mass is relative to the translation. Normally the
  /// center of mass updates the translation but if a user translates this body we need
  /// to shift the center of mass based upon the translation performed.
  void UpdateCenterMassFromWorldPosition();
  /// Recomputes the world-space inertia tensor for this body.
  void UpdateWorldInertiaTensor();
  /// Sets the allowed local-space axes that an object can rotate about.
  void SetAxisLock(bool xAxis, bool yAxis, bool zAxis);
  /// Queue up this body (and all necessary parents) for a mass update.
  void QueueMassUpdate();
  /// Force update any modified resources and update the physics queue state
  void UpdateResourcesAndQueue();


  // Integration/Position solving related (position/rotation updating)

  /// Updates the Rigid Body's center of mass by an offset vector.
  /// Also updates the cached world transform data.
  void UpdateCenterMass(Vec3Param offset);
  /// Updates the Rigid Body's orientation by an offset vector (for integration, this
  /// does a small angle approximation). This updates the body's cached world transform
  /// data as we not only rotate but the position might be rotating about the center of mass.
  void UpdateOrientation(QuatParam offset);
  /// Applies a rotation to the rigid body.
  void Rotate(QuatParam rotation);
  /// Shared logic when updating orientation (assumes mRotationQuat was already set and normalized).
  /// This will update the cached world transform matrix and re-compute the world translation.
  void InternalRecomputeOrientation();
  /// Generates an integration update for all of the colliders of this body.
  void GenerateIntegrationUpdate();
  /// Set the transform values from the current cached body-to-world values
  void PublishTransform();
  
  /// Adds an effect to be applied to this body.
  void AddBodyEffect(PhysicsEffect* effect);
  /// Apply all of the effects on this body to the passed in body. If the passed in
  /// body is null this will apply to the current body. An extra body must be passed
  /// in to properly deal with child static/kinematic bodies.
  void UpdateBodyEffects(real dt, RigidBody* rootBody = nullptr);
  /// Removes an effect from being applied to this body
  void RemoveBodyEffect(PhysicsEffect* effect);
  
  /// The max velocity to clamp setters to if there's no physics space available (serialization)
  const static real mMaxVelocity;

  /// Velocity is how many units per second this object will move.
  Vec3 mVelocity;
  Vec3 mVelocityOld;
  /// Angular velocity is how many radians per second this object will rotate.
  Vec3 mAngularVelocity;
  Vec3 mAngularVelocityOld;
  // Linear/Angular force
  Vec3 mForceAccumulator;
  Vec3 mTorqueAccumulator;
  
  /// The center of mass in world space of this body and its colliders.
  Vec3 mCenterOfMass;
  /// The offset from the center of mass to the Transform's translation in local space.
  /// This isn't in a true local space as scale is not included.
  /// Needed to update translation correctly as objects rotate about the center of mass not translation.
  Vec3 mPositionOffset;
  /// The rotation of the body. A quaternion is needed for integration.
  Quat mRotationQuat;
  /// Mass and inertia are stored as inverses for efficiency.
  Physics::Mass mInvMass;
  Physics::Inertia mInvInertia;

  /// How long this rigid body has been below the required linear and angular velocity
  /// thresholds for sleeping. A rigid body will only fall asleep when it hasn't "moved" for long enough.
  real mSleepTimer;

  BitField<RigidBodyStates::Enum> mState;

  PhysicsEffectList mEffects;
  IgnoreSpaceEffects* mSpaceEffectsToIgnore;

  // Hierarchy information
  PhysicsNode* mPhysicsNode;
  RigidBody* mParentBody;
  CompositeColliderList mColliders;
  ChildBodyList mChildBodies;
  
  // Space information
  PhysicsSpace* mSpace;
  Link<RigidBody> mSpaceLink;
};

typedef InList<RigidBody, &RigidBody::mSpaceLink> RigidBodyList;

}//namespace Zero
