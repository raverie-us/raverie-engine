// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace Events
{
DeclareEvent(PathFinderGridFinished);
}

// PathFinderAlgorithmGrid
class PathFinderAlgorithmGrid;

class PathFinderGridNodeRange
{
public:
  PathFinderGridNodeRange(PathFinderAlgorithmGrid* grid, IntVec3Param center);

  // Range Interface
  typedef Pair<IntVec3, float> FrontResult;
  inline bool Empty() const;
  inline void PopFront();
  inline Pair<IntVec3, float> Front() const;
  inline PathFinderGridNodeRange& All();

  // Internals

  // Returns true if it pops, or false if the cell is valid
  inline void PopUntilValid();

  PathFinderAlgorithmGrid* mGrid;
  IntVec3 mCenter;
  float mCurrentCost;
  IntVec3 mCurrentIntVec3;
  int mIndex;
};

/// A cell in the grid that contains the cost and collision information.
/// A cell only exists if it has either a non-zero cost or collision.
class PathFinderCell
{
public:
  RaverieDeclareType(PathFinderCell, TypeCopyMode::ReferenceType);

  PathFinderCell();

  /// The user added cost to traversing a node. This can be used to simulate
  /// areas that you would like a path to avoid (but can still go through if it
  /// must).
  float mCost;

  /// If this cell is non-traversable (paths may not move through it).
  bool mCollision;
};

class PathFinderAlgorithmGrid : public PathFinderAlgorithm<PathFinderAlgorithmGrid, IntVec3, PathFinderGridNodeRange>
{
public:
  PathFinderAlgorithmGrid();

  // PathFinderAlgorithm Interface
  PathFinderGridNodeRange QueryNeighbors(IntVec3Param node);
  bool QueryIsValid(IntVec3Param node);
  float QueryHeuristic(IntVec3Param node, IntVec3Param goal);

  /// If there is collision at a cell then the A* algorithm cannot traverse that
  /// cell.
  void SetCollision(IntVec3Param index, bool collision);
  bool GetCollision(IntVec3Param index);

  /// A higher cost of a cell makes the A* algorithm less likely to traverse
  /// that cell. The default cost of any cell is 1.0.
  void SetCost(IntVec3Param index, float cost);
  float GetCost(IntVec3Param index);

  /// Clear the grid of all collision and costs.
  void Clear();

  /// Whether the A* path can move diagonally or only on the cardinal axes.
  bool mDiagonalMovement;

  // Internals
  HashMap<IntVec3, PathFinderCell> mCells;
};

// PathFinderGrid
/// A* pathfinding on a grid. The grid supports cardinal or diagonal movement.
/// Collision and costs can be set for each tile to make path-finding avoid
/// cells. Sends the PathFinderGridFinished event on itself when a threaded
/// request completes.
class PathFinderGrid : public PathFinder
{
public:
  RaverieDeclareType(PathFinderGrid, TypeCopyMode::ReferenceType);

  PathFinderGrid();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void DebugDraw() override;

  // PathFinder Interface
  Variant WorldPositionToNodeKey(Vec3Param worldPosition) override;
  Vec3 NodeKeyToWorldPosition(VariantParam nodeKey) override;
  void FindPathGeneric(VariantParam start, VariantParam goal, Array<Variant>& pathOut) override;
  HandleOf<PathFinderRequest> FindPathGenericThreaded(VariantParam start, VariantParam goal) override;
  StringParam GetCustomEventName() override;

  // PathFinderGrid Interface
  /// If there is collision at a cell then the A* algorithm cannot traverse that
  /// cell.
  void SetCollision(IntVec3Param index, bool collision);
  bool GetCollision(IntVec3Param index);

  /// A higher cost of a cell makes the A* algorithm less likely to traverse
  /// that cell.
  void SetCost(IntVec3Param index, float cost);
  float GetCost(IntVec3Param index);

  /// Clear the grid of all collision and costs.
  void Clear();

  /// Finds a path between cell indices (or returns an empty array if no path
  /// could be found).
  HandleOf<ArrayClass<IntVec3>> FindPath(IntVec3Param start, IntVec3Param goal);

  /// Finds a path between world positions (or returns an empty array if no path
  /// could be found).
  HandleOf<ArrayClass<Vec3>> FindPath(Vec3Param worldStart, Vec3Param worldGoal);

  /// Finds a path on another thread between cell indices.
  /// When the thread is completed, the events PathFinderGridCompleted or
  /// PathFinderGridFailed will be sent on both the returned PathFinderRequest
  /// and on the Cog that owns this component (on this.Owner).
  HandleOf<PathFinderRequest> FindPathThreaded(IntVec3Param start, IntVec3Param goal);

  /// Finds a path on another thread between the closest nodes to the given
  /// world positions. When the thread is completed, the events
  /// PathFinderGridCompleted or PathFinderGridFailed will be sent on both the
  /// returned PathFinderRequest and on the Cog that owns this component (on
  /// this.Owner).
  HandleOf<PathFinderRequest> FindPathThreaded(Vec3Param worldStart, Vec3Param worldGoal);

  /// The size of the cell in local space units.
  /// If the PathFinderGrid has no parent, or the parent's transform has
  /// no scale then this will be the same as the world cell size.
  void SetCellSize(Vec3Param size);
  Vec3Param GetCellSize();

  /// Whether the A* path can move diagonally or only on the cardinal axes.
  void SetDiagonalMovement(bool value);
  bool GetDiagonalMovement();

  /// Returns the cell that the world position occupies.
  IntVec3 WorldPositionToCellIndex(Vec3Param worldPosition);

  /// Returns the cell that the local position occupies.
  IntVec3 LocalPositionToCellIndex(Vec3Param localPosition);

  /// Returns the center position of the cell in world space.
  Vec3 CellIndexToWorldPosition(IntVec3Param index);

  /// Returns the center position of the cell in local space.
  Vec3 CellIndexToLocalPosition(IntVec3Param index);

  // Internals
  Transform* mTransform;
  CopyOnWriteHandle<PathFinderAlgorithmGrid> mGrid;
  Vec3 mLocalCellSize;
};

} // namespace Raverie
