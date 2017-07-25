///////////////////////////////////////////////////////////////////////////////
///
/// \file Polygon.hpp
/// Declaration of the Polygon class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2012, DigiPen Institute of Technology
///
//////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum3(ShapeSegResult, None, Point, Segment);

//---------------------------------------------------------------------- Polygon
class Polygon
{
public:
  /// Forward declarations.
  struct range;

  /// Constructor.
  Polygon();

  /// Copy constructor.
  Polygon(const Polygon& rhs);
  Polygon(const Array<Vec2>& rhs);
  Polygon(const Vec2* verts, uint size);

  /// Indexing operators.
  Vec2& operator[](uint index);
  const Vec2& operator[](uint index) const;

  /// Adds the given vertex to the polygon.
  void AddVertex(Vec2Param vert);

  /// Quantizes the shape to a constant precision.
  void Quantize();

  //------------------------------------------------------------- Modification
  /// Clears all vertices in the polygon.
  void Clear();

  /// Translates the polygon by the given translation.
  void Translate(Vec2Param translation);

  /// Scales the polygon from its centroid by the given scalar.
  void Scale(Vec2Param scalar);

  /// Scales the polygon from the given center by the given scalar.
  void Scale(Vec2Param scalar, Vec2Param center);

  /// Rotates the polygon around its centroid by the given rotation.
  void Rotate(real radians);

  /// Rotates the polygon around the given center by the given rotation.
  void Rotate(real radians, Vec2Param center);

  /// Grows the polygon by the given distance and stores the result in
  /// the given output polygon.
  void Grow(real distance, bool beaking, Polygon* output);
  
  /// Grows the polygon by the given distance.
  void Grow(real distance, bool beaking);

  /// Removes all collinear points in the polygon based on the given tolerance.
  void RemoveCollinearPoints(real tolerance);

  /// Removes all duplicate points in the shape.
  void RemoveDuplicatePoints();

  /// Removes all self intersections.
  void RemoveSelfIntersections();

  /// Forces the polygon to be counter clockwise.
  void MakeCounterClockwise();
  
  /// Forces the polygon to be clockwise.
  void MakeClockwise();

  //--------------------------------------------------------------------- Info
  /// Returns the amount of vertices in the polygon.
  uint Size() const;

  /// Returns whether or not the polygon has any vertices.
  bool Empty();

  /// Returns the next index (wraps around).
  uint NextIndex(uint index) const;

  /// Returns the previous index (wraps around).
  uint PrevIndex(uint index);

  /// Returns a range of all the edges.
  range GetEdges();

  /// Get the length of the perimeter.
  real GetPerimeterLength();

  /// Calculates the barycenter (point average) of the polygon.
  Vec2 GetBarycenter();

  /// Calculates the centroid (center of mass) of the polygon.
  Vec2 GetCentroid();

  /// Constructs the axis-aligned bounding box of the polygon.
  Aabb GetBoundingBox();

  ///Returns the bisector of the given vectors.
  Vec2 GetBisector(Vec2Param v0, Vec2Param v1);

  /// Returns whether or not the given point is inside the polygon.
  bool ContainsPoint(Vec2Param point);

  /// Returns whether or not the given polygon overlaps with this polygon.
  ShapeSegResult::Enum Intersects(Polygon* rhs);

  /// Returns whether or not the points in the polygon are in clockwise order.
  bool IsClockwise();

  /// Makes sure the polygon is not self intersecting.
  bool Validate() const;
  bool Validate(Array<String>& errors) const;

  /// Debug draw.
  void DebugDraw(ByteColor color, bool windingOrder = false, float depth = 0.0f);
  void DebugDraw(ByteColor color, Mat4Param transform, bool windingOrder = false,
                 float depth = 0.0f);
  void DebugDrawFilled(ByteColor color, Mat4Param transform) const;

public:
  //--------------------------------------------------------------- Edge Range
  struct Edge
  {
    Edge(Vec2Ref a, Vec2Ref b);
    Vec2& p0;
    Vec2& p1;
  };

  struct range
  {
    range(Polygon* polygon);
    Edge Front();
    void PopFront();
    bool Empty();

  private:
    uint mCurrIndex;
    Polygon& mPolygon;
  };

//----------------------------------------------------------------------- Data
public:
  /// Each vertex in the polygon.
  Array<Vec2> mData;
};

//---------------------------------------------------------------------- Helpers
ShapeSegResult::Enum ShapeSegmentSegment(Vec2Param segmentStartA, Vec2Param segmentEndA,
                                         Vec2Param segmentStartB, Vec2Param segmentEndB, 
                                         Vec2 intersectionPoints[2]);

float PolygonQuantizeFloat(float value);

/// Transforms the polygon by the given matrix.
void TransformPolygon(Mat4Param matrix, Polygon* polygon);
void TransformPolygon(Mat3Param matrix, Polygon* polygon);

} //namespace Geometry

