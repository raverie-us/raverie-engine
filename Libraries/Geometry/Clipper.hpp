///////////////////////////////////////////////////////////////////////////////
///
/// \file Clipper.hpp
/// Interface for polygon clipper.
/// 
/// Authors: Killian Koenig
/// Copyright 2013, DigiPen Institute of Technology
///
//////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Public API
namespace Csg
{

DeclareEnum3(Operation, Intersect, Union, Subtract);

struct OperationInput
{
  Operation::Enum Operation;
  const Array< Array<Vec2> >* ContoursA;
  const Array< Array<Vec2> >* ContoursB;

  // Maximum linear distance that two points will be considered the same point
  f32 DistanceTolerance;

  // sin(t) of the maximum angle t that two subsequent edges will be 
  // considered collinear
  f32 CollinearAngleTolerance;

  OperationInput()
    : Operation(Operation::Union)
    , ContoursA(0)
    , ContoursB(0)
    , DistanceTolerance(0.f)
    , CollinearAngleTolerance(0.f)
  {
  }
};

bool Operate(const OperationInput& input, ContourArray* output);


} // namespace Csg

// Only exposed for debugging
namespace Csg
{

namespace Internal
{

typedef double float_t;

struct Vec2_t
{
  float_t x;
  float_t y;

  Vec2_t() {}
  Vec2_t(float_t xx, float_t yy) : x(xx), y(yy) {};
};
typedef Array<Array<Vec2_t> > HighPrecisionContourArray;

struct Simplex
{
  bool operator==(const Simplex& rhs)
  {
    return memcmp(this, &rhs, sizeof(Simplex)) == 0;
  }

  // Non-original edge goes from p1 to p2
  Vec2_t p1;
  Vec2_t p2;

  // The sign of the area of the triangle formed between this
  // edge and the origin
  f32 alpha;
};

// As part of the subdivision we need to maintain an intrusive point list
template <typename T>
struct LinkedVertex
{
  T position;
  Link<LinkedVertex> link;
};

// Given two sets of contours, computes all the intersection points between
// every edge pair and inserts them into the original contours
void Subdivide(Array<Vec2_t>* contourA, Array<Vec2_t>* contourB);
void Subdivide(HighPrecisionContourArray* contoursA, HighPrecisionContourArray* contoursB);

// Converts a set of contours into a simplical chain
void Build(const Array<Vec2_t>& contour, Array<Simplex>* chain);
void Build(const HighPrecisionContourArray& contours, Array<Simplex>* chain);

// Selects valid simplices based on the given Csg operation
void Select(Operation::Enum operation,
            const Array<Simplex>& chainA, 
            const Array<Simplex>& chainB,
            Array<Simplex>* selectedA,
            Array<Simplex>* selectedB);

// Merges two sets of partial simplical chains into a set of contours
bool Merge(const Array<Simplex>& selectedA, 
           const Array<Simplex>& selectedB,
           ContourArray* results);

void Clean(ContourArray* contours, 
           f32 angleTolerance = 0.f,
           f32 distanceTolerance = 0.f);

void Convert(const ContourArray& input,
             HighPrecisionContourArray* output);

} // namespace Internal

} // namespace Csg


} // namespace Zero



