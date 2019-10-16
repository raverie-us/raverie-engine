// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

const bool cDebugQuickHull = false;
#define TimeQuickHull 0
#if TimeQuickHull
#  define QHullScopedTimer(message) ProfileScope(message)
#else
#  define QHullScopedTimer(message)
#endif

template <typename ListType>
size_t CountInList(ListType& list)
{
  size_t count = 0;
  for (typename ListType::range range = list.All(); !range.Empty(); range.PopFront())
    ++count;
  return count;
}

bool IsWithinEpsilon(Vec3Param point, const Array<Vec3>& points, real epsilon)
{
  real epsilonSq = epsilon * epsilon;
  for (size_t i = 0; i < points.Size(); ++i)
  {
    real distSq = Math::DistanceSq(points[i], point);
    if (distSq <= epsilonSq)
      return true;
  }
  return false;
}

QuickHull3D::QuickHullVertex::QuickHullVertex()
{
  mPosition = Vec3::cZero;
  mConflictDistance = -Math::PositiveMax();
}

QuickHull3D::QuickHullEdge::QuickHullEdge()
{
  mTail = nullptr;
  mTwin = nullptr;
  mFace = nullptr;
}

QuickHull3D::QuickHullFace::QuickHullFace()
{
  mCenter = Vec3::cZero;
  mNormal = Vec3::cZero;
}

void QuickHull3D::QuickHullFace::RecomputeCenterAndNormal()
{
  // Use Newell's method to compute the face normal.
  mCenter = Vec3::cZero;
  mNormal = Vec3::cZero;
  if (mEdges.Empty())
    return;

  QuickHullEdge* prevEdge = &mEdges.Back();
  int count = 0;
  for (EdgeList::range edges = mEdges.All(); !edges.Empty(); edges.PopFront())
  {
    QuickHullEdge* currEdge = &edges.Front();
    Vec3 p0 = prevEdge->mTail->mPosition;
    Vec3 p1 = currEdge->mTail->mPosition;
    Vec3 diff = p0 - p1;

    mNormal.x += (p0.y - p1.y) * (p0.z + p1.z);
    mNormal.y += (p0.z - p1.z) * (p0.x + p1.x);
    mNormal.z += (p0.x - p1.x) * (p0.y + p1.y);
    mCenter += p1;
    ++count;
    prevEdge = currEdge;
  }

  mNormal.AttemptNormalize();
  mCenter /= (real)count;
}

bool QuickHull3D::QuickHullFace::IsConvexTo(QuickHullFace* other, real epsilon)
{
  // A face is convex to another if its center is strictly behind the
  // thick plane. Do this test symmetrically (a->b and b->a) for numerical
  // reasons.
  real face1Convex = Math::Dot(other->mCenter - this->mCenter, this->mNormal) < -epsilon;
  real face0Convex = Math::Dot(this->mCenter - other->mCenter, other->mNormal) < -epsilon;
  if (face0Convex && face1Convex)
    return true;
  return false;
}

size_t QuickHull3D::QuickHullFace::CountEdges()
{
  return CountInList(mEdges);
}

QuickHull3D::QuickHull3D()
{
  mVertexPool = nullptr;
  mEdgePool = nullptr;
  mFacePool = nullptr;
}

QuickHull3D::~QuickHull3D()
{
  Clear();
}

bool QuickHull3D::Build(const Array<Vec3>& points, DebugDrawStack* stack)
{
  QHullScopedTimer("Hull time:");

  // Clear the previous data in-case this hull is being re-used
  Clear();
  // Cache the debug drawing stack object (used to determine if we run debug
  // drawing steps)
  mDebugDrawStack = stack;

  // Can't construct a hull from less than 4 points
  if (points.Size() < 4)
    return false;

  // Allocate the memory pools for this run
  AllocatePools(points);

  // Turn all of the points into a working format (allocates vertices and
  // performs vertex welding)
  size_t resultPointCount;
  if (!BuildDataSet(points, resultPointCount))
    return false;

  // After vertex welding we dropped to less than 4 points. We can no longer
  // make a hull
  if (resultPointCount < 4)
    return false;

  // Compute a tetrahedron from the initial hull. Fails if
  // all points are on a line or plane (can't build a hull then).
  if (!BuildInitialHull())
    return false;

  // Partition all vertices to their "best" face. This allows
  // us to efficiently check for furthest away points.
  ComputeInitialConflictLists();

  // Iteratively find a vertex that is outside the hull and then add it to the
  // hull until no vertices remain.
  QuickHullVertex* conflictVertex = nullptr;
  QuickHullFace* conflictFace = nullptr;
  FindNextConflictVertex(conflictVertex, conflictFace);
  while (conflictVertex != nullptr)
  {
    // Draw a sub-step for debugging
    DrawFoundConflictVertex(conflictVertex, conflictFace);

    AddVertexToHull(conflictVertex, conflictFace);
    FindNextConflictVertex(conflictVertex, conflictFace);
  }

  // Debug drawing and validation for the final hull
  DrawFinalHull(points);
  if (cDebugQuickHull)
    ValidateFinalHull(points);

  return true;
}

void QuickHull3D::Clear()
{
  SafeDelete(mVertexPool);
  SafeDelete(mEdgePool);
  SafeDelete(mFacePool);
}

size_t QuickHull3D::ComputeVertexCount()
{
  size_t count = 0;

  // Compute the unique set of final vertices (faster to do at the end than
  // iteratively during the algorithm)
  HashSet<QuickHullVertex*> uniqueVertices;
  for (FaceList::range faces = mFaces.All(); !faces.Empty(); faces.PopFront())
  {
    QuickHullFace* face = &faces.Front();
    for (EdgeList::range edges = face->mEdges.All(); !edges.Empty(); edges.PopFront())
    {
      QuickHullEdge* edge = &edges.Front();
      QuickHullVertex* vertex = edge->mTail;
      if (!uniqueVertices.Contains(vertex))
      {
        uniqueVertices.Insert(vertex);
        ++count;
      }
    }
  }

  return count;
}

size_t QuickHull3D::ComputeEdgeCount()
{
  return ComputeHalfEdgeCount() / 2;
}

size_t QuickHull3D::ComputeHalfEdgeCount()
{
  // Sum all half-edges on all active faces
  size_t count = 0;
  for (FaceList::range faces = mFaces.All(); !faces.Empty(); faces.PopFront())
  {
    QuickHullFace* face = &faces.Front();
    count += CountInList(face->mEdges);
  }
  return count;
}

size_t QuickHull3D::ComputeFaceCount()
{
  return CountInList(mFaces);
}

QuickHull3D::FaceList::range QuickHull3D::GetFaces()
{
  return mFaces.All();
}

void QuickHull3D::AllocatePools(const Array<Vec3>& points)
{
  // Currently just used some fixed sized buffers. While this isn't as fast as
  // allocating the max memory up-front, this helps prevent crashes when the
  // initial vertex count is too large.
  size_t vertexPoolCount = 1024;
  size_t edgePoolCount = 1024;
  size_t facePoolCount = 1024;
  mVertexPool = new Memory::Pool("VertexPool", nullptr, sizeof(QuickHullVertex), vertexPoolCount, true);
  mEdgePool = new Memory::Pool("EdgePool", nullptr, sizeof(QuickHullEdge), edgePoolCount, true);
  mFacePool = new Memory::Pool("FacePool", nullptr, sizeof(QuickHullFace), facePoolCount, true);
}

