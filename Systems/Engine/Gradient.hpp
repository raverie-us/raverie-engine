///////////////////////////////////////////////////////////////////////////////
///
/// \file Gradient.hpp
/// Declaration of the Gradient class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Specifies a range of values that can be interpolated between.
template <typename type, type (*lerpFunc)(const type&, 
                                          const type&, 
                                          const float) = Math::Lerp>
class Gradient
{
public:
  /// Holds the value and at which interpolant it occurs.
  struct GradientKey
  {
    /// Serializes the key to the given stream.
    void Serialize(Serializer& stream)
    {
      SerializeName(Value);
      SerializeName(Interpolant);
    }

    type Value;
    float Interpolant;
  };

  /// Inserts a new key into the gradient at the given interpolant value.
  void Insert(type val, float interpolant)
  {
    // Create a new key
    GradientKey key;
    key.Value = val;
    key.Interpolant = interpolant;

    // Check to make sure the interpolant is valid
    ErrorIf(interpolant < 0.0f || interpolant > 1.0f, 
            "Interpolant value must be between [0,1].");

    uint count = mControlPoints.Size();

    const int firstIndex = 0;
    int lastIndex = count - 1;

    // If there are no values, simply add it
    if(count == 0)
    {
      mControlPoints.PushBack(key);
    }
    // If the interpolant is greater than the end, add it to the end
    else if(interpolant > mControlPoints[lastIndex].Interpolant)
    {
      mControlPoints.PushBack(key);
    }
    // If there is only 1 control point in there, we already know that the
    // new control point has to be before it, so we can just add it to the
    // back, and swap it with the first
    else if(count == 1)
    {
      mControlPoints.PushBack(key);
      Math::Swap(mControlPoints[0], mControlPoints[1]);
    }
    // Otherwise, we need to sort it (insertion sort)
    else
    {
      // Binary search to find the index we want to Insert at
      uint index = FindIndexOf(interpolant);

      // Insert the key
      mControlPoints.InsertAt(index, key);
    }
  }

  /// Returns the interpolated value at the given interpolant.
  type Sample(float interpolant)
  {
    // Check to make sure the interpolant is valid
    ErrorIf(interpolant < 0.0f || interpolant > 1.0f, 
            "Interpolant value must be between [0,1].");

    uint count = mControlPoints.Size();

    const int firstIndex = 0;
    int lastIndex = count - 1;

    // If there are no values, return a default
    if(count == 0)
    {
      return type();
    }
    // If there is only one value, return it (base case)
    else if(count == 1)
    {
      return mControlPoints[firstIndex].Value;
    }
    // If 'interpolant' is before the first value, return the first value
    else if(interpolant  <= mControlPoints[firstIndex].Interpolant)
    {
      return mControlPoints[firstIndex].Value;
    }
    // If 'interpolant' is after the last value, return the last value
    else if(interpolant >= mControlPoints[lastIndex].Interpolant)
    {
      return mControlPoints[lastIndex].Value;
    }
    // Otherwise, we need to interpolate to find the current value
    else
    {
      // Binary search for our location in the array
      uint index = FindIndexOf(interpolant);

      // Get the keys
      GradientKey& prevKey = mControlPoints[index - 1];
      GradientKey& nextKey = mControlPoints[index];

      // Compute the interpolant
      float range = nextKey.Interpolant - prevKey.Interpolant;
      float base = interpolant - prevKey.Interpolant;
      float currentInterpolant = base / range;

      // Lerp the values, and return the result
      return (lerpFunc)(prevKey.Value, nextKey.Value, currentInterpolant);
    }
  }

  /// Fills out the given output array with samples values of the gradient
  /// at the given sample rate.
  void Sample(Array<type>& output, float sampleRate)
  {
    // Check to make sure the sampleRate is valid
    ErrorIf(sampleRate < 0.0f || sampleRate > 1.0f, 
            "Sample Rate must be between [0,1].");

    // Make only one allocation
    uint size = uint(1.0f / sampleRate);
    output.Resize(size + 1);

    // Fill the output with samples based on the given sample rate
    for(uint i = 0; i < output.Size() ; ++i)
    {
      // Get the interpolant of the given value
      float interpolant = float(i) / float(size);

      // Add the sampled value to the output
      output[i] = Sample(interpolant);
    }
  }

  /// Clears all control points.
  void Clear()
  {
    mControlPoints.Clear();
  }

  /// Binary search for the index of the given interpolant.
  uint FindIndexOf(float interpolant)
  {
    int min = 0;
    int max = (int)(mControlPoints.Size() - 1);

    // Binary search
    while(min < max)
    {
      // Get the middle index
      int middle = (max + min) / 2;

      // Find which side to search
      if(mControlPoints[middle].Interpolant < interpolant)
      {
        min = middle + 1;
      }
      else
      {
        max = middle;
      }
    }

    // Return the index found
    return min;
  }

  /// Serializes the gradient to the given stream.
  void Serialize(Serializer& stream)
  {
    SerializeName(mControlPoints);
  }

  /// A list of all the control points in the gradient.
  typedef Array<GradientKey> KeyArray;
  KeyArray mControlPoints;
};

} //namespace Zero
