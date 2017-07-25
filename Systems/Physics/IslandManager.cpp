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

  
typedef Array<Collider*, HeapAllocator> ColliderStack;


void AddTreeToStack(Collider* collider, ColliderStack& stack)
{
  //if any collider in a tree is marked as on an island,
  //that means we've put the whole tree on the island
  if(collider->mState.IsSet(ColliderFlags::OnIsland))
    return;

  RigidBody* body = collider->GetActiveBody();
  if(body == nullptr)
    return;

  //loop up to find a dynamic parent body
  while(!body->IsDynamic() && body->mParentBody)
    body = body->mParentBody;

  if(body == nullptr)
    return;

  //now that we have the root body of the tree (dynamic root), we can
  //loop through the tree and add all colliders to the stack
  Array<RigidBody*> bodyStack;
  bodyStack.PushBack(body);

  while(!bodyStack.Empty())
  {
    RigidBody* currentBody = bodyStack.Back();
    bodyStack.PopBack();

    //add all child bodies that aren't dynamic to the body stack
    RigidBody::BodyRange bodies = currentBody->mChildBodies.All();
    for(; !bodies.Empty(); bodies.PopFront())
    {
      RigidBody* childBody = &bodies.Front();
      if(!childBody->IsDynamic())
        bodyStack.PushBack(childBody);
    }

    //add all colliders of this body to the body stack
    RigidBody::CompositeColliderRange colliders = currentBody->mColliders.All();
    for(; !colliders.Empty(); colliders.PopFront())
    {
      Collider* currentCollider = &colliders.Front();
      currentCollider->mState.SetFlag(ColliderFlags::OnIsland);
      stack.PushBack(currentCollider);
    }
  }
}

// When joints are invalid don't destroy them
void DestroyJoint(Joint* joint)
{
}

// Contacts should be destroyed when they are invalid (so that we get collision end)
void DestroyJoint(Contact* contact)
{
  contact->Destroy();
}

template <typename EdgeListType>
void AddCompositeEdges(EdgeListType& edgeList, Island* island, ColliderStack& stack)
{
  typename EdgeListType::range edgeRange = edgeList.All();

  while(!edgeRange.Empty())
  {
    typename EdgeListType::value_type& edge = edgeRange.Front();
    edgeRange.PopFront();

    if(edge.mJoint->GetOnIsland())
      continue;

    // If the joint isn't valid for some reason (one of the colliders/cogs is null)
    // then don't solve or traverse this edge
    if(!edge.mJoint->GetValid())
    {
      DestroyJoint(edge.mJoint);
      continue;
    }

    island->Add(edge.mJoint);
    Collider* otherCollider = edge.mOther;

    //don't add the other object if it already exists on this island
    if(otherCollider->mState.IsSet(ColliderFlags::OnIsland))
      continue;

    //don't add static objects to the island since islands don't extend over static objects
    if(otherCollider->GetActiveBody() != nullptr)
    {
      AddTreeToStack(otherCollider, stack);
    }
  }
}

void AddCompositeColliders(Collider* collider, ColliderStack& stack)
{
  //Push on other colliders in the composite object
  RigidBody* body = collider->GetActiveBody();
  if(body == nullptr)
    return;
  RigidBody::CompositeColliderList::range colliderRange = body->mColliders.All();
  for(; !colliderRange.Empty(); colliderRange.PopFront())
  {
    if(colliderRange.Front().mState.IsSet(ColliderFlags::OnIsland))
      continue;

    colliderRange.Front().mState.SetFlag(ColliderFlags::OnIsland);
    stack.PushBack(&colliderRange.Front());
  }
}

///Extends islands across kinematic objects so that a kinematic object can be on an island.
///Used when objects want to get everything in their island.
struct KinematicTraversal
{
  bool ValidCollider(Collider* collider)
  {
    //if this one is already on an island, it has already been taken
    //care of so ignore it.
    if(collider->mState.IsSet(ColliderFlags::OnIsland) || collider->IsAsleep())
    {
      //Objects have to be given an initialized flag so that they can get
      //contacts added even though they are asleep. This is because if the
      //contacts aren't added, then an island will never be formed, so objects
      //will not wake up if something under them wakes up.
      collider->mState.ClearFlag(ColliderFlags::Uninitialized);
      return false;
    }

    if(collider->mContactEdges.Empty() && collider->mJointEdges.Empty())
      return false;
    return true;
  }

