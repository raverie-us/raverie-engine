///////////////////////////////////////////////////////////////////////////////
///
/// \file ConvexMeshDecomposition.hpp
///
/// Authors: Joshua Davis
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace ConvexDecomposition
{

DeclareEnum1(ConvexMeshDecompositionMode, Triangulator);

struct SubShape
{
  typedef Array<uint> IndexArray;
  IndexArray mIndices;
};
typedef Array<SubShape> SubShapeArray;

/// Uses the Hertel-Mehlhorn algorithm for combining a triangle mesh into convex shapes.
void Combine2dConvexMeshes(const Array<Vec2>& vertices, SubShapeArray& shapes, SubShapeArray& newShapes);

/// Create 2d meshes with the triangulator then combine them into larger convex pieces.
bool Create2dMeshesWithTriangulator(const Array<Vec2>& vertices, SubShapeArray& meshes);

/// This will create a convex mesh decomposition from vertices.
/// This function will most likely need to be modified to return a new set
/// of points (since other convex mesh algorithms can add points)
bool Create2dMeshes(const Array<Vec2>& vertices, ConvexMeshDecompositionMode::Enum decompositionMode, SubShapeArray& meshes);

}//ConvexDecomposition

}//namespace Zero
