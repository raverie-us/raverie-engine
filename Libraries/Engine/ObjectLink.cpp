///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(ObjectLinkChanged);
  DefineEvent(ObjectLinkPointChanged);
}//namespace Events

//-------------------------------------------------------------------ObjectLinkEdge
ZilchDefineType(ObjectLinkEdge, builder, type)
{
  ZeroBindDocumented();
  // This should be safe to bind even though we contain an inlist link because when
  // we're returned we can't perform any operations on the link itself.
  ZilchBindDefaultCopyDestructor();

  ZilchBindGetterProperty(SelfCog);
  ZilchBindGetterProperty(OtherCog);
  ZilchBindGetterProperty(ObjectLink);
}

ObjectLinkEdge::ObjectLinkEdge()
{
}

bool ObjectLinkEdge::IsValid() const
{
  ObjectLinkAnchor* selfAnchor = mSelfAnchorHandle;
  return selfAnchor != nullptr;
}

Cog* ObjectLinkEdge::GetSelfCog()
{
  ObjectLinkAnchor* selfAnchor = mSelfAnchorHandle;
  if(selfAnchor != nullptr)
    return selfAnchor->GetOwner();
  return nullptr;
}

Cog* ObjectLinkEdge::GetOtherCog()
{
  ObjectLink* objectLink = GetObjectLink();
  if(objectLink == nullptr)
    return nullptr;

  if(&objectLink->mEdges[0] == this)
    return objectLink->mEdges[1].GetSelfCog();
  return objectLink->mEdges[0].GetSelfCog();
}

ObjectLink* ObjectLinkEdge::GetObjectLink()
{
  ObjectLink* objectLink = mObjectLinkHandle;
  return objectLink;
}

void ObjectLinkEdge::Set(ObjectLink* link, Cog* cog)
{
  mObjectLinkHandle = link;

  // Find the ObjectLinkAnchor on the passed in cog (if it doesn't exist then add it)
  ObjectLinkAnchor* cogAnchor = nullptr;
  if(cog != nullptr)
    cogAnchor = HasOrAdd<ObjectLinkAnchor>(cog);

  // If we were previously linked then unlink first
  ObjectLinkAnchor* oldSelfAnchor = mSelfAnchorHandle;
  if(oldSelfAnchor != nullptr)
    oldSelfAnchor->mEdges.Unlink(this);

  // Set the anchor and if we it exists (will always exist if the cog wasn't null) then add this edge to the anchor
  mSelfAnchorHandle = cogAnchor;
  if(cogAnchor != nullptr)
    cogAnchor->mEdges.PushBack(this);
}

void ObjectLinkEdge::Clear()
{
  // If we were already linked then first unlink from the anchor
  ObjectLinkAnchor* selfAnchor = mSelfAnchorHandle;
  if(selfAnchor != nullptr)
    selfAnchor->mEdges.Erase(this);
  
  // Debug safety...
  mSelfAnchorHandle = nullptr;
  mObjectLinkHandle = nullptr;
}

//-------------------------------------------------------------------ObjectLinkRange
ObjectLinkRange::ObjectLinkRange()
{
}

ObjectLinkRange::ObjectLinkRange(Cog* cog)
{
  ObjectLinkAnchor* anchor = cog->has(ObjectLinkAnchor);
  //if we have a anchor, make a valid range, otherwise make an empty range
  if(anchor)
    mRange = anchor->mEdges.All();
  else
    mRange = EdgeRange(nullptr, nullptr);
}

ObjectLinkRange::ObjectLinkRange(ObjectLinkAnchor* anchor)
{
  mRange = anchor->mEdges.All();
}

bool ObjectLinkRange::Empty()
{
  return mRange.Empty();
}

void ObjectLinkRange::PopFront()
{
  mRange.PopFront();
}

ObjectLinkEdge& ObjectLinkRange::Front()
{
  return mRange.Front();
}

