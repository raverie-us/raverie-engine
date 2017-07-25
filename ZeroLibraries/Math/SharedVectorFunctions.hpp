///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
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

/// Calculates the refraction vector through a plane given a certain index of refraction.
template <typename VectorType>
ZeroSharedTemplate VectorType GenericRefract(const VectorType& incidentVector, const VectorType& planeNormal, real refractionIndex)
{
  real iDotN = Math::Dot(incidentVector, planeNormal);
  real r = real(1.0) - refractionIndex * refractionIndex * (real(1.0) - iDotN * iDotN);
  if(r < 0)
    return VectorType::cZero;

  return refractionIndex * incidentVector - planeNormal * (refractionIndex * iDotN + Math::Sqrt(r));
}

/// Rotates one vector towards another with a max angle
template<typename type>
type GenericTowards(const type& a, const type& b, real maxAngle)
{
  const real cAngleEpsilon = real(0.0000001);

  real angle = AngleBetween(a, b);

  if(angle > Math::cPi)
  {
    angle -= Math::cTwoPi;
  }

  angle = Math::Abs(angle);
  if(angle > cAngleEpsilon)
  {
    real t = maxAngle / angle;
    if(t > real(1.0))
    {
      t = real(1.0);
    }
    return Slerp(a, b, t);
  }
  else
  {
    return b;
  }
}

/// Same as GenericTowards but SafeSlerp is used
template<typename type>
type SafeGenericTowards(const type& a, const type& b, real maxAngle)
{
  const real cAngleEpsilon = real(0.0000001);

  real angle = AngleBetween(a, b);

  if(angle > Math::cPi)
  {
    angle -= Math::cTwoPi;
  }

  angle = Math::Abs(angle);
  if(angle > cAngleEpsilon)
  {
    real t = maxAngle / angle;
    if(t > real(1.0))
    {
      t = real(1.0);
    }
    return SafeSlerp(a, b, t);
  }
  else
  {
    return b;
  }
}


}//namespace Math
