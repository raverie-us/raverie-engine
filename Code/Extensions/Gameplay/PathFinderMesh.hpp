// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace Events
{
DeclareEvent(PathFinderMeshFinished);
}

struct NavMeshPolygon;
struct NavMeshEdge;

/// The key used in the path-finding algorithm.
struct NavMeshNodeKey
{
  NavMeshPolygon* mPolygon;
  /// The edge we came through to enter the current polygon.
  /// mEdge->mPolygon == mPolygon
  NavMeshEdge* mEdge;
};

typedef u32 NavMeshEdgeId;
typedef u32 NavMeshPolygonId;

const NavMeshPolygonId cInvalidMeshId = (u32)-1;

class PathFinderAlgorithmMesh;

// Nav Mesh Edge
/// This edge is a half edge, meaning adjacent polygons each have their own
/// edge.
struct NavMeshEdge
{
  /// Constructor.
  NavMeshEdge(NavMeshPolygon* owner);

  /// Range of all connected triangles.
  struct PolygonRange
  {
    PolygonRange();
    PolygonRange(NavMeshEdge* edge, NavMeshPolygon* ignore);

    inline bool Empty() const;
    inline void PopFront();
    inline NavMeshPolygon* Front();
    inline void FindNextValid();
    inline PolygonRange& All();

    NavMeshEdge* mBegin;
    NavMeshEdge* mEnd;
    NavMeshPolygon* mIgnore;
  };

  /// Returns all triangles connected to this edge, except the given triangle to
  /// ignore.
  PolygonRange AllConnectedTriangles(NavMeshPolygon* ignore = nullptr);

  /// The user added cost to traversing this edge. This can be used to simulate
  /// areas that you would like a path to avoid (but can still go through if it
  /// must).
  float mCost;

  /// The index to the tail vertex of this edge. Edges are sorted in
  /// counter-clockwise order, so the tail vertex is on the clockwise side of
  /// the edge. To get the direction of the edge, you could write something like
  /// this:
  ///   'mNextEdge->mTailVertex' - 'mTailVertex'
  /// Note that these are indices and you would have to access the vertex buffer
  /// first.
  u32 mTailVertex;

  /// The owning polygon of this edge.
  NavMeshPolygon* mPolygon;

  /// A linked list of edges surrounding our owning polygon.
  Link<NavMeshEdge> mEdgeLink;

  /// A linked list of the sibling edges that are connected to other polygons.
  NavMeshEdge* mNextConnected;
  NavMeshEdge* mPreviousConnected;
};

// Nav Mesh Polygon
struct NavMeshPolygon
{
  typedef InList<NavMeshEdge, &NavMeshEdge::mEdgeLink> EdgeList;
  typedef EdgeList::range EdgeRange;

  Vec3 GetCenter(PathFinderAlgorithmMesh* mesh);

  /// Range of all adjacent triangles.
  struct PolygonRange
  {
    PolygonRange()
    {
    }
    PolygonRange(NavMeshPolygon* polygon);

    inline bool Empty();
    inline void PopFront();
    inline NavMeshEdge* CurrentEdge();
    inline NavMeshPolygon* Front();
    inline PolygonRange& All();
    inline void FindNextValid();
    inline float GetFrontCost();

    /// The triangle we're finding neighbors for.
    NavMeshPolygon* mPolygon;

    /// Our triangles current edge.
    EdgeRange mCurrentEdge;

    /// The current edges connected triangles.
    NavMeshEdge::PolygonRange mCurrentEdgesPolygons;
  };

  EdgeRange AllEdges();
  PolygonRange AllNeighboringPolygons();

  /// If this cell is non-traversable (paths may not move through it).
  bool mCollision;

  /// The user added cost to traversing a node. This can be used to simulate
  /// areas that you would like a path to avoid (but can still go through if it
  /// must).
  float mCost;

  /// A list of all the edges for this polygon.
  EdgeList mEdges;

  u32 mId;
};

// Finder Mesh Node Range
class PathFinderMeshNodeRange
{
public:
  PathFinderMeshNodeRange(NavMeshPolygon* polygon);

  // Range Interface
  typedef Pair<NavMeshPolygonId, float> FrontResult;
  inline bool Empty();
  inline void PopFront();
  inline FrontResult Front();
  inline PathFinderMeshNodeRange& All();

  // Returns true if it pops, or false if the cell is valid
  inline void PopUntilValid();

  NavMeshPolygon::PolygonRange mRange;
};

