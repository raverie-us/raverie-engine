///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Tags
{
  DefineTag(Joint);
}

ZilchDefineType(Joint, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();

  ZeroBindDependency(Cog);
  ZeroBindDependency(ObjectLink);
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindGetterSetterProperty(Active);
  ZilchBindGetterSetterProperty(SendsEvents);
  ZilchBindGetterSetterProperty(AutoSnaps);
  ZilchBindGetterSetterProperty(CollideConnected);
  ZilchBindGetterSetterProperty(MaxImpulse);
  ZilchBindMethod(GetOtherObject);
  ZilchBindMethod(GetCog);
  ZeroBindEvent(Events::JointExceedImpulseLimit, JointEvent);

  ZeroBindTag(Tags::Physics);
  ZeroBindTag(Tags::Joint);
}

Joint::Joint()
{
  mSolver = nullptr;
  mNode = new JointNode();
  mNode->mJoint = this;

  mFlags.Clear();
}

Joint::~Joint()
{ 
  delete mNode;
}

void Joint::Serialize(Serializer& stream)
{
  uint mask = JointFlags::OnIsland | JointFlags::Ghost | JointFlags::Valid | JointFlags::Initialized;
  SerializeBits(stream, mFlags, JointFlags::Names, mask, JointFlags::CollideConnected | JointFlags::Active | JointFlags::SendsEvents);
  real MaxImpulse;
  if(stream.GetMode() == SerializerMode::Loading)
  {
    SerializeNameDefault(MaxImpulse, Math::PositiveMax());
    SetMaxImpulse(MaxImpulse);
  }
  else
  {
    MaxImpulse = GetMaxImpulse();
    SerializeNameDefault(MaxImpulse, Math::PositiveMax());
  }
}

void Joint::Initialize(CogInitializer& initializer)
{
  Space* space = initializer.GetSpace();
  mSpace = space->has(PhysicsSpace);
  ErrorIf(!mSpace, "Joint's space was invalid.");
}

void Joint::OnAllObjectsCreated(CogInitializer& initializer)
{
  // Only do initialization once (can be called again from pulley or gear joint)
  if(mFlags.IsSet(JointFlags::Initialized) == true)
    return;
  
  mFlags.SetFlag(JointFlags::Initialized);
  ConnectThisTo(GetOwner(), Events::ObjectLinkChanged, OnObjectLinkChanged);
  ConnectThisTo(GetOwner(), Events::ObjectLinkPointChanged, OnObjectLinkPointChanged);

  // Always add to the space. This makes it easier to deal with partially invalid
  // joints being destroyed (and the space list is only there for easy iteration of joints)
  mSpace->AddComponent(this);

  // Link up the collider edges to the object link's data.
  LinkPair();
  // Also wake up the objects we're connected to so that newly created joints will work properly.
  // @JoshD: Should this only happen when during not object startup so that saved asleep objects stay asleep?
  Physics::JointHelpers::ForceAwakeJoint(this);

  // We were dynamically created so try to compute some logical initial values
  bool dynamicallyCreated = (initializer.Flags & CreationFlags::DynamicallyAdded) != 0;
  if(dynamicallyCreated)
    ComputeInitialConfiguration();
}

void Joint::OnDestroy(uint flags)
{
  // Always remove from the space, even if we weren't valid (because we're always added)
  mSpace->RemoveComponent(this);

  // If we were in a partially or completely invalid state then one of our colliders
  // doesn't exist. If one of them was valid we could wake it up, but there's not
  // really a point as this joint wasn't doing anything. Since it wasn't doing anything
  // removing the joint shouldn't cause any change to the dynamics of a body and therefore
  // waking it up isn't necessary. Same if we weren't active.
  if(GetValid() && GetActive())
  {
    mEdges[0].mCollider->ForceAwake();
    mEdges[1].mCollider->ForceAwake();
  }
  // Unlink from the colliders we were connected to. This also marks the joint
  // as not valid just in case any other calls happen that would rely on being connected.
  UnLinkPair();
}

