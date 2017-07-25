///////////////////////////////////////////////////////////////////////////////
///
/// \file Polygon.cpp
/// Implementation of the Polygon class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2012, DigiPen Institute of Technology
///
//////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------- Polygon
//Constructor.
Polygon::Polygon()
{
  
}

//Copy constructor.
Polygon::Polygon(const Polygon& rhs) : mData(rhs.mData)
{

}

Polygon::Polygon(const Array<Vec2>& rhs) : mData(rhs)
{

}

Polygon::Polygon(const Vec2* verts, uint size)
{
  mData.Resize(size);

  for(uint i = 0; i < size; ++i)
    mData[i] = verts[i];
}

//Indexing operators.
Vec2& Polygon::operator[](uint index)
{
  return mData[index];
}

const Vec2& Polygon::operator[](uint index) const
{
  return mData[index];
}

//Adds the given vertex to the polygon.
void Polygon::AddVertex(Vec2Param vert)
{
  mData.PushBack(vert);
}

float PolygonQuantizeFloat(float value)
{
//   const float precision = 0.001f;
//   const float inversePrecision = 1.0f / precision;
//   float result = Math::Floor(value * inversePrecision) * precision;
  const float cPlaces = 3.0f;
  const float cScale = Math::Pow(10.0f, cPlaces);
  return Math::Floor(value * cScale + 0.5f) / cScale;
}

//Quantizes the shape to a constant precision.
void Polygon::Quantize()
{
  for(uint i = 0; i < mData.Size(); ++i)
  {
    mData[i].x = PolygonQuantizeFloat(mData[i].x);
    mData[i].y = PolygonQuantizeFloat(mData[i].y);
  }
}

//--------------------------------------------------------------- Modification
//Clears all vertices in the polygon.
void Polygon::Clear()
{
  mData.Clear();
}

//Translates the polygon by the given translation.
void Polygon::Translate(Vec2Param translation)
{
  // Translate each vertex
  for(uint i = 0; i < mData.Size(); ++i)
    mData[i] += translation;
}

//Scales the polygon from its centroid by the given scalar.
void Polygon::Scale(Vec2Param scalar)
{
  Vec2 center = GetBarycenter();
  Scale(scalar, center);
}

//Scales the polygon from the given center by the given scalar.
void Polygon::Scale(Vec2Param scalar, Vec2Param center)
{
  // Scale each vertex
  for(uint i = 0; i < mData.Size(); ++i)
  {
    // Bring the coordinates into the local space of the center
    mData[i] -= center;
    // Scale the vertex
    mData[i] *= scalar;
    // Bring each vertex back to world space
    mData[i] += center;
  }
}

//Rotates the polygon around its centroid by the given rotation.
void Polygon::Rotate(real radians)
{
  Vec2 center = GetBarycenter();
  Rotate(radians, center);
}

//Rotates the polygon around the given center by the given rotation.
void Polygon::Rotate(real radians, Vec2Param center)
{
  // Build the rotation matrix
  Math::Matrix2 rotationMtx;
  rotationMtx.m00 = Math::Cos(radians);
  rotationMtx.m01 = -Math::Sin(radians);
  rotationMtx.m10 = Math::Sin(radians);
  rotationMtx.m11 = Math::Cos(radians);
  
  // Rotate each vertex
  for(uint i = 0; i < mData.Size(); ++i)
  {
    // Bring the coordinates into the local space of the center
    mData[i] -= center;
    // Rotate the vertex
    mData[i] = rotationMtx.Transform(mData[i]);
    // Bring each vertex back to world space
    mData[i] += center;
  }
}

//Grows the polygon by the given distance and stores the result in
//the given output polygon.
void Polygon::Grow(real distance, bool beaking, Polygon* output)
{
  // We need at least 3 vertices to do anything
  if(mData.Size() < 3)
    return;

   output->mData.Assign(mData.All());
   for(uint i = 0; i < mData.Size(); ++i)
   {
     Vec2 prev = mData[PrevIndex(i)];
     Vec2 curr = mData[i];
     Vec2 next = mData[NextIndex(i)];
 
     Vec2 dir = GetBisector(curr - prev, next - curr);
 
     output->mData[i] += Normalized(dir) * distance;
   }
}

