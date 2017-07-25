///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Nathan Carlson
/// Copyright 2015-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

static const bool sAllowBackfaces = false;

namespace Zero
{

//given a x-y basis, test what the angle of an axis is when projected onto this 2d basis
real GetAngle(Vec3Param yBasis, Vec3Param xBasis, Vec3Param testAxis)
{
  real xProj = Math::Dot(testAxis, xBasis);
  real yProj = Math::Dot(testAxis, yBasis);
  return Math::ArcTan2(yProj, xProj);
}

void ComputeEdgeInfoForTriangleA(Triangle& triA, uint indexA,
                                 Triangle& triB, TriangleInfoMap* infoMap)
{
  //hardcoded values at the moment, maybe expose or something later
  real equalVertexThreshold = real(.001);
  real planarEpsilon = real(.01);

  uint maxUint = (uint)-1;

  //first, make sure that both triangles are not degenerate triangles
  Vec3 normalA = triA.GetRawNormal();
  if(normalA.LengthSq() < equalVertexThreshold)
    return;
  Vec3 normalB = triB.GetRawNormal();
  if(normalB.LengthSq() < equalVertexThreshold)
    return;

  normalA.Normalize();
  normalB.Normalize();

  //keep a list of how many and what vertices are shared
  uint numSharedVertices = 0;
  uint sharedVerticesA[3] = {maxUint, maxUint, maxUint};
  uint sharedVerticesB[3] = {maxUint, maxUint, maxUint};

  //now we know that both triangles are valid, but we don't know if they
  //share a common edge. To figure this out, just loop through all of the
  //vertices and find if any of them are close enough
  for(uint iA = 0; iA < 3; ++iA)
  {
    for(uint iB = 0; iB < 3; ++iB)
    {
      //if the points are not close enough, we don't care
      Vec3 pointDifference = triA[iA] - triB[iB];
      if(pointDifference.LengthSq() > equalVertexThreshold)
        continue;

      if(numSharedVertices == 1 &&
        (sharedVerticesA[numSharedVertices - 1] == iA ||
        sharedVerticesB[numSharedVertices - 1] == iB))
        continue;

      //they are close enough, so mark what indices we are looking at
      //and how many shared vertices we now have
      sharedVerticesA[numSharedVertices] = iA;
      sharedVerticesB[numSharedVertices] = iB;
      ++numSharedVertices;
    }
  }

  //now we know how many shared vertices these triangles share,
  //but we only care if we are sharing an edge (aka 2 points)
  if(numSharedVertices != 2)
    return;

  //if we got the edge v0v2, we want to keep our counterclockwise winding,
  //so swap the vertices on both triangles
  if(sharedVerticesA[0] == 0 && sharedVerticesA[1] == 2)
  {
    Math::Swap(sharedVerticesA[0], sharedVerticesA[1]);
    Math::Swap(sharedVerticesB[0], sharedVerticesB[1]);
  }

  //first we need to get the triangle info associated with
  //triangle A and create it if it doesn't exist (that's what operator [] does)
  MeshTriangleInfo* info = &(*infoMap)[indexA];

  //we need to find the other vertex
  //(the sum of the vertices will also tell us which case we are in later)
  uint verticesSumA = sharedVerticesA[0] + sharedVerticesA[1];
  uint otherIndexA = 3 - verticesSumA;
  uint verticesSumB = sharedVerticesB[0] + sharedVerticesB[1];
  uint otherIndexB = 3 - verticesSumB;
  //also get the edge (points counter clockwise along A)
  Vec3 edge = triA[sharedVerticesA[1]] - triA[sharedVerticesA[0]];

  //now that we have this info we can get to the heart of this algorithm.
  //we want to compute the angle between the normals of triA and triB about the edge axis,
  //this tells us what the valid Voronoi region is for triangle A.
  //To do this, we have to compute some basis vectors and make sure
  //that they are facing in correct directions

  //first, we need to construct vectors on the surface of A and B that are
  //perpendicular to the normal and edge and facing outwards
  //(that is from the edge to the other point, the one not shared in the edge)
  Vec3 edgeCrossA = Math::Cross(normalA,edge);
  edgeCrossA.AttemptNormalize();
  Vec3 fromEdgeToOtherA = triA[otherIndexA] - triA[sharedVerticesA[0]];
  if(Math::Dot(fromEdgeToOtherA, edgeCrossA) < 0)
    edgeCrossA *= -1;

  Vec3 edgeCrossB = Math::Cross(normalB,edge);
  edgeCrossB.AttemptNormalize();
  Vec3 fromEdgeToOtherB = triB[otherIndexB] - triB[sharedVerticesB[0]];
  if(Math::Dot(fromEdgeToOtherB, edgeCrossB) < 0)
    edgeCrossB *= -1;

  //now recompute the edge, but facing in the correct direction for our basis
  //calculations. Also use the fact that ||cross(v1,v2)|| = sin(theta), then
  //use the fact that sin(theta) approx= theta for small angles.
  //This is ok since we are checking to see if the triangles are parallel
  Vec3 calculatedEdge = Math::Cross(edgeCrossA, edgeCrossB);
  real length = calculatedEdge.AttemptNormalize();

  real voronoiRegionAngle = real(0.0);
  bool isConvex = false;
  //we only care when they aren't planar (since we already set the angle for planar to 0)
  if(length >= planarEpsilon)
  {
    //now we need to build a basis where we have a normal on A as the yAxis
    //and the edge cross A as the x Axis, we can measure the angle of edge cross B
    //on this basis to find the angle between the two triangles, which
    //is also the angle between the normals which is the angle of the Voronoi region.

    //we already have our edges, but we need the correct facing normal, so compute one
    Vec3 computedNormalA = Math::Cross(calculatedEdge, edgeCrossA);
    computedNormalA.Normalize();

    real obtuseAngle = GetAngle(computedNormalA, edgeCrossA, edgeCrossB);
    //we computed the angle from A to B, but this was the angle that rotates
    //A to B such that edgeCrossA points in the same direction as edgeCrossB.
    //We want the opposite of that, so take the leftover angle.(calculated what
    //rotates the triangles on top of each other, not flat next to each other)
    voronoiRegionAngle = Math::cPi - obtuseAngle;
    //now that we've computed the angle, we can also determine if the triangle
    //edges are convex or concave, concave is when the point not shared in the
    //edge is in the positive direction of the other triangle's normal
    //See http://www.bulletphysics.org/Bullet/phpBB3/download/file.php?id=627 for a picture.

    //we compute convex by seeing if the normal of A and the edge direction
    //go opposite ways, if they do it's convex (draw it and you'll see)
    real dotA = Math::Dot(normalA, edgeCrossB);
    isConvex = dotA < real(0.0);

    //if we are convex, we need to flip the angle because our edge is
    //counterclockwise which will rotate us the wrong direction
    if(isConvex)
      voronoiRegionAngle *= -1;
  }

  //now that we've compute the angle we need to set the info structure.
  //That means determining which edge we are and marking the correct one's
  //angle and whether or not that edge was shared convexly or concavely.
  uint flags[3] = { TriangleInfoFlags::V0V1Convex, TriangleInfoFlags::V2V0Convex, TriangleInfoFlags::V1V2Convex };
  real* mEdgeAngles[3] = { &info->mEdgeV0V1Angle, &info->mEdgeV2V0Angle, &info->mEdgeV1V2Angle};
  //the index into the array is the sum index minus 1
  //(look at how the arrays above are constructed)
  uint flagIndex = verticesSumA - 1;

  //store the voronoi region (we made sure the sign is
  //correct for a counterclockwise edge earlier
  *mEdgeAngles[flagIndex] = voronoiRegionAngle;
  //mark as being in a convex or not state. Make sure to override this flag with the
  //state, not just if it's convex because this is run incrementally on painted meshes
  info->mEdgeFlags.SetState(flags[flagIndex], isConvex);
}

void GenerateInternalEdgeInfo(GenericPhysicsMesh* mesh, TriangleInfoMap* infoMap)
{
  infoMap->Clear();

  uint triangleCount = mesh->GetTriangleCount();

  //for now, loop n-squared through the triangles and see if any of
  //the share an edge. if so compute their voronoi regions.
  uint indexA = 0;
  while(indexA < triangleCount)
  {
    Triangle triA = mesh->GetTriangle(indexA);

    uint indexB = 0;
    while(indexB < triangleCount)
    {
      if(indexB != indexA)
      {
        Triangle triB = mesh->GetTriangle(indexB);
        ComputeEdgeInfoForTriangleA(triA, indexA, triB, infoMap);
      }
      ++indexB;
    }

    ++indexA;
  }
}

void GenerateInternalEdgeInfo(PhysicsMesh* mesh, TriangleInfoMap* infoMap)
{
  infoMap->Clear();

  //get the tree and make sure it exists
  typedef StaticAabbTree<uint> TreeType;
  TreeType* treePointer = mesh->GetAabbTree();
  if(treePointer == nullptr)
  {
    ErrorIf(true, "Physics mesh returned a null tree pointer, "
                  "tree must not have been constructed yet.");
    return;
  }
  //i believe the range macro below doesn't like pointers, so convert to a reference
  TreeType& tree = *treePointer;

  //loop over all of the triangles in the mesh, for each triangle send it through
  //the tree to determine which triangles should be checked for the more
  //expensive internal calculation (should I fatten the aabb?)
  uint triangleCount = mesh->GetTriangleCount();
  for(uint indexA = 0; indexA < triangleCount; ++indexA)
  {
    Triangle triA = mesh->GetTriangle(indexA);
    Aabb triAabb = ToAabb(triA);

    Array<TreeType::NodeType*> scratchData;
    AutoDeclare(range, tree.Query(triAabb,scratchData));
    for(; !range.Empty(); range.PopFront())
    //forRangeBroadphaseTree(StaticAabbTree<uint>, tree, Aabb, triAabb)
    {
      //aabb tree stores the index in the index buffer, so divide by 3,
      //would like to fix this but saved meshes would cause problems
      uint indexB = range.Front() / 3;
      //if not the same triangle, try to compute the voronoi edge info for the pair.
      if(indexA != indexB)
      {
        Triangle triB = mesh->GetTriangle(indexB);
        ComputeEdgeInfoForTriangleA(triA, indexA, triB, infoMap);
      }
    }
  }
}

static uint ComputeTriangleIndex(AbsoluteIndex absIndex, uint triIndex)
{
  uint mask = 0xffff;
  uint index = 0;
  index |= (((absIndex.x + 0x3fff) << 1) + triIndex) & mask;
  index |= ((absIndex.y + 0x7fff) & mask) << 16;
  return index;
}

void GenerateInternalEdgeInfo(HeightMapCollider* collider, TriangleInfoMap* infoMap)
{
  // Don't pre-cache any information for height-maps as they can be to big.
  // Instead cache as we visit a triangle (calls the dynamic version).
}

// Note yet used until brave cobra has height maps again
void GenerateInternalEdgeInfoDynamic(HeightMapCollider* collider, uint contactId)
{
  HeightMap* heightMap = collider->GetHeightMap();
  TriangleInfoMap* map = collider->GetInfoMap();
  
  // Get the indices of the contact id
  AbsoluteIndex absIndex;
  uint triIndex;
  CellIndex cellIndex;
  PatchIndex patchIndex;
  HeightMapCollider::KeyToTriangleIndex(contactId, absIndex, triIndex);
  heightMap->GetPatchAndCellIndex(absIndex, patchIndex, cellIndex);

  // Get the local position of the bottom left of the cell.
  // @JoshD: Using local position seems problematic with non-uniform scale (the angles change)
  real cellSize = heightMap->mUnitsPerPatch / HeightPatch::Size;
  Vec2 patchStart = heightMap->GetLocalPosition(patchIndex) - Vec2(heightMap->mUnitsPerPatch, heightMap->mUnitsPerPatch) * 0.5f;
  Vec2 cellStart = patchStart + Math::ToVec2(cellIndex) * cellSize;

  // For a given cell, there are potentially 4 extra triangles that need to be deal with.
  // For easy of naming, the bottom left of the current cell is called h11 instead of h00
  // so that cells to the left don't have to deal with negative numbers.
  //      h31
  //         |\    
  //         |  \  
  // h20_____|h21_\ h22
  //    \    |\    |\
  //      \  |  \  |  \
  //        \|____\|____\h13
  //      h11 \    |h12
  //            \  |
  //              \|
  //              h02

  // The 4 vertices of the current cell always need to be sampled
  float h11 = heightMap->SampleHeight(absIndex, Math::cInfinite);
  float h12 = heightMap->SampleHeight(heightMap->GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(1, 0)), Math::cInfinite);
  float h21 = heightMap->SampleHeight(heightMap->GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(0, 1)), Math::cInfinite);
  float h22 = heightMap->SampleHeight(heightMap->GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(1, 1)), Math::cInfinite);
  
  // If we are on the bottom left triangle then only build adjacency for the
  // 3 adjacent triangles: (h21, h12, h22), (h20, h11, h21), (h11, h02, h12)
  if(triIndex == 0)
  {
    // The triangle we are querying doesn't exist there's nothing to do
    if(h11 == Math::cInfinite || h12 == Math::cInfinite || h21 == Math::cInfinite)
      return;

    Vec3 p11 = Vec3(cellStart.x + cellSize * 0, h11, cellStart.y + cellSize * 0);
    Vec3 p12 = Vec3(cellStart.x + cellSize * 0, h12, cellStart.y + cellSize * 1);
    Vec3 p21 = Vec3(cellStart.x + cellSize * 1, h21, cellStart.y + cellSize * 0);
    Triangle mainTri = Triangle(p11, p12, p21);

    float h20 = heightMap->SampleHeight(heightMap->GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(-1, 1)), Math::cInfinite);
    float h02 = heightMap->SampleHeight(heightMap->GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(1, -1)), Math::cInfinite);

    // Try each adjacent triangle if it exists
    if(h22 != Math::cInfinite)
    {
      Vec3 p22 = Vec3(cellStart.x + cellSize * 1, h22, cellStart.y + cellSize * 1);
      Triangle testTri = Triangle(p12, p22, p21);
      ComputeEdgeInfoForTriangleA(mainTri, contactId, testTri, map);
    }
    if(h20 != Math::cInfinite)
    {
      Vec3 p20 = Vec3(cellStart.x + cellSize * -1, h20, cellStart.y + cellSize * 1);
      Triangle testTri = Triangle(p20, p11, p21);
      ComputeEdgeInfoForTriangleA(mainTri, contactId, testTri, map);
    }
    if(h02 != Math::cInfinite)
    {
      Vec3 p02 = Vec3(cellStart.x + cellSize * 1, h02, cellStart.y + cellSize * -1);
      Triangle testTri = Triangle(p11, p02, p12);
      ComputeEdgeInfoForTriangleA(mainTri, contactId, testTri, map);
    }
  }
  // If we are on the top right triangle then only build adjacency for the
  // 3 adjacent triangles: (h11, h12, h21), (h31, h21, h22), (h22, h12, h13)
  else
  {
    // The triangle we are querying doesn't exist there's nothing to do
    if(h21 == Math::cInfinite || h12 == Math::cInfinite || h22 == Math::cInfinite)
      return;

    Vec3 p21 = Vec3(cellStart.x + cellSize * 0, h21, cellStart.y + cellSize * 1);
    Vec3 p12 = Vec3(cellStart.x + cellSize * 1, h12, cellStart.y + cellSize * 0);
    Vec3 p22 = Vec3(cellStart.x + cellSize * 1, h22, cellStart.y + cellSize * 1);
    Triangle mainTri = Triangle(p21, p12, p22);

    float h31 = heightMap->SampleHeight(heightMap->GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(0, 1)), Math::cInfinite);
    float h13 = heightMap->SampleHeight(heightMap->GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(1, -1)), Math::cInfinite);

    // Try each adjacent triangle if it exists
    if(h11 != Math::cInfinite)
    {
      Vec3 p11 = Vec3(cellStart.x + cellSize * 0, h11, cellStart.y + cellSize * 0);
      Triangle testTri = Triangle(p11, p12, p21);
      ComputeEdgeInfoForTriangleA(mainTri, contactId, testTri, map);
    }
    if(h31 != Math::cInfinite)
    {
      Vec3 p31 = Vec3(cellStart.x + cellSize * 0, h31, cellStart.y + cellSize * 2);
      Triangle testTri = Triangle(p31, p21, p22);
      ComputeEdgeInfoForTriangleA(mainTri, contactId, testTri, map);
    }
    if(h13 != Math::cInfinite)
    {
      Vec3 p13 = Vec3(cellStart.x + cellSize * 2, h13, cellStart.y + cellSize * 0);
      Triangle testTri = Triangle(p22, p12, p13);
      ComputeEdgeInfoForTriangleA(mainTri, contactId, testTri, map);
    }
  }
}

