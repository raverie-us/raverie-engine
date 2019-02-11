// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// Validation that a float is not NAN/IND/INF
template <>
bool IsFinite<float>(float value)
{
  return isfinite(value);
}

// Validation that a double is not NAN/IND/INF
template <>
bool IsFinite<double>(double value)
{
  return isfinite(value);
}

// Correct floats that are NAN/IND/INF to zero
template <>
bool CorrectNonFiniteValues<float>(float& value)
{
  if (!IsFinite(value))
  {
    value = 0.0f;
    return false;
  }

  // Correct -0 to 0
  if (value == -0.0f)
    value = 0.0f;
  return true;
}

// Correct doubles that are NAN/IND/INF to zero
template <>
bool CorrectNonFiniteValues<double>(double& value)
{
  if (!IsFinite(value))
  {
    value = 0.0;
    return false;
  }

  // Correct -0 to 0
  if (value == -0.0)
    value = 0.0;
  return true;
}

} // namespace Zero
