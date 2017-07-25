///////////////////////////////////////////////////////////////////////////////
///
/// \file Shape2D.cpp
/// Implementation of the Shape2D class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2012, DigiPen Institute of Technology
///
//////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

f32 ComputeArea(const Array<Vec2>& contour)
{
  f32 area = 0;
  int j = (contour.Size() - 1);
  for(size_t i = 0; i < contour.Size(); ++i) 
  {
    area += (contour[j].x + contour[i].x) *
            (contour[j].y - contour[i].y); 
    j = i; 
  }

  return Math::Abs(area * .5f); 
}

struct ContourInfo
{
  size_t Index;
  f32 Area;
};

bool CompareArea(const ContourInfo& a, const ContourInfo& b)
{
  if(a.Area == b.Area)
  {
    return a.Index < b.Index;
  }
  return a.Area < b.Area;
};

void BuildShapesFromContours(const ContourArray& contours,
                             Array<Shape2D>* shapes)
{
  ContourArray holes;

  // In case of nested outer contours we need to sort them by area so
  // we can do correct containment checks
  Array<ContourInfo> infos;
  for(size_t i = 0; i < contours.Size(); ++i)
  {
    const Array<Vec2>& contour = contours[i];

    if (Geometry::DetermineWindingOrder(contour.Data(), contour.Size()) > 0.f)
    {
      shapes->PushBack(Shape2D());
      shapes->Back().mContours.PushBack(Polygon(contour));


      ContourInfo info;
      info.Index = shapes->Size() - 1;
      info.Area = ComputeArea(contour);
      infos.PushBack(info);
    }
    else
    {
      holes.PushBack(contour);
    }
  }

  Sort(infos.All(), CompareArea);

  for(size_t i = 0; i < holes.Size(); ++i)
  {
    for(size_t j = 0; j < infos.Size(); ++j)
    {
      const ContourInfo& info = infos[j];

      Shape2D& shape = (*shapes)[info.Index];
      if(shape.mContours[0].ContainsPoint(holes[i][0]))
      {
        shape.mContours.PushBack(Polygon(holes[i]));
        break;
      }
    }
  }  
}

//Constructor
Shape2D::Shape2D()
{
}

//Copy constructor.
Shape2D::Shape2D(const Shape2D& rhs)
{
  mContours.Resize(rhs.mContours.Size());
  for(uint i = 0; i < rhs.mContours.Size(); ++i)
    mContours[i] = rhs.mContours[i];
}

Shape2D::Shape2D(const Polygon& outerContour)
{
  mContours.PushBack(outerContour);
}

//Destructor.
Shape2D::~Shape2D()
{
  Clear();
}

//Assignment operator.
void Shape2D::operator=(const Shape2D& rhs)
{
  mContours = rhs.mContours;
}

//Copies the data from the given shape.
void Shape2D::Assign(const Shape2D& rhs)
{
  // Resize if we're growing in size
  mContours.Resize(rhs.mContours.Size());

  for(uint i = 0; i < rhs.mContours.Size(); ++i)
  {
    mContours[i] = rhs.mContours[i];
  }
}

//Adds a contour to the shape.
void Shape2D::AddContour(const Polygon& polygon)
{
  ErrorIf(polygon.Size() == 0, "Contour of size 0.");
  mContours.PushBack(polygon);
  Validate();
}

//Quantizes the shape to a constant precision.
void Shape2D::Quantize()
{
  for(uint i = 0; i < mContours.Size(); ++i)
    mContours[i].Quantize();
}

//----------------------------------------------------------------- Modification
//Removes all contours in the shape.
void Shape2D::Clear()
{
  mContours.Clear();
}

