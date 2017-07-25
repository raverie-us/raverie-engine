///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class BroadPhasePackage;
typedef Array<Collider*> ColliderArray;

DeclareBitField3(PhysicsSpaceFlags, AllowSleep, Mode2D, Deterministic);

namespace Tags
{
DeclareTag(Physics);
}

//-------------------------------------------------------------------SweepResult
/// Cast result from performing a sweep test.
struct SweepResult
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  friend bool operator<(const SweepResult& lhs, const SweepResult& rhs)
  {
    return lhs.mTime < rhs.mTime;
  }

  /// The time of impact that this collision first happens.
  real GetTime();
  /// The other collider being hit.
  Collider* GetOtherCollider();
  /// The other cog being hit.
  Cog* GetOtherObject();
  /// The amount of overlap with this object. Will typically be zero unless the objects start in contact.
  real GetPenetration();
  /// The point of intersection in world-space.
  Vec3 GetWorldPoint();
  /// The contact normal pointing from the other object towards the sweeping object.
  Vec3 GetWorldNormalTowardsSelf();
  /// The contact normal pointing from the sweeping object towards the other object.
  Vec3 GetWorldNormalTowardsOther();

  real mTime;
  Collider* mOtherCollider;
  real mPenetration;
  Vec3 mWorldPoint;
  Vec3 mWorldNormalTowardsSelf;
};

typedef Array<SweepResult> SweepResultArray;

//-------------------------------------------------------------------SweepResultRange
struct SweepResultRange
{
  typedef SweepResult value_type;
  typedef SweepResult& return_type;

  typedef SweepResult& FrontResult;

  SweepResultRange(){}
  SweepResultRange(const SweepResultArray& array);
  SweepResultRange(const SweepResultRange& other);
  ~SweepResultRange();

  bool Empty();
  FrontResult Front();
  void PopFront();
  uint Size();

  SweepResultArray mArray;
  SweepResultArray::range mRange;
};

//-------------------------------------------------------------------PhysicsSpace