//Grows the polygon by the given distance.
void Polygon::Grow(real distance, bool beaking)
{
  Polygon p;
  Grow(distance, beaking, &p);
  mData.Assign(p.mData.All());
}

bool Collinear(Vec2Param p0, Vec2Param p1, Vec2Param p2, real tolerance)
{
  //real dot = Dot(Normalized(p1 - p0), Normalized(p2 - p1));
  //return dot > (real(1.0) - 0.001f/*tolerance*/);
  real area = Geometry::Signed2DTriArea(p0, p1, p2);
  return Math::InRange(area, -tolerance, tolerance);
}

//Removes all collinear points in the polygon based on the given tolerance.
void Polygon::RemoveCollinearPoints(real tolerance)
{
  if(mData.Size() < 3)
    return;

  for(uint i = 0; i < mData.Size();)
  {
    Vec2 prev = mData[PrevIndex(i)];
    Vec2 curr = mData[i];
    Vec2 next = mData[NextIndex(i)];

    if(Collinear(prev, curr, next, tolerance))
      mData.EraseAt(i);
    else if(Length(curr - next) < 0.01f)
      mData.EraseAt(i);
    else
      ++i;

    if(mData.Size() < 3)
    {
      mData.Clear();
      return;
    }
  }
}

//Removes all duplicate points in the shape.
void Polygon::RemoveDuplicatePoints()
{
  for(uint i = 0; i < mData.Size();)
  {
    if(mData[i] == mData[NextIndex(i)])
      mData.EraseAt(i);
    else
      ++i;
  }
}

//Removes all self intersections.
void Polygon::RemoveSelfIntersections()
{
  for(uint i = 0; i < mData.Size(); ++i)
  {
    Vec2 p0 = mData[i];
    Vec2 p1 = mData[NextIndex(i)];

    for(uint j = i + 1; j < mData.Size(); ++j)
    {
      Vec2 p2 = mData[j];
      Vec2 p3 = mData[NextIndex(j)];

      using namespace Intersection;
      IntersectionPoint2D point;
      Intersection::Type result = SegmentSegment(p0, p1, p2, p3, &point);
      if(result == Intersection::Point)
      {
        if(i == 0)
        {
          mData.PopBack();
          p0 = point.Points[0];
          mData[0] = p0;
        }
        else
        {
          // Erase all vertices
          for(uint k = j; k > i; --k)
            mData.EraseAt(k);

          // Add the point
          p1 = point.Points[0];
          mData.InsertAt(i + 1, p1);
          j = i;
        }
      }
    }
  }
}

//Forces the polygon to be counter clockwise.
void Polygon::MakeCounterClockwise()
{
  // If it's clockwise, make it counter clockwise
  if(IsClockwise())
    Reverse(mData.Begin(), mData.End());
}

//Forces the polygon to be clockwise.
void Polygon::MakeClockwise()
{
  // If it's counter clockwise, make it clockwise
  if(!IsClockwise())
    Reverse(mData.Begin(), mData.End());
}

//------------------------------------------------------------------------- Info
//Returns the amount of vertices in the polygon.
uint Polygon::Size() const
{
  return mData.Size();
}

//Returns whether or not the polygon has any vertices.
bool Polygon::Empty()
{
  return mData.Empty();
}

//Returns the next index (wraps around).
uint Polygon::NextIndex(uint index) const
{
  return (index + 1) % Size();
}

//Returns the previous index (wraps around).
uint Polygon::PrevIndex(uint index)
{
  if(index == 0)
    return mData.Size() - 1;
  return index - 1;
}

//Returns a range of all the allEdges.
Polygon::range Polygon::GetEdges()
{
  return range(this);
}

real Polygon::GetPerimeterLength()
{
  real length = 0.0f;

  for(uint i = 0; i < mData.Size(); ++i)
  {
    Vec2 curr = mData[i];
    Vec2 next = mData[NextIndex(i)];

    length += Math::Distance(curr, next);
  }

  return length;
}

