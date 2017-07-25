///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef INTERP_H
#define INTERP_H

namespace Audio
{
  //----------------------------------------------------------------------------------- Custom Curve

  // Interpolation using a custom curve. 
  class CustomCurve 
  {
  public:
    CustomCurve() : CurveData(NULL) {}
    ~CustomCurve();
    
    // Calculates a value using the current percentage.
    float GetValue(const float current, const float total, const float startValue, const float endValue);

    void SetCurveData(Zero::Array<Math::Vec3>* curveData);

  private:
    // Array of custom curve values. 
    Zero::Array<Math::Vec3>* CurveData;
    
    friend class ThreadedEmitterObject;
    friend class InterpolatingObject;
  };

  //--------------------------------------------------------------------------- Interpolating Object

  class InterpolatingObject;

  class InterpolatorContainer
  {
  public:
    InterpolatorContainer();
    InterpolatorContainer(const InterpolatorContainer& copy);

    void Swap(InterpolatorContainer& other);

    bool Active;
    InterpolatingObject* Object;
  };

  // Object to interpolate either sequentially or with direct access. 
  class InterpolatingObject
  {
  public:
    // Sets default values to 0. Uses LinearCurve by default. 
    InterpolatingObject();

    // Calculates the next sequential value in the interpolation. If past the end point, returns end value. 
    float NextValue();
    // Calculates the interpolated value at a specified index. If past the end, returns end value.
    float ValueAtIndex(const unsigned index);
    // Calculates the interpolated value at a specified point. If past the total distance, returns end value. 
    float ValueAtDistance(const float distance);
    // Moves sequential interpolation forward by a specified number.
    void JumpForward(const unsigned howManyFrames);
    // Sets the interpolation to a specified position.
    void SetFrame(const unsigned frame);
    // Gets the total number of frames in the interpolation
    unsigned GetTotalFrames();
    // Gets the current frame of the interpolation.
    unsigned GetCurrentFrame();
    // Checks if the sequential interpolation has reached its end point.
    bool Finished();
    // Checks if the sequential interpolation has reached its end point and sends notification if true.
    bool Finished(SoundNode* nodeForEvent);
    // Gets the end value of the interpolation.
    float GetEndValue() const;
    // Gets the start value of the interpolation.
    float GetStartValue();
    // Gets the current value of a sequential interpolation.
    const float GetCurrentValue() const;
    // Gets the type of curve currently being used. 
    const CurveTypes GetCurveType() const;
    // Sets a new custom curve for this interpolator. Relies on curves being in 0-1 range.
    void SetCustomCurve(Zero::Array<Math::Vec3>* curveData);
    // Changes the curve type of this interpolator. 
    void SetCurve(const CurveTypes curveType);
    // Resets the object for sequential interpolation using NextValue or ValueAtIndex.
    void SetValues(const float startValue, const float endValue, const unsigned numberOfFrames);
    // Resets the object for direct-access interpolation using ValueAtDistance.
    void SetValues(const float startValue, const float endValue, const float distance);

  private:
    // Sets up object for sequential interpolation using (). Uses LinearCurve by default.
    InterpolatingObject(const float startValue, const float endValue, const unsigned numberOfFrames);
    // Sets up object for direct-access interpolation using []. Uses LinearCurve by default.
    InterpolatingObject(const float startValue, const float endValue, const float distance);
    InterpolatingObject(const InterpolatingObject &copy);

    // Starting value to interpolate from. 
    float StartValue;
    // Ending value to interpolate to. 
    float EndValue;
    // Total distance to interpolate over using direct-access. 
    float TotalDistance;
    // Total frames to interpolate over using sequential access. 
    unsigned TotalFrames;
    // Current frame of sequential interpolation. 
    unsigned CurrentFrame;

    CurveTypes CurrentCurveType;
    float(*GetValue)(const float current, const float total, const float startValue, const float endValue);
    CustomCurve CustomCurveObject;

    InterpolatorContainer* Container;

    friend class AudioSystemInternal;
    friend class InterpolatorContainer;
  };
}

#endif