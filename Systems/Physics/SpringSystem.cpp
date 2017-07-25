///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

void PointNode::AddNeighbor(uint adjacentPoint)
{
  AdjacencyInfo info;
  info.mJumps = 1;

  mAdjacentPoints[adjacentPoint] = info;
  //mAdjacentPoints.PushBack(info);
}

void PointGraph::AddPoint()
{
  mNodes.PushBack(PointNode());
}

void PointGraph::SetSize(uint pointCount)
{
  mNodes.Resize(pointCount);
  PointNode::AdjacencyInfo info;
  info.mJumps = pointCount + 1;
  for(uint i = 0; i < mNodes.Size(); ++i)
  {
    mNodes[i].mAdjacentPoints.Resize(pointCount,info);
  }
}

void PointGraph::AddEdge(uint p1, uint p2)
{
  mNodes[p1].AddNeighbor(p2);
  mNodes[p2].AddNeighbor(p1);
}

uint& PointGraph::operator()(uint x, uint y)
{
  return mNodes[x].mAdjacentPoints[y].mJumps;
}

uint& PointGraph::Get(uint x, uint y)
{
  return (*this)(x,y);
}

void PointGraph::Build()
{
  //warshall's algorithm
  for(uint i = 0; i < mNodes.Size(); ++i)
    Get(i,i) = 0;

  for(uint i = 0; i < mNodes.Size(); ++i)
  {
    for(uint j = 0; j < mNodes.Size(); ++j)
    {
      for(uint k = 0; k < mNodes.Size(); ++k)
      {
        uint& a = Get(j,k);
        uint b = Get(i,j);
        uint c = Get(i,k);
        if(b + c < a)
          a = b + c;
      }
    }
  }
}
//-------------------------------------------------------------------

ZilchDefineType(SpringSystem, builder, type)
{
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(Cog);

  ZilchBindFieldProperty(mCorrectionPercent);
  ZilchBindFieldProperty(mPointInvMass);
  ZilchBindGetterSetterProperty(SortOrder);
  ZilchBindGetterSetterProperty(DebugDrawMode);
  ZilchBindGetterSetterProperty(DebugDrawType);
}

void SpringSystem::Serialize(Serializer& stream)
{
  SerializeNameDefault(mPointMasses, PointMasses());
  SerializeNameDefault(mEdges, Edges());
  SerializeNameDefault(mFaces, Faces());
  //figure out how I want to serialize this later (so it doesn't screw up the rope and whatnot)
  //SerializeNameDefault(mSystemConnections, SystemConnections());

  SerializeNameDefault(mAnchors, AnchorPoints());

  SerializeNameDefault(mCorrectionPercent, real(1.0));
  SerializeNameDefault(mPointInvMass, real(1.0));
  SerializeEnumName(SpringSortOrder, mSortOrder);
  SerializeEnumName(SpringDebugDrawMode, mDebugDrawMode);
  SerializeEnumName(SpringDebugDrawType, mDebugDrawType);
}

void SpringSystem::Initialize(CogInitializer& initializer)
{
  PhysicsSpace* space = GetSpace()->has(PhysicsSpace);
  space->AddComponent(this);
}

void SpringSystem::OnAllObjectsCreated(CogInitializer& initializer)
{
  for(uint i = 0; i < mAnchors.Size(); ++i)
  {
    AnchorPoint* anchor = mAnchors[i];
    anchor->OnAllObjectsCreated(initializer);
    
    Cog* anchorCog = anchor->mAnchorObject;
    //if the anchor is invalid then don't hook it up to the point mass
    if(anchorCog != nullptr)
    {
      PointMass& pointMass = mPointMasses[anchor->mIndex];
      pointMass.mAnchor = anchor;
      pointMass.mInvMass = 0;

      //fix the point mass' position to the our anchor point
      Transform* t = anchorCog->has(Transform);
      if(t != nullptr)
        pointMass.mPosition = t->TransformPoint(anchor->mLocalAnchorPoint);
    }
  }

  //remove any invalid anchors
  for(int i = mAnchors.Size() - 1; i >= 0; --i)
  {
    AnchorPoint* anchor = mAnchors[i];
    Cog* anchorCog = anchor->mAnchorObject;
    if(anchorCog == nullptr)
    {
      delete anchor;
      mAnchors.EraseAt(i);
    }
  }

  if(mSortOrder != SpringSortOrder::None)
    SortEdges();
}

void SpringSystem::TransformUpdate(TransformUpdateInfo& info)
{

}

void SpringSystem::OnDestroy(uint flags)
{
  PhysicsSpace* space = GetSpace()->has(PhysicsSpace);
  space->RemoveComponent(this);

  for(uint i = 0; i < mAnchors.Size(); ++i)
  {
    delete mAnchors[i];
  }

  //have to clean up both owned and connected edges since we don't know if we'll
  //be destroyed before or after the other side is destroyed
  //(and we can't clean up if the other side was already destroyed)
  while(!mOwnedEdges.Empty())
  {
    SystemConnection& connection = mOwnedEdges.Front();
    RemoveConnection(&connection);
  }
  while(!mConnectedEdges.Empty())
  {
    SystemConnection& connection = mConnectedEdges.Front();
    RemoveConnection(&connection);
  }
}

