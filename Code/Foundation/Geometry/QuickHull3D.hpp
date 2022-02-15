// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

/// Helper class for an inlist to iterate over a range, skipping the sentinel
/// node if needed. Inlist's range cannot handle wrapping past the sentinel
/// node, but quick-hull needs this during horizon identification. This range
/// requires the original inlist to do this.
template <typename InListType>
struct InListWrappedRange
{
  typedef typename InListType::iterator Iterator;
  typedef typename InListType::sub_reference ResultType;

  InListWrappedRange()
  {
  }
  InListWrappedRange(InListType* list)
  {
    mList = list;
    mBegin = list->Begin();
    mEnd = mBegin;
    mPoppedOnce = list->Empty();
  }

  InListWrappedRange(InListType* list, Iterator& start)
  {
    mList = list;
    mBegin = start;
    mEnd = mBegin;

    mPoppedOnce = list->Empty();
  }

  bool Empty()
  {
    // We can't just check if begin == end since we
    // aren't starting at the sentinel node.
    return mBegin == mEnd && mPoppedOnce;
  }

  void PopFront()
  {
    mBegin = mList->NextWrap(mBegin);
    mPoppedOnce = true;
  }

  ResultType Front()
  {
    return **mBegin;
  }

  InListType* mList;
  Iterator mBegin;
  Iterator mEnd;
  bool mPoppedOnce;
};

/// Implementation of a 3D Quick-hull based upon Dirk Gregorius's GDC2014
/// presentation.
class QuickHull3D
{
public:
  class QuickHullVertex;
  class QuickHullEdge;
  class QuickHullFace;

  class QuickHullVertex
  {
  public:
    QuickHullVertex();

    Vec3 mPosition;
    real mConflictDistance;
    Link<QuickHullVertex> link;
  };
  class QuickHullEdge
  {
  public:
    QuickHullEdge();

    QuickHullEdge* mTwin;
    QuickHullVertex* mTail;
    QuickHullFace* mFace;

    Link<QuickHullEdge> link;
  };
  class QuickHullFace
  {
  public:
    QuickHullFace();

    // Compute the center and normal of this face.
    void RecomputeCenterAndNormal();
    // Is this face convex to the given face.
    bool IsConvexTo(QuickHullFace* other, real epsilon);
    // Count how many edges exist in this edge.
    size_t CountEdges();

    Vec3 mCenter;
    Vec3 mNormal;

    typedef InList<QuickHullEdge> EdgeList;
    typedef InList<QuickHullVertex> VertexList;
    EdgeList mEdges;
    VertexList mConflictList;
    Link<QuickHullFace> link;
  };
  typedef InList<QuickHullVertex> VertexList;
  typedef InList<QuickHullEdge> EdgeList;
  typedef InList<QuickHullFace> FaceList;

  QuickHull3D();
  ~QuickHull3D();

  /// Build a convex mesh from the given points. If the debug drawing
  /// stack is non-null then debug drawing information will be filled out.
  bool Build(const Array<Vec3>& points, DebugDrawStack* stack = nullptr);
  /// Clear all cached memory to start another quick-hull run.
  void Clear();

  /// Computes and returns how many vertices are in the final convex hull.
  size_t ComputeVertexCount();
  /// Computes and returns how many edges are in the final convex hull.
  size_t ComputeEdgeCount();
  /// Computes and returns how many half-edges are in the final convex hull.
  size_t ComputeHalfEdgeCount();
  /// Computes and returns how many faces are in the final convex hull.
  size_t ComputeFaceCount();

  /// A list of all faces in the final convex hull.
  FaceList::range GetFaces();

private:
  // Create the memory pools for this run of quickhull.
  void AllocatePools(const Array<Vec3>& points);
  void ComputeEpsilon(const Array<Vec3>& points);
  /// Converts the given points into a working format and performs
  /// vertex welding. Fills out how many points exist after welding.
  bool BuildDataSet(const Array<Vec3>& points, size_t& resultPointCount);
  void BuildDataSetNSquared(const Array<Vec3>& points, size_t& resultPointCount);
  void BuildDataSetGrid(const Array<Vec3>& points, size_t& resultPointCount);
  void BuildDataSetGridApproximation(const Array<Vec3>& points, size_t& resultPointCount);