void QuickHull3D::ComputeEpsilon(const Array<Vec3>& points)
{
  // Keep track of the largest absolute value on each axis (needed to compute a
  // proper epsilon)
  Vec3 maxVals = Vec3::cZero;
  for (size_t i = 0; i < points.Size(); ++i)
  {
    Vec3 absPoint = Math::Abs(points[i]);
    maxVals = Math::Max(maxVals, absPoint);
  }
  // Formula taken from Dirk's Quick-Hull presentation (likely from "Matrix
  // Computations"). If we use a hard-coded epsilon then either large meshes or
  // small meshes will work, but not both.
  mEpsilon = 3 * (maxVals.x + maxVals.y + maxVals.z) * FLT_EPSILON;
}

bool QuickHull3D::BuildDataSet(const Array<Vec3>& points, size_t& resultPointCount)
{
  resultPointCount = 0;
  ComputeEpsilon(points);
  if (mEpsilon == 0)
    return false;

  BuildDataSetGridApproximation(points, resultPointCount);
  // Legacy for performance testing
  // BuildDataSetNSquared(points, resultPoints);
  // BuildDataSetGrid(points, resultPoints);
  return true;
}

void QuickHull3D::BuildDataSetNSquared(const Array<Vec3>& points, size_t& resultPointCount)
{
  // Kept around for performance analysis at later points in time.

  // QHullScopedTimer("NSquared:");
  //
  // for(size_t i = 0; i < points.Size(); ++i)
  //{
  //  QuickHullVertex* vertex = AllocateVertex();
  //  vertex->mPosition = points[i];
  //  mVertices.PushBack(vertex);
  //  ++resultPointCount;
  //}
  //
  // real epsilonSq = mEpsilon * mEpsilon;
  //// See if any points are really close to each other. These points can
  //// cause numerical issues in the building of the hull. To fix this
  //// just merge points together that are too close.
  // VertexList::range r1 = mVertices.All();
  // for(; !r1.Empty(); r1.PopFront())
  //{
  //  QuickHullVertex& v1 = r1.Front();
  //  VertexList::range r2 = r1;
  //  r2.PopFront();
  //  while(!r2.Empty())
  //  {
  //    QuickHullVertex& v2 = r2.Front();
  //    r2.PopFront();
  //
  //    real lengthSq = (v1.mPosition - v2.mPosition).LengthSq();
  //    if(lengthSq <= epsilonSq)
  //    {
  //      mVertices.Unlink(&v2);
  //      --resultPointCount;
  //    }
  //  }
  //}
}

void QuickHull3D::BuildDataSetGrid(const Array<Vec3>& points, size_t& resultPointCount)
{
  // Kept around for performance analysis at later points in time.

  // QHullScopedTimer("Grid Accurate:");
  //
  // real gridSize = mEpsilon * 2;
  // HashMap<IntVec3, Array<Vec3> > mGrid;
  // for(size_t i = 0; i < points.Size(); ++i)
  //{
  //  Vec3 point = points[i];
  //  Vec3 vecKey = Math::Floor(point / gridSize);
  //  IntVec3 key;
  //  for(size_t i = 0; i < 3; ++i)
  //    key[i] = (int)vecKey[i];
  //  HashSet<IntVec3> keysToTest;
  //
  //  // Compute the grid indices for all 9 cells (there might be overlap)
  //  for(int z = -1; z <= 1; ++z)
  //  {
  //    for(int y = -1; y <= 1; ++y)
  //    {
  //      for(int x = -1; x <= 1; ++x)
  //      {
  //        Vec3 offset = point + mEpsilon * Vec3((real)x, (real)y, (real)z);
  //        Vec3 offsetVecKey = Math::Floor(offset / gridSize);
  //        IntVec3 offsetKey;
  //        for(size_t i = 0; i < 3; ++i)
  //          offsetKey[i] = (int)offsetVecKey[i];
  //        keysToTest.Insert(offsetKey);
  //      }
  //    }
  //  }
  //
  //  // Find if any cell has a point in it that is within epsilon from our
  //  current object bool isInEpsilon = false; forRange(IntVec3& testKey,
  //  keysToTest.All())
  //  {
  //    Array<Vec3>& testCell = mGrid[testKey];
  //    isInEpsilon = IsWithinEpsilon(point, testCell, mEpsilon);
  //    if(isInEpsilon)
  //      break;
  //  }
  //  // If there was no point to weld to then create the new vertex
  //  if(!isInEpsilon)
  //  {
  //    Array<Vec3>& cell = mGrid[key];
  //    QuickHullVertex* vertex = AllocateVertex();
  //    vertex->mPosition = points[i];
  //    mVertices.PushBack(vertex);
  //    ++resultPointCount;
  //    cell.PushBack(point);
  //  }
  //}
}

void QuickHull3D::BuildDataSetGridApproximation(const Array<Vec3>& points, size_t& resultPointCount)
{
  QHullScopedTimer("Grid Approximation:");

  // Map each object to a grid cell and allow only one point per cell. Don't
  // check neighboring cells though as an optimization. QuickHull should
  // handle redundant points well as long as they aren't equal.
  real gridSize = mEpsilon * 2;
  HashMap<IntVec3, bool> mGrid;
  for (size_t i = 0; i < points.Size(); ++i)
  {
    Vec3 point = points[i];
    Vec3 vecKey = Math::Floor(point / gridSize);
    IntVec3 key;
    for (size_t i = 0; i < 3; ++i)
      key[i] = (int)vecKey[i];

    bool& isFilled = mGrid[key];
    if (!isFilled)
    {
      isFilled = true;

      QuickHullVertex* vertex = AllocateVertex();
      vertex->mPosition = points[i];
      mVertices.PushBack(vertex);
      ++resultPointCount;
    }
  }
}