void SpringSystem::DebugDraw()
{
  if(mDebugDrawMode == SpringDebugDrawMode::None)
    return;

  //we draw the edges in their sorted order so we have to do some special stuff
  if(mDebugDrawType == SpringDebugDrawType::Sorted)
  {
    //first figure out the max anchor distance
    uint maxDistance = 1;
    for(uint i = 0; i < mEdges.Size(); ++i)
    {
      Edge& edge = mEdges[i];
      maxDistance = Math::Max(maxDistance, Math::Min(edge.mIndex0AnchorDistance, edge.mIndex1AnchorDistance));
    }

    //then draw all edges with a weighted coloring based upon the anchor distance
    Vec4 color = Vec4(1, 1, 1, 1);
    for(uint i = 0; i < mEdges.Size(); ++i)
    {
      Edge& edge = mEdges[i];
      PointMass& p0 = mPointMasses[edge.mIndex0];
      PointMass& p1 = mPointMasses[edge.mIndex1];

      real minAnchorDistance = (real)Math::Min(edge.mIndex0AnchorDistance, edge.mIndex1AnchorDistance);
      real weight = minAnchorDistance / (real)maxDistance;
      Vec4 tempColor = Vec4(weight, weight, weight, 1);
      gDebugDraw->Add(Debug::Line(p0.mPosition, p1.mPosition).Color(tempColor));
    }
  }
  //normal drawing
  else
  {
    for(uint i = 0; i < mEdges.Size(); ++i)
    {
      Edge& edge = mEdges[i];
      PointMass& p0 = mPointMasses[edge.mIndex0];
      PointMass& p1 = mPointMasses[edge.mIndex1];

      gDebugDraw->Add(Debug::Line(p0.mPosition, p1.mPosition));
    }
  }

  //always just draw the connected edges if we aren't in no drawing mode

  //need to not accidentally use a broken connection, so for simplicity just prune the list
  UpdateConnections();

  //only render owned edges (so they don't get drawn twice)
  for(OwnedEdgeList::range range = mOwnedEdges.All(); !range.Empty(); range.PopFront())
  {
    SystemConnection& connection = range.Front();

    for(uint edgeIndex = 0; edgeIndex < connection.mEdges.Size(); ++edgeIndex)
    {
      Edge& edge = connection.mEdges[edgeIndex];

      PointMass& p0 = connection.mOwningSystem->mPointMasses[edge.mIndex0];
      PointMass& p1 = connection.mOtherSystem->mPointMasses[edge.mIndex1];

      gDebugDraw->Add(Debug::Line(p0.mPosition, p1.mPosition));
    }
  }
}

void SpringSystem::RemoveConnection(SystemConnection* connection)
{
  //remove this connection from both lists and then delete it
  OwnedEdgeList::Unlink(connection);
  ConnectedEdgeList::Unlink(connection);
  delete connection;
}

void SpringSystem::UpdateConnections()
{
  //We only care about updating owned edges so that they are
  //valid since those are the only ones we solve. This allows us
  //to assume all edges are valid during the solving step.
  OwnedEdgeList::range range = mOwnedEdges.All();
  for(; !range.Empty(); range.PopFront())
  {
    SystemConnection& connection = range.Front();

    //if one of the systems is invalid or doesn't contain a spring system then remove the connection
    Cog* owningCog = connection.mOwningSystemId;
    if(owningCog == nullptr)
    {
      RemoveConnection(&connection);
      continue;
    }

    connection.mOwningSystem = owningCog->has(SpringSystem);
    if(connection.mOwningSystem == nullptr) 
    {
      RemoveConnection(&connection);
      continue;
    }

    Cog* otherCog = connection.mOtherSystemId;
    if(otherCog == nullptr)
    {
      RemoveConnection(&connection);
      continue;
    }

    connection.mOtherSystem = otherCog->has(SpringSystem);
    if(connection.mOtherSystem == nullptr) 
    {
      RemoveConnection(&connection);
      continue;
    }
  }
}

void SpringSystem::UpdateAnchors()
{
  for(uint i = 0; i < mAnchors.Size(); ++i)
  {
    AnchorPoint* anchor = mAnchors[i];
    PointMass& point = mPointMasses[anchor->mIndex];

    Cog* anchorObj = anchor->mAnchorObject;
    //if the anchor is invalid then reset the point's mass
    if(anchorObj == nullptr)
    {
      point.mInvMass = mPointInvMass;
      continue;
    }

    //otherwise the anchor is valid so clear the mass and update the position of the point
    point.mInvMass = real(0.0);

    Transform* t = anchorObj->has(Transform);
    if(t != nullptr)
      point.mPosition = t->TransformPoint(anchor->mLocalAnchorPoint);
  }
}

void SpringSystem::SolveInternalForces()
{

}

void SpringSystem::SolveSpringForces()
{
  //figure out how to have physically based springs and the jakobsen springs
  //for(uint i = 0; i < mEdges.Size(); ++i)
  //  {
  //    Edge& edge = mEdges[i];
  //
  //    PointMass& p0 = mPointMasses[edge.mIndex0];
  //    PointMass& p1 = mPointMasses[edge.mIndex1];
  //
  //    //calculate the current length and normalized direction vector
  //    Vec3 d = p1.mPosition - p0.mPosition;
  //    real currLength = d.Normalize();
  //
  //    //simply calculate the spring force
  //    Vec3 springForce = -edge.mK * (currLength - edge.mRestLength) * d;
  //
  //    //calculate the drag force slightly differently that discussed in class, only measure the
  //    //relative velocity in the direction of the spring (that's what the dot product is for)
  //    Vec3 v = p1.mVelocity - p0.mVelocity;
  //    Vec3 dragForce = -edge.mD * Math::Dot(v, d) * d;
  //    Vec3 totalForce = springForce + dragForce;
  //
  //    p0.mForce -= totalForce;
  //    p1.mForce += totalForce;
  //  }
}

