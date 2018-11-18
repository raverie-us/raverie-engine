///////////////////////////////////////////////////////////////////////////////
///
/// \file MarchingSquares.hpp
/// 
/// Authors: Joshua Davis
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Computes edges/contours from a density field. This class could be greatly optimized!
class MarchingSquares
{
public:
  /// Sample the density of the field at a given grid position
  typedef real (*DensitySamplerFunction)(Vec2Param pos, void* userData);
  /// Sample the position of the field at a given grid position
  typedef Vec2 (*PositionSamplerFunction)(Vec2Param pos, void* userData);

  typedef Pair<Vec2, Vec2> Segment2d;
  typedef Array<Vec2> Contour;

  MarchingSquares();

  /// Sample using traditional marching squares
  void Sample(Vec2Param startCoords, Vec2Param endCoords, Vec2Param sampleFrequency, void* userData);
  /// Sample using a modified marching squares that samples on
  /// the exact pixel boundaries (doesn't round corners)
  void SamplePixels(Vec2Param startCoords, Vec2Param endCoords, Vec2Param sampleFrequency, void* userData);

  /// Builds contours from the results of marching squares and simplifies them.
  /// The threshold is used to remove a point if the area of the triangle defined by [p_i-1, p_i, p_i+1] is below the threshold.
  /// This is most likely the only function that you should call for simplification (as it calls the below functions).
  void BuildAndSimplifyContours(real simplificationThreshold);

  /// The SamplePixels method can create duplicate segments so we first need to strip them.
  void RemoveRedundantSegments();
  /// Turn the edges into contours (by traversing edges to determine what segments are connected).
  void CreateContours();
  /// Make sure the contours are clockwise.
  void FixWindingOrder();
  void FixWindingOrder(Contour& contour);
  /// Remove harsh edges on the contours. Only remove a point if the
  /// edge will not cut into the interior (only expand outwards).
  void SimplifyContours(real simplificationThreshold);
  void SimplifyContour(Contour& contour, real simplificationThreshold);

private:
  Vec2 GetPositionOfZero(real val0, real val1, Vec2Param pos0, Vec2Param pos1);

  void SolveSingleEdge(real valC, real valX, real valY, Vec2Param posC, Vec2Param posX, Vec2Param posY, Array<Segment2d>& segments);

  void SolveDoubleEdge(real val0Edge0, real val1Edge0, real val0Edge1, real val1Edge1,
    Vec2Param pos0Edge0, Vec2Param pos1Edge0, Vec2Param pos0Edge1, Vec2Param pos1Edge1, Array<Segment2d>& segments);

public:
  DensitySamplerFunction mDensitySampler;
  PositionSamplerFunction mPositionSampler;
  /// Where the surface should be built at
  real mDensitySurfaceLevel;

  /// These segments will be invalid after simplification.
  Array<Segment2d> mSegments;
  /// The resulting contours (if simplification is done).
  Array<Contour> mContours;
};

} // namespace Zero