void Joint::ComponentAdded(BoundType* typeId, Component* component)
{
  if(typeId == ZilchTypeId(JointLimit))
  {
    JointLimit* limit = static_cast<JointLimit*>(component);
    limit->mAtomIds = GetDefaultLimitIds();
    Physics::JointHelpers::ForceAwakeJoint(this);
  }
  else if(typeId == ZilchTypeId(JointMotor))
  {
    JointMotor* motor = static_cast<JointMotor*>(component);
    motor->mAtomIds = GetDefaultMotorIds();
    Physics::JointHelpers::ForceAwakeJoint(this);
  }
  else if(typeId == ZilchTypeId(JointSpring))
  {
    JointSpring* spring = static_cast<JointSpring*>(component);
    spring->mAtomIds = GetDefaultSpringIds();
    Physics::JointHelpers::ForceAwakeJoint(this);
  }
}

void Joint::ComponentRemoved(BoundType* typeId, Component* component)
{
  if(typeId == ZilchTypeId(JointLimit))
    Physics::JointHelpers::ForceAwakeJoint(this);
  else if(typeId == ZilchTypeId(JointMotor))
    Physics::JointHelpers::ForceAwakeJoint(this);
  else if(typeId == ZilchTypeId(JointSpring))
    Physics::JointHelpers::ForceAwakeJoint(this);
}

uint Joint::GetAtomIndexFilterVirtual(uint atomIndex, real& desiredConstraintValue) const 
{ 
  desiredConstraintValue = 0;
  return 0;
}

void Joint::SetPair(Physics::ColliderPair& pair)
{
  mEdges[0].mCollider = pair.mObjects[0];
  mEdges[0].mOther = pair.mObjects[1];
  mEdges[0].mJoint = this;

  mEdges[1].mCollider = pair.mObjects[1];
  mEdges[1].mOther = pair.mObjects[0];
  mEdges[1].mJoint = this;

  // We put not CollideConnected at the front so we can do quick filtering
  if(GetCollideConnected())
  {
    if(pair.mObjects[0] != nullptr)
      pair.mObjects[0]->mJointEdges.PushBack(&mEdges[0]);
    if(pair.mObjects[1] != nullptr)
      pair.mObjects[1]->mJointEdges.PushBack(&mEdges[1]);
  }
  else
  {
    if(pair.mObjects[0] != nullptr)
      pair.mObjects[0]->mJointEdges.PushFront(&mEdges[0]);
    if(pair.mObjects[1] != nullptr)
      pair.mObjects[1]->mJointEdges.PushFront(&mEdges[1]);
  }

  // If either collider didn't exist, then don't become valid
  if(pair.mObjects[0] == nullptr || pair.mObjects[1] == nullptr)
    return;

  SetValid(true);
}

void Joint::LinkPair()
{
  ObjectLink* link = GetOwner()->has(ObjectLink);

  Cog* objectA = link->GetObjectA();
  Cog* objectB = link->GetObjectB();
  Collider* collider0 = nullptr;
  Collider* collider1 = nullptr;

  // Try to find the collider from each object
  if(objectA != nullptr)
    collider0 = objectA->has(Collider);
  if(objectB != nullptr)
    collider1 = objectB->has(Collider);

  // If we failed to get either collider then set this joint as not currently being valid (we can't solve)
  if(collider0 == nullptr || collider1 == nullptr)
    SetValid(false);

  // We do have to set the pair properly so that edges and whatnot can be properly traversed and unlinked
  ColliderPair pair;
  pair.mObjects[0] = collider0;
  pair.mObjects[1] = collider1;
  SetPair(pair);
}

void Joint::UnLinkPair()
{
  typedef InList<Joint, &Joint::SolverLink> JointList;

  // Only remove from the island and solver if we're on an island.
  if(GetOnIsland())
  {
    JointList::Unlink(this);
  }

  // Unlink from both colliders
  for(size_t i = 0; i < 2; ++i)
  {
    if(mEdges[i].mCollider != nullptr)
    {
      Collider::JointEdgeList::Unlink(&mEdges[i]);
      mEdges[i].mCollider = nullptr;
    }
  }

  // We aren't linked to two valid colliders so mark ourself as !Valid
  SetValid(false);
}

