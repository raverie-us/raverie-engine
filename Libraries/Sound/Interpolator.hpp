///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

/// The type of curve used for attenuating volume by SoundAttenuators.
/// <param name="Linear">Volume reduces linearly with distance.</param>
/// <param name="Squared">Volume reduces slowly at first then linearly.</param>
/// <param name="Sine">Volume reduces linearly at first then slowly.</param>
/// <param name="SquareRoot">Volume reduces quickly at first then linearly.</param>
/// <param name="Log">Volume reduces logarithmically, mimicking sound in real life.</param>
/// <param name="Custom">Sets the volume reduction using a SampleCurve resource.</param>
DeclareEnum6(FalloffCurveType, Linear, Squared, Sine, SquareRoot, Log, Custom);

//------------------------------------------------------------------------------------- Custom Curve

// Interpolation using a custom curve. 
class CustomCurve
{
public:
  CustomCurve() : CurveData(NULL) {}
  ~CustomCurve();

  // Calculates a value using the current percentage.
  float GetValue(const float current, const float total, const float startValue, const float endValue);
  // Sets the custom curve data pointer. Will delete this data on destruction.
  void SetCurveData(Array<Math::Vec3>* curveData);

private:
  // Array of custom curve values. 
  Array<Math::Vec3>* CurveData;
};

//----------------------------------------------------------------------------- Interpolating Object

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
  bool Finished(HandleOf<SoundNode> nodeForEvent);
  // Gets the end value of the interpolation.
  float GetEndValue() const;
  // Gets the start value of the interpolation.
  float GetStartValue();
  // Gets the current value of a sequential interpolation.
  const float GetCurrentValue();
  // Gets the type of curve currently being used. 
  const FalloffCurveType::Enum GetCurveType() const;
  // Sets a new custom curve for this interpolator. Relies on curves being in 0-1 range.
  void SetCustomCurve(Array<Math::Vec3>* curveData);
  // Changes the curve type of this interpolator. 
  void SetCurve(const FalloffCurveType::Enum curveType);
  // Resets the object for sequential interpolation using NextValue or ValueAtIndex.
  // If no start value is specified, will use the current value
  void SetValues(const float startValue, const float endValue, const unsigned numberOfFrames);
  void SetValues(const float endValue, const unsigned numberOfFrames);
  // Resets the object for direct-access interpolation using ValueAtDistance.
  void SetValues(const float startValue, const float endValue, const float distance);

private:
  // Starting value to interpolate from. 
  float mStartValue;
  // Ending value to interpolate to. 
  float mEndValue;
  // Total distance to interpolate over using direct-access. 
  float mTotalDistance;
  // Total frames to interpolate over using sequential access. 
  unsigned mTotalFrames;
  // Current frame of sequential interpolation. 
  unsigned mCurrentFrame;
  // The type of curve currently being used
  FalloffCurveType::Enum mCurrentCurveType;
  // The object used to handle custom curve data
  CustomCurve CustomCurveObject;
  // A pointer to the function used to get values. Set according to curve type.
  float(*GetValue)(const float current, const float total, const float startValue, const float endValue);
};

} // namespace Zero
