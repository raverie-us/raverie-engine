///////////////////////////////////////////////////////////////////////////////
///
/// \file MarchingSquares.cpp
/// 
/// Authors: Joshua Davis
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

MarchingSquares::MarchingSquares()
{
  mDensitySurfaceLevel = 0;
}

void MarchingSquares::Sample(Vec2Param startCoords, Vec2Param endCoords, Vec2Param sampleFrequency, void* userData)
{
  ReturnIf(mDensitySampler == NULL, , "Density sampler callback must be set");
  ReturnIf(mPositionSampler == NULL, , "Position sampler callback must be set");

  //a flag to mark if a cell has "fluid" (the cell has a high enough density to be full)
  const uint blFluid = 1 << 0;
  const uint brFluid = 1 << 1;
  const uint trFluid = 1 << 2;
  const uint tlFluid = 1 << 3;

  for(real y = startCoords.y; y <= endCoords.y; y += sampleFrequency.y)
  {
    for(real x = startCoords.x; x <= endCoords.x; x += sampleFrequency.x)
    {
      Vec2 samplePosition00 = Vec2(real(x +                     0), real(y +                     0));
      Vec2 samplePosition01 = Vec2(real(x +     sampleFrequency.x), real(y +                     0));
      Vec2 samplePosition02 = Vec2(real(x + 2 * sampleFrequency.x), real(y +                     0));
      Vec2 samplePosition10 = Vec2(real(x +                     0), real(y +     sampleFrequency.y));
      Vec2 samplePosition11 = Vec2(real(x +     sampleFrequency.x), real(y +     sampleFrequency.y));
      Vec2 samplePosition12 = Vec2(real(x + 2 * sampleFrequency.x), real(y +     sampleFrequency.y));
      Vec2 samplePosition20 = Vec2(real(x +                     0), real(y + 2 * sampleFrequency.y));
      Vec2 samplePosition21 = Vec2(real(x +     sampleFrequency.x), real(y + 2 * sampleFrequency.y));
      Vec2 samplePosition22 = Vec2(real(x + 2 * sampleFrequency.x), real(y + 2 * sampleFrequency.y));

      real tlDensity = (*mDensitySampler)(samplePosition00, userData);
      real trDensity = (*mDensitySampler)(samplePosition01, userData);
      real blDensity = (*mDensitySampler)(samplePosition10, userData);
      real brDensity = (*mDensitySampler)(samplePosition11, userData);
      real tlValue = tlDensity - mDensitySurfaceLevel;
      real trValue = trDensity - mDensitySurfaceLevel;
      real blValue = blDensity - mDensitySurfaceLevel;
      real brValue = brDensity - mDensitySurfaceLevel;


      uint id = 0;
      if(blValue >= 0)
        id |= blFluid;
      if(brValue >= 0)
        id |= brFluid;
      if(trValue >= 0)
        id |= trFluid;
      if(tlValue >= 0)
        id |= tlFluid;

      Vec2 tlPos = (*mPositionSampler)(samplePosition00, userData);
      Vec2 trPos = (*mPositionSampler)(samplePosition01, userData);
      Vec2 blPos = (*mPositionSampler)(samplePosition10, userData);
      Vec2 brPos = (*mPositionSampler)(samplePosition11, userData);


      switch(id)
      {
        case blFluid:
        {
          SolveSingleEdge(blValue, brValue, tlValue,
            blPos, brPos, tlPos, mSegments);
          break;
        }
        case brFluid:
        {
          SolveSingleEdge(brValue, blValue, trValue,
            brPos, blPos, trPos, mSegments);
          break;
        }
        case trFluid:
        {
          SolveSingleEdge(trValue, tlValue, brValue,
            trPos, tlPos, brPos, mSegments);
          break;
        }
        case tlFluid:
        {
          SolveSingleEdge(tlValue, trValue, blValue,
            tlPos, trPos, blPos, mSegments);
          break;
        }
        case blFluid | brFluid:
        {
          SolveDoubleEdge(blValue, tlValue, brValue, trValue,
            blPos, tlPos, brPos, trPos, mSegments);
          break;
        }
        case tlFluid | trFluid:
        {
          SolveDoubleEdge(brValue, trValue, blValue, tlValue,
            brPos, trPos, blPos, tlPos, mSegments);
          break;
        }
        case blFluid | tlFluid:
        {
          SolveDoubleEdge(blValue, brValue, tlValue, trValue,
            blPos, brPos, tlPos, trPos, mSegments);
          break;
        }
        case brFluid | trFluid:
        {
          SolveDoubleEdge(tlValue, trValue, blValue, brValue,
            tlPos, trPos, blPos, brPos, mSegments);
          break;
        }

        case tlFluid | brFluid:
        {
          real centerVal = (tlValue + trValue + blValue + brValue) * real(0.25);

          if(centerVal < 0)
          {
            SolveSingleEdge(tlValue, trValue, blValue,
              tlPos, trPos, blPos, mSegments);
            SolveSingleEdge(brValue, blValue, trValue,
              brPos, blPos, trPos, mSegments);
          }
          else
          {
            SolveSingleEdge(tlValue, trValue, blValue,
              tlPos, trPos, blPos, mSegments);
            SolveSingleEdge(trValue, tlValue, brValue,
              trPos, tlPos, brPos, mSegments);
          }
          break;
        }
        case blFluid | trFluid:
        {
          real centerVal = (tlValue + trValue + blValue + brValue) * real(0.25);

          if(centerVal < 0)
          {
            SolveSingleEdge(tlValue, trValue, blValue,
              tlPos, trPos, blPos, mSegments);
            SolveSingleEdge(trValue, tlValue, brValue,
              trPos, tlPos, brPos, mSegments);
          }
          else
          {
            SolveSingleEdge(tlValue, trValue, blValue,
              tlPos, trPos, blPos, mSegments);
            SolveSingleEdge(brValue, blValue, trValue,
              brPos, blPos, trPos, mSegments);
          }
          break;
        }
        case trFluid | tlFluid | brFluid:
        {
          SolveSingleEdge(blValue, brValue, tlValue,
            blPos, brPos, tlPos, mSegments);
          break;
        }
        case tlFluid | trFluid | blFluid:
        {
          SolveSingleEdge(brValue, blValue, trValue,
            brPos, blPos, trPos, mSegments);
          break;
        }
        case blFluid | brFluid | tlFluid:
        {             
          SolveSingleEdge(trValue, tlValue, brValue,
            trPos, tlPos, brPos, mSegments);
          break;
        }
        case brFluid | blFluid | trFluid:
        {
          SolveSingleEdge(tlValue, trValue, blValue,
            tlPos, trPos, blPos, mSegments);
          break;
        }
      }
    }
  }
}

