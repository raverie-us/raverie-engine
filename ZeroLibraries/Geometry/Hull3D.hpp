///////////////////////////////////////////////////////////////////////////////
///
/// \file Hull3D.hpp
/// Algorithm to take a point set and generate a convex mesh from it.
///
/// Authors: Joshua Davis
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
namespace Memory
{
class Pool;
}// namespace Memory
}// namespace Zero

namespace Geometry
{

DeclareBitField7(HullDebugDrawFlags,UnProcessedPoints,HullPoints,InteriorPoints,CurrentHull,NewHull,DeletedHull,WireFrameHull);

/// Algorithm to generate the convex hull of a set of points. The end result is a
/// collection of vertices, edges and faces on the hull. Each vertex Contains an
/// id that maps it to the index of the original data points.
class Hull3D
{
public:
  struct Vertex;
  struct Edge;
  struct Face;

  /// A vertex for the hull. This is a special data structure
  /// for efficient use in computing the hull.
  struct Vertex
  {
    OverloadedNew();
    Vertex(uint id, Vec3Param pos);

    /// This id maps to the index of the vertex in the original
    /// point list that the hull was created from.
    uint Id;
    /// The 3d position of this vertex.
    Vec3 Position;

    Zero::Link<Vertex> link;
  private:
    friend class Hull3D;
    /// When expanding the convex hull to a new face, a new vertex may be
    /// created from this point to the newly added point. To prevent
    /// duplicates from being made the vertex stores this edge here.
    Edge* mNewEdge;
  };

  /// An edge between two vertices. Also stores what faces are adjacent to it.
  struct Edge
  {
    OverloadedNew();
    Edge(uint id, Vertex* v0, Vertex* v1);

    /// The faces that are adjacent to this edge.
    Face* AdjFaces[2];
    /// The endpoints that this edge is between.
    Vertex* EndPoints[2];

    /// Id of this edge, doesn't have much of a purpose except for debugging.
    uint Id;

    Zero::Link<Edge> link;
  private:
    /// Sets the endpoints for the edge.
    void SetVertices(Vertex* v0, Vertex* v1);
    /// Sets the adjacent faces for this edge.
    void SetFaces(Face* f0, Face* f1);

    friend class Hull3D;
  };

  /// A face on the hull. Contains what vertices and edges are a part of it.
  struct Face
  {
    OverloadedNew();
    Face(uint id, Vertex* v0, Vertex* v1, Vertex* v2);

    /// Returns the normal of this face. The normal is the counter-clockwise
    /// normal of this face, that is Cross(v1 - v0, v2 - v0).
    Vec3 GetNormal();

    /// Debug verification. Tests if the face is visible to the passed in vertex.
    bool VerifyVisible(Vertex* v);

    /// The vertices that this face is on.
    Vertex* Vertices[3];
    /// The three edges of this face.
    Edge* Edges[3];

    /// Id of this face, doesn't have much of a purpose except for debugging.
    uint Id;

    Zero::Link<Face> link;
  private:
    /// The test for the vertex being visible to the face. Modifies the IsVisible flag.
    bool TestVisible(Vertex* v);

    void SetVertices(Vertex* v0, Vertex* v1, Vertex* v2);
    void SetEdges(Edge* e0, Edge* e1, Edge* e2);

    friend class Hull3D;
    /// Intermediary value that stores whether or
    /// not this face can see the current vertex.
    bool IsVisible;
  };

  typedef Zero::InList<Vertex> VertexList;
  typedef Zero::InList<Edge> EdgeList;
  typedef Zero::InList<Face> FaceList;

  Hull3D();
  ~Hull3D();

  /// Builds a convex hull. Returns false when there is no legitimate convex hull.
  bool Build(Zero::Array<Vec3>& verties);
  /// Builds a convex hull. Returns false when there is no legitimate convex hull.
  bool Build(const Vec3* vertices, uint size);

  /// Builds a convex hull for debugging purposes. This does not build the full hull,
  /// but builds it step by step. For further steps, DebugStep() must be called.
  bool DebugBuild(Vec3Ptr vertices, uint size);
  /// Adds one new vertex to the hull.
  bool DebugStep();
  /// Debug draws the current hull's state. Takes flags from HullDebugDrawFlags enum.
  bool DebugDraw(uint drawFlags);

