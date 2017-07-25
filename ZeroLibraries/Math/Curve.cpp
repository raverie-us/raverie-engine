///////////////////////////////////////////////////////////////////////////////
///
/// \file Curve.cpp
/// Implementation of the Curve class.
///
/// Authors: Joshua Davis
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{

static const Mat4 CatmulBasis = Mat4(Vec4(0,-1, 2,-1) / real(2.0),
                                      Vec4(2, 0,-5, 3) / real(2.0),
                                      Vec4(0, 1, 4,-3) / real(2.0),
                                      Vec4(0, 0,-1, 1) / real(2.0));

static const Mat4 BSplineBasis = Mat4(Vec4(-1, 3,-3, 1) / real(6.0),
                                      Vec4( 3,-6, 0, 4) / real(6.0),
                                      Vec4(-3, 3, 3, 1) / real(6.0),
                                      Vec4( 1, 0, 0, 0) / real(6.0));

//------------------------------------------------------------------------ Curve
SplineCurve::SplineCurve()
{
  mCurveType = CurveType::CatmulRom;
  mClosed = false;
}

void SplineCurve::AddControlPoint(Vec3Param controlPoint)
{
  ControlPoints.PushBack(controlPoint);
}

void SplineCurve::RemovePointAtIndex(uint index)
{
  if(index >= ControlPoints.Size())
    return;

  ControlPoints.EraseAt(index);
}

void SplineCurve::AddControlPoints(const Vec3Array& controlPoints)
{
  ControlPoints.Insert(ControlPoints.End(), controlPoints.Begin(), controlPoints.End());
}

void SplineCurve::SetControlPoints(const Vec3Array& controlPoints)
{
  ControlPoints = controlPoints;
}

void SplineCurve::GetPoints(Vec3Array& results, uint resolution) const
{
  if(resolution == 0)
    return;

  Vec3Array points;
  GetSmoothPoints(points);
  if(points.Size() < 4)
  {
    results = points;
    return;
  }

  //generate the points for the given curve type
  uint curveType = mCurveType;
  if(curveType == CurveType::Linear)
    results = points;
  else if(curveType == CurveType::BSpline)
    GetPoints<BSplinePolicy>(points,results,resolution);
  else if(curveType == CurveType::CatmulRom)
    GetPoints<CatmulRomPolicy>(points,results,resolution);
}

void SplineCurve::BakeAdaptive(Vec3Array& results, real error) const
{
  Vec3Array points;
  GetSmoothPoints(points);
  if(points.Size() < 4)
  {
    results = points;
    return;
  }

  //generate the points for the given curve type
  uint curveType = mCurveType;
  if(curveType == CurveType::Linear)
    results = points;
  if(curveType == CurveType::BSpline)
    GetPoints<BSplinePolicy>(points,results,error);
  else if(curveType == CurveType::CatmulRom)
    GetPoints<CatmulRomPolicy>(points,results,error);
}

Vec3Array& SplineCurve::GetControlPoints()
{
  return ControlPoints;
}

void SplineCurve::Clear(void)
{
  ControlPoints.Clear();
}

bool SplineCurve::GetClosed()
{
  return mClosed;
}

void SplineCurve::SetClosed(bool state)
{
  mClosed = state;
}

CurveType::Enum SplineCurve::GetCurveType()
{
  return mCurveType;
}

void SplineCurve::SetCurveType(CurveType::Enum curveType)
{
  mCurveType = curveType;
}

bool SplineCurve::DistanceSq(Vec3 point, uint resolution, real& distSq) const
{
  Vec3Array points;
  GetPoints(points, resolution);

  if (points.Size() < 2)
    return false;

  real minDistSq1 = Math::PositiveMax(); // smallest distance
  real minDistSq2 = Math::PositiveMax(); // second smallest distance
  int idx1 = 0; // index of the point with the smallest distance
  int idx2 = 0; // index of the point with the second smallest distance

  for (uint i = 0; i < points.Size(); ++i)
  {
    float currDistSq = (points[i] - point).LengthSq();
    if (currDistSq < minDistSq2)
    {
      minDistSq2 = currDistSq;
      idx2 = i;
    }
    if (currDistSq < minDistSq1)
    {
      minDistSq2 = minDistSq1;
      idx2 = idx1;
      minDistSq1 = currDistSq;
      idx1 = i;
    }
  }

  if (idx1 == 0 && idx2 == 0)
    return false;

  distSq = Math::DistanceToLineSq(points[idx1], points[idx2], point);
  return true;
}

void SplineCurve::GetSmoothPoints(Vec3Array& pts) const
{
  pts = ControlPoints;
  //if we have no points, just do nothing...
  if(pts.Size() == 0)
    return;

  //if we have less than 4 points, we can't make a curve,
  //so just switch to linear mode
  uint curveType = mCurveType;
  if(pts.Size() < 3)
    curveType = CurveType::Linear;

  //if we are in linear mode, just push back the control points
  //(and take into account the curve being closed)
  if(curveType == CurveType::Linear)
  {
    if(mClosed)
      pts.PushBack(ControlPoints[0]);
    return;
  }

  //Fix the curve to be either closed or continuous
  if(mClosed)
    MakeClosed(pts);
  else
  {
    //deal with only having 3 control points by pushing the last point on again
    //don't need to do this when the curve is closed though.
    if(pts.Size() == 3)
    {
      Vec3 point = pts.Back();
      pts.PushBack(point);
    }
    MakeContinuous(pts);
  }
}

void SplineCurve::MakeContinuous(Vec3Array& points) const
{
  //to make a curve continuous, we need an extra control point that keeps
  //the same direction. Therefore, compute a new point at both the
  //beginning and end that keeps the correct direction.
  Vec3 ab = points[0] - points[1];
  points.Insert(points.Begin(),points[0] + ab);

  uint size = points.Size();
  Vec3 cd = points[size - 1] - points[size - 2];
  points.PushBack(points[size - 1] + cd);
}

void SplineCurve::MakeClosed(Vec3Array& points) const
{
  //to make a curve closed, we have to duplicate 2 points at the other end
  //of the curve
  Vec3 end = points[points.Size() - 1];
  Vec3 secondEnd = points[points.Size() - 2];
  points.Insert(points.Begin(),end);
  points.Insert(points.Begin(),secondEnd);

  points.PushBack(points[2]);
  points.PushBack(points[3]);
}

template <typename Policy>
void SplineCurve::GetPoints(const Vec3Array& points, Vec3Array& results, uint resolution) const
{
  real t = real(0.0);
  uint start = 0;
  uint end = resolution;
  uint countControlPoints = uint(points.Size() - 3);

  real step = real(1.0) / static_cast<real>(resolution);

  for(uint i = mClosed ? 1 : 0; i < countControlPoints; ++i)
  {
    t = real(0.0);
    end = resolution;

    if(i == countControlPoints - 1)
      ++end;

    Vec3Param cp0 = points[i];
    Vec3Param cp1 = points[i + 1];
    Vec3Param cp2 = points[i + 2];
    Vec3Param cp3 = points[i + 3];

    for(uint j = start; j < end; ++j)
    {
      PointData data = ComputePointData<Policy>(t,cp0,cp1,cp2,cp3);
      Vec3 point = data.Point;
      results.PushBack(point);
      t += step;
    }
  }
}

template <typename Policy>
void SplineCurve::GetPoints(const Vec3Array& points, Vec3Array& results, real error) const
{
  uint countControlPoints = uint(points.Size() - 3);

  Vec3 veryLastPoint;
  for(uint i = mClosed ? 1 : 0; i < countControlPoints; ++i)
  {
    Vec3Param cp0 = points[i];
    Vec3Param cp1 = points[i + 1];
    Vec3Param cp2 = points[i + 2];
    Vec3Param cp3 = points[i + 3];

    //add the start, mid and last point to the stack (need the middle point
    //since the spline is cubic, this "approximates" each sub-section as a quadratic)
    Zero::Array<PointData> stack;
    PointData firstPoint = ComputePointData<Policy>(real(0.0), cp0, cp1, cp2, cp3);
    PointData centerPoint = ComputePointData<Policy>(real(0.5), cp0, cp1, cp2, cp3);
    PointData lastPoint = ComputePointData<Policy>(real(1.0), cp0, cp1, cp2, cp3);
    stack.PushBack(lastPoint);
    stack.PushBack(centerPoint);
    stack.PushBack(firstPoint);
    veryLastPoint = lastPoint.Point;
    
    while(stack.Size() != 1)
    {
      uint size = stack.Size();

      PointData data0 = stack[size - 1];
      PointData data1 = stack[size - 2];

      Vec3 movement = data1.Point - data0.Point;
      //calculate the point half-way in-between the two points on the stack
      real midT = (data0.T + data1.T) * real(0.5);
      PointData midData = ComputePointData<Policy>(midT, cp0, cp1, cp2, cp3);
      Vec3 midPoint = midData.Point;

      //calculate the distance of this point from the line
      //(aka, calculate the height of the triangle)
      real doubleArea = (Math::Cross(midPoint - data0.Point, midPoint - data1.Point)).Length();
      real area = doubleArea / 2;

      //if the area of the triangle is too large then we need to
      //subdivide more to get a better approximation of the curve
      if(area > error)
      {
        stack[size - 1] = midData;
        stack.PushBack(data0);
      }
      else
      {
        results.PushBack(data0.Point);
        results.PushBack(midPoint);
        stack.PopBack();
      }
    }
  }
  results.PushBack(veryLastPoint);
}

template <typename Policy>
Vec3 SplineCurve::ComputePoint(real t, Vec3Param a, Vec3Param b, Vec3Param c, Vec3Param d) const
{
  Vec4 param = Policy::GetParam(t);
  const Mat4& basis = Policy::GetBasis();

  Vec4 x(a.x,b.x,c.x,d.x);
  x = Math::Transform(basis,x);

  Vec4 y(a.y,b.y,c.y,d.y);
  y = Math::Transform(basis,y);

  Vec4 z(a.z,b.z,c.z,d.z);
  z = Math::Transform(basis,z);

  return Vec3(Math::Dot(param,x),Math::Dot(param,y),Math::Dot(param,z));
}

template <typename Policy>
SplineCurve::PointData SplineCurve::ComputePointData(real t, Vec3Param a, Vec3Param b, Vec3Param c, Vec3Param d) const
{
  PointData data;

  data.Point = ComputePoint<Policy>(t, a, b, c, d);
  data.T = t;
  return data;
}

//-------------------------------------------------------------- B-Spline Policy
const Mat4& SplineCurve::BSplinePolicy::GetBasis()
{
  return BSplineBasis;
}

Vec4 SplineCurve::BSplinePolicy::GetParam(real t)
{
  return Vec4(t*t*t,t*t,t,1);
}

//----------------------------------------------------------- Catmul-Rom Policy
const Mat4& SplineCurve::CatmulRomPolicy::GetBasis()
{
  return CatmulBasis;
}

Vec4 SplineCurve::CatmulRomPolicy::GetParam(real t)
{
  return Vec4(1,t,t*t,t*t*t);
}

//-------------------------------------------------------------------BakedCurve
void BakedCurve::Bake(const SplineCurve& curve, real error)
{
  //get all of the baked points from the curve (right now allocate a
  //temp array for this, maybe figure something better out later)
  Vec3Array pts;
  curve.BakeAdaptive(pts, error);
  if(pts.Size() == 0)
    return;

  float totalLength = 0.0f;
  
  mArcLengthTable.Resize(pts.Size());
  //the 1st point is at length 0, 
  mArcLengthTable[0].ArcLength = 0.0f;
  mArcLengthTable[0].Position = pts[0];

  //now compute the rest of the points
  for(uint i = 1; i < pts.Size(); ++i)
  {
    Vec3 oldPos = pts[i - 1];
    Vec3 curPos = pts[i];

    //get the vector from the previous point to the current point
    Vec3 dir = curPos - oldPos;
    //now increase the total arc length by this distance vector
    float length = dir.Length();
    totalLength += length;

    //make sure to set the position and arc length of this point
    mArcLengthTable[i].ArcLength = totalLength;
    mArcLengthTable[i].Position = pts[i];
  }
}

uint BakedCurve::Size() const
{
  return mArcLengthTable.Size();
}

real BakedCurve::GetTotalArcLength() const
{
  uint tableSize = Size();
  if(tableSize == 0)
    return real(0.0);

  return mArcLengthTable[tableSize - 1].ArcLength;
}

BakedCurve::BakedData BakedCurve::GetPoint(uint index)
{
  if(index >= mArcLengthTable.Size())
    return BakedData();

  return mArcLengthTable[index];
}

void BakedCurve::SetPoint(uint index, Vec3Param pos)
{
  if(index < mArcLengthTable.Size())
    mArcLengthTable[index].Position = pos;
}

Vec3 BakedCurve::SampleTable(float distance, Vec3* tangent) const
{
  // Handle the two special cases where we can't interpolate between two points
  if(mArcLengthTable.Size() == 0)
    return Vec3::cZero;
  if(mArcLengthTable.Size() == 1)
    return mArcLengthTable[0].Position;
  // There is no curve (the only points are on top of each other)
  if(GetTotalArcLength() == real(0.0))
    return mArcLengthTable[0].Position;

  real oldDistance = distance;
  distance = Math::FMod(distance, GetTotalArcLength());
  //If the user passes in the total arc-length, fmod will return the start point
  //not the end point. To fix this if fmod returns 0 but our original value
  //wasn't zero then instead use the total arc-length.
  if(distance == real(0.0) && oldDistance != real(0.0))
    distance = GetTotalArcLength();
  //fmod can still return negative numbers so convert to positive
  if(distance < real(0.0))
    distance += GetTotalArcLength();

  //get the indices of the two baked point we're between
  uint lowerBound = SampleLowerBound(distance);
  uint upperBound = lowerBound + 1;

  const BakedData& lowerBoundData = mArcLengthTable[lowerBound];
  const BakedData& upperBoundData = mArcLengthTable[upperBound];

  float d0 = lowerBoundData.ArcLength;
  float d1 = upperBoundData.ArcLength;
  //compute how far in-between these two points we are
  float t = (distance - d0) / (d1 - d0);

  Vec3 p0 = lowerBoundData.Position;
  Vec3 p1 = upperBoundData.Position;
  Vec3 samplePoint = Math::Lerp(p0, p1, t);

  //if the user wants a tangent, compute it simply as the vector from the first to
  //second point, where we are on this linear segment doesn't matter
  if(tangent != nullptr)
  {
    Vec3 tangentDir = p1 - p0;
    *tangent = tangentDir.AttemptNormalized();
  }

  return samplePoint;
}

Vec3 BakedCurve::SampleFunction(float x, bool clamp) const
{
  // Handle the two special cases where we can't interpolate between two points
  if(mArcLengthTable.Size() == 0)
    return Vec3::cZero;
  if(mArcLengthTable.Size() == 1)
    return mArcLengthTable[0].Position;
  // There is no curve (the only points are on top of each other)
  if(GetTotalArcLength() == real(0.0))
    return mArcLengthTable[0].Position;

  // Find nearest two points straddling the desired x value
  const BakedData* pMin = (mArcLengthTable.Data() + 0);
  const BakedData* pMax = (mArcLengthTable.Data() + (mArcLengthTable.Size() - 1));
  for(size_t i = 0; i < mArcLengthTable.Size(); ++i)
  {
    // Get point
    const BakedData* point = (mArcLengthTable.Data() + i);

    // Point is less than x?
    if(point->Position.x < x)
    {
      // Is a closer minimum point?
      if(point->Position.x > pMin->Position.x)
        pMin = point;
    }
    // Point is greater than x?
    else if(point->Position.x > x)
    {
      // Is a closer maximum point?
      if(point->Position.x < pMax->Position.x)
        pMax = point;
    }
    // Point is equal to x?
    else
    {
      // Return exact point
      return point->Position;
    }
  }

  // Compute the interpolant between the nearest two points
  float t = InverseLerp(x, pMin->Position.x, pMax->Position.x);

  // Clamp interpolant to remain within the curve?
  if(clamp)
    t = Clamp(t);

  // Interpolate along the two nearest points
  Vec3 result;
  result.x = x;
  result.y = Lerp(pMin->Position.y, pMax->Position.y, t);
  result.z = Lerp(pMin->Position.z, pMax->Position.z, t);

  // Return interpolated point
  return result;
}

uint BakedCurve::SampleLowerBound(real distance) const
{
  //binary search for the lower bound
  uint begin = 0;
  uint end = mArcLengthTable.Size() - 1;

  while(begin < end)
  {
    uint mid = (begin + end) / 2;
    if(mArcLengthTable[mid].ArcLength <= distance)
      begin = mid + 1;
    else
      end = mid;
  }

  //this binary search actually produced the upper bound,
  //the lower bound is the previous index
  return begin - 1;
}

//----------------------------------------------------------- Piecewise Function
/// Used to sort the control points by the x-position.
struct SortByX
{
  bool operator()(PiecewiseFunction::ControlPoint& left,
                  PiecewiseFunction::ControlPoint& right)
  {
    // We never want two control points to have the same Time,
    // so every time they're sorted, check and move one slightly
    if(left.Position.x == right.Position.x)
      right.Position.x += 0.0001f;
    return left.Position.x < right.Position.x;
  }
};

//******************************************************************************
PiecewiseFunction::PiecewiseFunction()
{
  mCurveType = CurveType::BSpline;
  mError = real(0.05);
}

//******************************************************************************
void PiecewiseFunction::Clear()
{
  // Clear all the control points
  mControlPoints.Clear();

  // The baked curve is no longer valid
  mBakedCurve.Clear();
}

//******************************************************************************
void PiecewiseFunction::AddControlPoint(Vec2Param pos, Vec2Param tanIn, 
                                        Vec2Param tanOut)
{
  ControlPoint cp;
  cp.Position = pos;
  cp.TangentIn = tanIn;
  cp.TangentOut = tanOut;
  mControlPoints.PushBack(cp);

  // Sort the control points along the x
  Sort(mControlPoints.All(), SortByX());

  // The baked curve is no longer valid
  mBakedCurve.Clear();
}

//******************************************************************************
void PiecewiseFunction::SetControlPoints(Zero::Array<ControlPoint>& controlPoints)
{
  mControlPoints.Assign(controlPoints.All());

  // Sort the control points along the x
  Sort(mControlPoints.All(), SortByX());

  // The baked curve is no longer valid
  mBakedCurve.Clear();
}

//******************************************************************************
float PiecewiseFunction::Sample(real x)
{
  // Default case
  if(mControlPoints.Empty())
    return 0.0f;

  // If it's to the left of the left most control point, return its y
  if(x <= mControlPoints.Front().Position.x)
    return mControlPoints.Front().Position.y;
  // If it's to the right of the right most control point, return its y
  else if(x >= mControlPoints.Back().Position.x)
    return mControlPoints.Back().Position.y;

  // Make sure the curve is baked
  if(!IsBaked())
    Bake();

  // Binary search to find the location of the sample
  int begin = 0;
  int end = (int)mBakedCurve.Size();

  while(begin < end)
  {
    int mid = (begin + end) / 2;
    if(mBakedCurve[mid].x < x)
      begin = mid + 1;
    else
      end = mid;
  }

  // Interpolate between the two points in the baked curve
  Vec3 p0 = mBakedCurve[begin - 1];
  Vec3 p1 = mBakedCurve[begin];
  real localT = (x - p0.x) / (p1.x - p0.x);
  return Math::Lerp(p0.y, p1.y, localT);
}

//******************************************************************************
void PiecewiseFunction::Bake()
{
  mBakedCurve.Clear();

  // Can't do anything with 0 control points
  if(mControlPoints.Empty())
    return;

  // With just one point, any sample value will result in the single point
  if(mControlPoints.Size() == 1)
  {
    mBakedCurve.PushBack(ToVector3(mControlPoints.Front().Position));
    return;
  }

  // We are going to use this to generate curves between control points
  static SplineCurve sCurve;
  sCurve.SetClosed(false);
  sCurve.mCurveType = mCurveType;

  // Walk through each set of control points
  for(uint i = 0; i < mControlPoints.Size() - 1; ++i)
  {
    // Get the two control points
    ControlPoint& cp0 = mControlPoints[i];
    ControlPoint& cp1 = mControlPoints[i + 1];

    // Clear the old curve
    sCurve.ControlPoints.Clear();

    // If it is discontinuous, 
    if(cp0.TangentOut.x == 0.0f || cp1.TangentIn.x == 0.0f)
    {
      // Only on the first iteration is the left control point not added
      if(i == 0)
        mBakedCurve.PushBack(Vec3(cp0.Position));
      mBakedCurve.PushBack(Vec3(Vec2(cp1.Position.x, cp0.Position.y)));
      mBakedCurve.PushBack(Vec3(cp1.Position));
      continue;
    }

    // It's a function, so we must clamp the tangent at the midway
    // point on the x-axis
    Vec2 halfWay = (cp0.Position + cp1.Position) * 0.5f;

    // Clamp the out tangent of the first control point
    float maxExtent = (halfWay.x - cp0.Position.x);
    float tangentScalar = maxExtent / cp0.TangentOut.x;
    tangentScalar = Math::Min(tangentScalar, 1.0f);

    // The newly clamped tangent
    Vec2 tangentOutClamped = cp0.Position + cp0.TangentOut * tangentScalar;

    // Add the first control point and clamped tangent out
    sCurve.AddControlPoint(Vec3(cp0.Position));
    sCurve.AddControlPoint(Vec3(tangentOutClamped));

    // Now do the same for the tangentIn on the second control point
    maxExtent = (halfWay.x - cp1.Position.x);
    tangentScalar = maxExtent / cp1.TangentIn.x;
    tangentScalar = Math::Min(tangentScalar, 1.0f);
    Vec2 tangentInClamped = cp1.Position + cp1.TangentIn * tangentScalar;

    // Add the clamped tangent in and the second control point
    sCurve.AddControlPoint(Vec3(tangentInClamped));
    sCurve.AddControlPoint(Vec3(cp1.Position));

    // Add the points of the curve
    sCurve.BakeAdaptive(mBakedCurve, mError);
  }
}

//******************************************************************************
bool PiecewiseFunction::IsBaked()
{
  return !mBakedCurve.Empty();
}

//******************************************************************************
Vec3Array::range PiecewiseFunction::GetBakedCurve()
{
  return mBakedCurve.All();
}

//******************************************************************************
bool PiecewiseFunction::Empty()
{
  return mBakedCurve.Empty();
}

}//namespace Math
