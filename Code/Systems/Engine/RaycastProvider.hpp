// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class Space;
class Viewport;

/// An individual entry from raycasting.
struct RayCastEntry
{
  RayCastEntry();
  bool operator>(RayCastEntry& rhs);
  Cog* HitCog;
  Handle Instance;
  Vec3 HitWorldPosition;
  Vec3 HitWorldNormal;

  // The UV may not be provided by all providers
  Vec2 HitUv;
  bool HitUvProvided;
  float T;
};

/// Class that stores and sorts the results from raycasts (any kind of cast).
/// This list will keep track of n items up to the capacity.
/// This also handles merging two lists together after.
class RaycastResultList
{
public:
  typedef Array<RayCastEntry> RayCastEntries;

  RaycastResultList(size_t capacity = 1);

  /// Change the maximum number of items that can be held.
  void SetCapacity(size_t capacity);
  /// Add an item to be sorted.
  void AddMetaItem(HandleParam object,
                   float t,
                   Vec3Param worldPosition,
                   Vec3Param worldNormal,
                   Vec2Param uv = Vec2::cZero,
                   bool uvProvided = false);
  void AddItem(Cog* hitCog,
               float t,
               Vec3Param worldPosition,
               Vec3Param worldNormal,
               Vec2Param uv = Vec2::cZero,
               bool uvProvided = false);
  void AddItem(RayCastEntry& entry);
  /// Merge the passed in list with this list.
  void AddList(RaycastResultList& list);

  size_t mCapacity;
  size_t mSize;
  RayCastEntries mEntries;
};

/// Info to be passed around for casting.
class CastInfo
{
public:
  /// Constructor for a raycast (1 point in screen space)
  CastInfo(Space* targetSpace, Cog* cameraCog, Vec2Param mousePosition);
  /// Constructor for a frustum cast (2 points in screen space)
  CastInfo(Space* targetSpace, Cog* cameraCog, Vec2Param dragStart, Vec2Param dragEnd);

  /// Helper for construction
  void SetInfo(Space* targetSpace, Cog* cameraCog, Vec2Param dragStart, Vec2Param dragEnd);

  /// The space that is being casted in.
  Space* mTargetSpace;
  /// The camera for the viewport through which casting is happening.
  Cog* mCameraCog;
  /// The point clicked for a raycast. For a frustum cast this is the drag end
  /// point.
  Vec2 mMouseCurrent;
  /// The point that a frustum cast drag started at.
  /// Will be set to mMouseCurrent when this is a raycast.
  Vec2 mMouseDragStart;
};

/// Base class for providing an interface for editor raycasting.
/// This interface only supports raycasting and frustum casting at the moment.
class RaycastProvider : public SafeId32Object
{
public:
  RaverieDeclareType(RaycastProvider, TypeCopyMode::ReferenceType);

  RaycastProvider()
  {
    mActive = true;
  }
  virtual ~RaycastProvider(){};

  // Serialization
  void Serialize(Serializer& stream) override;

  /// Fills out the hit items from a raycast into the result list.
  /// The list should be set to the desired capacity beforehand.
  virtual void RayCast(Ray& ray, CastInfo& castInfo, RaycastResultList& results) = 0;
  /// Fills out the hit items from a frustum into the result list.
  /// The list should be set to the desired capacity beforehand.
  virtual void FrustumCast(Frustum& frustum, CastInfo& castInfo, RaycastResultList& results) = 0;

  /// Whether or not this provider is active. Used to fully disable the
  /// provider.
  bool mActive;
};

/// Stores all of the providers and manages casting and merging through all of
/// them.
class Raycaster : public SafeId32
{
public:
  RaverieDeclareType(Raycaster, TypeCopyMode::ReferenceType);
  ~Raycaster();

  void Serialize(Serializer& stream);
  void SerializeProviders(Serializer& stream);

  /// Adds a new provider (that is owned by this class) to cast with during
  /// editor casts.
  void AddProvider(RaycastProvider* provider);

  /// Performs a raycast through all providers.
  /// The list should be set to the desired capacity beforehand.
  void RayCast(Ray& ray, CastInfo& castInfo, RaycastResultList& results);
  /// Performs a frustum cast through all providers.
  /// The list should be set to the desired capacity beforehand.
  void FrustumCast(Frustum& frustum, CastInfo& castInfo, RaycastResultList& results);

  typedef Array<RaycastProvider*> ProviderArray;
  ProviderArray mProviders;
};

class RaycasterMetaComposition : public MetaComposition
{
public:
  RaverieDeclareType(RaycasterMetaComposition, TypeCopyMode::ReferenceType);

  RaycasterMetaComposition();

  uint GetComponentCount(HandleParam owner) override;
  Handle GetComponentAt(HandleParam owner, uint index) override;
  Handle GetComponent(HandleParam owner, BoundType* componentType) override;
};

} // namespace Raverie