  void TraverseEdges(Collider* collider, ColliderStack& stack, Island* island)
  {
    RigidBody* body = collider->GetActiveBody();
    //Remove for now (clean-up later).
    //If this is in here then objects will not properly be woken up (timer) later.
    //if(body)
    //  body->mState.ClearFlag(RigidBodyStates::Asleep);

    //don't extend the island over static or asleep objects
    if(collider->IsStatic())
      return;

    AddCompositeEdges(collider->mJointEdges, island, stack);
    AddCompositeEdges(collider->mContactEdges, island, stack);

    AddTreeToStack(collider, stack);
  }
};

struct CompositeTraversal
{
  bool ValidCollider(Collider* collider)
  {
    //if this one is already on an island, it has already been taken
    //care of so ignore it.
    if(collider->mState.IsSet(ColliderFlags::OnIsland) || collider->IsAsleep())
      return false;

    if(collider->mContactEdges.Empty() && collider->mJointEdges.Empty())
      return true;
    return true;
  }

  void TraverseEdges(Collider* collider, ColliderStack& stack, Island* island)
  {
    collider->GetActiveBody()->mState.ClearFlag(RigidBodyStates::Asleep);

    //don't extend the island over static or asleep objects
    if(collider->GetActiveBody() == nullptr || !collider->GetActiveBody()->IsDynamic())
      return;

    AddCompositeEdges(collider->mJointEdges, island, stack);
    AddCompositeEdges(collider->mContactEdges, island, stack);

    AddTreeToStack(collider, stack);
  }
};

struct NoPreProcessing
{
  void PreProcess(IslandManager::IslandList& islands, Island*& newIsland)
  {
    islands.PushBack(newIsland);
    newIsland = nullptr;
  }
};

struct BasicPreProcessing
{
  void PreProcess(IslandManager::IslandList& islands, Island*& newIsland)
  {
    if(!islands.Empty())
    {
      Physics::Island* mergeIsland = &(islands.Back());
      if(mergeIsland->ColliderCount + newIsland->ColliderCount < 20)
      {
        mergeIsland->MergeIsland(*newIsland);
        newIsland->Clear();
      }
      else
      {
        islands.PushBack(newIsland);
        newIsland = nullptr;
      }
    }
    else
    {
      islands.PushBack(newIsland);
      newIsland = nullptr;
    }
  }
};

struct ConstraintCountPreProcessing
{
  void PreProcess(IslandManager::IslandList& islands, Island*& newIsland)
  {
    if(!islands.Empty())
    {
      Physics::Island* mergeIsland = &(islands.Back());
      uint mergeCount = mergeIsland->ContactCount + mergeIsland->JointCount;
      uint newCount = newIsland->ContactCount + newIsland->JointCount;
      if(mergeCount + newCount < 20)
      {
        mergeIsland->MergeIsland(*newIsland);
        newIsland->Clear();
      }
      else
      {
        islands.PushBack(newIsland);
        newIsland = nullptr;
      }
    }
    else
    {
      islands.PushBack(newIsland);
      newIsland = nullptr;
    }
  }
};

IslandManager::IslandManager(PhysicsSolverConfig* config)
{
  mIslandCount = 0;
  mIslandingType = PhysicsIslandType::Kinematics;
  mPreProcessingType = PhysicsIslandPreProcessingMode::None;
  mPhysicsSolverConfig = config;
  mSpace = nullptr;
  mPostProcess = false;
  mSharedSolver = nullptr;
  mShareSolver = false;
}

IslandManager::~IslandManager()
{
  Clear();
}

void IslandManager::SetSpace(PhysicsSpace* space)
{
  mSpace = space;
}

void IslandManager::SetSolverConfig(PhysicsSolverConfig* config)
{
  mPhysicsSolverConfig = config;

  //make sure to set all of the solver's up with the new config
  //(especially since we don't store resource references to them)
  if(mSharedSolver != nullptr)
    mSharedSolver->SetConfiguration(mPhysicsSolverConfig);

  IslandList::range range = mIslands.All();
  for(; !range.Empty(); range.PopFront())
  {
    Island& island = range.Front();
    island.mSolver->SetConfiguration(mPhysicsSolverConfig);
  }
}

