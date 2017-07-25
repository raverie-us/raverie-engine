///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Tags
{
  DefineTag(Collider);
}

// An ever incrementing id to give each collider a unique id.
static u32 colliderId = 0;

ZilchDefineType(Collider, builder, type)
{
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindTag(Tags::Physics);
  ZeroBindTag(Tags::Collider);
  ZeroBindDocumented();

  ZeroBindDependency(Cog);
  ZeroBindDependency(Transform);
  ZeroBindEvent(Events::CollisionStarted, CollisionEvent);
  ZeroBindEvent(Events::CollisionEnded, CollisionEvent);
  // Hide this for now because it's not that useful and needs renaming
  //BindSignal(Events::Collision, CollisionEvent);
  ZeroBindEvent(Events::CollisionPersisted, CollisionEvent);
  ZeroBindEvent(Events::GroupCollisionPersisted, CollisionGroupEvent);
  ZeroBindEvent(Events::GroupCollisionPreSolve, PreSolveEvent);

  ZilchBindGetterSetterProperty(Material);
  ZilchBindGetterSetterProperty(CollisionGroup);
  ZilchBindGetterSetterProperty(Ghost);
  ZilchBindGetterSetterProperty(SendsEvents);
  ZilchBindGetterSetterProperty(Offset);

  ZilchBindMethod(ComputeVolume);
  ZilchBindMethod(GetPointVelocity);
  
  ZilchBindGetter(ActiveBody);
  ZilchBindGetter(ContactCount);
  ZilchBindGetter(Contacts);
  ZilchBindGetter(JointCount);
  ZilchBindGetter(Joints);

  ZilchBindGetter(WorldAabb);
  ZilchBindGetter(WorldBoundingSphere);
}

Collider::Collider()
{
  mType = cInvalid;
  mContactCount = 0;
  mId = 0;
 
  mState.Clear();
  mState.SetFlag(ColliderFlags::Uninitialized);

  mTranslationOffset.ZeroOut();
  mCollisionOffset.ZeroOut();
  mAabb.SetCenterAndHalfExtents(Vec3::cZero, Vec3(real(0.5f)));

  mPhysicsNode = nullptr;
  mDirectRigidBody = nullptr;
  mActiveRigidBody = nullptr;
  mCollisionGroupInstance = nullptr;
  mSpace = nullptr;
}

void Collider::Serialize(Serializer& stream)
{
  /// There's quite a few flags that only store run-time state that we need to ignore when serializing.
  u32 mask = ColliderFlags::OnIsland | ColliderFlags::Uninitialized | 
             ColliderFlags::HasPairFilter | ColliderFlags::MasslessBody | ColliderFlags::MasslessCollider | ColliderFlags::Seamless;
  // The default state is to be not ghost
  SerializeBits(stream, mState, ColliderFlags::Names, mask, ~ColliderFlags::Ghost);


  SerializeResourceName(mMaterial, PhysicsMaterialManager);
  SerializeResourceImpl<CollisionGroupManager>("CollisionGroup", stream, mSerializedGroup);
  
  // Serialize the translation offset (previously named "Offset")
  stream.SerializeFieldDefault("Offset", mTranslationOffset, Vec3::cZero);

  // Now that we have our resource we can update one of our internal flags (the massless flag)
  UpdateMasslessColliderFlag();
}

void Collider::Initialize(CogInitializer& initializer)
{
  mId = GetNextColliderId();
  mSpace = initializer.mSpace->has(PhysicsSpace);
  ErrorIf(!mSpace, "Collider's CogInit did not contain a valid space.");

  mSpace->AddComponent(this);
  // Get the collision group instance from our space for the current serialized collision group
  mCollisionGroupInstance = mSpace->GetCollisionGroupInstance(mSerializedGroup->mResourceId);
  
  // Listen for when our physics material changes
  UpdatePhysicsMaterialConnections(nullptr, mMaterial);

  // Initialize the collider (mostly just set up physics nodes and some hierarchy information)
  bool dynamicallyCreated = (initializer.Flags & CreationFlags::DynamicallyAdded) != 0;
  ColliderInitialize(this, dynamicallyCreated);

  // Make sure all queue states are properly set up for a collider that was just initialized
  InternalTransformUpdate(cInitialize);
  
  // @JoshD needed?
  mPhysicsNode->RecomputeWorldTransform();
}