  /// Computes a tetrahedron for the initial hull. If this fails the hulling is
  /// not possible.
  bool BuildInitialHull();
  /// Finds the two points furthest away on a cardinal axis
  void FindInitialSpan(QuickHullVertex*& v0, QuickHullVertex*& v1);
  /// Finds the vertex furthest away from the line segment defined by v0 and v1.
  QuickHullVertex* FindVertexFurthestFrom(QuickHullVertex* v0, QuickHullVertex* v1);
  /// Finds the vertex furthest away from the triangle defined by v0, v1, and
  /// v2.
  QuickHullVertex* FindVertexFurthestFrom(QuickHullVertex* v0, QuickHullVertex* v1, QuickHullVertex* v2);

  /// Partition each vertex to a conflict list on one of the initial faces.
  /// is management helps speed up the inner loop of quick-hull.
  void ComputeInitialConflictLists();
  /// Finds what faces the given vertex is closest to (on the positive side).
  QuickHullFace* FindClosestFace(QuickHullVertex* vertex);

  /// Finds the the vertex that is furthest away from it's conflict face.
  /// This allows us to do the "most work" at any given step.
  void FindNextConflictVertex(QuickHullVertex*& conflictVertex, QuickHullFace*& conflictFace);

  /// Adds the given vertex to the hull. This removes all faces that can see
  /// this vertex, creates the new faces, and merges coplanar faces, and fixes
  /// topological errors.
  void AddVertexToHull(QuickHullVertex* conflictVertex, QuickHullFace* conflictFace);
  /// Find the horizon boundary given a conflict vertex. The resultant edges
  /// will be in counter-clockwise order and are the edges belonging to the
  /// visible faces.
  void IdentifyHorizon(QuickHullVertex* conflictVertex,
                       QuickHullFace* conflictFace,
                       Array<QuickHullEdge*>& horizonEdges,
                       Array<QuickHullFace*>& internalFaces);
  /// Creates the new faces from the horizon edge to the conflict face.
  void CreateNewHorizonFaces(QuickHullVertex* conflictVertex,
                             Array<QuickHullEdge*>& horizon,
                             Array<QuickHullFace*>& newFaces);

  /// Partitions all of the conflict vertices on the given faces to new faces.
  void PartitionOldFaceConflictLists(Array<QuickHullFace*>& faces);
  /// Absorbs a conflict list from one face into the given face.
  void AbsorbConflictList(QuickHullFace* face, VertexList& conflictList);
  /// Absorbs a conflict vertex from another face into the given face.
  /// This sorts the vertex with all other conflict vertices.
  void AbsorbConflictVertex(QuickHullFace* face, QuickHullVertex* vertex);
  void RemoveOldHorizonFaces(Array<QuickHullFace*>& faces);

  /// Given the list of newly created faces, check all of the neighbors of these
  /// faces to see if they are not convex and if so merge them to make strictly
  /// convex faces.
  void MergeFaces(Array<QuickHullFace*>& newFaces);
  /// Finds if any face is not convex to the given face. If so the
  /// edge between them (beloging to the given face) is returned.
  QuickHullEdge* FindQuickHullMergeFace(QuickHullFace* face);
  /// Absorb the adjacent face into face across the provided shared edge and
  /// twin.
  void
  AbsorbFace(QuickHullFace* face, QuickHullFace* adjacentFace, QuickHullEdge* sharedEdge, QuickHullEdge* sharedTwin);
  /// Insert a range of edges into a face after a given edge. Returns the last
  /// edge where we inserted after.
  QuickHullEdge* InsertEdgeRangeAfter(QuickHullFace* face,
                                      QuickHullEdge* edgeToInsertAfter,
                                      EdgeList::range& edgesToInsert);

  /// Find if a topological invariant exists and if so fix it. This function
  /// must be iteratively called
  bool FixTopologicalInvariants(QuickHullFace* face);
  /// Finds if a topological invariant exists with the given face. Fills out the
  /// two offending edges if found.
  bool FindTopoligicalInvariant(QuickHullFace* face, QuickHullEdge*& e0, QuickHullEdge*& e1);
  /// Fixes a topological invariant where the neighboring face is a triangle.
  void FixTriangleTopoligicalInvariant(QuickHullFace* face, QuickHullEdge* e0, QuickHullEdge* e1);
  /// Fixes a topological invariant where the neighboring face has 4 or more
  /// edges.
  void FixEdgeTopoligicalInvariant(QuickHullFace* face, QuickHullEdge* e0, QuickHullEdge* e1);

