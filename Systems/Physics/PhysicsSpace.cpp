///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Tags
{
  DefineTag(Physics);
}

//-------------------------------------------------------------------SweepResult
ZilchDefineType(SweepResult, builder, type)
{
  ZilchBindDefaultCopyDestructor();
  ZeroBindDocumented();

  ZilchBindGetterProperty(Time);
  ZilchBindGetterProperty(OtherCollider);
  ZilchBindGetterProperty(OtherObject);
  ZilchBindGetterProperty(Penetration);
  ZilchBindGetterProperty(WorldPoint);
  ZilchBindGetterProperty(WorldNormalTowardsSelf);
  ZilchBindGetterProperty(WorldNormalTowardsOther);
}

real SweepResult::GetTime()
{
  return mTime;
}

Collider* SweepResult::GetOtherCollider()
{
  return mOtherCollider;
}

Cog* SweepResult::GetOtherObject()
{
  return mOtherCollider->mOwner;
}

real SweepResult::GetPenetration()
{
  return mPenetration;
}

Vec3 SweepResult::GetWorldPoint()
{
  return mWorldPoint;
}

Vec3 SweepResult::GetWorldNormalTowardsSelf()
{
  return mWorldNormalTowardsSelf;
}

Vec3 SweepResult::GetWorldNormalTowardsOther()
{
  return -mWorldNormalTowardsSelf;
}

//-------------------------------------------------------------------SweepResultRange
SweepResultRange::SweepResultRange(const SweepResultArray& array)
  : mArray(array)
{
  mRange = mArray.All();
}

SweepResultRange::SweepResultRange(const SweepResultRange& other)
  : mArray(other.mArray)
{
  mRange = mArray.All();
}

SweepResultRange:: ~SweepResultRange()
{

}

bool SweepResultRange::Empty()
{
  return mRange.Empty();
}

SweepResult& SweepResultRange::Front()
{
  return mRange.Front();
}

void SweepResultRange::PopFront()
{
  mRange.PopFront();
}

uint SweepResultRange::Size()
{
  return mRange.Size();
}

//-------------------------------------------------------------------ClientPairSorter
bool ClientPairSorter(ClientPair& a, ClientPair& b)
{
  // Get the client data
  Collider* collider0 = static_cast<Collider*>(a.mClientData[0]);
  Collider* collider1 = static_cast<Collider*>(a.mClientData[1]);
  Collider* collider2 = static_cast<Collider*>(b.mClientData[0]);
  Collider* collider3 = static_cast<Collider*>(b.mClientData[1]);

  // Generate two collider pairs so that we can use its operator >
  ColliderPair pairA(collider0, collider1);
  ColliderPair pairB(collider2, collider3);

  return pairA > pairB;
}

//-------------------------------------------------------------------PhysicsSpace
ZilchDefineType(PhysicsSpace, builder, type)
{
  ZeroBindSetup(SetupMode::DefaultSerialization);
  type->AddAttribute(ObjectAttributes::cCore);

  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindTag(Tags::Physics);
  ZeroBindDependency(Space);

  ZeroBindEvent(Events::GroupCollisionStarted, CollisionGroupEvent);
  ZeroBindEvent(Events::GroupCollisionEnded, CollisionGroupEvent);
  ZeroBindEvent(Events::PhysicsUpdateFinished, ObjectEvent);

  ZilchBindGetterSetterProperty(SubStepCount);
  ZilchBindGetterSetterProperty(AllowSleep);
  ZilchBindGetterSetterProperty(Mode2D);
  ZilchBindGetterSetterProperty(Deterministic);
  ZilchBindGetterSetterProperty(CollisionTable);
  ZilchBindGetterSetterProperty(PhysicsSolverConfig);

  // Broad-phase types
  ZilchBindFieldProperty(mDynamicBroadphaseType)->Add(new DynamicBroadphasePropertyExtension());
  ZilchBindFieldProperty(mStaticBroadphaseType)->Add(new StaticBroadphasePropertyExtension());
  
  ZilchBindMethod(AddPairFilter);
  ZilchBindMethod(AddHierarchyPairFilter);
  ZilchBindMethod(RemovePairFilter);
  ZilchBindMethod(RemoveHierarchyPairFilter);

  ZilchBindMethod(FlushPhysicsQueue);

  // Ray Cast
  ZilchBindOverloadedMethod(CastRayFirst, ZilchInstanceOverload(CastResult, const Ray&));
  ZilchBindOverloadedMethod(CastRayFirst, ZilchInstanceOverload(CastResult, const Ray&, CastFilter&));
  ZilchBindOverloadedMethod(CastRay, ZilchInstanceOverload(CastResultsRange, const Ray&, uint));
  ZilchBindOverloadedMethod(CastRay, ZilchInstanceOverload(CastResultsRange, const Ray&, uint, CastFilter&));
  // Segment Cast
  ZilchBindOverloadedMethod(CastSegment, ZilchInstanceOverload(CastResultsRange, const Segment&, uint));
  ZilchBindOverloadedMethod(CastSegment, ZilchInstanceOverload(CastResultsRange, const Segment&, uint, CastFilter&));
  // Volume Cast
  ZilchBindOverloadedMethod(CastAabb, ZilchInstanceOverload(CastResultsRange, const Aabb&, uint, CastFilter&));
  ZilchBindOverloadedMethod(CastSphere, ZilchInstanceOverload(CastResultsRange, const Sphere&, uint, CastFilter&));
  ZilchBindOverloadedMethod(CastFrustum, ZilchInstanceOverload(CastResultsRange, const Frustum&, uint, CastFilter&));
  ZilchBindOverloadedMethod(CastCollider, ZilchInstanceOverload(CastResultsRange, Vec3Param, Collider*, CastFilter&));
  // Event Dispatching in Region
  ZilchBindMethod(DispatchWithinSphere);
  ZilchBindMethod(DispatchWithinAabb);

  // Extra collider detection methods
  ZilchBindMethod(SweepCollider);

  // Helpers
  ZilchBindMethod(CreateJoint);
  ZilchBindMethod(WhyAreTheyNotColliding);

  // Debugging features for JoshD. Disabled for all users for now
  //bool inDevConfig = Z::gEngine->GetConfigCog()->has(Zero::DeveloperConfig) != nullptr;
  //if(inDevConfig)
  //{
  //  ZilchBindGetterSetterProperty(IslandingType);
  //  ZilchBindGetterSetterProperty(IslandPreProcessType);
  //  ZilchBindGetterSetterProperty(PostProcessIslands);
  //  ZilchBindGetterSetterProperty(IsSolverShared);
  //  ZilchBindGetter(IslandCount);
  //}
  //else
  //{
  //  ZilchBindGetterSetter(IslandingType);
  //  ZilchBindGetterSetter(IslandPreProcessType);
  //  ZilchBindGetterSetter(PostProcessIslands);
  //  ZilchBindGetterSetter(IsSolverShared);
  //  ZilchBindGetter(IslandCount);
  //}
}

PhysicsSpace::PhysicsSpace()
{
  mHeap = nullptr;
  mDrawLevel = 0;
  mDebugDrawFlags.Clear();

  mBroadPhase = nullptr;
  mContactManager = nullptr;
  mIslandManager = nullptr;
  mWorldCollider = nullptr;
  mNodeManager = nullptr;

  mDebugDrawFlags.SetFlag(PhysicsSpaceDebugDrawFlags::DrawDebug);
  mIterationDt = real(.016);

  mInvalidVelocityOccurred = false;
  mMaxVelocity = real(1e+10);
}