struct StateInfo
{
  //per manifold data
  Collider* mCollider;
  MeshTriangleInfo* mInfo;
  Triangle mTri;
  Vec3 mTriNormal;
  uint mObjectIndex;

  //per manifold point data
  Vec3 mLocalContactNormal;
  Vec3 mNearestPoint;
  uint mBestEdge;
  real mClosestDistance;

  //evaluating edge data
  Vec3 mEdge;
  real mVoronoiAngle;
  uint mConvexTestFlag;
};

void TestEdgeCloseness(Vec3Param contactPoint, Vec3Param point1, Vec3Param point2,
                       real angle, uint testEdgeIndex, StateInfo& stateInfo)
{
  //if this edge is too sharp, then we don't correct the edge
  real maxAngleThreshold = Math::cTwoPi;
  if(angle >= maxAngleThreshold)
    return;

  //find the nearest point on the line segment to the contact point
  Vec3 nearestPoint = contactPoint;
  Intersection::ClosestPointOnSegmentToPoint(point1, point2, &nearestPoint);

  //get the distance between the contact point and the closest point on the line
  Vec3 dir = nearestPoint - contactPoint;
  real length = dir.Length();

  //the normal approach would just be to save this edge as closest if it's distance
  //is less than the previous edge, however there can be cases when the point is
  //right on a corner and the distances are incredibly close to each other where
  //the closest edge is not the desired one. In the case that we detect very close
  //edge proximity (aka within an epsilon of the last distance) we want the edge
  //that makes the most sense to rotate the triangle normal about to get the contact
  //normal. The way to test this is to take the edge that is most perpendicular to
  //the contact normal, this means it is the closest to a pure rotation to get the
  //tri normal into the contact normal.
  real absDiff = Math::Abs(length - stateInfo.mClosestDistance);
  if(absDiff < real(.0001))
  {
    //make the current edge
    Vec3 currEdge = (point2 - point1).Normalized();
    //test the angle between the current edge and the previous edge
    real oldEdgePerpTest = Math::Dot(stateInfo.mLocalContactNormal, stateInfo.mEdge);
    real newEdgePerpTest = Math::Dot(stateInfo.mLocalContactNormal, currEdge);

    //if the new edge is more perpendicular than the previous
    //one, save that as the closest
    if(Math::Abs(newEdgePerpTest) < Math::Abs(oldEdgePerpTest))
    {
      stateInfo.mClosestDistance = length;
      stateInfo.mBestEdge = testEdgeIndex;
      stateInfo.mNearestPoint = nearestPoint;
      stateInfo.mEdge = currEdge;
    }
    return;
  }

  //if this is now the closest distance, update the state variables
  if(length < stateInfo.mClosestDistance)
  {
    stateInfo.mClosestDistance = length;
    stateInfo.mBestEdge = testEdgeIndex;
    stateInfo.mNearestPoint = nearestPoint;
    stateInfo.mEdge = (point2 - point1).Normalized();
  }
}