bool QuickHull3D::BuildInitialHull()
{
  // Find the initial two points furthest away on either cardinal axis
  QuickHullVertex *v0, *v1;
  FindInitialSpan(v0, v1);

  // Degenerate initial span. Should only happen if vertex welding snapped all
  // inputs to the same point.
  // if(v0 == v1)
  //  return false;

  mVertices.Erase(v0);
  mVertices.Erase(v1);

  // Find the point furthest away from the line [v0, v1].
  QuickHullVertex* v2 = FindVertexFurthestFrom(v0, v1);
  // If we had colinear geometry we'd fail to find this point
  if (v2 == nullptr)
    return false;
  mVertices.Erase(v2);

  // Find the point furthest away from the triangle [v0, v1, v2].
  QuickHullVertex* v3 = FindVertexFurthestFrom(v0, v1, v2);
  // If we had coplanar geometry we'd fail to find this point
  if (v3 == nullptr)
    return false;
  mVertices.Erase(v3);

  // The signed volume of a tetrahedron can be computed as 1/6 the determinant
  // of this matrix. Since we don't care about the actual volume we can ignore
  // the 1/6 and the sign.
  Mat3 volumeMat = Mat3();
  volumeMat.SetBasis(0, v1->mPosition - v0->mPosition);
  volumeMat.SetBasis(1, v2->mPosition - v0->mPosition);
  volumeMat.SetBasis(2, v3->mPosition - v0->mPosition);
  real determinant = volumeMat.Determinant();

  // Degenerate tetrahedron
  if (Math::Abs(determinant) < mEpsilon)
    return false;

  QuickHullEdge* edge01 = AllocateEdge();
  QuickHullEdge* edge10 = AllocateEdge();
  QuickHullEdge* edge02 = AllocateEdge();
  QuickHullEdge* edge20 = AllocateEdge();
  QuickHullEdge* edge03 = AllocateEdge();
  QuickHullEdge* edge30 = AllocateEdge();
  QuickHullEdge* edge12 = AllocateEdge();
  QuickHullEdge* edge21 = AllocateEdge();
  QuickHullEdge* edge13 = AllocateEdge();
  QuickHullEdge* edge31 = AllocateEdge();
  QuickHullEdge* edge23 = AllocateEdge();
  QuickHullEdge* edge32 = AllocateEdge();

  CreateTwinEdge(v0, v1, edge01, edge10);
  CreateTwinEdge(v0, v2, edge02, edge20);
  CreateTwinEdge(v0, v3, edge03, edge30);
  CreateTwinEdge(v1, v2, edge12, edge21);
  CreateTwinEdge(v1, v3, edge13, edge31);
  CreateTwinEdge(v2, v3, edge23, edge32);

  // Depending on the direction we searched to find the last point we need to
  // construct the initial tetrahedron differently (for winding order)
  if (determinant > 0)
  {
    mFaces.PushBack(CreateFace(edge10, edge02, edge21));
    mFaces.PushBack(CreateFace(edge01, edge13, edge30));
    mFaces.PushBack(CreateFace(edge23, edge31, edge12));
    mFaces.PushBack(CreateFace(edge32, edge20, edge03));
  }
  else
  {
    mFaces.PushBack(CreateFace(edge01, edge12, edge20));
    mFaces.PushBack(CreateFace(edge10, edge03, edge31));
    mFaces.PushBack(CreateFace(edge23, edge30, edge02));
    mFaces.PushBack(CreateFace(edge32, edge21, edge13));
  }

  // Compute the center and normal of all of our initial faces
  for (FaceList::range faces = mFaces.All(); !faces.Empty(); faces.PopFront())
  {
    QuickHullFace* face = &faces.Front();
    face->RecomputeCenterAndNormal();
  }
  DrawInitialHull();

  if (cDebugQuickHull)
    ValidateHull();

  return true;
}

void QuickHull3D::FindInitialSpan(QuickHullVertex*& v0, QuickHullVertex*& v1)
{
  QuickHullVertex* first = &mVertices.Front();
  QuickHullVertex* minVertices[3] = {first, first, first};
  QuickHullVertex* maxVertices[3] = {first, first, first};
  // Find the furthest away points on each cardinal axis
  for (VertexList::range vertices = mVertices.All(); !vertices.Empty(); vertices.PopFront())
  {
    QuickHullVertex* vertex = &vertices.Front();
    for (size_t i = 0; i < 3; ++i)
    {
      if (vertex->mPosition[i] < minVertices[i]->mPosition[i])
        minVertices[i] = vertex;
      if (vertex->mPosition[i] > maxVertices[i]->mPosition[i])
        maxVertices[i] = vertex;
    }
  }

  // Calculate the distance away these points are
  real xSpanSq = Math::DistanceSq(minVertices[0]->mPosition, maxVertices[0]->mPosition);
  real ySpanSq = Math::DistanceSq(minVertices[1]->mPosition, maxVertices[1]->mPosition);
  real zSpanSq = Math::DistanceSq(minVertices[2]->mPosition, maxVertices[2]->mPosition);

  // Pick the span with the largest spread
  int maxSpreadIndex = 0;
  if (xSpanSq >= ySpanSq && xSpanSq >= zSpanSq)
    maxSpreadIndex = 0;
  else if (ySpanSq >= xSpanSq && ySpanSq >= zSpanSq)
    maxSpreadIndex = 1;
  else
    maxSpreadIndex = 2;

  // Grab the two points that were furthest apart
  v0 = minVertices[maxSpreadIndex];
  v1 = maxVertices[maxSpreadIndex];

  DrawInitialSpan(v0, v1);
}

QuickHull3D::QuickHullVertex* QuickHull3D::FindVertexFurthestFrom(QuickHullVertex* v0, QuickHullVertex* v1)
{
  // Find the furthest furthest away point from the line. Instead of calling a
  // helper function, we can cache a lot of common values from the line [v0, v1]
  // and re-use them in the calculation.

  // Let a = p0, b = p1, c = p
  // distance = ac^2 - Dot(ac, ab)^2 / ab^2
  // This means ab^2 can be cached
  Vec3 a = v0->mPosition;
  Vec3 b = v1->mPosition;
  Vec3 ab = b - a;
  real abSq = Math::LengthSq(ab);

  real maxDistance = 0;
  QuickHullVertex* furthestVertex = nullptr;
  for (VertexList::range vertices = mVertices.All(); !vertices.Empty(); vertices.PopFront())
  {
    QuickHullVertex* vertex = &vertices.Front();
    Vec3 c = vertex->mPosition;

    // Compute the squared distance this point is from the line
    Vec3 ac = c - a;
    real acSq = Math::LengthSq(ac);
    real dotAcAb = Math::Dot(ac, ab);
    real dotAcAbSq = dotAcAb * dotAcAb;
    real distance = acSq - dotAcAbSq / abSq;

    if (distance > maxDistance)
    {
      furthestVertex = vertex;
      maxDistance = distance;
    }
  }

  DrawInitialTriangle(v0, v1, furthestVertex);

  return furthestVertex;
}

QuickHull3D::QuickHullVertex* QuickHull3D::FindVertexFurthestFrom(QuickHullVertex* v0,
                                                                  QuickHullVertex* v1,
                                                                  QuickHullVertex* v2)
{
  // Find the furthest furthest away point from the triangle [v0, v1, v2].

  Vec3 p0 = v0->mPosition;
  Vec3 p1 = v1->mPosition;
  Vec3 p2 = v2->mPosition;

  // Cache the normal
  Vec3 normal = Math::Cross(p1 - p0, p2 - p0);
  normal.AttemptNormalize();

  real maxDistance = 0;
  QuickHullVertex* furthestVertex = nullptr;
  for (VertexList::range vertices = mVertices.All(); !vertices.Empty(); vertices.PopFront())
  {
    QuickHullVertex* vertex = &vertices.Front();
    real distance = Math::Abs(Math::Dot(normal, vertex->mPosition - p0));

    if (distance > maxDistance)
    {
      furthestVertex = vertex;
      maxDistance = distance;
    }
  }

  DrawInitialTetrahedron(v0, v1, v2, furthestVertex);

  return furthestVertex;
}