void Collider::OnAllObjectsCreated(CogInitializer& initializer)
{
  // Run the hierarchy OnAllObjectsCreated logic. This will hook up colliders to rigid bodies and more
  bool dynamicallyCreated = (initializer.Flags & CreationFlags::DynamicallyAdded) != 0;
  ColliderOnAllObjectsCreated(this, dynamicallyCreated);

  // If we were dynamically created, we have to check what joints were
  // connected to our object but don't know that a Collider is there.
  if(dynamicallyCreated)
    ForceRelinkJoints();
}

void Collider::TransformUpdate(TransformUpdateInfo& info)
{
  // If physics caused this transform update then we shouldn't do anything.
  // There's a chance someone could call this before we've been initialized
  // so guard against this by checking if we have a valid physics node.
  bool isPhysicsUpdate = (info.TransformFlags & TransformUpdateFlags::Physics) != 0;
  if(!isPhysicsUpdate && mPhysicsNode != nullptr)
    InternalTransformUpdate(cUpdate);
}

void Collider::AttachTo(AttachmentInfo& info)
{
  // Run all hierarchy logic for attachment
  PhysicsAttachTo(mPhysicsNode, info);
}

void Collider::Detached(AttachmentInfo& info)
{
  // Run all hierarchy logic for detachment
  PhysicsDetach(mPhysicsNode, info);
}

void Collider::DebugDraw()
{
  // We could draw the aabb or sphere here but we decided not to.
  // Keep the code commented out here for easy re-enabling when debugging.

  //mAabb.DebugDraw();
  //mBoundingSphere.DebugDraw();
}

void Collider::OnDestroy(uint flags)
{
  // Unlink/destroy joints/contacts (joints have to be unlinked while contacts actually have to be destroyed)
  UnlinkAllJoints();
  DestroyAllContacts(true);

  // Run hierarchy destruction information (clean up from rigid bodies, fix queues, remove from broadphase, etc...)
  bool dynamicallyRemoved = (flags & DestroyFlags::DynamicallyDestroyed) != 0;
  ColliderOnDestroy(this, dynamicallyRemoved);

  // Tell the space that this collider is being destroyed
  mSpace->RemoveComponent(this);

  // There's no need to clean up the BroadPhaseQueue or the PhysicsNode.
  // This is handled by the space when it detects a queue with no objects attached.
}

void Collider::CacheWorldValues()
{
  // Implement per collider to cache any world values
}

void Collider::ComputeWorldBoundingVolumes()
{
  CacheWorldValues();
  ComputeWorldAabbInternal();
  ComputeWorldBoundingSphereInternal();
}

void Collider::ComputeWorldBoundingSphereInternal()
{
  // By default set the bounding sphere to be the bounding sphere of the world-space aabb.
  // We use the aabb's position and size instead of our object's translation because the
  // "center" of the object might not be at the translation (e.g. convex meshes).
  Vec3 center, halfExtents;
  mAabb.GetCenterAndHalfExtents(center, halfExtents);
  mBoundingSphere.mCenter = center;
  mBoundingSphere.mRadius = halfExtents.Length();
}

real Collider::ComputeWorldVolumeInternal()
{
  // Expected that a derived collider will implement this
  return 0;
}

void Collider::ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia)
{
  // Expected that a derived collider will implement this
  localInvInertia.ZeroOut();
}

Vec3 Collider::GetColliderLocalCenterOfMass() const
{
  // Most collider's are symmetric so the local center of mass is at the origin.
  // If a collider isn't symmetric then it should override this (mesh collider for instance).
  return Vec3::cZero;
}

void Collider::RebuildModifiedResources()
{
  mMaterial->UpdateAndNotifyIfModified();
}

void Collider::Support(Vec3Param direction, Vec3Ptr support) const
{
  // If this is ever called on the base class then a derived type did not override this!
  Error("Support function not implemented.");
}

void Collider::GetCenter(Vec3Ref center) const
{
  // A good center point is the collider's translation
  center = GetWorldTranslation();
}

Intersection::SupportShape Collider::GetSupportShape(bool supportDelta)
{
  // Make a support shape class that wraps this collider's support functions
  return Intersection::MakeSupport(this, supportDelta);
}

PhysicsMaterial* Collider::GetMaterial()
{
  return mMaterial;
}

void Collider::SetMaterial(PhysicsMaterial* physicsMaterial)
{
  // Guard against users passing in a null material (from script)
  if(physicsMaterial == nullptr)
    return;

  // Disconnect from the old material and re-connect to the new material
  UpdatePhysicsMaterialConnections(mMaterial, physicsMaterial);
  mMaterial = physicsMaterial;

  OnPhysicsMaterialModified(nullptr);
}