void MarchingSquares::SamplePixels(Vec2Param startCoords, Vec2Param endCoords, Vec2Param sampleFrequency, void* userData)
{
  ReturnIf(mDensitySampler == NULL, , "Density sampler callback must be set");
  ReturnIf(mPositionSampler == NULL, , "Position sampler callback must be set");

  //a flag to mark if a cell has "fluid" (the cell has a high enough density to be full)
  const uint blFluid = 1 << 0;
  const uint brFluid = 1 << 1;
  const uint trFluid = 1 << 2;
  const uint tlFluid = 1 << 3;

  for(real y = startCoords.y; y <= endCoords.y; y += sampleFrequency.y)
  {
    for(real x = startCoords.x; x <= endCoords.x; x += sampleFrequency.x)
    {
      Vec2 samplePosition00 = Vec2(real(x +                     0), real(y +                     0));
      Vec2 samplePosition01 = Vec2(real(x +     sampleFrequency.x), real(y +                     0));
      Vec2 samplePosition02 = Vec2(real(x + 2 * sampleFrequency.x), real(y +                     0));
      Vec2 samplePosition10 = Vec2(real(x +                     0), real(y +     sampleFrequency.y));
      Vec2 samplePosition11 = Vec2(real(x +     sampleFrequency.x), real(y +     sampleFrequency.y));
      Vec2 samplePosition12 = Vec2(real(x + 2 * sampleFrequency.x), real(y +     sampleFrequency.y));
      Vec2 samplePosition20 = Vec2(real(x +                     0), real(y + 2 * sampleFrequency.y));
      Vec2 samplePosition21 = Vec2(real(x +     sampleFrequency.x), real(y + 2 * sampleFrequency.y));
      Vec2 samplePosition22 = Vec2(real(x + 2 * sampleFrequency.x), real(y + 2 * sampleFrequency.y));

      real tlDensity = (*mDensitySampler)(samplePosition00, userData);
      real trDensity = (*mDensitySampler)(samplePosition01, userData);
      real blDensity = (*mDensitySampler)(samplePosition10, userData);
      real brDensity = (*mDensitySampler)(samplePosition11, userData);
      real tlValue = tlDensity - mDensitySurfaceLevel;
      real trValue = trDensity - mDensitySurfaceLevel;
      real blValue = blDensity - mDensitySurfaceLevel;
      real brValue = brDensity - mDensitySurfaceLevel;


      uint id = 0;
      if(blValue >= 0)
        id |= blFluid;
      if(brValue >= 0)
        id |= brFluid;
      if(trValue >= 0)
        id |= trFluid;
      if(tlValue >= 0)
        id |= tlFluid;

      Vec2 pos00 = (*mPositionSampler)(samplePosition00, userData);
      Vec2 pos01 = (*mPositionSampler)(samplePosition01, userData);
      Vec2 pos02 = (*mPositionSampler)(samplePosition02, userData);
      Vec2 pos10 = (*mPositionSampler)(samplePosition10, userData);
      Vec2 pos11 = (*mPositionSampler)(samplePosition11, userData);
      Vec2 pos12 = (*mPositionSampler)(samplePosition12, userData);
      Vec2 pos20 = (*mPositionSampler)(samplePosition20, userData);
      Vec2 pos21 = (*mPositionSampler)(samplePosition21, userData);
      Vec2 pos22 = (*mPositionSampler)(samplePosition22, userData);

      switch(id)
      {
        case blFluid:
        {
          mSegments.PushBack(Segment2d(pos10, pos11));
          mSegments.PushBack(Segment2d(pos11, pos21));
          break;
        }
        case brFluid:
        {
          mSegments.PushBack(Segment2d(pos21, pos11));
          mSegments.PushBack(Segment2d(pos11, pos12));
          break;
        }
        case trFluid:
        {
          mSegments.PushBack(Segment2d(pos01, pos11));
          mSegments.PushBack(Segment2d(pos11, pos12));
          break;
        }
        case tlFluid:
        {
          mSegments.PushBack(Segment2d(pos10, pos11));
          mSegments.PushBack(Segment2d(pos11, pos01));
          break;
        }
        case blFluid | brFluid:
        {
          mSegments.PushBack(Segment2d(pos10, pos11));
          mSegments.PushBack(Segment2d(pos11, pos12));
          break;
        }
        case tlFluid | trFluid:
        {
          mSegments.PushBack(Segment2d(pos10, pos11));
          mSegments.PushBack(Segment2d(pos11, pos12));
          break;
        }
        case blFluid | tlFluid:
        {
          mSegments.PushBack(Segment2d(pos01, pos11));
          mSegments.PushBack(Segment2d(pos11, pos21));
          break;
        }
        case brFluid | trFluid:
        {
          mSegments.PushBack(Segment2d(pos21, pos11));
          mSegments.PushBack(Segment2d(pos11, pos01));
          break;
        }

        case tlFluid | brFluid:
        {
          mSegments.PushBack(Segment2d(pos10, pos11));
          mSegments.PushBack(Segment2d(pos11, pos01));
          mSegments.PushBack(Segment2d(pos21, pos11));
          mSegments.PushBack(Segment2d(pos11, pos12));
          break;
        }
        case blFluid | trFluid:
        {
          mSegments.PushBack(Segment2d(pos10, pos11));
          mSegments.PushBack(Segment2d(pos11, pos21));
          mSegments.PushBack(Segment2d(pos12, pos11));
          mSegments.PushBack(Segment2d(pos11, pos01));
          break;
        }
        case trFluid | tlFluid | brFluid:
        {
          mSegments.PushBack(Segment2d(pos10, pos11));
          mSegments.PushBack(Segment2d(pos11, pos21));
          break;
        }
        case tlFluid | trFluid | blFluid:
        {
          mSegments.PushBack(Segment2d(pos21, pos11));
          mSegments.PushBack(Segment2d(pos11, pos12));
          break;
        }
        case blFluid | brFluid | tlFluid:
        {             
          mSegments.PushBack(Segment2d(pos01, pos11));
          mSegments.PushBack(Segment2d(pos11, pos12));
          break;
        }
        case brFluid | blFluid | trFluid:
        {
          mSegments.PushBack(Segment2d(pos10, pos11));
          mSegments.PushBack(Segment2d(pos11, pos01));
          break;
        }
      }
    }
  }
}