void QuickHull3D::ComputeInitialConflictLists()
{
  // For each vertex, find if it's inside the hull and if not what face it's the
  // closest to.
  while (!mVertices.Empty())
  {
    QuickHullVertex* vertex = &mVertices.Front();
    mVertices.PopFront();

    // Find the face that this vertex is closest to on the positive side (ignore
    // negative side) and then add this vertex to that face's conflict list. If
    // we didn't get a face back then this vertex is inside the hull and we can
    // ignore it (memory will be cleaned up by the pool).
    QuickHullFace* face = FindClosestFace(vertex);
    if (face != nullptr)
      face->mConflictList.PushBack(vertex);
  }

  DrawConflictPartition();
}

QuickHull3D::QuickHullFace* QuickHull3D::FindClosestFace(QuickHullVertex* vertex)
{
  QuickHullFace* closestFace = nullptr;
  real closestDistance = Math::PositiveMax();
  // Find the face that we're closest to but on the (strictly) positive side
  for (FaceList::range faces = mFaces.All(); !faces.Empty(); faces.PopFront())
  {
    QuickHullFace* face = &faces.Front();
    real signedDistance = Math::Dot(vertex->mPosition - face->mCenter, face->mNormal);
    if (signedDistance >= mEpsilon && signedDistance < closestDistance)
    {
      closestDistance = signedDistance;
      closestFace = face;
    }
  }
  vertex->mConflictDistance = closestDistance;
  return closestFace;
}

void QuickHull3D::FindNextConflictVertex(QuickHullVertex*& conflictVertex, QuickHullFace*& conflictFace)
{
  conflictVertex = nullptr;
  conflictFace = nullptr;
  real furthestDistance = -1;
  // Find the vertex that's furthest away from its conflict face.
  for (FaceList::range faces = mFaces.All(); !faces.Empty(); faces.PopFront())
  {
    QuickHullFace* face = &faces.Front();
    for (VertexList::range vertices = face->mConflictList.All(); !vertices.Empty(); vertices.PopFront())
    {
      QuickHullVertex* vertex = &vertices.Front();
      if (vertex->mConflictDistance > furthestDistance)
      {
        conflictFace = face;
        conflictVertex = vertex;
        furthestDistance = vertex->mConflictDistance;
      }
    }
  }
}

void QuickHull3D::AddVertexToHull(QuickHullVertex* conflictVertex, QuickHullFace* conflictFace)
{
  // Remove the vertex from the face since we're going to partition it
  conflictFace->mConflictList.Erase(conflictVertex);

  // Find the edge horizon for the given vertex. The horizon is defined as the
  // edge boundary between faces where this vertex is visible and not visible.
  // This returns the counter-clockwise edge list of the horizon and a list of
  // all faces that were visible and need to be destroyed.
  Array<QuickHullEdge*> horizon;
  Array<QuickHullFace*> internalFaces;
  IdentifyHorizon(conflictVertex, conflictFace, horizon, internalFaces);

  // Remove the internal faces from our current face list.
  for (size_t i = 0; i < internalFaces.Size(); ++i)
  {
    QuickHullFace* face = internalFaces[i];
    mFaces.Erase(face);
  }

  // To add this new vertex we now need to add the new required faces from the
  // horizon to this vertex, partition all vertices in these old faces to new
  // faces, and delete the old faces. For numerical stability, we also have to
  // find any faces that are not strictly convex and merge them into one
  // polygonal face.
  Array<QuickHullFace*> newFaces;
  CreateNewHorizonFaces(conflictVertex, horizon, newFaces);
  PartitionOldFaceConflictLists(internalFaces);
  RemoveOldHorizonFaces(internalFaces);
  MergeFaces(newFaces);

  // Debug check to ensure the current mesh is valid (connectivity info, etc...)
  if (cDebugQuickHull)
    ValidateHull();
}

void QuickHull3D::IdentifyHorizon(QuickHullVertex* conflictVertex,
                                  QuickHullFace* conflictFace,
                                  Array<QuickHullEdge*>& horizonEdges,
                                  Array<QuickHullFace*>& internalFaces)
{
  // To identify the horizon we perform a DFS from the given face across each of
  // its edges. If the adjacent face for a given edge hasn't been visited
  // already and is visible to the vertex then we recurse into it. If the face
  // isn't visible then we add the edge to the horizon and pop the recursion.

  HashSet<QuickHullFace*> visitedFaces;
  Array<QuickHullSearchData> faceStack;
  visitedFaces.Insert(conflictFace);
  internalFaces.PushBack(conflictFace);

  // To correctly find this horizon, we need to iterate over each edge in a face
  // in order. This also means that when we jump over to another face from a
  // twin that we have to start iteration from the twin. To do this a stack is
  // maintained of the current edge-range for each face we're visiting.
  faceStack.PushBack(QuickHullSearchData(conflictFace));
  while (!faceStack.Empty())
  {
    QuickHullSearchData& searchData = faceStack.Back();
    QuickHullFace* face = searchData.mFace;

    // Make sure to grab a reference to the range so we advance it and not a
    // copy
    InListWrappedRange<EdgeList>& edgeRange = searchData.mRange;
    // We've exhausted all edges on this face. Go to the previous face
    if (edgeRange.Empty())
    {
      faceStack.PopBack();
      continue;
    }

    // Grab the current edge and advance the range.
    // We don't want to visit this edge twice so pop even if we recurse.
    QuickHullEdge* edge = &edgeRange.Front();
    edgeRange.PopFront();

    QuickHullEdge* twin = edge->mTwin;
    QuickHullFace* adjacentFace = twin->mFace;
    // If we've already visited the adjacent face then skip this edge
    bool adjacentFaceVisited = visitedFaces.Contains(adjacentFace);
    if (adjacentFaceVisited)
      continue;

    // If this face isn't visible then this edge is on the horizon.
    // Add the edge and then pop the stack back to continue iteration.
    bool isFaceVisible =
        Math::Dot(conflictVertex->mPosition - adjacentFace->mCenter, adjacentFace->mNormal) >= mEpsilon;
    if (!isFaceVisible)
    {
      horizonEdges.PushBack(edge);
      continue;
    }

    // Add the new face and it's edge range onto the stack (start iterating over
    // all edges after the twin)
    faceStack.PushBack(QuickHullSearchData(adjacentFace, twin));
    internalFaces.PushBack(adjacentFace);
    visitedFaces.Insert(adjacentFace);
  }

  // Debug
  DrawHorizon(conflictVertex, horizonEdges);
}