/// The PhysicsSpace is an "instance" of a world. This world
/// manages and stores all of the other physical components of this world.
/// PhysicSpaces act independently of each other.
class PhysicsSpace : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  friend class RigidBody;

  PhysicsSpace();
  virtual ~PhysicsSpace();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// How this space should build islands. For internal testing.
  PhysicsIslandType::Enum GetIslandingType() const;
  void SetIslandingType(PhysicsIslandType::Enum islandingType);
  /// What kinds of pre-processing should be performed on islands. Used to test performance.
  PhysicsIslandPreProcessingMode::Enum GetIslandPreProcessType() const;
  void SetIslandPreProcessType(PhysicsIslandPreProcessingMode::Enum preProcessType);
  /// Post-processing islands will merge islands that are too small.
  bool GetPostProcessIslands() const;
  void SetPostProcessIslands(bool postProcess);
  /// How many islands currently exist. For debugging.
  uint GetIslandCount() const;
  /// (Internal) Configures if one Solver is used across all islands. For performance testing.
  bool GetIsSolverShared() const;
  void SetIsSolverShared(bool shared);
  /// Filters two cogs to not resolve collisions with each other.
  /// This is a runtime only feature and will not be saved.
  void AddPairFilter(Cog* cog1, Cog* cog2);
  /// Adds a filter to ignore collision between both hierarchies
  /// passed in. See AddPairFilter for more info.
  void AddHierarchyPairFilter(Cog* cog1, Cog* cog2);
  void AddHierarchyPairFilterInternal(Cog* hierarchyCog, Cog* normalCog);
  void AddPairFilterInternal(Collider* collider1, Collider* collider2);
  /// Removes the filter between two cogs allowing collisions to be computed as normal.
  void RemovePairFilter(Cog* cog1, Cog* cog2);
  /// Removes the filters between both hierarchies. See RemovePairFilter for more info.
  void RemoveHierarchyPairFilter(Cog* cog1, Cog* cog2);
  void RemoveHierarchyPairFilterInternal(Cog* hierarchyCog, Cog* normalCog);
  void RemovePairFilterInternal(Collider* collider1, Collider* collider2);
  /// Creates a joint by name (e.g. StickJoint) between two cogs.
  /// The world points of the joint are both set to worldPoint.
  Cog* CreateJoint(Cog* cog0, Cog* cog1, StringParam jointName, Vec3Param worldPoint);

  /// The default 2D mode for this space. If a RigidBody is set to InheritFromSpace then it will use this value.
  bool GetMode2D() const;
  void SetMode2D(bool state);
  /// Performs extra work to help enforce determinism in the simulation.
  bool GetDeterministic() const;
  void SetDeterministic(bool state);

  /// Helper for a collider. Returns this space's instance for a CollisionGroup.
  CollisionGroupInstance* GetCollisionGroupInstance(ResourceId groupId) const;
  void FixColliderCollisionGroups(ColliderList& colliders);
  /// The collision table resource used to filter collisions in this space.
  CollisionTable* GetCollisionTable();
  void SetCollisionTable(CollisionTable* collisionTable);
  void FixCollisionTable(CollisionTable* table);
  /// The resource that controls how physics solves things. Mostly related to how collision is resolved.
  PhysicsSolverConfig* GetPhysicsSolverConfig();
  void SetPhysicsSolverConfig(PhysicsSolverConfig* config);

  /// Updates any modified resource (typically from script) that physics cares about.
  void UpdateModifiedResources();

  /// Debug draws and makes sure broadphase is up-to-date.
  void FrameUpdate();
  /// Updates every object in the space (integration, collision detection / resolution, etc...).
  void SystemLogicUpdate(UpdateEvent* updateEvent);

  /// Forces all queued computations in physics to be updated now. Should only be used for debugging.
  void FlushPhysicsQueue();
  /// Updates all queues for pending physics calculation. Beforehand, it also recomputes
  /// the world matrix values so that everything is in the right spot.
  void PushBroadPhaseQueue();
  /// Same as PushBroadPhaseQueue but also profiles the sub-steps. This should be
  /// unified later when a better profiling system is implemented.
  void PushBroadPhaseQueueProfiled();
  void UpdateTransformAndMassOfTree(PhysicsNode* node);
  /// Iterates one timestep of physics with the given dt. Does not take care of
  /// batch insertion/removal in broadphases or debug drawing.
  void IterateTimestep(real dt);

  /// Adds global effect to all bodies then integrates force to velocity.
  void IntegrateBodiesVelocity(real dt);
  /// Integrates velocity to position of all of bodies.
  void IntegrateBodiesPosition(real dt);
  /// Updates all BroadPhases and then finds all possible collision pairs.
  void BroadPhase();
  /// Takes the possible collisions from the BroadPhase step and checks if they
  /// actually collide. If they do collide then they are added to the IslandManager.
  void NarrowPhase();
  /// Sends out any pre-solve events so users can modify state before resolution.
  void PreSolve(real dt);
  /// Solves the constraints of all islands.
  void ResolutionPhase(real dt);
  /// If any constraint has position correction, then this solves the position constraints directly.
  void SolvePositions(real dt);
  /// Solve any spring systems
  void SolveSprings(real dt);

  //---------------------------------------------------------------- Ray Casting
  /// Returns the results of a Ray Cast. The results of the ray cast are
  /// stored in the passed in vector sorted by time of collision.  The number
  /// of results given is based on the size of the CastResults passed in.
  void CastRay(const Ray& worldRay, CastResults& results);
  
  /// Finds the first collider that a ray hits. A default CastFilter will be used.
  CastResult CastRayFirst(const Ray& worldRay);
  /// Finds the first collider that a ray hits using the given filter.
  CastResult CastRayFirst(const Ray& worldRay, CastFilter& filter);

  /// Finds all colliders in the space that a ray hits. This returns up 
  /// to maxCount number of objects. A default CastFilter will be used. 
  CastResultsRange CastRay(const Ray& worldRay, uint maxCount);
  /// Finds all colliders in the space that a ray hits using the
  /// given filter. This returns up to maxCount number of objects.
  CastResultsRange CastRay(const Ray& worldRay, uint maxCount, CastFilter& filter);

  //------------------------------------------------------------ Segment Casting
  /// Returns the results of a Segment Cast.  The results of the segment cast
  /// are stored in the passed in vector sorted by time of collision. The
  /// number of results given is based on the size of the CastResults passed in.
  void CastSegment(const Segment& segment, CastResults& results);
  /// Finds all colliders in the space that a line segment hits. This returns up 
  /// to maxCount number of objects. A default CastFilter will be used. 
  CastResultsRange CastSegment(const Segment& segment, uint maxCount);
  /// Finds all colliders in the space that a line segment hits using the
  /// given filter. This returns up to maxCount number of objects.
  CastResultsRange CastSegment(const Segment& segment, uint maxCount, CastFilter& filter);

  //------------------------------------------------------------- Aabb Casting
  void CastAabb(const Aabb& aabb, CastResults& results);
  /// Finds all colliders in the space that an Aabb hits using the
  /// given filter. This returns up to maxCount number of objects.
  CastResultsRange CastAabb(const Aabb& aabb, uint maxCount, CastFilter& filter);

  //------------------------------------------------------------- Sphere Casting
  void CastSphere(const Sphere& sphere, CastResults& results);
  /// Finds all colliders in the space that a Sphere hits using the
  /// given filter. This returns up to maxCount number of objects.
  CastResultsRange CastSphere(const Sphere& sphere, uint maxCount, CastFilter& filter);

  //------------------------------------------------------------- Frustum Casting
  void CastFrustum(const Frustum& frustum, CastResults& results);
  /// Finds all colliders in the space that a Frustum hits using the
  /// given filter. This returns up to maxCount number of objects.
  CastResultsRange CastFrustum(const Frustum& frustum, uint maxCount, CastFilter& filter);

  //------------------------------------------------------------- Collider Casting
  /// Currently a hack function for player controller sweeping
  void CastCollider(Vec3Param offset, Collider* testCollider, Physics::ManifoldArray& results, CastFilter& filter);
  /// Finds all colliders in the space that another collider hits using the
  /// given filter. The test collider's position can be offset to test at a different location.
  /// This returns up to maxCount number of objects.
  CastResultsRange CastCollider(Vec3Param offset, Collider* testCollider, CastFilter& filter);

  //------------------------------------------------------------- Collider Sweeping
  /// Performs a swept cast with a collider's shape and a given velocity.
  /// Returns a range of all objects the collider could've hit within 'dt' time.
  SweepResultRange SweepCollider(Collider* collider, Vec3Param velocity, real dt, CastFilter& filter);

  //------------------------------------------------------------- Collision Shape Events
  /// Dispatches an event to all objects within the given sphere.
  void DispatchWithinSphere(const Sphere& sphere, StringParam eventName, Event* toSend);
  /// Dispatches an event to all objects within the given aabb.
  void DispatchWithinAabb(const Aabb& aabb, StringParam eventName, Event* toSend);

  /// The number of iterations the physics space will take every frame.
  /// Used to achieve higher accuracy and increase visual results.
  uint GetSubStepCount() const;
  void SetSubStepCount(uint substeps);

  /// Determines if anything in the space is allowed to fall sleep.
  bool GetAllowSleep() const;
  void SetAllowSleep(bool allowSleep);

  /// Wakes up all asleep bodies.
  void ForceAwakeRigidBodies();

  /// Returns a debug string stating why physics does or doesn't think these two objects should be colliding.
  String WhyAreTheyNotColliding(Cog* cog1, Cog* cog2);

  //----------------------------------------------- Dynamic Component Add/Remove
  void AddComponent(RigidBody* body);
  void RemoveComponent(RigidBody* body);
  /// The given body has changed between Dynamic/Static/Kinematic.
  void ComponentStateChange(RigidBody* body);

  void AddComponent(Joint* joint);
  void RemoveComponent(Joint* joint);

  void AddComponent(Region* region);
  void RemoveComponent(Region* region);

  void AddComponent(Collider* collider);
  void RemoveComponent(Collider* collider);
  /// This collider's body has changed dynamic state.
  void ComponentStateChange(Collider* collider);

  void AddComponent(PhysicsCar* car);
  void RemoveComponent(PhysicsCar* car);

  void AddComponent(SpringSystem* system);
  void RemoveComponent(SpringSystem* system);

  /// Returns all global effects (on PhysicsSpace or LevelSettings)
  PhysicsEffectList::range GetGlobalEffects();
  void AddGlobalEffect(PhysicsEffect* effect);
  void RemoveGlobalEffect(PhysicsEffect* effect);

  /// Draw level for broad phase.
  void IncrementDrawLevel() { ++mDrawLevel; }
  void DecrementDrawLevel() { --mDrawLevel; }

  /// Queues the given physics node as having modifications
  void QueuePhysicsNode(PhysicsNode* node);

  /// Find the island that a collider is in (for debug purposes).
  typedef InList<Collider, &Collider::mIslandLink> IslandColliderList;
  IslandColliderList::range GetAllInIsland(Collider* collider);

  BroadPhasePackage* ReplaceBroadPhase(BroadPhasePackage* newBroadPhase);
  Physics::CollisionManager* GetCollisionManager();

  Link<PhysicsSpace> EngineLink;