//-------------------------------------------------------------------ObjectLinkAnchor
ZilchDefineType(ObjectLinkAnchor, builder, type)
{
  ZeroBindComponent();
  type->AddAttribute(ObjectAttributes::cHidden);

  ZeroBindDocumented();
  ZilchBindGetterProperty(ObjectLinks);
}

void ObjectLinkAnchor::OnDestroy(uint flags)
{
  ClearLinks();
}

ObjectLinkRange ObjectLinkAnchor::GetObjectLinks()
{
  return ObjectLinkRange(this);
}

void ObjectLinkAnchor::ClearLinks()
{
  // Remove all edges from this anchor
  while(!mEdges.Empty())
  {
    ObjectLinkEdge& edge = mEdges.Front();
    edge.Clear();
  }
}

//-------------------------------------------------------------------ObjectLink
ObjectLink::ObjectLink()
{
  mBodyPoints[IndexA].ZeroOut();
  mBodyPoints[IndexB].ZeroOut();
  mValid = false;
}

ZilchDefineType(ObjectLink, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(ObjectAPath)->AddAttribute(PropertyAttributes::cLocalModificationOverride);
  ZilchBindGetterSetterProperty(ObjectBPath)->AddAttribute(PropertyAttributes::cLocalModificationOverride);

  ZilchBindGetterSetter(ObjectA);
  ZilchBindGetterSetter(ObjectB);

  ZilchBindGetterSetterProperty(WorldPointA);
  ZilchBindGetterSetterProperty(WorldPointB);

  ZilchBindGetterSetterProperty(LocalPointA)->ZeroSerialize(Vec3::cZero);
  ZilchBindGetterSetterProperty(LocalPointB)->ZeroSerialize(Vec3::cZero);

  ZeroBindEvent(Events::ObjectLinkChanged, ObjectLinkEvent);
  ZeroBindEvent(Events::ObjectLinkPointChanged, ObjectLinkPointChangedEvent);
  ZeroBindTag(Tags::Core);
}

void ObjectLink::Serialize(Serializer& stream)
{
  stream.SerializeFieldDefault("ObjectAPath", mObjectPaths[IndexA], CogPath());
  stream.SerializeFieldDefault("ObjectBPath", mObjectPaths[IndexB], CogPath());

  stream.SerializeFieldDefault("LocalPointA", mBodyPoints[IndexA], Vec3::cZero);
  stream.SerializeFieldDefault("LocalPointB", mBodyPoints[IndexB], Vec3::cZero);
}

void ObjectLink::Initialize(CogInitializer& initializer)
{
  // We can't do anything in initialize as cog paths can't be resolved until
  // OnAllObjects Created. Just listen for when these cog path's change.
  ConnectThisTo(&mObjectPaths[IndexA], Events::CogPathCogChanged, OnObjectAChanged);
  ConnectThisTo(&mObjectPaths[IndexB], Events::CogPathCogChanged, OnObjectBChanged);

  // Hack: For now add the visualizer here (so you can click on the object link)
  if(Z::gRuntimeEditor)
    Z::gRuntimeEditor->Visualize(this, "Connection");
}

void ObjectLink::OnAllObjectsCreated(CogInitializer& initializer)
{
  // To deal with ordering issues, some joints might trigger an on all objects
  // created on this object. This may be the first time we are run and it may
  // not, so just prevent from being run twice here.
  if(mValid)
    return;

  mValid = true;

  // Now that all the objects have been created find the two objects
  Cog* objectA = mObjectPaths[IndexA].RestoreLink(initializer, this, "ObjectA");
  Cog* objectB = mObjectPaths[IndexB].RestoreLink(initializer, this, "ObjectB");

  // Set each edge to point at the resolved cogs (they can be null)
  // and pass in ourself as the owner of the edges
  mEdges[0].Set(this, objectA);
  mEdges[1].Set(this, objectB);
}

void ObjectLink::OnDestroy(uint flags)
{
  Unlink();
}