// Finder Algorithm Mesh
class PathFinderAlgorithmMesh
    : public PathFinderAlgorithm<PathFinderAlgorithmMesh, NavMeshPolygonId, PathFinderMeshNodeRange>
{
public:
  PathFinderAlgorithmMesh();

  /// PathFinderAlgorithm Interface
  PathFinderMeshNodeRange QueryNeighbors(NavMeshPolygonId polygonId);
  bool QueryIsValid(NavMeshPolygonId polygonId);
  float QueryHeuristic(NavMeshPolygonId start, NavMeshPolygonId goal);

  /// Returns the index of the newly created position.
  u32 AddVertex(Vec3Param pos);

  /// Creates an empty polygon and returns an id for edges to be added at a
  /// later time.
  NavMeshPolygonId AddPolygon();
  NavMeshPolygonId AddPolygon(Array<u32>& vertices);
  NavMeshPolygonId AddPolygon(ArrayClass<u32>& vertices);
  NavMeshPolygonId AddPolygon(u32 vertex0, u32 vertex1, u32 vertex2);
  NavMeshPolygonId AddPolygon(u32 vertex0, u32 vertex1, u32 vertex2, u32 vertex3);

  /// Adds an edge to the polygon with the given id.
  NavMeshEdgeId AddEdgeToPolygon(NavMeshPolygonId polygonId, u32 vertex0, u32 vertex1);

  /// A higher cost of a polygon makes the A* algorithm less likely to traverse
  /// that polygon.
  void SetPolygonCost(NavMeshPolygonId polygonId, float cost);

  /// The client data will be included in the final path.
  void SetPolygonClientData(NavMeshPolygonId polygonId, Cog* clientData);

  /// A higher cost of a polygon makes the A* algorithm less likely to traverse
  /// that polygon.
  void SetEdgeCost(NavMeshEdgeId edgeId, float cost);

  /// The client data will be included in the final path.
  void SetEdgeClientData(NavMeshEdgeId edgeId, Cog* clientData);

  /// Clear the grid of all collision and costs.
  void Clear();

  // Internals
  NavMeshPolygon* GetPolygon(NavMeshPolygonId id);
  NavMeshEdge* GetEdge(NavMeshEdgeId id);

  NavMeshPolygonId GetNextPolygonId();
  NavMeshEdgeId GetNextEdgeId();

  uint mCurrentPolygonId;
  uint mCurrentEdgeId;

  Array<Vec3> mVertices;
  HashMap<NavMeshEdgeId, NavMeshEdge*> mEdges;
  HashMap<NavMeshPolygonId, NavMeshPolygon*> mPolygons;

  HashMap<u64, NavMeshEdge*> mEdgeConnections;
  HashMap<NavMeshPolygon*, CogId> mPolygonClientData;
  HashMap<NavMeshEdge*, CogId> mEdgeClientData;
};

// Path Finder Mesh
/// A* pathfinding on a mesh.
class PathFinderMesh : public PathFinder
{
public:
  RaverieDeclareType(PathFinderMesh, TypeCopyMode::ReferenceType);

  PathFinderMesh();

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

  // NavMesh Interface
  /// Builds a
  void SetMesh(Mesh* graphicsMesh);
  void SetMesh(Mesh* graphicsMesh, float maxSlope);
  // void SetMesh(PhysicsMesh* mesh);

  /// Returns the index of the newly created position.
  u32 AddVertex(Vec3Param pos);

  /// Creates an empty polygon and returns an id for edges to be added at a
  /// later time.
  NavMeshPolygonId AddPolygon(Array<u32>& vertices);
  NavMeshPolygonId AddPolygon(ArrayClass<u32>& vertices);
  NavMeshPolygonId AddPolygon(u32 vertex0, u32 vertex1, u32 vertex2);
  NavMeshPolygonId AddPolygon(u32 vertex0, u32 vertex1, u32 vertex2, u32 vertex3);

  /// A higher cost of a polygon makes the A* algorithm less likely to traverse
  /// that polygon.
  void SetPolygonCost(NavMeshPolygonId polygonId, float cost);

  /// The client data will be included in the final path.
  void SetPolygonClientData(NavMeshPolygonId polygonId, Cog* clientData);

  /// A higher cost of a polygon makes the A* algorithm less likely to traverse
  /// that polygon.
  void SetEdgeCost(NavMeshEdgeId edgeId, float cost);

  /// The client data will be included in the final path.
  void SetEdgeClientData(NavMeshEdgeId edgeId, Cog* clientData);

  /// Clear the grid of all collision and costs.
  void Clear();

  /// Finds a path between cell indices (or returns an empty array if no path
  /// could be found).
  HandleOf<ArrayClass<Vec3>> FindPath(NavMeshPolygonId start, NavMeshPolygonId goal);
  using RaverieBase::FindPath;

  /// Finds a path on another thread between cell indices.
  /// When the thread is completed, the events PathFinderGridCompleted or
  /// PathFinderGridFailed will be sent on both the returned PathFinderRequest
  /// and on the Cog that owns this component (on this.Owner).
  HandleOf<PathFinderRequest> FindPathThreaded(NavMeshPolygonId start, NavMeshPolygonId goal);
  using RaverieBase::FindPathThreaded;

  /// Returns the triangle closest to the given world position.
  NavMeshPolygonId WorldPositionToPolygon(Vec3Param worldPosition);

  /// Returns the triangle closest to the given local position.
  NavMeshPolygonId LocalPositionToPolygon(Vec3Param localPosition);

  Vec3 PolygonToWorldPosition(NavMeshPolygonId polygonId);

  // Internals
  Transform* mTransform;
  CopyOnWriteHandle<PathFinderAlgorithmMesh> mMesh;
};

} // namespace Raverie