void Joint::Relink(uint index, Cog* cog)
{
  // Get the collider from the input cog
  Collider* collider = nullptr;
  if(cog != nullptr)
    collider = cog->has(Collider);

  // Remove ourself from the island we were on (if we were on one)
  typedef InList<Joint, &Joint::SolverLink> JointList;
  if(GetOnIsland())
  {
    SetOnIsland(false);
    JointList::Unlink(this);
  }

  EdgeType& mainEdge = mEdges[index];
  EdgeType& otherEdge = mEdges[(index + 1) % 2];

  // Unlink the old edge but only if that edge was to a
  // valid collider (aka the edge hasn't been cleared already).
  if(mainEdge.mJoint != nullptr && mainEdge.mCollider != nullptr)
    Collider::JointEdgeList::Unlink(&mainEdge);

  // Fix the colliders on the edges and add this edge to the new collider
  mainEdge.mJoint = this;
  mainEdge.mCollider = collider;
  otherEdge.mOther = collider;
  // If we have a collider then link its edge up
  if(collider != nullptr)
    collider->mJointEdges.PushBack(&mainEdge);

  // If we were in a completely invalid state before being setup and now we're
  // in a valid state we need to update valid (but not active, that should only ever be changed by the user)
  bool isValid = (mainEdge.mCollider != nullptr && otherEdge.mCollider != nullptr);
  SetValid(isValid);

  // Finally, let joints know that we relinked so any specific joint type
  // can respond (pulleys and gears need to hook-up some other pointers)
  if(isValid)
    SpecificJointRelink(index, collider);
}

void Joint::OnObjectLinkChanged(ObjectLinkEvent* event)
{
  Relink(event->EdgeId, event->NewCog);
}

void Joint::OnObjectLinkPointChanged(ObjectLinkPointChangedEvent* e)
{
  ObjectLinkPointUpdated(e->mEdgeId, e->mNewLocalPoint);
}

uint Joint::GetActiveFilter() const
{
  return mConstraintFilter & FilterFlag;
}

uint Joint::GetDefaultFilter() const
{
  return (mConstraintFilter >> DefaultOffset) & FilterFlag;
}

void Joint::ResetFilter()
{
  mConstraintFilter &= ~FilterFlag;
  mConstraintFilter |= (mConstraintFilter >> DefaultOffset) & FilterFlag;
}

Collider* Joint::GetCollider(uint index) const
{
  return mEdges[index].mCollider;
}

Cog* Joint::GetCog(uint index)
{
  Collider* collider = mEdges[index].mCollider;
  if(collider != nullptr)
    return collider->GetOwner();
  return nullptr;
}

Cog* Joint::GetOtherObject(Cog* cog)
{
  Cog* cog0 = GetCog(0);
  Cog* cog1 = GetCog(1);

  if(cog0 == cog)
    return cog1;
  else if(cog1 == cog)
    return cog0;
  return nullptr;
}

Vec3 Joint::GetLocalPointHelper(const Physics::AnchorAtom& anchor, uint index) const
{
  return anchor[index];
}

void Joint::SetLocalPointHelper(Physics::AnchorAtom& anchor, uint index, Vec3Param localPoint)
{
  anchor[index] = localPoint;
  Physics::JointHelpers::ForceAwakeJoint(this);
  ObjectLink* objectLink = GetOwner()->has(ObjectLink);
  ErrorIf(objectLink == nullptr, "Joint is missing object link");
  objectLink->SetLocalPoint(localPoint, (ObjectLink::ObjectIndex)index);
}

Vec3 Joint::GetWorldPointHelper(const Physics::AnchorAtom& anchor, uint index)
{
  Collider* collider = GetCollider(index);
  if(collider == nullptr)
    return Vec3::cZero;

  return Physics::JointHelpers::BodyRToWorldPoint(collider, anchor[index]);
}

void Joint::SetWorldPointAHelper(Physics::AnchorAtom& anchor, Vec3Param worldPoint)
{
  // Register side-effect properties
  if(OperationQueue::IsListeningForSideEffects())
    OperationQueue::RegisterSideEffect(this, "LocalPointA", anchor[0]);

  SetWorldPointHelper(anchor, worldPoint, 0);
}

