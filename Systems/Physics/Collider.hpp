///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareBitField8(ColliderFlags, Ghost,
                                SendsEvents,
                                OnIsland,
                                HasPairFilter,
                                Uninitialized,
                                Seamless,
                                MasslessBody,
                                MasslessCollider);

/// A collider controls how collision detection is performed for an object.
/// A collider also gives mass properties to a RigidBody (via the material and volume).
/// If there is no RigidBody associated with this collider then it is considered static.
/// Note: colliders without RigidBodies should not be moved at run-time!
class Collider : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  /// All possible collider types. Needed mostly for collision detection.
  /// The order is somewhat important as collision detection currently assumes
  /// that all simple shapes (primitive types) are before the complex shapes.
  enum ColliderType{ cSphere = 0, cBox, cCylinder, cEllipsoid, cCapsule,
                     cConvexMesh, cMultiConvexMesh, cMesh, cHeightMap,
                     cSize, cInvalid };
  /// Internal enum used for determining what kind of transform update is happening.
  enum eUpdateTransformState { cInitialize, cUpdate };

  typedef InList<Physics::ContactEdge, &Physics::ContactEdge::ColliderLink> ContactEdgeList;
  typedef InList<JointEdge, &JointEdge::ColliderLink> JointEdgeList;

  Collider();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void TransformUpdate(TransformUpdateInfo& info) override;
  void AttachTo(AttachmentInfo& info) override;
  void Detached(AttachmentInfo& info) override;
  void DebugDraw() override;
  void OnDestroy(uint flags) override;
  
  //-------------------------------------------------------------------Collider Interface
  /// Gives any derived collider a chance to cache world-space values (e.g. SphereCollider.WorldRadius)
  virtual void CacheWorldValues();
  /// Fix the world-space bounding volumes from the current transform and per collider property
  /// data (Sphere: mRadius, Box: mSize, etc...). If you need a bounding volume to be up-to-date
  /// this should be called, not either of the Compute functions for a bounding volume.
  virtual void ComputeWorldBoundingVolumes();
  /// Computes the world-space aabb. This is always called after CacheWorldValues has been called.
  /// This must be implemented on each collider type. The helper function 'SetWorldAabbFromHalfExtents'
  /// exists to compute the aabb from the current rotation and translation with the given world scale half-extents of an aabb.
  virtual void ComputeWorldAabbInternal() = 0;
  /// Compute the world-space bounding sphere. See ComputeWorldAabb for a description of when and why
  /// this is called (but for a sphere). The default behavior is to take the sphere centered at the object's
  /// translation with a radius to the furthest away point.
  virtual void ComputeWorldBoundingSphereInternal();
  /// Compute the volume of this collider in world space. This must be calculated in world space
  /// as there's no generic way to scale volume after-the-fact. All colliders that contain a volume
  /// should implement this. Some colliders (e.g. HeightMap or Mesh) don't as they aren't meant to
  /// work with rigid bodies. This is an internal function, meaning that it assumes the
  /// transform data is already up-to-date (should rarely be called by the outside).
  virtual real ComputeWorldVolumeInternal();
  /// Computes the local space (as in not rotated, the scale is still necessary but the translation
  /// is assumed to be zero) inertia tensor from the passed in mass.
  virtual void ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia);
  /// Returns the location of the local space center of mass for this collider. For all symmetric
  /// colliders this is the origin, but for non-symmetric colliders this can change. 
  /// This is needed for computing a rigid body's mass and inertia properties.
  virtual Vec3 GetColliderLocalCenterOfMass() const;
  /// Check all resources used by this collider to see if they need to be updated.
  /// Called by several "Get" functions to force up-to-date information.
  virtual void RebuildModifiedResources();
  //-------------------------------------------------------------------Support Shape Interface
  /// Support function for GJK/MPR or any other Minkowski difference algorithm.
  /// This sets the given support vector to the point (in world space) furthest in the given direction.
  /// Almost all colliders should implement this! The main exceptions are colliders that contain
  /// multiple primitives (such as MultiConvexMeshCollider) in which case each primitive type should have this function.
  virtual void Support(Vec3Param direction, Vec3Ptr support) const;
  /// Return a point deep inside the shape. Needed to initialize MPR and make some initial good guesses.
  /// Not commonly overridden in a derived collider type.
  virtual void GetCenter(Vec3Ref center) const;
  /// Creates a support shape wrapper class for this collider. This helps provide a generic interface to collision
  /// detection that works with colliders or other primitive shapes. The support delta is needed for Swept (@JoshD: clean up later).
  /// Should almost never be overridden in a derived collider type.
  virtual Intersection::SupportShape GetSupportShape(bool supportDelta = false);


  /// The material used to determine the density, restitution, and friction of this collider.
  PhysicsMaterial* GetMaterial();
  void SetMaterial(PhysicsMaterial* physicsMaterial);
  /// The collision group is a tag used to alter collision behavior
  /// based upon the space's CollisionTable.
  CollisionGroup* GetCollisionGroup();
  void SetCollisionGroup(CollisionGroup* collisionGroup);
  /// Ghosted colliders do not resolve collision. They do still detect collisions and send events.
  /// Ghosted colliders are typically used for trigger regions.
  bool GetGhost() const;
  void SetGhost(bool ghost);
  /// Determines if this object will send collision events. Used mainly for
  /// increasing performance by not sending unnecessary collision events.
  bool GetSendsEvents() const;
  void SetSendsEvents(bool sendsMessages);
  /// Moves the physics defined center of the object away from the transform's
  /// translation. Used to move physics to match a model.
  Vec3 GetOffset() const;
  void SetOffset(Vec3Param localOffset);

  /// Compute the world-space volume of this collider.
  real ComputeVolume();
  /// Determines if this collider should perform collision detection against the passed in collider.
  /// This checks everything from asleep to collision groups.
  bool ShouldCollide(Collider* otherCollider);

  /// The cached world-space scale of this collider.
  Vec3 GetWorldScale() const;
  /// The cached world-space rotation of this collider.
  Mat3 GetWorldRotation() const;
  /// The cached world-space translation of this collider.
  Vec3 GetWorldTranslation() const;
  /// The cached local to world transform data for this collider.
  WorldTransformation* GetWorldTransform() const;

  /// The rigid body that "owns" this collider. This is the body that forces/impulses/etc... should be applied to.
  RigidBody* GetActiveBody() const;
  /// Returns the rigid body's center of mass in world space. If there is no rigid body then
  /// the collider's world translation is returned instead.
  Vec3 GetRigidBodyCenterOfMass() const;
  /// Returns the point velocity of a world-space point with respect to the current rigid body's
  /// linear and angular velocity. If there is no rigid body this returns zero.
  Vec3 GetPointVelocity(Vec3Param worldPoint);
  /// Returns the world-space axis aligned bounding box (Aabb) of this collider.
  Aabb GetWorldAabb();
  /// Returns the world-space bounding sphere of this collider.
  Sphere GetWorldBoundingSphere();
  
  // Helpers that forward rigid body states
  bool IsDynamic() const;
  bool IsStatic() const;
  bool IsKinematic() const;
  bool Is2D() const;
  bool IsAsleep() const;
  void ForceAwake();
  
  /// The current number of contacts/collisions with this collider.
  uint GetContactCount();
  /// A range of all contacts for this collider.
  ContactRange GetContacts();
  /// The number of joints attached to this collider. 
  uint GetJointCount();
  /// A range of all joints attached to this collider.
  JointRange GetJoints();

  //-------------------------------------------------------------------Internal
  /// Which kind of collider is this? Mostly used to determine which collision function to call.
  ColliderType GetColliderType() const;
  /// Updates the unique collider Id.
  u32 GetNextColliderId();

  /// Disconnect all events from the old material and connect to the new material.
  void UpdatePhysicsMaterialConnections(PhysicsMaterial* oldMaterial, PhysicsMaterial* newMaterial);
  /// The physics material we are using has been modified. Update mass from density. The event isn't used.
  void OnPhysicsMaterialModified(Event* e);

  /// Sets the world-space aabb of this collider given a world-space half-extents of the aabb.
  /// Commonly used by box or box-like things to approximate the world-space aabb as the
  /// rotation of the (scaled) local-space aabb.
  void SetWorldAabbFromHalfExtents(Vec3Param worldHalfExtents);
  Vec3 TransformSupportDirectionToLocal(Vec3Param worldSupportDirection) const;
  Vec3 TransformSupportPointToWorld(Vec3Param localSupportPoint) const;
  /// Computes the mass of this collider from its volume and density.
  real ComputeMass();
  
  /// Computes the collider's center of mass in world space. This should only be
  /// called when computing a rigid body's world center of mass. 
  Vec3 GetColliderWorldCenterOfMass() const;
  /// Computes a point velocity of a world-space point from this collider's rigid body.  Used in
  /// collision resolution and in determining the separating velocity of a point.
  Vec3 ComputePointVelocityInternal(Vec3Param worldPoint);

  /// This encompasses whether or not a collider is allowed to collide with anything. Currently
  /// ghost colliders and ones that have no mass in the entire rigid body are unable to collide with anything.
  bool NotCollideable() const;
  /// Pair filters are used to define per-object pairings of collisions to skip. This
  /// function allows skipping a hashmap lookup if this collider has no pair filters.
  bool GetHasPairFilter() const;
  void SetHasPairFilter(bool hasPairFilter);
  /// Helper function that not only determines if two colliders should collide but also fills out a string
  /// with the reason for why they are not (for PhysicsSpace.WhyAreTheyNotColliding).
  bool ShouldCollide(Collider* otherCollider, String& reasonForNotColliding);
  /// Internal function that will return why they aren't colliding if the template
  /// argument is true (for efficiency to not build the string during normal collision detection)
  template <bool BuildReason>
  bool ShouldCollideInternal(Collider* otherCollider, String* reasonForNotColliding);

  void UpdateMasslessColliderFlag();
  /// Does this collider belong in dynamic or static broadphase?
  bool InDynamicBroadPhase() const;
  /// Helper function for all derived collider types to call whenever their base size properties change.
  /// If one of these changes then several things, ranging from half-extents to broadphase, have to be updated.
  void InternalSizeChanged();
  /// Takes care of all logic when a collider has its transform updated out from under it.
  /// This is broken up into two categories: initialization and update. In update a little more work has to be done.
  void InternalTransformUpdate(eUpdateTransformState updateState);
  /// Queues an update in the physics queues that says this collider has been integrated.
  /// Being integrated implies a very specific set of things that have to be updated.
  void GenerateIntegrationUpdate();
  /// Forces an update (if it exists) to the collider's mass and inertia properties.
  /// This occasionally is needed when some place needs up-to-date information and the
  /// cached body-to-world transforms could be out of date (in the middle of a user call).
  /// Joints are the main caller of this right now.
  void UpdateQueue();

  /// When a collider is dynamically added no joints are connected to it. To fix each joint we walk
  /// through all object links we're connected to and forcefully unlink and link them which will fix all joints.
  void ForceRelinkJoints();
  /// Unlink all joints connected to this collider. This is likely called because we are being destroyed
  void UnlinkAllJoints();
  /// Destroy all contacts connected to this collider. Unlike joints, contacts need to be deleted when either
  /// collider is disconnected from the contact. An additional bool is specified to determine whether or
  /// not to send contact ended events immediately (if we wait then the object will already
  /// be deleted and the .Cog on the event will be null)
  void DestroyAllContacts(bool sendImmediately = false);


  /// Various state flags, some of which are exposed to the user.
  BitField<ColliderFlags::Enum> mState;
  /// The world space aabb.
  Aabb mAabb;
  /// The world-space bounding sphere.
  Sphere mBoundingSphere;

