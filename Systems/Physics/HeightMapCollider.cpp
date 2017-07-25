///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(HeightMapCollider, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Collider);
  ZeroBindDependency(HeightMap);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(Thickness);
  ZilchBindMethod(ClearCachedEdgeAdjacency);
}

HeightMapCollider::HeightMapCollider()
{
  mType = cHeightMap;
  mLocalAabb.Zero();
}

void HeightMapCollider::Serialize(Serializer& stream)
{
  Collider::Serialize(stream);
  SerializeNameDefault(mThickness, 1.0f);
}

void HeightMapCollider::Initialize(CogInitializer& initializer)
{
  Collider::Initialize(initializer);

  Cog* owner = GetOwner();
  mMap = owner->has(HeightMap);
  ConnectThisTo(owner, Events::HeightMapPatchAdded, OnHeightMapPatchAdded);
  ConnectThisTo(owner, Events::HeightMapPatchRemoved, OnHeightMapPatchRemoved);
  ConnectThisTo(owner, Events::HeightMapPatchModified, OnHeightMapPatchModified);

  // Load all patches into memory
  ReloadAllPatches();

  // If we're in a game space then generate edge info to avoid
  // edge catching (don't do this in the editor)
  if(initializer.mSpace->IsEditorOrPreviewMode() == false)
    GenerateInternalEdgeInfo(this, &mInfoMap);
}

void HeightMapCollider::DebugDraw()
{
  // Draw all patch aabbs in world space)
  //CacheWorldValues();
  //PatchAabbMap::range range = mPatchAabbs.All();
  //for(; !range.Empty(); range.PopFront())
  //{
  //  Aabb localAabb = range.Front().second;
  //  Aabb worldAabb = localAabb.TransformAabb(GetWorldTransform()->GetWorldMatrix());
  //
  //  gDebugDraw->Add(Debug::Obb(worldAabb));
  //}
}

void HeightMapCollider::CacheWorldValues()
{
  PatchAabbMap::range range = mPatchAabbs.All();
  // If we have patches then combine all of their aabb's together
  if(!range.Empty())
  {
    mLocalAabb.SetInvalid();
    for(; !range.Empty(); range.PopFront())
      mLocalAabb.Combine(range.Front().second);
  }
  // Otherwise just set our local aabb to
  // one centered at our translation with no size
  else
    mLocalAabb.Set(GetWorldTranslation());
}

void HeightMapCollider::ComputeWorldAabbInternal()
{
  // Transform the local aabb into world space using a proper transform
  // when the aabb isn't centered at the origin in local space
  Mat4 worldMatrix = GetWorldTransform()->GetWorldMatrix();
  mAabb = mLocalAabb.TransformAabb(worldMatrix);
}

real HeightMapCollider::ComputeWorldVolumeInternal()
{
  return real(0);
}

void HeightMapCollider::ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia)
{
  // Height maps can't be dynamic
  localInvInertia.ZeroOut();
}

void HeightMapCollider::RebuildModifiedResources()
{
  Collider::RebuildModifiedResources();
}

real HeightMapCollider::GetThickness() const
{
  return mThickness;
}

void HeightMapCollider::SetThickness(real thickness)
{
  // To avoid numerical issues clamp the max bounds of the thickness
  mThickness = Math::Clamp(thickness, real(0.1f), real(10.0f));

  // Since the thickness changed we have to reload all patches 
  // so that we can update the world bounding volumes
  ReloadAllPatches();
}

void HeightMapCollider::ClearCachedEdgeAdjacency()
{
  mInfoMap.Clear();
}

HeightMapCollider::HeightMapRangeWrapper::HeightMapRangeWrapper(HeightMap* map, Aabb& aabb, real thickness)
{
  mRange.SetLocal(map, aabb, thickness);
}

void HeightMapCollider::HeightMapRangeWrapper::PopFront()
{
  mRange.PopFront();
}

HeightMapCollider::HeightMapRangeWrapper::InternalObject& HeightMapCollider::HeightMapRangeWrapper::Front()
{
  HeightMapAabbRange::TriangleInfo item = mRange.Front();
  AbsoluteIndex absIndex = mRange.mMap->GetAbsoluteIndex(item.mPatchIndex,item.mCellIndex);
  uint triIndex = mRange.mTriangleIndex;

  // Convert the triangle's info into a unique 32-bit key (change the key later to be bigger?)
  uint key;
  HeightMapCollider::TriangleIndexToKey(absIndex, triIndex, key);
  mObj.Index = key;
  mObj.Shape.BaseTri = item.mLocalTri;

  return mObj;
}

bool HeightMapCollider::HeightMapRangeWrapper::Empty()
{
  return mRange.Empty();
}

Triangle HeightMapCollider::GetTriangle(uint key)
{
  // Convert the key back into the height map's absolute and triangle index
  uint triIndex;
  AbsoluteIndex absIndex;
  HeightMapCollider::KeyToTriangleIndex(key, absIndex, triIndex);

  // Fetch the quad from the height map
  uint count;
  Triangle triangles[2];
  mMap->GetQuadAtIndex(absIndex, triangles, count);

  // Have to restructure query if we want to eliminate this garbage return
  if(!count || (triIndex && count == 1))
  {
    Error("We didn't find a cell in the height map (it's possibly too big!)");
    return Triangle();
  }

  return triangles[triIndex];
}