bool Shape2D::Union(const Shape2D& shapeAA, 
                    const Shape2D& shapeBB,
                    Array<Shape2D>* outputs)
{
  const f32 angleTolerance = 0.00872654f;
  const f32 distanceTolerance = 0.00001f;

  ContourArray results;

  Shape2D shapeA = shapeAA;
  Shape2D shapeB = shapeBB;

  shapeA.Quantize();
  shapeB.Quantize();

  ContourArray contoursA;
  for(size_t i = 0; i < shapeA.mContours.Size(); ++i)
  {
    contoursA.PushBack(shapeA.mContours[i].mData);
  }

  ContourArray contoursB;
  for(size_t i = 0; i < shapeB.mContours.Size(); ++i)
  {
    contoursB.PushBack(shapeB.mContours[i].mData);
  }

  Csg::OperationInput input;
  input.Operation = Csg::Operation::Union;
  input.ContoursA = &contoursA;
  input.ContoursB = &contoursB;
  input.CollinearAngleTolerance = angleTolerance;
  input.DistanceTolerance = distanceTolerance;
  bool success = Csg::Operate(input, &results);
  if (success == false)
  {
    return false;
  }

  BuildShapesFromContours(results, outputs);

  return true;
}

bool Shape2D::Subtract(const Shape2D& shapeAA, 
                       const Shape2D& shapeBB,
                       Array<Shape2D>* outputs)
{
  const f32 angleTolerance = 0.00872654f;
  const f32 distanceTolerance = 0.00001f;
  
  ContourArray results;

  Shape2D shapeA = shapeAA;
  Shape2D shapeB = shapeBB;

  shapeA.Quantize();
  shapeB.Quantize();

  ContourArray contoursA;
  for(size_t i = 0; i < shapeA.mContours.Size(); ++i)
  {
    contoursA.PushBack(shapeA.mContours[i].mData);
  }

  ContourArray contoursB;
  for(size_t i = 0; i < shapeB.mContours.Size(); ++i)
  {
    contoursB.PushBack(shapeB.mContours[i].mData);
  }

  Csg::OperationInput input;
  input.Operation = Csg::Operation::Subtract;
  input.ContoursA = &contoursA;
  input.ContoursB = &contoursB;
  input.CollinearAngleTolerance = angleTolerance;
  input.DistanceTolerance = distanceTolerance;
  bool success = Csg::Operate(input, &results);
  if(success == false)
  {
    return false;
  }

  BuildShapesFromContours(results, outputs);

  return true;
}

//Translates the shape by the given translation.
void Shape2D::Translate(Vec2Param translation)
{
  // Translate each contour in the shape
  for(uint i = 0; i < mContours.Size(); ++i)
    mContours[i].Translate(translation);
}

//Scales the shape from its centroid by the given scalar.
void Shape2D::Scale(Vec2Param scalar)
{
  // Get the center of the shape
  Vec2 center = GetBarycenter();
  
  // Scale each contour around the center of the shape
  for(uint i = 0; i < mContours.Size(); ++i)
    mContours[i].Scale(scalar, center);
}

//Rotates the shape around its centroid by the given rotation.
void Shape2D::Rotate(real rotation)
{
  // Get the center of the shape
  Vec2 center = GetBarycenter();

  // Rotate each contour around the center of the shape
  for(uint i = 0; i < mContours.Size(); ++i)
    mContours[i].Rotate(rotation, center);
}

//Grows the shape by the given distance and stores it in the given shape.
void Shape2D::Grow(real distance, bool beaking, Shape2D* shape)
{
  // Grow each polygon in the shape
  for(uint i = 0; i < mContours.Size(); ++i)
    shape->mContours[i].Grow(distance, beaking);
}

//Grows the shape by the given distance.
void Shape2D::Grow(real distance, bool beaking)
{
  Grow(distance, beaking, this);
}

//Debug Draw.
void Shape2D::DebugDraw(ByteColor color, bool filled, bool triangleEdges, 
                        float depth)
{
  DebugDraw(color, Mat4::cIdentity, filled, triangleEdges, depth);
}