void QuickHull3D::CreateNewHorizonFaces(QuickHullVertex* conflictVertex,
                                        Array<QuickHullEdge*>& horizon,
                                        Array<QuickHullFace*>& newFaces)
{
  // Add new faces for each edge
  size_t size = horizon.Size();
  for (size_t i = 0; i < size; ++i)
  {
    QuickHullEdge* edge01 = horizon[i];
    // Get the twin of this edge so we can get both vertices of the edge
    QuickHullEdge* twin10 = edge01->mTwin;

    // Get all 3 vertices of the new triangle face
    QuickHullVertex* v0 = edge01->mTail;
    QuickHullVertex* v1 = twin10->mTail;
    QuickHullVertex* v2 = conflictVertex;

    // Remove the horizon edge from it's old face. We'll keep this edge
    // around as a part of the new face (requires fewer re-linkings).
    QuickHullFace* edge01Face = edge01->mFace;
    edge01Face->mEdges.Unlink(edge01);

    // Create the two new edges. Use these two new edges along with
    // the old horizon edge to create the new horizon face.
    QuickHullEdge* edge12 = AllocateEdge();
    QuickHullEdge* edge20 = AllocateEdge();
    QuickHullFace* newFace = CreateFace(edge01, edge12, edge20);

    // Hook up the new edges to their tail vertices
    edge12->mTail = v1;
    edge20->mTail = v2;

    // Compute the normal and center of this new face
    newFace->RecomputeCenterAndNormal();
    mFaces.PushBack(newFace);
    newFaces.PushBack(newFace);
  }

  // Do a second pass to fix the twin edges of the new horizon
  // edges (from horizon vertices to new vertex)
  for (size_t i = 0; i < size; ++i)
  {
    QuickHullEdge* edge = horizon[i];
    QuickHullEdge* next = horizon[(i + 1) % size];

    // To get the twins we need to iterate the current edge forward to get the
    // edge from v1 to v2 (conflict vertex). To get the twin we need to go to
    // the next adjacent face but go backwards to get the edge from v2 to v1.
    QuickHullEdge* edgeNext = edge->mFace->mEdges.NextWrap(edge);
    QuickHullEdge* nextPrev = next->mFace->mEdges.PrevWrap(next);

    edgeNext->mTwin = nextPrev;
    nextPrev->mTwin = edgeNext;
  }

  // Debug
  DrawExpandedHull();
}

void QuickHull3D::PartitionOldFaceConflictLists(Array<QuickHullFace*>& faces)
{
  // Partition old face conflict lists
  for (size_t i = 0; i < faces.Size(); ++i)
  {
    QuickHullFace* face = faces[i];
    while (!face->mConflictList.Empty())
    {
      QuickHullVertex* vertex = &face->mConflictList.Front();
      face->mConflictList.PopFront();

      // Find the next closest face. If we got one back then add this vertex to
      // that face's conflict list. Otherwise this vertex is inside the hull so
      // ignore it (memory will be cleaned up via the memory pool at the end).
      QuickHullFace* newFace = FindClosestFace(vertex);
      if (newFace != nullptr)
        AbsorbConflictVertex(newFace, vertex);
    }
  }
}

void QuickHull3D::AbsorbConflictList(QuickHullFace* face, VertexList& conflictList)
{
  // Absorb each vertex in the list into the given face. We don't splice the
  // vertex in because we can sort the vertices by conflict distance to speed up
  // finding the best conflict vertex.
  while (!conflictList.Empty())
  {
    QuickHullVertex* vertex = &conflictList.Front();
    conflictList.PopFront();

    AbsorbConflictVertex(face, vertex);
  }
}

void QuickHull3D::AbsorbConflictVertex(QuickHullFace* face, QuickHullVertex* vertex)
{
  // For now just insert at the end. This should be sorted into place later.
  face->mConflictList.PushBack(vertex);
}

void QuickHull3D::RemoveOldHorizonFaces(Array<QuickHullFace*>& faces)
{
  // Collect all edges from the faces we're deleting. They could be deleted
  // mid-iteration, but for debugging purposes we're collecting them all to
  // delete afterwards. Additionally it is slightly faster to collect them in a
  // list (less in-list pointers have to be updated if the entire list is
  // spliced)
  EdgeList edgesToDelete;

  // Remove all old edges from their faces and then delete the face
  for (size_t i = 0; i < faces.Size(); ++i)
  {
    QuickHullFace* face = faces[i];
    if (!face->mEdges.Empty())
      edgesToDelete.Splice(edgesToDelete.End(), face->mEdges.All());
    DeallocateFace(face);
  }
  faces.Clear();

  // Clean up memory for all old edges
  while (!edgesToDelete.Empty())
  {
    QuickHullEdge* edge = &edgesToDelete.Front();
    edgesToDelete.PopFront();
    DeallocateEdge(edge);
  }
}

void QuickHull3D::MergeFaces(Array<QuickHullFace*>& newFaces)
{
  // We need to iterate over all of the new faces and test their neighbors
  // to see if a merge should happen. There's a chance that two of the new faces
  // could merge together (or be collapsed in a topological invariant fix) so we
  // can't just iterate through the list. Instead it's easiest to convert the
  // list to a separate inlist so we can generically unlink from the list to
  // test if needed.
  FaceList facesToTestForMerge;
  for (size_t i = 0; i < newFaces.Size(); ++i)
  {
    FaceList::Unlink(newFaces[i]);
    facesToTestForMerge.PushBack(newFaces[i]);
  }

  while (!facesToTestForMerge.Empty())
  {
    // Mark this face as being processed by adding it back to the final face
    // list
    QuickHullFace* face = &facesToTestForMerge.Front();
    facesToTestForMerge.PopFront();
    mFaces.PushBack(face);

    // Find a face that is not convex to the current face (represented by an
    // edge)
    QuickHullEdge* edge = FindQuickHullMergeFace(face);
    while (edge != nullptr)
    {
      QuickHullEdge* twin = edge->mTwin;
      QuickHullFace* adjacentFace = twin->mFace;
      ErrorIf(face == adjacentFace, "");

      // Absorb the adjacent face across the edge into the current face
      AbsorbFace(face, adjacentFace, edge, twin);

      // Find a topological invariant if it exists and fix it. Iteratively do
      // this until all invariants are gone from this face (fixing one could
      // create another).
      bool isInvariants = true;
      while (isInvariants)
        isInvariants = FixTopologicalInvariants(face);

      // Fix the face normal and center now after merging and fixing topological
      // issues. We must fix the normal here instead of after merging all faces.
      // This is because a non-convex face isn't unnecessarily parallel. It
      // could be a very small face completely within the thick plane that
      // points in the opposite direction.
      face->RecomputeCenterAndNormal();

      // Find the next face to merge across
      edge = FindQuickHullMergeFace(face);
    }
    // Debug
    if (cDebugQuickHull)
      ValidateFace(face);
  }
}

QuickHull3D::QuickHullEdge* QuickHull3D::FindQuickHullMergeFace(QuickHullFace* face)
{
  // Walk over all edges in this face. Test the adjacent face to see if it's
  // strictly convex to this face. If it isn't then return the edge bordering
  // these faces as we need to merge across this edge.
  for (EdgeList::range edges = face->mEdges.All(); !edges.Empty(); edges.PopFront())
  {
    QuickHullEdge* edge = &edges.Front();
    QuickHullFace* adjacentFace = edge->mTwin->mFace;

    if (face->IsConvexTo(adjacentFace, mEpsilon))
      continue;
    return edge;
  }
  // Otherwise all faces are convex to this face
  return nullptr;
}

