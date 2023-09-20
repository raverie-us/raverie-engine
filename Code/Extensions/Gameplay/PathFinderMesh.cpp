// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
DefineEvent(PathFinderMeshFinished);
}

// Nav Mesh Edge
NavMeshEdge::NavMeshEdge(NavMeshPolygon* owner) :
    mCost(0.0f),
    mTailVertex(u32(-1)),
    mPolygon(owner),
    mNextConnected(this),
    mPreviousConnected(this)
{
  owner->mEdges.PushBack(this);
}

// Edge Triangle Range
NavMeshEdge::PolygonRange::PolygonRange() : mIgnore(nullptr)
{
}

NavMeshEdge::PolygonRange::PolygonRange(NavMeshEdge* edge, NavMeshPolygon* ignore) :
    mBegin(edge->mNextConnected),
    mEnd(edge),
    mIgnore(ignore)
{
  FindNextValid();
}

bool NavMeshEdge::PolygonRange::Empty() const
{
  return mBegin == mEnd;
}

void NavMeshEdge::PolygonRange::PopFront()
{
  mBegin = mBegin->mNextConnected;
  FindNextValid();
}

NavMeshPolygon* NavMeshEdge::PolygonRange::Front()
{
  return mBegin->mPolygon;
}

void NavMeshEdge::PolygonRange::FindNextValid()
{
  while (!Empty())
  {
    if (Front() != mIgnore)
      break;

    PopFront();
  }
}

NavMeshEdge::PolygonRange& NavMeshEdge::PolygonRange::All()
{
  return *this;
}

NavMeshEdge::PolygonRange NavMeshEdge::AllConnectedTriangles(NavMeshPolygon* ignore)
{
  return PolygonRange(this, ignore);
}

// Nav Mesh Triangle
Vec3 NavMeshPolygon::GetCenter(PathFinderAlgorithmMesh* mesh)
{
  Array<Vec3>& vertices = mesh->mVertices;

  Vec3 center = Vec3::cZero;
  u32 count = 0;
  forRange (NavMeshEdge& edge, AllEdges())
  {
    center += vertices[edge.mTailVertex];
    ++count;
  }

  center /= (float)count;

  return center;
}

NavMeshPolygon::EdgeRange NavMeshPolygon::AllEdges()
{
  return mEdges.All();
}

NavMeshPolygon::PolygonRange NavMeshPolygon::AllNeighboringPolygons()
{
  return PolygonRange(this);
}

// Triangle Triangle Range
NavMeshPolygon::PolygonRange::PolygonRange(NavMeshPolygon* polygon) : mCurrentEdge(polygon->AllEdges())
{
  mCurrentEdgesPolygons = CurrentEdge()->AllConnectedTriangles(mPolygon);
  FindNextValid();
}

bool NavMeshPolygon::PolygonRange::Empty()
{
  return mCurrentEdge.Empty();
}

void NavMeshPolygon::PolygonRange::PopFront()
{
  mCurrentEdgesPolygons.PopFront();
  FindNextValid();
}

NavMeshEdge* NavMeshPolygon::PolygonRange::CurrentEdge()
{
  return &mCurrentEdge.Front();
}

NavMeshPolygon* NavMeshPolygon::PolygonRange::Front()
{
  return mCurrentEdgesPolygons.Front();
}

NavMeshPolygon::PolygonRange& NavMeshPolygon::PolygonRange::All()
{
  return *this;
}

void NavMeshPolygon::PolygonRange::FindNextValid()
{
  while (!mCurrentEdge.Empty() && mCurrentEdgesPolygons.Empty())
  {
    // If the triangles for the current edge are empty, move to the next edge
    mCurrentEdge.PopFront();
    mCurrentEdgesPolygons = CurrentEdge()->AllConnectedTriangles(mPolygon);
  }
}

float NavMeshPolygon::PolygonRange::GetFrontCost()
{
  return CurrentEdge()->mCost + Front()->mCost;
}

// Finder Mesh Node Range
PathFinderMeshNodeRange::PathFinderMeshNodeRange(NavMeshPolygon* polygon) : mRange(polygon->AllNeighboringPolygons())
{
  PopUntilValid();
}

bool PathFinderMeshNodeRange::Empty()
{
  return mRange.Empty();
}

void PathFinderMeshNodeRange::PopFront()
{
  mRange.PopFront();
  PopUntilValid();
}

PathFinderMeshNodeRange::FrontResult PathFinderMeshNodeRange::Front()
{
  return FrontResult(mRange.Front()->mId, mRange.GetFrontCost());
}

PathFinderMeshNodeRange& PathFinderMeshNodeRange::All()
{
  return *this;
}