//clamps the normal to the voronoi region (on one side) if it is outside the bounds
bool ClampNormal(Vec3Param edge, Vec3Param triNormal, Vec3Param localContactNormal,
                 real voronoiAngle, Vec3Ref clampedLocalNormal)
{
  //build our remaining vector needed for out basis
  Vec3 edgeCross = Math::Cross(edge, triNormal);
  //get the angle that the contact normal is from the edge, normal basis (y-x ordering)
  real angle = GetAngle(edgeCross, triNormal, localContactNormal);

  //if our current angle from the triangle normal is outside of the Voronoi
  //region bounds on the correct side then we need to correct it.
  //We only correct on one side because we only need to fix normals that point
  //in the direction away from the edge as those are the only ones that will
  //cause collision issues.
  if((voronoiAngle < 0 && angle < voronoiAngle) ||
     (voronoiAngle >= 0 && angle > voronoiAngle))
  {
    //compute the matrix that will rotate us right to the edge of the Voronoi region
    real angleDiff = voronoiAngle - angle;
    Mat3 correctionRotation = Math::ToMatrix3(edge, angleDiff);
    //that rotated normal is the new contact normal
    clampedLocalNormal = Math::Transform(correctionRotation, localContactNormal);
    return true;
  }

  return false;
}

void DebugDrawVoronoiRegion(Physics::ManifoldPoint& point, StateInfo& stateInfo, Vec3Param nA, Vec3Param nB)
{
  Vec3 p = point.BodyPoints[stateInfo.mObjectIndex];
  Mat4 rot = stateInfo.mCollider->GetOwner()->has(Transform)->GetWorldMatrix();
  p = Math::TransformPoint(rot,p);

  gDebugDraw->Add(Debug::Line(p, p + nA * real(2.0)).Color(Color::OrangeRed));
  gDebugDraw->Add(Debug::Line(p, p + nB * real(2.0)).Color(Color::Red));
}

