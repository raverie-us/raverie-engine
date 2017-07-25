///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class UpdateEvent;

/// Stores how many jumps it takes to get from this node to every other node.
struct PointNode
{
  struct AdjacencyInfo
  {
    AdjacencyInfo() { mJumps = (uint)(-1); }
    uint mJumps;
  };

  void AddNeighbor(uint adjacentPoint);

  Array<AdjacencyInfo> mAdjacentPoints;
};

/// A graph that stores how many edges must be crossed to get from any node to
/// any other node. If there is no way to get from a to b then the value will be
/// "point count" even though it is technically not possible to reach that node.
struct PointGraph
{
  void AddPoint();
  void SetSize(uint pointCount);
  void AddEdge(uint p1, uint p2);
  uint& operator()(uint x, uint y);
  uint& Get(uint x, uint y);
  void Build();

  Array<PointNode> mNodes;
};

/// When should debug drawing of a spring system happen.
/// <param name="None">Never debug draw.</param>
/// <param name="WhenNoMesh">Only debug draw if no graphical mesh is present.</param>
/// <param name="Always">Always debug draw.</param>
DeclareEnum3(SpringDebugDrawMode, None, WhenNoMesh, Always);
/// How should springs be drawn?
/// <param name="Normal">Draw all springs equally (same color).</param>
/// <param name="Sorted">Draw the springs in sorted order. This alters the
/// color based upon the distance from an anchor point.</param>
DeclareEnum2(SpringDebugDrawType, Normal, Sorted);
/// How should springs be sorted for solving. This effects the convergence rate of a system.
/// <param name="None">No sorting is preserved.</param>
/// <param name="TopDown">Sort so that springs closer to anchors are solved first.</param>
/// <param name="BottomUp">Sort so that springs further away from anchors are solved first.</param>
DeclareEnum3(SpringSortOrder, None, TopDown, BottomUp);