void PathFinderMeshNodeRange::PopUntilValid()
{
  while (!mRange.Empty())
  {
    NavMeshPolygon* triangle = mRange.Front();

    // Skip over triangles with collision
    if (triangle->mCollision == false)
      break;
    mRange.PopFront();
  }
}

// Finder Algorithm Mesh
PathFinderAlgorithmMesh::PathFinderAlgorithmMesh() : mCurrentPolygonId(0), mCurrentEdgeId(0)
{
}

PathFinderMeshNodeRange PathFinderAlgorithmMesh::QueryNeighbors(NavMeshPolygonId polygonId)
{
  return PathFinderMeshNodeRange(GetPolygon(polygonId));
}

bool PathFinderAlgorithmMesh::QueryIsValid(NavMeshPolygonId polygonId)
{
  return (polygonId != u32(-1));
}

float PathFinderAlgorithmMesh::QueryHeuristic(NavMeshPolygonId start, NavMeshPolygonId goal)
{
  Vec3 startPos = GetPolygon(start)->GetCenter(this);
  Vec3 goalPos = GetPolygon(goal)->GetCenter(this);

  return Math::DistanceSq(startPos, goalPos);
}

u32 PathFinderAlgorithmMesh::AddVertex(Vec3Param pos)
{
  mVertices.PushBack(pos);
  return mVertices.Size() - 1;
}

NavMeshPolygonId PathFinderAlgorithmMesh::AddPolygon()
{
  NavMeshPolygonId id = GetNextPolygonId();
  NavMeshPolygon* polygon = new NavMeshPolygon();
  polygon->mId = id;
  mPolygons.Insert(id, polygon);
  return id;
}

NavMeshPolygonId PathFinderAlgorithmMesh::AddPolygon(Array<u32>& vertices)
{
  // We must be at least a triangle
  if (vertices.Size() <= 2)
  {
    DoNotifyException("Cannot create polygon", "Polygons must have at least 3 vertices");
    return u32(-1);
  }

  // Create the new polygon
  NavMeshPolygonId id = AddPolygon();

  // Add all the given edges
  u32 previous = vertices.Front();
  for (uint i = 1; i < vertices.Size(); ++i)
  {
    u32 current = vertices[i];
    AddEdgeToPolygon(id, previous, current);
    previous = current;
  }

  return id;
}

NavMeshPolygonId PathFinderAlgorithmMesh::AddPolygon(ArrayClass<u32>& vertices)
{
  return AddPolygon(vertices.NativeArray);
}

NavMeshPolygonId PathFinderAlgorithmMesh::AddPolygon(u32 vertex0, u32 vertex1, u32 vertex2)
{
  // Create the new polygon
  NavMeshPolygonId id = AddPolygon();

  AddEdgeToPolygon(id, vertex0, vertex1);
  AddEdgeToPolygon(id, vertex1, vertex2);
  AddEdgeToPolygon(id, vertex2, vertex0);

  return id;
}

NavMeshPolygonId PathFinderAlgorithmMesh::AddPolygon(u32 vertex0, u32 vertex1, u32 vertex2, u32 vertex3)
{
  // Create the new polygon
  NavMeshPolygonId id = AddPolygon();

  AddEdgeToPolygon(id, vertex0, vertex1);
  AddEdgeToPolygon(id, vertex1, vertex2);
  AddEdgeToPolygon(id, vertex2, vertex3);
  AddEdgeToPolygon(id, vertex3, vertex0);

  return id;
}

NavMeshEdgeId PathFinderAlgorithmMesh::AddEdgeToPolygon(NavMeshPolygonId polygonId, u32 vertex0, u32 vertex1)
{
  NavMeshPolygon* polygon = GetPolygon(polygonId);

  if (polygon == nullptr)
  {
    DoNotifyException("Cannot add edge to polygon", "Given polygon id is invalid");
    return u32(-1);
  }

  NavMeshEdge* edge = new NavMeshEdge(polygon);
  NavMeshEdgeId edgeId = GetNextEdgeId();
  mEdges.Insert(edgeId, edge);

  edge->mTailVertex = vertex0;

  // We need to search for other polygons connected to these two vertices
  u64 edgeIndex = GetLexicographicId(vertex0, vertex1);
  if (NavMeshEdge* connected = mEdgeConnections.FindValue(edgeIndex, nullptr))
  {
    // Insert the new edge into the linked list
    NavMeshEdge* previous = connected;
    NavMeshEdge* next = previous->mNextConnected;

    next->mPreviousConnected = edge;
    previous->mNextConnected = edge;

    edge->mNextConnected = next;
    edge->mPreviousConnected = connected;
  }
  else
  {
    // Add it if the connection doesn't already exist
    mEdgeConnections.Insert(edgeIndex, edge);
  }

  return edgeId;
}

