///////////////////////////////////////////////////////////////////////////////
///
/// \file Serialization.cpp
/// Implementation of the serialization utility functions.
///
/// Authors: Trevor Sundberg
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Validation that a float is not NAN/IND/INF
template <>
bool IsFinite<float>(float value)
{
#ifdef _MSC_VER
  return _finite(value) != 0;
#else
  return true;
#endif
}

// Validation that a double is not NAN/IND/INF
template <>
bool IsFinite<double>(double value)
{
#ifdef _MSC_VER
  return _finite(value) != 0;
#else
  return true;
#endif
}
  
// Correct floats that are NAN/IND/INF to zero
template <>
bool CorrectNonFiniteValues<float>(float& value)
{
  if(!IsFinite(value))
  {
    value = 0.0f;
    return false;
  }
  return true;
}
  
// Correct doubles that are NAN/IND/INF to zero
template <>
bool CorrectNonFiniteValues<double>(double& value)
{
  if(!IsFinite(value))
  {
    value = 0.0;
    return false;
  }
  return true;
}

}//namespace Zero