PhysicsSpace::~PhysicsSpace()
{
  PhysicsEffectList::range range = mGlobalEffects.All();
  while(!range.Empty())
  {
    PhysicsEffect& effect = range.Front();
    range.PopFront();
    effect.PhysicsEffect::ComponentRemoved(ZilchTypeId(PhysicsSpace), this);
  }

  mNodeManager->CommitChanges(mBroadPhase);

  Memory::HeapDeallocate(mHeap, mEventManager);
  Memory::HeapDeallocate(mHeap, mNodeManager);
  Memory::HeapDeallocate(mHeap, mIslandManager);
  Memory::HeapDeallocate(mHeap, mContactManager);

  SafeDelete(mBroadPhase);

  mPhysicsEngine->RemoveSpace(this);
}

void PhysicsSpace::Serialize(Serializer& stream)
{
  // Serialize AllowSleep and Deterministic.
  uint defaultFlags = PhysicsSpaceFlags::AllowSleep | PhysicsSpaceFlags::Deterministic;
  SerializeBits(stream, mStateFlags, PhysicsSpaceFlags::Names, 0, defaultFlags);
  SerializeNameDefault(mSubStepCount, 1u);
  SerializeResourceName(mCollisionTable, CollisionTableManager);
  SerializeResourceName(mPhysicsSolverConfig, PhysicsSolverConfigManager);

  // For now just save what broadphase to use, not any broadphase properties
  SerializeNameDefault(mDynamicBroadphaseType, ZilchTypeId(DynamicAabbTreeBroadPhase)->Name);
  SerializeNameDefault(mStaticBroadphaseType, ZilchTypeId(StaticAabbTreeBroadPhase)->Name);
}

void PhysicsSpace::Initialize(CogInitializer& initializer)
{
  mPhysicsEngine = Z::gEngine->has(PhysicsEngine);
  mHeap = mPhysicsEngine->mHeap;

  mContactManager = Memory::HeapAllocate<Physics::ContactManager>(mHeap);
  mContactManager->mSpace = this;

  mIslandManager = Memory::HeapAllocate<Physics::IslandManager>(mHeap, mPhysicsSolverConfig);
  mIslandManager->SetSpace(this);
  mNodeManager = Memory::HeapAllocate<Physics::PhysicsNodeManager>(mHeap);
  mEventManager = Memory::HeapAllocate<Physics::PhysicsEventManager>(mHeap);
  mEventManager->SetAllocator(mHeap);
  mCollisionManager = mPhysicsEngine->mCollisionManager;

  // Create the broadphases
  mBroadPhase = new BroadPhasePackage();
  // Switch the static broadphase to the DynamicAabbTree if in
  // editor mode (so moving static objects isn't slow)
  String staticBroadPhaseType = mStaticBroadphaseType;
  if(GetOwner()->GetSpace()->IsEditorMode())
    staticBroadPhaseType = ZilchTypeId(DynamicAabbTreeBroadPhase)->Name;

  BroadPhaseLibrary* library = Z::gBroadPhaseLibrary;
  mBroadPhase->AddBroadPhase(BroadPhase::Static, library->CreateBroadPhase(staticBroadPhaseType));
  mBroadPhase->AddBroadPhase(BroadPhase::Dynamic, library->CreateBroadPhase(mDynamicBroadphaseType));
  mBroadPhase->Initialize();

  mPhysicsEngine->AddSpace(this);

  mCollisionEvents.SetAllocator(HeapAllocator(mHeap));

  mTimeSpace = GetOwner()->has(TimeSpace);

  // Create a world object
  Space* space = (Space*)GetOwner();
  Cog* world = space->CreateNamed(CoreArchetypes::WorldAnchor);
  world->SetEditorOnly();
  world->mFlags.SetFlag(CogFlags::EditorViewportHidden | CogFlags::ObjectViewHidden);
  world->SetName(SpecialCogNames::WorldAnchor);
  mWorldCollider = world->has(Collider);
  // Hack to remove the world object.
  PushBroadPhaseQueue();
  // Remove the object as a component so that it is not in broadphase
  RemovalAction action(mWorldCollider);
  // We want this change right away.
  PushBroadPhaseQueue();

  ConnectThisTo(GetOwner(), Events::SystemLogicUpdate, SystemLogicUpdate);
}

PhysicsIslandType::Enum PhysicsSpace::GetIslandingType() const
{
  return mIslandManager->mIslandingType;
}

void PhysicsSpace::SetIslandingType(PhysicsIslandType::Enum islandingType)
{
  mIslandManager->mIslandingType = islandingType;
}

PhysicsIslandPreProcessingMode::Enum PhysicsSpace::GetIslandPreProcessType() const
{
  return mIslandManager->mPreProcessingType;
}

void PhysicsSpace::SetIslandPreProcessType(PhysicsIslandPreProcessingMode::Enum preProcessType)
{
  mIslandManager->mPreProcessingType = preProcessType;
}

bool PhysicsSpace::GetPostProcessIslands() const
{
  return mIslandManager->mPostProcess;
}

void PhysicsSpace::SetPostProcessIslands(bool postProcess)
{
  mIslandManager->mPostProcess = postProcess;
}

uint PhysicsSpace::GetIslandCount() const
{
  return mIslandManager->mIslandCount;
}

bool PhysicsSpace::GetIsSolverShared() const
{
  return mIslandManager->mShareSolver;
}

void PhysicsSpace::SetIsSolverShared(bool shared)
{
  mIslandManager->mShareSolver = shared;
}

void PhysicsSpace::AddPairFilter(Cog* cog1, Cog* cog2)
{
  if(cog1 == nullptr || cog2 == nullptr)
  {
    DoNotifyWarning("Invalid parameters", "Invalid cogs passed in. One of them is null");
    return;
  }

  Collider* collider0 = cog1->has(Collider);
  Collider* collider1 = cog2->has(Collider);
  if(collider0 == nullptr || collider1 == nullptr)
    return;

  AddPairFilterInternal(collider0, collider1);
}

void PhysicsSpace::AddHierarchyPairFilter(Cog* cog1, Cog* cog2)
{
  if(cog1 == nullptr || cog2 == nullptr)
  {
    DoNotifyWarning("Invalid parameters", "Invalid cogs passed in. One of them is null");
    return;
  }

  AddHierarchyPairFilterInternal(cog1, cog2);
  HierarchyRange range = HierarchyRange::SubTree(cog2);
  for(; !range.Empty(); range.PopFront())
  {
    AddHierarchyPairFilterInternal(cog1, &range.Front());
  }
}

void PhysicsSpace::AddHierarchyPairFilterInternal(Cog* hierarchyCog, Cog* normalCog)
{
  Collider* normalCollider = normalCog->has(Collider);
  if(normalCollider == nullptr)
    return;

  // HierarchyRange::SubTree doesn't include the cog itself,
  // so add a pair filter for the root manually
  Collider* hierarchyCollider = hierarchyCog->has(Collider);
  if(hierarchyCollider != nullptr)
    AddPairFilterInternal(hierarchyCollider, normalCollider);

  HierarchyRange range = HierarchyRange::SubTree(hierarchyCog);
  for(; !range.Empty(); range.PopFront())
  {
    Collider* collider1 = range.Front().has(Collider);
    if(collider1 != nullptr)
      AddPairFilterInternal(collider1, normalCollider);
  }
}

void PhysicsSpace::AddPairFilterInternal(Collider* collider1, Collider* collider2)
{
  collider1->SetHasPairFilter(true);
  collider2->SetHasPairFilter(true);
  CogId cogId1 = CogId(collider1->GetOwner());
  CogId cogId2 = CogId(collider2->GetOwner());

  u64 packedId = GetLexicographicId(cogId1.GetId(), cogId2.GetId());
  mFilteredPairs.Insert(packedId);
}

void PhysicsSpace::RemovePairFilter(Cog* cog1, Cog* cog2)
{
  if(cog1 == nullptr || cog2 == nullptr)
  {
    DoNotifyWarning("Invalid parameters", "Invalid cogs passed in. One of them are null");
    return;
  }

  Collider* collider1 = cog1->has(Collider);
  Collider* collider2 = cog2->has(Collider);
  if(collider1 == nullptr || collider2 == nullptr)
    return;

  RemovePairFilterInternal(collider1, collider2);
}

