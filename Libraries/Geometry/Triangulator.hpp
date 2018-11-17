///////////////////////////////////////////////////////////////////////////////
///
/// \file Triangulator.hpp
/// Interface for Seidel's triangulation algorithm.
/// 
/// Authors: Killian Koenig
/// Copyright 2013, DigiPen Institute of Technology
///
//////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Geometry
{

// Implementation of Seidel's incremental randomized triangulation algorithm
// Described in detail here:
//    http://www.ime.usp.br/~walterfm/cursos/mac0331/2006/seidel.pdf
//    http://sigbjorn.vik.name/projects/Triangulation.pdf

// The high-level algorithm can be described in 3 steps
//    1) Construct a trapezoid map data structure based on the input shape
//    2) Use the trapezoid map to generate lists of monotone mountains that
//       partition the original shape
//    3) Triangulate the monotone mountains

// Preconditions
// In its current state the algorithm cannot correctly handle the following:
//    - shared vertices between contours
//    - self intersecting contours
//    - duplicate but non contiguous vertices in the same contour
//    - collinear points
// In the event that it detects any error it will return immediately

// Postconditions
// No checks are done to verify that resulting triangles are non-degenerate
// It should be possible to get back 0 area triangles, but that is left
// up to the user to decide what to do

// Note:
// The ordering of the contours doesn't matter, ie the outer contour does
// not have to be first
// There can be multiple outer and inner contours

// vertices - the vertices from each contour lined up in one
//            contiguous array
// contourSizes - array of sizes of each contour in the order they
//              - lie in the vertex array
// indices - resulting index buffer

DeclareEnum4(FillRule,  NonZeroWinding, EvenOddWinding, All, PositiveWinding);

bool Triangulate(const Zero::Array<Vec2>& vertices,
                 const Zero::Array<uint>& contourSizes,
                 Zero::Array<uint>* indices,
                 FillRule::Type rule = FillRule::EvenOddWinding);

// Wrapper for easy of use on single contour shapes
bool Triangulate(const Zero::Array<Vec2>& vertices, 
                 Zero::Array<uint>* indices);

////////////////////////////////////////////////////////////////////////////////
//  
//  Internal (for debugging)
//
////////////////////////////////////////////////////////////////////////////////  

bool BuildSet(const Zero::TrapezoidMap& map, Zero::Array<uint>* indices, s32 seed, FillRule::Type rule);

} // Geometry