CollisionGroup* Collider::GetCollisionGroup()
{
  return mSerializedGroup;
}

void Collider::SetCollisionGroup(CollisionGroup* collisionGroup)
{
  // Guard against users passing in a null group (from script)
  if(collisionGroup == nullptr)
    return;

  // Save the actual collision group but then look up the instance for the current table.
  // If we can find a collision group instance that is different then update the instance
  mSerializedGroup = collisionGroup;
  CollisionGroupInstance* newInstance = mSpace->GetCollisionGroupInstance(mSerializedGroup->mResourceId);
  if(newInstance != nullptr && newInstance != mCollisionGroupInstance)
  {
    mCollisionGroupInstance = newInstance;
    // As we've changed groups our collision logic could've also
    // changed so make sure to wake up our body.
    ForceAwake();
  }
}

bool Collider::GetGhost() const
{
  return mState.IsSet(ColliderFlags::Ghost);
}

void Collider::SetGhost(bool ghost)
{
  mState.SetState(ColliderFlags::Ghost, ghost);

  // Wake up all colliders in contact with this one since collisions could appear/disappear.
  // Normally islanding would take care of this and wake up all connected objects. If this is a
  // static or kinematic object then this will not wake the object up as waking asleep objects
  // only visits dynamic rigid bodies, not static or kinematic.
  Physics::JointHelpers::WakeUpConnected(mContactEdges);
}

bool Collider::GetSendsEvents() const
{
  return mState.IsSet(ColliderFlags::SendsEvents);
}

void Collider::SetSendsEvents(bool sendsMessages)
{
  mState.SetState(ColliderFlags::SendsEvents, sendsMessages);
}

Vec3 Collider::GetOffset() const
{
  // Make sure that the cached transform is up-to-date
  // (this will recompute mTranslation offset if necessary)
  mPhysicsNode->UpdateTransformAndMass(mSpace);

  return mTranslationOffset;
}

void Collider::SetOffset(Vec3Param localOffset)
{
  mTranslationOffset = localOffset;
  
  InternalTransformUpdate(cUpdate);
}

real Collider::ComputeVolume()
{
  // Since this function is exposed to the user, make sure that the
  // cached transform is up-to-date (so we compute the correct volume)
  RebuildModifiedResources();
  mPhysicsNode->UpdateTransformAndMass(mSpace);
  return ComputeWorldVolumeInternal();
}

bool Collider::ShouldCollide(Collider* otherCollider)
{
  return ShouldCollideInternal<false>(otherCollider, nullptr);
}

Vec3 Collider::GetWorldScale() const
{
  return mPhysicsNode->GetTransform()->GetWorldScale();
}

Mat3 Collider::GetWorldRotation() const
{
  return mPhysicsNode->GetTransform()->GetWorldRotation();
}

Vec3 Collider::GetWorldTranslation() const
{
  return mPhysicsNode->GetTransform()->GetWorldTranslation();
}

WorldTransformation* Collider::GetWorldTransform() const
{
  return mPhysicsNode->GetTransform();
}

RigidBody* Collider::GetActiveBody() const
{
  return mActiveRigidBody;
}

Vec3 Collider::GetRigidBodyCenterOfMass() const
{
  RigidBody* body = GetActiveBody();
  if(body != nullptr)
    return body->GetWorldCenterOfMass();
  return GetWorldTranslation();
}

Vec3 Collider::GetPointVelocity(Vec3Param worldPoint)
{
  // Since this function is exposed to the user, make sure
  // that the cached transform is up-to-date
  mPhysicsNode->UpdateTransformAndMass(mSpace);

  return ComputePointVelocityInternal(worldPoint);
}

Aabb Collider::GetWorldAabb()
{
  // Rebuild any modified resources (technically only meshes are needed but too much work to isolate)
  // and then make sure we update our own queue (which will update transform, aabb, etc...)
  RebuildModifiedResources();
  mSpace->UpdateTransformAndMassOfTree(mPhysicsNode);
  return mAabb;
}

Sphere Collider::GetWorldBoundingSphere()
{
  // See GetWorldAabb
  RebuildModifiedResources();
  mSpace->UpdateTransformAndMassOfTree(mPhysicsNode);
  return mBoundingSphere;
}