void SpringSystem::RelaxSprings()
{
  //solve all internal edges
  for(uint i = 0; i < mEdges.Size(); ++i)
  {
    Edge& edge = mEdges[i];

    PointMass& p0 = mPointMasses[edge.mIndex0];
    PointMass& p1 = mPointMasses[edge.mIndex1];

    SolveEdge(p0, p1, edge.mRestLength);
  }

  //solve all connected edges (only the ones we own to avoid a double solve)
  for(OwnedEdgeList::range range = mOwnedEdges.All(); !range.Empty(); range.PopFront())
  {
    SystemConnection& connection = range.Front();
    
    for(uint edgeIndex = 0; edgeIndex < connection.mEdges.Size(); ++edgeIndex)
    {
      Edge& edge = connection.mEdges[edgeIndex];
      
      PointMass& p0 = connection.mOwningSystem->mPointMasses[edge.mIndex0];
      PointMass& p1 = connection.mOtherSystem->mPointMasses[edge.mIndex1];
    
      SolveEdge(p0, p1, edge.mRestLength);
    }
  }
}

void SpringSystem::SolveEdge(PointMass& p0, PointMass& p1, real restLength)
{
  real invMassSum = p1.mInvMass + p0.mInvMass;
  //should pre-sort the edges so we don't solve ones with a zero inverse sum
  if(invMassSum == 0)
    return;

  //based up a Jakobsen spring which is corrected by just snapping each
  //particle to be at the rest length (after doing mass ratios and whatnot)
  Vec3 posDiff = p1.mPosition - p0.mPosition;
  real length = posDiff.Length();
  if(length == real(0.0))
    return;

  real diff = -(length - restLength);
  diff /= length * invMassSum;

  Vec3 impulse = posDiff * diff * mCorrectionPercent;

  p0.mPosition -= p0.mInvMass * impulse;
  p1.mPosition += p1.mInvMass * impulse;
}

void SpringSystem::UpdateVelocities(real dt)
{
  //since position was directly modified we have an incorrect velocity so
  //approximate the correct velocity based upon the current and old position
  real invDt = real(1.0) / dt;
  for(uint i = 0; i < mPointMasses.Size(); ++i)
  {
    PointMass& p = mPointMasses[i];
    //make sure to not update anchors
    if(p.mAnchor != nullptr)
      continue;

    p.mVelocity = (p.mPosition - p.mOldPosition) * invDt;
    p.mOldPosition = p.mPosition;
  }
}

void SpringSystem::IntegrateVelocity(real dt)
{
  //simple Euler integration
  for(uint i = 0; i < mPointMasses.Size(); ++i)
  {
    PointMass& p = mPointMasses[i];
    p.mVelocity += p.mInvMass * p.mForce * dt;
    p.mForce = Vec3::cZero;
  }
}

void SpringSystem::IntegratePosition(real dt)
{
  //simple Euler integration
  for(uint i = 0; i < mPointMasses.Size(); ++i)
  {
    PointMass& p = mPointMasses[i];
    p.mPosition += p.mVelocity * dt;
  }
}

SpringSystem::Edge& SpringSystem::AddEdge(uint index0, uint index1, real errCorrection)
{
  Vec3 pos0 = mPointMasses[index0].mPosition;
  Vec3 pos1 = mPointMasses[index1].mPosition;
  Edge& edge = mEdges.PushBack();
  edge.Set(index0, index1, pos0, pos1);
  //To help with certain systems (ropes) it's useful to add a correction term to improve stiffness.
  //This is because each link will have a small amount of error so shortening each
  //edge by a small percentage will help to mitigate that error.
  edge.mRestLength = Math::Max(edge.mRestLength - errCorrection, real(0.0));
  return edge;
}

void SpringSystem::AddPointMass(Vec3Param position)
{
  PointMass point;
  point.mOldPosition = point.mPosition = position;
  mPointMasses.PushBack(point);
}

void SpringSystem::SetPointMassAnchor(uint index, Cog* anchorCog)
{
  if(index >= mPointMasses.Size())
    return;

  PointMass& pointMass = mPointMasses[index];
  //if the anchor cog is not null then we are setting the anchor
  if(anchorCog != nullptr)
  {
    //if the point mass didn't already have an anchor then create one
    if(pointMass.mAnchor == nullptr)
    {
      AnchorPoint* newAnchor = new AnchorPoint();
      mAnchors.PushBack(newAnchor);
      //make sure to tell the anchor how to get back to the point
      newAnchor->mIndex = index;
      pointMass.mAnchor = newAnchor;
    }

    //make sure to clear out the mass so this point is now static
    pointMass.mInvMass = real(0.0);
    //hook up the anchor to the object we're connected to
    AnchorPoint& anchor = *pointMass.mAnchor;
    anchor.mAnchorObject = anchorCog;

    Transform* anchorTransform = anchorCog->has(Transform);
    //now we need to compute the local point that we are anchored to
    if(anchorTransform != nullptr)
      anchor.mLocalAnchorPoint = anchorTransform->TransformPointInverse(pointMass.mPosition);
  }
  //otherwise since it is null we are clearing the anchor
  else
  {
    //if the point mass already didn't have an anchor then don't do anything
    if(pointMass.mAnchor == nullptr)
      return;

    //set the point back to its mass and remove the anchor
    pointMass.mInvMass = mPointInvMass;
    mAnchors.EraseValueError(pointMass.mAnchor);
    pointMass.mAnchor = nullptr;
    delete pointMass.mAnchor;
  }
}