//Calculates the barycenter (point average) of the polygon.
Vec2 Polygon::GetBarycenter()
{
  if(mData.Size() < 1)
    return Vec2();

  Vec2 center = mData[0];
  for(uint i = 1; i < mData.Size(); ++i)
    center += mData[i];

  center /= float(mData.Size());
  return center;
}

//Calculates the centroid (center of mass) of the polygon.
Vec2 Polygon::GetCentroid()
{
  Vec2 centroid;
  Geometry::CalculatePolygonCentriod(mData.Data(), mData.Size(), &centroid);
  return centroid;
}

//Constructs the axis-aligned bounding box of the polygon.
Aabb Polygon::GetBoundingBox()
{
  Vec2 min, max;
  Geometry::GenerateAabb(mData.Data(), mData.Size(), &min, &max);
  Aabb aabb;
  aabb.SetMinAndMax(Vec3(min), Vec3(max));
  return aabb;
}

Vec2 Polygon::GetBisector(Vec2Param v0, Vec2Param v1)
{
  // Get the perpendicular of each vector
  Vec2 perp0 = Vec2(v0.y, -v0.x);
  Vec2 perp1 = Vec2(v1.y, -v1.x);

  AttemptNormalize(perp0);
  AttemptNormalize(perp1);

  Vec2 dir = (perp0 + perp1) * 0.5f;
  return Math::Normalized(dir);
}

//Returns whether or not the given point is inside the polygon.
bool Polygon::ContainsPoint(Vec2Param point)
{
  int winding = 0;

  // Iterate through polygon's edges
  forRange(Edge e, GetEdges())
  {
    // Pull out the points
    Vec2 p0 = e.p0;
    Vec2 p1 = e.p1;

    // Test if a point is directly on the edge
    Vec2 edge = p1 - p0;
    float area = Geometry::Signed2DTriArea(p0, p1, point);
    if(area == real(0.0) && Dot((point - p0), edge) >= real(0.0) 
                         && Dot((point - p1), edge) <= real(0.0))
    {
      return false;
    }
    // Test edge for intersection with ray from point
    if(p0.y <= point.y)
    {
      if(p1.y > point.y && area > real(0.0))
        ++winding;
    }
    else if(p1.y <= point.y && area < real(0.0))
    {
      --winding;
    }
  }
  if(winding == 0)
    return false;
  return true;
}

//Returns whether or not the given polygon overlaps with this polygon.
ShapeSegResult::Enum Polygon::Intersects(Polygon* rhs)
{
  forRange(Edge e0, this->GetEdges())
  {
    forRange(Edge e1, rhs->GetEdges())
    {
      Vec2 points[2];
      ShapeSegResult::Enum result = ShapeSegmentSegment(e0.p0, e0.p1, e1.p0, e1.p1, points);
      // If any edges are intersecting, 
      if(result != ShapeSegResult::None)
        return result;
    }
  }
  return ShapeSegResult::None;
}

//Returns whether or not the points in the polygon are in clockwise order.
bool Polygon::IsClockwise()
{
  return Geometry::DetermineWindingOrder(mData.Data(), mData.Size()) < real(0.0);
}

bool Polygon::Validate() const
{
  Array<String> errors;
  return Validate(errors);
}