void FixOtherPoint(Physics::ManifoldPoint& point, StateInfo& stateInfo)
{
  //Now that we've altered the normal, we need to move the point on the
  //triangle mesh to be along the normal from the point on the other object.
  //If we don't do this it'll cause problems with the persistent
  //manifold when checking the perpendicular (shear) distance.

  //get the indices of the triangle mesh and the other object, also flip
  //the normal sign to point from the other object to the triangle mesh
  uint indexA = stateInfo.mObjectIndex;
  uint indexB = (indexA + 1) % 2;
  Vec3 normal = point.Normal;
  if(indexA == 1)
    normal *= -1;

  //create the new point on the mesh along the normal by the penetration distance
  Vec3 worldPoint = point.WorldPoints[indexB] + normal * point.Penetration;
  point.WorldPoints[indexA] = worldPoint;

  //update the local point as well
  Transform* t = stateInfo.mCollider->GetOwner()->has(Transform);
  Mat4 transform = t->GetWorldMatrix();
  Mat4 invTransform = transform.Inverted();
  point.BodyPoints[indexA] = Math::TransformPoint(invTransform, worldPoint);
}

//simple helper to fix the normal in the concave case
void CorrectConcaveNormal(Physics::ManifoldPoint& point, StateInfo& stateInfo)
{
  real dirEpsilon = real(.05);

  //in a concave case, we just want to use the triangle normal (as the edge of
  //the voronoi region we care about is our normal). However, we have to make
  //sure that the normal faces the correct direction. If the triangle normal is
  //facing the opposite way of the contact normal then use the negative tri normal.
  Vec3 triNormal = stateInfo.mTriNormal;
  Vec3 contactNormal = stateInfo.mLocalContactNormal;
  real dirTest = Math::Dot(triNormal, contactNormal);
  if(dirTest < -dirEpsilon)
   triNormal *= -1;

  //triangle normal is in local space, bring it to world space
  point.Normal = Math::Transform(stateInfo.mCollider->GetWorldRotation(), triNormal);
  point.Normal.Normalize();
  FixOtherPoint(point, stateInfo);
}