void SpringSystem::AddConnection(SpringSystem* otherSystem, uint indexA, uint indexB)
{
  SystemConnection* connection = FindConnection(otherSystem);
  //if a connection did not already exist then create one between these two systems
  if(connection == nullptr)
  {
    connection = new SystemConnection();
    mOwnedEdges.PushBack(connection);
    otherSystem->mConnectedEdges.PushBack(connection);

    connection->mOwningSystem = this;
    connection->mOwningSystemId = GetOwner();
    connection->mOtherSystem = otherSystem;
    connection->mOtherSystemId = otherSystem->GetOwner();
  }

  Vec3 pointA = mPointMasses[indexA].mPosition;
  Vec3 pointB = otherSystem->mPointMasses[indexB].mPosition;

  Edge& edge = connection->mEdges.PushBack();
  edge.Set(indexA, indexB, pointA, pointB);
}

SpringSystem::SystemConnection* SpringSystem::FindConnection(SpringSystem* otherSystem)
{
  OwnedEdgeList::range range = mOwnedEdges.All();
  for(; !range.Empty(); range.PopFront())
  {
    SystemConnection* connection = &range.Front();
    if(connection->mOtherSystem == otherSystem)
      return connection;
  }
  return nullptr;
}

struct EdgeInfo;
/// Information needed to build the adjacency
/// graph for points to do a breadth first search.
struct PointInfo
{
  // How many edge jumps it takes to get from an anchor to this point.
  uint mDistanceFromAnchor;
  // All edges that are connected to this point
  Array<EdgeInfo*> mEdges;
  // Link needed for the stack
  Link<PointInfo> link;
};

/// Information 
struct EdgeInfo
{
  PointInfo* mPoint0;
  PointInfo* mPoint1;
  uint mOriginalEdgeIndex;

  // This edge's distance from an anchor is based upon the
  // minimum distance of the two points we're connected to.
  uint GetDistance() const
  {
    return Math::Min(mPoint0->mDistanceFromAnchor, mPoint1->mDistanceFromAnchor);
  }

  bool operator<(const EdgeInfo& rhs) const
  {
    return GetDistance() < rhs.GetDistance();
  }

  bool operator>(const EdgeInfo& rhs) const
  {
    return GetDistance() > rhs.GetDistance();
  }
};

SpringSortOrder::Enum SpringSystem::GetSortOrder()
{
  return mSortOrder;
}

void SpringSystem::SetSortOrder(SpringSortOrder::Enum orderingType)
{
  mSortOrder = orderingType;
}

void SpringSystem::SortEdges()
{
  //resize the graph information we need to build adjacency graph
  Array<PointInfo> points;
  points.Resize(mPointMasses.Size());
  Array<EdgeInfo> edges;
  edges.Resize(mEdges.Size());
  //we also need a stack (of what to visit) and a map what we've already visited
  HashSet<PointInfo*> visitedPointSet;
  InList<PointInfo> stack;

  //first setup all of the point masses and initialize all anchors to
  //be at zero distance and make sure they're in the stack
  //(we need to traverse all adjacent points) and in the visited list
  for(uint i = 0; i < mPointMasses.Size(); ++i)
  {
    PointInfo& info = points[i];
    info.mDistanceFromAnchor = (uint)-1;
    if(mPointMasses[i].mAnchor != nullptr)
    {
      info.mDistanceFromAnchor = 0;
      stack.PushBack(&info);
      visitedPointSet.Insert(&info);
    }
  }
  //initialize all edges to point to the appropriate point info's and give them
  //a mapping back to the original edge (so we can sort them later)
  for(uint i = 0; i < mEdges.Size(); ++i)
  {
    EdgeInfo& info = edges[i];
    PointInfo& point0 = points[mEdges[i].mIndex0];
    PointInfo& point1 = points[mEdges[i].mIndex1];
    info.mPoint0 = &point0;
    info.mPoint1 = &point1;
    //each point needs to know what edges are connected to it
    point0.mEdges.PushBack(&info);
    point1.mEdges.PushBack(&info);
    info.mOriginalEdgeIndex = i;
  }

  //keep going as long as there are points to process
  while(!stack.Empty())
  {
    PointInfo* pointInfo = &stack.Front();
    stack.PopFront();

    //go through all edges adjacent to this point
    for(uint i = 0; i < pointInfo->mEdges.Size(); ++i)
    {
      EdgeInfo* edgeInfo = pointInfo->mEdges[i];
      //get the other point on this edge
      PointInfo* otherPoint;
      if(pointInfo == edgeInfo->mPoint0)
        otherPoint = edgeInfo->mPoint1;
      else
        otherPoint = edgeInfo->mPoint0;

      //if we've already visited the other point then we've found the distance from
      //anchors already so skip it (this distance will be >= the current one)
      if(visitedPointSet.Contains(otherPoint))
        continue;

      //update the distance of this point
      otherPoint->mDistanceFromAnchor = pointInfo->mDistanceFromAnchor + 1;
      //make sure we never visit this point again, we've found its distance
      visitedPointSet.Insert(otherPoint);
      //we have to walk all of the edges of this point so add it to the stack
      stack.PushBack(otherPoint);
    }
  }

  //sort all of the edges (choosing top-down/bottom-up is mostly
  //for debugging, top down should always be better)
  if(mSortOrder == SpringSortOrder::TopDown)
    Sort(edges.All(), less<EdgeInfo>());
  else
    Sort(edges.All(), greater<EdgeInfo>());

  //build up the new array of sorted edges by copying over the old
  //edges to the new correct positions (optimize later)
  Array<Edge> newEdges;
  newEdges.Resize(mEdges.Size());
  for(uint i = 0; i < edges.Size(); ++i)
  {
    Edge& newEdge = newEdges[i];
    newEdge = mEdges[edges[i].mOriginalEdgeIndex];
    newEdge.mIndex0AnchorDistance = edges[i].mPoint0->mDistanceFromAnchor;
    newEdge.mIndex1AnchorDistance = edges[i].mPoint1->mDistanceFromAnchor;
  }
  mEdges.Swap(newEdges);
}

