// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

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
  if (capacity == 0)
  {
    ErrorIf(capacity == 0, "Capacity cannot be set to 0. Setting capacity to 1.");
    capacity = 1;
  }

  // If we shrink our capacity then make sure to shrink the current size
  if (capacity < mCapacity)
    mSize = Math::Min(mSize, capacity);

  mCapacity = capacity;
  // Make sure that we can access up to the full capacity
  mEntries.Resize(capacity);
}

void RaycastResultList::AddItem(RayCastEntry& inputEntry)
{
  // If this is the first item then just insert it
  if (mSize == 0)
  {
    mEntries[0] = inputEntry;
    ++mSize;
    return;
  }

  // Otherwise find out what index to start at
  size_t startIndex = mSize;
  // If we're at the max size
  if (mSize == mCapacity)
  {
    // Then we start at the last item. If the time of intersection is after the
    // last item then we do not want to insert this item as it happens too late
    // in time.
    startIndex = mCapacity - 1;
    if (inputEntry.T >= mEntries[startIndex].T)
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
  while (startIndex > 0 && mEntries[startIndex - 1] > mEntries[startIndex])
  {
    Math::Swap(mEntries[startIndex - 1], mEntries[startIndex]);
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
  if (mSize == 0)
  {
    // Add items from the other list up to our max capacity
    for (size_t i = 0; i < mCapacity && i < list.mSize; ++i)
    {
      mEntries[i] = list.mEntries[i];
      ++mSize;
    }
    return;
  }
  // If the new list is empty then there's nothing to do
  if (list.mSize == 0)
    return;

  // Since multiple providers may point to the same object we need to filter out
  // multiple occurrences of the same item. To do this we just insert all of the
  // items from both list into a map and keep the smaller time if the item was
  // already there.

  // Add all the items from this list
  typedef HashMap<Handle, RayCastEntry> CastMap;
  CastMap map;
  for (size_t i = 0; i < mSize; ++i)
  {
    RayCastEntry& entry = mEntries[i];
    map.Insert(entry.Instance, entry);
  }

  // Add all of the items from the other list (keep the smallest time of an
  // item)
  for (uint i = 0; i < list.mSize; ++i)
  {
    RayCastEntry& entry = list.mEntries[i];

    CastMap::InsertResult result = map.InsertNoOverwrite(entry.Instance, entry);
    if (!result.mIsNewInsert)
    {
      RayCastEntry& existing = result.mValue->second;
      if (existing > entry)
        existing = entry;
    }
  }

  // Now the list contains all of the items with their smallest times.
  // Just add all of the items (which will automatically sort and store up to
  // capacity)
  mSize = 0;
  CastMap::range range = map.All();
  for (; !range.Empty(); range.PopFront())
  {
    CastMap::value_type& pair = range.Front();
    RayCastEntry& entry = pair.second;
    AddItem(entry);
  }
}

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

RaverieDefineType(RaycastProvider, builder, type)
{
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindFieldProperty(mActive);

  type->Add(new MetaSerialization());
}

void RaycastProvider::Serialize(Serializer& stream)
{
  SerializeNameDefault(mActive, true);
}

RaverieDefineType(Raycaster, builder, type)
{
  RaverieBindSetup(SetupMode::DefaultSerialization);

  // Set meta composition
  type->Add(new RaycasterMetaComposition());
}

Raycaster::~Raycaster()
{
  // We own all of the providers, so delete them
  for (uint i = 0; i < mProviders.Size(); ++i)
    delete mProviders[i];
  mProviders.Clear();
}

void Raycaster::Serialize(Serializer& stream)
{
  SerializeProviders(stream);
}

// If the given block requests to be setup via default serialization then run
// that
void SetupBlock(Handle& handle, BoundType* blockMeta)
{
  RaycastProvider* block = handle.Get<RaycastProvider*>();
  // This should probably be done with SFINAE at some point
  CogComponentMeta* metaComponent = blockMeta->HasInherited<CogComponentMeta>();
  if (metaComponent != nullptr)
  {
    SetupMode::Enum constructionMode = metaComponent->mSetupMode;
    if (constructionMode == SetupMode::DefaultSerialization)
    {
      DefaultSerializer defaultSerializer;
      block->Serialize(defaultSerializer);
    }
  }
}

Handle AllocateBlock(BoundType* blockMeta, bool runSetup)
{
  Handle handle = RaverieAllocate(RaycastProvider, blockMeta);

  // Run default serialization if necessary
  if (runSetup)
    SetupBlock(handle, blockMeta);

  return handle;
}

void Raycaster::SerializeProviders(Serializer& stream)
{
  // Polymorphic serialization for block types
  if (stream.GetMode() == SerializerMode::Saving)
  {
    // If we're saving, grab each block and serialize it
    ProviderArray::range range = mProviders.All();
    for (; !range.Empty(); range.PopFront())
    {
      RaycastProvider* provider = range.Front();
      MetaSerializeObject(provider, stream);
    }
  }
  else
  {
    // If we're loading things are a little more complicated
    PolymorphicNode node;
    while (stream.GetPolymorphic(node))
    {
      // For every node, create a block from it's typename and id
      BoundType* type = MetaDatabase::FindType(node.TypeName);
      if (type == nullptr)
      {
        Error("Type not found");
        continue;
      }

      RaycastProvider* provider = AllocateBlock(type, false).Get<RaycastProvider*>();
      if (provider)
      {
        // Serialize the block and then add it to our list
        MetaSerializeObject(provider, stream);
        mProviders.PushBack(provider);
      }
      stream.EndPolymorphic();
    }
  }
}

void Raycaster::AddProvider(RaycastProvider* provider)
{
  mProviders.PushBack(provider);
}

void Raycaster::RayCast(Ray& ray, CastInfo& castInfo, RaycastResultList& results)
{
  ray.Direction.Normalize();
  for (uint i = 0; i < mProviders.Size(); ++i)
  {
    RaycastProvider* provider = mProviders[i];
    // Skip inactive providers
    if (!provider->mActive)
      continue;

    RaycastResultList providerResults(results.mCapacity);
    provider->RayCast(ray, castInfo, providerResults);
    // Merge the new list into the total list
    results.AddList(providerResults);
  }
}

void Raycaster::FrustumCast(Frustum& frustum, CastInfo& castInfo, RaycastResultList& results)
{
  for (uint i = 0; i < mProviders.Size(); ++i)
  {
    RaycastProvider* provider = mProviders[i];
    // Skip inactive providers
    if (!provider->mActive)
      continue;

    RaycastResultList providerResults(results.mCapacity);
    provider->FrustumCast(frustum, castInfo, providerResults);
    // Merge the new list into the total list
    results.AddList(providerResults);
  }
}

RaverieDefineType(RaycasterMetaComposition, builder, type)
{
}

RaycasterMetaComposition::RaycasterMetaComposition() : MetaComposition(RaverieTypeId(RaycastProvider))
{
  mSupportsComponentAddition = false;
  mSupportsComponentRemoval = false;
}

uint RaycasterMetaComposition::GetComponentCount(HandleParam owner)
{
  Raycaster* raycaster = owner.Get<Raycaster*>();
  return raycaster->mProviders.Size();
}

Handle RaycasterMetaComposition::GetComponentAt(HandleParam owner, uint index)
{
  Raycaster* raycaster = owner.Get<Raycaster*>();
  RaycastProvider* provider = raycaster->mProviders[index];
  return provider;
}

Handle RaycasterMetaComposition::GetComponent(HandleParam owner, BoundType* componentType)
{
  Raycaster* raycaster = owner.Get<Raycaster*>();
  forRange (RaycastProvider* provider, raycaster->mProviders.All())
  {
    if (RaverieVirtualTypeId(provider)->IsA(componentType))
      return provider;
  }
  return Handle();
}

} // namespace Raverie
