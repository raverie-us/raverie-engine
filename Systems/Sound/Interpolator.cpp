///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{
//**************************************************************************************************
float GetValueLinearCurve(const float current, const float total, const float startValue,
  const float endValue)
{
  return ((current / total) * (endValue - startValue)) + startValue;
}

//**************************************************************************************************
float GetValueSquaredCurve(const float current, const float total, const float startValue,
  const float endValue)
{
  float percent = current / total;
  return ((percent * percent) * (endValue - startValue)) + startValue;
}

//**************************************************************************************************
float GetValueSineCurve(const float current, const float total, const float startValue,
  const float endValue)
{
  return (Math::Sin((current / total) * Math::cTwoPi) * (endValue - startValue)) + startValue;
}

//**************************************************************************************************
float GetValueSquareRootCurve(const float current, const float total, const float startValue,
  const float endValue)
{
  float percent = current / total;

  if (percent < 0)
    return startValue;

  return (Math::Sqrt(percent) * (endValue - startValue)) + startValue;
}

//**************************************************************************************************
const float expDenominator = Math::Log(101.0f);
float GetValueExponentialCurve(const float current, const float total, const float startValue,
  const float endValue)
{
  return ((Math::Log(((current / total) * 100.0f) + 1.0f) / expDenominator) *
    (endValue - startValue)) + startValue;
}

//----------------------------------------------------------------------------- Interpolating Object

//**************************************************************************************************
InterpolatingObject::InterpolatingObject() :
  mStartValue(0),
  mEndValue(0),
  mTotalFrames(0),
  mCurrentFrame(0),
  mTotalDistance(0),
  mCurrentCurveType(FalloffCurveType::Linear)
{
  GetValue = GetValueLinearCurve;
}

//**************************************************************************************************
float InterpolatingObject::NextValue()
{
  if (mTotalFrames == 0 || mCurrentFrame >= mTotalFrames || mEndValue == mStartValue)
    return mEndValue;

  if (mCurrentCurveType != FalloffCurveType::Custom)
    return GetValue((float)mCurrentFrame++, (float)mTotalFrames, mStartValue, mEndValue);
  else
    return CustomCurveObject.GetValue((float)mCurrentFrame++, (float)mTotalFrames, mStartValue, mEndValue);
}

//**************************************************************************************************
float InterpolatingObject::ValueAtIndex(const unsigned index)
{
  if (mTotalFrames == 0 || index >= mTotalFrames || mEndValue == mStartValue)
    return mEndValue;

  if (index == 0)
    return mStartValue;

  if (mCurrentCurveType != FalloffCurveType::Custom)
    return GetValue((float)index, (float)mTotalFrames, mStartValue, mEndValue);
  else
    return CustomCurveObject.GetValue((float)index, (float)mTotalFrames, mStartValue, mEndValue);
}

//**************************************************************************************************
float InterpolatingObject::ValueAtDistance(const float currentDistance)
{
  if (currentDistance == 0.0f)
    return mStartValue;

  if (currentDistance >= mTotalDistance || mEndValue == mStartValue)
    return mEndValue;

  if (mCurrentCurveType != FalloffCurveType::Custom)
    return GetValue(currentDistance, mTotalDistance, mStartValue, mEndValue);
  else
    return CustomCurveObject.GetValue(currentDistance, mTotalDistance, mStartValue, mEndValue);
}

//**************************************************************************************************
void InterpolatingObject::SetValues(const float start, const float end, const unsigned frames)
{
  mStartValue = start;
  mEndValue = end;
  mTotalFrames = frames;
  mCurrentFrame = 0;
  mTotalDistance = 0;

  if (start == end)
    mCurrentFrame = mTotalFrames;
}

//**************************************************************************************************
void InterpolatingObject::SetValues(const float endValue, const unsigned numberOfFrames)
{
  mStartValue = GetCurrentValue();

  mEndValue = endValue;
  mTotalFrames = numberOfFrames;
  mCurrentFrame = 0;
  mTotalDistance = 0;

  if (mStartValue == mEndValue)
    mCurrentFrame = mTotalFrames;
}