void MarchingSquares::BuildAndSimplifyContours(real simplificationThreshold)
{
  RemoveRedundantSegments();
  CreateContours();
  FixWindingOrder();
  SimplifyContours(simplificationThreshold);
}

void MarchingSquares::RemoveRedundantSegments()
{
  for(uint i = 0; i < mSegments.Size(); ++i)
  {
    Segment2d& segment = mSegments[i];
    Vec2 start = segment.first;
    Vec2 end = segment.second;

    //check to see if any other segment is the same as this one, if so remove it
    uint j = i + 1;
    while(j < mSegments.Size())
    {
      if((mSegments[j].first == start && mSegments[j].second == end) || 
        (mSegments[j].second == start && mSegments[j].first == end))
      {
        mSegments.EraseAt(j);
        continue;
      }
      ++j;
    }
  }
}

void MarchingSquares::CreateContours()
{
  //create the first contour
  Contour* contour = &mContours.PushBack();
  //go through every edge
  while(!mSegments.Empty())
  {
    //grab the first edge and try to find an edge that is connected to it
    Segment2d& segment = mSegments[0];
    Vec2 start = segment.first;
    Vec2 end = segment.second;

    //add the second point to the current contour
    contour->PushBack(end);

    //try to find a segment that has the current segment's end point as one of it's edges
    uint j = 1;
    for(j = 1; j < mSegments.Size(); ++j)
    {
      if(mSegments[j].first == end)
        break;

      if(mSegments[j].second == end)
      {
        Math::Swap(mSegments[j].first, mSegments[j].second);
        break;
      }
    }
    //if we only had one edge then fix that edge case
    if(mSegments.Size() == 1)
      j = 0;

    //if we didn't find the corresponding segment then we're starting a new contour
    if(j == mSegments.Size())
    {
      contour = &mContours.PushBack();
      mSegments.EraseAt(0);
      continue;
    }

    //move the next edge to the beginning of the list and then remove the current edge
    Math::Swap(mSegments[0], mSegments[j]);
    mSegments.EraseAt(j);
  }
}