void QuickHull3D::AbsorbFace(QuickHullFace* face,
                             QuickHullFace* adjacentFace,
                             QuickHullEdge* sharedEdge,
                             QuickHullEdge* sharedTwin)
{
  // Draw the two faces that aren't convex to each other
  DrawNonConvexFaces(face, adjacentFace);

  // In order to absorb a face, we need to merge all edges from the adjacent
  // face into this face. The edge order needs to be preserved between this face
  // and the adjacent face though. To do this we need to walk all edges after
  // the shared edge between them. The InList's dummy node needs to be skipped
  // though. The easiest way to do this is to split the edge list we're merging
  // into two pieces: the range from the shared edge to the end of the list and
  // the range from the start of the list up to the shared edge.
  EdgeList::range beforeRange(adjacentFace->mEdges.Begin(), sharedTwin);
  EdgeList::range afterRange(sharedTwin, adjacentFace->mEdges.End());
  // Remove the shared twin from the range since we're deleting it
  afterRange.PopFront();

  // Insert both edge ranges in order
  QuickHullEdge* it = InsertEdgeRangeAfter(face, sharedEdge, afterRange);
  InsertEdgeRangeAfter(face, it, beforeRange);

  // We need to handle the conflict list of the other face.
  AbsorbConflictList(face, adjacentFace->mConflictList);

  // Cleanup the shared edge and it's twin
  EdgeList::Unlink(sharedEdge);
  EdgeList::Unlink(sharedTwin);
  DeallocateEdge(sharedEdge);
  DeallocateEdge(sharedTwin);

  // Remove the adjacent face since we merged it in
  FaceList::Unlink(adjacentFace);
  DeallocateFace(adjacentFace);

  // Draw the resultant merged face
  DrawHullWithFace("Merged Faces", face);
}

QuickHull3D::QuickHullEdge* QuickHull3D::InsertEdgeRangeAfter(QuickHullFace* face,
                                                              QuickHullEdge* edgeToInsertAfter,
                                                              EdgeList::range& edgesToInsert)
{
  // Iteratively add each edge in the given range after the provided edge
  QuickHullEdge* it = edgeToInsertAfter;
  while (!edgesToInsert.Empty())
  {
    // Remove the edge from it's current face
    QuickHullEdge* toInsert = &edgesToInsert.Front();
    edgesToInsert.PopFront();
    EdgeList::Unlink(toInsert);

    ErrorIf(toInsert->mFace == face, "Bad face");
    // Add the edge to its new face
    toInsert->mFace = face;
    face->mEdges.InsertAfter(it, toInsert);

    it = toInsert;
  }
  // Return where we left of inserting so that more edges could be inserted
  // after this later
  return it;
}

bool QuickHull3D::FixTopologicalInvariants(QuickHullFace* face)
{
  // Find any topological issue we need to fix (see the function for a comment)
  QuickHullEdge* e0 = nullptr;
  QuickHullEdge* e1 = nullptr;
  if (FindTopoligicalInvariant(face, e0, e1))
  {
    // The way we need to fix this depends on how many edges are in the adjacent
    // face (triangle or otherwise)
    QuickHullFace* adjacentFace = e0->mTwin->mFace;
    size_t count = adjacentFace->CountEdges();
    if (count == 3)
      FixTriangleTopoligicalInvariant(face, e0, e1);
    else
      FixEdgeTopoligicalInvariant(face, e0, e1);
    // Return that we found and fixed a topological issue so we aren't done
    // (fixing one could cause another)
    return true;
  }
  // Return that there were no topological issues so we're done with this face
  return false;
}

bool QuickHull3D::FindTopoligicalInvariant(QuickHullFace* face, QuickHullEdge*& e0, QuickHullEdge*& e1)
{
  // The only topological invariant we care about is two faces sharing more than
  // one edge. We can detect this by iterating over all edges and seeing if two
  // adjacent edge's have the same twin face.
  QuickHullEdge* prev = &face->mEdges.Back();
  for (EdgeList::range edges = face->mEdges.All(); !edges.Empty(); edges.PopFront())
  {
    QuickHullEdge* edge = &edges.Front();
    if (prev->mTwin->mFace == edge->mTwin->mFace)
    {
      e0 = prev;
      e1 = edge;
      return true;
    }
    prev = edge;
  }
  return false;
}

void QuickHull3D::FixTriangleTopoligicalInvariant(QuickHullFace* face, QuickHullEdge* e0, QuickHullEdge* e1)
{
  // When the adjacent edge is a triangle, we need to remove the two shared
  // edges of the adjacent face, remove the internal vertex, and absorb the
  // remaining edge into the given face.
  QuickHullEdge* t0 = e0->mTwin;
  QuickHullEdge* t1 = e1->mTwin;
  QuickHullFace* adjacentFace = t0->mFace;
  // Grab the edge that will stick around after absorbing the triangle
  QuickHullEdge* remainingEdge = adjacentFace->mEdges.NextWrap(t0);
  // We don't actually need to do anything with this vertex other than stop
  // referencing it. Memory will be taken care of from the memory pool.
  // QuickHullVertex* vertexToRemove = e1->mTail;

  // Debug draw what the issue was we found
  DrawTopologicalFix("Identified triangle error", face, adjacentFace, e0, e1);

  // Remove the remaining edge from the adjacent face (one we're absorbing) and
  // insert it into this face in the correct order (before e0).
  adjacentFace->mEdges.Erase(remainingEdge);
  face->mEdges.InsertBefore(e0, remainingEdge);
  remainingEdge->mFace = face;

  // Partition all vertices in the conflict list of the triangle we're absorbing
  // into this face
  AbsorbConflictList(face, adjacentFace->mConflictList);

  // Remove the edges that we're absorbing (twins as well)
  face->mEdges.Erase(e0);
  face->mEdges.Erase(e1);
  adjacentFace->mEdges.Erase(t0);
  adjacentFace->mEdges.Erase(t1);

  // Sanity
  ErrorIf(!adjacentFace->mEdges.Empty(), "Adjacent face isn't empty");

  // Clean up memory
  DeallocateEdge(e0);
  DeallocateEdge(e1);
  DeallocateEdge(t0);
  DeallocateEdge(t1);

  // Delete the face we just absorbed
  FaceList::Unlink(adjacentFace);
  DeallocateFace(adjacentFace);

  // Debug
  DrawHullWithFace("Collapse triangle", face);
}

void QuickHull3D::FixEdgeTopoligicalInvariant(QuickHullFace* face, QuickHullEdge* e0, QuickHullEdge* e1)
{
  // To fix an edge topological issue, we need to merge the two
  // edges into one, removing the vertex in the middle.
  QuickHullEdge* t0 = e0->mTwin;
  QuickHullEdge* t1 = e1->mTwin;

  // Debug
  QuickHullFace* adjacentFace = t1->mFace;
  DrawTopologicalFix("Identified edge error", face, adjacentFace, e0, e1);

  // Remove the middle vertex.
  // Don't have to deallocate the vertex since we'll clean it up in the memory
  // pool QuickHullVertex* vertexToRemove = t1->mTail;

  // Make sure to unlink t0 instead of t1 because t1 has the
  // correct tail vertex already set (we're removing the middle vertex)
  EdgeList::Unlink(t0);
  EdgeList::Unlink(e1);
  DeallocateEdge(t0);
  DeallocateEdge(e1);

  // Since we removed e1 and t0 we have to fix the twin edge pointers now.
  t1->mTwin = e0;
  e0->mTwin = t1;

  // Debug
  DrawHullWithFace("Collapse edge", face);
}