void PathFinderAlgorithmMesh::SetPolygonCost(NavMeshPolygonId polygonId, float cost)
{
  if (NavMeshPolygon* polygon = GetPolygon(polygonId))
    polygon->mCost = cost;
  else
    DoNotifyException("Cannot set polygon cost", "Given polygon id is invalid");
}

void PathFinderAlgorithmMesh::SetPolygonClientData(NavMeshPolygonId polygonId, Cog* clientData)
{
  if (NavMeshPolygon* polygon = GetPolygon(polygonId))
    mPolygonClientData.Insert(polygon, clientData->GetId());
  else
    DoNotifyException("Cannot set polygon client data", "Given polygon id is invalid");
}

void PathFinderAlgorithmMesh::SetEdgeCost(NavMeshEdgeId edgeId, float cost)
{
  if (NavMeshEdge* edge = GetEdge(edgeId))
    edge->mCost = cost;
  else
    DoNotifyException("Cannot set edge cost", "Given edge id is invalid");
}

void PathFinderAlgorithmMesh::SetEdgeClientData(NavMeshEdgeId edgeId, Cog* clientData)
{
  if (NavMeshEdge* edge = GetEdge(edgeId))
    mEdgeClientData.Insert(edge, clientData->GetId());
  else
    DoNotifyException("Cannot set edge cost", "Given edge id is invalid");
}

void PathFinderAlgorithmMesh::Clear()
{
  forRange (NavMeshEdge* edge, mEdges.Values())
    delete edge;
  forRange (NavMeshPolygon* triangle, mPolygons.Values())
    delete triangle;

  mEdgeConnections.Clear();
  mVertices.Clear();
  mEdges.Clear();
  mPolygons.Clear();
  mPolygonClientData.Clear();
  mEdgeClientData.Clear();
}

NavMeshPolygon* PathFinderAlgorithmMesh::GetPolygon(NavMeshPolygonId id)
{
  return mPolygons.FindValue(id, nullptr);
}

NavMeshEdge* PathFinderAlgorithmMesh::GetEdge(NavMeshEdgeId id)
{
  return mEdges.FindValue(id, nullptr);
}

NavMeshPolygonId PathFinderAlgorithmMesh::GetNextPolygonId()
{
  return mCurrentPolygonId++;
}

NavMeshEdgeId PathFinderAlgorithmMesh::GetNextEdgeId()
{
  return mCurrentEdgeId++;
}

// Path Finder Mesh
RaverieDefineType(PathFinderMesh, builder, type)
{
  RaverieBindDocumented();
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindInterface(PathFinder);
  RaverieBindDependency(Transform);
  RaverieBindEvent(Events::PathFinderGridFinished, PathFinderEvent<Vec3>);

  RaverieBindOverloadedMethod(SetMesh, RaverieInstanceOverload(void, Mesh*));
  RaverieBindOverloadedMethod(SetMesh, RaverieInstanceOverload(void, Mesh*, float));

  RaverieBindMethod(AddVertex);

  // RaverieBindOverloadedMethod(AddPolygon,
  // RaverieInstanceOverload(NavMeshPolygonId, ArrayClass<u32>&));
  RaverieBindOverloadedMethod(AddPolygon, RaverieInstanceOverload(NavMeshPolygonId, u32, u32, u32));
  RaverieBindOverloadedMethod(AddPolygon, RaverieInstanceOverload(NavMeshPolygonId, u32, u32, u32, u32));

  RaverieBindMethod(SetPolygonCost);
  RaverieBindMethod(SetPolygonClientData);
  RaverieBindMethod(SetEdgeCost);
  RaverieBindMethod(SetEdgeClientData);
  RaverieBindMethod(Clear);

  // RaverieBindOverloadedMethod(FindPath,
  // RaverieInstanceOverload(HandleOf<ArrayClass<IntVec3>>, IntVec3Param,
  // IntVec3Param)); RaverieBindOverloadedMethod(FindPath,
  // RaverieInstanceOverload(HandleOf<ArrayClass<Real3>>, Real3Param,
  // Real3Param)); RaverieBindOverloadedMethod(FindPathThreaded,
  // RaverieInstanceOverload(HandleOf<PathFinderRequest>, IntVec3Param,
  // IntVec3Param)); RaverieBindOverloadedMethod(FindPathThreaded,
  // RaverieInstanceOverload(HandleOf<PathFinderRequest>, Real3Param,
  // Real3Param));

  RaverieBindMethod(WorldPositionToPolygon);
  RaverieBindMethod(LocalPositionToPolygon);
}