private:
  /// The physics material needs to be private so that no one ever tries to swap the material without
  /// going through the setter. This is required as we need to listen to events on the material.
  HandleOf<PhysicsMaterial> mMaterial;

public:
  /// The regular collision group needs to be serialized, but there needs to be per-space data stored (the instance).
  HandleOf<CollisionGroup> mSerializedGroup;
  /// The run-time information needed for collision groups for this space's collision table.
  /// This Contains cached information for efficient run-time queries.
  CollisionGroupInstance* mCollisionGroupInstance;
  /// A local space offset for the collider's position. Used to shift a collider's position to match
  /// up with a model or something without having to create a child object.
  Vec3 mTranslationOffset;
  
  /// The node is shared data needed between colliders and rigid bodies.
  /// This pointer is also used as an "is initialized" flag.
  PhysicsNode* mPhysicsNode;
  /// The rigid body that owns this collider. That is, this is
  /// the body where forces, constraints, impulses, etc... should be applied.
  /// May not be the body that our mass is actually account for in though.
  RigidBody* mActiveRigidBody;
  /// The body that our mass is actually accounted for in. May be the same
  /// as the active body.
  RigidBody* mDirectRigidBody;

  /// All of the physics effects to be applied to the active RigidBody of this collider
  PhysicsEffectList mEffects;
  
  /// How many contacts this collider has. This is cached instead of computed
  /// each frame as this can be a fairly common operation.
  uint mContactCount;
  ContactEdgeList mContactEdges;
  JointEdgeList mJointEdges;

  /// What kind of collider this is. Used mostly for collision detection.
  ColliderType mType;
  /// This id is incremented for each collider. This is used to sort colliders 
  /// for consistent ordering and forming pair ids.
  u32 mId;
  Link<Collider> mIslandLink;
  /// Link for composite bodies
  Link<Collider> mBodyLink;
  // Space Information
  PhysicsSpace* mSpace;
  Link<Collider> mSpaceLink;
  

  // This is a hack variable, should not be used unless
  // I (Josh Davis) am doing something with it. Remove eventually!!!
  Vec3 mCollisionOffset;
};

typedef InList<Collider, &Collider::mSpaceLink> ColliderList;

}//namespace Zero
