///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(CastFilterCallback);
}//namespace Events

//-------------------------------------------------------------------CastFilterEvent
ZilchDefineType(CastFilterEvent, builder, type)
{
  ZeroBindDocumented();
  ZilchBindGetterProperty(Object);
  ZilchBindFieldProperty(mFilterState);
}

CastFilterEvent::CastFilterEvent()
{
  mCollider = nullptr;
  mFilterState = CastFilterState::DefaultBehavior;
}

Cog* CastFilterEvent::GetObject()
{
  return mCollider->GetOwner();
}

//-------------------------------------------------------------------CastFilter
ZilchDefineType(CastFilter, builder, type)
{
  ZeroBindDocumented();
  ZilchBindGetterSetterProperty(CollisionGroup);

  ZilchBindDefaultCopyDestructor();

  ZilchBindGetterSetterProperty(IgnoreCog);

  ZeroBindEvent(Events::CastFilterCallback, CastFilterEvent);
  ZilchBindFieldProperty(mCallbackObject);
  ZilchBindFieldProperty(mCallbackEventName);
}

CastFilter::CastFilter() : BaseCastFilter()
{
  mFilterGroup = nullptr;
  mCallbackObject = nullptr;
  mCallbackEventName = Events::CastFilterCallback;
}

bool CastFilter::IsValid(void* clientData)
{
  Collider* collider = static_cast<Collider*>(clientData);
  
  // If there is a callback object then check to see if we should skip this object
  if(mCallbackObject != nullptr)
  {
    CastFilterEvent toSend;
    toSend.mCollider = collider;
    EventDispatcher* dispatcher = mCallbackObject->GetDispatcher();
    if(dispatcher != nullptr)
    {
      dispatcher->Dispatch(mCallbackEventName, &toSend);
      // If the user changed the state from the default behavior then don't perform
      // any other filter logic, simply return whether or not the user accepts this object.
      if(toSend.mFilterState != CastFilterState::DefaultBehavior)
        return toSend.mFilterState == CastFilterState::Accept;
    }
  }

  if(collider->GetOwner()->mFlags.IsSet(CogFlags::Locked | CogFlags::SelectionLimited))
    return false;

  if(IsSet(BaseCastFilterFlags::IgnoreGhost) && collider->GetGhost())
    return false;
  if(IsSet(BaseCastFilterFlags::IgnoreKinematic) && collider->IsKinematic())
    return false;
  if(IsSet(BaseCastFilterFlags::IgnoreDynamic) && collider->IsDynamic())
    return false;
  if(IsSet(BaseCastFilterFlags::IgnoreStatic) && collider->IsStatic())
    return false;

  // Check for the cog to ignore
  if(collider->GetOwner() == static_cast<Cog*>(mIgnoredCog))
    return false;

  // Check for collision groups
  if(mFilterGroup != nullptr)
  {
    CollisionGroupInstance* groupInstance = collider->mSpace->GetCollisionGroupInstance(mFilterGroup->mResourceId);
    
    if(groupInstance->SkipDetection(*(collider->mCollisionGroupInstance)))
      return false;
  }

  return true;
}

CollisionGroup* CastFilter::GetCollisionGroup()
{
  return mFilterGroup;
}

void CastFilter::SetCollisionGroup(CollisionGroup* group)
{
  mFilterGroup = group;
}

Cog* CastFilter::GetIgnoreCog() const
{
  return static_cast<Cog*>(mIgnoredCog);
}

void CastFilter::SetIgnoreCog(Cog* cog)
{
  mIgnoredCog = cog;
}

//-------------------------------------------------------------------CastResult
ZilchDefineType(CastResult, builder, type)
{
  ZilchBindDefaultCopyDestructor();
  ZeroBindDocumented();

  ZilchBindGetterProperty(ObjectHit);
  ZilchBindGetterProperty(Collider);
  ZilchBindGetterProperty(WorldPosition);
  ZilchBindGetterProperty(Normal);
  ZilchBindGetterProperty(Distance);
  ZilchBindMethod(GetLocalPosition);
}

CastResult::CastResult()
{
  mObjectHit = nullptr;
  mPoints[0] = Vec3::cZero;
  mPoints[1] = Vec3::cZero;
  mContactNormal = Vec3::cZero;
  mTime = Math::PositiveMax();
  mShapeIndex = 0;
}

CastResult::CastResult(const CastResult& rhs)
{
  mObjectHit = rhs.mObjectHit;
  mPoints[0] = rhs.mPoints[0];
  mPoints[1] = rhs.mPoints[1];
  mContactNormal = rhs.mContactNormal;
  mTime = rhs.mTime;
  mShapeIndex = rhs.mShapeIndex;
}

void CastResult::operator=(const CastResult& rhs)
{
  mObjectHit = rhs.mObjectHit;
  mPoints[0] = rhs.mPoints[0];
  mPoints[1] = rhs.mPoints[1];
  mContactNormal = rhs.mContactNormal;
  mTime = rhs.mTime;
  mShapeIndex = rhs.mShapeIndex;
}