void PhysicsSpace::RemoveHierarchyPairFilter(Cog* cog1, Cog* cog2)
{
  if(cog1 == nullptr || cog2 == nullptr)
  {
    DoNotifyWarning("Invalid parameters", "Invalid cogs passed in. One of them are null");
    return;
  }

  RemoveHierarchyPairFilterInternal(cog1, cog2);
  HierarchyRange range = HierarchyRange::SubTree(cog2);
  for(; !range.Empty(); range.PopFront())
  {
    RemoveHierarchyPairFilterInternal(cog1, &range.Front());
  }
}

void PhysicsSpace::RemoveHierarchyPairFilterInternal(Cog* hierarchyCog, Cog* normalCog)
{
  Collider* normalCollider = normalCog->has(Collider);
  if(normalCollider == nullptr)
    return;

  // HierarchyRange::SubTree doesn't include the cog itself,
  // so remove a pair filter for the root manually
  Collider* hierarchyCollider = hierarchyCog->has(Collider);
  if(hierarchyCollider != nullptr)
    RemovePairFilterInternal(hierarchyCollider, normalCollider);

  HierarchyRange range = HierarchyRange::SubTree(hierarchyCog);
  for(; !range.Empty(); range.PopFront())
  {
    Collider* collider1 = range.Front().has(Collider);
    if(collider1 != nullptr)
      RemovePairFilterInternal(collider1, normalCollider);
  }
}

void PhysicsSpace::RemovePairFilterInternal(Collider* collider1, Collider* collider2)
{
  if(!collider1->GetHasPairFilter() || !collider2->GetHasPairFilter())
    return;

  u32 id1 = CogId(collider1->GetOwner()).GetId();
  u32 id2 = CogId(collider2->GetOwner()).GetId();
  u64 packedId = GetLexicographicId(id1, id2);

  mFilteredPairs.Erase(packedId);
}

Cog* PhysicsSpace::CreateJoint(Cog* cog0, Cog* cog1, StringParam jointName,
                               Vec3Param worldPoint)
{
  if(cog0 == nullptr || cog1 == nullptr)
  {
    String msg = String::Format("Either Cog0 (%p) or Cog1 (%p) is null.", cog0, cog1);
    DoNotifyException("Invalid object(s) in joint connection", msg);
    return nullptr;
  }

  JointCreator creator;
  return creator.CreateWorldPoints(cog0, cog1, jointName, worldPoint);
}

bool PhysicsSpace::GetMode2D() const
{
  return mStateFlags.IsSet(PhysicsSpaceFlags::Mode2D);
}

void PhysicsSpace::SetMode2D(bool state)
{
  mStateFlags.SetState(PhysicsSpaceFlags::Mode2D,state);

  // Since we changed our default state we need to iterate over all
  // rigid bodies and force update any that were set to InheritFromSpace
  RigidBodyList::range r = mRigidBodies.All();
  for(; !r.Empty(); r.PopFront())
    r.Front().UpdateMode2D();
  r = mInactiveRigidBodies.All();
  for(; !r.Empty(); r.PopFront())
    r.Front().UpdateMode2D();
  r = mMovingKinematicBodies.All();
  for(; !r.Empty(); r.PopFront())
    r.Front().UpdateMode2D();
  r = mStoppedKinematicBodies.All();
  for(; !r.Empty(); r.PopFront())
    r.Front().UpdateMode2D();
  r = mInactiveKinematicBodies.All();
  for(; !r.Empty(); r.PopFront())
    r.Front().UpdateMode2D();
}

bool PhysicsSpace::GetDeterministic() const
{
  return mStateFlags.IsSet(PhysicsSpaceFlags::Deterministic);
}

void PhysicsSpace::SetDeterministic(bool state)
{
  mStateFlags.SetState(PhysicsSpaceFlags::Deterministic, state);
}

CollisionGroupInstance* PhysicsSpace::GetCollisionGroupInstance(ResourceId groupId) const
{
  return mCollisionTable->GetGroupInstance(groupId);
}

void PhysicsSpace::FixColliderCollisionGroups(ColliderList& colliders)
{
  // Re-fetch the collision group instance for every collider.
  // If it's group doesn't exist anymore it'll get the default group.
  CollisionGroupInstance* instance;
  ColliderList::range range = colliders.All();
  for(; !range.Empty(); range.PopFront())
  {
    Collider& collider = range.Front();
    ResourceId resourceId = collider.mCollisionGroupInstance->mResource->mResourceId;
    instance = mCollisionTable->GetGroupInstance(resourceId);
    collider.mCollisionGroupInstance = instance;
    collider.ForceAwake();
  }
}

CollisionTable* PhysicsSpace::GetCollisionTable()
{
  return mCollisionTable;
}

void PhysicsSpace::SetCollisionTable(CollisionTable* collisionTable)
{
  // Needs to be fixed to re-establish all collider's instances as well as make
  // sure all of their instance types are registered with the filter.
  if(collisionTable == nullptr)
    return;

  mCollisionTable = collisionTable;
  FixCollisionTable(mCollisionTable);
}

void PhysicsSpace::FixCollisionTable(CollisionTable* table)
{
  mCollisionTable = table;
  FixColliderCollisionGroups(mDynamicColliders);
  FixColliderCollisionGroups(mStaticColliders);
}

PhysicsSolverConfig* PhysicsSpace::GetPhysicsSolverConfig()
{
  return mPhysicsSolverConfig;
}

void PhysicsSpace::SetPhysicsSolverConfig(PhysicsSolverConfig* config)
{
  mPhysicsSolverConfig = config;
  mIslandManager->SetSolverConfig(mPhysicsSolverConfig);
}

void PhysicsSpace::UpdateModifiedResources()
{
  ConvexMeshManager::GetInstance()->UpdateAndNotifyModifiedResources();
  MultiConvexMeshManager::GetInstance()->UpdateAndNotifyModifiedResources();
  PhysicsMeshManager::GetInstance()->UpdateAndNotifyModifiedResources();
  PhysicsMaterialManager::GetInstance()->UpdateAndNotifyModifiedResources();
}

void PhysicsSpace::FrameUpdate()
{
  // Needed to make sure broadphase is updated for debug drawing
  PushBroadPhaseQueueProfiled();

  DebugDraw();
}

void PhysicsSpace::SystemLogicUpdate(UpdateEvent* updateEvent)
{
  ProfileScopeTree("Physics", "Engine", ByteColorRGBA(232, 0, 34, 255));

  {
    FpuExceptionsEnabler();

    // If any resources are modified then make sure to update them now (probably modified in script)
    UpdateModifiedResources();

    // This push is necessary to put physics into a valid state after the rest of the
    // engine may have done things to physics
    PushBroadPhaseQueueProfiled();

    ReturnIf(mSubStepCount == 0, , "Physics is set to have no iteration steps.");

    // If this is a preview space, don't run the timestep. This will prevent
    // previews from integrating forces, solving joints, etc...
    if(!GetSpace()->IsPreviewMode())
    {
      real frameTime = updateEvent->Dt;
      real dt = frameTime / real(mSubStepCount);
      for(uint i = 0; i < mSubStepCount; ++i)
        IterateTimestep(dt);
    }
  }

  // Now that we've updated our objects, tell the rest of the world what we've done.
  Publish();

  SolveSprings(0.016f);

  Cog* owner = GetOwner();
  ObjectEvent e(owner);
  owner->DispatchEvent(Events::PhysicsUpdateFinished, &e);
}

void PhysicsSpace::FlushPhysicsQueue()
{
  PushBroadPhaseQueue();
}

void PhysicsSpace::PushBroadPhaseQueue()
{
  mNodeManager->CommitChanges(mBroadPhase);
}