private:
  friend class PhysicsEngine;

  /// Serializes the broad phase information.
  void SerializeBroadPhases(Serializer& stream);

  /// Tell the rest of the engine what objects have been updated (integration).
  void Publish();
  /// Send out any queued events (Contacts, Joints, etc...)
  void PublishEvents();

  /// Tell all PhysicsEffects to pre-calculate any shared information for this frame.
  void PreCalculateEffects(real dt);
  /// Apply all region effects.
  void UpdateRegions(real dt);
  /// Apply misc. effects sitting in the middle of a hierarchy
  void ApplyHierarchyEffects(real dt);
  /// Apply global effects (PhysicsSpace/LevelSettings) to the given body
  void ApplyGlobalEffects(RigidBody* body, real dt);

  /// Checks all inactive objects to see if they should be woken up.
  void WakeInactiveMovingBodies();

  /// Takes a kinematic object and moves it to the moving kinematic list.
  void ActivateKinematic(RigidBody* body);
  /// Computes a kinematic body's velocities.
  void UpdateKinematicVelocities();
  /// Updates kinematic objects between the three internal states of moving, stopping, and inactive.
  void UpdateKinematicState();
  /// Updates all the physics cars. (needs to happen before resolution)
  void UpdatePhysicsCars(real dt);
  /// Updates the positions of the wheels for the cars.
  /// (Needs to happen after position integration)
  void UpdatePhysicsCarsTransforms(real dt);

  void DebugDraw();
  void DrawColliders();
  void DrawRigidBodies();
  void DrawInactiveObjects();

  /// Helper to get broadphase data for a collider
  void ColliderToBroadPhaseData(Collider* collider, BroadPhaseData& data);

  int mDrawLevel;
  BitField<PhysicsSpaceFlags::Enum> mStateFlags;

  /// Debug drawing flags
  BitField<PhysicsSpaceDebugDrawFlags::Enum> mDebugDrawFlags;

  // Used for narrow phase.
  Physics::CollisionManager* mCollisionManager;
  Physics::ContactManager* mContactManager;
  Physics::IslandManager* mIslandManager;
  // Stores the objects returned from the broad phase for that frame.  It is
  // not created on the stack each frame to avoid allocations.
  ClientPairArray mPossiblePairs;

  // Stores all broad phase information.
  BroadPhasePackage* mBroadPhase;

  // Components
  RigidBodyList  mRigidBodies;
  /// Asleep bodies.
  RigidBodyList  mInactiveRigidBodies;
  /// Kinematic bodies that have had a transform update called in the last frame.
  RigidBodyList  mMovingKinematicBodies;
  /// Kinematic bodies that had a transform update called two frames ago.
  /// This is used as a temporary holding place for a bodies so that
  /// their velocities can be cleared at an appropriate
  /// time before they are marked as inactive.
  RigidBodyList  mStoppedKinematicBodies;
  /// Kinematic bodies that have not had a transform update recently.
  /// They do not need any iteration whatsoever.
  RigidBodyList  mInactiveKinematicBodies;
  // Separate dynamic and static components to reduce queries.
  ColliderList   mDynamicColliders;
  ColliderList   mStaticColliders;
  RegionList     mRegions;

  typedef InList<PhysicsCar, &PhysicsCar::SpaceLink> CarList;
  CarList mCars;

  typedef InList<SpringSystem, &SpringSystem::SpaceLink> SpringSystems;
  SpringSystems mSprings;

  typedef InList<Joint, &Joint::SpaceLink> JointList;
  JointList mJoints;

  TimeSpace* mTimeSpace;
  PhysicsEngine* mPhysicsEngine;

  typedef Array<CollisionEvent*> CollisionEvents;
  CollisionEvents mCollisionEvents;
  typedef InList<JointEvent> JointEventList;
  JointEventList mJointEvents;
  typedef Array<CollisionGroupEvent*> CollisionGroupEvents;
  CollisionGroupEvents mCollisionGroupEvents;
  Physics::PhysicsNodeManager* mNodeManager;
  HandleOf<PhysicsSolverConfig> mPhysicsSolverConfig;