void EvaluateBestEdge(Physics::ManifoldPoint& point, StateInfo& stateInfo, bool& onEdge)
{
  //hardcoded values for now, maybe make tweakable or something later
  real convexEpsilon = real(.001);
  real edgeDistanceThreshold = real(.001);

  //if we weren't closest enough to the edge, then we are in the
  //center of the triangle and we don't want to fix anything
  if(stateInfo.mClosestDistance >= edgeDistanceThreshold)
  {
    onEdge = false;
    //unless the normal is on the backface, then flip it
    if (!sAllowBackfaces)
    {
      real dirEpsilon = real(.05);
      Vec3 triNormal = stateInfo.mTriNormal;
      Vec3 contactNormal = stateInfo.mLocalContactNormal;
      real dirTest = Math::Dot(triNormal, contactNormal);
      if(dirTest < -dirEpsilon)
        point.Normal = -point.Normal;
    }
    return;
  }

  onEdge = true;

  //get the edge we are testing
  Vec3 edge = stateInfo.mEdge;
  edge.Normalize();

  //if the triangles are flat on this edge, then just treat it as a
  //concave case (which just uses the triangle normal)
  if(stateInfo.mVoronoiAngle == real(0.0))
  {
    //if backface collision is allowed, we can use concave correction here
    if (sAllowBackfaces)
    {
      CorrectConcaveNormal(point, stateInfo);
    }
    //otherwise we need the triangle normal and not allow it to be flipped
    else
    {
      point.Normal = Math::Transform(stateInfo.mCollider->GetWorldRotation(), stateInfo.mTriNormal);
      point.Normal.Normalize();
      FixOtherPoint(point, stateInfo);
    }
    return;
  }

  //otherwise we are in a convex or concave state. We need to see which
  //one we are in to figure out how we are fixing the normal.
  bool isConvex = stateInfo.mInfo->mEdgeFlags.IsSet(stateInfo.mConvexTestFlag);

  real voronoiAngle = stateInfo.mVoronoiAngle;
  //If we are concave, we want to change our test to be the back face
  //(or the "convex" side) of the edge. To do this we swap the normals to be on
  //the negative side. This also has the effect that if we test to see if we are
  //on the "back face" (the concave side) of the edge then we just use the normal.
  real swapFactor = isConvex ? real(1.0) : real(-1.0);

  //get the normal of A (this triangle)
  Vec3 nA = stateInfo.mTriNormal;// * swapFactor;
  //build a rotation matrix that transforms A's normal into B
  Mat3 rot = Math::ToMatrix3(edge, voronoiAngle);
  //get B's normal
  Vec3 nB = Math::Transform(rot, nA);

  //hack debug draw right now
  //DebugDrawVoronoiRegion(point, stateInfo, nA, nB);

  //check the directionality of the contact normal with both of the triangle normals
  real nDotA = Math::Dot(stateInfo.mLocalContactNormal, nA);
  real nDotB = Math::Dot(stateInfo.mLocalContactNormal, nB);

  //test to see if we are back facing on both triangles (if we are on the
  //negative side of both normals). If so, we don't correct to the Voronoi region,
  //we instead use the triangle normal, however since we are on the
  //back use the negative normal
  bool backFacingNormal = (nDotA < convexEpsilon);// && (nDotB < convexEpsilon);
  if(backFacingNormal)
  {
    if (sAllowBackfaces)
    {
      CorrectConcaveNormal(point, stateInfo);
    }
    //not allowing backfaces, so there are only two cases here
    else
    {
      //if edge is convex, then the closest front face normal is on the other triangle (B's normal)
      if (isConvex)
        point.Normal = Math::Transform(stateInfo.mCollider->GetWorldRotation(), nB);
      //when concave, the closest normal in the voronoi region is our own (A's normal)
      else
        point.Normal = Math::Transform(stateInfo.mCollider->GetWorldRotation(), nA);

      point.Normal.Normalize();
      FixOtherPoint(point, stateInfo);
    }
    return;
  }

  //if contact normal is forward facing and edge is concave
  if (!isConvex)
  {
    CorrectConcaveNormal(point, stateInfo);
    return;
  }

  //otherwise, we actually want to clamp the normal to the Voronoi region
  Vec3 clampedNormal;
  bool isClamped = ClampNormal(edge, nA*swapFactor, stateInfo.mLocalContactNormal, voronoiAngle, clampedNormal);

  //if we didn't clamp then there is nothing to do
  if(!isClamped)
    return;

  //if we did clamp, update the normal (add fixing the point later too)
  Vec3 newNormal = Math::Transform(stateInfo.mCollider->GetWorldRotation(), clampedNormal);
  point.Normal = newNormal;
  point.Normal.Normalize();
  FixOtherPoint(point, stateInfo);
}