void MarchingSquares::FixWindingOrder()
{
  for(uint i = 0; i < mContours.Size(); ++i)
    FixWindingOrder(mContours[i]);
}

void MarchingSquares::FixWindingOrder(Contour& contour)
{
  //compute the signed area of the contour, this will tell us
  //if it is mostly clockwise or counter-clockwise (Gause-Green's theorem)
  real area = real(0.0);
  for(uint i = 1; i < contour.Size(); ++i)
  {
    Vec2 p0 = contour[i - 1];
    Vec2 p1 = contour[i];

    area += Math::Cross(p0, p1);
  }

  //if the area is negative then the points are defined in a clockwise direction and we don't have to do anything
  if(area < 0)
    return;

  //the contour is backwards, flip the winding order
  for(uint i = 0; i < contour.Size() / 2; ++i)
    Math::Swap(contour[i], contour[contour.Size() - 1 - i]);
}

void MarchingSquares::SimplifyContours(real simplificationThreshold)
{
  for(uint i = 0; i < mContours.Size(); ++i)
    SimplifyContour(mContours[i], simplificationThreshold);
}

void MarchingSquares::SimplifyContour(Contour& contour, real simplificationThreshold)
{
  Contour tempContour;
  //remove co-linear edges
  for(uint i = 0; i < contour.Size(); ++i)
  {
    Vec2 prevPoint = contour[(i + contour.Size() - 1) % contour.Size()];
    Vec2 currPoint = contour[i];
    Vec2 nextPoint = contour[(i + 1) % contour.Size()];
    //have to normalize the edges since we're checking the angle
    Vec2 dir0 = (currPoint - prevPoint).Normalized();
    Vec2 dir1 = (nextPoint - currPoint).Normalized();

    //if the edges are almost parallel then skip this vertex
    if(Math::Abs(Math::Dot(dir0, dir1)) > .99f)
      continue;

    tempContour.PushBack(contour[i]);
  }
  contour = tempContour;
  tempContour.Clear();


  //Iteratively try to remove points that form a triangle with too small of an area.
  //This algorithm continues until no more progress is made.
  uint prevCount = contour.Size() + 1;
  while(prevCount != contour.Size())
  {
    prevCount = contour.Size();
    uint i = 0;
    while(i < contour.Size())
    {
      Vec2 prevPoint = contour[(i + contour.Size() - 1) % contour.Size()];
      Vec2 currPoint = contour[i];
      Vec2 nextPoint = contour[(i + 1) % contour.Size()];
      Vec2 dir0 = (currPoint - prevPoint);
      Vec2 dir1 = (nextPoint - currPoint);

      //compute the area of the triangle
      real crossTerm = Math::Cross(dir0, dir1) * 0.5f;
      //if the edges are clockwise then we don't remove them (because that would cut into the shape),
      //otherwise remove the point if the triangle area is below the given threshold
      if(crossTerm >= real(0) && Math::Abs(crossTerm) < simplificationThreshold)
      {
        contour.EraseAt(i);
        continue;
      }
      ++i;
    }
    tempContour.Clear();
  }
}

