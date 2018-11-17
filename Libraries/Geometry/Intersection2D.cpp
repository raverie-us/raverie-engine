///////////////////////////////////////////////////////////////////////////////
///
/// \file Intersection2D.cpp
/// All of the functions that test for intersection in 2 dimensions.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Geometry/Intersection.hpp"
#include "Geometry/Geometry.hpp"

namespace Intersection
{

Manifold2D::Manifold2D(uint pointCount)
  : PointCount(pointCount)
{
  //
}

IntersectionPoint2D& Manifold2D::PointAt(uint index)
{
  return Points[index];
}


//--------------------------------------------------------- File-Scope Functions

///Find the closest point on a line segment to the given point.
Type ClosestPointOnSegmentToPoint(Vec2Param segmentPointA,
  Vec2Param segmentPointB,
  Vec2Ptr point)
{
  ErrorIf(point == nullptr, "Intersection - Null pointer passed, this function"\
    " needs a valid pointer.");

  Vec2 lineSegment = segmentPointB - segmentPointA;

  //Project c onto ab, computing parameterized position d(t) = a + t * (b - a)
  real t = Dot(*point - segmentPointA, lineSegment);
  if(t <= real(0.0))
  {
    //c projects outside the [a,b] interval, on the a side; clamp to a
    *point = segmentPointA;
    return Outside;
  }
  else
  {
    real denom = Dot(lineSegment, lineSegment);
    if(t >= denom)
    {
      //c projects outside the [a,b] interval, on the b side; clamp to b
      *point = segmentPointB;
      return Outside;
    }
    else
    {
      //c projects inside the [a,b] interval; must do deferred divide now
      t /= denom;

      //This variable is now being treated as the closest point
      lineSegment *= t;
      lineSegment += segmentPointA;
      *point = lineSegment;
      return Inside;
    }
  }
}

//Find the intersection (the shared values) between the two intervals and return
//the difference between the minimum and maximum value of the resulting 
//interval. A negative result indicates that the two intervals are not 
//intersecting.
real IntersectIntervals(Vec2Param intervalA, Vec2Param intervalB)
{
  const uint min = 0;
  const uint max = 1;
  real minVal = Math::Max(intervalA[min], intervalB[min]);
  real maxVal = Math::Min(intervalA[max], intervalB[max]);
  return maxVal - minVal;
}

//Project a box onto the given axis, storing its (absolute-valued) minimum and
//maximum projections in the return vector.
Vec2 ProjectBoxOntoAxis(Vec2Param boxCenter, Vec2Param boxHalfExtents, 
                        const Vec2* boxAxes, Vec2Param axis)
{
  //Compute all 4 points of the box
  Vec2 boxPoints[4] = { boxCenter + (boxAxes[0] * boxHalfExtents[0])
                                  + (boxAxes[1] * boxHalfExtents[1]),
                        boxCenter - (boxAxes[0] * boxHalfExtents[0])
                                  + (boxAxes[1] * boxHalfExtents[1]),
                        boxCenter - (boxAxes[0] * boxHalfExtents[0])
                                  - (boxAxes[1] * boxHalfExtents[1]),
                        boxCenter + (boxAxes[0] * boxHalfExtents[0])
                                  - (boxAxes[1] * boxHalfExtents[1]) };

  //Resulting minimum and maximum projections onto the axis
  real minProj =  Math::PositiveMax();
  real maxProj = -Math::PositiveMax();

  //Project all 4 points onto the axes, saving off the results
  for(uint i = 0; i < 4; ++i)
  {
    real projection = Math::Dot(boxPoints[i], axis);
    if(projection < minProj)
    {
      minProj = projection;
    }
    if(projection > maxProj)
    {
      maxProj = projection;
    }
  }
  return Vec2(minProj, maxProj);
}

//Project a polygon onto the given axis, storing its (absolute-valued) minimum 
//and maximum projections in the return vector.
Vec2 ProjectPolygonOntoAxis(const Vec2* polygonPoints, uint polygonPointCount,
                            Vec2 axis)
{
  //Stores the minimum and maximum projections, with minimum in the x-component
  //and maximum in y-component
  real minProj = Math::PositiveMax();
  real maxProj = -Math::PositiveMax();
  for(uint i = 0; i < polygonPointCount; ++i)
  {
    real proj = Math::Abs(Dot(polygonPoints[i], axis));
    if(proj < minProj)
    {
      minProj = proj;
    }
    if(proj > maxProj)
    {
      maxProj = proj;
    }
  }

  return Vec2(minProj, maxProj);
}


//---------------------------------------------------- Header-Declared Functions

///Intersect a rotated box with a rotated box.
Type BoxBox(Vec2Param boxCenterA, Vec2Param boxHalfExtentsA, 
            const Vec2* boxAxesA, Vec2Param boxCenterB, 
            Vec2Param boxHalfExtentsB, const Vec2* boxAxesB, 
            Manifold2D* manifold)
{
  real minOverlap = Math::PositiveMax();
  Vec2 minAxis;
  uint axisIndex = uint(-1);

  //----------------------------------------------------------------------------
  //Project the boxes onto the axes of box A
  for(uint i = 0; i < 2; ++i)
  {
    Vec2 axis = boxAxesA[i];
    Vec2 intervalA = ProjectBoxOntoAxis(boxCenterA, boxHalfExtentsA, boxAxesA, 
                                        axis);
    Vec2 intervalB = ProjectBoxOntoAxis(boxCenterB, boxHalfExtentsB, boxAxesB,  
                                        axis);
    real overlapAmount = IntersectIntervals(intervalA, intervalB);

    //Overlap amount is negative, interval is invalid, no intersection
    if(overlapAmount < real(0.0))
    {
      return None;
    }
    if(overlapAmount < minOverlap)
    {
      minOverlap = overlapAmount;
      minAxis = axis;
      axisIndex = i;
    }
  }

  //----------------------------------------------------------------------------
  //Project the boxes onto the axes of box B
  for(uint i = 0; i < 2; ++i)
  {
    Vec2 axis = boxAxesB[i];
    Vec2 intervalA = ProjectBoxOntoAxis(boxCenterA, boxHalfExtentsA, boxAxesA, 
                                        axis);
    Vec2 intervalB = ProjectBoxOntoAxis(boxCenterB, boxHalfExtentsB, boxAxesB,  
                                        axis);
    real overlapAmount = IntersectIntervals(intervalA, intervalB);

    //Overlap amount is negative, interval is invalid, no intersection
    if(overlapAmount < real(0.0))
    {
      return None;
    }
    if(overlapAmount < minOverlap)
    {
      minOverlap = overlapAmount;
      minAxis = axis;
      axisIndex = i + 2;
    }
  }

  //----------------------------------------------------------------------------
  ErrorIf(axisIndex == uint(-1), "Intersection - Axis index is invalid, "\
                                 "impossible for this to break.");

  //No information needed & boxes are colliding
  if(manifold == nullptr)
  {
    return Point;
  }

  //Make sure that the normal is pointing from box A to box B
  Vec2 aToB = boxCenterB - boxCenterA;
  if(Dot(minAxis, aToB) < real(0.0))
  {
    minAxis *= real(-1.0);
  }

  /*
       4-----------1
       |           |
       |           |
       |           |
       |           |
       |           |
       3-----------2
  */
  const real pointSigns[4][2] = { { real(1.0), real(1.0) },
                                  { real(1.0), real(-1.0) }, 
                                  { real(-1.0), real(-1.0) }, 
                                  { real(-1.0), real(1.0) } };

  Vec2 pointOnA, pointOnB;

  //Edge on box A, intersectionPoint/edge on box B
  if(axisIndex < 2)
  {
    //Find the intersectionPoint on box B that is furthest in the direction of box A
    real testValue = -Math::PositiveMax();
    for(uint i = 0; i < 4; ++i)
    {
      Vec2 tempPoint = boxAxesB[0] * pointSigns[i][0] * boxHalfExtentsB[0] +
                       boxAxesB[1] * pointSigns[i][1] * boxHalfExtentsB[1];
      real tempValue = Dot(-minAxis, tempPoint);
      if(tempValue > testValue)
      {
        pointOnB = tempPoint;
        testValue = tempValue;
      }
    }

    //Find the edge of box A that's closest to box B
    //First find the side on box A that's facing box B
    Vec2 boxPointsA[2] = { boxCenterA, boxCenterA };
    real sign;
    if(Dot(boxAxesA[axisIndex], aToB) < real(0.0))
    {
      sign = real(-1.0);
    }
    else
    {
      sign = real(1.0);
    }
    uint altIndex = (axisIndex + 1) % 2;

    //All this is just to get points on the edge on box A closest to box B
    boxPointsA[0] += (sign * boxAxesA[axisIndex] * boxHalfExtentsA[axisIndex]) +
                     boxAxesA[altIndex] * boxHalfExtentsA[altIndex];

    boxPointsA[1] += (sign * boxAxesA[axisIndex] * boxHalfExtentsA[axisIndex]) -
                     boxAxesA[altIndex] * boxHalfExtentsA[altIndex];

    //Find the closest intersectionPoint on A's edge to B's intersectionPoint
    pointOnA = pointOnB;
    ClosestPointOnSegmentToPoint(boxPointsA[0], boxPointsA[1], &pointOnA);
  }
  //Edge on box B, intersectionPoint/edge on box A
  else
  {
    //Find the intersectionPoint on box A that is furthest in the direction of box B
    real testValue = -Math::PositiveMax();
    for(uint i = 0; i < 4; ++i)
    {
      Vec2 tempPoint = boxAxesA[0] * pointSigns[i][0] * boxHalfExtentsA[0] +
                       boxAxesA[1] * pointSigns[i][1] * boxHalfExtentsA[1];
      real tempValue = Dot(minAxis, tempPoint);
      if(tempValue > testValue)
      {
        pointOnA = tempPoint;
        testValue = tempValue;
      }
    }

    //Find the edge of box B that's closest to box A
    //First find the side on box B that's facing box A
    Vec2 boxPointsB[2] = { boxCenterB, boxCenterB };
    real sign;
    axisIndex -= 2;
    if(Dot(boxAxesB[axisIndex], aToB) > real(0.0))
    {
      sign = real(-1.0);
    }
    else
    {
      sign = real(1.0);
    }
    uint altIndex = (axisIndex + 1) % 2;

    //All this is just to get points on the edge on box A closest to box B
    boxPointsB[0] += (sign * boxAxesB[axisIndex] * boxHalfExtentsB[axisIndex]) +
                     boxAxesB[altIndex] * boxHalfExtentsB[altIndex];

    boxPointsB[1] += (sign * boxAxesB[axisIndex] * boxHalfExtentsB[axisIndex]) -
                     boxAxesB[altIndex] * boxHalfExtentsB[altIndex];

    //Find the closest intersectionPoint on B's edge to A's intersectionPoint
    pointOnB = pointOnA;
    ClosestPointOnSegmentToPoint(boxPointsB[0], boxPointsB[1], &pointOnB);
  }

  manifold->PointAt(0).Points[0] = pointOnA;
  manifold->PointAt(0).Points[1] = pointOnB;
  manifold->PointAt(0).Depth = minOverlap;
  manifold->Normal = minAxis;
  manifold->PointCount = 1;

  return Point;
}

///Intersect a rotated box with a circle.
Type BoxCircle(Vec2Param boxCenter, Vec2Param boxHalfExtents, 
               const Vec2* boxAxes, Vec2Param circleCenter, 
               real circleRadius, Manifold2D* manifold)
{
  const Vec2 boxPoints[4] = { boxCenter + boxAxes[0] * boxHalfExtents[0]
                                        + boxAxes[1] * boxHalfExtents[1],
                              boxCenter + boxAxes[0] * boxHalfExtents[0]
                                        - boxAxes[1] * boxHalfExtents[1],
                              boxCenter - boxAxes[0] * boxHalfExtents[0]
                                        - boxAxes[1] * boxHalfExtents[1],
                              boxCenter - boxAxes[0] * boxHalfExtents[0]
                                        + boxAxes[1] * boxHalfExtents[1] };

  //Find the closest intersectionPoint on the box to the circle
  Vec2 closestPoint;
  real shortestLength = Math::PositiveMax();
  for(uint i = 0; i < 4; ++i)
  {
    Vec2 tempPoint = circleCenter;
    ClosestPointOnSegmentToPoint(boxPoints[i], boxPoints[(i + 1) % 4], 
                                 &tempPoint);

    real tempLength = Length(circleCenter - tempPoint);
    if(tempLength < shortestLength)
    {
      closestPoint = tempPoint;
      shortestLength = tempLength;
    }
  }

  //Circle isn't touching the box, no collision
  if(shortestLength > circleRadius)
  {
    return None;
  }

  if(manifold != nullptr)
  {
    Vec2 normal = Normalized(circleCenter - closestPoint);

    Vec2 circlePoint = circleCenter - (normal * circleRadius);

    Vec2 boxPoint = closestPoint;

    real overlapAmount = Length(circlePoint - boxPoint);

    manifold->PointAt(0).Points[0] = boxPoint;
    manifold->PointAt(0).Points[1] = circlePoint;
    manifold->PointAt(0).Depth = overlapAmount;
    manifold->Normal = normal;
    manifold->PointCount = 1;
  }

  return Point;
}

///Intersect a circle with a circle
Type CircleCircle(Vec2Param circleCenterA, real circleRadiusA,
                  Vec2Param circleCenterB, real circleRadiusB, 
                  Manifold2D* manifold)
{
  //Represents the distance that the centers of the circles can be before the
  //circles begin to overlap
  real radiiSum = circleRadiusA + circleRadiusB;

  //Vector and distance between the centers of the two circles
  Vec2 aToB = circleCenterB - circleCenterA;
  real centerDistance = Length(aToB);

  //Distance between the centers of the circles is less than the sum of their
  //radii, must be intersecting
  if(centerDistance < radiiSum)
  {
    if(manifold != nullptr)
    {
      //Unit length direction vector pointing from circle A to circle B
      Vec2 normal = aToB / centerDistance;

      //Point on circle A deepest into circle B
      Vec2 pointOnA = circleCenterA + (normal * circleRadiusA);

      //Point on circle B deepest into circle A
      Vec2 pointOnB = circleCenterB - (normal * circleRadiusB);

      //Maximum amount that the two circles are overlapping
      real overlapAmount = radiiSum - centerDistance;

      manifold->PointAt(0).Points[0] = pointOnA;
      manifold->PointAt(0).Points[1] = pointOnB;
      manifold->PointAt(0).Depth = overlapAmount;
      manifold->Normal = normal;
      manifold->PointCount = 1;
    }

    //Only one intersectionPoint is furthest inside of each circle
    return Point;
  }

  //Circles are not intersecting
  return None;
}

///Intersect a circle with an n-sided convex polygon. Does not handle the case
///where the circle is inside the polygon.
Type CircleConvexPolygon(Vec2Param circleCenter, real circleRadius, 
                         const Vec2* convexPolygonPoints, 
                         uint convexPolygonPointCount, Manifold2D* manifold)
{
  real minCircleToPolyLength = Math::PositiveMax();
  Vec2 closestPoint;

  //Loop through all of the polygon's sides/segments to find the intersectionPoint on the
  //polygon that is closest to the center of the sphere
  const uint lastPoint = convexPolygonPointCount - 1;
  uint currPoint, nextPoint;
  for(uint i = 0; i < convexPolygonPointCount; ++i)
  {
    currPoint = i;
    nextPoint = (i == lastPoint) ? 0 : i + 1;

    Vec2 tempPoint = circleCenter;
    ClosestPointOnSegmentToPoint(convexPolygonPoints[currPoint], 
                                 convexPolygonPoints[nextPoint], &tempPoint);

    real length = Length(circleCenter - tempPoint);
    if(length < minCircleToPolyLength)
    {
      minCircleToPolyLength = length;
      closestPoint = tempPoint;
    }
  }

  //If the distance of intersectionPoint on the polygon closest to the circle's center is 
  //less than the circle's radius, then the circle is intersecting the polygon.
  if(minCircleToPolyLength < circleRadius)
  {
    if(manifold != nullptr)
    {
      Vec2 normal = closestPoint - circleCenter;
      normal /= minCircleToPolyLength;

      Vec2 pointOnPolygon = closestPoint;
    
      Vec2 pointOnCircle = circleCenter + (normal * circleRadius);

      real overlapAmount = circleRadius - minCircleToPolyLength;

      manifold->PointAt(0).Points[0] = pointOnCircle;
      manifold->PointAt(0).Points[1] = pointOnPolygon;
      manifold->PointAt(0).Depth = overlapAmount;
      manifold->Normal = normal;
      manifold->PointCount = 1;
    }
    return Point;
  }

  return None;  
}

///Intersect an n-sided convex polygon with an n-sided convex polygon.
Type ConvexPolygonConvexPolygon(const Vec2* convexPolygonPointsA, 
                                uint convexPolygonPointCountA,
                                const Vec2* convexPolygonPointsB, 
                                uint convexPolygonPointCountB,
                                Manifold2D* manifold)
{
  //Train Of Thought: 
  //   Assume that the two polygons are intersecting unless proved otherwise. If
  // there is at least one axis found where the projections of the two polygons
  // are not intersecting then the two polygons are not intersecting. However,
  // if the polygons are intersecting, then we want to find collision 
  // information with regards to the axis of LEAST overlap, as this is usually
  // the most desired direction in which to resolve the collision.
  //   The test axes come from the vectors perpendicular to the sides of the 
  // polygons. Usually, duplicate axes are not tested (no reason to test the 
  // same axis twice), however there is an arbitrary number of axes to test and
  // finding duplicates seemed to make the code harder to understand.

  //Store the minimum overlap and axis of intersection
  real minOverlap = Math::PositiveMax();
  Vec2 minAxis;

  //----------------------------------------------------------------------------
  //Project both polygons onto all of the axes of polygon A.
  uint lastPoint = convexPolygonPointCountA - 1;
  for(uint i = 0; i < convexPolygonPointCountA; ++i)
  {
    uint currPointIndex = i;
    uint nextPointIndex = (i == lastPoint) ? 0 : i + 1;
    const Vec2& currPoint = convexPolygonPointsA[currPointIndex];
    const Vec2& nextPoint = convexPolygonPointsA[nextPointIndex];

    //In 2D, the vector perpendicular to V = (x, y) is P = (y, -x)
    Vec2 axis = Normalized(nextPoint - currPoint);
    axis.Set(axis.y, -axis.x);

    Vec2 aProjection = ProjectPolygonOntoAxis(convexPolygonPointsA,
                                              convexPolygonPointCountA, axis);
    Vec2 bProjection = ProjectPolygonOntoAxis(convexPolygonPointsB,
                                              convexPolygonPointCountB, axis);

    real overlapAmount = IntersectIntervals(aProjection, bProjection);
    if(overlapAmount < real(0.0))
    {
      return None;
    }
    else if(overlapAmount < minOverlap)
    {
      minOverlap = overlapAmount;
      minAxis = axis;
    }
  }

  //----------------------------------------------------------------------------
  //Project both polygons onto all of the axes of polygon B.
  lastPoint = convexPolygonPointCountB - 1;
  for(uint i = 0; i < convexPolygonPointCountB; ++i)
  {
    uint currPointIndex = i;
    uint nextPointIndex = (i == lastPoint) ? 0 : i + 1;
    const Vec2& currPoint = convexPolygonPointsB[currPointIndex];
    const Vec2& nextPoint = convexPolygonPointsB[nextPointIndex];

    //In 2D, the vector perpendicular to V = (x, y) is P = (y, -x)
    Vec2 axis = Normalized(nextPoint - currPoint);
    axis.Set(axis.y, -axis.x);

    Vec2 aProjection = ProjectPolygonOntoAxis(convexPolygonPointsA,
                                              convexPolygonPointCountA, axis);
    Vec2 bProjection = ProjectPolygonOntoAxis(convexPolygonPointsB,
                                              convexPolygonPointCountB, axis);

    real overlapAmount = IntersectIntervals(aProjection, bProjection);
    if(overlapAmount < real(0.0))
    {
      return None;
    }
    else if(overlapAmount < minOverlap)
    {
      minOverlap = overlapAmount;
      minAxis = axis;
    }
  }

  //No information requested, and by this intersectionPoint we know that there is a 
  //collision between the two polygons
  if(manifold == nullptr)
  {
    return Point;
  }

  //----------------------------------------------------------------------------
  //Now we know that the two polygons are intersecting, all that's left is to 
  //calculate the information needed to resolve the collision
  return None;
}

///Test if a point lies on a line segment.
Type PointSegment(Vec2Param point, Vec2Param segmentStart, Vec2Param segmentEnd)
{
  // If the segment is in fact a point
  if(segmentStart == segmentEnd)
  {
    return point == segmentStart ? Inside : Outside;
  }

  // If the area of the triangle made by the segment and the point is 0,
  // the point must lie on the infinite line defined by the two points
  if(Geometry::Signed2DTriArea(segmentStart, segmentEnd, point) != real(0.0))
    return Outside;

  // Now that we know the point is on the infinite line, we must check if it's
  // within the endpoints of the segment
  Vec2 toEnd = segmentEnd - segmentStart;
  if(toEnd.Dot(point - segmentEnd) <= real(0.0) && 
     toEnd.Dot(point - segmentStart) >= real(0.0))
  {
    return Inside;
  }

  return Outside;
}

Type RayAabb(Vec2Param rayStart, Vec2Param rayDirection,
             Vec2Param aabbMinPoint, Vec2Param aabbMaxPoint,
             Interval* interval)
{
  ErrorIf(interval == nullptr, "Intersection - Invalid interval passed to " \
          "function.");
  ErrorIf((aabbMinPoint.x > aabbMaxPoint.x) ||
          (aabbMinPoint.y > aabbMaxPoint.y),
          "Intersection - Axis-aligned bounding box's minimum point is greater"\
          " than it's maximum point.");

  real tMin = real(0.0);
  real tMax = Math::PositiveMax();

  for(uint i = 0; i < 2; ++i)
  {
    //If the ray's direction is perpendicular to the current axis and the ray's
    //start isn't within the bounds of the box's faces on that axis then the 
    //ray is not intersecting the box.
    if(Math::IsZero(rayDirection[i]))
    {
      //Ray is parallel to slab. No hit if the origin is not within the slab.
      if((rayStart[i] < aabbMinPoint[i]) || (rayStart[i] > aabbMaxPoint[i]))
      {
        return None;
      }
    }
    else
    {
      //Compute intersection t-value of ray with near and far plane of slab.
      real ood = real(1.0) / rayDirection[i];
      real t1 = (aabbMinPoint[i] - rayStart[i]) * ood;
      real t2 = (aabbMaxPoint[i] - rayStart[i]) * ood;

      //Make t1 be intersection with near plane, t2 with far plane.
      if(t1 > t2)
      {
        Math::Swap(t1, t2);
      }

      //Compute the intersection of slab intersection intervals.
      tMin = Math::Max(tMin,t1);
      tMax = Math::Min(tMax,t2);

      //Exit with no collision as soon as slab intersection becomes empty.
      if(tMin > tMax)
      {
        return None;
      }
    }
  }

  //Ray intersects all 2 slabs. Return intersection t-value.
  interval->Min = tMin;
  interval->Max = tMax;
  return Other;
}

///Intersect two 2D segments.
Type SegmentSegment(Vec2Param segmentStartA, Vec2Param segmentEndA,
                    Vec2Param segmentStartB, Vec2Param segmentEndB, 
                    IntersectionPoint2D* intersectionPoint)
{
  const Vec2& a = segmentStartA;
  const Vec2& b = segmentEndA;
  const Vec2& c = segmentStartB;
  const Vec2& d = segmentEndB;

  //Sign of areas correspond to which side of ab points c and d are.
  //Compute winding of abd (+/-)
  real area1 = Geometry::Signed2DTriArea(a, b, d);
  //To intersect, must have sign opposite of area1
  real area2 = Geometry::Signed2DTriArea(a, b, c);

  //If c and d are on different sides of ab, areas have different signs.
  if(area1 * area2 < real(0.0))
  {
    //Compute signs for a and b with respect to segment cd.
    //Compute winding of cda (+/-)
    real area3 = Geometry::Signed2DTriArea(c, d, a);

    //Since area is constant area1 - area2 = area3 - area4, 
    //or area4 = area3 + area2 - area1
    real area4 = area3 + area2 - area1;

    //Points a and b on different sides of cd if areas have different signs.
    if(area3 * area4 < real(0.0))
    {
      if(intersectionPoint != nullptr)
      {
        //Segments intersect. Find intersection point along L(t) = a + t * (b-a)
        //Given height h1 of a over cd and height h2 of b over cd,
        //t = h1 / (h1 - h2) = (b * h1 / 2) / (b * h1 / 2 - b * h2 / h3) 
        //                   = area3 / (area3 - area4), where b (the base of the
        //triangles cda cdb, i.e., the length of cd) cancels out.
        intersectionPoint->T = area3 / (area3 - area4);
        intersectionPoint->Points[0] = a + intersectionPoint->T * (b - a);
        intersectionPoint->Points[1] = intersectionPoint->Points[0];
      }
      return Point;
    }
  }

  //Segments not intersecting (or collinear).
  return None;
}

}//namespace Intersection
