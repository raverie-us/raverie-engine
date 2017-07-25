///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------RayCastEntry
RayCastEntry::RayCastEntry()
{
  HitCog = nullptr;
  HitUv = Vec2::cZero;
  HitUvProvided = false;
}

bool RayCastEntry::operator>(RayCastEntry& rhs)
{
  // We first compare T values, then we sort those that provided uvs first
  // (more information, especially for Reactive)
  return T > rhs.T || (T == rhs.T && !HitUvProvided);
}

//-------------------------------------------------------------------RaycastResultList
RaycastResultList::RaycastResultList(size_t capacity)
{
  // Start out with no size
  mSize = 0;
  mCapacity = 0;
  SetCapacity(capacity);
}

void RaycastResultList::SetCapacity(size_t capacity)
{
  // Handle a capacity of 0
  if(capacity == 0)
  {
    ErrorIf(capacity == 0, "Capacity cannot be set to 0. Setting capacity to 1.");
    capacity = 1;
  }

  // If we shrink our capacity then make sure to shrink the current size
  if(capacity < mCapacity)
    mSize = Math::Min(mSize,capacity);

  mCapacity = capacity;
  // Make sure that we can access up to the full capacity
  mEntries.Resize(capacity);
}

void RaycastResultList::AddItem(RayCastEntry& inputEntry)
{
  // If this is the first item then just insert it
  if(mSize == 0)
  {
    mEntries[0] = inputEntry;
    ++mSize;
    return;
  }

  // Otherwise find out what index to start at
  size_t startIndex = mSize;
  // If we're at the max size
  if(mSize == mCapacity)
  {
    // Then we start at the last item. If the time of intersection is after the last item
    // then we do not want to insert this item as it happens too late in time.
    startIndex = mCapacity - 1;
    if(inputEntry.T >= mEntries[startIndex].T)
      return;
  }
  // If we're not at the max size then we just grow by one
  else
    ++mSize;

  // Insert at the starting index
  mEntries[startIndex] = inputEntry;

  // While we still have indices to go through, if the item before the current
  // one is larger then swap the two. This will effectively sort the item down
  // into the correct position (insertion sort)
  while(startIndex > 0 && mEntries[startIndex - 1] > mEntries[startIndex])
  {
    Math::Swap(mEntries[startIndex - 1],mEntries[startIndex]);
    --startIndex;
  }
}

void RaycastResultList::AddItem(Cog* hitCog, float t, Vec3Param worldPosition, Vec3Param worldNormal, Vec2Param uv, bool uvProvided)
{
  RayCastEntry entry;
  entry.HitCog = hitCog;
  entry.Instance = hitCog;
  entry.T = t;
  entry.HitWorldPosition = worldPosition;
  entry.HitWorldNormal = worldNormal;
  entry.HitUv = uv;
  entry.HitUvProvided = uvProvided;
  AddItem(entry);
}

void RaycastResultList::AddMetaItem(HandleParam object, float t, Vec3Param worldPosition, Vec3Param worldNormal, Vec2Param uv, bool uvProvided)
{
  RayCastEntry entry;
  entry.Instance = object;
  entry.T = t;
  entry.HitWorldPosition = worldPosition;
  entry.HitWorldNormal = worldNormal;
  entry.HitUv = uv;
  entry.HitUvProvided = uvProvided;
  AddItem(entry);
}

void RaycastResultList::AddList(RaycastResultList& list)
{
  // If our current list is empty
  if(mSize == 0)
  {
    // Add items from the other list up to our max capacity
    for(size_t i = 0; i < mCapacity && i < list.mSize; ++i)
    {
      mEntries[i] = list.mEntries[i];
      ++mSize;
    }
    return;
  }
  // If the new list is empty then there's nothing to do
  if(list.mSize == 0)
    return;

  // Since multiple providers may point to the same object we need to filter out
  // multiple occurrences of the same item. To do this we just insert all of the
  // items from both list into a map and keep the smaller time if the item was already there.

  // Add all the items from this list
  typedef HashMap<Handle, RayCastEntry> CastMap;
  CastMap map;
  for(size_t i = 0; i < mSize; ++i)
  {
    RayCastEntry& entry = mEntries[i];
    map.Insert(entry.Instance, entry);
  }

  // Add all of the items from the other list (keep the smallest time of an item)
  for(uint i = 0; i < list.mSize; ++i)
  {
    RayCastEntry& entry = list.mEntries[i];
    RayCastEntry* foundEntry = map.FindPointer(entry.Instance);
    if(foundEntry == nullptr)
      map.Insert(entry.Instance, entry);
    else if(*foundEntry > entry)
      *foundEntry = entry;
  }

  // Now the list contains all of the items with their smallest times.
  // Just add all of the items (which will automatically sort and store up to capacity)
  mSize = 0;
  CastMap::range range = map.All();
  for(; !range.Empty(); range.PopFront())
  {
    CastMap::value_type& pair = range.Front();
    RayCastEntry& entry = pair.second;
    AddItem(entry);
  }
}