/// A spring system meant for one-way interaction between a character (or some sort of driven body).
/// That is, the cloth is affected by collision but does not affect the things that collide with it.
class SpringSystem : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void TransformUpdate(TransformUpdateInfo& info) override;
  void OnDestroy(uint flags = 0) override;
  void DebugDraw() override;

  struct PointMass;
  struct Edge;
  struct Face;
  struct SystemConnection;

  /// Small helper to remove a connection between spring systems.
  void RemoveConnection(SystemConnection* connection);
  /// Make sure all system connections are valid and remove invalid ones.
  void UpdateConnections();
  /// Update all the positions of all anchored point masses.
  void UpdateAnchors();
  
  // Solving functions of the spring system
  void SolveInternalForces();
  void SolveSpringForces();
  /// Iterate through all edges, iteratively solving them,
  /// to try to relax the system to a better global solution.
  void RelaxSprings();
  /// Solve one edge to be at the exact rest length based upon the mass ratio of the points.
  void SolveEdge(PointMass& p0, PointMass& p1, real restLength);
  /// Approximate the velocity of a point based upon its old position and new position.
  void UpdateVelocities(real dt);
  void IntegrateVelocity(real dt);
  void IntegratePosition(real dt);
  /// Commit all results to whatever structure is needed after solving (such as a mesh).
  virtual void Commit() {};

  /// Helper to add an edges between two point masses. The error correction is how
  /// much to take off of the rest length to help with small numerical errors.
  Edge& AddEdge(uint index0, uint index1, real errCorrection = real(0.0));
  /// Add a new point mass at the given position
  void AddPointMass(Vec3Param position);
  /// Set the point mass at the given index to be anchored (based upon its relative position)
  /// to another cog. If null is passed in then the anchor is cleared.
  void SetPointMassAnchor(uint index, Cog* anchorCog);
  /// Adds a connection between a point mass on this system and a point mass on another spring system.
  void AddConnection(SpringSystem* otherSystem, uint indexA, uint indexB);
  /// Finds if there is a system connection to the passed in other system.
  SystemConnection* FindConnection(SpringSystem* otherSystem);

  /// How should edges be sorted? Sorted so we solve at anchors and moving out or the opposite?
  SpringSortOrder::Enum GetSortOrder();
  void SetSortOrder(SpringSortOrder::Enum orderingType);
  /// Sorts all of the edges based upon our current sorting method.
  void SortEdges();

  /// When do we debug draw? Never? Only when we have no mesh?
  SpringDebugDrawMode::Enum GetDebugDrawMode();
  void SetDebugDrawMode(SpringDebugDrawMode::Enum debugDrawMode);
  /// What type of debug drawing do we do? Color coding based upon edge distance?
  SpringDebugDrawType::Enum GetDebugDrawType();
  void SetDebugDrawType(SpringDebugDrawType::Enum debugDrawType);

  /// Do a raycast against all of the faces of this mesh and return what face and where it was hit.
  bool Cast(RayParam ray, Face& resultFace, Vec3Ref intersectionPoint);

  
  /// Represents a anchor point for a PointMass to a given cog (not another spring system).
  struct AnchorPoint
  {
    void Serialize(Serializer& stream);
    void OnAllObjectsCreated(CogInitializer& initializer);

    /// The index of the PointMass that we are anchoring.
    uint mIndex;
    /// What cog we are anchored to.
    CogId mAnchorObject;
    /// The local space position of the PointMass relative to the anchor cog.
    Vec3 mLocalAnchorPoint;
  };
  typedef Array<AnchorPoint*> AnchorPoints;
  AnchorPoints mAnchors;


  struct PointMass
  {
    PointMass()
    {
      mInitialOffset = mOldPosition = mPosition = mVelocity = mForce = Vec3::cZero;
      mInvMass = real(1.0);
      mAnchor = nullptr;
    }

    void Serialize(Serializer& stream);

    /// Old position is needed to calculate the velocity after positions are changed
    Vec3 mOldPosition;
    Vec3 mPosition;
    /// Maybe remove this and serialize in the local position (as old position)?
    Vec3 mInitialOffset;
    Vec3 mVelocity;
    Vec3 mForce;
    real mInvMass;

    AnchorPoint* mAnchor;
  };
  typedef Array<PointMass> PointMasses;
  PointMasses mPointMasses;

  /// An edge between two point masses that typically represents a spring.
  struct Edge
  {
    Edge()
    {
      mIndex0AnchorDistance = 0;
      mIndex1AnchorDistance = 0;
    }

    Edge(uint index0, uint index1, Vec3Param point0, Vec3Param point1)
    {
      Set(index0, index1, point0, point1);
    }

    void Set(uint index0, uint index1, Vec3Param point0, Vec3Param point1)
    {
      mIndex0 = index0;
      mIndex1 = index1;
      mRestLength = Math::Length(point0 - point1);
      mK = 1600;
      mD = 1;
      mIndex0AnchorDistance = 0;
      mIndex1AnchorDistance = 0;
    }

    void Serialize(Serializer& stream);

    uint mIndex0;
    uint mIndex1;
    //for debug drawing right now
    uint mIndex0AnchorDistance;
    uint mIndex1AnchorDistance;
    //attachment distance
    real mRestLength;
    real mK;
    real mD;
  };
  typedef Array<Edge> Edges;
  Edges mEdges;

  /// A triangle face for the mesh. Used to update for rendering and also to apply
  /// several physics effects that are based upon the hit surface area.
  struct Face
  {
    void Serialize(Serializer& stream);

    uint mIndex0;
    uint mIndex1;
    uint mIndex2;
  };
  typedef Array<Face> Faces;
  Faces mFaces;

  /// A connection of edges between two different spring systems.
  /// This allows connecting ropes and other spring systems together.
  struct SystemConnection
  {
    void Serialize(Serializer& stream);
    void OnAllObjectsCreated(CogInitializer& initializer);

    CogId mOwningSystemId;
    CogId mOtherSystemId;
    SpringSystem* mOwningSystem;
    SpringSystem* mOtherSystem;

    Edges mEdges;

    Link<SystemConnection> mOwnedEdge;
    Link<SystemConnection> mConnectedEdge;
  };
  
  // We need a double edged list so that it can be traversed from each side, but
  // to make sure they aren't solved twice the edges are separated into owned and connected edges.
  typedef InList<SystemConnection, &SystemConnection::mOwnedEdge> OwnedEdgeList;
  OwnedEdgeList mOwnedEdges;
  typedef InList<SystemConnection, &SystemConnection::mConnectedEdge> ConnectedEdgeList;
  ConnectedEdgeList mConnectedEdges;

  Link<SpringSystem> SpaceLink;
  Link<SpringSystem> mSpringGroupLink;

  real mCorrectionPercent;
  real mPointInvMass;
  SpringSortOrder::Enum mSortOrder;

  SpringDebugDrawMode::Enum mDebugDrawMode;
  SpringDebugDrawType::Enum mDebugDrawType;
};

