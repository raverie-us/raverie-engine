///////////////////////////////////////////////////////////////////////////////
///
/// \file ConvexMeshDecomposition.cpp
///
/// Authors: Joshua Davis
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace ConvexDecomposition
{

struct ConvexShape;
/// An edge between two points in the outer contour. Used for combining meshes together.
struct Edge
{
  uint mPoint0Index;
  uint mPoint1Index;
  /// The other edge that is between these two points (if there is one).
  Edge* mAdjacentEdge;
  /// What shape this edge belongs to.
  ConvexShape* mShape;

  Link<Edge> shapeLink;
  Link<Edge> internalEdgeLink;
};

/// A convex shape composed of several edges.
struct ConvexShape
{
  typedef InList<Edge, &Edge::shapeLink> EdgeList;
  EdgeList mEdges;

  Link<ConvexShape> link;
};

/// Policy to hash edges based upon what two points they are connected between.
struct EdgeHashingPolicy
{
  size_t operator()(const Edge* value) const
  {
    //multiply the larger index by a large prime to help distribute the hashing function
    uint value0 = value->mPoint0Index;
    uint value1 = value->mPoint1Index;
    if(value0 > value1)
      Math::Swap(value0, value1);

    return (size_t)(value0 ^ (value1 * LargePrime));
  }

  inline bool Equal(const Edge* left, const Edge* right) const
  {
    //the edges are equal if they are between the same two points (even if the order is flipped)
    return (left->mPoint0Index == right->mPoint0Index && left->mPoint1Index == right->mPoint1Index) ||
           (left->mPoint0Index == right->mPoint1Index && left->mPoint1Index == right->mPoint0Index);
  }

  static const uint LargePrime = 75372313;
};

/// Sorter for edges based upon the length of the edge. This will put longer
//edges in front (which seems to be a good heuristic for the combining order)
struct EdgeGreaterThanSorter
{
  EdgeGreaterThanSorter(const Array<Vec2>* vertices)
  {
    mVertices = vertices;
  }

  bool operator()(const Edge& lhs, const Edge& rhs)
  {
    float lengthLhs = Math::Length((*mVertices)[lhs.mPoint1Index] - (*mVertices)[lhs.mPoint0Index]);
    float lengthRhs = Math::Length((*mVertices)[rhs.mPoint1Index] - (*mVertices)[rhs.mPoint0Index]);

    return lengthLhs > lengthRhs;
  }

  const Array<Vec2>* mVertices;
};

void Combine2dConvexMeshes(const Array<Vec2>& vertices, SubShapeArray& shapes, SubShapeArray& newShapes)
{
  //count how many edges and shapes we have (so we can allocate all the memory we need up front)
  uint shapeCount = shapes.Size();
  SubShapeArray::range subShapes = shapes.All();
  uint edgeCount = 0;
  for(; !subShapes.Empty(); subShapes.PopFront())
  {
    SubShape& subShape = subShapes.Front();
    edgeCount += subShape.mIndices.Size();
  }

  //create a temporary memory pool for the edges and
  //shapes so we can clean up all the memory once we're done
  Memory::Pool edgePool("", nullptr, sizeof(Edge), edgeCount, true);
  Memory::Pool shapePool("", nullptr, sizeof(ConvexShape), shapeCount, true);
  
  //we need to store the resultant convex shapes
  typedef InList<ConvexShape> ConvexShapeList;
  ConvexShapeList convexShapes;
  //it will only require one iteration through all internal edges to get a decomposition
  typedef InList<Edge, &Edge::internalEdgeLink> InternalEdgeList;
  InternalEdgeList internalEdges;
  //use a map for now to find edge pairs (any internal edge will have a pair edge)
  HashSet<Edge*, EdgeHashingPolicy> edgeMap;

  uint vertexCount = vertices.Size();
  //The first step is to build each shape and determine which edges are internal.
  //During this step we'll also find the mirror edge for each internal edge.
  subShapes = shapes.All();
  for(; !subShapes.Empty(); subShapes.PopFront())
  {
    SubShape& subShape = subShapes.Front();
    ConvexShape* convexShape = shapePool.AllocateType<ConvexShape>();
    convexShapes.PushBack(convexShape);

    //iterate through all the edges of this shape
    uint indexCount = subShape.mIndices.Size();
    for(uint i = 0; i < indexCount; ++i)
    {
      //create the edge and add it to this shape
      Edge* edge = edgePool.AllocateType<Edge>();
      edge->mPoint0Index = subShape.mIndices[i];
      edge->mPoint1Index = subShape.mIndices[(i + 1) % indexCount];
      edge->mAdjacentEdge = nullptr;
      edge->mShape = convexShape;
      convexShape->mEdges.PushBack(edge);

      //Determine if this edge is an internal edge. If the next second
      //point on this edge is either the next or previous point in the original
      //vertex list then the point is on the outer contour.
      uint nextVertex = (edge->mPoint0Index + 1) % vertexCount;
      uint prevVertex = (edge->mPoint0Index + vertexCount - 1) % vertexCount;
      if(edge->mPoint1Index == nextVertex || edge->mPoint1Index == prevVertex)
        continue;

      //Try to find the adjacent edge. If we find it we need to hook up the adjacency for
      //each edge but we don't need to add it to the internal edges list because we handle
      //the internal edges in pairs so we only need one of them in there. Also, there is no
      //need to add it to the map because there is only 1 pair for an edge.
      Edge* adjacentEdge = edgeMap.FindValue(edge, nullptr);
      if(adjacentEdge != nullptr)
      {
        adjacentEdge->mAdjacentEdge = edge;
        edge->mAdjacentEdge = adjacentEdge;
        continue;
      }

      //otherwise we haven't already found the adjacent edge so
      //add this edge to the map and edge list
      edgeMap.Insert(edge);
      internalEdges.PushBack(edge);
    }
  }

  //a good heuristic seems to be to combine the longer edges first
  EdgeGreaterThanSorter sorter(&vertices);
  internalEdges.Sort(sorter);

  //Now iterate through all of the internal edges and determine whether or not
  //it is "essential". The edge is essential if removing it will make the combined meshes concave.
  //This can be determined locally by checking the angles of the adjacent edges.
  while(!internalEdges.Empty())
  {
    Edge& edgeA = internalEdges.Front();
    internalEdges.PopFront();

    //a safeguard, should never happen unless the passed in mesh has problems (such as duplicate vertices)
    if(edgeA.mAdjacentEdge == nullptr)
      continue;

    Edge& edgeB = *edgeA.mAdjacentEdge;
    
    //get the edges before and after the edge we're trying to remove on both shapes
    Edge* prevEdgeA = edgeA.mShape->mEdges.PrevWrap(&edgeA);
    Edge* nextEdgeA = edgeA.mShape->mEdges.NextWrap(&edgeA);
    Edge* prevEdgeB = edgeB.mShape->mEdges.PrevWrap(&edgeB);
    Edge* nextEdgeB = edgeB.mShape->mEdges.NextWrap(&edgeB);

    //check the edges off of point0 on edgeA (just calling this the top)
    //to determine if they would be concave or convex
    Vec2 pTop0 = vertices[prevEdgeA->mPoint0Index];
    Vec2 pTop1 = vertices[prevEdgeA->mPoint1Index];
    Vec2 pTop2 = vertices[nextEdgeB->mPoint1Index];
    Vec2 pTopEdge0 = pTop1 - pTop0;
    Vec2 pTopEdge1 = pTop2 - pTop1;
    //use the sign of the z axis (the result of the 2d cross product)
    //to determine if this edge is convex or concave
    if(Math::Cross(pTopEdge0, pTopEdge1) < 0)
      continue;

    //do the same for the edges off of point1 on edge A (the "bottom")
    Vec2 pBot0 = vertices[prevEdgeB->mPoint0Index];
    Vec2 pBot1 = vertices[prevEdgeB->mPoint1Index];
    Vec2 pBot2 = vertices[nextEdgeA->mPoint1Index];
    Vec2 pBotEdge0 = pBot1 - pBot0;
    Vec2 pBotEdge1 = pBot2 - pBot1;
    if(Math::Cross(pBotEdge0, pBotEdge1) < 0)
      continue;

    //shapeA and shapeB will be convex when the edge between
    //them is removed so merge shapeB into shapeA
    ConvexShape* shapeB = nextEdgeB->mShape;
    //iterate through all edges in shape b from nextEdgeB through
    //prevEdgeB (so all edges except edgeB, the one we're removing)
    Edge* edgeToInsert = nextEdgeB;
    while(edgeToInsert != &edgeB)
    {
      //get the next edge (making sure to skip the sentinel)
      Edge* nextEdge = shapeB->mEdges.NextWrap(edgeToInsert);

      //remove the edge from shapeB and add it to shapeA before
      //nextEdgeA (to maintain the appropriate ordering of the edges)
      shapeB->mEdges.Erase(edgeToInsert);
      edgeToInsert->mShape = edgeA.mShape;
      edgeToInsert->mShape->mEdges.InsertBefore(nextEdgeA, edgeToInsert);
      edgeToInsert = nextEdge;
    }

    //remove the edge from both shapes
    edgeA.mShape->mEdges.Erase(&edgeA);
    edgeB.mShape->mEdges.Erase(&edgeB);
  }

  //now we just have to populate the new set of shapes
  ConvexShapeList::range convexShapeRange = convexShapes.All();
  for(; !convexShapeRange.Empty(); convexShapeRange.PopFront())
  {
    ConvexShape& shape = convexShapeRange.Front();
    //if this shape is empty (because we merged it with another shape) then ignore it
    if(shape.mEdges.Empty())
      continue;

    SubShape& subShape = newShapes.PushBack();

    //copy over all the indices
    ConvexShape::EdgeList::range edges = shape.mEdges.All();
    for(; !edges.Empty(); edges.PopFront())
      subShape.mIndices.PushBack(edges.Front().mPoint1Index);
  }
}

void BuildShapesFromTriangulation(const Array<Vec2>& vertices, Array<uint>& indices, SubShapeArray& shapes)
{
  shapes.Resize(indices.Size() / 3);
  for(uint i = 0; i < indices.Size(); i += 3)
  {
    uint shapeIndex = i / 3;
    SubShape& subShape = shapes[shapeIndex];

    subShape.mIndices.Resize(3);
    subShape.mIndices[0] = indices[i];
    subShape.mIndices[1] = indices[i + 1];
    subShape.mIndices[2] = indices[i + 2];
    
    //make sure the triangle is counter clockwise
    Vec2 p0 = vertices[subShape.mIndices[0]];
    Vec2 p1 = vertices[subShape.mIndices[1]];
    Vec2 p2 = vertices[subShape.mIndices[2]];
    //if they aren't then swap two indices
    if(Math::Cross(p1 - p0, p2 - p1) < 0)
      Math::Swap(subShape.mIndices[0], subShape.mIndices[1]);
  }
}

bool Create2dMeshesWithTriangulator(const Array<Vec2>& vertices, SubShapeArray& meshes)
{
  Array<uint> indices;
  if(Geometry::Triangulate(vertices, &indices) == false)
    return false;

  //convert the triangulation format to a format for combining the
  //meshes (and make sure the winding order is correct)
  SubShapeArray shapes;
  BuildShapesFromTriangulation(vertices, indices, shapes);

  //combine the meshes
  Combine2dConvexMeshes(vertices, shapes, meshes);

  return true;
}

bool Create2dMeshes(const Array<Vec2>& vertices, ConvexMeshDecompositionMode::Enum decompositionMode, SubShapeArray& meshes)
{
  if(decompositionMode == ConvexMeshDecompositionMode::Triangulator)
    return Create2dMeshesWithTriangulator(vertices, meshes);

  return true;
}

}//ConvexDecomposition

}//namespace Zero