void PhysicsSpace::PushBroadPhaseQueueProfiled()
{
  mNodeManager->CommitChangesProfiled(mBroadPhase);
}

void PhysicsSpace::UpdateTransformAndMassOfTree(PhysicsNode* node)
{
  mNodeManager->UpdateNodeTree(node);
}

void PhysicsSpace::IterateTimestep(real dt)
{
  mIterationDt = dt;

  ProfileScopeTree("Iteration", "Physics", Color::SaddleBrown);

  {
    ProfileScopeTree("Velocity Integration", "Iteration", Color::Goldenrod);
    PreCalculateEffects(dt);
    ApplyHierarchyEffects(dt);
    UpdateRegions(dt);
    IntegrateBodiesVelocity(dt);
  }

  // Update the queues so that broadphase and transform are correct. It is also the
  // appropriate time to update the kinematic states so that we can avoid calculating
  // velocity (sometimes incorrectly) more than once a frame.
  PushBroadPhaseQueue();
  UpdateKinematicVelocities();

  BroadPhase();
  NarrowPhase();

  // Send out the pre-solve events
  PreSolve(dt);
  UpdatePhysicsCars(dt);

  ResolutionPhase(dt);
  WakeInactiveMovingBodies();

  // Deal with all kinematic objects that are
  // transitioning between moving and not moving.
  // Also deal with updating the old position of the kinematic body.
  UpdateKinematicState();

  {
    ProfileScopeTree("Position Integration", "Iteration", Color::Goldenrod);
    IntegrateBodiesPosition(dt);
  }

  SolvePositions(dt);

  // This needs to be done after position integration, otherwise the wheels
  // will not be at the correct spot after they change their translation
  // from raycasting since they are positioned based upon their parent's position.
  UpdatePhysicsCarsTransforms(dt);

  PushBroadPhaseQueue();
}

void PhysicsSpace::IntegrateBodiesVelocity(real dt)
{
  RigidBodyList::range range = mRigidBodies.All();

  while(!range.Empty())
  {
    RigidBody& body = range.Front();
    range.PopFront();

    bool isKinematic = body.GetKinematic();
    ErrorIf(isKinematic, "Kinematic object should not be in the rigid body list.");
    
    ApplyGlobalEffects(&body, dt);
    body.UpdateBodyEffects(dt);

    // Check for asleep bodies
    if(body.mState.IsSet(RigidBodyStates::Asleep))
    {
      // Change to the inactive list
      mRigidBodies.Erase(&body);
      mInactiveRigidBodies.PushBack(&body);
      continue;
    }

    if(!body.GetStatic())
      Physics::Integration::IntegrateVelocity(&body, dt);

    body.mForceAccumulator.ZeroOut();
    body.mTorqueAccumulator.ZeroOut();
  }
}

void PhysicsSpace::IntegrateBodiesPosition(real dt)
{
  RigidBodyList::range range = mRigidBodies.All();

  while(!range.Empty())
  {
    RigidBody& body = range.Front();

    if(!body.GetStatic())
    {
      Physics::Integration::IntegratePosition(&body, dt);
      // Attempt to sleep the body.
      body.UpdateSleepTimer(dt);
    }

    range.PopFront();
  }
}

void PhysicsSpace::BroadPhase()
{
  ProfileScopeTree("BroadPhase", "Iteration", Color::SaddleBrown);
  mPossiblePairs.Clear();

  BroadPhaseData broadPhaseData;
  BroadPhaseDataArray dataArray;
  dataArray.Reserve(mPossiblePairs.capacity());

  ColliderList::range range = mDynamicColliders.All();

  while(!range.Empty())
  {
    Collider& collider = range.Front();

    // If the object is asleep or flagged as static, there's no reason to test it for collision.
    if((collider.IsAsleep()
        && !collider.mState.IsSet(ColliderFlags::Uninitialized))
        || collider.IsStatic())
    {
      range.PopFront();
      continue;
    }

    ColliderToBroadPhaseData(&collider, broadPhaseData);
    dataArray.PushBack(broadPhaseData);
    range.PopFront();
  }

  mBroadPhase->RegisterCollisions();
  // Query the dynamic broad phase
  mBroadPhase->SelfQuery(mPossiblePairs);
  // Query the static broad phase
  mBroadPhase->BatchQuery(dataArray, mPossiblePairs);

  // Sort the pairs for determinism!
  if(GetDeterministic())
    Sort(mPossiblePairs.All(), &ClientPairSorter);
}

void PhysicsSpace::NarrowPhase()
{
  ProfileScopeTree("NarrowPhase", "Iteration", Color::Salmon);

  HeapAllocator allocator(mHeap);
  Physics::ManifoldArray tempManifolds;
  tempManifolds.SetAllocator(allocator);

  Array<NodePointerPair> Collisions;
  Collisions.SetAllocator(allocator);

  uint size = mPossiblePairs.Size();
  for(unsigned pairIndex = 0; pairIndex < size; ++pairIndex)
  {
    ClientPair* clientPair = &mPossiblePairs[pairIndex];
    Collider* collider1 = static_cast<Collider*>(clientPair->mClientData[0]);
    Collider* collider2 = static_cast<Collider*>(clientPair->mClientData[1]);
    // Convert the proxy to a collider
    ColliderPair pair(collider1, collider2);

    // Test for collision
    if(!mCollisionManager->TestCollision(pair, tempManifolds))
    {
      tempManifolds.Clear();
      continue;
    }

    // If tracking is enabled, we need to record the collision
    if(mBroadPhase->IsTracking())
    {
      NodePointerPair nodePair(clientPair->mClientData[0],
                               clientPair->mClientData[1]);
      Collisions.PushBack(nodePair);
    }

    // Add all manifolds to the contact manager
    for(uint i = 0; i < tempManifolds.Size(); ++i)
    {
      Physics::Manifold& manifold = tempManifolds[i];
      mContactManager->AddManifold(tempManifolds[i]);
      manifold.Clear();
    }

    tempManifolds.Clear();
  }

  mBroadPhase->RecordFrameResults(Collisions);

  // We have all connections for the frame so build the islands.
  mIslandManager->BuildIslands(mDynamicColliders);
}

void PhysicsSpace::PreSolve(real dt)
{
  // Send out pre-solve events
  ProfileScopeTree("PreSolveEvents", "Iteration", Color::Peru);
  mEventManager->DispatchPreSolveEvents(this);
  PushBroadPhaseQueue();
}

void PhysicsSpace::ResolutionPhase(real dt)
{
  // Solve all velocity constraints
  ProfileScopeTree("ResolutionPhase", "Iteration", Color::OrangeRed);
  mIslandManager->Solve(dt, GetAllowSleep(), mDebugDrawFlags.Field);
}

void PhysicsSpace::SolvePositions(real dt)
{
  // Solve all position constraints
  ProfileScopeTree("SolvePositions", "Iteration", Color::PaleTurquoise);
  mIslandManager->SolvePositions(dt);
}

void PhysicsSpace::SolveSprings(real dt)
{
  // Make sure to solve a system only once
  HashSet<SpringSystem*> systems;
  SpringGroup group;

  SpringSystems::range range = mSprings.All();
  for(; !range.Empty(); range.PopFront())
  {
    group.mSystems.Clear();

    SpringSystem* system = &range.Front();

    Array<SpringSystem*> stack;
    stack.PushBack(system);

    // Start a new grouping (until our stack is empty)
    while(!stack.Empty())
    {
      system = stack.Back();
      stack.PopBack();

      // We've already added this system
      if(systems.Contains(system))
        continue;

      systems.Insert(system);
      group.mSystems.PushBack(system);

      // Add all connected systems of edges this system owns
      SpringSystem::OwnedEdgeList::range ownedRange = system->mOwnedEdges.All();
      for(; !ownedRange.Empty(); ownedRange.PopFront())
      {
        SpringSystem::SystemConnection& connection = ownedRange.Front();
        
        if(systems.Contains(connection.mOtherSystem) == false)
          stack.PushBack(connection.mOtherSystem);
      }
      // Add all connected systems of edges this system doesn't own
      SpringSystem::ConnectedEdgeList::range otherRange = system->mConnectedEdges.All();
      for(; !otherRange.Empty(); otherRange.PopFront())
      {
        SpringSystem::SystemConnection& connection = otherRange.Front();

        if(systems.Contains(connection.mOwningSystem) == false)
          stack.PushBack(connection.mOwningSystem);
      }
    }

    // Solve the entire grouping of systems together
    group.Solve(this, dt);
  }
  group.mSystems.Clear();
}