CogPath ObjectLink::GetObjectAPath()
{
  return mObjectPaths[IndexA];
}

void ObjectLink::SetObjectAPath(CogPath& path)
{
  mObjectPaths[IndexA] = path;
}

CogPath ObjectLink::GetObjectBPath()
{
  return mObjectPaths[IndexB];
}

void ObjectLink::SetObjectBPath(CogPath& path)
{
  mObjectPaths[IndexB] = path;
}

Cog* ObjectLink::GetObjectA()
{
  return GetCog(IndexA);
}

void ObjectLink::SetObjectA(Cog* cog)
{
  SetCog(cog, IndexA);
}

Cog* ObjectLink::GetObjectB()
{
  return GetCog(IndexB);
}

void ObjectLink::SetObjectB(Cog* cog)
{
  SetCog(cog, IndexB);
}

Vec3 ObjectLink::GetLocalPointA()
{
  return GetLocalPoint(IndexA);
}

void ObjectLink::SetLocalPointA(Vec3 newPoint)
{
  SetLocalPoint(newPoint, IndexA);
}

Vec3 ObjectLink::GetLocalPointB()
{
  return GetLocalPoint(IndexB);
}

void ObjectLink::SetLocalPointB(Vec3 newPoint)
{
  SetLocalPoint(newPoint, IndexB);
}

Vec3 ObjectLink::GetWorldPointA()
{
  return GetWorldPoint(IndexA);
}

void ObjectLink::SetWorldPointA(Vec3Param worldPoint)
{
  // Register side-effect properties
  if(OperationQueue::IsListeningForSideEffects())
    OperationQueue::RegisterSideEffect(this, "LocalPointA", GetLocalPointA());

  SetWorldPoint(worldPoint, IndexA);
}

Vec3 ObjectLink::GetWorldPointB()
{
  return GetWorldPoint(IndexB);
}

void ObjectLink::SetWorldPointB(Vec3Param worldPoint)
{
  // Register side-effect properties
  if(OperationQueue::IsListeningForSideEffects())
    OperationQueue::RegisterSideEffect(this, "LocalPointB", GetLocalPointB());

  SetWorldPoint(worldPoint, IndexB);
}

Vec3 ObjectLink::GetWorldPosition()
{ 
  // Get the world point from each object
  Vec3 worldPointA = GetWorldPointA();
  Vec3 worldPointB = GetWorldPointB();

  // Get each objects transform so we can determine if we should average the positions or not
  Transform* transformA = GetTransform(IndexA);
  Transform* transformB = GetTransform(IndexB);

  // If A doesn't have a transform but B does then just use B's position
  if(transformA == nullptr && transformB != nullptr)
    return worldPointB;
  // The flip case (we have A but not B)
  if(transformB == nullptr && transformA != nullptr)
    return worldPointA;

  // Otherwise show the center of the 2
  return (worldPointA + worldPointB) / 2.0f;
}

void ObjectLink::OnObjectAChanged(Event* e)
{
  // The cog path was already changed so we shouldn't set it again
  // (otherwise we are trying to double set), we just need to link the edges up
  LinkCog(GetCog(IndexA), IndexA);
}

void ObjectLink::OnObjectBChanged(Event* e)
{
  // The cog path was already changed so we shouldn't set it again
  // (otherwise we are trying to double set), we just need to link the edges up
  LinkCog(GetCog(IndexB), IndexB);
}

Cog* ObjectLink::GetCog(ObjectIndex index)
{
  if(index != IndexA && index != IndexB)
    return nullptr;

  return mObjectPaths[index].GetCog();
}

void ObjectLink::SetCog(Cog* newCog, ObjectIndex index)
{
  if(index != IndexA && index != IndexB)
    return;

  // Set the cog path for the new cog
  SetCogInternal(newCog, index);

  // Link the new cog's edge
  LinkCog(newCog, index);
}

