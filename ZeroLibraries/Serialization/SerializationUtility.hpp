///////////////////////////////////////////////////////////////////////////////
///
/// \file Utility.hpp
/// Declaration of the serialization utility functions.
///
/// Authors: Trevor Sundberg
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma  once

namespace Zero
{

// Used to validate values we serialize
template <typename T>
bool IsFinite(T value)
{
  // Assume all types are finite (we specialize float and double)
  return true;
}

// Attempt to correct a value to a finite number of zero if the value is not valid
// (a very poor recovery attempt, but better than nothing)
// Used to prevent floating point values from saving out NAN/IND/INF
// Returns true if the value was finite
template <typename T>
bool CorrectNonFiniteValues(T& value)
{
  // Assume all types are finite (we specialize float and double)
  return true;
}

// Specializations for float and double to correct NAN/IND/INF
template <>
bool IsFinite<float>(float value);
template <>
bool IsFinite<double>(double value);
template <>
bool CorrectNonFiniteValues<float>(float& value);
template <>
bool CorrectNonFiniteValues<double>(double& value);

}//namespace Zero