bool Collider::IsDynamic() const
{
  // If we have a rigid body return its state, otherwise we're static
  RigidBody* body = GetActiveBody();
  if(body != nullptr)
    return body->IsDynamic();
  return false;
}

bool Collider::IsStatic() const
{
  // If we have a rigid body return its state, otherwise we're static
  RigidBody* body = GetActiveBody();
  if(body != nullptr)
    return body->GetStatic();
  return true;
}

bool Collider::IsKinematic() const
{
  // If we have a rigid body return its state, otherwise we're static (not kinematic)
  RigidBody* body = GetActiveBody();
  if(body != nullptr)
    return body->GetKinematic();
  return false;
}

bool Collider::Is2D() const
{
  // If we have a rigid body return its state, otherwise we're not 2d
  RigidBody* body = GetActiveBody();
  if(body != nullptr)
    return body->mState.IsSet(RigidBodyStates::Mode2D);
  return false;
}

bool Collider::IsAsleep() const
{
  // If we have a rigid body return its state, otherwise we're not considered asleep
  RigidBody* body = GetActiveBody();
  if(body != nullptr)
    return body->IsAsleep();
  return false;
}

void Collider::ForceAwake()
{
  // If we have a rigid body then wake it up
  RigidBody* body = GetActiveBody();
  if(body != nullptr)
    body->ForceAwake();
}

uint Collider::GetContactCount()
{
  return mContactCount;
}

ContactRange Collider::GetContacts()
{
  return FilterContactRange(this);
}

uint Collider::GetJointCount()
{
  // This doesn't happen often so I deem this acceptable to count at run-time
  uint count = 0;
  JointEdgeList::range r = mJointEdges.All();
  for(; !r.Empty(); r.PopFront())
    ++count;
  return count;
}

JointRange Collider::GetJoints()
{
  return FilterJointRange<Joint>(this);
}

//-------------------------------------------------------------------Internal
Collider::ColliderType Collider::GetColliderType() const
{
  return mType;
}

u32 Collider::GetNextColliderId()
{
  return colliderId++;
}

void Collider::UpdatePhysicsMaterialConnections(PhysicsMaterial* oldMaterial, PhysicsMaterial* newMaterial)
{
  if(oldMaterial != nullptr)
    DisconnectAll(oldMaterial, this);
  if(newMaterial != nullptr)
    ConnectThisTo(newMaterial, Events::ResourceModified, OnPhysicsMaterialModified);
}

void Collider::OnPhysicsMaterialModified(Event* e)
{
  // Fix the massless flag now that we have a new material (deals with density of 0)
  UpdateMasslessColliderFlag();

  // If we have a rigid body then make sure to tell it to recompute
  // mass terms since our density was (likely) just changed.
  if(mActiveRigidBody)
    mActiveRigidBody->QueueMassUpdate();
}

void Collider::SetWorldAabbFromHalfExtents(Vec3Param worldHalfExtents)
{
  // Set the aabb's translation and scale and then "rotate" it. The efficient way to do
  // this is to compute the aabb of the rotated aabb (the aabb of the obb)
  mAabb.SetCenter(GetWorldTranslation());
  mAabb.SetHalfExtents(worldHalfExtents);
  mAabb.Transform(GetWorldRotation());
}

Vec3 Collider::TransformSupportDirectionToLocal(Vec3Param worldSupportDirection) const
{
  // The support direction behaves like a surface normal in that it needs the "inverse transposed" applied.
  // Since we're going to local space though this means that we need to apply the world scale not the inverse 
  // world scale (inverse of inverse). This is easiest to see with a diamond shape with a search direction of (1, 2) in world space.
  // If the shape has a scale of (1, 100) then the inverse scale would effectively remove the entire y-component
  // which would return the wrong point.
  WorldTransformation* worldTransform = GetWorldTransform();
  Vec3 localSpaceDir = worldTransform->InverseTransformSurfaceNormal(worldSupportDirection);
  return localSpaceDir;
}

Vec3 Collider::TransformSupportPointToWorld(Vec3Param localSupportPoint) const
{
  WorldTransformation* worldTransform = GetWorldTransform();
  Vec3 worldPoint = worldTransform->TransformPoint(localSupportPoint);
  return worldPoint;
}

real Collider::ComputeMass()
{
  // Since this function is internal we assume that the cached transform is up-to-date
  real volume = ComputeWorldVolumeInternal();
  real mass = volume * mMaterial->mDensity;
  return mass;
}

