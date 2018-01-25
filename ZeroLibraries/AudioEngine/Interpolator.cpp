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
  static float GetValueSineCurve(const float current, const float total, const float startValue, 
    const float endValue)
  {
    return (Math::Sin((current / total) * Math::cTwoPi) * (endValue - startValue)) + startValue;
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

  //--------------------------------------------------------------------------- Interpolating Object

  //************************************************************************************************
  InterpolatingObject::InterpolatingObject() : 
    StartValue(0), 
    EndValue(0), 
    TotalFrames(0),
    CurrentFrame(0), 
    TotalDistance(0),
    CurrentCurveType(CurveTypes::Linear)
  {
    GetValue = GetValueLinearCurve;
  }

  //************************************************************************************************
  float InterpolatingObject::NextValue()
  {
    if (TotalFrames == 0 || CurrentFrame >= TotalFrames || EndValue == StartValue)
      return EndValue;

    if (CurrentCurveType != CurveTypes::Custom)
      return GetValue((float)CurrentFrame++, (float)TotalFrames, StartValue, EndValue);
    else
      return CustomCurveObject.GetValue((float)CurrentFrame++, (float)TotalFrames, StartValue, EndValue);
  }

  //************************************************************************************************
  float InterpolatingObject::ValueAtIndex(const unsigned index)
  {
    if (TotalFrames == 0 || index >= TotalFrames || EndValue == StartValue)
      return EndValue;

    if (index == 0)
      return StartValue;

    if (CurrentCurveType != CurveTypes::Custom)
      return GetValue((float)index, (float)TotalFrames, StartValue, EndValue);
    else
      return CustomCurveObject.GetValue((float)index, (float)TotalFrames, StartValue, EndValue);
  }

  //************************************************************************************************
  float InterpolatingObject::ValueAtDistance(const float currentDistance)
  {
    if (currentDistance == 0.0f)
      return StartValue;

    if (currentDistance >= TotalDistance || EndValue == StartValue)
      return EndValue;

    if (CurrentCurveType != CurveTypes::Custom)
      return GetValue(currentDistance, TotalDistance, StartValue, EndValue);
    else
      return CustomCurveObject.GetValue(currentDistance, TotalDistance, StartValue, EndValue);
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
  void InterpolatingObject::JumpBackward(const unsigned howManyFrames)
  {
    if (CurrentFrame > howManyFrames)
      CurrentFrame -= howManyFrames;
    else
      CurrentFrame = 0;
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
          nodeForEvent, AudioEventTypes::InterpolationDone, (void*)nullptr));

      return true;
    }
  }

  //************************************************************************************************
  void InterpolatingObject::SetCustomCurve(Zero::Array<Math::Vec3>* curveData)
  {
    CurrentCurveType = CurveTypes::Custom;
    CustomCurveObject.SetCurveData(curveData);
  }

  //************************************************************************************************
  void InterpolatingObject::SetCurve(const CurveTypes::Enum curveType)
  {
    CurrentCurveType = curveType;

    switch (curveType)
    {
    case CurveTypes::Linear:
      GetValue = GetValueLinearCurve;
      break;
    case CurveTypes::Squared:
      GetValue = GetValueSquaredCurve;
      break;
    case CurveTypes::Sine:
      GetValue = GetValueSineCurve;
      break;
    case CurveTypes::SquareRoot:
      GetValue = GetValueSquareRootCurve;
      break;
    case CurveTypes::Log:
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
  const float InterpolatingObject::GetCurrentValue()  
  {
    if (TotalFrames == 0 || CurrentFrame >= TotalFrames)
      return EndValue;
    
    if (CurrentFrame == 0)
      return StartValue;

    if (CurrentCurveType != CurveTypes::Custom)
      return GetValue((float)CurrentFrame, (float)TotalFrames, StartValue, EndValue);
    else
      return CustomCurveObject.GetValue((float)CurrentFrame, (float)TotalFrames, StartValue, EndValue);
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
  const CurveTypes::Enum InterpolatingObject::GetCurveType() const
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