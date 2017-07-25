///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct ProxyResult;
struct BaseCastFilter;

/// Defines collision for a height map.
class HeightMapCollider : public Collider
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  HeightMapCollider();

  // Component interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void DebugDraw() override;

  // Collider Interface
  void CacheWorldValues() override;
  void ComputeWorldAabbInternal() override;
  real ComputeWorldVolumeInternal() override;
  void ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia) override;
  void RebuildModifiedResources() override;

  /// How thick the surface of the height map is. Used to avoid tunneling problems.
  real GetThickness() const;
  void SetThickness(real thickness);

  /// Clear the cached information used to avoid catching edges. Typically called
  /// internally by physics, but is exposed for manual triggering.
  void ClearCachedEdgeAdjacency();

  //-------------------------------------------------------------------Internal
  
  /// A range for returning the local-space triangles that need to have collision checked.
  /// All triangles returned intersect the passed in local space aabb. The work horse is
  /// the internal HeightMapAabbRange, this just wraps that for intersection and
  /// computes a unique id for each triangles.
  struct HeightMapRangeWrapper
  {
    /// Represents a unique (swept) triangle int he height map
    struct InternalObject
    {
      int Index;
      SweptTriangle Shape;
    };

    HeightMapRangeWrapper(HeightMap* map, Aabb& aabb, real thickness);

    // Range Interface
    void PopFront();
    InternalObject& Front();
    bool Empty();

    HeightMapAabbRange mRange;
    InternalObject mObj;
  };

  /// Returns the triangle associated with the given key (the key should come from our own range).
  Triangle GetTriangle(uint key);

  /// Used to tell the collision system that this collider stores information in local space.
  /// This means that the passed in aabb for GetOverlapRange should be transformed to local space.
  typedef true_type RangeInLocalSpace;
  /// Used in the collision system. @JoshD: Maybe replace with AutoDeclare later?
  typedef HeightMapRangeWrapper RangeType;
  /// Returns a range of local-space triangles that overlap the passed in local-space aabb.
  HeightMapRangeWrapper GetOverlapRange(Aabb& localAabb);

  /// This is a specialization of Ray vs. HeightMap that goes through the internal mid-phase with an
  /// optimized ray-casting algorithm instead of the generic GetOverlapAabb function.
  /// Note: the ray here is expected to be in this cog's local space.
  bool Cast(const Ray& localRay, ProxyResult& result, BaseCastFilter& filter);

  HeightMap* GetHeightMap();
  TriangleInfoMap* GetInfoMap();

  // Helpers to transform back and forth between they height map's key and the cast id
  static void TriangleIndexToKey(const AbsoluteIndex& absolueIndex, uint triIndex, uint& key);
  static void KeyToTriangleIndex(uint key, AbsoluteIndex& absolueIndex, uint& triIndex);

private:
  typedef HashMap<PatchIndex, Aabb> PatchAabbMap;

  void LoadPatch(HeightMap* map, HeightPatch* mapPatch);
  void ReloadAllPatches();
  void OnHeightMapPatchAdded(HeightMapEvent* hEvent);
  void OnHeightMapPatchRemoved(HeightMapEvent* hEvent);
  void OnHeightMapPatchModified(HeightMapEvent* hEvent);

  real mThickness;
  Aabb mLocalAabb;
  PatchAabbMap mPatchAabbs;

  HeightMap* mMap;
  TriangleInfoMap mInfoMap;
};

}//namespace Zero