void QuickHull3D::CreateTwinEdge(QuickHullVertex* v0, QuickHullVertex* v1, QuickHullEdge* edge01, QuickHullEdge* edge10)
{
  edge01->mTail = v0;
  edge10->mTail = v1;
  edge01->mTwin = edge10;
  edge10->mTwin = edge01;
}

QuickHull3D::QuickHullFace* QuickHull3D::CreateFace(QuickHullEdge* e0, QuickHullEdge* e1, QuickHullEdge* e2)
{
  QuickHullFace* face = AllocateFace();
  face->mEdges.PushBack(e0);
  face->mEdges.PushBack(e1);
  face->mEdges.PushBack(e2);
  e0->mFace = face;
  e1->mFace = face;
  e2->mFace = face;
  return face;
}

QuickHull3D::QuickHullVertex* QuickHull3D::AllocateVertex()
{
  return mVertexPool->AllocateType<QuickHullVertex>();
}

QuickHull3D::QuickHullEdge* QuickHull3D::AllocateEdge()
{
  return mEdgePool->AllocateType<QuickHullEdge>();
}

QuickHull3D::QuickHullFace* QuickHull3D::AllocateFace()
{
  return mFacePool->AllocateType<QuickHullFace>();
}

void QuickHull3D::DeallocateVertex(QuickHullVertex* vertex)
{
  mVertexPool->DeallocateType<QuickHullVertex>(vertex);
}

void QuickHull3D::DeallocateEdge(QuickHullEdge* edge)
{
  mEdgePool->DeallocateType<QuickHullEdge>(edge);
}

void QuickHull3D::DeallocateFace(QuickHullFace* face)
{
  ErrorIf(!face->mEdges.Empty(), "Deallocating face with edges");
  ErrorIf(!face->mConflictList.Empty(), "Deallocating face with conflict vertices");

  mFacePool->DeallocateType<QuickHullFace>(face);
}

void QuickHull3D::ValidateFace(QuickHullFace* face)
{
  size_t edgeCount = 0;
  for (EdgeList::range edges = face->mEdges.All(); !edges.Empty(); edges.PopFront())
  {
    ++edgeCount;
    QuickHullEdge* edge = &edges.Front();
    QuickHullEdge* twin = edge->mTwin;

    ErrorIf(twin->mTwin != edge, "Invalid twin");
    ErrorIf(edge->mFace != face, "Invalid face");
    ErrorIf(twin->mFace == face || twin->mFace == nullptr, "Invalid twin face");
    ErrorIf(edge->mTail == nullptr, "Invalid tail");

    QuickHullFace* adjacentFace = twin->mFace;
    real signedDistance0 = Math::Dot(face->mNormal, adjacentFace->mCenter - face->mCenter);
    real signedDistance1 = Math::Dot(adjacentFace->mNormal, face->mCenter - adjacentFace->mCenter);
    ErrorIf(signedDistance0 > -mEpsilon, "Face 1 isn't convex to Face 0");
    ErrorIf(signedDistance1 > -mEpsilon, "Face 0 isn't convex to Face 1");

    size_t edgeCount = face->CountEdges();
    ErrorIf(edgeCount < 3, "Too few edges");
  }
}

void QuickHull3D::ValidateHull()
{
  for (FaceList::range faces = mFaces.All(); !faces.Empty(); faces.PopFront())
  {
    ValidateFace(&faces.Front());
  }
}

void QuickHull3D::ValidateFinalHull(const Array<Vec3>& points)
{
  ValidateHull();
  // Artificially scale up epsilon to deal with face merges which
  // can put a point slightly outside a face.
  float epsilon = mEpsilon * 10;
  for (size_t i = 0; i < points.Size(); ++i)
  {
    bool isInside = IsInsideHull(points[i], epsilon);
    ErrorIf(!isInside, "Point isn't convex");
  }
}

bool QuickHull3D::IsInsideHull(Vec3Param point, float epsilon)
{
  forRange (QuickHullFace& face, mFaces.All())
  {
    real signedDistance = Math::Dot(face.mNormal, point - face.mCenter);
    if (signedDistance > epsilon)
      return false;
  }
  return true;
}

DebugDrawStep& QuickHull3D::CreateStep(StringParam text)
{
  DebugDrawStep& step = mDebugDrawStack->mSteps.PushBack();
  Debug::Text debugText = mDebugDrawStack->mBaseText;
  debugText.mText = text;
  step.Add(debugText);
  return step;
}

DebugDrawStep& QuickHull3D::DrawHull(DebugDrawStep& step, bool filled)
{
  for (FaceList::range faces = mFaces.All(); !faces.Empty(); faces.PopFront())
  {
    QuickHullFace& face = faces.Front();
    DrawFace(step, &face);
  }
  return step;
}

void QuickHull3D::DrawVertices(DebugDrawStep& step, VertexList& vertexList, Vec4Param color)
{
  for (VertexList::range vertices = vertexList.All(); !vertices.Empty(); vertices.PopFront())
  {
    QuickHullVertex* vertex = &vertices.Front();
    DrawPoint(step, vertex->mPosition, color);
  }
}

void QuickHull3D::DrawConflictVertices(DebugDrawStep& step, Vec4Param color)
{
  for (FaceList::range faces = mFaces.All(); !faces.Empty(); faces.PopFront())
  {
    QuickHullFace* face = &faces.Front();
    DrawVertices(step, face->mConflictList, color);
  }
}

void QuickHull3D::DrawRemainingVertices(DebugDrawStep& step, Vec4Param color)
{
  DrawVertices(step, mVertices, color);
}

void QuickHull3D::DrawPoint(DebugDrawStep& step, Vec3Param p0, Vec4Param color)
{
  Debug::Sphere point = mDebugDrawStack->mBaseSphere;
  point.mRadius = 0.1f;
  point.mPosition = p0;
  point.mColor = color;
  step.Add(point);
}

void QuickHull3D::DrawLine(DebugDrawStep& step, Vec3Param p0, Vec3Param p1, Vec4Param color)
{
  Debug::Line line = mDebugDrawStack->mBaseLine;
  line.mStart = p0;
  line.mEnd = p1;
  line.mColor = color;
  step.Add(line);
}

void QuickHull3D::DrawRay(DebugDrawStep& step, Vec3Param p0, Vec3Param p1, real headSize, Vec4Param color)
{
  Debug::Line line = mDebugDrawStack->mBaseLine;
  line.mStart = p0;
  line.mEnd = p1;
  line.mColor = color;
  line.mHeadSize = headSize;
  step.Add(line);
}

void QuickHull3D::DrawEdge(DebugDrawStep& step, QuickHullEdge* edge, Vec4Param color)
{
  Vec3 p0 = edge->mTail->mPosition;
  Vec3 p1 = edge->mTwin->mTail->mPosition;
  DrawLine(step, p0, p1, color);
}

