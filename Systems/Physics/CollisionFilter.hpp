///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareBitField6(FilterFlags, SkipResolution, SkipDetectingCollision,
                 StartEvent, EndEvent, PersistedEvent, PreSolveEvent);

/// Controls what parts of collision detection/resolution are run for a CollisionGroup pair.
/// <param name="SkipDetection">Don't run collision detection. No events will be sent.</param>
/// <param name="SkipResolution">Don't run collision resolution. Detection will still be run so events might be sent out.</param>
/// <param name="Resolve">Run both collision detection and resolution as normal.</param>
DeclareEnum3(CollisionFilterCollisionFlags, SkipDetection, SkipResolution, Resolve);

/// Returns a display string for the filter
String GroupFilterDisplay(CollisionFilter* filter);

/// A filter for storing the relationship between a pair of groups.
/// Stores flags for the kind of filter this is, as well as what events to
/// send out and to whom.
struct CollisionFilter : public SafeId32EventObject
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CollisionFilter() {}
  CollisionFilter(ResourceId first, ResourceId second);
  ~CollisionFilter();

  void SetPair(ResourceId first, ResourceId second);

  void Serialize(Serializer& stream);

  void SetDefaults();

  // Properties

  /// The collision state between the two types. Controls whether the
  /// types skip detection, skip resolution, or resolve as normal.
  CollisionFilterCollisionFlags::Enum GetCollisionFlag();
  void SetCollisionFlag(CollisionFilterCollisionFlags::Enum state);

  // Helper functions
  ResourceId first() const;
  ResourceId second() const;
  CollisionGroup* GetCollisionGroupA() const;
  CollisionGroup* GetCollisionGroupB() const;
  String GetTypeAName() const;
  String GetTypeBName() const;
  String GetTypeADisplayName() const;
  String GetTypeBDisplayName() const;

  CollisionFilter* Clone() const;

  // Composition interface
  uint GetSize() const;
  HandleOf<CollisionFilterBlock> GetBlockAt(uint index);
  HandleOf<CollisionFilterBlock> GetById(BoundType* typeId);
  void Add(const HandleOf<CollisionFilterBlock>& blockHandle, int index);
  bool Remove(const HandleOf<CollisionFilterBlock>& blockHandle);

  // Hashing functions
  size_t Hash() const;
  bool operator==(const CollisionFilter& rhs) const;

  typedef Pair<ResourceId,ResourceId> ResourcePair;
  /// The pair use for hashing.
  ResourcePair mPair;
  /// The flags for determining different behavior.
  BitField<FilterFlags::Enum> mFilterFlags;
  /// The group ids that are unsorted (for consistent ordering with the flags).
  ResourceId TypeA, TypeB;

  typedef Array<CollisionFilterBlock*> BlockArray;
  /// The CollisionFilterBlocks that we have. These are used to choose which
  /// collision group events we send. Note: this list is not sorted currently
  /// so a linear search is needed to find an item. However, mFilterFlags will
  /// have to corresponding bit set if the block type exists so a quick rejection can be done.
  BlockArray mBlocks;

public:
  CollisionTable* mTable;
};

// Hash policy for a pointer to collision filter
template<>
struct HashPolicy<CollisionFilter*>
{
  typedef CollisionFilter* type;
  inline size_t operator()(const type& value) const
  {
    return value->Hash();
  }

  inline bool Equal(const type& left, const type& right) const
  {
    return (*left) == (*right); 
  }
};
typedef typename HashPolicy<CollisionFilter*> CollisionFilterHashPolicy;

}//namespace Zero