//---------------------------------------------------------------- Ray Casting
void PhysicsSpace::CastRay(const Ray& worldRay, CastResults& results)
{
  // Have to always push here because otherwise an object that has already been
  // deleted could be returned
  PushBroadPhaseQueue();
  mBroadPhase->CastRay(worldRay.Start, worldRay.Direction.AttemptNormalized(), results.mResults);
  results.ConvertToColliders();
}

CastResult PhysicsSpace::CastRayFirst(const Ray& worldRay)
{
  CastFilter filter;
  return CastRayFirst(worldRay, filter);
}

CastResult PhysicsSpace::CastRayFirst(const Ray& worldRay, CastFilter& filter)
{
  // Push any changes made to objects
  PushBroadPhaseQueue();

  CastResults results(1, filter);
  mBroadPhase->CastRay(worldRay.Start, worldRay.Direction.AttemptNormalized(), results.mResults);  
  results.ConvertToColliders();

  // Deal with an empty range
  if(results.All().Empty())
    return CastResult();

  // Otherwise, return the object hit
  return results.All().Front();
}

CastResultsRange PhysicsSpace::CastRay(const Ray& worldRay, uint maxCount)
{
  CastFilter filter;
  return CastRay(worldRay, maxCount, filter);
}

CastResultsRange PhysicsSpace::CastRay(const Ray& worldRay, uint maxCount, CastFilter& filter)
{
  PushBroadPhaseQueue();
  CastResults results(maxCount, filter);
  CastRay(worldRay, results);
  return CastResultsRange(results);
}

void PhysicsSpace::CastSegment(const Segment& segment, CastResults& results)
{
  PushBroadPhaseQueue();
  mBroadPhase->CastSegment(segment.Start, segment.End, results.mResults);
  results.ConvertToColliders();
}

CastResultsRange PhysicsSpace::CastSegment(const Segment& segment, uint count)
{
  CastFilter filter;
  return CastSegment(segment, count, filter);
}

CastResultsRange PhysicsSpace::CastSegment(const Segment& segment, uint maxCount, CastFilter& filter)
{
  CastResults results(maxCount, filter);
  CastSegment(segment, results);
  return CastResultsRange(results);
}

void PhysicsSpace::CastAabb(const Aabb& aabb, CastResults& results)
{
  BaseCastFilter& filter = results.mResults.Filter;
  filter.ClearFlag(BaseCastFilterFlags::IgnoreInternalCasts);

  PushBroadPhaseQueue();
  mBroadPhase->CastAabb(aabb, results.mResults);
  results.ConvertToColliders();
}

CastResultsRange PhysicsSpace::CastAabb(const Aabb& aabb, uint maxCount, CastFilter& filter)
{
  CastResults results(maxCount, filter);
  CastAabb(aabb, results);
  return CastResultsRange(results);
}

void PhysicsSpace::CastSphere(const Sphere& sphere, CastResults& results)
{
  BaseCastFilter& filter = results.mResults.Filter;
  filter.ClearFlag(BaseCastFilterFlags::IgnoreInternalCasts);

  PushBroadPhaseQueue();
  mBroadPhase->CastSphere(sphere, results.mResults);
  results.ConvertToColliders();
}

CastResultsRange PhysicsSpace::CastSphere(const Sphere& sphere, uint maxCount, CastFilter& filter)
{
  CastResults results(maxCount, filter);
  CastSphere(sphere, results);
  return CastResultsRange(results);
}

void PhysicsSpace::CastFrustum(const Frustum& frustum, CastResults& results)
{
  BaseCastFilter& filter = results.mResults.Filter;
  filter.ClearFlag(BaseCastFilterFlags::IgnoreInternalCasts);

  PushBroadPhaseQueue();
  mBroadPhase->CastFrustum(frustum, results.mResults);
  results.ConvertToColliders();
}

CastResultsRange PhysicsSpace::CastFrustum(const Frustum& frustum, uint maxCount, CastFilter& filter)
{
  CastResults results(maxCount, filter);
  CastFrustum(frustum, results);
  return CastResultsRange(results);
}

void PhysicsSpace::CastCollider(Vec3Param offset, Collider* testCollider, Physics::ManifoldArray& results, CastFilter& filter)
{
  // Set the offset variable on the collider (total hack variable!!)
  testCollider->mCollisionOffset = offset;
  // Make sure broadphase is up to date
  PushBroadPhaseQueue();

  //Make the broadphase data for the test collider
  BroadPhaseData data;
  ColliderToBroadPhaseData(testCollider, data);
  Vec3 worldPos = testCollider->GetWorldTranslation() + offset;
  data.mAabb.SetCenter(worldPos);
  data.mBoundingSphere.mCenter = worldPos;

  // Query both static and dynamic broadphase to see what to do narrow phase on
  ClientPairArray possiblePairs;
  mBroadPhase->QueryBoth(data, possiblePairs);

  // Run narrow phase
  for(uint i = 0; i < possiblePairs.Size(); ++i)
  {
    ClientPair* clientPair = &possiblePairs[i];
    Collider* collider1 = static_cast<Collider*>(clientPair->mClientData[0]);
    Collider* collider2 = static_cast<Collider*>(clientPair->mClientData[1]);

    // Isn't really necessary, but figure out which collider is the cast collider
    Collider* otherCollider = collider1;
    if(collider1 == testCollider)
      otherCollider = collider2;
    if(testCollider == otherCollider)
      continue;

    // If the other collider isn't valid according to the filter then skip it
    if(!filter.IsValid(otherCollider))
      continue;

    // Set the pair's order so that the test collider is always ColliderA.
    ColliderPair pair;
    pair.A = testCollider;
    pair.B = otherCollider;
    // Force test will ignore flags like sleeping and testing static against
    // static and other stuff like that. Aka, it always tests the two objects.
    mCollisionManager->ForceTestCollision(pair, results);
  }

  // Set the collision offset back to zero
  testCollider->mCollisionOffset.ZeroOut();
}

CastResultsRange PhysicsSpace::CastCollider(Vec3Param offset, Collider* testCollider, CastFilter& filter)
{
  // Safeguard against bad input
  if(testCollider == nullptr)
  {
    DoNotifyException("Invalid Collider Cast", "Cannot cast a null Collider");
    return CastResultsRange(CastResults());
  }

  // Call the internal cast function
  Physics::ManifoldArray manifoldResults;
  CastCollider(offset, testCollider, manifoldResults, filter);
  CastResults results(Math::Max(manifoldResults.Size() * cMaxContacts, (size_t)1), filter);

  size_t count = 0;
  for (size_t i = 0; i < manifoldResults.Size(); ++i)
  {
    CastResult& castResult = results.mArray[count];
    Physics::Manifold& manifold = manifoldResults[i];

    for (size_t c = 0; c < manifold.ContactCount; ++c)
    {
      Physics::ManifoldPoint& contact = manifold.Contacts[c];

      castResult.mContactNormal = contact.Normal;
      float normalLength = contact.Normal.Length();

      if (normalLength != 0.0f)
      {
        castResult.mContactNormal /= normalLength;
        castResult.mTime = contact.Penetration / normalLength;
      }
      else
      {
        continue;
      }

      if (manifold.Objects.A == testCollider)
      {
        castResult.mObjectHit = manifold.Objects.B;
        castResult.mContactNormal = -castResult.mContactNormal;
      }
      else
      {
        castResult.mObjectHit = manifold.Objects.A;
      }

      castResult.mPoints[0] = contact.WorldPoints[0];
      castResult.mPoints[1] = contact.WorldPoints[1];
    }
    ++count;
  }

  results.mResults.CurrSize = count;
  return CastResultsRange(results);
}