Vec3 Collider::GetColliderWorldCenterOfMass() const
{
  Vec3 localCenterOfMass = GetColliderLocalCenterOfMass();
  WorldTransformation* transform = GetWorldTransform();
  return transform->TransformPoint(localCenterOfMass);
}

Vec3 Collider::ComputePointVelocityInternal(Vec3Param worldPoint)
{
  RigidBody* body = GetActiveBody();
  if(body)
    return body->GetPointVelocityInternal(worldPoint);
  return Vec3::cZero;
}

bool Collider::NotCollideable() const
{
  return mState.IsSet(ColliderFlags::Ghost | ColliderFlags::MasslessBody);
}

bool Collider::GetHasPairFilter() const
{
  return mState.IsSet(ColliderFlags::HasPairFilter);
}

void Collider::SetHasPairFilter(bool hasPairFilter)
{
  mState.SetState(ColliderFlags::HasPairFilter, hasPairFilter);
}

bool Collider::ShouldCollide(Collider* otherCollider, String& reasonForNotColliding)
{
  return ShouldCollideInternal<true>(otherCollider, &reasonForNotColliding);
}

template <bool BuildReason>
bool Collider::ShouldCollideInternal(Collider* otherCollider, String* reasonForNotColliding)
{
  RigidBody* body1 = GetActiveBody();
  RigidBody* body2 = otherCollider->GetActiveBody();

  // Find each body's active parent. This probably isn't necessary since ActiveBody should return this, 
  // however this is done so we properly deal with static child bodies.
  while(body1 && !body1->IsDynamic() && body1->mParentBody != nullptr)
    body1 = body1->mParentBody;
  while(body2 && !body2->IsDynamic() && body2->mParentBody != nullptr)
    body2 = body2->mParentBody;

  // If the two colliders share an active body (or they don't have a body) then they shouldn't collide
  if(body1 == body2)
  {
    if(BuildReason)
      *reasonForNotColliding = "The colliders shared a active rigid body and hence don't check against each other.";
    return false;
  }

  // Check if the collision group says to skip detection
  if(mCollisionGroupInstance->SkipDetection(*(otherCollider->mCollisionGroupInstance)))
  {
    if(BuildReason)
      *reasonForNotColliding = "The collision table says to skip detection.";
    return false;
  }

  // If either collider has a pair filter then check if this pair has been register
  if(GetHasPairFilter() || otherCollider->GetHasPairFilter())
  {
    // Get the id of this pair and check if it's been registered with the current space
    CogId id1 = CogId(GetOwner());
    CogId id2 = CogId(otherCollider->GetOwner());
    u64 packedId = GetLexicographicId(id1.GetId(), id2.GetId());
    if(mSpace->mFilteredPairs.Contains(packedId))
    {
      if(BuildReason)
        *reasonForNotColliding = "A pair filter has been added to the physics space to ignore collision between these two objects.";
      return false;
    }
  }

  bool obj1Asleep = IsAsleep();
  bool obj1Static = IsStatic();
  bool obj1Kinematic = IsKinematic();
  bool obj1Ghost = NotCollideable();
  bool obj1Uninitialized = mState.IsSet(ColliderFlags::Uninitialized);
  bool obj2Asleep = otherCollider->IsAsleep();
  bool obj2Static = otherCollider->IsStatic();
  bool obj2Kinematic = otherCollider->IsKinematic();
  bool obj2Ghost =  otherCollider->NotCollideable();
  bool obj2Uninitialized = otherCollider->mState.IsSet(ColliderFlags::Uninitialized);

  // If both objects are asleep or static (and they weren't just created) then we skip detection
  if(((obj1Asleep && !obj1Uninitialized) || obj1Static) && 
    ((obj2Asleep && !obj2Uninitialized) || obj2Static))
  {
    if(BuildReason)
      *reasonForNotColliding = "Both objects are asleep or static.";
    return false;
  }

  // Check if there is a joint between these objects that says to skip detection
  JointEdgeList::range jointRange = mJointEdges.All();
  for(; !jointRange.Empty(); jointRange.PopFront())
  {
    JointEdge& edge = jointRange.Front();
    Joint* joint = edge.mJoint;

    // The list of joints is sorted such that all joints where CollideConnected is
    // false are before ones where CollidedConnected are true. Hence if we reach
    // one where it is true we can not check the rest.
    if(joint->GetCollideConnected())
      break;

    // If we can find one of these joints that is connected to the other collider then we can skip detection
    if(edge.mOther == otherCollider)
    {
      if(BuildReason)
        *reasonForNotColliding = "The colliders are connected by a joint that says to not collide connected.";
      return false;
    }
  }

  // Only after all this can we determine we should check collision
  return true;
}

