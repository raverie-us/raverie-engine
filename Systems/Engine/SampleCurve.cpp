///////////////////////////////////////////////////////////////////////////////
///
/// \file SampleCurve.cpp
/// Implementation of the SampleCurve resource.
///
/// Authors: Joshua Claeys
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(SampleCurve, builder, type)
{
  ZilchBindMethod(Sample);
  ZilchBindMethod(DebugSample);
  ZilchBindGetterProperty(WidthMax);
  ZilchBindGetterProperty(HeightMax);
}

SampleCurve::SampleCurve()
{
}


void SampleCurve::SetDefaults()
{
  mWidthRange.Set(0.0f, 1.0f);
  mHeightRange.Set(0.0f, 1.0f);
  mError = 0.01f;
  AddControlPoint(Vec2(0.0f, 0.0f), Vec2(-1.0f, -1.0f).Normalized() * 0.1f);
  AddControlPoint(Vec2(1.0f, 1.0f), Vec2(-1.0f, -1.0f).Normalized() * 0.1f);
}

void SampleCurve::Serialize(Serializer& stream)
{
  SerializeName(mWidthRange);
  SerializeName(mHeightRange);
  SerializeNameDefault(mError, 0.01f);
  SerializeName(mControlPoints);
}

void SampleCurve::Initialize()
{
  // Set the range
  mRange.Set(mWidthRange.y - mWidthRange.x, mHeightRange.y - mHeightRange.x);

  mPiecewiseFunction.mError = mError;
  forRange(ControlPoint& cp, mControlPoints.All())
  {
    mPiecewiseFunction.AddControlPoint(cp.GetPosition(), cp.TangentIn, cp.TangentOut);
  }

  // Bake the curve for faster sampling
  Bake();
}

float SampleCurve::Sample(float t)
{
  // Normalize the t value between 0.0 and 1.0 by the max extent
  t /= mHeightRange.y;

  // Sample the piecewise function
  return mPiecewiseFunction.Sample(t) * mRange.x;
}

float SampleCurve::DebugSample(float t, StringParam name, Vec4Param color)
{
  float val = Sample(t);
  SampleKey id;
  id.LastSampledTime = t;
  id.Color = color;
  DebugSamples.Insert(name, id);
  return val;
}

void SampleCurve::Bake()
{
  mPiecewiseFunction.Bake();
}

bool SampleCurve::IsBaked()
{
  return mPiecewiseFunction.IsBaked();
}

uint SampleCurve::AddControlPoint(Vec2Param pos, Vec2Param tanIn,
                                  uint editorFlags)
{
  return AddControlPoint(pos, tanIn, -tanIn, editorFlags);
}

uint SampleCurve::AddControlPoint(Vec2Param pos, Vec2Param tanIn,
                                  Vec2Param tanOut, uint editorFlags)
{
  return InsertControlPoint(ControlPoint(pos, tanIn, tanOut, editorFlags));
}

void SampleCurve::Clear()
{
  mControlPoints.Clear();
  mPiecewiseFunction.Clear();
}

void SampleCurve::GetCurve(Vec3Array& curve)
{
  static Math::PiecewiseFunction sFunction;
  sFunction.mError = mError;
  sFunction.Clear();
  sFunction.mControlPoints.Reserve(mControlPoints.Size());

  forRange(ControlPoint& cp, mControlPoints.All())
  {
    sFunction.AddControlPoint(cp.GetPosition(), cp.TangentIn, cp.TangentOut);
  }

  sFunction.Bake();
  curve.Assign(sFunction.GetBakedCurve());
}

SampleCurve::ControlPointArray& SampleCurve::GetControlPoints()
{
  return mControlPoints;
}

void SampleCurve::SetWidthMin(float min)
{
  mWidthRange.x = min;
  UpdateRange();
}

void SampleCurve::SetWidthMax(float max)
{
  mWidthRange.y = max;
  UpdateRange();
}

float SampleCurve::GetWidthMin()
{
  return mWidthRange.x;
}

float SampleCurve::GetWidthMax()
{
  return mWidthRange.y;
}

void SampleCurve::SetWidthRange(float min, float max)
{
  SetWidthMin(min);
  SetWidthMax(max);
}

Vec2 SampleCurve::GetWidthRange()
{
  return mWidthRange;
}

void SampleCurve::SetHeightMin(float min)
{
  mHeightRange.x = min;
  UpdateRange();
}

void SampleCurve::SetHeightMax(float max)
{
  mHeightRange.y = max;
  UpdateRange();
}

float SampleCurve::GetHeightMin()
{
  return mHeightRange.x;
}

float SampleCurve::GetHeightMax()
{
  return mHeightRange.y;
}

void SampleCurve::SetHeightRange(float min, float max)
{
  SetHeightMin(min);
  SetHeightMax(max);
}

Vec2 SampleCurve::GetHeightRange()
{
  return mHeightRange;
}

Vec2 SampleCurve::GetRange()
{
  return mRange;
}

void SampleCurve::UpdateRange()
{
  mRange = Vec2(mWidthRange.y - mWidthRange.x, mHeightRange.y - mHeightRange.x);
}

uint SampleCurve::InsertControlPoint(ControlPoint cp)
{
  // Add the control point
  mControlPoints.PushBack(cp);

  // Sort the control points along the x
  Sort(mControlPoints.All(), SortByX());

  mPiecewiseFunction.AddControlPoint(cp.GetPosition(), cp.TangentIn, cp.TangentOut);

  return mControlPoints.FindIndex(cp);
}

//---------------------------------------------------------------- Control Point
SampleCurve::ControlPoint::ControlPoint(Vec2Param pos, Vec2Param tanIn,
                                          Vec2Param tanOut, uint editorFlags)
  : Time(pos.x), Value(pos.y), TangentIn(tanIn),
    TangentOut(tanOut), EditorFlags(editorFlags)
{

}

bool SampleCurve::ControlPoint::operator==(const ControlPoint& rhs)
{
  return Time == rhs.Time;
}

void SampleCurve::ControlPoint::Serialize(Serializer& stream)
{
  SerializeName(Time);
  SerializeName(Value);
  SerializeName(TangentIn);
  SerializeName(TangentOut);
  SerializeNameDefault(EditorFlags, (uint)0);
}

Vec2 SampleCurve::ControlPoint::GetPosition()
{
  return Vec2(Time, Value);
}

ImplementResourceManager(CurveManager, SampleCurve);

CurveManager::CurveManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  AddLoader("SampleCurve", new TextDataFileLoader<CurveManager>());
  DefaultResourceName = "DefaultCurve";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.SampleCurve.data"));
  mCanCreateNew = true;
  mCanDuplicate = true;
  mExtension = DataResourceExtension;
}

SampleCurve* CurveManager::CreateNewResourceInternal(StringParam name)
{
  SampleCurve* curve = new SampleCurve();
  curve->SetDefaults();
  curve->Initialize();
  return curve;
}

}//namespace Zero