void Shape2D::DebugDraw(ByteColor color, Mat4Param transform, bool filled, 
                        bool triangleEdges, float depth)
{
  // Draw each contour
  for(uint i = 0; i < mContours.Size(); ++i)
    mContours[i].DebugDraw(color, transform, triangleEdges, depth);


  // Draw the triangles
  if(filled)
  {
    SetAlphaByte(color, 40);
    DrawTriangles(color, transform, triangleEdges, depth);
  }
}

//Draws the triangles of the shape.
void Shape2D::DrawTriangles(ByteColor color, bool borders, float depth)
{
  DrawTriangles(color, Mat4::cIdentity, borders, depth);
}

void Shape2D::DrawTriangles(ByteColor color, Mat4Param transform, 
                            bool borders, float depth)
{
  Array<Vec2> vertices;
  GetVertices(&vertices);

  if(vertices.Size() < 3)
    return;

  Array<uint> indices;
  GetTriangleIndices(&indices);

  for(uint i = 0; i < indices.Size(); i += 3)
  {
    // Get the 3 points
    Vec3 a = Math::ToVector3(vertices[indices[i]], depth);
    Vec3 b = Math::ToVector3(vertices[indices[i + 1]], depth);
    Vec3 c = Math::ToVector3(vertices[indices[i + 2]], depth);

    // Transform the points
    a = Math::TransformPoint(transform, a);
    b = Math::TransformPoint(transform, b);
    c = Math::TransformPoint(transform, c);

    using namespace Zero;
    gDebugDraw->Add(Debug::Triangle(a, b, c).Color(color).Alpha(128).Border(borders));
  }
}

//----------------------------------------------------------------------- Info
//Returns whether or not the polygon is empty.
bool Shape2D::Empty()
{
  return GetVertexCount() == 0;
}

//Returns the total amount of vertices in the entire shape.
uint Shape2D::GetVertexCount()
{
  uint count = 0;
  for(uint i = 0; i < mContours.Size(); ++i)
    count += mContours[i].Size();
  return count;
}

//Fills out the given array with all the vertices of the entire shape.
void Shape2D::GetVertices(Array<Vec2>* vertices)
{
  uint vertexCount = GetVertexCount();
  vertices->Reserve(vertexCount);
  for(uint i = 0; i < mContours.Size(); ++i)
  {
    Array<Vec2>& polyVerts = mContours[i].mData;
    for(uint j = 0; j < polyVerts.Size(); ++j)
      vertices->PushBack(polyVerts[j]);
  }
}

//Fills out the given array with the size of each contour.
void Shape2D::GetContours(Array<uint>* contours)
{
  contours->Resize(mContours.Size());
  for(uint i = 0; i < mContours.Size(); ++i)
    (*contours)[i] = mContours[i].Size();
}

//Triangulates the mesh and fills out the given array of indices.
void Shape2D::GetTriangleIndices(Array<uint>* indices)
{
  Array<Vec2> vertices;
  Array<uint> contours;
  GetVertices(&vertices);
  GetContours(&contours);
  GetTriangleIndices(vertices, contours, indices);
}

//Triangulates the mesh and fills out the given array of indices.
void Shape2D::GetTriangleIndices(Array<Vec2>& vertices, Array<uint>& contours, 
                                 Array<uint>* indices)
{
  Array<String> errorCodes;
  Validate(errorCodes);

  for(uint i = 0; i < errorCodes.Size(); ++i)
    ErrorIf(true, errorCodes[i].c_str());

  if(!errorCodes.Empty())
    return;

  bool result = Geometry::Triangulate(vertices, contours, indices);
  if(result == false || indices->Size() == 0)
  {
    //ErrorIf(true);
  }
}

//Calculates the barycenter (point average) of the shape.
Vec2 Shape2D::GetBarycenter()
{
  Vec2 center = Vec2::cZero;
  for(uint i = 0; i < mContours.Size(); ++i)
    center += mContours[i].GetBarycenter();
  if(mContours.Size() != 0)
    center /= real(mContours.Size());
  return center;
}