HeightMapCollider::HeightMapRangeWrapper HeightMapCollider::GetOverlapRange(Aabb& localAabb)
{
  HeightMapRangeWrapper range(mMap, localAabb, mThickness);
  // This only needs to be set once and it will persist through all objects in the range
  range.mObj.Shape.ScaledDir = HeightMap::UpVector * -mThickness;
  return range;
}

bool HeightMapCollider::Cast(const Ray& localRay, ProxyResult& result, BaseCastFilter& filter)
{
  // Cast a local ray (already transformed by the collision manager) against the internal height map
  HeightMapRayRange range = mMap->CastLocalRay(localRay);
  // If the height map didn't get any results then stop
  if(range.Empty())
    return false;

  // Otherwise we got a triangle that we actually hit
  HeightMapRayRange::TriangleInfo& triInfo = range.Front();
  Triangle& tri = triInfo.mLocalTri;

  // Copy the intersection info into the cast result
  Intersection::IntersectionPoint& point = triInfo.mIntersectionInfo;
  result.mPoints[0] = point.Points[0];
  result.mPoints[1] = point.Points[1];
  result.mDistance = point.T;

  // Unfortunately, the intersection info does not contain the normal
  // so we have to compute that ourself if it's requested
  if(filter.IsSet(BaseCastFilterFlags::GetContactNormal))
  {
    Vec3 normal = Geometry::NormalFromPointOnTriangle(result.mPoints[0], tri[0], tri[1], tri[2]);

    // Since the normal only comes from the point on the object it will always be the positive normal 
    // of the triangle. We want the normal to be the "reflection normal" from the ray though. To deal
    // with this simply negate the normal if it doesn't point towards the ray's start. (could replace with rayDir?)
    if(Dot(normal, tri[0] - localRay.Start) > 0)
      normal *= real(-1.0f);

    result.mContactNormal = normal;
  }

  // Compute this triangle's key so we can look it back up if needed
  AbsoluteIndex absIndex = mMap->GetAbsoluteIndex(range.mPatchRange.Front(), range.mCellRange.Front());
  HeightMapCollider::TriangleIndexToKey(absIndex, range.mTriangleIndex, result.ShapeIndex);

  return true;
}

HeightMap* HeightMapCollider::GetHeightMap()
{
  return mMap;
}

TriangleInfoMap* HeightMapCollider::GetInfoMap()
{
  return &mInfoMap;
}

void HeightMapCollider::TriangleIndexToKey(const AbsoluteIndex& absolueIndex, uint triIndex, uint& key)
{
  // set the top 16 bits to the y index and the bottom
  // 16 to the x axis (times 2 because of 2 triangles)
  int mask = 0xffff;
  key = 0;
  key |= (((absolueIndex.x + 0x3fff) << 1) + triIndex) & mask;
  key |= ((absolueIndex.y + 0x7fff) & mask) << 16;
}

void HeightMapCollider::KeyToTriangleIndex(uint key, AbsoluteIndex& absolueIndex, uint& triIndex)
{
  // Reconstruct indices
  int mask = 0xffff;
  int x = ((key & mask) >> 1) - 0x3fff;
  int y = (key >> 16) - 0x7fff;
  triIndex = key & 0x1;
  absolueIndex = AbsoluteIndex(x, y);
}

void HeightMapCollider::LoadPatch(HeightMap* map, HeightPatch* mapPatch)
{
  // When loading a patch we don't actually need any triangle info. We do however
  // need to compute the local space aabb so we can properly broad and narrow-phase.
  // Instead of finding this from triangles or vertices we can directly compute a patch's
  // aabb from the patch size and the stored min/max value for the patch.
  Aabb& patchAabb = mPatchAabbs[mapPatch->Index];
  patchAabb = map->GetPatchLocalAabb(mapPatch);

  // Extend the aabb on the bottom by the thickness
  patchAabb.mMin -= HeightMap::UpVector * mThickness;

  // Updating common size info (e.g. bounding volumes) should be handled by
  // the caller since they could be calling this multiple times
}

void HeightMapCollider::ReloadAllPatches()
{
  // To reload all patches we simply walk over and call load on each
  // patch (this only updates bounding volume info)
  PatchMap::valuerange range = mMap->GetAllPatches();
  for(; !range.Empty(); range.PopFront())
  {
    if(range.Front() != nullptr)
      LoadPatch(mMap, range.Front());
  }
  // Since our internal size changed make sure to run all common update code
  InternalSizeChanged();
}

void HeightMapCollider::OnHeightMapPatchAdded(HeightMapEvent* hEvent)
{
  // Create a new patch aabb and load/compute the patch's local space aabb
  mPatchAabbs.Insert(hEvent->Patch->Index, Aabb());
  LoadPatch(hEvent->Map, hEvent->Patch);

  // Since our internal size changed make sure to run all common update code
  InternalSizeChanged();
}

void HeightMapCollider::OnHeightMapPatchRemoved(HeightMapEvent* hEvent)
{
  // Remove a patch aabb
  mPatchAabbs.Erase(hEvent->Patch->Index);

  // Since our internal size changed make sure to run all common update code
  InternalSizeChanged();
}

void HeightMapCollider::OnHeightMapPatchModified(HeightMapEvent* hEvent)
{
  // One of the patches were modified. For simplicity just reload the patch.
  LoadPatch(hEvent->Map, hEvent->Patch);

  // Since our internal size changed make sure to run all common update code
  InternalSizeChanged();
}

}//namespace Zero
