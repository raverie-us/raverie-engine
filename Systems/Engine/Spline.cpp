///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{


namespace Events
{
DefineEvent(SplineModified);
DefineEvent(QuerySpline);
}//namespace Events

 //-------------------------------------------------------------------SplineEvent
ZilchDefineType(SplineEvent, builder, type)
{
  ZilchBindGetterSetterProperty(Spline);
  ZilchBindDefaultCopyDestructor();
  type->CreatableInScript = true;
}

Spline* SplineEvent::GetSpline() const
{
  return mSpline;
}

void SplineEvent::SetSpline(Spline* spline)
{
  mSpline = spline;
}

//-------------------------------------------------------------------SplineControlPoint
ZilchDefineType(SplineControlPoint, builder, type)
{
  ZeroBindDocumented();
  ZilchBindMember(mWorldPosition);
  ZilchBindDestructor();
  ZilchBindDefaultConstructor();
  ZilchBindConstructor(Vec3Param);
}

SplineControlPoint::SplineControlPoint()
{
  mWorldPosition = Vec3::cZero;
}

SplineControlPoint::SplineControlPoint(Vec3Param worldPosition)
{
  mWorldPosition = worldPosition;
}

//-------------------------------------------------------------------SplineControlPoints
ZilchDefineType(SplineControlPoints, builder, type)
{
  ZeroBindDocumented();

  ZilchBindMethod(Get);
  ZilchBindMethod(Set);
  ZilchBindMethod(Add);
  ZilchBindMethod(Clear);
  ZilchBindGetterProperty(Count);
}

SplineControlPoints::SplineControlPoints()
{
  mOwner = nullptr;
}

SplineControlPoints::~SplineControlPoints()
{
}

void SplineControlPoints::Add(const SplineControlPoint& controlPoint)
{
  mControlPoints.PushBack(controlPoint);
  mOwner->mIsModified = true;
}

void SplineControlPoints::Insert(int index, const SplineControlPoint& controlPoint)
{
  int count = GetCount();
  if(index >= count)
  {
    String msg = String::Format("Index %d is invalid. Array only contains %d elements.", index, count);
    DoNotifyException("Invalid index", msg);
    return;
  }

  mControlPoints.InsertAt(index, controlPoint);
  mOwner->mIsModified = true;
}

void SplineControlPoints::RemoveAt(int index)
{
  int count = GetCount();
  if(index >= count)
  {
    String msg = String::Format("Index %d is invalid. Array only contains %d elements.", index, count);
    DoNotifyException("Invalid index", msg);
    return;
  }

  mControlPoints.EraseAt(index);
  mOwner->mIsModified = true;
}

SplineControlPoint SplineControlPoints::Get(int index) const
{
  int count = GetCount();
  if(index >= count)
  {
    String msg = String::Format("Index %d is invalid. Array only contains %d elements.", index, count);
    DoNotifyException("Invalid index", msg);
    return Vec3::cZero;
  }

  return mControlPoints[index];
}

void SplineControlPoints::Set(int index, const SplineControlPoint& value)
{
  int count = GetCount();
  if(index >= count)
  {
    String msg = String::Format("Index %d is invalid. Array only contains %d elements.", index, count);
    DoNotifyException("Invalid index", msg);
    return;
  }
  mControlPoints[index] = value;
  mOwner->mIsModified = true;
}

void SplineControlPoints::Clear()
{
  mControlPoints.Clear();
  mOwner->mIsModified = true;
}

int SplineControlPoints::GetCount() const
{
  return mControlPoints.Size();
}

//-------------------------------------------------------------------SplineBakedPoint
ZilchDefineType(SplineBakedPoint, builder, type)
{
  ZeroBindDocumented();
  ZilchBindMember(mWorldPosition);
  ZilchBindDestructor();
  ZilchBindDefaultConstructor();
  ZilchBindConstructor(Vec3Param);
}

SplineBakedPoint::SplineBakedPoint()
{
  mWorldPosition = Vec3::cZero;
}

SplineBakedPoint::SplineBakedPoint(Vec3Param worldPosition)
{
  mWorldPosition = worldPosition;
}

//-------------------------------------------------------------------SplineBakedPoints
ZilchDefineType(SplineBakedPoints, builder, type)
{
  ZeroBindDocumented();

  ZilchBindMethod(Get);
  ZilchBindGetterProperty(Count);
}

SplineBakedPoints::SplineBakedPoints()
{
  mOwner = nullptr;
}
SplineBakedPoints::~SplineBakedPoints()
{
}

int SplineBakedPoints::GetCount() const
{
  mOwner->RebuildIfModified();
  return mOwner->mBakedCurve.Size();
}