  /// Returns all of the vertices that are on the surface of the hull.
  VertexList::range GetHullVertices();
  /// Returns all of the vertices that are inside of the convex hull.
  VertexList::range GetInternalVertices();
  /// Returns all of the edges of the hull.
  EdgeList::range GetEdges();
  /// Returns all of the faces on the hull.
  FaceList::range GetFaces();

  /// Gets the number of vertices on the hull.
  uint GetHullVertexCount();
  /// Gets the number of edges on the hull.
  uint GetEdgeCount();
  /// Gets the number of faces on the hull.
  uint GetFaceCount();

private:
  /// Deletes all data of the current hull.
  void ClearHullData(void);
  /// Creates the hull from the vertices.
  void BuildVertices(const Vec3* vertices, uint size);
  /// Builds the initial convex hull, a tetrahedron, from the given vertices.
  /// Returns false if there is no valid convex hull.
  bool ConstructInitialTetrahedron();
  /// The heart of the algorithm. Adds a new vertex to the convex
  /// hull and then recomputes all faces and edges. Returns false if an error is encountered.
  bool AddVertex();
  /// Checks all faces to see what ones should be removed when adding
  /// the new vertex. Moves visible faces to the visible faces list.
  bool CheckVisibility(Vertex* v);
  /// Creates a new face for the given edge and the newly added vertex.
  /// Also creates new edges that need to be part of that new face.
  void CreateNewConeFace(Edge* incidentEdge, Vertex* newVertex);
  /// After adding a new point, removes old edges and faces
  /// and moves points from the hull to the interior.
  void Cleanup();
  /// Filters down the indices of the edges so there are no holes.
  /// Also counts the total number of edges.
  void FixEdgeIndices();
  /// Filters down the indices of the faces so there are no holes.
  /// Also counts the total number of faces.
  void FixFaceIndices();

  /// Makes a new edge for the given vertices.
  /// Gives it the correct Id and puts it in the new edge list.
  Edge* MakeNewEdge(Vertex* v0, Vertex* v1);
  /// Makes a new face for the given vertices.
  /// Gives it the correct Id and puts it in the new face list.
  Face* MakeNewFace(Vertex* v0, Vertex* v1, Vertex* v2);

  /// Helper template that deletes all items from the list
  template <typename ListType>
  void DeleteList(ListType& list);

  /// Debug verification. Checks all debug verification tests.
  void VerifyHull();
  /// Makes sure that all of the faces are counter clockwise and that no vertex
  /// is on the positive side of any face (convex test).
  bool VerifyWinding();
  /// Verifies that all of the graph (edge to face to vertices) info is correct.
  bool VerifyGraph();

  static Zero::Memory::Pool* sVertexPool;
  static Zero::Memory::Pool* sEdgePool;
  static Zero::Memory::Pool* sFacePool;

  /// The vertices left to be processed.
  VertexList mVertices;
  /// The vertices currently on the hull.
  VertexList mHullVertices;
  /// The vertices inside of the hull.
  VertexList mInternalVertices;
  /// The edges currently on the hull.
  EdgeList mEdges;
  /// The faces currently on the hull.
  FaceList mFaces;

  // Internal lists for separation of state

  /// Edges that were on the hull that are now internal.
  /// These need to be deleted after the current pass.
  EdgeList mToDeleteEdges;
  /// Newly created edges in this pass. These edges should not be
  /// iterated through during the creation of the new faces for the current vertex.
  EdgeList mNewEdges;
  /// The faces that are visible to the newly added vertex.
  /// These should be deleted after the current pass.
  FaceList mVisibleFaces;
  /// Newly created faces in this pass. Mostly separated for debug drawing.
  FaceList mNewFaces;

  /// Current number of edges. Used mainly to keep track of the next Id to give an edge.
  uint mEdgeCount;
  /// Current number of faces. Used mainly to keep track of the next Id to give a face.
  uint mFaceCount;
  /// If the algorithm has reached a fail case and can't continue.
  bool mFailed;
};

}//namespace Geometry