SweepResultRange PhysicsSpace::SweepCollider(Collider* collider, Vec3Param velocity, real dt, CastFilter& filter)
{
  // No Collider, return an empty range
  if (!collider)
    return SweepResultRange(SweepResultArray());

  // Update position changes due to game logic
  collider->mPhysicsNode->ReadTransform();
  collider->mPhysicsNode->RecomputeWorldTransform();

  // Use the collider to get the world translation as it may have a
  // translation offset separate from the transform's position
  Vec3 pos = collider->GetWorldTranslation();
  Vec3 vel = velocity;
  Capsule sweptBoundingSphere(pos, pos + vel * dt, collider->mBoundingSphere.mRadius);
  Aabb sweptVolume = ToAabb(sweptBoundingSphere);

  // Cleared for the broadphase query, otherwise it will ignore some results that are needed
  filter.ClearFlag(BaseCastFilterFlags::IgnoreInternalCasts);

  CastResults castResults(128, filter);
  mBroadPhase->CastAabb(sweptVolume, castResults.mResults);

  SweepResultArray impacts;
  forRange (CastResult castResult, castResults.All())
  {
    Collider* otherCollider = castResult.GetCollider();
    if (otherCollider == collider)
      continue;
    // Not done by the broadphase query, probably should be
    if (filter.IsSet(BaseCastFilterFlags::IgnoreChildren) && collider->GetOwner()->IsDescendant(otherCollider->GetOwner()))
      continue;

    SweepResult impact;
    TimeOfImpactData data(collider, otherCollider, dt, velocity, true);
    TimeOfImpact(&data);

    for (unsigned i = 0; i < data.ImpactTimes.Size(); ++i)
    {
      impact.mTime = data.ImpactTimes[i];
      Physics::Manifold& manifold = data.Manifolds[i];

      if (impact.mTime < dt)
      {
        impact.mOtherCollider = otherCollider;
        impact.mPenetration = manifold.GetPoint(0).Penetration;

        if (collider == manifold.Objects.A)
        {
          impact.mWorldPoint = manifold.GetPoint(0).WorldPoints[0];
          impact.mWorldNormalTowardsSelf = -manifold.GetPoint(0).Normal;
        }
        else
        {
          impact.mWorldPoint = manifold.GetPoint(0).WorldPoints[1];
          impact.mWorldNormalTowardsSelf = manifold.GetPoint(0).Normal;
        }
        impacts.PushBack(impact);
      }
    }
  }

  Sort(impacts.All());

  SweepResultRange range(impacts);
  return range;
}

void PhysicsSpace::DispatchWithinSphere(const Sphere& sphere, StringParam eventName, Event* toSend)
{
  CastResults results(100);
  CastSphere(sphere, results);
  CastResults::range r = results.All();
  while(!r.Empty())
  {
    Cog* object = r.Front().GetCollider()->GetOwner();
    object->DispatchEvent(eventName, toSend);
    r.PopFront();
  }
}

void PhysicsSpace::DispatchWithinAabb(const Aabb& aabb, StringParam eventName, Event* toSend)
{
  CastResults results(100);
  CastAabb(aabb, results);
  CastResults::range r = results.All();
  while(!r.Empty())
  {
    Cog* object = r.Front().GetCollider()->GetOwner();
    object->DispatchEvent(eventName, toSend);
    r.PopFront();
  }
}

uint PhysicsSpace::GetSubStepCount() const
{
  return mSubStepCount;
}

void PhysicsSpace::SetSubStepCount(uint substeps)
{
  if(substeps < 1 || 50 < substeps)
  {
    substeps = Math::Clamp(substeps, 1u, 50u);
    DoNotifyWarning("Invalid SubSteps", "The sub-step count of physics must be between 1 and 50. The value has been clamped.");
  }
  mSubStepCount = substeps;
}

bool PhysicsSpace::GetAllowSleep() const
{
  return mStateFlags.IsSet(PhysicsSpaceFlags::AllowSleep);
}

void PhysicsSpace::SetAllowSleep(bool allowSleep)
{
  mStateFlags.SetState(PhysicsSpaceFlags::AllowSleep, allowSleep);

  ColliderList::range range = mDynamicColliders.All();
  while(!range.Empty())
  {
    range.Front().ForceAwake();
    range.PopFront();
  }
}

void PhysicsSpace::SerializeBroadPhases(Serializer& stream)
{
  // Allocate the broad phase if we're loading
  if(stream.GetMode() == SerializerMode::Loading)
    mBroadPhase = new BroadPhasePackage();

  mBroadPhase->Serialize(stream);
}

void PhysicsSpace::Publish()
{
  ProfileScopeTree("Publish", "Physics", Color::RosyBrown);
  // Publish rigid body transforms
  RigidBodyList::range range = mRigidBodies.All();
  while(!range.Empty())
  {
    RigidBody& r = range.Front();
    range.PopFront();

    r.PublishTransform();
  }

  // Now send out all events
  PublishEvents();
}

void PhysicsSpace::PublishEvents()
{
  mEventManager->DispatchEvents(this);
  // Now that we have sent all events, we can delete the contacts that we
  // queued up for delayed destruction. The contacts are delay destructed to avoid
  // deleting their manifold before the events are sent.
  mContactManager->DestroyContacts();
}

void PhysicsSpace::PreCalculateEffects(real dt)
{
  // Tell all effects to cache any information that doesn't change between bodies
  // (world vectors, etc...). Unfortunately this is called on effects that will
  // never apply to anything (regions with no collisions) but the cost should be minimal.
  SpaceEffectList::range effects = mEffects.All();
  for(; !effects.Empty(); effects.PopFront())
  {
    PhysicsEffect& effect = effects.Front();
    effect.PreCalculate(dt);
  }
}

void PhysicsSpace::UpdateRegions(real dt)
{
  RegionList::range range = mRegions.All();

  while(!range.Empty())
  {
    Region& region = range.Front();
    region.Update(dt);
    range.PopFront();
  }
}

void PhysicsSpace::ApplyHierarchyEffects(real dt)
{
  // If an effect is somewhere randomly in a hierarchy then
  // apply it to the nearest parent rigid body
  PhysicsEffectList::range effects = mHierarchyEffects.All();
  for(; !effects.Empty(); effects.PopFront())
  {
    PhysicsEffect& effect = effects.Front();
    // Find the nearest parent rigid body
    Cog* cog = effect.GetOwner();
    while(cog)
    {
      RigidBody* body = cog->has(RigidBody);
      if(body != nullptr)
      {
        effect.ApplyEffect(body, dt);
        break;
      }
      cog = cog->GetParent();
    }
  }
}

void PhysicsSpace::ApplyGlobalEffects(RigidBody* body, real dt)
{
  PhysicsEffectList::range range = mGlobalEffects.All();

  // Deal with IgnoreSpaceEffects
  IgnoreSpaceEffects* effectsToIgnore = body->mSpaceEffectsToIgnore;
  if(effectsToIgnore)
  {
    while(!range.Empty())
    {
      PhysicsEffect& effect = range.Front();

      // Skip if this effect type is ignored or not active
      if(!effectsToIgnore->IsIgnored(&effect) && effect.GetActive())
        effect.ApplyEffect(body, dt);

      range.PopFront();
    }
    return;
  }

  while(!range.Empty())
  {
    PhysicsEffect& effect = range.Front();

    if(effect.GetActive())
      effect.ApplyEffect(body, dt);

    range.PopFront();
  }
}