void IslandManager::BuildIslands(ColliderList& colliders)
{
  Clear();

  if(mShareSolver)
    mSharedSolver = GetNewSolver();

  if(mIslandingType == PhysicsIslandType::ForcedOne)
    CreateSingleIsland(KinematicTraversal(), colliders);
  else if(mIslandingType == PhysicsIslandType::Kinematics)
    CreateCompactIslands(KinematicTraversal(), colliders);
  else if(mIslandingType == PhysicsIslandType::Composites)
    CreateCompactIslands(CompositeTraversal(), colliders);
  else
    ErrorIf(true, "Invalid islanding type specified.");

  if(mPostProcess)
    PostProcessIslands();

  mIslandCount = 0;
  IslandList::range range = mIslands.All();
  for(; !range.Empty(); range.PopFront())
    ++mIslandCount;
}

void IslandManager::PostProcessIslands()
{
  if(mIslands.Empty())
    return;

  uint ColliderMergeThreshold = 20;

  IslandList largeEnoughIslands;
  Island* mergeIsland = &(mIslands.Front());
  mIslands.Unlink(mergeIsland);
  if(mergeIsland->ColliderCount >= ColliderMergeThreshold)
  {
    largeEnoughIslands.PushBack(mergeIsland);
    mergeIsland = nullptr;
  }

  while(!mIslands.Empty())
  {
    Island* island = &(mIslands.Front());
    mIslands.Unlink(island);

    if(mergeIsland == nullptr)
    {
      mergeIsland = island;
      continue;
    }

    if(island->ColliderCount >= ColliderMergeThreshold)
    {
      largeEnoughIslands.PushBack(island);
      continue;
    }

    if(island->ColliderCount + mergeIsland->ColliderCount >= ColliderMergeThreshold)
    {
      largeEnoughIslands.PushBack(mergeIsland);
      mergeIsland = island;
      continue;
    }

    mergeIsland->MergeIsland(*island);
    delete island;
  }

  if(mergeIsland)
    largeEnoughIslands.PushBack(mergeIsland);
  mIslands.Swap(largeEnoughIslands);
}

void IslandManager::Solve(real dt, bool allowSleeping, uint debugFlags)
{
  if(mShareSolver)
  {
    IslandList::range islandRange = mIslands.All();
    for(; !islandRange.Empty(); islandRange.PopFront())
      islandRange.Front().CommitConstraints();

    mSharedSolver->Solve(dt);

    //solve all of the islands.
    islandRange = mIslands.All();
    for(; !islandRange.Empty(); islandRange.PopFront())
      islandRange.Front().UpdateSleep(dt, allowSleeping, debugFlags);

    return;
  }

  //solve all of the islands.
  IslandList::range islandRange = mIslands.All();
  for(; !islandRange.Empty(); islandRange.PopFront())
    islandRange.Front().Solve(dt, allowSleeping, debugFlags);
}

void IslandManager::SolvePositions(real dt)
{
  IslandList::range islandRange = mIslands.All();
  for(; !islandRange.Empty(); islandRange.PopFront())
    islandRange.Front().SolvePositions(dt);
}

void IslandManager::Draw(uint flags)
{
  IslandList::range range = mIslands.All();
  for(; !range.Empty(); range.PopFront())
  {
    Island& island = range.Front();
    island.mSolver->DebugDraw(flags);
  }
}

void IslandManager::RemoveCollider(Collider* collider)
{
  //have the collider unlink itself if it is on an island
  if(collider->mState.IsSet(ColliderFlags::OnIsland))
  {
    //remove the constraints from the solver but don't unlink them from the colliders
    //the colliders remove them in the destructor. Otherwise objects being moved by their
    //transform being set will have their constraints undone
    Physics::JointHelpers::UnlinkJointsFromSolver(collider);
    Physics::Island::Colliders::Unlink(collider);
    collider->mState.ClearFlag(ColliderFlags::OnIsland);
  }
}

void IslandManager::Clear()
{
  mIslandCount = 0;

  DeleteObjectsIn<Island, &Island::ManagerLink>(mIslands);
  if(mShareSolver && mSharedSolver != nullptr)
  {
    mSharedSolver->Clear();
    delete mSharedSolver;
  }
}

