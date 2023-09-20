// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class Spline;

DeclareEnum3(SplineType, Linear, BSpline, CatmullRom);

namespace Events
{
DeclareEvent(SplineModified);
DeclareEvent(QuerySpline);
} // namespace Events

/// Event sent out for notifications about a spline
class SplineEvent : public Event
{
public:
  RaverieDeclareType(SplineEvent, TypeCopyMode::ReferenceType);

  SplineEvent();
  SplineEvent(Spline* spline);

  Spline* GetSpline() const;
  void SetSpline(Spline* spline);

  HandleOf<Spline> mSpline;
};

/// Data for a spline's control point
class SplineControlPoint
{
public:
  RaverieDeclareType(SplineControlPoint, TypeCopyMode::ValueType);

  SplineControlPoint();
  SplineControlPoint(Vec3Param position);

  /// The position of a control point
  Vec3 mPosition;
};

/// Control points for the Spline class. Modifying this will cause the spline to
/// be marked as modified to rebuild the baked curve when needed.
class SplineControlPoints : public SafeId32Object
{
public:
  RaverieDeclareType(SplineControlPoints, TypeCopyMode::ReferenceType);

  SplineControlPoints();
  ~SplineControlPoints();

  /// Add a new point to the end of the array
  void Add(const SplineControlPoint& controlPoint);
  /// Inserts the given point at the specified index.
  void Insert(int index, const SplineControlPoint& controlPoint);
  /// Remove the item at the given index
  void RemoveAt(int index);
  /// Get the control point at the given index.
  SplineControlPoint Get(int index) const;
  /// Sets the control point at the given index.
  void Set(int index, const SplineControlPoint& value);
  /// Clear all control points
  void Clear();
  /// The number of control points contained
  int GetCount() const;

  Array<SplineControlPoint> mControlPoints;
  Spline* mOwner;
};

/// Data for a spline's baked point
class SplineBakedPoint
{
public:
  RaverieDeclareType(SplineBakedPoint, TypeCopyMode::ValueType);

  SplineBakedPoint();
  SplineBakedPoint(Vec3Param position);

  /// The position of a baked point
  Vec3 mPosition;
};

/// Read-only baked points for the Spline class. Will auto-recompute if the
/// control points have changed.
class SplineBakedPoints : SafeId32Object
{
public:
  RaverieDeclareType(SplineBakedPoints, TypeCopyMode::ReferenceType);

  SplineBakedPoints();
  ~SplineBakedPoints();

  int GetCount() const;
  SplineBakedPoint Get(uint index) const;

  Spline* mOwner;
};

/// Returned data from sampling a spline at a given arc-length distance.
class SplineSampleData
{
public:
  RaverieDeclareType(SplineSampleData, TypeCopyMode::ValueType);

  SplineSampleData();

  /// The point on the curve.
  Vec3 mPoint;
  // The tangent of the curve (traveling forward) at the sampled point.
  Vec3 mTangent;
};

/// A spline built from control points. Bakes out the curve using an error
/// term (distance from actual spline). Provides an interface to sample the
/// curve at a given arc-length distance in order to provide constant speed
/// interpolation.
class Spline : public ReferenceCountedEventObject
{
public:
  RaverieDeclareType(Spline, TypeCopyMode::ReferenceType);

  Spline();

  /// Create a new instance of a spline.
  static Spline* Create();
  /// Create a new copy of this spline
  Spline* Clone() const;

  /// The kind of spline (Linear, BSpline, CatmullRom).
  /// Determines how the control points affect the curve.
  SplineType::Enum GetSplineType() const;
  void SetSplineType(SplineType::Enum splineType);
  /// Does the spline loop back on itself?
  bool GetClosed() const;
  void SetClosed(bool closed);
  /// The max number of units that a line segment is
  /// allowed to deviate from the curve.
  real GetError() const;
  void SetError(real error);

  /// The total arc-length of the curve. Use to normalize the curve if you wish.
  real GetTotalDistance();
  /// Samples the curve at a given arc-length distance.
  SplineSampleData SampleDistance(float distance);
  /// Samples the curve with a time in the range of [0, 1].
  SplineSampleData SampleNormalized(float time);

  /// Rebuild the baked points from the control points if they have changed.
  /// Should not need to be manually called unless the user wants to control the
  /// timing when the points are baked.
  void RebuildIfModified();
  /// Forcibly rebuild the baked points from the control points.
  void ForceRebuild();
  /// Draw the baked points of the curve with the provided color.
  void DebugDraw(Vec4Param color);

  /// The control points used to bake out the curve.
  SplineControlPoints* GetControlPoints();
  /// The read-only curve points baked out to line segments using the provided
  /// error.
  SplineBakedPoints* GetBakedPoints();

private:
  friend SplineControlPoints;
  friend SplineBakedPoints;
  SplineControlPoints mControlPoints;
  SplineBakedPoints mBakedPoints;

  bool mIsModified;
  real mError;
  Math::SplineCurve mCurve;
  Math::BakedCurve mBakedCurve;
};

} // namespace Raverie
