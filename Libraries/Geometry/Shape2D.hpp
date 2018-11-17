  ///////////////////////////////////////////////////////////////////////////////
  ///
  /// \file Shape2D.hpp
  /// Declaration of the Shape2D class.
  /// 
  /// Authors: Joshua Claeys
  /// Copyright 2012, DigiPen Institute of Technology
  ///
  //////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Polygon;

//--------------------------------------------------------------------- Shape 2D
class Shape2D
{
public:
  /// Constructor.
  Shape2D();

  /// Copy constructor.
  Shape2D(const Shape2D& rhs);

  Shape2D(const Polygon& outerContour);

  /// Destructor.
  ~Shape2D();

  /// Assignment operator.
  void operator=(const Shape2D& rhs);

  /// Copies the data from the given shape.
  void Assign(const Shape2D& rhs);

  /// Adds a contour to the shape.
  void AddContour(const Polygon& polygon);

  /// Quantizes the shape to a constant precision.
  void Quantize();

  //--------------------------------------------------------------- Modification
  /// Removes all contours in the shape.
  void Clear();

  static bool Subtract(const Shape2D& shapeA, 
                       const Shape2D& shapeB, 
                       Array<Shape2D>* outputs);

  static bool Union(const Shape2D& shapeA, 
                    const Shape2D& shapeB,
                    Array<Shape2D>* outputs);

  /// Translates the shape by the given translation.
  void Translate(Vec2Param translation);

  /// Scales the shape from its centroid by the given scalar.
  void Scale(Vec2Param scalar);

  /// Rotates the shape around its centroid by the given rotation.
  void Rotate(real rotation);

  /// Grows the shape by the given distance and stores it in the given shape.
  void Grow(real distance, bool beaking, Shape2D* shape);

  /// Grows the shape by the given distance.
  void Grow(real distance, bool beaking);

  /// Debug Draw.
  void DebugDraw(ByteColor color, bool filled, 
                 bool triangleEdges = false, float depth = 0.0f);
  void DebugDraw(ByteColor color, Mat4Param transform, bool filled, 
                 bool triangleEdges = false, float depth = 0.0f);

  /// Draws the triangles of the shape.
  void DrawTriangles(ByteColor color, bool borders = false, float depth = 0.0f);
  void DrawTriangles(ByteColor color, Mat4Param transform, bool borders = false, 
                     float depth = 0.0f);

  //----------------------------------------------------------------------- Info
  /// Returns whether or not the polygon is empty.
  bool Empty();

  /// Returns the total amount of vertices in the entire shape.
  uint GetVertexCount();

  /// Fills out the given array with all the vertices of the entire shape.
  void GetVertices(Array<Vec2>* vertices);

  /// Fills out the given array with the size of each contour.
  void GetContours(Array<uint>* contours);

  /// Triangulates the mesh and fills out the given array of indices.
  void GetTriangleIndices(Array<uint>* indices);

  /// Triangulates the mesh and fills out the given array of indices.
  void GetTriangleIndices(Array<Vec2>& vertices, Array<uint>& contours, 
                          Array<uint>* indices);

  /// Calculates the barycenter (point average) of the shape.
  Vec2 GetBarycenter();

  Aabb GetBoundingBox();

  /// Returns whether or not the given point is inside the polygon.
  bool ContainsPoint(Vec2Param point);

   //Remove edges with length smaller than vCalibration
  void Calibrate(const float& vCalibration);

  /// Validates the Shape and throws an error if it's invalid.
  bool Validate();
  bool Validate(Array<String>& errors);

  //------------------------------------------------------------------- Indexing
  struct Index
  {
    Index() : Contour(-1), Vertex(-1) {}
    Index(int contour, int vertex) : Contour(contour), Vertex(vertex) {}
    bool operator==(const Index& rhs){return (Contour == rhs.Contour && Vertex == rhs.Vertex);}
    bool operator!=(const Index& rhs){return !(*this == rhs);}
    bool IsValid() { return Contour != -1 && Vertex != -1; }
    void Invalidate() { Contour = -1; Vertex = -1; }

    int Contour;
    int Vertex;
  };

  Index PickVertex(Vec2Param pos, real vertRadius);
  Index PickEdge(Vec2Param pos, real edgeRadius);
  Index AddVertexOnEdge(Index& edgeIndex, Vec2Param vertPos);
  bool SlideVertex(Index& index, Vec2Param newPos);
  void RemoveVertexAtIndex(Index index);
  Vec2* GetVertexAtIndex(Index index);

  //---------------------------------------------------------------------- Range
  struct range
  {
    range(Shape2D* shape);
    Vec2 Front();
    void PopFront();
    bool Empty();
    size_t Length();

    Index CurrIndex();

  private:
    uint mContour;
    uint mVertex;
    Shape2D* mShape;
  };

  /// Returns a range of all vertices in the shape.
  range All();

  //----------------------------------------------------------------------- Data
public:
  /// The contours that make up the shape.  The first will always be the outer
  /// contour (counter-clockwise), and any after the first will be inner
  /// contours (holes, clockwise).
  Array<Polygon> mContours;

  Array<uint> mTriangleIndices;
};

void BuildShapesFromContours(const ContourArray& contours,
                             Array<Shape2D>* shapes);

//---------------------------------------------------------------------- Helpers
/// Transforms the shape by the given matrix.
void TransformShape(Mat4Param matrix, Shape2D* shape);
void TransformShape(Mat3Param matrix, Shape2D* shape);

}//namespace Zero