SpringDebugDrawMode::Enum SpringSystem::GetDebugDrawMode()
{
  return mDebugDrawMode;
}

void SpringSystem::SetDebugDrawMode(SpringDebugDrawMode::Enum debugDrawMode)
{
  mDebugDrawMode = debugDrawMode;
}

SpringDebugDrawType::Enum SpringSystem::GetDebugDrawType()
{
  return mDebugDrawType;
}

void SpringSystem::SetDebugDrawType(SpringDebugDrawType::Enum debugDrawType)
{
  mDebugDrawType = debugDrawType;
}

bool SpringSystem::Cast(RayParam ray, Face& resultFace, Vec3Ref intersectionPoint)
{
  int closestFace = -1;
  real closestDistance = Math::PositiveMax();

  //find the face that we hit first
  for(uint i = 0; i < mFaces.Size(); ++i)
  {
    SpringSystem::Face& face = mFaces[i];

    Triangle tri;
    tri.p0 = mPointMasses[face.mIndex0].mPosition;
    tri.p1 = mPointMasses[face.mIndex1].mPosition;
    tri.p2 = mPointMasses[face.mIndex2].mPosition;

    //see if the ray intersects this triangle
    Intersection::IntersectionPoint point;
    Intersection::Type result = Intersection::RayTriangle(ray.Start, ray.Direction, tri.p0, tri.p1, tri.p2, &point);
    if(result < (Intersection::Type)0)
      continue;

    //if this intersection happens sooner than the last one then save it
    if(point.T < closestDistance)
    {
      closestDistance = point.T;
      closestFace = i;
      intersectionPoint = point.Points[0];
    }
  }

  //if we never overwrote the closest face then we didn't hit anything
  if(closestFace < 0)
    return false;

  //otherwise fill out the results from the closest face
  resultFace = mFaces[closestFace];
  return true;
}

//-------------------------------------------------------------------SpringSystem::PointMass
void SpringSystem::PointMass::Serialize(Serializer& stream)
{
  SerializeNameDefault(mInitialOffset, Vec3::cZero);
  SerializeNameDefault(mPosition, Vec3::cZero);

  //if we're loading then we need to start the old position at our current position
  if(stream.GetMode() == SerializerMode::Loading)
    mOldPosition = mPosition;
}

//-------------------------------------------------------------------SpringSystem::Edge
void SpringSystem::Edge::Serialize(Serializer& stream)
{
  SerializeNameDefault(mIndex0, 0u);
  SerializeNameDefault(mIndex1, 0u);
  SerializeNameDefault(mRestLength, real(0.0));
}

//-------------------------------------------------------------------SpringSystem::Face
void SpringSystem::Face::Serialize(Serializer& stream)
{
  SerializeNameDefault(mIndex0, 0u);
  SerializeNameDefault(mIndex1, 0u);
  SerializeNameDefault(mIndex2, 0u);
}

//-------------------------------------------------------------------SpringSystem::AnchorPoint
void SpringSystem::AnchorPoint::Serialize(Serializer& stream)
{
  SerializeNameDefault(mIndex, 0u);
  SerializeNameDefault(mAnchorObject, CogId());
  SerializeNameDefault(mLocalAnchorPoint, Vec3::cZero);
}

void SpringSystem::AnchorPoint::OnAllObjectsCreated(CogInitializer& initializer)
{
  //restore the CogId to our anchored object
  mAnchorObject.OnAllObjectsCreated(initializer);
}

//-------------------------------------------------------------------SpringSystem::SystemConnection
void SpringSystem::SystemConnection::Serialize(Serializer& stream)
{
  SerializeNameDefault(mOwningSystemId, CogId());
  SerializeNameDefault(mOtherSystemId, CogId());
  SerializeNameDefault(mEdges, Edges());
}

void SpringSystem::SystemConnection::OnAllObjectsCreated(CogInitializer& initializer)
{
  //restore our CogIds to the systems we're connected to
  mOwningSystemId.OnAllObjectsCreated(initializer);
  mOtherSystemId.OnAllObjectsCreated(initializer);
}

//-------------------------------------------------------------------SpringMassAggregate
ZilchDefineType(DecorativeCloth, builder, type)
{
  ZeroBindInterface(SpringSystem);
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindComponent();

  ZilchBindMethod(LoadFromMesh);
  ZilchBindGetterSetterProperty(Mesh);

  ZilchBindFieldProperty(mJakobsen);
  ZilchBindGetterSetterProperty(SpringStiffness);
  ZilchBindGetterSetterProperty(SpringDamping);
  ZilchBindGetterSetterProperty(ConnectivityCounter);

  ZilchBindMethodProperty(ResetMeshPositions);
}