SplineBakedPoint SplineBakedPoints::Get(uint index) const
{
  mOwner->RebuildIfModified();
  size_t count = GetCount();
  if(index >= count)
  {
    String msg = String::Format("Index %d is invalid. Array only contains %d elements.", index, count);
    DoNotifyException("Invalid index", msg);
    return Vec3::cZero;
  }

  Vec3 position = mOwner->mBakedCurve.GetPoint(index).Position;
  return SplineBakedPoint(position);
}

//-------------------------------------------------------------------SplineSampleData
ZilchDefineType(SplineSampleData, builder, type)
{
  ZilchBindDestructor();
  ZilchBindDefaultConstructor();
  ZilchBindMember(mWorldPoint);
  ZilchBindMember(mWorldTangent);
}

SplineSampleData::SplineSampleData()
{
  mWorldTangent = mWorldTangent = Vec3::cZero;
}

//-------------------------------------------------------------------Spline
ZilchDefineType(Spline, builder, type)
{
  ZeroBindDocumented();

  ZilchBindMethod(Create);
  ZilchBindMethod(Clone);

  ZilchBindGetterSetterProperty(SplineType);
  ZilchBindGetterSetterProperty(Closed);
  ZilchBindGetterSetterProperty(Error);

  ZilchBindGetterProperty(TotalDistance);
  ZilchBindMethod(SampleDistance);
  
  ZilchBindMethod(RebuildIfModified);
  ZilchBindMethod(ForceRebuild);
  ZilchBindMethod(DebugDraw);
  
  ZilchBindGetterProperty(ControlPoints);
  ZilchBindGetterProperty(BakedPoints);
}

Spline::Spline()
{
  mIsModified = true;
  mError = 0.01f;
  mCurve.mCurveType = Math::CurveType::CatmulRom;
  mCurve.mClosed = false;
  mControlPoints.mOwner = this;
  mBakedPoints.mOwner = this;
}

Spline* Spline::Create()
{
  Spline* spline = new Spline();
  return spline;
}

Spline* Spline::Clone() const
{
  Spline* newSpline = new Spline();
  newSpline->SetClosed(GetClosed());
  newSpline->SetSplineType(GetSplineType());
  newSpline->SetError(GetError());
  for(int i = 0; i < mControlPoints.GetCount(); ++i)
    newSpline->mControlPoints.Add(mControlPoints.Get(i));
  return newSpline;
}

SplineType::Enum Spline::GetSplineType() const
{
  return (SplineType::Enum)mCurve.mCurveType;
}

void Spline::SetSplineType(SplineType::Enum splineType)
{
  mCurve.mCurveType = (Math::CurveType::Enum)splineType;
  mIsModified = true;
}

bool Spline::GetClosed() const
{
  return mCurve.mClosed;
}

void Spline::SetClosed(bool closed)
{
  mCurve.mClosed = closed;
  mIsModified = true;
}

real Spline::GetError() const
{
  return mError;
}

void Spline::SetError(real error)
{
  real minError = real(0.0001);
  if(error < minError)
  {
    String msg = String::Format("The error cannot be smaller than %g. "
      "The error has been clamped.", minError);
    DoNotifyWarning("Error too small", msg);
    error = minError;
  }

  mError = error;
  mIsModified = true;
}

real Spline::GetTotalDistance()
{
  RebuildIfModified();

  return mBakedCurve.GetTotalArcLength();
}

SplineSampleData Spline::SampleDistance(float distance)
{
  RebuildIfModified();

  SplineSampleData data;
  data.mWorldPoint = mBakedCurve.SampleTable(distance, &data.mWorldTangent);
  return data;
}

void Spline::RebuildIfModified()
{
  if(!mIsModified)
    return;

  ForceRebuild();
}

void Spline::ForceRebuild()
{
  mIsModified = false;
  mCurve.Clear();
  Vec3Array controlPoints;
  for(size_t i = 0; i < mControlPoints.mControlPoints.Size(); ++i)
    controlPoints.PushBack(mControlPoints.mControlPoints[i].mWorldPosition);
  mCurve.AddControlPoints(controlPoints);
  mBakedCurve.Bake(mCurve, mError);

  SplineEvent toSend;
  toSend.mSpline = this;
  DispatchEvent(Events::SplineModified, &toSend);
}

void Spline::DebugDraw(Vec4Param color)
{
  RebuildIfModified();

  for(size_t i = 1; i < mBakedCurve.Size(); ++i)
  {
    Vec3 p0 = mBakedCurve.GetPoint(i - 1).Position;
    Vec3 p1 = mBakedCurve.GetPoint(i).Position;

    Debug::Line line = Debug::Line(p0, p1);
    line.Color(color);
    gDebugDraw->Add(line);
  }
}

SplineControlPoints* Spline::GetControlPoints()
{
  return &mControlPoints;
}

SplineBakedPoints* Spline::GetBakedPoints()
{
  return &mBakedPoints;
}

}//namespace Zero