PathFinderMesh::PathFinderMesh() : mTransform(nullptr), mMesh(new CopyOnWriteData<PathFinderAlgorithmMesh>())
{
}

void PathFinderMesh::Serialize(Serializer& stream)
{
  PathFinder::Serialize(stream);
}

void PathFinderMesh::Initialize(CogInitializer& initializer)
{
  RaverieBase::Initialize(initializer);
  mTransform = GetOwner()->has(Transform);
}

void PathFinderMesh::DebugDraw()
{
  PathFinderAlgorithmMesh* mesh = mMesh;
  Array<Vec3>& vertices = mesh->mVertices;

  forRange (NavMeshPolygon* polygon, mesh->mPolygons.Values())
  {
    NavMeshPolygon::EdgeRange r = polygon->AllEdges();
    u32 i0 = r.Front().mTailVertex;
    r.PopFront();
    u32 i1 = r.Front().mTailVertex;
    r.PopFront();

    Vec3 p0 = vertices[i0];
    Vec3 p1 = vertices[i1];
    Vec3 p2;

    // First edge
    Debug::Line debugLine(p0, p1);
    debugLine.mColor = ToFloatColor(Color::Purple);
    gDebugDraw->Add(debugLine);

    forRange (NavMeshEdge& edge, r)
    {
      u32 i2 = edge.mTailVertex;

      p1 = vertices[i1];
      p2 = vertices[i2];

      Debug::Triangle debugTriangle(p0, p1, p2);
      debugTriangle.mColor = ToFloatColor(Color::Purple);
      debugTriangle.mColor.w = 0.1f;
      debugTriangle.SetFilled(true);
      gDebugDraw->Add(debugTriangle);

      Debug::Line debugLine(p1, p2);
      debugLine.mColor = ToFloatColor(Color::Purple);
      gDebugDraw->Add(debugLine);

      i1 = i2;
    }

    // Last edge
    Debug::Line debugLineLast(p2, p0);
    debugLineLast.mColor = ToFloatColor(Color::Purple);
    gDebugDraw->Add(debugLineLast);
  }
}

Variant PathFinderMesh::WorldPositionToNodeKey(Vec3Param worldPosition)
{
  return Variant(WorldPositionToPolygon(worldPosition));
}

Vec3 PathFinderMesh::NodeKeyToWorldPosition(VariantParam nodeKey)
{
  NavMeshPolygon* triangle = (NavMeshPolygon*)nodeKey;
  return triangle->GetCenter(mMesh);
}

void PathFinderMesh::FindPathGeneric(VariantParam start, VariantParam goal, Array<Variant>& pathOut)
{
  GenericFindPathHelper<NavMeshPolygonId, PathFinderAlgorithmMesh>(mMesh, start, goal, pathOut, mMaxIterations);
}

HandleOf<PathFinderRequest> PathFinderMesh::FindPathGenericThreaded(VariantParam start, VariantParam goal)
{
  return GenericFindPathThreadedHelper<NavMeshPolygonId, PathFinderAlgorithmMesh>(mMesh, start, goal, mMaxIterations);
}

StringParam PathFinderMesh::GetCustomEventName()
{
  return Events::PathFinderMeshFinished;
}

void PathFinderMesh::SetMesh(Mesh* graphicsMesh)
{
  SetMesh(graphicsMesh, Math::DegToRad(90.0f));
}

void PathFinderMesh::SetMesh(Mesh* graphicsMesh, float maxSlope)
{
  mMesh.CopyIfNeeded();
  PathFinderAlgorithmMesh* mesh = mMesh;

  // Add all vertices
  VertexBuffer& vertices = graphicsMesh->mVertices;
  for (uint i = 0; i < vertices.GetVertexCount(); ++i)
  {
    Vec3 position = vertices.GetData<Vec3>(i, VertexSemantic::Position);
    mesh->AddVertex(position);
  }

  // Add all triangles
  IndexBuffer& indices = graphicsMesh->mIndices;
  for (uint i = 0; i < indices.GetCount(); i += 3)
  {
    uint index0 = indices.Get(i);
    uint index1 = indices.Get(i + 1);
    uint index2 = indices.Get(i + 2);
    mesh->AddPolygon(index0, index1, index2);
  }
}

uint PathFinderMesh::AddVertex(Vec3Param pos)
{
  mMesh.CopyIfNeeded();
  return mMesh->AddVertex(pos);
}

NavMeshPolygonId PathFinderMesh::AddPolygon(Array<u32>& vertices)
{
  mMesh.CopyIfNeeded();
  return mMesh->AddPolygon(vertices);
}

