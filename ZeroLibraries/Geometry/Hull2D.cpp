///////////////////////////////////////////////////////////////////////////////
///
/// \file Hull2D.cpp
/// Algorithm to take a point set and generate a convex polygon from it.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Geometry
{

namespace
{
//--------------------------------------------------------- File-Scope Constants
const Vec3      cWorldPlaneNormal = Vec3(real(0.0), real(0.0), real(1.0));
const real      cWorldPlaneOffset = real(0.0);
const real      cPointRadius = real(0.075);
const uint      cInvalidPointId = uint(-1);
const ByteColor cDefaultPointColor = Color::White;
const ByteColor cSelectedPointColor = Color::Red;

//--------------------------------------------------------- File Scope Functions
bool LessThanX(Vec2Param left, Vec2Param right)
{
  return left.x < right.x;
}

real Determinant(Vec2Param pointA, Vec2Param pointB, Vec2Param pointC)
{
  return ((pointB.x - pointA.x) * (pointC.y - pointA.y)) - 
         ((pointC.x - pointA.x) * (pointB.y - pointA.y));
}

//---------------------------------------------------------------- Debug Drawing
void DrawPoint(Vec2Param point, const ByteColor& color)
{
  Vec3 p = Vec3(point.x, point.y, real(0.0));
  //Circle drawn around target point.
  const Vec3 cCircleAxis = cWorldPlaneNormal;
  Zero::gDebugDraw->Add(Zero::Debug::Circle(p, cCircleAxis, 
                                         cPointRadius).Color(color)
                                                      .OnTop(true));

  //Plus sign drawn with intersection at target point.
  const Vec3 cUpDown = Vec3::cYAxis * cPointRadius;
  const Vec3 cLeftRight = Vec3::cXAxis * cPointRadius;
  Vec3 circlePoints[4] = { p + cUpDown,     //Top 
                           p - cUpDown,     //Bottom
                           p - cLeftRight,  //Left
                           p + cLeftRight,  //Right 
                         };
  Zero::gDebugDraw->Add(Zero::Debug::Line(circlePoints[0], 
                                       circlePoints[1]).Color(color)
                                                       .OnTop(true));
  Zero::gDebugDraw->Add(Zero::Debug::Line(circlePoints[2],
                                       circlePoints[3]).Color(color)
                                                       .OnTop(true));
}

void DrawLine(Vec2Param start, Vec2Param end, const ByteColor& color)
{
  Vec3 a = Vec3(start.x, start.y, real(0.0));
  Vec3 b = Vec3(end.x, end.y, real(0.0));
  Zero::gDebugDraw->Add(Zero::Debug::Line(a, b).Color(color).OnTop(true));
}

}// namespace

//----------------------------------------------------------------------- Hull2D
/*
Algorithm ConvexHull('S')
  Sort all points in 'S' based on their position on the X axis
  Designate point 'left' as the leftmost point
  Designate point 'right' as the rightmost point
  Remove 'left' and 'right' from 'S'
  While there are still points in 'S'
    remove 'Point' from 'S'
    if 'Point' is above the line from 'left' to 'right'
      add 'Point' to the end of array 'upper'
    else
      add 'Point' to the end of array 'lower'

Construct the lower hull
  Add 'left' to the 'lower_hull'
  While 'lower' is not empty
    add 'lower[0]' to the end of 'lower_hull'
    remove 'lower[0]' from 'lower'
    while Size('lower_hull') >= 3 and the last 3 points of 'lower_hull' are not convex
      remove the next to last element from 'lower_hull'

Construct the upper hull
  Add 'left' to 'upper_hull'
    While 'upper' is not empty
      add 'upper[0]' to the end of 'upper_hull'
      remove 'upper[0]' from 'upper'
      while Size('upper_hull') >= 3 and the last 3 points of 'upper_hull' are not convex
        remove the next to last element from 'upper_hull'

  Merge 'upper_hull' and 'lower_hull' to form 'hull'
  return 'hull'
*/

Hull2D::Hull2D(void)
{
  mDebugStep = 0;
  mUsingDebug = false;
}

Hull2D::Hull2D(const Vec2Ptr vertices, uint count)
{
  mDebugStep = 0;
}

Hull2D::Hull2D(const Vec3Ptr vertices, uint count)
{
  mDebugStep = 0;
}

Hull2D::~Hull2D(void)
{
  //
}

//Builds the convex hull from the set of 2D points.
void Hull2D::Build(const Vec2Ptr vertices, uint count)
{
  InitializeBuild(count);
  CopyPoints(vertices, count);
  SortPoints();
  ClassifyPoints();
  ComputeNegativeHalfHull();
  ComputePositiveHalfHull();
  MergeHalves();
  SortHullPoints();
}
  
//Builds the convex hull from the set of 3D points. All of the points are 
//assumed to be on a plane and only the x and y components of the vectors are
//considered.
void Hull2D::Build(const Vec3Ptr vertices, uint count)
{
  InitializeBuild(count);
  CopyPoints(vertices, count);
  SortPoints();
  ClassifyPoints();
  ComputeNegativeHalfHull();
  ComputePositiveHalfHull();
  MergeHalves();
  SortHullPoints();
}

uint Hull2D::GetPoints(Vec2Ptr* vertices)
{
  *vertices = &(mPoints[0]);
  return uint(mPoints.Size());
}

void Hull2D::DebugBuildStart(const Vec3Ptr vertices, uint count)
{
  mUsingDebug = true;
  InitializeBuild(count);
  CopyPoints(vertices, count);
}

void Hull2D::DebugBuildStep(void)
{
  if(mUsingDebug == false)
  {
    return;
  }

  switch(mDebugStep)
  {
    //-------------------------------------------------------------- Sort Points
    case 0:
    {
      Zero::Sort(mPoints.All(), LessThanX);
      ++mDebugStep;

      mDebugCounter = 1;  //This is set for the next state
    }
    break;

    //---------------------------------------------------------- Classify Points
    case 1:
    {
      //Create a line from the most negative x-value to the most positive 
      //x-value.
      const Vec2& first = mPoints.Front();
      const Vec2& last = mPoints.Back();

      //Go through all of the points and classify them as either on the positive
      //or negative side of the line.
      uint pointCount = uint(mPoints.Size()) - 1;
      if(mDebugCounter < pointCount)
      {
        const Vec2& point = mPoints[mDebugCounter];
        real det = Determinant(first, point, last);
        if(det < real(0.0))
        {
          mNegativePoints.PushBack(mDebugCounter);
        }
        else
        {
          mPositivePoints.PushBack(mDebugCounter);
        }
        ++mDebugCounter;
      }

      //Once done, go onto the next state
      if(mDebugCounter == pointCount)
      {
        //Add the last point to both arrays.
        mNegativePoints.PushBack(pointCount);
        mPositivePoints.PushBack(pointCount);
        ++mDebugStep;

        mDebugArray.Resize(0);
        mDebugArray.PushBack(0);
        mDebugCounter = 0;
      }
    }
    break;

    //----------------------------------------------- Compute Negative Half Hull
    case 2:
    {
      if(!mNegativePoints.Empty())
      {
        uint pointCount = uint(mNegativePoints.Size());
        if(mDebugCounter < pointCount)
        {
          mDebugArray.PushBack(mNegativePoints[mDebugCounter]);
          //Only remove points if there are enough points to use to check for 
          //convexity and if the last 3 points are NOT convex.
          while(mDebugArray.Size() >= 3 && 
                (ConvexCheck(mDebugArray) > real(0.0)))
          {
            //Remove the next to last element.
            uint pointCount = uint(mDebugArray.Size());
            uint a = pointCount - 1;  //Last element.
            uint b = pointCount - 2;  //Next to last element.
            Math::Swap(mDebugArray[a], mDebugArray[b]);
            mDebugArray.PopBack();
          }
          ++mDebugCounter;
        }

        //Once done, go to the next state
        if(mDebugCounter == pointCount)
        {
          ++mDebugStep;

          mNegativePoints = mDebugArray;
          mDebugArray.Resize(0);
          mDebugArray.PushBack(0);
          mDebugCounter = 0;
        }
      }
      else
      {
        ++mDebugStep;
      }
    }
    break;

    //----------------------------------------------- Compute Positive Half Hull
    case 3:
    {
      if(!mPositivePoints.Empty())
      {
        uint pointCount = uint(mPositivePoints.Size());
        if(mDebugCounter < pointCount)
        {
          mDebugArray.PushBack(mPositivePoints[mDebugCounter]);
          //Only remove points if there are enough points to use the check for 
          //convexity and if the last 3 points are NOT convex.
          while(mDebugArray.Size() >= 3 && 
                (ConvexCheck(mDebugArray) < real(0.0)))
          {
            //Remove the next to last element.
            uint pointCount = uint(mDebugArray.Size());
            uint a = pointCount - 1;  //Last element.
            uint b = pointCount - 2;  //Next to last element.
            Math::Swap(mDebugArray[a], mDebugArray[b]);
            mDebugArray.PopBack();
          }
          ++mDebugCounter;
        }

        //Once done, go to the next state.
        if(mDebugCounter == pointCount)
        {
          ++mDebugStep;

          mPositivePoints = mDebugArray;
          mDebugArray.Resize(0);
          mDebugArray.PushBack(0);
          mDebugCounter = 0;
        }
      }
      else
      {
        ++mDebugStep;
      }
    }
    break;

    //Merge halves
    case 4:
    {
      MergeHalves();
      ++mDebugStep;
    }
    break;
  }
}

void Hull2D::Draw(void) const
{
#ifdef DrawHull2D
  uint pointCount = uint(mPoints.Size());
  for(uint i = 0; i < pointCount; ++i)
  {
    Vec3 thisPoint = Vec3(mPoints[i].x, mPoints[i].y, real(0.0));
    uint j = (i == (pointCount - 1)) ? 0 : i + 1;
    Vec3 nextPoint = Vec3(mPoints[j].x, mPoints[j].y, real(0.0));

    const ByteColor color = Color::Red;
    Zero::gDebugDraw->Add(Zero::Debug::Line(thisPoint, nextPoint).Color(color)
                                                              .OnTop(true));
  }

  if(mUsingDebug)
  {
    const ByteColor negativeColor = Color::Blue;
    const ByteColor positiveColor = Color::Red;

    //Draw line between the first and last points.
    uint pointCount = uint(mPoints.Size()) - 1;
    Vec3 start = Vec3(mPoints[0].x, mPoints[0].y, real(0.0));
    Vec3 end = Vec3(mPoints[pointCount].x, mPoints[pointCount].y, real(0.0));
    Zero::gDebugDraw->Add(Zero::Debug::Line(start, end).Color(Color::Yellow)
                                                    .OnTop(true));

    pointCount = uint(mNegativePoints.Size());
    for(uint i = 0; i < pointCount; ++i)
    {
      DrawPoint(mPoints[mNegativePoints[i]], Color::Blue);
    }
    pointCount = uint(mPositivePoints.Size());
    for(uint i = 0; i < pointCount; ++i)
    {
      DrawPoint(mPoints[mPositivePoints[i]], Color::Red);
    }

    if(mDebugStep == 2 || mDebugStep == 3)
    {
      pointCount = uint(mDebugArray.Size());
      for(uint i = 0; i < (pointCount - 1); ++i)
      {
        const Vec2& thisPoint = mPoints[mDebugArray[i]];
        const Vec2& nextPoint = mPoints[mDebugArray[i + 1]];
        DrawLine(thisPoint, nextPoint, mDebugStep == 2 ? negativeColor
                                                       : positiveColor);
      }
    }
  }
#endif
}

//------------------------------------------------------------------------------
void Hull2D::InitializeBuild(uint count)
{
  mPositivePoints.Clear();
  mNegativePoints.Clear();
  mPoints.Resize(count);
}

void Hull2D::CopyPoints(const Vec2Ptr points, uint count)
{
  //Copy all of the points into a local array.
  for(uint i = 0; i < count; ++i)
  {
    mPoints[i] = points[i];
  }
}

void Hull2D::CopyPoints(const Vec3Ptr points, uint count)
{
  //Copy all of the points into a local array.
  for(uint i = 0; i < count; ++i)
  {
    mPoints[i].Set(points[i].x, points[i].y);
  }
}

void Hull2D::SortPoints(void)
{
  //Sort the data based on the x-axis value of all the points.
  Zero::Sort(mPoints.All(), LessThanX);
}

void Hull2D::ClassifyPoints(void)
{
  //Create a line from the most negative x-value to the most positive x-value.
  const Vec2& first = mPoints.Front();
  const Vec2& last = mPoints.Back();

  //Go through all of the points and classify them as either on the positive or
  //negative side of the line.
  uint pointCount = uint(mPoints.Size()) - 1;
  for(uint i = 1; i < pointCount; ++i)
  {
    const Vec2& point = mPoints[i];
    real det = Determinant(first, point, last);
    if(det < real(0.0))
    {
      mNegativePoints.PushBack(i);
    }
    else
    {
      mPositivePoints.PushBack(i);
    }
  }

  //Add the last point to both arrays.
  mNegativePoints.PushBack(pointCount);
  mPositivePoints.PushBack(pointCount);
}

void Hull2D::ComputePositiveHalfHull(void)
{
  if(mPositivePoints.Empty())
  {
    return;
  }

  IndexArray tempArray;
  tempArray.PushBack(0);

  uint pointCount = uint(mPositivePoints.Size());
  for(uint i = 0; i < pointCount; ++i)
  {
    tempArray.PushBack(mPositivePoints[i]);

    //Only remove points if there are enough points to use to check for 
    //convexity and if the last 3 points are NOT convex.
    while(tempArray.Size() >= 3 && (ConvexCheck(tempArray) <= real(0.0)))
    {
      //Remove the next to last element.
      uint pointCount = uint(tempArray.Size());
      uint a = pointCount - 1;  //Last element.
      uint b = pointCount - 2;  //Next to last element.
      Math::Swap(tempArray[a], tempArray[b]);
      tempArray.PopBack();
    }
  }
  mPositivePoints = tempArray;
}

void Hull2D::ComputeNegativeHalfHull(void)
{
  if(mNegativePoints.Empty())
  {
    return;
  }

  IndexArray tempArray;
  tempArray.PushBack(0);

  uint pointCount = uint(mNegativePoints.Size());
  for(uint i = 0; i < pointCount; ++i)
  {
    tempArray.PushBack(mNegativePoints[i]);

    //Only remove points if there are enough points to use to check for 
    //convexity and if the last 3 points are NOT convex.
    while(tempArray.Size() >= 3 && (ConvexCheck(tempArray) >= real(0.0)))
    {
      //Remove the next to last element.
      uint pointCount = uint(tempArray.Size());
      uint a = pointCount - 1;  //Last element.
      uint b = pointCount - 2;  //Next to last element.
      Math::Swap(tempArray[a], tempArray[b]);
      tempArray.PopBack();
    }
  }
  mNegativePoints = tempArray;
}

void Hull2D::MergeHalves(void)
{
  //Hull Contains the points in both the negative and positive regions, minus
  //the first and last points being duplicated.
  uint hullSize = uint(mNegativePoints.Size() + mPositivePoints.Size()) - 2;
  mHull.Resize(hullSize);

  uint pointCount = uint(mNegativePoints.Size());
  for(uint i = 0; i < pointCount; ++i)
  {
    mHull[i] = mNegativePoints[i];
  }
  
  //Start at the end of the positive points and add them to the hull that way.
  uint hullEnd = pointCount;
  pointCount = uint(mPositivePoints.Size());
  for(uint i = pointCount - 2; i != 0; --i, ++hullEnd)
  {
    mHull[hullEnd] = mPositivePoints[i];
  }
}

void Hull2D::SortHullPoints(void)
{
  const uint pointCount = uint(mHull.Size());
  Vec2Array hullPoints(pointCount);
  for(uint i = 0; i < pointCount; ++i)
  {
    hullPoints[i] = mPoints[mHull[i]];
  }
  mPoints = hullPoints;
}

real Hull2D::ConvexCheck(const IndexArray& points)
{
  uint pointCount = uint(points.Size());
  uint a = points[pointCount - 3];  //Third to last point
  uint b = points[pointCount - 2];  //Second to last point
  uint c = points[pointCount - 1];  //Last point
  return Determinant(mPoints[a], mPoints[b], mPoints[c]);
}

}// namespace Geometry