public:
  // Physics effects that sit randomly in a hierarchy and need to apply to their nearest rigid-body parent.
  PhysicsEffectList mHierarchyEffects;
  // Global effects (regions which do not have Collider's). The effects in these
  // Regions will be applied to all objects in the space.
  PhysicsEffectList mGlobalEffects;
  // All effects owned by this space (needed for the pre-calculate phase)
  SpaceEffectList mEffects;

  Physics::PhysicsEventManager* mEventManager;
  /// The table that defines the relationship between collision groups.
  /// Used to filter collisions and send special pair events.
  HandleOf<CollisionTable> mCollisionTable;

  uint mSubStepCount;
  HashSet<u64> mFilteredPairs;

  // Dt of the current iteration. Stored for when I don't want to pass down dt
  // 20 layers to use in one place. The object can grab this from it's space
  // if it is operating during IterateTimestep.
  real mIterationDt;

  // These variables control the max velocity that a rigid body can be set to.
  // The bool is used to only display an error message the first time this happens.
  bool mInvalidVelocityOccurred;
  real mMaxVelocity;

  /// What kind of broadphase is used for dynamic objects (those with RigidBodies).
  String mDynamicBroadphaseType;
  /// What kind of broadphase is used for static objects (those without RigidBodies).
  String mStaticBroadphaseType;

  Memory::Heap* mHeap;
  /// Dummy collider used when things attach to the world. Makes life
  /// easier by not special casing world connections.
  Collider* mWorldCollider;
};

}//namespace Zero
