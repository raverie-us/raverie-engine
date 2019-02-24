// MIT Licensed (see LICENSE.md).
#pragma once

namespace Math
{

// Projects the input vector onto the given vector (must be normalized)
template <typename VectorType>
ZeroSharedTemplate VectorType GenericProjectOnVector(const VectorType& input, const VectorType& normalizedVector)
{
  return normalizedVector * Math::Dot(input, normalizedVector);
}

// Projects the input vector onto a plane (the normal must be normalized)
template <typename VectorType>
ZeroSharedTemplate VectorType GenericProjectOnPlane(const VectorType& input, const VectorType& planeNormal)
{
  return input - GenericProjectOnVector(input, planeNormal);
}

/// Calculates the reflection vector across a given plane.
template <typename VectorType>
ZeroSharedTemplate VectorType GenericReflectAcrossPlane(const VectorType& input, const VectorType& planeNormal)
{
  return input - 2 * GenericProjectOnVector(input, planeNormal);
}

/// Calculates the reflection vector across a given vector.
template <typename VectorType>
ZeroSharedTemplate VectorType GenericReflectAcrossVector(const VectorType& input, const VectorType& planeNormal)
{
  return 2 * GenericProjectOnVector(input, planeNormal) - input;
}

/// Calculates the refraction vector through a plane given a certain index of
/// refraction.
template <typename VectorType>
ZeroSharedTemplate VectorType GenericRefract(const VectorType& incidentVector,
                                             const VectorType& planeNormal,
                                             real refractionIndex)
{
  real iDotN = Math::Dot(incidentVector, planeNormal);
  real r = real(1.0) - refractionIndex * refractionIndex * (real(1.0) - iDotN * iDotN);
  if (r < 0)
    return VectorType::cZero;

  return refractionIndex * incidentVector - planeNormal * (refractionIndex * iDotN + Math::Sqrt(r));
}

// Equal to Cos(ToRadius(1))
const float cSlerpParallelEpsilon = 0.999849f;

// Geometric Slerp between two vectors. This version assumes the user has
// pre-validated the data so the inputs are not zero, are normalized, and that
// there isn't a degenerate solution (e.g. the vectors are parallel). Used when
// the data-set has been pre-emptively cleaned up so that multiple validation
// checks are done 'offline' and all calls can be fast.
template <typename VectorType>
VectorType FastGeometricSlerp(const VectorType& start, const VectorType& end, real t)
{
  real cosTheta = Math::Dot(start, end);
  // Clamp for float errors (ACos can't handle values outside [-1, 1]
  cosTheta = Math::Clamp(cosTheta, -1.0f, 1.0f);
  // Handle the vectors being parallel by a small angle approximation
  if (Math::Abs(cosTheta) > cSlerpParallelEpsilon)
    return Math::Lerp(start, end, t);

  real theta = Math::ArcCos(cosTheta);

  real u = Math::Cos(theta * t);
  real v = Math::Sin(theta * t);
  // Compute an orthonormal bases to use for interpolation by projecting start
  // out of end.
  VectorType perpendicularVector = end - start * cosTheta;
  perpendicularVector.Normalize();

  VectorType result = u * start + v * perpendicularVector;
  return result;
}

// Geometric Slerp between two vectors. This version will check for any possible
// degeneracy cases and normalize the input vectors.
template <typename VectorType>
VectorType SafeGeometricSlerp(const VectorType& startUnNormalized, const VectorType& endUnNormalized, real t)
{
  VectorType start = Math::AttemptNormalized(startUnNormalized);
  VectorType end = Math::AttemptNormalized(endUnNormalized);

  real cosTheta = Math::Dot(start, end);
  // Clamp for float errors (ACos can't handle values outside [-1, 1]
  cosTheta = Math::Clamp(cosTheta, -1.0f, 1.0f);
  real theta = Math::ArcCos(cosTheta);

  real u = Math::Cos(theta * t);
  real v = Math::Sin(theta * t);
  VectorType perpendicularVector;

  // If the inputs are not parallel
  if (Math::Abs(cosTheta) < cSlerpParallelEpsilon)
  {
    // Compute an orthonormal bases to use for interpolation by projecting start
    // out of end.
    perpendicularVector = end - start * cosTheta;
    perpendicularVector.AttemptNormalize();
  }
  // Otherwise the vectors are almost parallel so we need to special case the
  // perpendicular vector
  else
  {
    // The vectors are parallel. Fall back to a small angle approximation and
    // just lerp.
    if (cosTheta > 0)
    {
      perpendicularVector = end;
      u = (1 - t);
      v = t;
    }
    // The vectors are anti-parallel. There are multiple solutions so pick any
    // perpendicular vector.
    else
    {
      perpendicularVector = GeneratePerpendicularVector(start);
      perpendicularVector.AttemptNormalize();
    }
  }

  VectorType result = u * start + v * perpendicularVector;
  return result;
}

// Geometric Slerp between two vectors. This is the 'pure' mathematical
// Slerp function. This effectively traces along an ellipse defined by the two
// input vectors.
template <typename VectorType>
VectorType SafeGeometricSlerpUnnormalized(const VectorType& start, const VectorType& end, real t)
{
  real startLength = Math::Length(start);
  real endLength = Math::Length(end);
  if (startLength == 0 || endLength == 0)
    return start;

  real cosTheta = Math::Dot(start, end);
  // Compute the actual angle since (the inputs are not assumed to be
  // normalized)
  cosTheta /= (startLength * endLength);
  // Clamp for float errors (ACos can't handle values outside [-1, 1]
  cosTheta = Math::Clamp(cosTheta, -1.0f, 1.0f);
  real theta = Math::ArcCos(cosTheta);

  real u = Math::Cos(theta * t);
  real v = Math::Sin(theta * t);
  VectorType perpendicularVector;

  // If the inputs are not parallel
  if (Math::Abs(cosTheta) < cSlerpParallelEpsilon)
  {
    // Compute an orthonormal bases to use for interpolation by projecting start
    // out of end.
    perpendicularVector = end - start * cosTheta;
    // Instead of normalizing this vector, we need to apply the normalization
    // factor from the generic Slerp formula (see Wikipedia).
    perpendicularVector /= Math::Sin(theta);
  }
  // Otherwise the vectors are almost parallel so we need to special case the
  // perpendicular vector
  else
  {
    // The vectors are parallel. Fall back to a small angle approximation and
    // just lerp.
    if (cosTheta > 0)
    {
      perpendicularVector = end;
      u = (1 - t);
      v = t;
    }
    // The vectors are anti-parallel. There are multiple solutions so pick any
    // perpendicular vector.
    else
    {
      perpendicularVector = GeneratePerpendicularVector(start);
      perpendicularVector.AttemptNormalize();
    }
  }

  VectorType result = u * start + v * perpendicularVector;
  return result;
}

/// Rotates one vector towards another with a max angle
template <typename type>
type GenericTowards(const type& a, const type& b, real maxAngle)
{
  const real cAngleEpsilon = real(0.0000001);

  real angle = AngleBetween(a, b);

  if (angle > Math::cPi)
  {
    angle -= Math::cTwoPi;
  }

  angle = Math::Abs(angle);
  if (angle > cAngleEpsilon)
  {
    real t = maxAngle / angle;
    if (t > real(1.0))
    {
      t = real(1.0);
    }
    return FastGeometricSlerp(a, b, t);
  }
  else
  {
    return b;
  }
}

/// Same as GenericTowards but SafeSlerp is used
template <typename type>
type SafeGenericTowards(const type& a, const type& b, real maxAngle)
{
  const real cAngleEpsilon = real(0.0000001);

  real angle = AngleBetween(a, b);

  if (angle > Math::cPi)
  {
    angle -= Math::cTwoPi;
  }

  angle = Math::Abs(angle);
  if (angle > cAngleEpsilon)
  {
    real t = maxAngle / angle;
    if (t > real(1.0))
    {
      t = real(1.0);
    }
    return SafeGeometricSlerp(a, b, t);
  }
  else
  {
    return b;
  }
}

} // namespace Math