void Joint::SetWorldPointBHelper(Physics::AnchorAtom& anchor, Vec3Param worldPoint)
{
  // Register side-effect properties
  if(OperationQueue::IsListeningForSideEffects())
    OperationQueue::RegisterSideEffect(this, "LocalPointB", anchor[1]);

  SetWorldPointHelper(anchor, worldPoint, 1);
}

void Joint::SetWorldPointHelper(Physics::AnchorAtom& anchor, Vec3Param worldPoint, uint index)
{
  Collider* collider = GetCollider(index);
  if(collider == nullptr)
    return;

  anchor[index] = Physics::JointHelpers::WorldPointToBodyR(collider, worldPoint);
  Physics::JointHelpers::ForceAwakeJoint(this);
  ObjectLink* objectLink = GetOwner()->has(ObjectLink);
  ErrorIf(objectLink == nullptr, "Joint is missing object link");
  objectLink->SetLocalPoint(anchor[index], (ObjectLink::ObjectIndex)index);
}

void Joint::SetWorldPointsHelper(Physics::AnchorAtom& anchor, Vec3Param point)
{
  // Register side-effect properties
  if(OperationQueue::IsListeningForSideEffects())
  {
    OperationQueue::RegisterSideEffect(this, "LocalPointA", anchor[0]);
    OperationQueue::RegisterSideEffect(this, "LocalPointB", anchor[1]);
  }

  Collider* collider0 = GetCollider(0);
  Collider* collider1 = GetCollider(1);
  if(collider0 == nullptr || collider1 == nullptr)
    return;

  ObjectLink* objectLink = GetOwner()->has(ObjectLink);
  ErrorIf(objectLink == nullptr, "Joint is missing object link");

  anchor[0] = Physics::JointHelpers::WorldPointToBodyR(collider0, point);
  anchor[1] = Physics::JointHelpers::WorldPointToBodyR(collider1, point);
  objectLink->SetLocalPointA(anchor[0]);
  objectLink->SetLocalPointB(anchor[1]);
  Physics::JointHelpers::ForceAwakeJoint(this);
}

void Joint::ObjectLinkPointUpdatedHelper(Physics::AnchorAtom& anchor, size_t edgeIndex, Vec3Param localPoint)
{
  anchor[edgeIndex] = localPoint;
  Physics::JointHelpers::ForceAwakeJoint(this);
}

Vec3 Joint::GetLocalAxisHelper(const Physics::AxisAtom& axisAtom, uint index) const
{
  return axisAtom[index];
}

void Joint::SetLocalAxisHelper(Physics::AxisAtom& axisAtom, uint index, Vec3Param localAxis)
{
  if(localAxis == Vec3::cZero)
    return;

  axisAtom[index] = localAxis;
  Physics::JointHelpers::ForceAwakeJoint(this);
}

Vec3 Joint::GetWorldAxisHelper(const Physics::AxisAtom& axisAtom) const
{
  // There's no real logical world-space axis if the local space vectors don't
  // map to the same value. Just use the local axis from object 0.
  Collider* collider0 = GetCollider(0);
  if(collider0 == nullptr)
    return Vec3::cZero;

  return Physics::JointHelpers::BodyToWorldR(collider0, axisAtom[0]);
}

void Joint::SetWorldAxisHelper(Physics::AxisAtom& axisAtom, Vec3Param worldAxis)
{
  // Register side-effect properties
  if(OperationQueue::IsListeningForSideEffects())
  {
    OperationQueue::RegisterSideEffect(this, "LocalAxisA", axisAtom[0]);
    OperationQueue::RegisterSideEffect(this, "LocalAxisB", axisAtom[1]);
  }

  Vec3 fixedAxis = worldAxis;
  if(fixedAxis == Vec3::cZero)
    return;

  Collider* collider0 = GetCollider(0);
  Collider* collider1 = GetCollider(1);
  if(collider0 == nullptr || collider1 == nullptr)
    return;

  axisAtom[0] = Physics::JointHelpers::WorldToBodyR(collider0, fixedAxis);
  axisAtom[1] = Physics::JointHelpers::WorldToBodyR(collider1, fixedAxis);
  Physics::JointHelpers::ForceAwakeJoint(this);
}