void QuickHull3D::DrawFace(DebugDrawStep& step, QuickHullFace* face, Vec4Param color, bool drawEdges, bool drawFaceVec)
{
  face->RecomputeCenterAndNormal();
  for (EdgeList::range edges = face->mEdges.All(); !edges.Empty(); edges.PopFront())
  {
    QuickHullEdge& edge = edges.Front();

    Vec3 start = edge.mTail->mPosition;
    Vec3 end = edge.mTwin->mTail->mPosition;
    DrawLine(step, start, end, color);

    if (drawEdges)
    {
      start = 0.9f * (start - face->mCenter) + face->mCenter;
      end = 0.9f * (end - face->mCenter) + face->mCenter;
      DrawRay(step, start, end, 0.05f, color * 0.5f);
    }

    if (drawFaceVec)
    {
      Vec3 mid = (start + end) / 2.0f;
      Vec3 dir = Math::AttemptNormalized(edge.mFace->mCenter - mid);
      end = mid + dir * 0.25f;
      DrawRay(step, mid, end, 0.025f, color * 0.25f);
    }
  }

  DrawRay(step, face->mCenter, face->mCenter + face->mNormal * 0.5f, 0.1f, color);
}

void QuickHull3D::DrawInitialSpan(QuickHullVertex* v0, QuickHullVertex* v1)
{
  if (mDebugDrawStack == nullptr)
    return;

  DebugDrawStep& step = CreateStep("Find two furthest away points");
  DrawPoint(step, v0->mPosition);
  DrawPoint(step, v1->mPosition);
  DrawLine(step, v0->mPosition, v1->mPosition);
}

void QuickHull3D::DrawInitialTriangle(QuickHullVertex* v0, QuickHullVertex* v1, QuickHullVertex* v2)
{
  if (mDebugDrawStack == nullptr)
    return;

  DebugDrawStep& step = CreateStep("Find point furthest from line");
  DrawPoint(step, v0->mPosition);
  DrawPoint(step, v1->mPosition);
  DrawPoint(step, v2->mPosition);
  DrawLine(step, v0->mPosition, v1->mPosition);

  Vec3 axis = Math::AttemptNormalized(v0->mPosition - v1->mPosition);
  Vec3 projectionPoint = v0->mPosition + Math::ProjectOnVector(v2->mPosition - v0->mPosition, axis);
  DrawPoint(step, projectionPoint);
  DrawLine(step, projectionPoint, v2->mPosition);
}

void QuickHull3D::DrawInitialTetrahedron(QuickHullVertex* v0,
                                         QuickHullVertex* v1,
                                         QuickHullVertex* v2,
                                         QuickHullVertex* v3)
{
  if (mDebugDrawStack == nullptr)
    return;

  DebugDrawStep& step = CreateStep("Find point furthest from triangle");
  DrawPoint(step, v0->mPosition);
  DrawPoint(step, v1->mPosition);
  DrawPoint(step, v2->mPosition);
  DrawPoint(step, v3->mPosition);
  DrawLine(step, v0->mPosition, v1->mPosition);
  DrawLine(step, v0->mPosition, v2->mPosition);
  DrawLine(step, v1->mPosition, v2->mPosition);

  Vec3 normal = Math::Cross(v1->mPosition - v0->mPosition, v2->mPosition - v0->mPosition);
  normal.AttemptNormalize();
  Vec3 projectionPoint = v0->mPosition + Math::ProjectOnPlane(v3->mPosition - v0->mPosition, normal);
  DrawPoint(step, projectionPoint);
  DrawLine(step, projectionPoint, v3->mPosition);
}

void QuickHull3D::DrawInitialHull()
{
  if (mDebugDrawStack == nullptr)
    return;

  DebugDrawStep& step = CreateStep("Compute Hull");
  DrawHull(step);
  DrawRemainingVertices(step);
  DrawConflictVertices(step);
}

void QuickHull3D::DrawConflictPartition()
{
  if (mDebugDrawStack == nullptr)
    return;

  DebugDrawStep& step = CreateStep("Compute Conflict Lists");
  DrawHull(step);
  DrawConflictVertices(step);
}

void QuickHull3D::DrawFoundConflictVertex(QuickHullVertex* conflictVertex, QuickHullFace* conflictFace)
{
  if (mDebugDrawStack == nullptr)
    return;

  Vec4 conflictColor = ToFloatColor(Color::Red);
  DebugDrawStep& step = CreateStep("Find Next Conflict Vertex");
  DrawHull(step);
  DrawPoint(step, conflictVertex->mPosition, conflictColor);
  DrawFace(step, conflictFace, conflictColor);
}

void QuickHull3D::DrawHorizon(QuickHullVertex* eye, Array<QuickHullEdge*>& edges)
{
  if (mDebugDrawStack == nullptr)
    return;

  Vec4 conflictColor = ToFloatColor(Color::Red);
  DebugDrawStep& step = CreateStep("Identify Horizon");
  DrawHull(step);
  for (size_t i = 0; i < edges.Size(); ++i)
  {
    DrawEdge(step, edges[i], conflictColor);
  }
  DrawPoint(step, eye->mPosition, conflictColor);
}

void QuickHull3D::DrawExpandedHull()
{
  if (mDebugDrawStack == nullptr)
    return;

  DebugDrawStep& step = CreateStep("Expand hull");
  DrawHull(step);
}

void QuickHull3D::DrawNonConvexFaces(QuickHullFace* face0, QuickHullFace* face1)
{
  if (mDebugDrawStack == nullptr)
    return;

  Vec4 conflictColor = ToFloatColor(Color::Red);
  DebugDrawStep& step = CreateStep("Found non-convex faces");
  DrawHull(step);
  DrawFace(step, face0, conflictColor, true, true);
  DrawFace(step, face1, conflictColor, true, true);
}

void QuickHull3D::DrawHullWithFace(StringParam text, QuickHullFace* face, Vec4Param color)
{
  if (mDebugDrawStack == nullptr)
    return;

  DebugDrawStep& step = CreateStep(text);
  DrawHull(step);
  DrawFace(step, face, color, true, true);
}

void QuickHull3D::DrawFaceMerge(QuickHullFace* face0, QuickHullFace* face1, Vec4Param color)
{
  if (mDebugDrawStack == nullptr)
    return;

  DebugDrawStep& step = CreateStep("Merge Faces");
  DrawHull(step);
  DrawFace(step, face0, color, true, true);
  DrawFace(step, face1, color, true, true);
}

void QuickHull3D::DrawTopologicalFix(
    StringParam text, QuickHullFace* face0, QuickHullFace* face1, QuickHullEdge* edge0, QuickHullEdge* edge1)
{
  if (mDebugDrawStack == nullptr)
    return;

  Vec4 conflictColor = ToFloatColor(Color::Red);
  DebugDrawStep& step = CreateStep(text);
  // DrawHull(step);
  DrawEdge(step, edge0, conflictColor);
  DrawEdge(step, edge1, conflictColor);
}

void QuickHull3D::DrawHullWithDescription(StringParam text)
{
  if (mDebugDrawStack == nullptr)
    return;

  DebugDrawStep& step = CreateStep(text);
  DrawHull(step);
}

void QuickHull3D::DrawFinalHull(const Array<Vec3>& points)
{
  if (mDebugDrawStack == nullptr)
    return;

  DebugDrawStep& step = CreateStep("Final Hull");
  DrawHull(step);
}

} // namespace Zero