//Constructs the axis-aligned bounding box of the shape.
Aabb Shape2D::GetBoundingBox()
{
  if(mContours.Empty())
    return Aabb();
  return mContours.Front().GetBoundingBox();
}

//Returns whether or not the given point is inside the polygon.
bool Shape2D::ContainsPoint(Vec2Param point)
{
  // Default case
  if(mContours.Empty())
    return false;

  // If the point is not inside the outer contour, it can't be inside at all
  if(!mContours[0].ContainsPoint(point))
    return false;

  // If any inner contour (hole) Contains the point, the shape does 
  // not contain the point
  for(uint i = 1; i < mContours.Size(); ++i)
  {
    if(mContours[i].ContainsPoint(point))
      return false;
  }

  return true;
}

void Shape2D::Calibrate(const float& vCalibration)
{
  for(uint i = 0; i < mContours.Size(); ++i)
  {
    Polygon* sPolygon = &mContours[i];
    Shape2D::Index sIndex;
    sIndex.Contour = i;

    for(uint j = 0; j < sPolygon->Size(); ++j)
    {
      Vec3 sPrev = ToVector3(sPolygon->mData[sPolygon->PrevIndex(j)]);
      Vec3 sCurr = ToVector3(sPolygon->mData[j]);
      Vec3 sNext = ToVector3(sPolygon->mData[sPolygon->NextIndex(j)]);

      Triangle sTriangle(sPrev, sCurr, sNext);

      if(sTriangle.GetArea() < vCalibration)
      {
        sIndex.Vertex = j;

        RemoveVertexAtIndex(sIndex);
      }

      if(sPolygon->Size() <= 2)
      {
        mContours.EraseAt(sIndex.Contour);
        break;
      }

    }
  }
}

bool Shape2D::Validate()
{
  Array<String> errors;
  return Validate(errors);
}

//Validates the Shape and throws an error if it's invalid.
bool Shape2D::Validate(Array<String>& errors)
{
  // The shape is not valid if empty
  if(mContours.Empty())
  {
    errors.PushBack("Shapes cannot be empty.");
    return false;
  }

  // Get the outer contour (the first polygon)
  Polygon* outerContour = &mContours[0];

  // Validate it
  outerContour->Validate(errors);

  // The outer contour must be counter clockwise
  if(outerContour->IsClockwise())
    errors.PushBack("The outer contour must be counter-clockwise.");

  // Validate each inner contour
  for(uint i = 1; i < mContours.Size(); ++i)
  {
    // Get the current contour
    Polygon* currContour = &mContours[i];

    if(currContour->mData.Empty())
    {
      errors.PushBack("Inner contour is empty.");
      continue;
    }

    // Validate it
    currContour->Validate(errors);

    // Inner contours must be clockwise
    if(!currContour->IsClockwise())
      errors.PushBack("Inner contours must be clockwise.");

    // The outer contour must contain the inner contour
    if(!outerContour->ContainsPoint(currContour->mData[0]))
      errors.PushBack("Inner contours must be inside the outer contour.");
  }

  // Check if contours are intersecting
  for(uint i = 0; i < mContours.Size(); ++i)
  {
    Polygon* polyA = &mContours[i];

    for(uint j = i + 1; j < mContours.Size(); ++j)
    {
      Polygon* polyB = &mContours[j];

      // Test if the polygons are intersecting
      ShapeSegResult::Enum result = polyA->Intersects(polyB);

      if(result == ShapeSegResult::Point)
        errors.PushBack("Shapes cannot have shared points on different contours.");
      else if(result == ShapeSegResult::Segment)
        errors.PushBack("Shapes cannot have overlapping edges between different contours.");
    }
  }

  return errors.Empty();
}

Shape2D::Index Shape2D::PickVertex(Vec2Param pos, real vertRadius)
{
  real radiusSquared = vertRadius * vertRadius;

  Index closestIndex;
  real closestDistance = Math::PositiveMax();

  for(uint i = 0; i < mContours.Size(); ++i)
  {
    Polygon& poly = mContours[i];

    for(uint j = 0; j < poly.Size(); ++j)
    {
      // Get the distance
      real distance = LengthSq(pos - poly[j]);
      if(distance < radiusSquared && distance < closestDistance)
      {
        closestDistance = distance;
        closestIndex = Index(i, j);
      }
    }
  }

  return closestIndex;
}