void DecorativeCloth::Serialize(Serializer& stream)
{
  SpringSystem::Serialize(stream);

  SerializeNameDefault(mSpringStiffness, real(1000));
  SerializeNameDefault(mSpringDamping, real(3));
  SerializeNameDefault(mConnectivityCounter, 1u);
  SerializeNameDefault(mJakobsen, true);

  if(stream.GetMode() == SerializerMode::Loading)
  {
    for(uint i = 0; i < mEdges.Size(); ++i)
    {
      mEdges[i].mD = mSpringDamping;
      mEdges[i].mK = mSpringStiffness;
    }
  }
}

void DecorativeCloth::Initialize(CogInitializer& initializer)
{
  SpringSystem::Initialize(initializer);
}

void DecorativeCloth::OnAllObjectsCreated(CogInitializer& initializer)
{
  bool dynamicallyCreated = (initializer.Flags & CreationFlags::DynamicallyAdded) != 0;
  //if we weren't dynamically created then just call the base class (which will fix our anchors)
  if(dynamicallyCreated == false)
  {
    SpringSystem::OnAllObjectsCreated(initializer);
    return;
  }

  //otherwise, we need to load our initial data
  LoadFromMesh(mMesh, false);
}

void DecorativeCloth::TransformUpdate(TransformUpdateInfo& info)
{
  if(GetSpace()->IsEditorMode() == false)
    return;

  PhysicsMesh* mesh = mMesh;
  if(mesh != nullptr)
    LoadPointMeshData(mesh->GetVertexArray(), false);
}

void DecorativeCloth::ResetMeshPositions()
{
  LoadFromMesh(mMesh, true);
}

void DecorativeCloth::UpdatePointMassPosition(uint index, Vec3Param position)
{
  if(index >= mPointMasses.Size())
    return;

  PointMass& pointMass = mPointMasses[index];
  pointMass.mPosition = position;

  Transform* t = GetOwner()->has(Transform);
  Vec3 localPoint = t->TransformPointInverse(pointMass.mPosition);

  Vec3 initPos;
  initPos = mMesh->GetVertexArray()[index];
  pointMass.mInitialOffset = localPoint - initPos;
}

void DecorativeCloth::UpdateAnchorPoint(uint index, Vec3Param position, Cog* anchorCog)
{
  if(index >= mPointMasses.Size())
    return;

  PointMass& pointMass = mPointMasses[index];
  pointMass.mPosition = position;

  Transform* t = GetOwner()->has(Transform);
  Vec3 localPoint = t->TransformPointInverse(pointMass.mPosition);

  Vec3 initPos;
  initPos = mMesh->GetVertexArray()[index];
  pointMass.mInitialOffset = localPoint - initPos;

  AnchorPoint* anchor = pointMass.mAnchor;
  if(anchor != nullptr && anchorCog != nullptr)
  {
    Transform* anchorTransform = anchorCog->has(Transform);
    anchor->mLocalAnchorPoint = anchorTransform->TransformPointInverse(pointMass.mPosition);
  }
}

void DecorativeCloth::LoadFromMesh(PhysicsMesh* mesh, bool clearOldData)
{
  if(mesh == nullptr)
    return;

  const Vec3Array& verts = mesh->GetVertexArray();
  const PhysicsMesh::IndexArray& indices = mesh->GetIndexArray();

  LoadFromMeshData(verts, indices, clearOldData);
}

void DecorativeCloth::LoadPointMeshData(const Array<Vec3>& verts, bool clearOldData)
{
  if(mPointMasses.Size() == 0)
    clearOldData = true;
  if(clearOldData)
    mPointMasses.Clear();

  if(mPointMasses.Size() != verts.Size())
    mPointMasses.Resize(verts.Size());

  Transform* t = GetOwner()->has(Transform);

  for(uint i = 0; i < verts.Size(); ++i)
  {
    PointMass* point;
    point = &mPointMasses[i];

    point->mPosition = t->TransformPoint(verts[i] + point->mInitialOffset);
    point->mOldPosition = point->mPosition;
  }
}

void DecorativeCloth::LoadFromMeshData(const Array<Vec3>& verts, const Array<uint>& indices, bool clearOldData)
{
  LoadPointMeshData(verts, clearOldData);

  mEdges.Clear();
  mFaces.Clear();

  PointGraph graph;
  graph.SetSize(mPointMasses.Size());

  for(uint i = 0; i < indices.Size(); i += 3)
  {
    uint index0 = indices[i];
    uint index1 = indices[i + 1];
    uint index2 = indices[i + 2];

    graph.AddEdge(index0, index1);
    graph.AddEdge(index0, index2);
    graph.AddEdge(index1, index2);

    Face face;
    face.mIndex0 = index0;
    face.mIndex1 = index1;
    face.mIndex2 = index2;
    mFaces.PushBack(face);
  }

  //should probably do a breadth first search at some point instead of this
  if(mConnectivityCounter != 1)
    graph.Build();

  for(uint i = 0; i < mPointMasses.Size(); ++i)
  {
    for(uint j = i + 1; j < mPointMasses.Size(); ++j)
    {
      PointNode::AdjacencyInfo& adjacencyInfo = graph.mNodes[i].mAdjacentPoints[j];
      if(adjacencyInfo.mJumps > mConnectivityCounter)
        continue;

      AddEdge(i, j);
    }
  }
}