Quat Joint::GetLocalAngleHelper(const Physics::AngleAtom& angleAtom, uint index) const
{
  return angleAtom[index];
}

void Joint::SetLocalAngleHelper(Physics::AngleAtom& angleAtom, uint index, QuatParam localReferenceFrame)
{
  angleAtom[index] = localReferenceFrame;
  Physics::JointHelpers::ForceAwakeJoint(this);
}

bool Joint::GetShouldBaumgarteBeUsed(uint type) const
{
  PhysicsSolverConfig* config = mSolver->mSolverConfig;
  ConstraintConfigBlock& block = config->mJointBlocks[type];
  JointConfigOverride* configOverride = mNode->mConfigOverride;

  // Check for the config override component and use its override if it has one.
  if(configOverride != nullptr)
  {
    if(configOverride->mPositionCorrectionType == ConstraintPositionCorrection::PostStabilization)
      return false;

    if(configOverride->mPositionCorrectionType == ConstraintPositionCorrection::Baumgarte)
      return true;
  }

  // Check the block type for the given joint. If it specifies one correction type then use that.
  if(block.GetPositionCorrectionType() == ConstraintPositionCorrection::PostStabilization)
    return false;
  if(block.GetPositionCorrectionType() == ConstraintPositionCorrection::Baumgarte)
    return true;

  // Otherwise check the global state.
  if(config->mPositionCorrectionType == PhysicsSolverPositionCorrection::PostStabilization)
    return false;

  return true;
}

real Joint::GetLinearBaumgarte(uint type) const
{
  // The baumgarte term is always returned even if we aren't using baumgarte.
  // This is because a joint could have a spring on it, in which case we ignore the
  // position correction mode and always use baumgarte. Therefore, the code that calls
  // this should determine whether or not to apply the baumgarte
  // using GetShouldBaumgarteBeUsed (Same for angular baumgarte)
  PhysicsSolverConfig* config = mSolver->mSolverConfig;
  ConstraintConfigBlock& block = config->mJointBlocks[type];
  JointConfigOverride* configOverride = mNode->mConfigOverride;

  if(configOverride != nullptr)
    return configOverride->mLinearBaumgarte;

  return block.mLinearBaumgarte;
}

real Joint::GetAngularBaumgarte(uint type) const
{
  // See the comment at the top of GetLinearBaumgarte for why we always return the baumgarte value.
  PhysicsSolverConfig* config = mSolver->mSolverConfig;
  ConstraintConfigBlock& block = config->mJointBlocks[type];
  JointConfigOverride* configOverride = mNode->mConfigOverride;

  if(configOverride != nullptr)
    return configOverride->mAngularBaumgarte;

  return block.mAngularBaumgarte;
}

real Joint::GetLinearErrorCorrection(uint type) const
{
  PhysicsSolverConfig* config = mSolver->mSolverConfig;
  ConstraintConfigBlock& block = config->mJointBlocks[type];
  JointConfigOverride* configOverride = mNode->mConfigOverride;

  if(configOverride != nullptr)
    return configOverride->mLinearErrorCorrection;

  return block.mLinearErrorCorrection;
}

real Joint::GetLinearErrorCorrection() const
{
  return GetLinearErrorCorrection(GetJointType());
}

real Joint::GetAngularErrorCorrection(uint type) const
{
  PhysicsSolverConfig* config = mSolver->mSolverConfig;
  ConstraintConfigBlock& block = config->mJointBlocks[type];
  JointConfigOverride* configOverride = mNode->mConfigOverride;

  if(configOverride != nullptr)
    return configOverride->mAngularErrorCorrection;

  return block.mAngularErrorCorrection;
}

real Joint::GetAngularErrorCorrection() const
{
  return GetAngularErrorCorrection(GetJointType());
}

real Joint::GetSlop() const
{
  PhysicsSolverConfig* config = mSolver->mSolverConfig;
  ConstraintConfigBlock& block = config->mJointBlocks[GetJointType()];
  JointConfigOverride* configOverride = mNode->mConfigOverride;

  if(configOverride != nullptr)
    return configOverride->mSlop;

  return block.mSlop;
}