  void CreateTwinEdge(QuickHullVertex* v0, QuickHullVertex* v1, QuickHullEdge* edge01, QuickHullEdge* edge10);
  QuickHullFace* CreateFace(QuickHullEdge* e0, QuickHullEdge* e1, QuickHullEdge* e2);

  QuickHullVertex* AllocateVertex();
  QuickHullEdge* AllocateEdge();
  QuickHullFace* AllocateFace();
  void DeallocateVertex(QuickHullVertex* vertex);
  void DeallocateEdge(QuickHullEdge* edge);
  void DeallocateFace(QuickHullFace* face);

  void ValidateFace(QuickHullFace* face);
  void ValidateHull();
  void ValidateFinalHull(const Array<Vec3>& points);
  bool IsInsideHull(Vec3Param point, float epsilon);

  DebugDrawStack* mDebugDrawStack;
  real mEpsilon;

  /// Stack data needed during the horizon identification phase.
  struct QuickHullSearchData
  {
    QuickHullSearchData()
    {
    }
    QuickHullSearchData(QuickHullFace* face)
    {
      mFace = face;
      mRange = InListWrappedRange<EdgeList>(&face->mEdges);
    }
    QuickHullSearchData(QuickHullFace* face, EdgeList::iterator it)
    {
      mFace = face;
      mRange = InListWrappedRange<EdgeList>(&face->mEdges, it);
    }
    QuickHullFace* mFace;
    InListWrappedRange<EdgeList> mRange;
  };

  VertexList mVertices;
  EdgeList mEdges;
  FaceList mFaces;

  DebugDrawStep& CreateStep(StringParam text);
  DebugDrawStep& DrawHull(DebugDrawStep& step, bool filled = false);
  void DrawVertices(DebugDrawStep& step, VertexList& vertexList, Vec4Param color);
  void DrawConflictVertices(DebugDrawStep& step, Vec4Param color = ToFloatColor(Color::Green));
  void DrawRemainingVertices(DebugDrawStep& step, Vec4Param color = ToFloatColor(Color::Gray));
  void DrawPoint(DebugDrawStep& step, Vec3Param p0, Vec4Param color = ToFloatColor(Color::White));
  void DrawLine(DebugDrawStep& step, Vec3Param p0, Vec3Param p1, Vec4Param color = ToFloatColor(Color::White));
  void
  DrawRay(DebugDrawStep& step, Vec3Param p0, Vec3Param p1, real headSize, Vec4Param color = ToFloatColor(Color::White));
  void DrawEdge(DebugDrawStep& step, QuickHullEdge* edge, Vec4Param color = ToFloatColor(Color::White));
  void DrawFace(DebugDrawStep& step,
                QuickHullFace* face,
                Vec4Param color = ToFloatColor(Color::White),
                bool drawEdges = false,
                bool drawFaceVec = false);
  void DrawInitialSpan(QuickHullVertex* v0, QuickHullVertex* v1);
  void DrawInitialTriangle(QuickHullVertex* v0, QuickHullVertex* v1, QuickHullVertex* v2);
  void DrawInitialTetrahedron(QuickHullVertex* v0, QuickHullVertex* v1, QuickHullVertex* v2, QuickHullVertex* v3);
  void DrawInitialHull();
  void DrawConflictPartition();
  void DrawFoundConflictVertex(QuickHullVertex* conflictVertex, QuickHullFace* conflictFace);
  void DrawHorizon(QuickHullVertex* eye, Array<QuickHullEdge*>& edges);
  void DrawNonConvexFaces(QuickHullFace* face0, QuickHullFace* face1);
  void DrawHullWithFace(StringParam text, QuickHullFace* face, Vec4Param color = ToFloatColor(Color::Red));
  void DrawFaceMerge(QuickHullFace* face0, QuickHullFace* face1, Vec4Param color = ToFloatColor(Color::Red));
  void DrawTopologicalFix(
      StringParam text, QuickHullFace* face0, QuickHullFace* face1, QuickHullEdge* edge0, QuickHullEdge* edge1);
  void DrawExpandedHull();
  void DrawHullWithDescription(StringParam text);
  void DrawFinalHull(const Array<Vec3>& points);

  Memory::Pool* mVertexPool;
  Memory::Pool* mEdgePool;
  Memory::Pool* mFacePool;
};

} // namespace Zero