//**************************************************************************************************
void InterpolatingObject::SetValues(const float start, const float end, const float distance)
{
  mStartValue = start;
  mEndValue = end;
  mTotalFrames = 0;
  mCurrentFrame = 0;
  mTotalDistance = distance;
}

//**************************************************************************************************
void InterpolatingObject::JumpForward(const unsigned howManyFrames)
{
  mCurrentFrame += howManyFrames;
}

//**************************************************************************************************
void InterpolatingObject::JumpBackward(const unsigned howManyFrames)
{
  if (mCurrentFrame > howManyFrames)
    mCurrentFrame -= howManyFrames;
  else
    mCurrentFrame = 0;
}

//**************************************************************************************************
bool InterpolatingObject::Finished()
{
  return mCurrentFrame >= mTotalFrames;
}

//**************************************************************************************************
bool InterpolatingObject::Finished(HandleOf<SoundNode> nodeForEvent)
{
  if (mCurrentFrame < mTotalFrames)
    return false;
  else
  {
    if (nodeForEvent)
    {
      // Notify the external object that the interpolation is done
      Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundNode::DispatchEventFromMixThread, 
        *nodeForEvent, Events::AudioInterpolationDone), nodeForEvent);
    }

    return true;
  }
}

//**************************************************************************************************
void InterpolatingObject::SetCustomCurve(Zero::Array<Math::Vec3>* curveData)
{
  mCurrentCurveType = FalloffCurveType::Custom;
  CustomCurveObject.SetCurveData(curveData);
}

//**************************************************************************************************
void InterpolatingObject::SetCurve(const FalloffCurveType::Enum curveType)
{
  mCurrentCurveType = curveType;

  switch (curveType)
  {
  case FalloffCurveType::Linear:
    GetValue = GetValueLinearCurve;
    break;
  case FalloffCurveType::Squared:
    GetValue = GetValueSquaredCurve;
    break;
  case FalloffCurveType::Sine:
    GetValue = GetValueSineCurve;
    break;
  case FalloffCurveType::SquareRoot:
    GetValue = GetValueSquareRootCurve;
    break;
  case FalloffCurveType::Log:
    GetValue = GetValueExponentialCurve;
    break;
  default:
    GetValue = GetValueLinearCurve;
    break;
  }
}

//**************************************************************************************************
float InterpolatingObject::GetStartValue()
{
  return mStartValue;
}

//**************************************************************************************************
float InterpolatingObject::GetEndValue() const
{
  return mEndValue;
}

//**************************************************************************************************
const float InterpolatingObject::GetCurrentValue()
{
  if (mTotalFrames == 0 || mCurrentFrame >= mTotalFrames)
    return mEndValue;

  if (mCurrentFrame == 0)
    return mStartValue;

  if (mCurrentCurveType != FalloffCurveType::Custom)
    return GetValue((float)mCurrentFrame, (float)mTotalFrames, mStartValue, mEndValue);
  else
    return CustomCurveObject.GetValue((float)mCurrentFrame, (float)mTotalFrames, mStartValue, mEndValue);
}

//**************************************************************************************************
void InterpolatingObject::SetFrame(const unsigned frame)
{
  mCurrentFrame = frame;
}

//**************************************************************************************************
unsigned InterpolatingObject::GetTotalFrames()
{
  return mTotalFrames;
}

//**************************************************************************************************
unsigned InterpolatingObject::GetCurrentFrame()
{
  return mCurrentFrame;
}

//**************************************************************************************************
const FalloffCurveType::Enum InterpolatingObject::GetCurveType() const
{
  return mCurrentCurveType;
}

//------------------------------------------------------------------------------------- Custom Curve

//**************************************************************************************************
CustomCurve::~CustomCurve()
{
  if (CurveData)
    delete CurveData;
}

//**************************************************************************************************
void CustomCurve::SetCurveData(Zero::Array<Math::Vec3>* newCurveData)
{
  if (CurveData)
    delete CurveData;

  CurveData = newCurveData;
}

//**************************************************************************************************
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