Vec2 MarchingSquares::GetPositionOfZero(real val0, real val1, Vec2Param pos0, Vec2Param pos1)
{
  real t = -val0 / (val1 - val0);
  return Math::Lerp(pos0, pos1, t);
}

void MarchingSquares::SolveSingleEdge(real valC, real valX, real valY, Vec2Param posC, Vec2Param posX, Vec2Param posY, Array<Segment2d>& segments)
{
  Vec2 p0 = GetPositionOfZero(valC, valX, posC, posX);
  Vec2 p1 = GetPositionOfZero(valC, valY, posC, posY);
  segments.PushBack(Segment2d(p0, p1));
}

void MarchingSquares::SolveDoubleEdge(real val0Edge0, real val1Edge0, real val0Edge1, real val1Edge1,
  Vec2Param pos0Edge0, Vec2Param pos1Edge0, Vec2Param pos0Edge1, Vec2Param pos1Edge1, Array<Segment2d>& segments)
{
  Vec2 p0 = GetPositionOfZero(val0Edge0, val1Edge0, pos0Edge0, pos1Edge0);
  Vec2 p1 = GetPositionOfZero(val0Edge1, val1Edge1, pos0Edge1, pos1Edge1);
  segments.PushBack(Segment2d(p0, p1));
}

} // namespace Zero