void PhysicsSpace::WakeInactiveMovingBodies()
{
  RigidBodyList::range range = mInactiveRigidBodies.All();

  while(!range.Empty())
  {
    RigidBody& body = range.Front();
    range.PopFront();

    if(body.GetStatic())
      continue;

    if(body.mVelocity.LengthSq() != real(0.0) ||
       body.mAngularVelocity.Length() != real(0.0))
      body.WakeUp();
  }
}

void PhysicsSpace::ForceAwakeRigidBodies()
{
  RigidBodyList::range range = mInactiveRigidBodies.All();

  while(!range.Empty())
  {
    RigidBody& body = range.Front();
    range.PopFront();

    if(body.GetStatic())
      continue;

    body.ForceAwake();
  }
}

String PhysicsSpace::WhyAreTheyNotColliding(Cog* cogA, Cog* cogB)
{
  if(cogA == nullptr)
    return "CogA is null...";
  if(cogB == nullptr)
    return "CogB is null...";
  if(cogA == cogB)
    return "The same cog was passed in";

  Collider* colliderA = cogA->has(Collider);
  Collider* colliderB = cogB->has(Collider);

  String cogADescription = cogA->GetDescription();
  String cogBDescription = cogB->GetDescription();

  if(colliderA == nullptr)
    return String::Format("Cog A (%s) doesn't have a collider.", cogADescription.c_str());
  if(colliderB == nullptr)
    return String::Format("Cog B (%s) doesn't have a collider.", cogBDescription.c_str());

  String colliderADescription = colliderA->GetDescription();
  String colliderBDescription = colliderB->GetDescription();

  if(colliderA->mSpace != colliderB->mSpace)
    return "Colliders are in different spaces";

  if(colliderA->IsStatic() && colliderB->IsStatic())
    return "Both colliders are static. Static colliders don't check against each other.";

  if(!colliderA->mPhysicsNode->IsInBroadPhase())
    return String::Format("Collider A (%s) is not in broadphase yet. Was it just created this frame?", colliderADescription.c_str());
  if(!colliderB->mPhysicsNode->IsInBroadPhase())
    return String::Format("Collider B (%s) is not in broadphase yet. Was it just created this frame?", colliderBDescription.c_str());

  // Check to see if colliderA hits colliderB
  ClientPairArray results;
  BroadPhaseData data;
  ColliderToBroadPhaseData(colliderA, data);
  mBroadPhase->QueryBoth(data, results);

  bool passed = false;
  ClientPairArray::range resultRange = results.All();
  for(; !resultRange.Empty(); resultRange.PopFront())
  {
    ClientPair pair = resultRange.Front();

    if(pair.mClientData[0] == colliderB || pair.mClientData[1] == colliderB)
    {
      passed = true;
      break;
    }
  }

  if(passed == false)
    return "Colliders did not pass broadphase, they are probably too far apart. (Maybe separating on the z axis?).";

  if(colliderA->IsAsleep() && colliderB->IsAsleep())
    return "Both objects are asleep.";

  // Check all the random reasons for not colliding such as joints
  // saying to skip collision, collision tables, etc...
  String reasonForNotColliding;
  if(colliderA->ShouldCollide(colliderB, reasonForNotColliding) == false)
    return reasonForNotColliding;

  // Check narrowphase
  Physics::ManifoldArray tempManifolds;
  ColliderPair pair(colliderA, colliderB);
  if(mCollisionManager->TestCollision(pair, tempManifolds) == false)
    return "They didn't pass collision detection, they aren't actually colliding.";

  // At this point we know they passed narrowphase, but they might have not collided for some other reason.
  // Also, whether or not they collide is now completely independent from whether or not they send events

  // First determine a string for what's happening with events
  String eventsString = "Both colliders send events";
  if(colliderA->GetSendsEvents() == false && colliderB->GetSendsEvents() == false)
    eventsString = "Both colliders don't send events.";
  else if(colliderA->GetSendsEvents() == false)
    eventsString = String::Format("Collider A (%s) doesn't send events.", colliderADescription.c_str());
  else if(colliderB->GetSendsEvents() == false)
    eventsString = String::Format("Collider B (%s) doesn't send events.", colliderADescription.c_str());

  // If either is ghost they won't resolve, but they could still send events
  if(colliderA->GetGhost() || colliderB->GetGhost())
    return BuildString("One collider is marked ghost. ", eventsString);

  // If the collision group says to not check collision, then don't
  if(colliderA->mCollisionGroupInstance->SkipResolution(*(colliderB->mCollisionGroupInstance)))
    return BuildString("Collision table says they skip resolution. ", eventsString);

  return BuildString("They do collide. ", eventsString);
}

void PhysicsSpace::AddComponent(RigidBody* body)
{
  // If the object inherits from the space the override
  // the 2d state, otherwise it's already correct
  if(body->mState.IsSet(RigidBodyStates::Inherit2DMode))
  {
    if(GetMode2D() == true)
      body->Set2DInternal(true);
    else
      body->Set2DInternal(false);
  }

  if(body->mState.IsSet(RigidBodyStates::Kinematic))
    mMovingKinematicBodies.PushBack(body);
  else if(body->mState.IsSet(RigidBodyStates::Asleep | RigidBodyStates::Static))
    mInactiveRigidBodies.PushBack(body);
  else
    mRigidBodies.PushBack(body);
}

void PhysicsSpace::RemoveComponent(RigidBody* body)
{
  // This can be in several lists, unlink from whichever
  RigidBodyList::Unlink(body);
}

void PhysicsSpace::ComponentStateChange(RigidBody* body)
{
  RigidBodyList::Unlink(body);

  if(body->GetStatic() || body->IsAsleep())
    mInactiveRigidBodies.PushBack(body);
  else if(body->GetKinematic())
    mMovingKinematicBodies.PushBack(body);
  else
    mRigidBodies.PushBack(body);
}

void PhysicsSpace::AddComponent(Joint* joint)
{
  mJoints.PushBack(joint);
}

void PhysicsSpace::RemoveComponent(Joint* joint)
{
  mJoints.Unlink(joint);
}

void PhysicsSpace::AddComponent(Region* region)
{
  mRegions.PushBack(region);
}

void PhysicsSpace::RemoveComponent(Region* region)
{
  mRegions.Erase(region);
}

void PhysicsSpace::AddComponent(Collider* collider)
{
  if(collider->GetActiveBody())
    mDynamicColliders.PushBack(collider);
  else
    mStaticColliders.PushBack(collider);
}

void PhysicsSpace::RemoveComponent(Collider* collider)
{
  mIslandManager->RemoveCollider(collider);
  ColliderList::Unlink(collider);

  //////////////////////////////////////////////////////////////////////////
  // Doesn't work now because the id we have at this point is already gone.
  // Valid code below for when the id problem is fixed


  if(!collider->GetHasPairFilter())
    return;

  u32 id = CogId(collider->GetOwner()).GetId();

  HashSet<u64>::range range = mFilteredPairs.All();
  while(!range.Empty())
  {
    u64 packedId = range.Front();
    range.PopFront();

    u32 id1, id2;
    UnPackLexicographicId(id1, id2, packedId);
    if(id1 == id || id2 == id)
      mFilteredPairs.Erase(packedId);
  }
}

void PhysicsSpace::ComponentStateChange(Collider* collider)
{
  ColliderList::Unlink(collider);

  if(!collider->GetActiveBody())
    mStaticColliders.PushBack(collider);
  else
    mDynamicColliders.PushBack(collider);
}

void PhysicsSpace::AddComponent(PhysicsCar* car)
{
  mCars.PushBack(car);
}