//-------------------------------------------------------------------CastInfo
CastInfo::CastInfo(Space* targetSpace, Cog* cameraCog, Vec2Param mousePosition)
{
  SetInfo(targetSpace, cameraCog, mousePosition, mousePosition);
}

CastInfo::CastInfo(Space* targetSpace, Cog* cameraCog, Vec2Param dragStart, Vec2Param dragEnd)
{
  SetInfo(targetSpace, cameraCog, dragStart, dragEnd);
}

void CastInfo::SetInfo(Space* targetSpace, Cog* cameraCog, Vec2Param dragStart, Vec2Param dragEnd)
{
  mTargetSpace = targetSpace;
  mCameraCog = cameraCog;
  mMouseCurrent = dragEnd;
  mMouseDragStart = dragStart;
}

//-------------------------------------------------------------------RaycastProvider
ZilchDefineType(RaycastProvider, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
  ZilchBindFieldProperty(mActive);
}

//-------------------------------------------------------------------Raycaster
ZilchDefineType(Raycaster, builder, type)
{
}

Raycaster::~Raycaster()
{
  // We own all of the providers, so delete them
  for(uint i = 0; i < mProviders.Size(); ++i)
    delete mProviders[i];
  mProviders.Clear();
}

void Raycaster::AddProvider(RaycastProvider* provider)
{
  mProviders.PushBack(provider);
}

void Raycaster::RayCast(Ray& ray, CastInfo& castInfo, RaycastResultList& results)
{
  ray.Direction.Normalize();
  for(uint i = 0; i < mProviders.Size(); ++i)
  {
    RaycastProvider* provider = mProviders[i];
    // Skip inactive providers
    if(!provider->mActive)
      continue;

    RaycastResultList providerResults(results.mCapacity);
    provider->RayCast(ray,castInfo,providerResults);
    // Merge the new list into the total list
    results.AddList(providerResults);
  }
}

void Raycaster::FrustumCast(Frustum& frustum, CastInfo& castInfo, RaycastResultList& results)
{
  for(uint i = 0; i < mProviders.Size(); ++i)
  {
    RaycastProvider* provider = mProviders[i];
    // Skip inactive providers
    if(!provider->mActive)
      continue;

    RaycastResultList providerResults(results.mCapacity);
    provider->FrustumCast(frustum,castInfo,providerResults);
    // Merge the new list into the total list
    results.AddList(providerResults);
  }
}

//-------------------------------------------------------------------RaycasterMetaComposition
ZilchDefineType(RaycasterMetaComposition, builder, type)
{
}

RaycasterMetaComposition::RaycasterMetaComposition(size_t raycasterClassOffset) :
  MetaComposition(ZilchTypeId(Raycaster)),
  mRaycasterClassOffset(raycasterClassOffset)
{
  mSupportsComponentRemoval = false;
}

uint RaycasterMetaComposition::GetComponentCount(HandleParam owner)
{
  Raycaster* raycaster = GetRaycaster(owner);
  return raycaster->mProviders.Size();
}

Handle RaycasterMetaComposition::GetComponentAt(HandleParam owner, uint index)
{
  Raycaster* raycaster = GetRaycaster(owner);
  RaycastProvider* provider = raycaster->mProviders[index];
  return provider;
}

Handle RaycasterMetaComposition::GetComponent(HandleParam owner, BoundType* componentType)
{
  Raycaster* raycaster = GetRaycaster(owner);
  forRange(RaycastProvider* provider, raycaster->mProviders.All())
  {
    if(ZilchVirtualTypeId(provider)->IsA(componentType))
      return provider;
  }
  return Handle();
}

Raycaster* RaycasterMetaComposition::GetRaycaster(HandleParam instance)
{
  byte* instanceData = instance.Dereference();
  return (Raycaster*)(((byte*)instanceData) + mRaycasterClassOffset);
}

}//namespace Zero