//Makes sure the polygon is not self intersecting.
bool Polygon::Validate(Array<String>& errors) const
{
  //No need to validate polygons with no points.
  if(mData.Size() < 3)
  {
    errors.PushBack("Polygons must have more than 3 vertices.");
    return false;
  }

  // Check for duplicate points
  for(uint i = 0; i < mData.Size(); ++i)
  {
    if(mData[i] == mData[NextIndex(i)])
    {
     // errors.PushBack("Polygons cannot have duplicate points.");
      return false;
    }
  }

  // Go through all of the segments in the polygon and test to see if any are 
  // intersecting
  uint pointCount = uint(mData.Size());
  for(uint i = 0; i < (pointCount - 2); ++i)
  {
    //Test the current edge against all non-neighboring edges
    //(there's no way to intersect neighboring edges unless overlapping linearly)
    uint edgeI[2] = { i, i + 1 };
    for(uint j = i + 2; j < pointCount; ++j)
    {
      if(i == 0 && j == (pointCount - 1))
      {
        continue;
      }

      uint edgeJ[2] = { j, NextIndex(j)};
      Vec2 points[2];
      ShapeSegResult::Enum result = ShapeSegmentSegment(mData[edgeI[0]], mData[edgeI[1]],
                                                        mData[edgeJ[0]], mData[edgeJ[1]], points);
      if(result == ShapeSegResult::Point)
        errors.PushBack("Polygons cannot be self-intersecting.");
    }
  }

  // Go through all of the segments and check for intersection with their neighbors
  for(uint i = 0; i < (pointCount - 1); ++i)
  {
    uint thisEdge[2] = { i, i + 1 };
    uint nextEdge[2] = { i + 1, (i + 2) % pointCount };
    Vec2 points[2];
    ShapeSegResult::Enum result = ShapeSegmentSegment(mData[thisEdge[0]], mData[thisEdge[1]],
                                                      mData[nextEdge[0]], mData[nextEdge[1]],
                                                      points);
    if(result == ShapeSegResult::Segment)
      errors.PushBack("Polygons cannot be self-intersecting.");
  }

  return errors.Empty();
}

//Debug draw.
void Polygon::DebugDraw(ByteColor color, bool windingOrder, float depth)
{
  DebugDraw(color, Mat4::cIdentity, windingOrder, depth);
}

void Polygon::DebugDraw(ByteColor color, Mat4Param transform, bool windingOrder, 
                        float depth)
{
  forRange(Edge edge, GetEdges())
  {
    using namespace Zero;
    Vec3 p0 = Math::ToVector3(edge.p0, depth);
    Vec3 p1 = Math::ToVector3(edge.p1, depth);
    
    // Transform the points
    p0 = Math::TransformPoint(transform, p0);
    p1 = Math::TransformPoint(transform, p1);

    if(windingOrder)
      gDebugDraw->Add(Debug::Line(p0, p1).OnTop(true).HeadSize(0.18f).Color(color));
    else
      gDebugDraw->Add(Debug::Line(p0, p1).OnTop(true).Color(color));
  }
}

void Polygon::DebugDrawFilled(ByteColor color, Mat4Param transform) const
{
  Polygon copy = *this;


  copy.RemoveSelfIntersections();
  copy.RemoveDuplicatePoints();

  for(size_t i = 0; i < copy.Size(); ++i)
  {

    for (size_t j = i + 1; j < copy.Size(); ++j)
    {
      if(copy.mData[i] == copy.mData[j])
      {

        return;
      }

    }
  }
  
  if(copy.mData.Size() < 3 || copy.IsClockwise())
  {
    return;
  }

  Array<uint> indices;
  Array<uint> contours;
  contours.PushBack(copy.mData.Size());
  bool result = Geometry::Triangulate(copy.mData, contours, &indices);

  uint triangleCount = indices.Size() / 3;
  if(result == true && triangleCount > 0)
  {
    for(size_t i = 0; i < triangleCount * 3; i += 3)
    {
      Vec3 a = Math::ToVector3(copy[indices[i]]);
      Vec3 b = Math::ToVector3(copy[indices[i + 1]]);
      Vec3 c = Math::ToVector3(copy[indices[i + 2]]);

      a = Math::TransformPoint(transform, a);
      b = Math::TransformPoint(transform, b);
      c = Math::TransformPoint(transform, c);

      gDebugDraw->Add(Debug::Triangle(a, b, c)
        .Color(color)
        );
    }
  }
}

//------------------------------------------------------------------- Edge Range
Polygon::Edge::Edge(Vec2Ref a, Vec2Ref b)
  : p0(a), p1(b)
{

}

Polygon::range::range(Polygon* polygon)
  : mPolygon(*polygon)
{
  mCurrIndex = 0;
}