void PhysicsSpace::RemoveComponent(PhysicsCar* car)
{
  CarList::Unlink(car);
}

void PhysicsSpace::AddComponent(SpringSystem* system)
{
  mSprings.PushBack(system);
}

void PhysicsSpace::RemoveComponent(SpringSystem* system)
{
  SpringSystems::Unlink(system);
}

PhysicsEffectList::range PhysicsSpace::GetGlobalEffects()
{
  return mGlobalEffects.All();
}

void PhysicsSpace::AddGlobalEffect(PhysicsEffect* effect)
{
  mGlobalEffects.PushBack(effect);
}

void PhysicsSpace::RemoveGlobalEffect(PhysicsEffect* effect)
{
  mGlobalEffects.Erase(effect);
}

void PhysicsSpace::QueuePhysicsNode(PhysicsNode* node)
{
  mNodeManager->AddNode(node);
}

PhysicsSpace::IslandColliderList::range PhysicsSpace::GetAllInIsland(Collider* collider)
{
  Physics::Island* island = mIslandManager->GetObjectsIsland(collider);
  if(island)
    return island->mColliders.All();

  return Physics::Island::Colliders::range();
}

BroadPhasePackage* PhysicsSpace::ReplaceBroadPhase(BroadPhasePackage* newBroadPhase)
{
  // Remove all dynamic colliders from the broadphase
  ColliderList::range r = mDynamicColliders.All();
  while(!r.Empty())
  {
    Collider* collider = &r.Front();
    RemovalAction action(collider);
    r.PopFront();
  }

  // Remove all static colliders from the broadphase
  r = mStaticColliders.All();
  while(!r.Empty())
  {
    Collider* collider = &r.Front();
    RemovalAction action(collider);
    r.PopFront();
  }

  FlushPhysicsQueue();

  // Swap the broad phases
  BroadPhasePackage* old = mBroadPhase;
  mBroadPhase = newBroadPhase;

  // Re-insert all colliders
  r = mDynamicColliders.All();
  while(!r.Empty())
  {
    Collider* collider = &r.Front();
    InsertionAction action(collider);
    r.PopFront();
  }

  r = mStaticColliders.All();
  while(!r.Empty())
  {
    Collider* collider = &r.Front();
    InsertionAction action(collider);
    r.PopFront();
  }

  FlushPhysicsQueue();

  return old;
}

Physics::CollisionManager* PhysicsSpace::GetCollisionManager()
{
  return mCollisionManager;
}

void PhysicsSpace::ActivateKinematic(RigidBody* body)
{
  RigidBodyList::Unlink(body);
  mMovingKinematicBodies.PushBack(body);
}

void PhysicsSpace::UpdateKinematicVelocities()
{
  // Now that we've finished our physics frame, it's safe
  // to update the old transform values of this body and update the kinematic
  // velocity from where we were to where we ended up.
  RigidBodyList::range movingBodyRange = mMovingKinematicBodies.All();
  for(; !movingBodyRange.Empty(); movingBodyRange.PopFront())
  {
    RigidBody& body = movingBodyRange.Front();
    WorldTransformation* transform = body.mPhysicsNode->GetTransform();

    Vec3 oldTranslation = transform->GetOldTranslation();
    Mat3 oldRotation = transform->GetOldRotation();
    transform->ComputeOldValues();
    body.ComputeVelocities(oldTranslation,oldRotation, mIterationDt);
  }
}

void PhysicsSpace::UpdateKinematicState()
{
  // Take all of the stopped kinematic bodies
  // (ones who are just now not getting transform updates)
  // and clear out their velocity. Now they are inactive
  // (since they aren't moving).
  RigidBodyList::range range = mStoppedKinematicBodies.All();
  for(; !range.Empty(); range.PopFront())
  {
    RigidBody& body = range.Front();

    body.mVelocity.ZeroOut();
    body.mAngularVelocity.ZeroOut();
  }

  // Move all of the stopped bodies to the inactive list
  if(!mStoppedKinematicBodies.Empty())
  {
    mInactiveKinematicBodies.Splice(mInactiveKinematicBodies.End(),
                                    mStoppedKinematicBodies.All());
  }

  // Take all of the moving kinematics and move them to the stopped list.
  // If they get a transform update between the end of this frame and the
  // beginning of next, they will move themselves back into the moving list.
  mStoppedKinematicBodies.Swap(mMovingKinematicBodies);
}

void PhysicsSpace::UpdatePhysicsCars(real dt)
{
  CarList::range range = mCars.All();
  while(!range.Empty())
  {
    range.Front().Update(dt);
    range.PopFront();
  }
}

void PhysicsSpace::UpdatePhysicsCarsTransforms(real dt)
{
  CarList::range range = mCars.All();
  while(!range.Empty())
  {
    range.Front().UpdatePositions(dt);
    range.PopFront();
  }
}

void PhysicsSpace::DebugDraw()
{
  //Debug::DefaultConfig config;
  //config.SpaceId(this->GetOwner()->GetId().Id);

  ProfileScopeTree("DebugDraw", "Physics", Color::Yellow);
  if(mDebugDrawFlags.IsSet(PhysicsSpaceDebugDrawFlags::DrawDebug))
  {
    mIslandManager->Draw(mDebugDrawFlags.Field);

    DrawColliders();
    DrawRigidBodies();

    if(mDebugDrawFlags.IsSet(PhysicsSpaceDebugDrawFlags::DrawBroadPhase))
      mBroadPhase->Draw(mDrawLevel, mDebugDrawFlags.Field);

    if(mDebugDrawFlags.IsSet(PhysicsSpaceDebugDrawFlags::DrawSleeping))
      DrawInactiveObjects();
  }
}

void PhysicsSpace::DrawColliders()
{
  if(!mDebugDrawFlags.IsSet(PhysicsSpaceDebugDrawFlags::DrawBroadPhase))
    return;

  bool onTop = mDebugDrawFlags.IsSet(PhysicsSpaceDebugDrawFlags::DrawOnTop);

  ColliderList::range range = mDynamicColliders.All();

  while(!range.Empty())
  {
    gDebugDraw->Add(Debug::Obb(range.Front().mAabb).Color(Color::MintCream).OnTop(onTop));
    gDebugDraw->Add(Debug::Sphere(range.Front().mBoundingSphere).OnTop(onTop));
    ContactRange contacts = FilterContactRange(&range.Front());
    for(; !contacts.Empty(); contacts.PopFront())
      contacts.Front().GetConstraint().DebugDraw();
    range.PopFront();
  }
}

void PhysicsSpace::DrawRigidBodies()
{
  if(!mDebugDrawFlags.IsSet(PhysicsSpaceDebugDrawFlags::DrawCenterMass))
    return;

  RigidBodyList::range range = mRigidBodies.All();
  for(; !range.Empty(); range.PopFront())
  {
    Vec3Param centerMass = range.Front().GetWorldCenterOfMass();
    gDebugDraw->Add(Debug::LineCross(centerMass, 2));
  }
}

void PhysicsSpace::DrawInactiveObjects()
{
  RigidBodyList::range range = mInactiveRigidBodies.All();

  while(!range.Empty())
  {
    RigidBody& body = range.Front();
    RigidBody::CompositeColliderList::range colliders = body.GetColliders();

    while(!colliders.Empty())
    {
      Aabb aabb = colliders.Front().mAabb;
      gDebugDraw->Add(Debug::Obb(aabb.GetCenter(), aabb.GetHalfExtents()).Color(Color::Gold));
      colliders.PopFront();
    }
    range.PopFront();
  }
}

void PhysicsSpace::ColliderToBroadPhaseData(Collider* collider, BroadPhaseData& data)
{
  data.mAabb = collider->mAabb;
  data.mClientData = (void*)collider;
  data.mBoundingSphere = collider->mBoundingSphere;
}

}//namespace Zero