Shape2D::Index Shape2D::PickEdge(Vec2Param pos, real edgeRadius)
{
  real radiusSquared = edgeRadius * edgeRadius;

  Index closestIndex;
  real closestDistance = Math::PositiveMax();

  for(uint i = 0; i < mContours.Size(); ++i)
  {
    Polygon& poly = mContours[i];

    for(uint j = 0; j < poly.Size(); ++j)
    {
      // Get the edge
      Vec2 p0 = poly[j];
      Vec2 p1 = poly[(j + 1) % poly.Size()];

      // Get the distance to the edge
      Vec2 closestPoint = pos;
      Intersection::ClosestPointOnSegmentToPoint(p0, p1, &closestPoint);

      real distance = (closestPoint - pos).LengthSq();
      if(distance < radiusSquared && distance < closestDistance)
      {
        closestDistance = distance;
        closestIndex = Index(i, j);
      }
    }
  }

  return closestIndex;
}

Shape2D::Index Shape2D::AddVertexOnEdge(Index& edgeIndex, Vec2Param vertPos)
{
  // Get the polygon
  Polygon* polygon = &mContours[edgeIndex.Contour];

  // Insert the vertex
  polygon->mData.InsertAt(edgeIndex.Vertex + 1, vertPos);

  return Index(edgeIndex.Contour, edgeIndex.Vertex + 1);
}

bool Shape2D::SlideVertex(Index& index, Vec2Param newPos)
{
  return false;
}

void Shape2D::RemoveVertexAtIndex(Index index)
{
  // Get the polygon
  Polygon* polygon = &mContours[index.Contour];

  // Remove at the index
  polygon->mData.EraseAt(index.Vertex);
}

Vec2* Shape2D::GetVertexAtIndex(Index index)
{
  // Get the polygon
  Polygon* polygon = &mContours[index.Contour];

  // Return the vertex
  return &(*polygon)[index.Vertex];
}

//------------------------------------------------------------------------ Range
Shape2D::range::range(Shape2D* shape)
{
  mShape = shape;
  mContour = 0;
  mVertex = 0;
}

Vec2 Shape2D::range::Front()
{
  // Get the contour
  Polygon* contour = &mShape->mContours[mContour];

  // Return the vertex
  return contour->mData[mVertex];
}

void Shape2D::range::PopFront()
{
  ++mVertex;

  // Get the contour
  Polygon* contour = &mShape->mContours[mContour];

  if(mVertex >= contour->mData.Size())
  {
    ++mContour;
    mVertex = 0;
  }
}

bool Shape2D::range::Empty()
{
  return mContour >= mShape->mContours.Size();
}

size_t Shape2D::range::Length()
{
  return (size_t)mShape->GetVertexCount();
}

Shape2D::Index Shape2D::range::CurrIndex()
{
  return Shape2D::Index(mContour, mVertex);
}

/// Returns a range of all vertices in the shape.
Shape2D::range Shape2D::All()
{
  return range(this);
}

//---------------------------------------------------------------------- Helpers
/// Transforms the shape by the given matrix.
void TransformShape(Mat3Param matrix, Shape2D* shape)
{
  uint contourCount = shape->mContours.Size();

  // Transform each contour
  for(uint i = 0; i < contourCount; ++i)
    TransformPolygon(matrix, &shape->mContours[i]);
}

/// Transforms the shape by the given matrix.
void TransformShape(Mat4Param matrix, Shape2D* shape)
{
  uint contourCount = shape->mContours.Size();

  // Transform each contour
  for(uint i = 0; i < contourCount; ++i)
    TransformPolygon(matrix, &shape->mContours[i]);
}

}//namespace Zero
