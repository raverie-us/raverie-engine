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
    // Sets the custom curve data pointer. Will delete this data on destruction.
    void SetCurveData(Zero::Array<Math::Vec3>* curveData);

  private:
    // Array of custom curve values. 
    Zero::Array<Math::Vec3>* CurveData;
  };

  //--------------------------------------------------------------------------- Interpolating Object

  class SoundNode;

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
    // Moves sequential interpolation backward by a specified number.
    void JumpBackward(const unsigned howManyFrames);
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
    const float GetCurrentValue();
    // Gets the type of curve currently being used. 
    const CurveTypes::Enum GetCurveType() const;
    // Sets a new custom curve for this interpolator. Relies on curves being in 0-1 range.
    void SetCustomCurve(Zero::Array<Math::Vec3>* curveData);
    // Changes the curve type of this interpolator. 
    void SetCurve(const CurveTypes::Enum curveType);
    // Resets the object for sequential interpolation using NextValue or ValueAtIndex.
    void SetValues(const float startValue, const float endValue, const unsigned numberOfFrames);
    // Resets the object for direct-access interpolation using ValueAtDistance.
    void SetValues(const float startValue, const float endValue, const float distance);

  private:
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
    // The type of curve currently being used
    CurveTypes::Enum CurrentCurveType;
    // The object used to handle custom curve data
    CustomCurve CustomCurveObject;
    // A pointer to the function used to get values. Set according to curve type.
    float(*GetValue)(const float current, const float total, const float startValue, const float endValue);
  };
}

#endif