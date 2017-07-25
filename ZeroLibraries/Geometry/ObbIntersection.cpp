///////////////////////////////////////////////////////////////////////////////
///
/// \file ObbIntersection.cpp
/// All of the intersection tests that involve oriented bounding boxes.
///
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "Geometry/GeometryStandard.hpp"
#include "Geometry/Intersection.hpp"
#include "Geometry/Geometry.hpp"
#include "Geometry/Solids.hpp"


#define DrawIntersectionInfoX
#ifdef DrawIntersectionInfo
#include "DebugDraw.hpp"
#define VisualizeObbObbTestAxes
#endif

namespace Intersection
{

namespace
{
//Threshold used to determine what kind of collision has occurred between an
//oriented bounding box and a halfspace. The points of the oriented bounding box
//are projected onto the halfspace's normal and if the negative projections
//differ by less than this amount then the points are grouped together to result
//in a face, edge, or point for the contact point.
const real cObbPlanePoints = real(0.07);

//Threshold used to determine if two oriented bounding boxes are colliding on an
//axis. Their points are projected onto that axis and if the distance between
//the centers of the oriented bounding boxes is greater than the half-lengths of
//their projections then they are separating. This test is done using
//"projections - distance" and that value is compared with this threshold.
const real cObbObbZero = real(0.0002);

//This is used to bias towards or against the edge-edge obb-obb cases. In
//general, we bias against the edge-edge cases to counteract situations where
//a face-something case is more desirable.
const real cObbObbFudgeFactor = real(1.05);

//This is used to check if the normal is pointing from box A to box B. The
//normal is compared (dotted) against the vector from the center of box A to box
//B. If this dot product is above this epsilon, then the normal is pointing the
//wrong way.
const real cObbObbNormalCheck = real(-0.0000001);

//This is used to check if the contact points are stored with box A first and
//box B second (in an array). The vector from box A's contact point to box B's
//contact point is compared (dotted) against the vector from the center of box A
//to box B. If this dot product is above this epsilon, then the points are in
//the wrong order.
const real cObbObbPointCheck = real(0.0000001);

//This is used to bias toward the face-triangle case for the obb-triangle test.
//The reason for this is because the axes in the test are tested with the cross
//product axes first, then the AABBs of the two objects, then the triangle's
//normal last.
const real cObbTriFudgeFactor = real(0.95);


const real cObbTriangleZero = real(-0.0002);

//Threshold used to determine if two oriented bounding boxes orientation axes
//are parallel. If they are parallel then their dot product results in a value
//close to 1. If their dot product is greater than this value then they are
//considered to be parallel.
const real cObbParallelAxis = real(0.98);

const real cObbEpsilon = real(0.000001);

const real cSimdEpsilon = real(1.192092896e-07);

//Used to precompute indices for the axes of oriented bounding boxes in the
//ObbObb test
//                          Primary Axes
const uint cObbAxes[15] = {  65536,  // One's X                 0x00010000
                            131072,  // One's Y                 0x00020000
                            196608,  // One's Z                 0x00030000
                                 1,  // Two's X                 0x00000001
                                 2,  // Two's Y                 0x00000002
                                 3,  // Two's Z                 0x00000003
//                          Cross Product Axes
                             65537,  // One's X CROSS Two's X   0x00010001
                             65538,  // One's X CROSS Two's Y   0x00010002
                             65539,  // One's X CROSS Two's Z   0x00010003
                            131073,  // One's Y CROSS Two's X   0x00020001
                            131074,  // One's Y CROSS Two's Y   0x00020002
                            131075,  // One's Y CROSS Two's Z   0x00020003
                            196609,  // One's Y CROSS Two's X   0x00030001
                            196610,  // One's Z CROSS Two's Y   0x00030002
                            196611   // One's Z CROSS Two's Z   0x00030003
                          };
}// namespace

 ///Contains a point and a projection value for use in intersection tests 
 ///involving shapes with discrete points (eg: oriented-bounding boxes)
struct PointInfo
{
  Vec3 Point;
  real Projection;
  uint Index;

  PointInfo(void)
    : Point(Vec3::cZero), Projection(real(0.0))
  {
    //
  }

  PointInfo(Vec3Param point, real projection)
    : Point(point), Projection(projection)
  {
    //
  }

  bool operator < (const PointInfo& rhs) const
  {
    return Projection < rhs.Projection;
  }

  PointInfo& operator += (const PointInfo& rhs)
  {
    Point += rhs.Point;
    Projection += rhs.Projection;
    return *this;
  }