Polygon::Edge Polygon::range::Front()
{
  Vec2& p0 = mPolygon[mCurrIndex];
  Vec2& p1 = mPolygon[mPolygon.NextIndex(mCurrIndex)];
  return Edge(p0, p1);
}

void Polygon::range::PopFront()
{
  ++mCurrIndex;
}

bool Polygon::range::Empty()
{
  return mCurrIndex >= mPolygon.Size();
}

//---------------------------------------------------------------------- Helpers
ShapeSegResult::Enum ShapeSegmentSegment(Vec2Param segmentStartA, Vec2Param segmentEndA,
                                        Vec2Param segmentStartB, Vec2Param segmentEndB, 
                                        Vec2 intersectionPoints[2])
{
  const real cAreaEpsilon = real(0.00000);
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
  if(area1 * area2 <= cAreaEpsilon)
  {
    //Compute signs for a and b with respect to segment cd.
    //Compute winding of cda (+/-)
    real area3 = Geometry::Signed2DTriArea(c, d, a);

    //Since area is constant area1 - area2 = area3 - area4, 
    //or area4 = area3 + area2 - area1
    real area4 = area3 + area2 - area1;

    //Points a and b on different sides of cd if areas have different signs.
    if(area3 * area4 <= cAreaEpsilon)
    {
      //If the lines are collinear.
      if(Math::InRange(area1, -cAreaEpsilon, cAreaEpsilon) &&
         Math::InRange(area2, -cAreaEpsilon, cAreaEpsilon))
      {
        Vec2 points[4] = {a, b, c, d};
        Vec2 lineDir = Normalized(b - a);
        real extents[4];
        extents[0] = Dot(lineDir, a);
        extents[1] = Dot(lineDir, b);
        extents[2] = Dot(lineDir, c);
        extents[3] = Dot(lineDir, d);

        // Sort the points
        uint indices[4] = {0,1,2,3};

        // Bubble sort
        for(uint i = 0; i < 3; ++i)
        {
          for(uint j = 0; j < 3 - i; ++j)
          {
            if(extents[indices[j]] < extents[indices[j + 1]])
              Math::Swap(indices[j], indices[j + 1]);
          }
        }

        //Check to see if the neighboring points are on the same line segment.
        if((points[indices[0]] == a && points[indices[1]] == b) || 
           (points[indices[0]] == b && points[indices[1]] == a) || 
           (points[indices[0]] == c && points[indices[1]] == d) || 
           (points[indices[0]] == d && points[indices[1]] == c))
        {
          //If so, the two lines cannot be overlapping collinearly.
          return ShapeSegResult::None;
        }

        // Pull out the middle of the two points
        intersectionPoints[0] = points[indices[1]];
        intersectionPoints[1] = points[indices[2]];

        return ShapeSegResult::Segment;
      }

      //Segments intersect. Find intersection point along L(t) = a + t * (b-a)
      //Given height h1 of a over cd and height h2 of b over cd,
      //t = h1 / (h1 - h2) = (b * h1 / 2) / (b * h1 / 2 - b * h2 / h3) 
      //                   = area3 / (area3 - area4), where b (the base of the
      //triangles cda cdb, i.e., the length of cd) cancels out.
      real time = area3 / (area3 - area4);
      intersectionPoints[0] = a + time * (b - a);

      return ShapeSegResult::Point;
    }
  }

  //Segments not intersecting
  return ShapeSegResult::None;
}

/// Transforms the polygon by the given matrix.
void TransformPolygon(Mat3Param matrix, Polygon* polygon)
{
  uint size = polygon->Size();
  // Transform each point
  for(uint i = 0; i < size; ++i)
    polygon->mData[i] = Math::TransformPoint(matrix, polygon->mData[i]);
}

void TransformPolygon(Mat4Param matrix, Polygon* polygon)
{
  uint size = polygon->Size();
  // Transform each point
  for(uint i = 0; i < size; ++i)
    polygon->mData[i] = Math::ToVector2(Math::TransformPoint(matrix, Math::ToVector3(polygon->mData[i])));
}

}//namespace Geometry
