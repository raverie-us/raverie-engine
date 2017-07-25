///////////////////////////////////////////////////////////////////////////////
///
/// \file Curve.hpp
/// Declaration of the Curve class.
///
/// Authors: Joshua Davis
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Math
{

typedef Zero::Array<Math::Vector3> Vec3Array;

DeclareEnum3(CurveType, Linear, BSpline, CatmulRom);

//------------------------------------------------------------------------ Curve
/// A generic curve object that can switched between different spline types.
struct ZeroShared SplineCurve
{
  SplineCurve();

  void AddControlPoint(Vec3Param controlPoint);
  void RemovePointAtIndex(uint index);
  void AddControlPoints(const Vec3Array& controlPoints);
  void SetControlPoints(const Vec3Array& controlPoints);

  void GetPoints(Vec3Array& results, uint resolution) const;
  /// Bake the curve out using adaptive sampling. The error is the allowed
  /// area of a triangle from the baked points.
  void BakeAdaptive(Vec3Array& results, real error) const;

  Vec3Array& GetControlPoints();
  void Clear();

  /// Does the curve loop back in on itself at the end or does it just stop?
  bool GetClosed();
  void SetClosed(bool state);

  /// How the control points are used to generate the curve.
  CurveType::Enum GetCurveType();
  void SetCurveType(CurveType::Enum curveType);

  /// Estimates the distance between a point and the curve using point-to-line
  /// on each line segment generated using the given resolution
  bool DistanceSq(Vec3 point, uint resolution, real& distSq) const;

private:
  /// Get the correct set of control points for baking (continuous set or closed set).
  void GetSmoothPoints(Vec3Array& pts) const;
  void MakeContinuous(Vec3Array& points) const;
  void MakeClosed(Vec3Array& points) const;

  template <typename Policy>
  void GetPoints(const Vec3Array& points, Vec3Array& results, uint resolution) const;
  template <typename Policy>
  void GetPoints(const Vec3Array& points, Vec3Array& results, real error) const;

  template <typename Policy>
  Vec3 ComputePoint(real t, Vec3Param a, Vec3Param b, Vec3Param c, Vec3Param d) const;

  // Used to store a stack of a point on the curve (and how to compute that point)
  // so that sub-division can be performed at a later time.
  struct PointData
  {
    PointData() {};
    PointData(real t, Vec3Param point)
    {
      T = t;
      Point = point;
    }

    real T;
    Vec3 Point;
  };
  template <typename Policy>
  PointData ComputePointData(real t, Vec3Param a, Vec3Param b, Vec3Param c, Vec3Param d) const;

  //---------------------------------------------------------- B-Spline Policy
  struct BSplinePolicy
  {
    static const Mat4& GetBasis();
    static Vec4 GetParam(real t);
  };

  //------------------------------------------------------- Catmul-Rom Policy
  struct CatmulRomPolicy
  {
    static const Mat4& GetBasis();
    static Vec4 GetParam(real t);
  };

public:
  Vec3Array ControlPoints;
  CurveType::Enum mCurveType;
  bool mClosed;
};

/// A curve that has been baked out to a set of points and their respective arc-lengths.
/// This table can be used to find a point at a given distance along a curve.
class ZeroShared BakedCurve
{
public:
  struct BakedData
  {
    Vec3 Position;
    real ArcLength;
  };

  /// Bake out the given curve. The error term is the max number of
  /// units that a line segment is allowed to deviate from the curve.
  void Bake(const SplineCurve& curve, real error);
  /// The total number of points this curve was baked out to.
  uint Size() const;
  real GetTotalArcLength() const;
  /// Get one of the baked out points. Used primarily for debug drawing.
  BakedData GetPoint(uint index);
  void SetPoint(uint index, Vec3Param pos);

  /// Finds the point on the curve at the given arc-length distance.
  /// The tangent at this point can optionally be computed.
  Vec3 SampleTable(float distance, Vec3* tangent = nullptr) const;

  /// Samples the curve as a function at the specified x value
  /// Note: This is a linear operation
  Vec3 SampleFunction(float x, bool clamp = true) const;

private:
  // Binary search to find the index just below the given arc-length.
  uint SampleLowerBound(real distance) const;

  Zero::Array<BakedData> mArcLengthTable;
};

//----------------------------------------------------------- Piecewise Function
struct ZeroShared PiecewiseFunction
{
  struct ControlPoint;

  /// Constructor.
  PiecewiseFunction();

  /// Clears the entire curve. This will invalidate the baked curve.
  void Clear();

  /// Adds the given control point to the curve. Sorts after insertion.
  /// This will invalidate the baked curve.
  void AddControlPoint(Vec2Param pos, Vec2Param tanIn, Vec2Param tanOut);

  /// Sets all the control points and sorts after.
  /// This will invalidate the baked curve.
  void SetControlPoints(Zero::Array<ControlPoint>& controlPoints);

  /// Samples the curve at the given point. This will bake the curve
  /// if not already baked.
  float Sample(real x);

  /// Bakes the curve to the given array of points.
  void Bake();

  /// Returns whether or not the curve is baked.
  bool IsBaked();

  /// Returns a range of the baked curve.
  Vec3Array::range GetBakedCurve();

  /// Returns whether or not there are any control points in the curve.
  bool Empty();

  //-------------------------------------------------------------- Control Point
  struct ControlPoint
  {
    Vec2 Position;
    Vec2 TangentIn;
    Vec2 TangentOut;
  };

  /// The type of curve.
  CurveType::Enum mCurveType;

  /// All control points in the curve.
  Zero::Array<ControlPoint> mControlPoints;

  /// The allowed distance of a point on the curve from the baked approximation.
  real mError;

private:
  /// We don't want to have to rebuild the curve every time it is sampled,
  /// so we will bake it out to this array for faster sample time.
  /// Whenever the piecewise function is modified, it will be invalid
  /// until baked again.
  Vec3Array mBakedCurve;
};

}//namespace Math