NavMeshPolygonId PathFinderMesh::AddPolygon(ArrayClass<u32>& vertices)
{
  mMesh.CopyIfNeeded();
  return mMesh->AddPolygon(vertices);
}

NavMeshPolygonId PathFinderMesh::AddPolygon(u32 vertex0, u32 vertex1, u32 vertex2)
{
  mMesh.CopyIfNeeded();
  return mMesh->AddPolygon(vertex0, vertex1, vertex2);
}

NavMeshPolygonId PathFinderMesh::AddPolygon(u32 vertex0, u32 vertex1, u32 vertex2, u32 vertex3)
{
  mMesh.CopyIfNeeded();
  return mMesh->AddPolygon(vertex0, vertex1, vertex2, vertex3);
}

void PathFinderMesh::SetPolygonCost(NavMeshPolygonId polygonId, float cost)
{
  mMesh.CopyIfNeeded();
  mMesh->SetPolygonCost(polygonId, cost);
}

void PathFinderMesh::SetPolygonClientData(NavMeshPolygonId polygonId, Cog* clientData)
{
  mMesh.CopyIfNeeded();
  mMesh->SetPolygonClientData(polygonId, clientData);
}

void PathFinderMesh::SetEdgeCost(NavMeshEdgeId edgeId, float cost)
{
  mMesh.CopyIfNeeded();
  mMesh->SetEdgeCost(edgeId, cost);
}

void PathFinderMesh::SetEdgeClientData(NavMeshEdgeId edgeId, Cog* clientData)
{
  mMesh.CopyIfNeeded();
  mMesh->SetEdgeClientData(edgeId, clientData);
}

void PathFinderMesh::Clear()
{
  mMesh.CopyIfNeeded();
  mMesh->Clear();
}

HandleOf<ArrayClass<Vec3>> PathFinderMesh::FindPath(NavMeshPolygonId start, NavMeshPolygonId goal)
{
  PathFinderAlgorithmMesh* mesh = mMesh;
  // NavMeshPolygon* startPoly = mesh->GetPolygon(start);
  // NavMeshPolygon* goalPoly = mesh->GetPolygon(goal);

  HandleOf<ArrayClass<NavMeshPolygonId>> polygons =
      FindPathHelper<NavMeshPolygonId, PathFinderAlgorithmMesh>(mMesh, start, goal, mMaxIterations);

  HandleOf<ArrayClass<Vec3>> output = RaverieAllocate(ArrayClass<Vec3>);
  forRange (NavMeshPolygonId polygonId, polygons->NativeArray.All())
  {
    NavMeshPolygon* polygon = mesh->GetPolygon(polygonId);
    output->NativeArray.PushBack(polygon->GetCenter(mesh));
  }
  return output;
}

HandleOf<PathFinderRequest> PathFinderMesh::FindPathThreaded(NavMeshPolygonId start, NavMeshPolygonId goal)
{
  PathFinderAlgorithmMesh* mesh = mMesh;
  // NavMeshPolygon* startPoly = mesh->GetPolygon(start);
  // NavMeshPolygon* goalPoly = mesh->GetPolygon(goal);

  return FindPathThreadedHelper<NavMeshPolygonId, PathFinderAlgorithmMesh>(mMesh, start, goal, mMaxIterations);
}

NavMeshPolygonId PathFinderMesh::WorldPositionToPolygon(Vec3Param worldPosition)
{
  PathFinderAlgorithmMesh* mesh = mMesh;
  NavMeshPolygonId closest = u32(-1);
  float closestDistance = Math::PositiveMax();

  typedef Pair<NavMeshPolygonId, NavMeshPolygon*> PairEntry;
  forRange (PairEntry& entry, mesh->mPolygons.All())
  {
    NavMeshPolygon* polygon = entry.second;
    Vec3 center = polygon->GetCenter(mesh);
    float currDistance = Math::DistanceSq(worldPosition, center);

    if (currDistance < closestDistance)
    {
      closest = entry.first;
      closestDistance = currDistance;
    }
  }

  return closest;
}

NavMeshPolygonId PathFinderMesh::LocalPositionToPolygon(Vec3Param localPosition)
{
  return 0;
}

Vec3 PathFinderMesh::PolygonToWorldPosition(NavMeshPolygonId polygonId)
{
  PathFinderAlgorithmMesh* mesh = mMesh;
  if (NavMeshPolygon* polygon = mesh->GetPolygon(polygonId))
    return polygon->GetCenter(mesh);
  return Vec3::cZero;
}

} // namespace Raverie