Island* IslandManager::GetObjectsIsland(const Collider* collider)
{
  IslandList::range islandRange = mIslands.All();
  for(; !islandRange.Empty(); islandRange.PopFront())
  {
    Island* island = &islandRange.Front();
    if(island->ContainsCollider(collider))
      return island;
  }

  return nullptr;
}

template <typename Policy> 
void IslandManager::CreateCompactIslands(Policy policy, ColliderList& colliders)
{
  if(mPreProcessingType == PhysicsIslandPreProcessingMode::None)
    CreateCompactIslands(policy, NoPreProcessing(), colliders);
  else if(mPreProcessingType == PhysicsIslandPreProcessingMode::ColliderCount)
    CreateCompactIslands(policy, BasicPreProcessing(), colliders);
  else if(mPreProcessingType == PhysicsIslandPreProcessingMode::ConstraintCount)
    CreateCompactIslands(policy, ConstraintCountPreProcessing(), colliders);
  else 
    ErrorIf(true,"Invalid Pre-Processing type specified.");
}

template <typename Policy, typename PreProcessing>
void IslandManager::CreateCompactIslands(Policy policy, PreProcessing prePolicy, ColliderList& colliders)
{
  ColliderStack stack;
  stack.SetAllocator(HeapAllocator(mSpace->mHeap));

  Physics::Island* island = nullptr;

  ColliderList::range colliderRange = colliders.All();
  //loop over all of the colliders
  for(; !colliderRange.Empty(); colliderRange.PopFront())
  {
    Collider& obj = colliderRange.Front();
    if(!policy.ValidCollider(&obj))
      continue;

    AddTreeToStack(&obj, stack);

    if(island == nullptr)
      island = CreateNewIsland();
    obj.mState.SetFlag(ColliderFlags::OnIsland);

    //the stack represents the objects in this island that have
    //yet to be visited, so we have to iterate until the stack is empty.
    while(!stack.Empty())
    {
      Collider* collider = stack.Back();
      stack.PopBack();
      island->Add(collider);

      policy.TraverseEdges(collider, stack, island);
    }

    prePolicy.PreProcess(mIslands, island);
  }

  if(island != nullptr)
    delete island;
}

template <typename Policy>
void IslandManager::CreateSingleIsland(Policy policy, ColliderList& colliders)
{
  Physics::Island* island = CreateNewIsland();

  ColliderStack stack;
  stack.SetAllocator(HeapAllocator(mSpace->mHeap));
  

  ColliderList::range colliderRange = colliders.All();
  //loop over all of the colliders
  for(; !colliderRange.Empty(); colliderRange.PopFront())
  {
    Collider& obj = colliderRange.Front();
    if(!policy.ValidCollider(&obj))
      continue;

    stack.PushBack(&obj);
    obj.mState.SetFlag(ColliderFlags::OnIsland);

    //the stack is all of the connections that need to be spanned for an island
    while(!stack.Empty())
    {
      Collider* collider = stack.Back();
      stack.PopBack();
      island->Add(collider);

      policy.TraverseEdges(collider, stack, island);
    }
  }

  mIslands.PushBack(island);
}

IConstraintSolver* IslandManager::GetNewSolver()
{
  IConstraintSolver* solver = nullptr;
  if(mPhysicsSolverConfig->mSolverType == PhysicsSolverType::Basic)
    solver = new BasicSolver();
  else if(mPhysicsSolverConfig->mSolverType == PhysicsSolverType::GenericBasic)
    solver = new GenericBasicSolver();
  else if(mPhysicsSolverConfig->mSolverType == PhysicsSolverType::Normal)
    solver = new NormalSolver();
  else if(mPhysicsSolverConfig->mSolverType == PhysicsSolverType::Threaded)
    solver = new ThreadedSolver();
  else
    ErrorIf(true,"Invalid Solver type specified.");

  if(mPhysicsSolverConfig != nullptr)
    solver->SetConfiguration(mPhysicsSolverConfig);
  solver->SetHeap(mSpace->mHeap);

  return solver;
}

Island* IslandManager::CreateNewIsland()
{
  Island* island = new Island();

  if(mShareSolver)
    island->SetSolver(mSharedSolver, false);
  else
    island->SetSolver(GetNewSolver(), true);

  return island;
}

}//namespace Physics

}//namespace Zero
