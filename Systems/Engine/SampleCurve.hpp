///////////////////////////////////////////////////////////////////////////////
///
/// \file SampleCurve.hpp
/// Declaration of the SampleCurve resource.
///
/// Authors: Joshua Claeys
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class SampleCurve : public DataResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  struct ControlPoint;
  typedef Array<ControlPoint> ControlPointArray;
  SampleCurve();

  /// Set default data.
  void SetDefaults();

  /// Serialize the curve to/from a file.
  void Serialize(Serializer& stream);

  /// Initializes the curve.
  void Initialize();

  /// Sample the curve at the given t.
  float Sample(float t);
  float DebugSample(float t, StringParam id, Vec4Param color);

  /// Bakes out the curve so that it doesn't have to when the first sample
  /// is requested.
  void Bake();
  /// Returns whether or not that curve is already baked.
  bool IsBaked();

  /// Adds a control point.
  uint AddControlPoint(Vec2Param pos, Vec2Param tanIn, uint editorFlags = 0);
  uint AddControlPoint(Vec2Param pos, Vec2Param tanIn, Vec2Param tanOut,
                       uint editorFlags = 0);

  /// Clears all control points in the curve
  void Clear();

  /// Fills out the given array with the curve.
  void GetCurve(Vec3Array& curve);

  /// Returns the control points of the curve.
  ControlPointArray& GetControlPoints();

  /// Set / get the width range of the graph.
  void SetWidthMin(float min);
  void SetWidthMax(float max);
  float GetWidthMin();
  float GetWidthMax();
  void SetWidthRange(float min, float max);
  Vec2 GetWidthRange();

  /// Set / get the height range of the graph.
  void SetHeightMin(float min);
  void SetHeightMax(float max);
  float GetHeightMin();
  float GetHeightMax();
  void SetHeightRange(float min, float max);
  Vec2 GetHeightRange();

  /// Returns the range
  Vec2 GetRange();

  /// Internal control point
  struct ControlPoint
  {
    ControlPoint(){EditorFlags = 0;}
    ControlPoint(Vec2Param pos, Vec2Param tanIn, Vec2Param tanOut,
                 uint editorFlags = 0);

    /// Used for detecting piecewise curves.
    bool operator==(const ControlPoint& rhs);

    void Serialize(Serializer& stream);

    /// Returns the time and value as a vec2.
    Vec2 GetPosition();

    /// The time (or x-value) of the control point.
    float Time;
    /// The value at the given time.
    float Value;

    Vec2 TangentIn;
    Vec2 TangentOut;
    uint EditorFlags;
  };

  struct SampleKey
  {
    float LastSampledTime;
    Vec4 Color;
  };

  typedef HashMap<String, SampleKey> DebugSampleMap;
  DebugSampleMap DebugSamples;

  /// The allowed distance of a point on the curve from the baked approximation.
  float mError;

private:
  void UpdateRange();
  uint InsertControlPoint(ControlPoint cp);

  /// Used to sort the control points by the x-position.
  struct SortByX
  {
    bool operator()(ControlPoint& left, ControlPoint& right)
    {
      // We never want two control points to have the same Time,
      // so every time they're sorted, check and move one slightly
      if(left.Time == right.Time)
        right.Time += 0.0001f;
      return left.Time < right.Time;
    }
  };

  /// The ranges.
  Vec2 mWidthRange;
  Vec2 mHeightRange;

  Vec2 mRange;

  /// All control points in the curve.
  /// The control points are in the range [0, 1].
  ControlPointArray mControlPoints;

  Math::PiecewiseFunction mPiecewiseFunction;
};

///Manages all of the SampleCurve's.
class CurveManager : public ResourceManager
{
public:
  DeclareResourceManager(CurveManager, SampleCurve);

  CurveManager(BoundType* resourceType);
  SampleCurve* CreateNewResourceInternal(StringParam name) override;
};

}//namespace Zero