void Collider::UpdateMasslessColliderFlag()
{
  // We're massless if the density is zero
  bool massless = false;
  if(mMaterial->GetDensity() == real(0))
    massless = true;
  mState.SetState(ColliderFlags::MasslessCollider, massless);
}

bool Collider::InDynamicBroadPhase() const
{
  // We belong in dynamic broadphase if we have a rigid body or are a spring system
  return mDirectRigidBody;
}

void Collider::InternalSizeChanged()
{
  // Our transform was effectively updated. We have to change our
  // half-extents, where we are in broadphase, mass terms, etc...
  InternalTransformUpdate(cUpdate);
  // Since the user changed our size we need to make sure the cached world values are updated.
  // This might not be needed since the transform update will cause this to happen later
  // but to avoid any missing dirty bit checks force this here.
  CacheWorldValues();
}

void Collider::InternalTransformUpdate(eUpdateTransformState updateState)
{
  // Since our transform has been updated out from under us we need to queue up a
  // full transform update (reading the Transform component and updating the cached local-to-world information)
  FullTransformAction transformAction(this);

  // If this object is massless, then there is no reason to update the mass properties
  // since transforming it will not change anything. Children who are not massless will
  // get their own transform update since it is propagated through the hierarchy.
  // (this will probably double the queuing, but oh well, it's not expensive)
  RigidBody* body = GetActiveBody();
  if(body != nullptr && !mState.IsSet(ColliderFlags::MasslessCollider))
  {
    body->QueueMassUpdate();
  }

  // This is all we need to do if this was initialization (not update)
  if(updateState != cUpdate)
    return;

  // Since we were transformed we could get new contacts, enter
  // force regions, etc... so make sure to wake up
  ForceAwake();
  // Also queue that we've moved the object
  MovementAction action(this);
}

void Collider::GenerateIntegrationUpdate()
{
  IntegrationAction action(this);
}

void Collider::UpdateQueue()
{
  // Force flush this object's transform and mass state (ignore spatial partition for now)
  mPhysicsNode->UpdateTransformAndMass(mSpace);
}

void Collider::ForceRelinkJoints()
{
  // Check if we have an ObjectLinkAnchor. If we do then there's likely
  // some joints connected to use we need to fix.
  ObjectLinkAnchor* anchor = GetOwner()->has(ObjectLinkAnchor);
  if(anchor == nullptr)
    return;

  // Force all object links attached to us to re-link
  ObjectLinkAnchor::EdgeList::range edges = anchor->mEdges.All();
  for(; !edges.Empty(); edges.PopFront())
  {
    ObjectLink* link = edges.Front().GetObjectLink();

    // If the object link doesn't have a joint, then we don't care about it
    Joint* joint = link->GetOwner()->has(Joint);
    if(joint == nullptr)
      continue;
 
    // Easiest way to fix joints to know about this collider is to unlink and then link again
    joint->UnLinkPair();
    joint->LinkPair();
  }
}

void Collider::UnlinkAllJoints()
{
  JointEdgeList::range jointRange = mJointEdges.All();
  while(!jointRange.Empty())
  {
    Joint* joint = jointRange.Front().mJoint;
    jointRange.PopFront();
    
    // We need to remove ourself from the joint however due to joints being allowed to
    // live even when they're invalid we can't just tell the joint to unlink itself.
    // Instead we need to remove ourself from this joint while keeping the other collider
    // in-tact so it can become valid again. Afterwards the joint should be marked invalid.
    if(joint->GetCollider(0) == this)
    {
      joint->mEdges[0].mCollider = nullptr;
      JointEdgeList::Unlink(&joint->mEdges[0]);
    }
    else if(joint->GetCollider(1) == this)
    {
      joint->mEdges[1].mCollider = nullptr;
      JointEdgeList::Unlink(&joint->mEdges[1]); 
    }
    joint->SetValid(false);
  }
  mJointEdges.Clear();
}

void Collider::DestroyAllContacts(bool sendImmediately)
{
  ContactEdgeList::range contactRange = mContactEdges.All();
  while(!contactRange.Empty())
  {
    Physics::Contact* contact = contactRange.Front().mContact;
    contactRange.PopFront();
    contact->Destroy(sendImmediately);
  }
  mContactEdges.Clear();
  mContactCount = 0;
}

}//namespace Zero