bool CorrectPointInternalEdgeNormal(Physics::ManifoldPoint& point, StateInfo& stateInfo)
{
  //we have to bring the normal back into local space on the mesh since all
  //of the Voronoi region info was computed in local space
  Mat3 invRot = stateInfo.mCollider->GetWorldRotation().Transposed();
  //We always assume the normal is pointing from the correction object to the corrected
  //object (eg. from mesh to sphere). If the order is the opposite then flip the normal for correction
  if(stateInfo.mObjectIndex == 1)
    point.Normal *= -1;
  stateInfo.mLocalContactNormal = Math::Transform(invRot, point.Normal);
  stateInfo.mLocalContactNormal.Normalize();

  //initialize the closest info data
  stateInfo.mNearestPoint = Vec3::cZero;
  stateInfo.mBestEdge = (uint)-1;
  stateInfo.mClosestDistance = Math::PositiveMax();

  Vec3 bodyPoint = point.BodyPoints[stateInfo.mObjectIndex];
  Triangle tri = stateInfo.mTri;
  MeshTriangleInfo* info = stateInfo.mInfo;
  //test all 3 edges to see which one is the closest, store which one
  //is the closest, how close it is as well as what the closest point
  //on the edge is to the contact point
  TestEdgeCloseness(bodyPoint, tri.p0, tri.p1, info->mEdgeV0V1Angle, 0, stateInfo);
  TestEdgeCloseness(bodyPoint, tri.p2, tri.p0, info->mEdgeV2V0Angle, 1, stateInfo);
  TestEdgeCloseness(bodyPoint, tri.p1, tri.p2, info->mEdgeV1V2Angle, 2, stateInfo);

  bool onEdge = false;
  //now evaluate the best edge and see whether or not we need to fix the normal
  if(stateInfo.mBestEdge == 0)
  {
    stateInfo.mVoronoiAngle = info->mEdgeV0V1Angle;
    stateInfo.mConvexTestFlag = TriangleInfoFlags::V0V1Convex;
    stateInfo.mEdge = stateInfo.mTri.p0 - stateInfo.mTri.p1;
    EvaluateBestEdge(point, stateInfo, onEdge);
  }
  else if(stateInfo.mBestEdge == 1)
  {
    stateInfo.mVoronoiAngle = info->mEdgeV2V0Angle;
    stateInfo.mConvexTestFlag = TriangleInfoFlags::V2V0Convex;
    stateInfo.mEdge = stateInfo.mTri.p2 - stateInfo.mTri.p0;
    EvaluateBestEdge(point, stateInfo, onEdge);
  }
  else if(stateInfo.mBestEdge == 2)
  {
    stateInfo.mVoronoiAngle = info->mEdgeV1V2Angle;
    stateInfo.mConvexTestFlag = TriangleInfoFlags::V1V2Convex;
    stateInfo.mEdge = stateInfo.mTri.p1 - stateInfo.mTri.p2;
    EvaluateBestEdge(point, stateInfo, onEdge);
  }

  if(stateInfo.mObjectIndex == 1)
    point.Normal *= -1;

  return onEdge;
}