  PointInfo& operator *= (real rhs)
  {
    Point *= rhs;
    Projection *= rhs;
    return *this;
  }
};

typedef Zero::PodArray<PointInfo> PointInfoArray;

///Calculate the min and the max of two values and store them in the input
///parameters in the respective order
void MinMaxInPlace(real& min, real& max)
{
  if(min > max)
  {
    Math::Swap(min, max);
  }
}

///Given the point in world coordinates, find the vertex of the box that is
///closest to it.
bool ObbPointIndexFromPoint(Vec3Param worldPoint, Vec3Param obbCenter,
                            Mat3Param obbBasis, uint& index)
{
  Vec3 obbToPoint = worldPoint - obbCenter;
  TransposedTransform(obbBasis, &obbToPoint);
  for(uint i = 0; i < 8; ++i)
  {
    bool pointFound = true;
    for(uint j = 0; j < 3; ++j)
    {
      if(Math::GetSign(obbToPoint[j]) !=
         Math::GetSign(Geometry::Box::cPoint[i][j]))
      {
        pointFound = false;
        break;
      }
    }
    if(pointFound)
    {
      index = i;
      return true;
    }
  }
  return false;
}

///Clip the quadrilateral against the rectangle. This assumes that the
///rectangle's center is at the origin and its axes are aligned with the world
///axes. This is all done in a 2D space rather than 3D. Borrowed heavily from
///Bullet.
uint ClipQuadWithRectangle(Vec2Param rectangleHalfExtents, const Vec2* inPoints,
                           Vec2* outPoints)
{
  //Rectangle sides are tested in the following order: -x, +x, -y, +y
  uint quadPointCount = 4;  //Quadrilateral starts off with 4 points
  uint clipPointCount = 0;  //No points have been clipped yet

  //Temporary buffer to store clipped points in
  Vec2 tempPoints[8];

  const Vec2* quadPoint = inPoints;
  Vec2* clipPoint = outPoints;

  //For the x-axis and the y-axis...
  for(uint d = 0; d < 2; ++d)
  {
    //sign is used to determine which side of the rectangle we are currently
    //clipping against (negative side or positive side)
    for(int sign = -1; sign < 2; sign += 2)
    {
      const real realSign = real(sign);   //Used throughout the loop
      const Vec2* curr = quadPoint;  //Current point of the quad
      Vec2* clip = clipPoint;        //Where to store the end resulting point
      clipPointCount = 0;            //No clipping has occurred on this side yet

      //Used to help clarify when the next point of the quad should be the first
      //point of the quad
      uint nextEnd = quadPointCount - 1;

      //Go through all of the edges of the quad and attempt to clip them against
      //the current side of the rectangle.
      for(uint i = 0; i < quadPointCount; ++i)
      {
        //Test to see which side of the current rectangle edge the current
        //quadrilateral point is on.
        bool currInside = (realSign * (*curr)[d]) < rectangleHalfExtents[d];

        //The current quadrilateral point is on the inside of the current
        //rectangle edge
        if(currInside)
        {
          //Add this point to the "new" quad
          *clip = *curr;

          //Go to the next spot in the clipped points buffer
          ++clip;

          //The number of points in the clipped quadrilateral has increased
          ++clipPointCount;

          //If the number of maximum points have been generated, quit
          if(clipPointCount == Geometry::cMaxSupportPoints)
          {
            quadPoint = clipPoint;
            memcpy(outPoints, quadPoint, clipPointCount * sizeof(Vec2));
            return clipPointCount;
          }
        }

        //Now test to see if the current edge of the quadrilateral crosses the
        //current edge of the rectangle. If so, calculate the intersection of
        //the two edges and save the result.

        //Get the next point in the quadrilateral. If at the end, use the first
        //point in the quad
        const Vec2* next = (i == nextEnd) ? quadPoint : curr + 1;

        //Test to see which side of the current rectangle edge the next quad
        //point is on.
        bool nextInside = (realSign * (*next)[d]) < rectangleHalfExtents[d];

        //Only one of the points is behind the plane
        if(currInside ^ nextInside)
        {
          //Get the axis we aren't on.
          uint n = 1 - d;

          //Clip the quad edge against the rectangle edge.
          // EXAMPLE
          //                                        (newPoint.x - curr.x)
          // newPoint.y = curr.y + (next.y - curr.y)(--------------------)
          //                                        (next.x - curr.x  )
          Vec2& newPoint = *clip;
          newPoint[d] = realSign * rectangleHalfExtents[d];
          newPoint[n] = newPoint[d] - (*curr)[d];
          newPoint[n] /= (*next)[d] - (*curr)[d];
          newPoint[n] *= (*next)[n] - (*curr)[n];
          newPoint[n] += (*curr)[n];

          //Go to the next spot in the clipped points buffer
          ++clip;

          //The number of points in the clipped quadrilateral has increased
          ++clipPointCount;

          //If the number of maximum points have been generated, quit
          if(clipPointCount == Geometry::cMaxSupportPoints)
          {
            quadPoint = clipPoint;
            memcpy(outPoints, quadPoint, clipPointCount * sizeof(Vec2));
            return clipPointCount;
          }
        }

        //Go to the next point in the quad
        ++curr;
      }

      //The quad is now the results of the clipping that may have occurred
      quadPoint = clipPoint;

      //Alternate between the clippedPoints and the temporary point buffer
      clipPoint = (quadPoint == outPoints) ? tempPoints : outPoints;

      //The quad now has a new point count, after the clipping
      quadPointCount = clipPointCount;
    }
  }

  //If, at the end, the quad isn't pointing at the return buffer, copy the data
  //from the quad buffer into the return buffer.
  if(quadPoint != outPoints)
  {
    memcpy(outPoints, quadPoint, clipPointCount * sizeof(Vec2));
  }
  return clipPointCount;
}

void GetExtremePoints(const Vec2* points, uint pointCount, uint maxPoints,
                      uint initIndex, uint* pointIndices)
{
  Vec2 centroid;
  Geometry::CalculatePolygonCentriod(points, pointCount, &centroid);

  //Compute the angle of each point with respect to the centroid and initialize
  //the array that represents whether or not the point has been added to the
  //index array (0 = added, 1 = not added).
  //(Combined into 1 loop in order to reduce code size)
  real angles[8];
  bool inArray[8];
  for(uint i = 0; i < pointCount; ++i)
  {
    angles[i] = Math::ArcTan2(points[i].y - centroid.y,
                              points[i].x - centroid.x);
    inArray[i] = false;
  }

  //The initial point is always added.
  inArray[initIndex] = true;

  //For moving the pointer through the array rather than indexing it all the
  //time when setting new values.
  uint* currPointIndex = pointIndices;

  //The initial point is always added.
  *currPointIndex = initIndex;
  ++currPointIndex;

  //Sweep around the centroid of the polygon searching for points most evenly
  //distributed across the polygon. Hard to explain!

  //This is essentially the unit circle cut up into "maxPoints-number-of-slices"
  const real cPieceOfPi = Math::cTwoPi / real(maxPoints);

  real offsetFromInit;
  for(uint i = 0; i < maxPoints; ++i)
  {
    offsetFromInit = real(i) * cPieceOfPi + angles[initIndex];

    //Keep the angle offset in the range of (-PI/2, PI/2]
    if(offsetFromInit > Math::cPi)
    {
      offsetFromInit -= Math::cTwoPi;
    }

    //Typical "find the smallest value" variable for a "for" loop.
    real minAngleDiff = Math::PositiveMax();

    //Used for each iteration below
    real angleDiff;

    //This is just for error checking. If this doesn't get set to some other
    //index then something went wrong.
    *currPointIndex = initIndex;

    for(uint j = 0; j < pointCount; ++j)
    {
      //Only consider points that haven't been added
      if(inArray[j] == false)
      {
        angleDiff = Math::Abs(angles[j] - offsetFromInit);

        //Keep the angle difference in the range of [0, PI/2)
        if(angleDiff > Math::cPi)
        {
          angleDiff = Math::cTwoPi - angleDiff;
        }

        //This angle difference is smaller, this point is closer to the current
        //offset from the point at the initial index.
        if(angleDiff < minAngleDiff)
        {
          minAngleDiff = angleDiff;
          *currPointIndex = j;
        }
      }
    }

    //Make sure that the index got set.
    ErrorIf(*currPointIndex == initIndex, "Intersection - Something went wrong"\
                                          " when trying to find points.");

    inArray[*currPointIndex] = true;
    ++currPointIndex;
  }
}

void ObbObbEdgeCase(Vec3Param obbOneCenter, Vec3Param obbOneHalfExtents,
                    const Vec3* obbOneAxes, Mat3Param obbOneBasis,
                    Vec3Param obbTwoCenter, Vec3Param obbTwoHalfExtents,
                    const Vec3* obbTwoAxes, Mat3Param obbTwoBasis,
                    Vec3Ref normal, uint axisCase, real projection,
                    Manifold& manifold)
{
  //The normal is currently in OBB one's space, bring it into world space.
  Transform(obbOneBasis, &normal);

  //Make sure the normal is pointing from One to Two
  {
    Vec3 oneToTwo = obbTwoCenter - obbOneCenter;
    real normalCheck = Dot(oneToTwo, normal);
    if(normalCheck < real(0.0))
    {
      normal *= real(-1.0);
    }
  }

  //Look for one of the points that makes up the intersecting edge on OBB one.
  Vec3 obbOnePoint = obbOneCenter;
  for(uint i = 0; i < 3; ++i)
  {
    real sign = Math::GetSign(Dot(normal, obbOneAxes[i]));
    obbOnePoint = Vector3::MultiplyAdd(obbOnePoint, obbOneAxes[i], obbOneHalfExtents[i] * sign);
  }

  //Look for one of the points that makes up the intersecting edge on OBB two.
  Vec3 obbTwoPoint = obbTwoCenter;
  for(uint i = 0; i < 3; ++i)
  {
    real sign = -Math::GetSign(Dot(normal, obbTwoAxes[i]));
    obbTwoPoint = Vector3::MultiplyAdd(obbTwoPoint, obbTwoAxes[i], obbTwoHalfExtents[i] * sign);
  }

  //The index of one's edge axis is stored in the upper 16 bits of the number.
  uint oneEdgeIndex = (cObbAxes[axisCase - 1] >> 16) - 1;
  Vec3 obbOneEdgeAxis = obbOneAxes[oneEdgeIndex];

  //The index of two's edge axis is stored in the lower 16 bits of the number.
  uint twoEdgeIndex = (cObbAxes[axisCase - 1] & 0x0000FFFF) - 1;
  Vec3 obbTwoEdgeAxis = obbTwoAxes[twoEdgeIndex];

  ClosestPointsOfTwoLines(obbOnePoint, obbOneEdgeAxis, obbTwoPoint,
                          obbTwoEdgeAxis, &obbOnePoint, &obbTwoPoint);

  manifold.Normal = normal;
  manifold.PointAt(0).Points[0] = obbOnePoint;
  manifold.PointAt(0).Points[1] = obbTwoPoint;
  manifold.PointAt(0).Depth = Math::Abs(projection);
  manifold.PointCount = 1;
}

//Borrowed heavily from Bullet. :D
Type ObbObbContactGeneration(Vec3Param obbOneCenter,
                             Vec3Param obbOneHalfExtents,
                             const Vec3* obbOneAxes, Mat3Param obbOneBasis,
                             Vec3Param obbTwoCenter,
                             Vec3Param obbTwoHalfExtents,
                             const Vec3* obbTwoAxes, Mat3Param obbTwoBasis,
                             Vec3Ref normal, uint axisCase, real projection,
                             Manifold& manifold)
{
  ErrorIf(axisCase == 0, "Intersection - It seems the boxes are colliding, yet"\
                         " a valid axis of intersection was not found.");
  //----------------------------------------------------------------------------
  //If any of the 9 latter OBB axes were used (those that are generated from the
  //cross products of the OBBs' face normals), then it is an edge-edge case. Now
  //we find the closest points of the two edges.
  if(axisCase > 6)
  {
    ObbObbEdgeCase(obbOneCenter, obbOneHalfExtents, obbOneAxes, obbOneBasis,
                   obbTwoCenter, obbTwoHalfExtents, obbTwoAxes, obbTwoBasis,
                   normal, axisCase, projection, manifold);
    return EdgeEdge;
  }

  //----------------------------------------------------------------------------
  //Make sure the normal is pointing from One to Two
  {
    Vec3 oneToTwo = obbTwoCenter - obbOneCenter;
    real normalCheck = Dot(oneToTwo, normal);
    if(normalCheck < real(0.0))
    {
      normal *= real(-1.0);
    }
  }


  //----------------------------------------------------------------------------
  //Otherwise the axis of intersection is one of the OBBs' face normals and we
  //are dealing with a face-(point/edge/face) case
  uint aIndex;                          uint bIndex;
  const Vec3* aAxes[3];                 const Vec3* bAxes[3];
                                        const Mat3* bBasis;
  const Vec3* aCenter;                  const Vec3* bCenter;
  const Vec3* aHalfExtents;             const Vec3* bHalfExtents;

  //Normal to the face of box A, axis of intersection, in world space
  Vec3 aNormal = normal;
  if(axisCase < 4)
  {
    aIndex = 0;                         bIndex = 1;
    aAxes[0] = &(obbOneAxes[0]);        bAxes[0] = &(obbTwoAxes[0]);
    aAxes[1] = &(obbOneAxes[1]);        bAxes[1] = &(obbTwoAxes[1]);
    aAxes[2] = &(obbOneAxes[2]);        bAxes[2] = &(obbTwoAxes[2]);
                                        bBasis = &obbTwoBasis;
    aCenter = &obbOneCenter;            bCenter = &obbTwoCenter;
    aHalfExtents = &obbOneHalfExtents;  bHalfExtents = &obbTwoHalfExtents;
  }
  else
  {
    aIndex = 1;                         bIndex = 0;
    aAxes[0] = &(obbTwoAxes[0]);        bAxes[0] = &(obbOneAxes[0]);
    aAxes[1] = &(obbTwoAxes[1]);        bAxes[1] = &(obbOneAxes[1]);
    aAxes[2] = &(obbTwoAxes[2]);        bAxes[2] = &(obbOneAxes[2]);
                                        bBasis = &obbOneBasis;
    aCenter = &obbTwoCenter;            bCenter = &obbOneCenter;
    aHalfExtents = &obbTwoHalfExtents;  bHalfExtents = &obbOneHalfExtents;
    aNormal *= real(-1.0);
  }

  //----------------------------------------------------------------------------
  //The largest component of the bNormal corresponds to the normal for B's face.
  //The perpendicular components are stored as well.
  uint bAxis, bOne, bTwo;
  Vec3 bFaceCenter = *bCenter - *aCenter; //Used later (see end of scope)
  {
    //--------------------------------------------------------------------------
    //Axis of intersection in the reference frame of box B
    Vec3 bNormal = aNormal;
    Math::TransposedTransform(*bBasis, &bNormal);

    //--------------------------------------------------------------------------
    //Absolute value version of bNormal
    Vec3 absBNormal = Abs(bNormal);

    //Y-axis is larger than X-axis and Z-axis
    if((absBNormal[1] > absBNormal[0]) && (absBNormal[1] > absBNormal[2]))
    {
      bAxis = 1;
      bOne = 0;
      bTwo = 2;
    }
    //X-axis is larger than Y-axis and Z-axis
    else if(absBNormal[0] > absBNormal[2])
    {
      bAxis = 0;
      bOne = 1;
      bTwo = 2;
    }
    //Z-axis is larger than X-axis and Y-axis
    else
    {
      bAxis = 2;
      bOne = 0;
      bTwo = 1;
    }

    //--------------------------------------------------------------------------
    //Compute the center of B's face in A's face coordinates.
    if(Math::IsNegative(bNormal[bAxis]))
    {
      real halfExtents = (*bHalfExtents)[bAxis];
      const Vec3& longestAxis = *(bAxes[bAxis]);
      bFaceCenter = MultiplyAdd(bFaceCenter, longestAxis, halfExtents);
    }
    else
    {
      real halfExtents = (*bHalfExtents)[bAxis];
      const Vec3& longestAxis = *(bAxes[bAxis]);
      bFaceCenter = MultiplyAdd(bFaceCenter, longestAxis, -halfExtents);
    }
  }

  //----------------------------------------------------------------------------
  //Find the largest component of the aNormal. The perpendicular components are
  //stored as well.
  uint aAxis, aOne, aTwo;
  aAxis = (axisCase <= 3) ? (axisCase - 1) : (axisCase - 4);
  switch(aAxis)
  {
    case 0: //Normal is largest along A's local x-axis
    {
      aOne = 1; aTwo = 2;
    }
    break;

    case 1: //Normal is largest along A's local  y-axis
    {
      aOne = 0; aTwo = 2;
    }
    break;

    case 2: //Normal is largest along A's local  z-axis
    {
      aOne = 0; aTwo = 1;
    }
    break;
  }

  //----------------------------------------------------------------------------
  //The following code computes the points of B's face in A's face's reference
  //frame. This is akin to finding the four points on B's face and projecting
  //them onto the plane of A's face. The result is a 2D projection of the two
  //faces, which simplifies the clipping that needs to be done in order to find
  //the contact information for the two boxes.

  //Find the four points of B's face in A's reference frame.
  Vec2 bFacePoints[4];  //2D coordinates of B's face
  Vec2 bFaceXY;

  //Get the center of B's face in A's reference frame. These two lines of code
  //are projecting B's face onto the plane of A's face.
  bFaceXY.x = Dot(bFaceCenter, *(aAxes[aOne]));
  bFaceXY.y = Dot(bFaceCenter, *(aAxes[aTwo]));

  //Take the axes of B's face from world coordinates and bring them into A's
  //face's reference frame.
  Vec2 bFaceAxes[2];
  bFaceAxes[0].x = Dot(*(aAxes[aOne]), *(bAxes[bOne]));
  bFaceAxes[0].y = Dot(*(aAxes[aTwo]), *(bAxes[bOne]));
  bFaceAxes[1].x = Dot(*(aAxes[aOne]), *(bAxes[bTwo]));
  bFaceAxes[1].y = Dot(*(aAxes[aTwo]), *(bAxes[bTwo]));
  {
    //Now compute the four points of B's face in term's of A's face's reference
    //frame. These are just the axes of B's face scaled by B's face's extents.
    //The four combinations of these (--, -+, ++, +-) are used to generate all
    //four of the points on B's face
    Vec2 bFaceBasis[2] = { bFaceAxes[0] * (*bHalfExtents)[bOne],
                           bFaceAxes[1] * (*bHalfExtents)[bTwo] };

    // --
    bFacePoints[0].x = bFaceXY.x - bFaceBasis[0].x - bFaceBasis[1].x;
    bFacePoints[0].y = bFaceXY.y - bFaceBasis[0].y - bFaceBasis[1].y;

    // -+
    bFacePoints[1].x = bFaceXY.x - bFaceBasis[0].x + bFaceBasis[1].x;
    bFacePoints[1].y = bFaceXY.y - bFaceBasis[0].y + bFaceBasis[1].y;

    // ++
    bFacePoints[2].x = bFaceXY.x + bFaceBasis[0].x + bFaceBasis[1].x;
    bFacePoints[2].y = bFaceXY.y + bFaceBasis[0].y + bFaceBasis[1].y;

    // +-
    bFacePoints[3].x = bFaceXY.x + bFaceBasis[0].x - bFaceBasis[1].x;
    bFacePoints[3].y = bFaceXY.y + bFaceBasis[0].y - bFaceBasis[1].y;
  }

  //----------------------------------------------------------------------------
  //Get the half extents of A's face.
  Vec2 aFaceExtents((*aHalfExtents)[aOne], (*aHalfExtents)[aTwo]);

  //----------------------------------------------------------------------------
  //B's face is now in A's face's reference frame. The nice thing is that now
  //A's face can be represented by its half extents, with the center of A's face
  //being the origin and the axes of A's face being the x-axis and y-axis (or
  //z-axis, depending on how you view things). Now clip B's face against A's
  //face.
  Vec2 bFace[8];
  uint bFacePointCount = ClipQuadWithRectangle(aFaceExtents, bFacePoints,
                                               bFace);
  if(bFacePointCount == 0)
  {
    return None;
  }

  //----------------------------------------------------------------------------
  //Convert the intersection points into A's face's coordinates and compute the
  //contact point and depth for each point. Only keep the points that lie on the
  //inside of A's face (ie. those that are inside box A).
  {
    //Determinant of the matrix that brought B's points into A's reference
    //frame.
    real inverseDeterminant = real(1.0) / (bFaceAxes[0].x * bFaceAxes[1].y -
                                           bFaceAxes[1].x * bFaceAxes[0].y);

    //Invert the matrix to bring B's points back into world space but with the
    //center of A's face as the origin.
    bFaceAxes[0] *= inverseDeterminant;
    bFaceAxes[1] *= inverseDeterminant;
  }

  //----------------------------------------------------------------------------
  //Number of penetrating points found.
  uint pointCount = 0;
  Vec3 points[8];
  real depths[8];
  for(uint i = 0; i < bFacePointCount; ++i)
  {
    //Recreate the clipped points in the space where A's center is the origin by
    //representing them in terms of the clipped face's center and distance along
    //the face's x- and y-axes. This block of code is doing the "representing
    //them in terms of the face's center and distance along the face's x- and
    //y-axes" part.
    real xChange = bFace[i].x - bFaceXY.x;
    real yChange = bFace[i].y - bFaceXY.y;
    real xExtent =  bFaceAxes[1].y * xChange - bFaceAxes[1].x * yChange;
    real yExtent = -bFaceAxes[0].y * xChange + bFaceAxes[0].x * yChange;

    //This block of code is using the aforementioned point representation to
    //compute the points' positions in world space
    points[pointCount] = bFaceCenter;
    points[pointCount] = Math::MultiplyAdd(points[pointCount], *(bAxes[bOne]), xExtent);
    points[pointCount] = Math::MultiplyAdd(points[pointCount], *(bAxes[bTwo]), yExtent);

    //Dot the face normal with the point and compute the depth
    //Since A's center is the origin, A's face's distance from the origin is the
    //half extents of A along the normal
    const real faceDistance = (*aHalfExtents)[aAxis];
    depths[pointCount] = Geometry::SignedDistanceToPlane(points[pointCount],
                                                         aNormal, faceDistance);

    //Point is inside of the face, keep it
    if(Math::IsNegative(depths[pointCount]))
    {
      depths[pointCount] *= real(-1.0);
      bFace[pointCount] = bFace[i];
      ++pointCount;
    }
  }

  //No points were added
  if(pointCount == 0)
  {
    return None;
  }

  //----------------------------------------------------------------------------
  //Was not able to generate as many points as requested
  uint& maxPoints = manifold.PointCount;
  maxPoints = 4;
  if(maxPoints > pointCount)
  {
    maxPoints = pointCount;
  }

  //Not sure why this would happen...
  if(maxPoints == 0)
  {
    maxPoints = 1;
  }
  
  manifold.Normal = aNormal;

  if(axisCase > 3)
  {
    Negate(&(manifold.Normal));
  }

  //----------------------------------------------------------------------------
  if(pointCount <= maxPoints)
  {
    //The amount of contacts generated may not meet the amount of contacts
    //requested, therefore all of the generated contacts are returned.
    Vec3 pointInWorld;
    for(uint i = 0; i < pointCount; ++i)
    {
      manifold.PointAt(i).Depth = depths[i];

      //This point is the point on box B in world space
      pointInWorld = points[i] + *aCenter;
      manifold.PointAt(i).Points[bIndex] = pointInWorld;

      //This point is the point on box A in world space
      pointInWorld = Math::MultiplyAdd(pointInWorld, aNormal, depths[i]);
      manifold.PointAt(i).Points[aIndex] = pointInWorld;

      if(cGeometrySafeChecks)
      {
        Vec3 oneToTwo[2] = { obbTwoCenter - obbOneCenter,
                             manifold.PointAt(i).Points[1] -
                             manifold.PointAt(i).Points[0] };
        real dot = Dot(oneToTwo[0], oneToTwo[1]);
        ErrorIf(dot > cObbObbPointCheck, "Intersection - Points are not stored"\
                                         " with OBB one 1st and OBB two 2nd.");

        dot = Dot(oneToTwo[0], manifold.Normal);
        ErrorIf(dot < cObbObbNormalCheck, "Intersection - Normal is not "\
                                          "pointing from OBB one to OBB two.");
      }
    }
  }
  //More points were generated than were asked for, some of them must be culled.
  else
  {
    //Find the deepest point, it is always the first contact point.
    uint maxDepthIndex = 0;
    real maxDepth = depths[0];
    Vec2 bPoints[8];
    for(uint i = 0; i < pointCount; ++i)
    {
      bPoints[i] = bFace[i];
      if(depths[i] > maxDepth)
      {
        maxDepth = depths[i];
        maxDepthIndex = i;
      }
    }

    uint bestPoints[8];
    GetExtremePoints(bPoints, pointCount, maxPoints, maxDepthIndex, bestPoints);
    for(uint i = 0; i < maxPoints; ++i)
    {
      //The amount of contacts generated may not meet the amount of contacts
      //requested, therefore all of the generated contacts are returned.
      Vec3 pointInWorld;
      uint index = bestPoints[i];
      manifold.PointAt(i).Depth = depths[index];

      //This point is the point on box B in world space
      pointInWorld = points[index] + *aCenter;
      manifold.PointAt(i).Points[bIndex] = pointInWorld;

      //This point is the point on box A in world space
      pointInWorld = Math::MultiplyAdd(pointInWorld, aNormal, depths[index]);
      manifold.PointAt(i).Points[aIndex] = pointInWorld;

      if(cGeometrySafeChecks)
      {
        Vec3 oneToTwo[2] = { obbTwoCenter - obbOneCenter,
                             manifold.PointAt(i).Points[1] -
                             manifold.PointAt(i).Points[0] };
        real dot = Dot(oneToTwo[0], oneToTwo[1]);
        ErrorIf(dot > cObbObbPointCheck, "Intersection - Points are not stored"\
                                         " with OBB one 1st and OBB two 2nd.");

        dot = Dot(oneToTwo[0], manifold.Normal);
        ErrorIf(dot < cObbObbNormalCheck, "Intersection - Normal is not "\
                                          "pointing from OBB one to OBB two.");
      }
    }
  }

  if(axisCase < 4)
  {
    return FaceEdge;
  }
  else
  {
    return EdgeFace;
  }
}

///Intersect an oriented bounding box with an oriented bounding box.
Type ObbObb(Vec3Param obbOneCenter, Vec3Param obbOneHalfExtents,
            Mat3Param obbOneBasis, Vec3Param obbTwoCenter,
            Vec3Param obbTwoHalfExtents, Mat3Param obbTwoBasis,
            Manifold* manifold)
{
  const Vec3 obbOneAxes[3] = { obbOneBasis.GetBasis(0),
                               obbOneBasis.GetBasis(1),
                               obbOneBasis.GetBasis(2) };
  const Vec3 obbTwoAxes[3] = { obbTwoBasis.GetBasis(0),
                               obbTwoBasis.GetBasis(1),
                               obbTwoBasis.GetBasis(2) };

#ifdef VisualizeObbObbTestAxes
  //Axes visualization
  {
    const float headSize = 0.1f;
    ByteColor axes[3] = { Color::Red, Color::Green, Color::Blue };
    for(uint i = 0; i < 3; ++i)
    {
      Vec3 lineStart = obbOneCenter + obbOneAxes[i] * obbOneHalfExtents[i];
      Vec3 lineEnd = lineStart + obbOneAxes[i];

      Zero::gDebugDraw->Add(Zero::Debug::Line(lineStart, lineEnd).Color(axes[i])
                                                              .HeadSize(headSize));
    }
    for(uint i = 0; i < 3; ++i)
    {
      Vec3 lineStart = obbTwoCenter + obbTwoAxes[i] * obbTwoHalfExtents[i];
      Vec3 lineEnd = lineStart + obbTwoAxes[i];

      Zero::gDebugDraw->Add(Zero::Debug::Line(lineStart, lineEnd).Color(axes[i])
                                                              .HeadSize(headSize));
    }
#define MultiColoredLine(start, end, colorA, colorB)                                              \
    {                                                                                             \
      Vec3 halfway = ((start) + (end)) / real(2.0);                                               \
      Zero::gDebugDraw->Add(Zero::Debug::Line((start), halfway).Color((colorA)));                 \
      Zero::gDebugDraw->Add(Zero::Debug::Line(halfway, (end)).Color((colorB)).HeadSize(headSize));\
    }
  
    for(uint i = 0; i < 3; ++i)
    {
      for(uint j = 0; j < 3; ++j)
      {
        Vec3 axis = Cross(obbOneAxes[i], obbTwoAxes[j]);
        real length = AttemptNormalize(axis);
        if(length < real(0.000001))
        {
          continue;
        }
        //MultiColoredLine(Vec3::cZero, axis, axes[i], axes[j]);
        axis *= real(5.0);
        MultiColoredLine(obbOneCenter, obbOneCenter + axis, axes[i], axes[j]);
        MultiColoredLine(obbTwoCenter, obbTwoCenter + axis, axes[i], axes[j]);
      }
    }
  }
#endif

  bool existsParallel = false;

  //Compute the vector from One's center to Two's center and bring the vector 
  //into One's coordinate frame
  Vec3 T = obbTwoCenter - obbOneCenter;
  Math::TransposedTransform(obbOneBasis, &T);

  real boxProjections;
  real rOverlap;
  Vec3 normal;
  real min = Math::PositiveMax();
  uint axisCase = 0;

  real rot[3][3], absRot[3][3];

  //----------------------------------------------------------------------------
  //Test axes L = one0, L = one1, L = one2
  //----------------------------------------------------------------------------
  for(uint i = 0; i < 3; ++i)
  {
    //--------------------------------------------------------------------------
    //Create the matrix that takes points from Two's space to One's space
    //(World to One Matrix) * (Two to World Matrix) = (Two to One Matrix)
    for(uint j = 0; j < 3; ++j)
    {
      rot[i][j] = Dot(obbOneAxes[i], obbTwoAxes[j]);

      real absRotValue = Math::Abs(rot[i][j]);
      if(absRotValue > cObbParallelAxis)
      {
        existsParallel = true;
      }

      //Compute common subexpressions.* Add in an epsilon term to counteract
      //arithmetic errors when two edges are parallel and their cross product is
      //(near) zero.
      // *We take the absolute value because to get the box's full projection
      //  onto the axis we'd have to do "abs(x) + abs(y) + abs(z)". Half extents
      //  are always positive, so the matrix's values are the only place
      //  negatives can appear. By making them all positive, we circumvent
      //  having to do so for each axis test.
      absRot[i][j] = absRotValue + cObbEpsilon;
    }
    //--------------------------------------------------------------------------

    boxProjections  = obbOneHalfExtents[i];
    boxProjections += obbTwoHalfExtents[0] * absRot[i][0];
    boxProjections += obbTwoHalfExtents[1] * absRot[i][1];
    boxProjections += obbTwoHalfExtents[2] * absRot[i][2];
    rOverlap = boxProjections - Math::Abs(T[i]);

    if(rOverlap < cObbObbZero)
    {
      return None;
    }
    else if(rOverlap < min)
    {
      min = rOverlap;
      normal = obbOneAxes[i];
      axisCase = i + 1;
    }
  }

  //----------------------------------------------------------------------------
  //Test axes L = two0, L = two1, L = two2
  //----------------------------------------------------------------------------
  for(uint i = 0; i < 3; ++i)
  {
    boxProjections  = obbOneHalfExtents[0] * absRot[0][i];
    boxProjections += obbOneHalfExtents[1] * absRot[1][i];
    boxProjections += obbOneHalfExtents[2] * absRot[2][i];
    boxProjections += obbTwoHalfExtents[i];

    real test  = T[0] * rot[0][i];
         test += T[1] * rot[1][i];
         test += T[2] * rot[2][i];
    rOverlap = boxProjections - Math::Abs(test);

    if(rOverlap < cObbObbZero)
    {
      return None;
    }
    else if(rOverlap < min)
    {
      min = rOverlap;
      normal = obbTwoAxes[i];
      axisCase = i + 4;
    }
  }

  if(existsParallel == false)
  {
    #define HandleObbOverlap(nX, nY, nZ, aCase)                           \
    {                                                                     \
      rOverlap = boxProjections - Math::Abs(test);                        \
      if(rOverlap < cObbObbZero)                                          \
      {                                                                   \
        return None;                                                      \
      }                                                                   \
      real length = Math::Sqrt((nX) * (nX) + (nY) * (nY) + (nZ) * (nZ));  \
      if(length > cSimdEpsilon)                                           \
      {                                                                   \
        rOverlap /= length;                                               \
        if((rOverlap * cObbObbFudgeFactor) < min)                         \
        {                                                                 \
          min = rOverlap;                                                 \
          normal.x = (nX) / length;                                       \
          normal.y = (nY) / length;                                       \
          normal.z = (nZ) / length;                                       \
          axisCase = (aCase);                                             \
        }                                                                 \
      }                                                                   \
    }

    //--------------------------------------------------------------------------
    //Test axis L = one0 x two0
    //--------------------------------------------------------------------------
    boxProjections  = obbOneHalfExtents[1] * absRot[2][0];
    boxProjections += obbOneHalfExtents[2] * absRot[1][0];
    boxProjections += obbTwoHalfExtents[1] * absRot[0][2];
    boxProjections += obbTwoHalfExtents[2] * absRot[0][1];

    real test  = T[2] * rot[1][0];
         test -= T[1] * rot[2][0];
    HandleObbOverlap(real(0.0), -rot[2][0], rot[1][0], 7);

    //--------------------------------------------------------------------------
    //Test axis L = one0 x two1
    //--------------------------------------------------------------------------
    boxProjections  = obbOneHalfExtents[1] * absRot[2][1];
    boxProjections += obbOneHalfExtents[2] * absRot[1][1];
    boxProjections += obbTwoHalfExtents[0] * absRot[0][2];
    boxProjections += obbTwoHalfExtents[2] * absRot[0][0];

    test  = T[2] * rot[1][1];
    test -= T[1] * rot[2][1];
    HandleObbOverlap(real(0.0), -rot[2][1], rot[1][1], 8);

    //--------------------------------------------------------------------------
    //Test axis L = one0 x two2
    //--------------------------------------------------------------------------
    boxProjections  = obbOneHalfExtents[1] * absRot[2][2];
    boxProjections += obbOneHalfExtents[2] * absRot[1][2];
    boxProjections += obbTwoHalfExtents[0] * absRot[0][1];
    boxProjections += obbTwoHalfExtents[1] * absRot[0][0];

    test  = T[2] * rot[1][2];
    test -= T[1] * rot[2][2];
    HandleObbOverlap(real(0.0), -rot[2][2], rot[1][2], 9);

    //--------------------------------------------------------------------------
    //Test axis L = one1 x two0
    //--------------------------------------------------------------------------
    boxProjections  = obbOneHalfExtents[0] * absRot[2][0];
    boxProjections += obbOneHalfExtents[2] * absRot[0][0];
    boxProjections += obbTwoHalfExtents[1] * absRot[1][2];
    boxProjections += obbTwoHalfExtents[2] * absRot[1][1];

    test  = T[0] * rot[2][0];
    test -= T[2] * rot[0][0];
    HandleObbOverlap(rot[2][0], real(0.0), -rot[0][0], 10);

    //--------------------------------------------------------------------------
    //Test axis L = one1 x two1
    //--------------------------------------------------------------------------
    boxProjections  = obbOneHalfExtents[0] * absRot[2][1];
    boxProjections += obbOneHalfExtents[2] * absRot[0][1];
    boxProjections += obbTwoHalfExtents[0] * absRot[1][2];
    boxProjections += obbTwoHalfExtents[2] * absRot[1][0];

    test  = T[0] * rot[2][1];
    test -= T[2] * rot[0][1];
    HandleObbOverlap(rot[2][1], real(0.0), -rot[0][1], 11);

    //--------------------------------------------------------------------------
    //Test axis L = one1 x two2
    //--------------------------------------------------------------------------
    boxProjections  = obbOneHalfExtents[0] * absRot[2][2];
    boxProjections += obbOneHalfExtents[2] * absRot[0][2];
    boxProjections += obbTwoHalfExtents[0] * absRot[1][1];
    boxProjections += obbTwoHalfExtents[1] * absRot[1][0];

    test  = T[0] * rot[2][2];
    test -= T[2] * rot[0][2];
    HandleObbOverlap(rot[2][2], real(0.0), -rot[0][2], 12);

    //--------------------------------------------------------------------------
    //Test axis L = one2 x two0
    //--------------------------------------------------------------------------
    boxProjections  = obbOneHalfExtents[0] * absRot[1][0];
    boxProjections += obbOneHalfExtents[1] * absRot[0][0];
    boxProjections += obbTwoHalfExtents[1] * absRot[2][2];
    boxProjections += obbTwoHalfExtents[2] * absRot[2][1];

    test  = T[1] * rot[0][0];
    test -= T[0] * rot[1][0];
    HandleObbOverlap(-rot[1][0], rot[0][0], real(0.0), 13);

    //--------------------------------------------------------------------------
    //Test axis L = one2 x two1
    //--------------------------------------------------------------------------
    boxProjections  = obbOneHalfExtents[0] * absRot[1][1];
    boxProjections += obbOneHalfExtents[1] * absRot[0][1];
    boxProjections += obbTwoHalfExtents[0] * absRot[2][2];
    boxProjections += obbTwoHalfExtents[2] * absRot[2][0];

    test  = T[1] * rot[0][1];
    test -= T[0] * rot[1][1];
    HandleObbOverlap(-rot[1][1], rot[0][1], real(0.0), 14);

    //--------------------------------------------------------------------------
    //Test axis L = one2 x two2
    //--------------------------------------------------------------------------
    boxProjections  = obbOneHalfExtents[0] * absRot[1][2];
    boxProjections += obbOneHalfExtents[1] * absRot[0][2];
    boxProjections += obbTwoHalfExtents[0] * absRot[2][1];
    boxProjections += obbTwoHalfExtents[1] * absRot[2][0];

    test  = T[1] * rot[0][2];
    test -= T[0] * rot[1][2];
    HandleObbOverlap(-rot[1][2], rot[0][2], real(0.0), 15);

    #undef HandleOverlap
  }

  if(manifold != nullptr)
  {
    //Since no separating axis is found, the OBBs must be intersecting
    return ObbObbContactGeneration(obbOneCenter, obbOneHalfExtents,
                                   &obbOneAxes[0], obbOneBasis, obbTwoCenter,
                                   obbTwoHalfExtents, &obbTwoAxes[0],
                                   obbTwoBasis, normal, axisCase, min,
                                   *manifold);
  }
  return Other;
}

///Intersect an oriented bounding box with a plane.
Type ObbPlane(Vec3Param obbCenter, Vec3Param obbHalfExtents, Mat3Param obbBasis,
              Vec3Param planeNormal, real planeDistance,
              Manifold* manifold)
{
  //Early out with the simple test if no information was requested.
  if(manifold == nullptr)
  {
    //Compute the projection interval radius of box onto L(t) = b.c + t * p.n
    real radius  = obbHalfExtents[0] *
                   Math::Abs(Dot(planeNormal, obbBasis.GetBasis(0)));
         radius += obbHalfExtents[1] *
                   Math::Abs(Dot(planeNormal, obbBasis.GetBasis(1)));
         radius += obbHalfExtents[2] *
                   Math::Abs(Dot(planeNormal, obbBasis.GetBasis(2)));

    //Compute the signed distance of the box's center from the plane
    real signedDistance = Geometry::SignedDistanceToPlane(obbCenter,
                                                          planeNormal,
                                                          planeDistance);

    //Intersection occurs when the signed distance falls within the
    //[-radius, +radius] interval
    if(Math::Abs(signedDistance) <= radius)
    {
      return Other;
    }
    return None;
  }

  //Once the plane has been translated, take it into the box's local space by
  //applying the box's inverse rotation matrix to the plane's normal. The
  //distance will stay the same since it represents the plane's distance from
  //the origin along the plane's normal.
  Vec3 normal = Math::TransposedTransform(obbBasis, planeNormal);
  uint axisM;         //Longest axis of the normal
  {
    Vec3 absNormal = Math::Abs(normal);
    if((absNormal.x > absNormal.y) && (absNormal.x > absNormal.z))
    {
      //X-axis is the largest
      axisM = 0;
    }
    else if(absNormal.y > absNormal.z)
    {
      //Y-axis is the largest
      axisM = 1;
    }
    else
    {
      //Z-axis is the largest
      axisM = 2;
    }
  }

  //Now that the longest axis is found, determine which face of the box is the
  //most-negative in terms of the plane's normal.
  uint face;

  if(Math::IsNegative(normal[axisM]))
  {
    //Positive face is wanted
    face = Geometry::Box::cFaceAxisSign[axisM][0];
  }
  else
  {
    //Negative face is wanted
    face = Geometry::Box::cFaceAxisSign[axisM][1];
  }

  //With the most-negative face identified, test to see which of the face's
  //points are on the negative side of the plane.

  //Do an early out check to see if all of the box's face points are on the
  //negative side of the plane.
  real sign = Geometry::SignedDistanceToPlane(obbCenter, planeNormal,
                                              planeDistance);
  if(Math::IsNegative(sign))
  {
    //All of the box's face points are negative, add as many as we can and early
    //out.
    manifold->Normal = planeNormal;

    PointInfoArray points;
    points.Resize(4);

    Vec3 obbAxes[3] = { obbBasis.GetBasis(0) * obbHalfExtents[0],
                        obbBasis.GetBasis(1) * obbHalfExtents[1],
                        obbBasis.GetBasis(2) * obbHalfExtents[2] };

    //Generate the face points and their projections onto the plane's normal
    for(uint i = 0; i < 4; ++i)
    {
      uint basePoint = Geometry::Box::cPointFaceSign[face][i];
      const Vec3& signs = Geometry::Box::cPoint[basePoint];
      Vec3 point(obbCenter);
      point = Math::MultiplyAdd(point, obbAxes[0], signs[0]);
      point = Math::MultiplyAdd(point, obbAxes[1], signs[1]);
      point = Math::MultiplyAdd(point, obbAxes[2], signs[2]);
      points[i].Point = point;
      points[i].Projection = Geometry::SignedDistanceToPlane(point,
                                                             planeNormal,
                                                             planeDistance);
    }

    uint maxCount = manifold->PointCount;
    if(maxCount < 4)
    {
      //Can't add all the points so put the most negative ones first
      Zero::Sort(points.All());
    }
    for(uint i = 0; i < maxCount; ++i)
    {
      IntersectionPoint& manifoldPoint = manifold->PointAt(i);
      manifoldPoint.Points[0] = points[i].Point;
      manifoldPoint.Points[1] = points[i].Point;
      ClosestPointOnPlaneToPoint(planeNormal, planeDistance,
                                 &(manifoldPoint.Points[1]));
      manifoldPoint.Depth = -points[i].Projection;
    }
    return Face;
  }

  const Vec3 obbAxes[3] = { obbBasis.GetBasis(0) * obbHalfExtents[0],
                            obbBasis.GetBasis(1) * obbHalfExtents[1],
                            obbBasis.GetBasis(2) * obbHalfExtents[2] };

  //Use the pod array just in case sorting is needed.
  PointInfoArray points;
  points.Reserve(4);

  //Generate the face points and their projections onto the plane's normal
  for(uint i = 0; i < 4; ++i)
  {
    uint basePoint = Geometry::Box::cPointFaceSign[face][i];
    const Vec3& signs = Geometry::Box::cPoint[basePoint];
    Vec3 point(obbCenter);
    point = Math::MultiplyAdd(point, obbAxes[0], signs[0]);
    point = Math::MultiplyAdd(point, obbAxes[1], signs[1]);
    point = Math::MultiplyAdd(point, obbAxes[2], signs[2]);

    real depth = Geometry::SignedDistanceToPlane(point, planeNormal,
                                                 planeDistance);
    if(Math::IsNegative(depth))
    {
      points.PushBack(PointInfo(point, depth));
    }
  }

  if(points.Empty())
  {
    return Inside;
  }

  uint maxCount = manifold->PointCount;
  if(maxCount < points.Size())
  {
    //Can't add all the points so put the most negative ones first
    Zero::Sort(points.All());
  }
  for(uint i = 0; i < maxCount; ++i)
  {
    IntersectionPoint& manifoldPoint = manifold->PointAt(i);
    manifoldPoint.Points[0] = points[i].Point;
    manifoldPoint.Points[1] = points[i].Point;
    ClosestPointOnPlaneToPoint(planeNormal, planeDistance,
                               &(manifoldPoint.Points[1]));
    manifoldPoint.Depth = -points[i].Projection;
  }
  return Other;
}

///Intersect an oriented bounding box with a sphere.
Type ObbSphere(Vec3Param obbCenter, Vec3Param obbHalfExtents,
               Mat3Param obbBasis, Vec3Param sphereCenter, real sphereRadius,
               Manifold* manifold)
{
  Vec3 newSphereCenter = sphereCenter - obbCenter;
  Math::TransposedTransform(obbBasis, &newSphereCenter);
  Type result = AabbSphere(-obbHalfExtents, obbHalfExtents, newSphereCenter,
                           sphereRadius, manifold);

  if((manifold != nullptr) && (result != None))
  {
    Math::Transform(obbBasis, &(manifold->PointAt(0).Points[0]));
    manifold->PointAt(0).Points[0] += obbCenter;

    Math::Transform(obbBasis, &(manifold->PointAt(0).Points[1]));
    manifold->PointAt(0).Points[1] += obbCenter;

    Math::Transform(obbBasis, &(manifold->Normal));
  }
  return result;
}

void ObbTriangleEdgeCase(Vec3Param obbCenter, Vec3Param obbHalfExtents,
                         Mat3Param obbBasis,const Vec3 trianglePoints[3],
                         Vec3Ref normal, uint axisCase, real projection,
                         Manifold& manifold)
{
  Vec3 obbAxes[3] = { obbBasis.GetBasis(0),
                      obbBasis.GetBasis(1),
                      obbBasis.GetBasis(2) };

  //Normal is currently in the body space of the box, transform the normal back
  //into world space
  Math::Transform(obbBasis, &normal);

  //Make sure the normal is pointing from the box to the triangle
  {
    //Compute the triangle's barycenter
    Vec3 triangleBarycenter;
    Geometry::CalculateBarycenter(trianglePoints, 3, &triangleBarycenter);

    //Get the vector from the box's center to the triangle's barycenter
    Vec3 boxToTri = triangleBarycenter - obbCenter;

    real normalCheck = Dot(normal, boxToTri);
    if(normalCheck < real(0.0))
    {
      Negate(&normal);
    }
  }

  //Look for the point and the axis that make up the box's intersecting edge
  Vec3 boxEdgePoint = obbCenter;
  for(uint i = 0; i < 3; ++i)
  {
    real sign = Math::GetSign(Dot(normal, obbAxes[i]));
    boxEdgePoint = Math::MultiplyAdd(boxEdgePoint, obbAxes[i], obbHalfExtents[i] * sign);
  }

  //Get the point and the axis that make up the triangle's intersecting edge
  uint triIndex = (axisCase - 5) % 3;
  Vec3 triEdgePoint = trianglePoints[triIndex];
  Vec3 triEdgeAxis = trianglePoints[(triIndex + 1) % 3] - triEdgePoint;
  Normalize(triEdgeAxis);

  //Index of the box's edge axis is stored in the upper 16 bits of the number.
  uint boxEdgeIndex;// = (cObbAxes[axisCase - 1] >> 16) - 1;
  switch(axisCase)
  {
    case 5:
    case 6:
    case 7:
      boxEdgeIndex = 0;
    break;

    case 8:
    case 9:
    case 10:
      boxEdgeIndex = 1;
    break;

    case 11:
    case 12:
    case 13:
      boxEdgeIndex = 2;
    break;
  }
  Vec3 boxEdgeAxis = obbAxes[boxEdgeIndex];

  ClosestPointsOfTwoLines(boxEdgePoint, boxEdgeAxis, triEdgePoint, triEdgeAxis,
                          &boxEdgePoint, &triEdgePoint);

  manifold.Normal = normal;
  manifold.PointAt(0).Points[0] = boxEdgePoint;
  manifold.PointAt(0).Points[1] = triEdgePoint;
  manifold.PointAt(0).Depth = Math::Abs(projection);
  manifold.PointCount = 1;
}

Type ClipTriangleAgainstBox(Vec3Param obbCenter, Vec3Param obbHalfExtents,
                            Mat3Param obbBasis, const Vec3 trianglePoints[3],
                            uint axisCase, Vec3Param normal, Manifold& manifold)
{
  Vec3 obbAxes[3] = { obbBasis.GetBasis(0),
                      obbBasis.GetBasis(1),
                      obbBasis.GetBasis(2) };

  //----------------------------------------------------------------------------
  //Find the largest component of the normal. The perpendicular components are
  //stored as well.
  uint boxAxis = axisCase - 1;
  uint boxOne, boxTwo;
  switch(boxAxis)
  {
    case 0: //Normal is largest along box's local x-axis
    {
      boxOne = 1; boxTwo = 2;
    }
    break;

    case 1: //Normal is largest along box's local y-axis
    {
      boxOne = 0; boxTwo = 2;
    }
    break;

    case 2: //Normal is largest along box's local z-axis
    {
      boxOne = 0; boxTwo = 1;
    }
    break;
  }

  //----------------------------------------------------------------------------
  //Now that we have the face on the box that the triangle is said to be
  //intersecting with, clip the triangle.

  //Maximum amount of clipped points possible between a triangle and a square.
  Vec3 triPoints[7];
  uint triPointCount;
  {
    const Vec3& axisOne = obbAxes[boxOne];
    const Vec3& axisTwo = obbAxes[boxTwo];
    const real extentOne = obbHalfExtents[boxOne];
    const real extentTwo = obbHalfExtents[boxTwo];

    //Build clipping planes
    Vec4 clippingPlanes[4] = { Vec4(axisOne.x,  axisOne.y,  axisOne.z,
                                     extentOne + Dot(obbCenter, axisOne)),
                               Vec4(-axisOne.x, -axisOne.y, -axisOne.z,
                                     extentOne - Dot(obbCenter, axisOne)),
                               Vec4(axisTwo.x,  axisTwo.y,  axisTwo.z,
                                     extentTwo + Dot(obbCenter, axisTwo)),
                               Vec4(-axisTwo.x, -axisTwo.y, -axisTwo.z,
                                     extentTwo - Dot(obbCenter, axisTwo)) };
    //Clip the triangle against the box's face
    triPointCount = Geometry::ClipPolygonWithPlanes(clippingPlanes, 4,
                                                    trianglePoints, 3,
                                                    triPoints);
  }
  uint pointCount = 0;
  Vec3 points[7];
  real depths[7];
  {
    real faceDistance = Dot(obbCenter, normal) + obbHalfExtents[boxAxis];
    for(uint i = 0; i < triPointCount; ++i)
    {
      depths[pointCount] = Dot(triPoints[i], normal) - faceDistance;
      if(depths[pointCount] < real(0.0))
      {
        points[pointCount] = triPoints[i];
        depths[pointCount] *= real(-1.0);
        ++pointCount;
      }
    }
  }

  //No points were added
  if(pointCount == 0)
  {
    return None;
  }

  //----------------------------------------------------------------------------
  //Was not able to generate as many points as requested
  uint& maxPoints = manifold.PointCount;
  if(maxPoints > pointCount)
  {
    maxPoints = pointCount;
  }

  //Not sure why this would happen...
  if(maxPoints == 0)
  {
    maxPoints = 1;
  }

  manifold.Normal = normal;

  //----------------------------------------------------------------------------
  if(pointCount <= maxPoints)
  {
    //The amount of contacts generated may not meet the amount of contacts
    //requested, therefore all of the generated contacts are returned.
    Vec3 pointInWorld;
    for(uint i = 0; i < pointCount; ++i)
    {
      manifold.PointAt(i).Depth = depths[i];

      //This point is the point on the triangle
      pointInWorld = points[i];
      manifold.PointAt(i).Points[1] = pointInWorld;

      //This point is the point on the box in world space
      pointInWorld = Math::MultiplyAdd(pointInWorld, normal, depths[i]);
      manifold.PointAt(i).Points[0] = pointInWorld;
    }
  }
  //More points were generated than were asked for, some of them must be culled.
  else
  {
    //Create the triangle's orientation matrix as if the triangle's normal were
    //the z-axis. This is probably a good place to start optimizing.
    Vec3 triCenter;
    Geometry::CalculateBarycenter(trianglePoints, 3, &triCenter);
    Vec3 zAxis = Cross(trianglePoints[1] - trianglePoints[0],
                       trianglePoints[2] - trianglePoints[0]);
    Vec3 xAxis = trianglePoints[0] - triCenter;
    Vec3 yAxis = Cross(xAxis, zAxis);
    Mat3 triMatrix(xAxis[0], yAxis[0], zAxis[0],
                   xAxis[1], yAxis[1], zAxis[1],
                   xAxis[2], yAxis[2], zAxis[2]);
    triMatrix.Orthonormalize();

    //Bring the clipped triangle points back to the triangle's "model space" so
    //we can treat it as a 2D polygon for the purposes of finding the "best"
    //points that describe the contact region.
    Vec2 triPlanePoints[7];
    for(uint i = 0; i < pointCount; ++i)
    {
      Vec3 triSpacePoint = Math::TransposedTransform(triMatrix,
                                                     (points[i] - triCenter));
      triPlanePoints[i].Set(triSpacePoint.x, triSpacePoint.y);
    }

    //Holds the indices of the "best" points of the clipped triangle.
    uint bestPoints[7];
    {
      //Find the deepest point, it is always the first contact point.
      uint maxDepthIndex = 0;
      real maxDepth = depths[0];
      for(uint i = 0; i < pointCount; ++i)
      {
        if(depths[i] > maxDepth)
        {
          maxDepth = depths[i];
          maxDepthIndex = i;
        }
      }
      GetExtremePoints(triPlanePoints, pointCount, maxPoints, maxDepthIndex,
                       bestPoints);
    }

    Vec3 pointInWorld;
    for(uint i = 0; i < maxPoints; ++i)
    {
      uint index = bestPoints[i];
      manifold.PointAt(i).Depth = depths[index];

      //This point is the point on the triangle
      pointInWorld = points[index];
      manifold.PointAt(i).Points[1] = pointInWorld;

      //This point is the point on the box in world space
      pointInWorld = Math::MultiplyAdd(pointInWorld, normal, depths[index]);
      manifold.PointAt(i).Points[0] = pointInWorld;
    }
  }
  return FaceTri;
}

Type ClipBoxAgainstTriangle(Vec3Param obbCenter, Vec3Param obbHalfExtents,
                            Mat3Param obbBasis, const Vec3 trianglePoints[3],
                            uint axisCase, Vec3Param normal, Manifold& manifold)
{
  Vec3 obbAxes[3] = { obbBasis.GetBasis(0),
                      obbBasis.GetBasis(1),
                      obbBasis.GetBasis(2) };

  Vec3 triNormal = -normal;
  uint boxAxis, boxOne, boxTwo;

  //----------------------------------------------------------------------------
  //Axis of intersection in the reference frame of the box.
  Vec3 boxNormal = Math::TransposedTransform(obbBasis, normal);

  //----------------------------------------------------------------------------
  //Absolute value version of the normal in the box's frame of reference.
  Vec3 absBoxNormal = Abs(boxNormal);

  //Y-axis is larger than X-axis and Z-axis
  if((absBoxNormal.y > absBoxNormal.x) && (absBoxNormal.y > absBoxNormal.z))
  {
    boxAxis = 1;  boxOne = 0; boxTwo = 2;
  }
  //X-axis is larger than Y-axis and Z-axis
  else if(absBoxNormal.x > absBoxNormal.z)
  {
    boxAxis = 0;  boxOne = 1; boxTwo = 2;
  }
  //Z-axis is larger than X-axis and Y-axis
  else
  {
    boxAxis = 2;  boxOne = 0; boxTwo = 1;
  }

  Vec3 boxPoints[7];
  uint boxPointCount;

  //Get the four points on the face of the box that most faces the triangle and
  //clip them against the triangle
  {
    Vec3 facePoints[4];
    real sign = Math::GetSign(boxNormal[boxAxis]);
    Vec3 faceCenter = obbCenter + obbAxes[boxAxis] * (sign *
                                                      obbHalfExtents[boxAxis]);
    //++
    facePoints[0] = faceCenter + obbAxes[boxOne] * obbHalfExtents[boxOne]
                               + obbAxes[boxTwo] * obbHalfExtents[boxTwo];
    //+-
    facePoints[1] = faceCenter + obbAxes[boxOne] * obbHalfExtents[boxOne]
                               - obbAxes[boxTwo] * obbHalfExtents[boxTwo];
    //--
    facePoints[2] = faceCenter - obbAxes[boxOne] * obbHalfExtents[boxOne]
                               - obbAxes[boxTwo] * obbHalfExtents[boxTwo];
    //-+
    facePoints[3] = faceCenter - obbAxes[boxOne] * obbHalfExtents[boxOne]
                               + obbAxes[boxTwo] * obbHalfExtents[boxTwo];
    boxPointCount = Geometry::ClipPolygonWithPolygon(trianglePoints, 3,
                                                     facePoints, 4, boxPoints);
  }

  uint pointCount = 0;
  Vec3 points[7];
  real depths[7];
  {
    real planeDistance = Dot(trianglePoints[0], triNormal);
    for(uint i = 0; i < boxPointCount; ++i)
    {
      depths[pointCount] = Dot(boxPoints[i], triNormal) - planeDistance;
      if(depths[pointCount] < real(0.0))
      {
        points[pointCount] = boxPoints[i];
        depths[pointCount] *= real(-1.0);
        ++pointCount;
      }
    }
  }

  //No points were added
  if(pointCount == 0)
  {
    return None;
  }

  //----------------------------------------------------------------------------
  //Was not able to generate as many points as requested
  uint& maxPoints = manifold.PointCount;
  if(maxPoints > pointCount)
  {
    maxPoints = pointCount;
  }

  //Not sure why this would happen
  if(maxPoints == 0)
  {
    maxPoints = 1;
  }

  manifold.Normal = normal;

  //----------------------------------------------------------------------------
  if(pointCount <= maxPoints)
  {
    //The amount of contact points generated may not meet the amount of contact
    //points requested, therefore all of the generated contacts are returned.
    Vec3 pointInWorld;
    for(uint i = 0; i < pointCount; ++i)
    {
      manifold.PointAt(i).Depth = depths[i];

      //This point is the point on the box
      pointInWorld = points[i];
      manifold.PointAt(i).Points[0] = pointInWorld;

      //This point is the point on the triangle
      pointInWorld = Math::MultiplyAdd(pointInWorld, triNormal, depths[i]);
      manifold.PointAt(i).Points[1] = pointInWorld;
    }
  }
  //More points were generated than were asked for, some of them must be culled.
  else
  {
    //Create the box's orientation matrix as if the box's face normal was the
    //z-axis. This is probably a good place to start optimizing.
    real sign = Math::GetSign(boxNormal[boxAxis]);
    Vec3 faceCenter = obbCenter + obbAxes[boxAxis] * obbHalfExtents[boxAxis];

    Vec2 boxPlanePoints[7];
    for(uint i = 0; i < pointCount; ++i)
    {
      Vec3 boxSpacePoint = Math::TransposedTransform(obbBasis,
                                                     points[i] - faceCenter);
      boxPlanePoints[i].Set(boxSpacePoint[boxOne], boxSpacePoint[boxTwo]);
    }

    //Holds the indices of the "best" points of the clipped box face.
    uint bestPoints[7];
    {
      //Find the deepest point, it is always the first contact point.
      uint maxDepthIndex = 0;
      real maxDepth = depths[0];
      for(uint i = 0; i < pointCount; ++i)
      {
        if(depths[i] > maxDepth)
        {
          maxDepth = depths[i];
          maxDepthIndex = i;
        }
      }
      GetExtremePoints(boxPlanePoints, pointCount, maxPoints, maxDepthIndex,
                       bestPoints);
    }

    Vec3 pointInWorld;
    for(uint i = 0; i < maxPoints; ++i)
    {
      uint index = bestPoints[i];
      manifold.PointAt(i).Depth = depths[index];

      //This point is the point on the box
      pointInWorld = points[index];
      manifold.PointAt(i).Points[0] = pointInWorld;

      //This point is the point on the triangle
      pointInWorld = Math::MultiplyAdd(pointInWorld, triNormal, depths[index]);
      manifold.PointAt(i).Points[1] = pointInWorld;
    }
  }
  return FaceTri;
}

Type ObbTriangleContactGeneration(Vec3Param obbCenter, Vec3Param obbHalfExtents,
                                  Mat3Param obbBasis, const Vec3 triPoints[3],
                                  Vec3Ref normal, uint axisCase,
                                  real projection, Manifold& manifold)
{
  //----------------------------------------------------------------------------
  //If any of the 9 latter box-triangle axes were used (those that are generated
  //from the cross products of the box's face normals and the triangle's edge
  //normals), then it is an edge-edge case. Now we find the closest points of
  //the two edges.
  if(axisCase > 4)
  {
    ObbTriangleEdgeCase(obbCenter, obbHalfExtents, obbBasis, triPoints,
                        normal, axisCase, projection, manifold);
    return EdgeEdge;
  }

  //----------------------------------------------------------------------------
  //Make sure the normal is pointing from the box to the triangle.
  {
    Vec3 triCenter;
    Geometry::CalculateBarycenter(triPoints, 3, &triCenter);
    Vec3 boxToTri = triCenter - obbCenter;
    real oneToTwo = Dot(boxToTri, normal);
    if(oneToTwo < real(0.0))
    {
      normal *= real(-1.0);
    }
  }

  //----------------------------------------------------------------------------
  //If the axis case falls in the range of [1,3], then the triangle will be
  //clipped against one of the box's faces.
  if(axisCase != 4)
  {
    return ClipTriangleAgainstBox(obbCenter, obbHalfExtents, obbBasis,
                                  triPoints, axisCase, normal, manifold);
  }

  //----------------------------------------------------------------------------
  //If the axis case is 4, then the face on the box that is most oriented toward
  //the triangle will be used to clip the triangle.
  else
  {
    return ClipBoxAgainstTriangle(obbCenter, obbHalfExtents, obbBasis,
                                  triPoints, axisCase, normal, manifold);
  }

  /* To get the triangle all setup for clipping against the face, first the
     triangle's points need to be in the space of the clipping face. The end
     result of this has the clipping face lying on the xy-plane (or xz, however
     you wish to view it) and with it's center at the origin. To get the
     triangle in this space, we first need to translate the center of the
     clipping face to the origin. This is done by subtracting the position of
     the center of the clipping face from all 3 of the triangle's points.  Next
     we need to rotate the triangle in order to get the face lying on one of the
     aforementioned planes.  This is done by dotting the translated triangle
     points by the axes of the face (which are simply the other 2 axes of the
     box that aren't the face's normal).
  */
  //return None;
}

///Intersect an oriented bounding box with a triangle.
Type ObbTriangle(Vec3Param obbCenter, Vec3Param obbHalfExtents,
                 Mat3Param obbBasis, Vec3Param trianglePointA,
                 Vec3Param trianglePointB, Vec3Param trianglePointC,
                 Manifold* manifold)
{
  Vec3 trianglePoints[3] = { trianglePointA, trianglePointB, trianglePointC };
  return ObbTriangle(obbCenter, obbHalfExtents, obbBasis, trianglePoints,
                     manifold);
}

///Intersect an oriented bounding box with a triangle. Different parameters.
Type ObbTriangle(Vec3Param obbCenter, Vec3Param obbHalfExtents,
                 Mat3Param obbBasis, const Vec3 trianglePoints[3],
                 Manifold* manifold)
{
  //Take the triangle into the box's space
  Vec3 triPoint[3] = { trianglePoints[0] - obbCenter,
                       trianglePoints[1] - obbCenter,
                       trianglePoints[2] - obbCenter };
  Math::TransposedTransform(obbBasis, &(triPoint[0]));
  Math::TransposedTransform(obbBasis, &(triPoint[1]));
  Math::TransposedTransform(obbBasis, &(triPoint[2]));

  Vec3 triEdge[3] = { triPoint[1] - triPoint[0],
                      triPoint[2] - triPoint[1],
                      triPoint[0] - triPoint[2] };

  Vec3 normal;                            //Axis of minimum overlap
  real minOverlap = Math::PositiveMax();  //Overall minimum overlap
  uint axisCase = 0;                      //ID of axis of minimum overlap
  real triProj[2];                        //Persistent variables

  {
    real boxProj;
    //--------------------------------------------------------------------------
    // Category 3: Test cross product axes (9 axes, cases 5 - 13)
    //--------------------------------------------------------------------------
    #define HandleObbTriOverlap(nX, nY, nZ, aCase)                            \
    {                                                                         \
      MinMaxInPlace(triProj[0], triProj[1]);                                  \
      triProj[0] = boxProj - triProj[0];                                      \
      triProj[1] += boxProj;                                                  \
      if((triProj[0] < cObbTriangleZero) || (triProj[1] < cObbTriangleZero))  \
      {                                                                       \
        return None;                                                          \
      }                                                                       \
      real curOverlap = Math::Min(triProj[0], triProj[1]);                    \
      {                                                                       \
        real length = Math::Sqrt((nX) * (nX) + (nY) * (nY) + (nZ) * (nZ));    \
        if(length > cSimdEpsilon)                                             \
        {                                                                     \
          curOverlap /= length;                                               \
          if(curOverlap < minOverlap)                                         \
          {                                                                   \
            minOverlap = curOverlap;                                          \
            normal.x = (nX) / length;                                         \
            normal.y = (nY) / length;                                         \
            normal.z = (nZ) / length;                                         \
            axisCase = (aCase);                                               \
          }                                                                   \
        }                                                                     \
      }                                                                       \
    }

    //--------------------------------------------------------------------------
    //Test axis L = box0 x tri0 = (0, -edge[0].z, edge[0].y)
    //--------------------------------------------------------------------------
    triProj[0] = triPoint[0].z * triPoint[1].y - triPoint[0].y * triPoint[1].z;
    triProj[1] = triPoint[2].z * triEdge[0].y - triPoint[2].y * triEdge[0].z;
    boxProj = obbHalfExtents[1] * Math::Abs(triEdge[0].z) +
              obbHalfExtents[2] * Math::Abs(triEdge[0].y);
    HandleObbTriOverlap(real(0.0), -triEdge[0].z, triEdge[0].y, 5);

    //--------------------------------------------------------------------------
    //Test axis L = box0 x tri1 = (0, -edge[1].z, edge[1].y)
    //--------------------------------------------------------------------------
    triProj[0] = triPoint[1].z * triPoint[2].y - triPoint[1].y * triPoint[2].z;
    triProj[1] = triPoint[0].z * triEdge[1].y - triPoint[0].y * triEdge[1].z;
    boxProj = obbHalfExtents[1] * Math::Abs(triEdge[1].z) +
              obbHalfExtents[2] * Math::Abs(triEdge[1].y);
    HandleObbTriOverlap(real(0.0), -triEdge[1].z, triEdge[1].y, 6);

    //--------------------------------------------------------------------------
    //Test axis L = box0 x tri2 = (0, -edge[2].z, edge[2].y)
    //--------------------------------------------------------------------------
    triProj[0] = triPoint[0].y * triPoint[2].z - triPoint[0].z * triPoint[2].y;
    triProj[1] = triPoint[1].z * triEdge[2].y - triPoint[1].y * triEdge[2].z;
    boxProj = obbHalfExtents[1] * Math::Abs(triEdge[2].z) +
              obbHalfExtents[2] * Math::Abs(triEdge[2].y);
    HandleObbTriOverlap(real(0.0), -triEdge[2].z, triEdge[2].y, 7);

    //--------------------------------------------------------------------------
    //Test axis L = box1 x tri0 = (edge[0].z, 0, -edge[0].x)
    //--------------------------------------------------------------------------
    triProj[0] = triPoint[0].x * triPoint[1].z - triPoint[0].z * triPoint[1].x;
    triProj[1] = triPoint[2].x * triEdge[0].z - triPoint[2].z * triEdge[0].x;
    boxProj = obbHalfExtents[0] * Math::Abs(triEdge[0].z) +
              obbHalfExtents[2] * Math::Abs(triEdge[0].x);
    HandleObbTriOverlap(triEdge[0].z, real(0.0), -triEdge[0].x, 8);

    //--------------------------------------------------------------------------
    //Test axis L = box1 x tri1 = (edge[1].z, 0, -edge[1].x)
    //--------------------------------------------------------------------------
    triProj[0] = triPoint[1].x * triPoint[2].z - triPoint[1].z * triPoint[2].x;
    triProj[1] = triPoint[0].x * triEdge[1].z - triPoint[0].z * triEdge[1].x;
    boxProj = obbHalfExtents[0] * Math::Abs(triEdge[1].z) +
              obbHalfExtents[2] * Math::Abs(triEdge[1].x);
    HandleObbTriOverlap(triEdge[1].z, real(0.0), -triEdge[1].x, 9);

    //--------------------------------------------------------------------------
    //Test axis L = box1 x tri2 = (edge[2].z, 0, -edge[2].x)
    //--------------------------------------------------------------------------
    triProj[0] = triPoint[0].z * triPoint[2].x - triPoint[0].x * triPoint[2].z;
    triProj[1] = triPoint[1].x * triEdge[2].z - triPoint[1].z * triEdge[2].x;
    boxProj = obbHalfExtents[0] * Math::Abs(triEdge[2].z) +
              obbHalfExtents[2] * Math::Abs(triEdge[2].x);
    HandleObbTriOverlap(triEdge[2].z, real(0.0), -triEdge[2].x, 10);

    //--------------------------------------------------------------------------
    //Test axis L = box2 x tri0 = (-edge[0].y, edge[0].x, 0)
    //--------------------------------------------------------------------------
    triProj[0] = triPoint[0].y * triPoint[1].x - triPoint[0].x * triPoint[1].y;
    triProj[1] = triPoint[2].y * triEdge[0].x - triPoint[2].x * triEdge[0].y;
    boxProj = obbHalfExtents[0] * Math::Abs(triEdge[0].y) +
              obbHalfExtents[1] * Math::Abs(triEdge[0].x);
    HandleObbTriOverlap(-triEdge[0].y, triEdge[0].x, real(0.0), 11);

    //--------------------------------------------------------------------------
    //Test axis L = box2 x tri1 = (-edge[1].y, edge[1].x, 0)
    //--------------------------------------------------------------------------
    triProj[0] = triPoint[1].y * triPoint[2].x - triPoint[1].x * triPoint[2].y;
    triProj[1] = triPoint[0].y * triEdge[1].x - triPoint[0].x * triEdge[1].y;
    boxProj = obbHalfExtents[0] * Math::Abs(triEdge[1].y) +
              obbHalfExtents[1] * Math::Abs(triEdge[1].x);
    HandleObbTriOverlap(-triEdge[1].y, triEdge[1].x, real(0.0), 12);

    //--------------------------------------------------------------------------
    //Test axis L = box2 x tri2 = (-edge[2].y, edge[2].x, 0)
    //--------------------------------------------------------------------------
    triProj[0] = triPoint[0].x * triPoint[2].y - triPoint[0].y * triPoint[2].x;
    triProj[1] = triPoint[1].y * triEdge[2].x - triPoint[1].x * triEdge[2].y;
    boxProj = obbHalfExtents[0] * Math::Abs(triEdge[2].y) +
              obbHalfExtents[1] * Math::Abs(triEdge[2].x);
    HandleObbTriOverlap(-triEdge[2].y, triEdge[2].x, real(0.0), 13);

    #undef HandleObbTriOverlap
  }

  //----------------------------------------------------------------------------
  // Category 1: Test AABB against triangle's AABB (3 axes, cases 1 - 3)
  //----------------------------------------------------------------------------
#define Min3(a, b, c) Math::Min(Math::Min((a), (b)), (c))
#define Max3(a, b, c) Math::Max(Math::Max((a), (b)), (c))
  for(uint i = 0; i < 3; ++i)
  {
    //Compute the difference between box's max/tri's min and box's min/tri's max
    triProj[0] = -Min3(triPoint[0][i], triPoint[1][i], triPoint[2][i]);
    triProj[0] += obbHalfExtents[i];

    triProj[1] = Max3(triPoint[0][i], triPoint[1][i], triPoint[2][i]);
    triProj[1] += obbHalfExtents[i];

    if((triProj[0] < cObbTriangleZero) || (triProj[1] < cObbTriangleZero))
    {
      return None;
    }
    else
    {
      //Find the smaller of the two differences, this is the minimum overlap
      //amount along this axis
      triProj[0] = Math::Min(triProj[0], triProj[1]);
      if((triProj[0] * cObbTriFudgeFactor) <= minOverlap)
      {
        axisCase = i + 1;
        normal = obbBasis.GetBasis(i);
        minOverlap = triProj[0];
      }
    }
  }
#undef Max3
#undef Min3

  //----------------------------------------------------------------------------
  // Category 2: Test AABB against triangle's normal (1 axis, case 4)
  //----------------------------------------------------------------------------
  //This code is the simple early-out case for AABB vs plane. The only reason
  //I'm not calling that code is because I need the projection difference in
  //case this axis has the minimum overlap.
  {
    //Compute the normal of the plane that the triangle lives on.
    Vec3 planeNormal = Cross(triEdge[0], triEdge[1]);
    float normalLength = planeNormal.Length();
    if (normalLength == 0.0f)
    {
      return None;
    }
    //Normalize the plane's normal
    planeNormal /= normalLength;\


    Normalize(planeNormal);
    real planeDistance = Dot(planeNormal, triPoint[0]);

    //Compute the projection interval radius of box onto L(t) = b.c + t * p.n
    real radius = obbHalfExtents.x * Math::Abs(planeNormal.x) +
                  obbHalfExtents.y * Math::Abs(planeNormal.y) +
                  obbHalfExtents.z * Math::Abs(planeNormal.z);

    //Compute the signed distance of the box's center from the plane. Since the
    //box's center is at the origin, (n * p) - d will end up as just -d
    real distance = radius - Math::Abs(planeDistance);
    if(distance < cObbTriangleZero)
    {
      return None;
    }
    else if(distance < minOverlap)
    {
      //Bring the normal back into world space
      normal = Math::Transform(obbBasis, planeNormal);
      minOverlap = distance;
      axisCase = 4;
    }
  }

  if(manifold != nullptr)
  {
    return ObbTriangleContactGeneration(obbCenter, obbHalfExtents, obbBasis,
                                        trianglePoints, normal, axisCase,
                                        minOverlap, *manifold);
  }
  return Other;
}

}// namespace Intersection
