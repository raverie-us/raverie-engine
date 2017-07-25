///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{

  //************************************************************************************************
  static float GetValueLinearCurve(const float current, const float total, const float startValue, 
    const float endValue)
  {
    return ((current / total) * (endValue - startValue)) + startValue;
  }

  //************************************************************************************************
  static float GetValueSquaredCurve(const float current, const float total, const float startValue, 
    const float endValue)
  {
    float percent = current / total;
    return ((percent * percent) * (endValue - startValue)) + startValue;
  }

  //************************************************************************************************
  float halfPI = 2.0f * Math::ArcTan(1.0f);
  static float GetValueSineCurve(const float current, const float total, const float startValue, 
    const float endValue)
  {
    return (Math::Sin((current / total) * halfPI) * (endValue - startValue)) + startValue;
  }

  //************************************************************************************************
  static float GetValueSquareRootCurve(const float current, const float total, const float startValue, 
    const float endValue)
  {
    float percent = current / total;

    if (percent < 0)
      return startValue;

    return (Math::Sqrt(percent) * (endValue - startValue)) + startValue;
  }

  //************************************************************************************************
  float expDenominator = Math::Log(101.0f);
  static float GetValueExponentialCurve(const float current, const float total, const float startValue, 
    const float endValue)
  {
    return ((Math::Log(((current / total) * 100.0f) + 1.0f) / expDenominator) * 
      (endValue - startValue)) + startValue;
  }


  //------------------------------------------------------------------------- Interpolator Container

  //************************************************************************************************
  InterpolatorContainer::InterpolatorContainer() : 
    Active(false), 
    Object(new InterpolatingObject()) 
  { 
    Object->Container = this; 
  }

  //************************************************************************************************
  InterpolatorContainer::InterpolatorContainer(const InterpolatorContainer& copy) :
    Active(copy.Active),
    Object(copy.Object)
  {
    Object->Container = this;
  }

  //************************************************************************************************
  void InterpolatorContainer::Swap(InterpolatorContainer& other)
  {
    bool active = Active;
    Active = other.Active;
    other.Active = active;

    InterpolatingObject* object = Object;
    Object = other.Object;
    other.Object = object;
    Object->Container = this;
    other.Object->Container = &other;
  }

  //--------------------------------------------------------------------------- Interpolating Object

  //************************************************************************************************
  InterpolatingObject::InterpolatingObject(const float startValue, const float endValue, 
      const unsigned numberOfFrames) :
    StartValue(startValue),
    EndValue(endValue), 
    TotalFrames(numberOfFrames),
    CurrentFrame(0), 
    TotalDistance(0),
    CurrentCurveType(LinearCurveType)
  {
    GetValue = GetValueLinearCurve;
  }

  //************************************************************************************************
  InterpolatingObject::InterpolatingObject(const float startValue, const float endValue, 
      const float distance) :
    StartValue(startValue), 
    EndValue(endValue), 
    TotalFrames(0), 
    CurrentFrame(0), 
    TotalDistance(distance),
    CurrentCurveType(LinearCurveType)
  {
    GetValue = GetValueLinearCurve;
  }

  //************************************************************************************************
  InterpolatingObject::InterpolatingObject() : 
    StartValue(0), 
    EndValue(0), 
    TotalFrames(0),
    CurrentFrame(0), 
    TotalDistance(0),
    CurrentCurveType(LinearCurveType)
  {
    GetValue = GetValueLinearCurve;
  }

  //************************************************************************************************
  InterpolatingObject::InterpolatingObject(const InterpolatingObject &copy) : 
    StartValue(copy.StartValue), 
    EndValue(copy.EndValue), 
    TotalFrames(copy.TotalFrames), 
    CurrentFrame(copy.CurrentFrame), 
    TotalDistance(copy.TotalDistance),
    CurrentCurveType(copy.CurrentCurveType)
  {
    switch (copy.CurrentCurveType)
    {
    case LinearCurveType:
      GetValue = GetValueLinearCurve;
      break;
    case SquaredCurveType:
      GetValue = GetValueSquaredCurve;
      break;
    case SineCurveType:
      GetValue = GetValueSineCurve;
      break;
    case SquareRootCurveType:
      GetValue = GetValueSquareRootCurve;
      break;
    case LogCurveType:
      GetValue = GetValueExponentialCurve;
      break;
    case CustomCurveType:
      CustomCurveObject.CurveData->Clear();
      (*CustomCurveObject.CurveData) = (*copy.CustomCurveObject.CurveData);
      break;
    }
  }

  //************************************************************************************************
  float InterpolatingObject::NextValue()
  {
    if (TotalFrames == 0)
      return EndValue;

    if (CurrentFrame >= TotalFrames)
      return EndValue;

    if (EndValue == StartValue)
      return EndValue;

    return GetValue((float)CurrentFrame++, (float)TotalFrames, StartValue, EndValue);
  }

  //************************************************************************************************
  float InterpolatingObject::ValueAtIndex(const unsigned index)
  {
    if (TotalFrames == 0)
      return EndValue;

    if (index >= TotalFrames)
      return EndValue;

    if (EndValue == StartValue)
      return EndValue;

    CurrentFrame = index;

    return GetValue((float)index, (float)TotalFrames, StartValue, EndValue);
  }

  //************************************************************************************************
  float InterpolatingObject::ValueAtDistance(const float currentDistance)
  {
    if (currentDistance == 0.0f)
      return StartValue;

    if (currentDistance >= TotalDistance)
      return EndValue;

    if (EndValue == StartValue)
      return EndValue;

    return GetValue(currentDistance, TotalDistance, StartValue, EndValue);
  }

  //************************************************************************************************
  void InterpolatingObject::SetValues(const float start, const float end, const unsigned frames)
  {
    StartValue = start;
    EndValue = end;
    TotalFrames = frames;
    CurrentFrame = 0;
    TotalDistance = 0;

    if (start == end)
      CurrentFrame = TotalFrames;
  }

  //************************************************************************************************
  void InterpolatingObject::SetValues(const float start, const float end, const float distance)
  {
    StartValue = start;
    EndValue = end;
    TotalFrames = 0;
    CurrentFrame = 0;
    TotalDistance = distance;
  }

  //************************************************************************************************
  void InterpolatingObject::JumpForward(const unsigned howManyFrames)
  {
    CurrentFrame += howManyFrames;
  }

  //************************************************************************************************
  bool InterpolatingObject::Finished()
  {
    return CurrentFrame >= TotalFrames;
  }

  //************************************************************************************************
  bool InterpolatingObject::Finished(SoundNode* nodeForEvent)
  {
    if (CurrentFrame < TotalFrames)
      return false;
    else
    {
      if (nodeForEvent)
        // Notify the external object that the interpolation is done
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
          nodeForEvent, Notify_InterpolationDone, (void*)nullptr));

      return true;
    }
  }

  //************************************************************************************************
  void InterpolatingObject::SetCustomCurve(Zero::Array<Math::Vec3>* curveData)
  {
    CurrentCurveType = CustomCurveType;
    CustomCurveObject.SetCurveData(curveData);
  }

  //************************************************************************************************
  void InterpolatingObject::SetCurve(const CurveTypes curveType)
  {
    CurrentCurveType = curveType;

    switch (curveType)
    {
    case LinearCurveType:
      GetValue = GetValueLinearCurve;
      break;
    case SquaredCurveType:
      GetValue = GetValueSquaredCurve;
      break;
    case SineCurveType:
      GetValue = GetValueSineCurve;
      break;
    case SquareRootCurveType:
      GetValue = GetValueSquareRootCurve;
      break;
    case LogCurveType:
      GetValue = GetValueExponentialCurve;
      break;
    default:
      GetValue = GetValueLinearCurve;
      break;
    }
  }

  //************************************************************************************************
  float InterpolatingObject::GetStartValue()
  {
    return StartValue;
  }

  //************************************************************************************************
  float InterpolatingObject::GetEndValue() const
  {
    return EndValue;
  }

  //************************************************************************************************
  const float InterpolatingObject::GetCurrentValue() const 
  {
    if (TotalFrames == 0 || CurrentFrame >= TotalFrames)
      return EndValue;
    else if (CurrentFrame == 0)
      return StartValue;
    else
      return GetValue((float)CurrentFrame, (float)TotalFrames, StartValue, EndValue);
  }

  //************************************************************************************************
  void InterpolatingObject::SetFrame(const unsigned frame)
  {
    CurrentFrame = frame;
  }

  //************************************************************************************************
  unsigned InterpolatingObject::GetTotalFrames()
  {
    return TotalFrames;
  }

  //************************************************************************************************
  unsigned InterpolatingObject::GetCurrentFrame()
  {
    return CurrentFrame;
  }

  //************************************************************************************************
  const CurveTypes InterpolatingObject::GetCurveType() const
  {
    return CurrentCurveType;
  }

  //----------------------------------------------------------------------------------- Custom Curve

  //************************************************************************************************
  CustomCurve::~CustomCurve()
  {
    if (CurveData)
      delete CurveData;
  }

  //************************************************************************************************
  void CustomCurve::SetCurveData(Zero::Array<Math::Vec3>* newCurveData)
  {
    if (CurveData)
      delete CurveData;

    CurveData = newCurveData;
  }

  //************************************************************************************************
  float CustomCurve::GetValue(const float current, const float total, const float startValue, 
    const float endValue)
  {
    if (CurveData->Empty())
      return 0.0f;

    float xPoint = current;

    // Adjust xPoint to be a 0-1 range
    xPoint = xPoint / total;

    // If current x is less than the beginning of the curve, return the starting value
    if (xPoint <= 0.0f)
      return startValue;
    // If it's after the end, return the ending value
    else if (xPoint >= 1.0f)
      return endValue;

    // Binary search to find the indexes for the current x
    int begin = 0;
    int end = (int)CurveData->Size();
    while (begin < end)
    {
      int middle = (begin + end) / 2;
      if ((*CurveData)[middle].x < xPoint)
        begin = middle + 1;
      else
        end = middle;
    }

    // Interpolate between the two indexes
    Math::Vec3 p0 = (*CurveData)[begin - 1];
    Math::Vec3 p1 = (*CurveData)[begin];
    float t = (xPoint - p0.x) / (p1.x - p0.x);
    float result = ((1.0f - t) * p0.y) + (t * p1.y);

    // Adjust to be a 0-1 range 
    if (result < 0.0f)
      result = 0.0f;
    else if (result > 1.0f)
      result = 1.0f;
    
    return result * (endValue - startValue) + startValue;
  }
}