void CorrectInternalEdgeNormal(Physics::ManifoldPoint& point, uint objectIndex, Collider* meshCollider, uint triId)
{
  GenericPhysicsMesh* mesh = nullptr;
  HeightMapCollider* heightMap = nullptr;
  //get the generic physics mesh from the collider
  if(meshCollider->GetColliderType() == Collider::cConvexMesh)
  {
    ConvexMeshCollider* cmCollider = static_cast<ConvexMeshCollider*>(meshCollider);
    ConvexMesh* convexMesh = cmCollider->GetConvexMesh();
    mesh = convexMesh;
  }
  else if(meshCollider->GetColliderType() == Collider::cMesh)
  {
    MeshCollider* mCollider = static_cast<MeshCollider*>(meshCollider);
    PhysicsMesh* physicsMesh = mCollider->GetPhysicsMesh();
    mesh = physicsMesh;
  }
  else if (meshCollider->GetColliderType() == Collider::cHeightMap)
  {
    heightMap = static_cast<HeightMapCollider*>(meshCollider);
  }
  else
  {
    ErrorIf(true, "Invalid collider type for edge correction. "
      "Only allowed types are convex mesh and normal mesh.", meshCollider);
    return;
  }

  //get the collided triangle and it's normal
  Triangle tri;
  TriangleInfoMap* map = nullptr;
  if (mesh)
  {
    tri = mesh->GetTriangle(triId);
    map = mesh->GetInfoMap();
  }
  else
  {
    tri = heightMap->GetTriangle(triId);
    map = heightMap->GetInfoMap();
  }

  MeshTriangleInfo* info = map->FindPointer(triId);

  ErrorIf(info == nullptr, "Triangle map did not contain info for triangle of index %d", triId);

  StateInfo stateInfo;
  stateInfo.mCollider = meshCollider;
  stateInfo.mInfo = info;
  stateInfo.mTri = tri;
  stateInfo.mTriNormal = tri.GetNormal();
  stateInfo.mObjectIndex = objectIndex;

  CorrectPointInternalEdgeNormal(point, stateInfo);
}