void ObjectLink::LinkCog(Cog* newCog, ObjectIndex index)
{
  Cog* oldCog = GetCog(index);

  // Remove the old link info and set the new info
  if(mEdges[index].IsValid())
    mEdges[index].Clear();
  mEdges[index].Set(this, newCog);

  // Send an event so that other components can know a cog we linked to changed.
  // This is mostly for joints to know and re-hook up.
  ObjectLinkEvent toSend;
  toSend.Set(index, mEdges[index].GetSelfCog(), newCog);
  GetOwner()->DispatchEvent(Events::ObjectLinkChanged, &toSend);
}

void ObjectLink::SetCogAInternal(Cog* cog)
{
  SetCogInternal(cog, IndexA);
}

void ObjectLink::SetCogBInternal(Cog* cog)
{
  SetCogInternal(cog, IndexB);
}

void ObjectLink::SetCogInternal(Cog* cog, ObjectIndex index)
{
  mObjectPaths[index].SetCog(cog);
}

Transform* ObjectLink::GetTransform(ObjectIndex index)
{
  Cog* cog = GetCog(index);

  Transform* transform = nullptr;
  if(cog != nullptr)
    transform = cog->has(Transform);
  
  return transform;
}

Vec3 ObjectLink::GetLocalPoint(ObjectIndex index)
{
  return mBodyPoints[index];
}

void ObjectLink::SetLocalPoint(Vec3Param localPoint, ObjectIndex index)
{
  ObjectLinkPointChangedEvent toSend;
  toSend.Set(index, mBodyPoints[index], localPoint);
  GetOwner()->DispatchEvent(Events::ObjectLinkPointChanged, &toSend);

  mBodyPoints[index] = localPoint;
}

Vec3 ObjectLink::GetWorldPoint(ObjectIndex index)
{
  // Start with the local point
  Vec3 worldPoint = GetLocalPoint(index);
  
  // If we can get a transform then bring the point into world space
  Transform* transform = GetTransform(index);
  if(transform != nullptr)
    worldPoint = transform->TransformPoint(worldPoint);

  return worldPoint;
}

void ObjectLink::SetWorldPoint(Vec3Param worldPoint, ObjectIndex index)
{
  Vec3 localPoint = worldPoint;

  // If we can get a transform the bring the world point into local space
  Transform* transform = GetTransform(index);
  if(transform != nullptr)
    localPoint = transform->TransformPointInverse(worldPoint);

  SetLocalPoint(localPoint, index);
}

void ObjectLink::Unlink()
{
  //unlink the edges and mark ourself as being invalid now
  mEdges[0].Clear();
  mEdges[1].Clear();
  mValid = false;
}

//-------------------------------------------------------------------ObjectLinkEvent
ZilchDefineType(ObjectLinkEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindFieldProperty(NewCog);
  ZilchBindFieldProperty(OldCog);
  ZilchBindFieldProperty(EdgeId);
}

ObjectLinkEvent::ObjectLinkEvent()
{
  NewCog = nullptr;
  OldCog = nullptr;
  EdgeId = 0;
}

void ObjectLinkEvent::Set(uint edgeIndex, Cog* oldCog, Cog* newCog)
{
  EdgeId = edgeIndex;
  OldCog = oldCog;
  NewCog = newCog;
}

//-------------------------------------------------------------------ObjectLinkPointChangedEvent
ZilchDefineType(ObjectLinkPointChangedEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindFieldProperty(mOldLocalPoint);
  ZilchBindFieldProperty(mNewLocalPoint);
  ZilchBindFieldProperty(mEdgeId);
}

ObjectLinkPointChangedEvent::ObjectLinkPointChangedEvent()
{
  mEdgeId = 0;
}

void ObjectLinkPointChangedEvent::Set(uint edgeIndex, Vec3Param oldPoint, Vec3Param newPoint)
{
  mEdgeId = edgeIndex;
  mOldLocalPoint = oldPoint;
  mNewLocalPoint = newPoint;
}

}//namespace Zero