void DecorativeCloth::AddEdge(uint index0, uint index1, real errCorrection)
{
  Edge& edge = SpringSystem::AddEdge(index0, index1, errCorrection);
  edge.mK = mSpringStiffness;
  edge.mD = mSpringDamping;
}

void DecorativeCloth::Commit()
{
  UploadToMesh();
}

void DecorativeCloth::UploadToMesh()
{
  if(mDebugDrawType != SpringDebugDrawMode::None)
    DebugDraw();
}

PhysicsMesh* DecorativeCloth::GetMesh()
{
  return mMesh;
}

void DecorativeCloth::SetMesh(PhysicsMesh* mesh)
{
  mMesh = mesh;
  
  LoadFromMesh(mesh, true);
}

uint DecorativeCloth::GetConnectivityCounter()
{
  return mConnectivityCounter;
}

void DecorativeCloth::SetConnectivityCounter(uint counter)
{
  if(counter == mConnectivityCounter || counter == 0)
    return;

  mConnectivityCounter = counter;

  if(mMesh)
    LoadFromMesh(mMesh, false);
}

real DecorativeCloth::GetSpringStiffness()
{
  return mSpringStiffness;
}

void DecorativeCloth::SetSpringStiffness(real stiffness)
{
  mSpringStiffness = stiffness;
}

real DecorativeCloth::GetSpringDamping()
{
  return mSpringDamping;
}

void DecorativeCloth::SetSpringDamping(real damping)
{
  mSpringDamping = damping;
}

//-------------------------------------------------------------------SpringRope
ZilchDefineType(DecorativeRope, builder, type)
{
  ZeroBindInterface(SpringSystem);
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindComponent();

  ZilchBindGetterSetterProperty(NumberOfLinks);
  ZilchBindFieldProperty(mErrorCorrection);
  ZilchBindFieldProperty(mAnchorA);
  ZilchBindFieldProperty(mAnchorB);
}

void DecorativeRope::Serialize(Serializer& stream)
{
  SpringSystem::Serialize(stream);

  SerializeNameDefault(mNumberOfLinks, 5u);
  SerializeNameDefault(mErrorCorrection, real(0.1));

  SerializeNameDefault(mCogA, CogId());
  SerializeNameDefault(mCogB, CogId());
  SerializeNameDefault(mPointMassIndexA, 0u);
  SerializeNameDefault(mPointMassIndexB, 0u);
  SerializeNameDefault(mLocalPointA, Vec3::cZero);
  SerializeNameDefault(mLocalPointB, Vec3::cZero);
  SerializeNameDefault(mAnchorA, true);
  SerializeNameDefault(mAnchorB, true);
}

void DecorativeRope::Initialize(CogInitializer& initializer)
{
  SpringSystem::Initialize(initializer);
}

void DecorativeRope::OnAllObjectsCreated(CogInitializer& initializer)
{
  mCogA.OnAllObjectsCreated(initializer);
  mCogB.OnAllObjectsCreated(initializer);

  //only create our links when the game is played
  if(GetSpace()->IsEditorMode())
    return;

  CreateLinks();
  SpringSystem::OnAllObjectsCreated(initializer);
}

void DecorativeRope::DebugDraw()
{
  //if we're in the editor then we don't actually have any edges to
  //debug draw, so just draw an edge from our two connection points
  if(GetSpace()->IsEditorMode())
  {
    Cog* cogA = mCogA;
    Cog* cogB = mCogB;
    if(cogA == nullptr || cogB == nullptr)
      return;

    //need a dummy to use the helper function
    SpringSystem* dummySystem;
    Vec3 worldPointA = GetWorldPoint(cogA, mLocalPointA, mPointMassIndexA, dummySystem);
    Vec3 worldPointB = GetWorldPoint(cogB, mLocalPointB, mPointMassIndexB, dummySystem);

    gDebugDraw->Add(Debug::Line(worldPointA, worldPointB));
    return;
  }

  //otherwise just call our normal debug draw
  SpringSystem::DebugDraw();
}

void DecorativeRope::Commit()
{
  if(mDebugDrawType != SpringDebugDrawMode::None)
    DebugDraw();
}

void DecorativeRope::GetPoints(Array<Vec3>& points)
{
  points.Clear();

  Cog* cogA = mCogA;
  Cog* cogB = mCogB;
  if(cogA == nullptr || cogB == nullptr)
    return;

  //if a cog is a spring system then we are missing a point mass in our list since
  //it belongs to the other object. If that's the case then we have to manually add
  //it back in either at the beginning or the end so that we have the full length of the rope.
  SpringSystem* systemA = nullptr;
  SpringSystem* systemB = nullptr;
  Vec3 worldPointA = GetWorldPoint(cogA, mLocalPointA, mPointMassIndexA, systemA);
  Vec3 worldPointB = GetWorldPoint(cogB, mLocalPointB, mPointMassIndexB, systemB);

  if(systemA != nullptr)
    points.PushBack(worldPointA);
  for(uint i = 0; i < mPointMasses.Size(); ++i)
    points.PushBack(mPointMasses[i].mPosition);
  if(systemB != nullptr)
    points.PushBack(worldPointB);
}