CogId CastResult::GetCogId()
{
  if(mObjectHit == nullptr)
    return CogId();

  return mObjectHit->GetOwner()->GetId();
}

Collider* CastResult::GetCollider()
{
  return mObjectHit;
}

Cog* CastResult::GetObjectHit()
{
  if(mObjectHit == nullptr)
    return nullptr;

  return mObjectHit->GetOwner();
}

Vec3 CastResult::GetLocalPosition(uint pointIndex)
{
  if(mObjectHit == nullptr)
    return Vec3::cZero;

  if(pointIndex >= 2)
  {
    String msg = String::Format("Index %d is invalid. Index 0 and 1 are the only valid values", pointIndex);
    DoNotifyException("Invalid index", msg);
  }

  WorldTransformation* transform = mObjectHit->GetWorldTransform();
  return transform->InverseTransformPoint(mPoints[pointIndex]);
}

Vec3 CastResult::GetWorldPosition()
{
  return mPoints[0];
}

Vec3 CastResult::GetNormal()
{
  return mContactNormal;
}

real CastResult::GetDistance()
{
  return mTime;
}

//-------------------------------------------------------------------CastResults
ZilchDefineType(CastResults, builder, type)
{
  ZilchBindMethod(All);
  ZilchBindMethod(Empty);
  ZilchBindMethod(Clear);
}

CastFilter CastResults::mDefaultFilter;

CastResults::CastResults(uint amount, BaseCastFilter& filter) :
  mResults((ProxyCastResultArray&)mArray, filter)
{
  const uint maxResults = 100000;

  // Sanity check.  Can't cast a ray and get back 0 results.
  if(amount == 0)
  {
    DoNotifyTimer("Ray/Volume Cast Error",
      "Cannot make a cast with 0 results.  Result count set to 1.",
      "Warning", 1.0f);
    amount = 1;
  }
  if(amount > maxResults)
  {
    DoNotifyTimer("Ray/Volume Cast Error",
      String::Format("Cannot have %d results in a cast, clamping to %d", amount, maxResults),
      "Warning", 1.0f);
    amount = maxResults;
  }
  mArray.Resize(amount);
}

CastResults::CastResults(const CastResults& rhs) :
  mResults((ProxyCastResultArray&)mArray, rhs.mResults.Filter)
{
  uint count = rhs.Capacity();
  mArray.Resize(count);
  for(uint i = 0; i < rhs.Size(); ++i)
    mArray[i] = rhs[i];
  mResults.CurrSize = rhs.mResults.CurrSize;
}

CastResult& CastResults::operator[](uint index)
{
  ErrorIf(index >= mResults.CurrSize, "Index out of range.");
  return mArray[index];
}

const CastResult& CastResults::operator[](uint index) const
{
  ErrorIf(index >= mResults.CurrSize, "Index out of range.");
  return mArray[index];
}

uint CastResults::Size() const
{
  return mResults.GetCurrentSize();
}

bool CastResults::Empty()
{
  return Size() == 0;
}

CastResults::range CastResults::All()
{
  return range(mArray.Begin(), mArray.Begin() + mResults.CurrSize);
}

void CastResults::Clear()
{
  mResults.Clear();
}

void CastResults::Resize(uint amount)
{
  mArray.Resize(amount);
}

uint CastResults::Capacity() const
{
  return mArray.Size();
}

void CastResults::ConvertToColliders()
{
  // The pointer stored in the array (in mArray) is a pointer to a proxy, even
  // though it's type is a Collider pointer. We allowed the broad phase to think
  // it was a proxy pointer to avoid allocations.  Now we're just grabbing the
  // data pointer (the Collider) stored in the proxy and replacing the proxy
  // pointer with the Collider pointer.
  // NOTE: I'm going through mResults.Results[i], which is a reference to mArray,
  // to avoid having to cast twice on one line.
  for(uint i = 0; i < mResults.CurrSize; ++i)
  {
    void* clientData = mResults.Results[i].mObjectHit;
    mArray[i].mObjectHit = static_cast<Collider*>(clientData);
  }
}

//------------------------------------------------------------CastResultsRange
CastResultsRange::CastResultsRange(const CastResults& castResults)
{
  uint count = castResults.Size();
  mArray.Resize(count);
  for(uint i = 0; i < count; ++i)
    mArray[i] = castResults[i];
  mRange = mArray.All();
}

CastResultsRange::CastResultsRange(const CastResultsRange& rhs)
{
  uint count = rhs.mArray.Size();
  mArray.Resize(count);
  for(uint i = 0; i < count; ++i)
    mArray[i] = rhs.mArray[i];
  mRange = mArray.All();
}

bool CastResultsRange::Empty()
{
  return mRange.Empty();
}

CastResult& CastResultsRange::Front()
{
  return mRange.Front();
}

void CastResultsRange::PopFront()
{
  mRange.PopFront();
}

uint CastResultsRange::Size()
{
  return mRange.Size();
}

}//namespace Zero