void CorrectInternalEdgeNormal(GenericPhysicsMesh* mesh, Physics::Manifold* manifold, uint objectIndex, uint contactId)
{
  //get the collider that is the mesh
  Collider* collider = manifold->Objects[objectIndex];
  //get the collided triangle and it's normal
  Triangle tri = mesh->GetTriangle(contactId);
  //grab the mesh info for the collided triangle, if it
  //didn't exist then there is nothing for us to do
  TriangleInfoMap* map = mesh->GetInfoMap();
  MeshTriangleInfo* info = map->FindPointer(contactId);
  if(info == nullptr)
    return;

  //set up some info on our state struct for ease of passing around
  StateInfo stateInfo;
  stateInfo.mCollider = collider;
  stateInfo.mInfo = info;
  stateInfo.mTri = tri;
  stateInfo.mTriNormal = tri.GetNormal();
  stateInfo.mObjectIndex = objectIndex;

  bool edgeCorrected[cMaxContacts] = {0};

  //attempt to correct all of the points
  for(uint i = 0; i < manifold->ContactCount; ++i)
    edgeCorrected[i] = CorrectPointInternalEdgeNormal(manifold->Contacts[i],stateInfo);

  // Correct manifold points to nearest corrected point on edge (if any)
  for (uint i = 0; i < manifold->ContactCount; ++i)
  {
    // Don't modify edge corrected
    if (!edgeCorrected[i])
    {
      real closestDistance = Math::PositiveMax();
      bool hasClosest = false;
      uint index = uint(-1);

      // Search for other point
      for (uint j = 0; j < manifold->ContactCount; ++j)
      {
        // Ignore self and non edge corrected
        if (i == j) continue;
        if (!edgeCorrected[j]) continue;

        real distance = (manifold->Contacts[i].WorldPoints[objectIndex] - manifold->Contacts[j].WorldPoints[objectIndex]).Length();
        if (!hasClosest)
        {
          closestDistance = distance;
          index = j;
          hasClosest = true;
        }
        else if (distance < closestDistance)
        {
          closestDistance = distance;
          index = j;
        }
      }

      // Change normal and other point
      if (hasClosest)
      {
        manifold->Contacts[i].Normal = manifold->Contacts[index].Normal;
        FixOtherPoint(manifold->Contacts[i], stateInfo);
      }
    }
  }
}

void CorrectInternalEdgeNormal(HeightMapCollider* collider, Physics::Manifold* manifold, uint objectIndex, uint contactId)
{
  //get the collided triangle and it's normal
  Triangle tri = collider->GetTriangle(contactId);
  //grab the mesh info for the collided triangle, if it
  //didn't exist then there is nothing for us to do
  TriangleInfoMap* map = collider->GetInfoMap();

  // If the map is too big then we risk crashing since the map is contiguous memory.
  // For now it's easiest to clear the map when we exceed some threshold and let it
  // gradually re-populate over time. This should only happen if lots of triangles have
  // been touched over different areas of the height map.
  if(map->Size() > 100000)
    collider->ClearCachedEdgeAdjacency();

  MeshTriangleInfo* info = map->FindPointer(contactId);
  // If this entry didn't already exist then create it
  if(info == nullptr)
  {
    GenerateInternalEdgeInfoDynamic(collider, contactId);
    info = map->FindPointer(contactId);
    ReturnIf(info == nullptr, , "Somehow creating an entry for a contact id didn't actually create the entry");
  }

  //set up some info on our state struct for ease of passing around
  StateInfo stateInfo;
  stateInfo.mCollider = collider;
  stateInfo.mInfo = info;
  stateInfo.mTri = tri;
  stateInfo.mTriNormal = tri.GetNormal();
  stateInfo.mObjectIndex = objectIndex;

  //attempt to correct all of the points
  for(uint i = 0; i < manifold->ContactCount; ++i)
  {
    Physics::ManifoldPoint point = manifold->Contacts[i];
    // Project point P along direction d onto plane N,Q to get point R
    // R = P + d * t, (R - Q).N = 0
    // ((P + d * t) - Q).N = 0
    // P.N + d.N * t - Q.N = 0
    // d.N * t = Q.N - P.N
    // t = (Q - P).N / d.N
    Vec3 N = stateInfo.mTriNormal;
    Vec3 Q = stateInfo.mTri.p0;
    Vec3 P = point.BodyPoints[objectIndex];
    Vec3 d = HeightMap::UpVector;
    real t = Math::Dot(Q - P, N) / Math::Dot(d, N);
    Vec3 R = P + d * t;

    // Changing body point to be on the triangle so that edge correction
    // can identify which swept edge was hit without modifying how correction works
    Mat4 transform = collider->GetOwner()->has(Transform)->GetWorldMatrix();
    manifold->Contacts[i].BodyPoints[objectIndex] = R;
    manifold->Contacts[i].WorldPoints[objectIndex] = Math::TransformPoint(transform, R);
    // Penetration should not be over written by the projection parameter
    // manifold->Contacts[i].Penetration = t;
    CorrectPointInternalEdgeNormal(manifold->Contacts[i],stateInfo);
  }
}

}//namespace Zero