class DecorativeCloth : public SpringSystem
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void TransformUpdate(TransformUpdateInfo& info) override;


  void ResetMeshPositions();

  void UpdatePointMassPosition(uint index, Vec3Param position);
  void UpdateAnchorPoint(uint index, Vec3Param position, Cog* anchorCog);
  void LoadFromMesh(PhysicsMesh* mesh, bool clearOldData);
  void LoadPointMeshData(const Array<Vec3>& verts, bool clearOldData);
  void LoadFromMeshData(const Array<Vec3>& verts, const Array<uint>& indices, bool clearOldData);
  void AddEdge(uint index0, uint index1, real errCorrection = real(0.01));
  
  void Commit() override;
  void UploadToMesh();

  PhysicsMesh* GetMesh();
  void SetMesh(PhysicsMesh* mesh);

  /// Connects edges between vertices up to n hops away.
  uint GetConnectivityCounter();
  void SetConnectivityCounter(uint counter);

  HandleOf<PhysicsMesh> mMesh;


  bool mJakobsen;

  real GetSpringStiffness();
  void SetSpringStiffness(real stiffness);
  real GetSpringDamping();
  void SetSpringDamping(real damping);

  real mSpringStiffness;
  real mSpringDamping;

  uint mConnectivityCounter;
};

class DecorativeRope : public SpringSystem
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void DebugDraw() override;

  void Commit() override;

  // Internal helper
  void GetPoints(Array<Vec3>& points);
  Vec3 GetWorldPoint(Cog* cog, Vec3Param localPoint, uint pointMassIndex, SpringSystem*& resultingSystem);
  void CreateLinks();

  /// How many links there should be between the two connected points that
  /// will be generated at runtime. There is a minimum of 2 links required.
  uint GetNumberOfLinks();
  void SetNumberOfLinks(uint linkNumber);

  
  /// Reduces the rest length of the created springs by
  /// some amount to help a rope to be more taught.
  real mErrorCorrection;
  /// How many links should be created between the two connected points.
  uint mNumberOfLinks;

  CogId mCogA;
  CogId mCogB;
  uint mPointMassIndexA;
  uint mPointMassIndexB;
  Vec3 mLocalPointA;
  Vec3 mLocalPointB;

  bool mAnchorA;
  bool mAnchorB;

  Link<DecorativeRope> SpaceLink;
  Link<DecorativeRope> mSpringGroupLink;
};

/// To correctly solve a collection of connected spring systems, their solve
/// must be interleaved. This group is a collection of systems (found with a graph traversal)
/// that are solved together to help guarantee a more correct global solution.
class SpringGroup
{
public:
  void Solve(PhysicsSpace* space, real dt);

  /// Helper to apply the global physics effects to a spring system.
  void ApplyGlobalEffects(PhysicsSpace* space, SpringSystem* system);

  typedef InList<SpringSystem, &SpringSystem::mSpringGroupLink> SpringSystems;
  SpringSystems mSystems;
};

}//namespace Zero