void Joint::UpdateColliderCachedTransforms()
{
  Collider* collider0 = GetCollider(0);
  if(collider0 != nullptr)
    collider0->UpdateQueue();

  Collider* collider1 = GetCollider(1);
  if(collider1 != nullptr)
    collider1->UpdateQueue();
}

void Joint::ComputeCurrentAnchors(Physics::AnchorAtom& anchors)
{
  // Just grab the body points from the object link
  ObjectLink* link = GetOwner()->has(ObjectLink);
  anchors[0] = link->GetLocalPointA();
  anchors[1] = link->GetLocalPointB();
}

void Joint::ComputeCurrentReferenceAngle(Physics::AngleAtom& referenceAngle)
{
  ObjectLink* link = GetOwner()->has(ObjectLink);
  Cog* cogs[2];
  cogs[0] = link->GetObjectA();
  cogs[1] = link->GetObjectB();

  // Compute the relative angles to make the objects maintain their current
  // rotations. This is done by computing the orientation that will align
  // the object with the identity (aka the inverse).
  for(uint i = 0; i < 2; ++i)
  {
    Cog* cog = cogs[i];
    if(cog != nullptr)
    {
      Transform* t = cog->has(Transform);
      if(t != nullptr)
        referenceAngle[i] = t->GetWorldRotation().Inverted();
    }
  }
}

bool Joint::GetOnIsland() const
{
  return mFlags.IsSet(JointFlags::OnIsland);
}

void Joint::SetOnIsland(bool onIsland)
{
  mFlags.SetState(JointFlags::OnIsland, onIsland);
}

bool Joint::GetGhost() const
{
  return mFlags.IsSet(JointFlags::Ghost);
}

void Joint::SetGhost(bool ghost)
{
  mFlags.SetState(JointFlags::Ghost, ghost);
}

bool Joint::GetValid() const
{
  return mFlags.IsSet(JointFlags::Valid);
}

void Joint::SetValid(bool valid)
{
  mFlags.SetState(JointFlags::Valid, valid);
}

bool Joint::GetActive() const
{
  return mFlags.IsSet(JointFlags::Active);
}

void Joint::SetActive(bool active)
{
  mFlags.SetState(JointFlags::Active, active);
  Physics::JointHelpers::ForceAwakeJoint(this);
}

bool Joint::GetSendsEvents() const
{
  return mFlags.IsSet(JointFlags::SendsEvents);
}

void Joint::SetSendsEvents(bool sendsEvents)
{
  mFlags.SetState(JointFlags::SendsEvents, sendsEvents);
}

bool Joint::GetAutoSnaps() const
{
  return mFlags.IsSet(JointFlags::AutoSnaps);
}

void Joint::SetAutoSnaps(bool autoSnaps)
{
  mFlags.SetState(JointFlags::AutoSnaps, autoSnaps);
}

bool Joint::GetCollideConnected() const
{
  return mFlags.IsSet(JointFlags::CollideConnected);
}

void Joint::SetCollideConnected(bool collideConnected)
{
  mFlags.SetState(JointFlags::CollideConnected, collideConnected);
  
  if(!GetValid())
    return;

  Collider::JointEdgeList::Unlink(&mEdges[0]);
  Collider::JointEdgeList::Unlink(&mEdges[1]);
  // By putting not collide connected at the front of each collider's list
  // we can only check for not CollideConnected objects and stop once we reach a
  // CollideConnected for quick rejection in ShouldCollide
  if(collideConnected)
  {
    mEdges[0].mCollider->mJointEdges.PushBack(&mEdges[0]);
    mEdges[1].mCollider->mJointEdges.PushBack(&mEdges[1]);
  }
  else
  {
    mEdges[0].mCollider->mJointEdges.PushFront(&mEdges[0]);
    mEdges[1].mCollider->mJointEdges.PushFront(&mEdges[1]);
  }
}

real Joint::GetMaxImpulse() const
{
  if(mMaxImpulse == Math::PositiveMax())
    return real(0.0);
  return mMaxImpulse;
}

void Joint::SetMaxImpulse(real maxImpulse)
{
  if(maxImpulse <= real(0.0))
    maxImpulse = Math::PositiveMax();
  mMaxImpulse = maxImpulse;
}

}//namespace Zero