Vec3 DecorativeRope::GetWorldPoint(Cog* cog, Vec3Param localPoint, uint pointMassIndex, SpringSystem*& resultingSystem)
{
  //if the cog has a spring system then return the point mass at the given index
  resultingSystem = cog->has(SpringSystem);
  if(resultingSystem != nullptr)
    return resultingSystem->mPointMasses[pointMassIndex].mPosition;

  //otherwise if the cog has a transform the local point
  Transform* t = cog->has(Transform);
  if(t != nullptr)
    return t->TransformPoint(localPoint);

  return Vec3::cZero;
}

void DecorativeRope::CreateLinks()
{
  Cog* cogA = mCogA;
  Cog* cogB = mCogB;
  if(cogA == nullptr || cogB == nullptr)
    return;

  mPointMasses.Clear();
  mEdges.Clear();

  //Since we might connect to another object or a spring system, we might not actually
  //create all of the normal links. If we're connected to a spring system we'll
  //have one less point mass and spring on each side.
  uint startIndex = 0;
  uint endIndex = mNumberOfLinks;

  //Get the world points of each connection and if they had a system
  SpringSystem* systemA = nullptr;
  SpringSystem* systemB = nullptr;
  Vec3 worldPointA = GetWorldPoint(cogA, mLocalPointA, mPointMassIndexA, systemA);
  Vec3 worldPointB = GetWorldPoint(cogB, mLocalPointB, mPointMassIndexB, systemB);

  //if we had a system on either side then shrink that index by 1
  if(systemA != nullptr)
    ++startIndex;
  if(systemB != nullptr)
    --endIndex;

  //figure out how many links we'll actually create and get how long each one is
  uint linksToCreate = endIndex - startIndex;
  Vec3 dir = worldPointB - worldPointA;
  //while we may choose to create a smaller number of links and point masses
  //(due to connections to other springs) the length of each edge should always
  //be based upon the total number of links (since we'll be creating that many edges no matter what)
  dir /= (real)mNumberOfLinks;
  real linkLength = Math::Length(dir);

  //create each point mass along the direction vector between our anchor points
  for(uint i = startIndex; i <= endIndex; ++i)
  {
    Vec3 position = worldPointA + dir * (real)i;
    AddPointMass(position);
  }

  //add an edge between each point mass we actually created (this will exclude the edges to other spring systems)
  for(uint i = 1; i <= linksToCreate; ++i)
    AddEdge(i - 1, i, mErrorCorrection);

  //If we had a system on an edge then we need to add the connection
  //spring between our point and the system's point. Otherwise we need
  //to anchor that end to the cog we're connected to.
  if(systemA != nullptr)
    AddConnection(systemA, 0, mPointMassIndexA);
  else if(mAnchorA)
    SetPointMassAnchor(0, cogA);

  if(systemB != nullptr)
    AddConnection(systemB, mPointMasses.Size() - 1, mPointMassIndexB);
  else if(mAnchorB)
    SetPointMassAnchor(mPointMasses.Size() - 1, cogB);
}

uint DecorativeRope::GetNumberOfLinks()
{
  return mNumberOfLinks;
}

void DecorativeRope::SetNumberOfLinks(uint linkNumber)
{
  //To make life easier, we can have at min 2 links, otherwise we'd be creating only one
  //link without connecting to any internal point masses. This would create another branch in setup that we want to avoid.
  mNumberOfLinks = Math::Clamp(linkNumber, 2u, 100u);
}

void SpringGroup::Solve(PhysicsSpace* space, real dt)
{
  //update all anchors and effects (accumulate forces)
  SpringSystems::range range = mSystems.All();
  for(; !range.Empty(); range.PopFront())
  {
    SpringSystem& system = range.Front();

    system.UpdateAnchors();

    ApplyGlobalEffects(space, &system);
    system.IntegrateVelocity(dt);
    system.IntegratePosition(dt);
  }

  //solve the springs together (it's important that this is interleaved)
  for(uint iteration = 0; iteration < 4; ++iteration)
  {
    range = mSystems.All();
    for(; !range.Empty(); range.PopFront())
    {
      SpringSystem& system = range.Front();
      //if(system.mJakobsen)
        system.RelaxSprings();
      //else
      //  system.SolveInternalForces();
    }
  }
  
  //now finish off with integration and committing
  range = mSystems.All();
  for(; !range.Empty(); range.PopFront())
  {
    SpringSystem& system = range.Front();

    system.UpdateVelocities(dt);
    system.Commit();
  }
}

void SpringGroup::ApplyGlobalEffects(PhysicsSpace* space, SpringSystem* system)
{
  real dt = space->mIterationDt;

  //if the system has an IgnoreSpaceEffects component then make
  //sure an effect isn't ignored before we apply it
  IgnoreSpaceEffects* effectsToIgnore = system->GetOwner()->has(IgnoreSpaceEffects);
  if(effectsToIgnore != nullptr)
  {
    PhysicsEffectList::range effects = space->GetGlobalEffects();
    for(; !effects.Empty(); effects.PopFront())
    {
      PhysicsEffect& effect = effects.Front();
      if(!effectsToIgnore->IsIgnored(&effect) && effect.GetActive())
        effect.ApplyEffect(system, dt);
    }
  }

  //otherwise apply all of the effects as long as they are active
  PhysicsEffectList::range effects = space->GetGlobalEffects();
  for(; !effects.Empty(); effects.PopFront())
  {
    PhysicsEffect& effect = effects.Front();
    if(effect.GetActive())
      effect.ApplyEffect(system, dt);
  }
}

}//namespace Zero
