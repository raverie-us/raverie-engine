#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(GraphicalSort);
}

// GraphicalEntryData is everything that GraphicalEntry needs to perform its
// operations. This data is aggregated by the Graphical implementations so that
// the size of GraphicalEntry can remain very small.
class GraphicalEntryData
{
public:
  // Need pointer back to graphical because GraphicalEntry points at this class
  Graphical* mGraphical;
  // Must be initialized to -1 every frame that this class is used
  // GraphicsSpace uses this to detect if a FrameNode has been created yet
  int mFrameNodeIndex;
  // Used to compute depth sort values
  Vec3 mPosition;
  // For Graphicals to identify sub-entries in extract calls if needed
  u64 mUtility;
};

// GraphicalEntry is used as a place holder for data that needs to be populated
// to render Graphicals. This class must stay very small so that sorting is as
// fast as possible. The final set of entries is used to populate data from the
// Graphicals in already sorted order.
/// Represents a Graphical that has been identified as visible from broadphase. Used for sorting.
class GraphicalEntry
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  bool operator<(const GraphicalEntry& other) const;

  /// The object that has the Graphical Component.
  Cog* GetCog();

  /// Sets the value that will be used to define draw order (lowest to highest).
  void SetGraphicalSortValue(s32 sortValue);

  // Set by the graphics engine.
  void SetRenderGroupSortValue(s32 sortValue);

  // Data that's needed for sorting and data extraction
  GraphicalEntryData* mData;
  // Used to account for all sorting requirements
  u64 mSort;
};

typedef Array<GraphicalEntry>::range GraphicalEntryRange;

/// Sent for RenderGroups that require custom logic for sort values.
class GraphicalSortEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Range of Graphicals that can have their sort values set.
  GraphicalEntryRange GetGraphicalEntries() { return mGraphicalEntries; }
  GraphicalEntryRange mGraphicalEntries;

  /// The RenderGroup that this event is being sent for.
  RenderGroup* GetRenderGroup() { return mRenderGroup; }
  RenderGroup* mRenderGroup;
};

s32 GetGraphicalSortValue(Graphical& graphical, GraphicalSortMethod::Enum sortMethod, Vec3 pos, Vec3 camPos, Vec3 camDir);

} // namespace Zero
