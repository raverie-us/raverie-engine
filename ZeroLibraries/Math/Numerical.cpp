///////////////////////////////////////////////////////////////////////////////
///
///  \file Numerical.cpp
///  Contains the implementation of the functions that operate on numerical data
///  as functions.
///
///  Authors: Benjamin Strukus
///  Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Math/Numerical.hpp"

namespace Math
{

namespace
{

const uint cRootIterations = 2;

//------------------------------------------------------------------------------
real Cubed(real x)
{
  return x * x * x;
}

//------------------------------------------------------------------------------
real ToThe4th(real x)
{
  return x * x * x * x;
}

//------------------------------------------------------------------------------
real CubeRoot(real x)
{
  real sign = Math::GetSign(x);
  return sign * Math::Pow(Math::Abs(x), real(1.0 / 3.0));
}

//------------------------------------------------------------------------------
void MinMaxInPlace(real& x, real& y)
{
  if(y < x)
  {
    Swap(x, y);
  }
}

//------------------------------------------------------------------------------
void MinMaxInPlace(real& x, real& y, real& z)
{
  MinMaxInPlace(x, y);
  MinMaxInPlace(x, z);
  MinMaxInPlace(y, z);
}

//------------------------------------------------------------------------------
void MinMaxInPlace(real& x, real& y, real& z, real& w)
{
  MinMaxInPlace(x, y);
  MinMaxInPlace(x, z);
  MinMaxInPlace(x, w);
  MinMaxInPlace(y, z);
  MinMaxInPlace(y, w);
  MinMaxInPlace(z, w);
}

//------------------------------------------------------------------------------
void MinMaxInPlace(uint count, real* values)
{
  switch(count)
  {
    case 2:
    {
      MinMaxInPlace(values[0], values[1]);
    }
    break;

    case 3:
    {
      MinMaxInPlace(values[0], values[1], values[2]);
    }
    break;

    case 4:
    {
      MinMaxInPlace(values[0], values[1], values[2], values[3]);
    }
    break;
  }
}

}// namespace

///Evaluates the quadratic polynomial at the given x-value.
///                          a2 * x^2 + a1 * x + a0
real EvaluateQuadratic(real x, real a0, real a1, real a2)
{
  return a0 + x * (a1 + x * a2);
}

///Evaluates the cubic polynomial at the given x-value.
///                    a3 * x^3 + a2 * x^2 + a1 * x + a0
real EvaluateCubic(real x, real a0, real a1, real a2, real a3)
{
  return a0 + x * (a1 + x * (a2 + x * a3));
}

///Evaluates the quartic polynomial at the given x-value.
///               a4 * x^4 + a3 * x^3 + a2 * x^2 + a1 * x + a0
real EvaluateQuartic(real x, real a0, real a1, real a2, real a3, real a4)
{
  return a0 + x * (a1 + x * (a2 + x * (a3 + x * a4)));
}

///Evaluates the polynomial at the given x-value.
///             a[count] * x^(count) + ... + a[1] * x + a[0]
real EvaluatePolynomial(real x, real* coefficients, uint coefficientCount)
{
  ErrorIf(coefficientCount == 0, "Math - No coefficients passed for the " \
                                 "polynomial.");
  ErrorIf(coefficients == nullptr, "Math - No coefficients passed for the "  \
                                "polynomial.");

  uint n = coefficientCount - 1;
  real result = coefficients[n];
  for(int i = n - 1; i > -1; --i)
  {
    result = coefficients[i] + x * result;
  }
  return result;
}

///Solves the quadratic polynomial
///                        a2 * x^2 + a1 * x + a0 = 0
///returns the number of real roots found and stores the roots (if any) in the 
///last parameter. If the last parameter is null, the roots will not be 
///calculated.
uint SolveQuadratic(real a0, real a1, real a2, real* roots)
{
  real p = a1 / (real(2.0) * a2);
  real q = a0 / a2;
  real discr = p * p - q;
  if(Math::IsZero(discr))
  {
    if(roots != nullptr)
    {
      roots[0] = -p;
    }    
    return 1;
  }
  else if(discr > real(0.0))
  {
    if(roots != nullptr)
    {
      discr = Math::Sqrt(discr);
      roots[0] = -discr - p;
      roots[1] =  discr - p;
    }
    return 2;
  }
  return 0;
}

///Solves the cubic polynomial 
///                 a3 * x^3 + a2 * x^2 + a1 * x + a0 = 0
///returns the number of real roots found and stores the roots (if any) in the 
///last parameter. If the last parameter is null, the roots will not be 
///calculated.
uint SolveCubic(real a0, real a1, real a2, real a3, real* roots)
{
  real r[3] = { real(0.0) };
  uint rootCount = 0;

  //Solve the cubic equation
  if(a3 != real(0.0))
  {
    const real inv3 = real(1.0 / 3.0);
    const real invA = real(1.0) / a3;
    const real s = -a2 * inv3 * invA;
    const real ss = s * s;
    real p = a1 * inv3 * invA - ss;
    p = p * p * p;
    real q = real(0.5) * (real(2.0) * ss * s - (a1 * s + a0) * invA);
    real discr = q * q + p;
    if(discr < real(0.0))
    {
      if(roots == nullptr)
      {
        return 3;
      }

      rootCount = 3;
      real arg = q / Math::Sqrt(-p);
      real phi;
      //This can be optimized to remove the trig functions!
      if(arg < real(-1.0))
      {
        phi = cPi * inv3;
      }
      else if(arg > real(1.0))
      {
        phi = real(0.0);
      }
      else
      {
        phi = Math::ArcCos(arg) * inv3;
      }
      p = real(2.0) * Math::Pow(-p, real(1.0 / 6.0));
      r[0] = p * Math::Cos(phi) + s;
      r[1] = p * Math::Cos(phi + real(2.0 / 3.0) * cPi) + s;
      r[2] = p * Math::Cos(phi + real(4.0 / 3.0) * cPi) + s;
    }
    else
    {
      if(roots == nullptr)
      {
        return 1;
      }

      rootCount = 1;
      discr = Math::Sqrt(discr);
      r[0] = CubeRoot(q + discr) + CubeRoot(q - discr) + s;
    }
  }
  //Solve the quadratic equation
  else if(a2 != real(0.0))
  {
    real invB = real(1.0) / a2;
    real p = real(0.5) * a1 * invB;
    real discr = p * p - a0 * invB;
    if(discr > real(0.0))
    {
      if(roots == nullptr)
      {
        return 2;
      }

      rootCount = 2;
      discr = Sqrt(discr);
      r[0] = -discr - p;
      r[1] =  discr - p;
    }
    else
    {
      return 0;
    }
  }
  //Solve the linear equation
  else if(a1 != real(0.0))
  {
    if(roots == nullptr)
    {
      return 1;
    }

    rootCount = 1;
    r[0] = a0 / a1;
  }

  const real two = real(2.0);
  const real three = real(3.0);
  roots[2] = r[2];
  roots[1] = r[1];
  roots[0] = r[0];
  MinMaxInPlace(rootCount, roots);
  //Clean up the roots with one iteration of Newton's method.
  switch(rootCount)
  {
    case 3:
      roots[2] = roots[2] - Math::EvaluateCubic(roots[2], a0, a1, a2, a3) / 
                 Math::EvaluateQuadratic(roots[2], a1, two * a2, three * a3);
    case 2:
      roots[1] = roots[1] - Math::EvaluateCubic(roots[1], a0, a1, a2, a3) /
                 Math::EvaluateQuadratic(roots[1], a1, two * a2, three * a3);
    case 1:
      roots[0] = roots[0] - Math::EvaluateCubic(roots[0], a0, a1, a2, a3) /
                 Math::EvaluateQuadratic(roots[0], a1, two * a2, three * a3);
    case 0:
      break;
  }
  return rootCount;
}

bool NearZero(float value)
{
  return Math::Abs(value) < 0.00001f;
}

///Solves the quartic polynomial 
///             a4 * x^4 + a3 * x^3 + a2 * x^2 + a1 * x + a0 = 0
///returns the number of real roots found and stores the roots (if any) in the 
///last parameter. If the last parameter is null, the roots will not be 
///calculated.
uint SolveQuartic(real a0, real a1, real a2, real a3, real a4, real* roots)
{
  if(a4 == real(0.0))
  {
    return SolveCubic(a0, a1, a2, a3, roots);
  }

  if(a4 != real(1.0))
  {
    a3 /= a4;
    a2 /= a4;
    a1 /= a4;
    a0 /= a4;
  }

  real p = real(-3.0 / 8.0) * Math::Sq(a3) + a2;
  real q = Cubed(a3) / real(8.0) - (a3 * a2) / real(2.0) + a1;
  real r = real(-3.0 / 256.0) * Math::Sq(a3) * Math::Sq(a3) + 
           (Math::Sq(a3) * a2) / real(16.0) - (a3 * a1) / real(4.0) + a0;


  uint rootCount = 0;
  //Find solution to: y^3 - p/2 * y^2 - r * y + (4 * r * p - q^2)/8 = 0
  real c = (real(4.0) * r * p - (q * q)) / real(8.0);
  rootCount = SolveCubic(c, -r, -p / real(2.0), real(1.0), roots);

  //Use the result of the cubic polynomial to solve the two quadratics
  const real z = roots[0];

  real u = z * z - r;
  real v = real(2.0) * z - p;

  if(NearZero(u))
    u = 0.0f;
  else if(u > real(0.0))
    u = Math::Sqrt(u);
  else
    return 0;

  if(NearZero(v))
    v = 0.0f;
  else if(v > real(0.0))
    v = Math::Sqrt(v);
  else
    return 0;

  //Solve the roots for the two quadratic equations
  if(q >= real(0.0))
  {
    rootCount  = SolveQuadratic(z - u,  v, real(1.0), roots);
    rootCount += SolveQuadratic(z + u, -v, real(1.0), 
                                roots + (roots == nullptr ? 0 : rootCount));
  }
  else
  {
    rootCount  = SolveQuadratic(z + u,  v, real(1.0), roots);
    rootCount += SolveQuadratic(z - u, -v, real(1.0),
                                roots + (roots == nullptr ? 0 : rootCount));
  }
  if(roots == nullptr)
  {
    return rootCount;
  }

  //Get the roots in the order of most negative to most positive
  MinMaxInPlace(rootCount, roots);
 
  //Calculate the t-values for the quartic roots
  for(uint i = 0; i < rootCount; ++i)
  {
    roots[i] -= a3 / real(4.0);
  }

  //Clean up the roots with one iteration of Newton's method.
  const real two = real(2.0);
  const real three = real(3.0);
  const real four = real(4.0);
  for(uint i = 0; i < cRootIterations; ++i)
  {
    switch(rootCount)
    {
      case 4:
        roots[3] = roots[3] - 
                   Math::EvaluateQuartic(roots[3], a0, a1, a2, a3, a4) / 
                   Math::EvaluateCubic(roots[3], a1, two * a2, three * a3, 
                                       four * a4);
      case 3:
        roots[2] = roots[2] - 
                   Math::EvaluateQuartic(roots[2], a0, a1, a2, a3, a4) / 
                   Math::EvaluateCubic(roots[2], a1, two * a2, three * a3, 
                                       four * a4);
      case 2:
        roots[1] = roots[1] - 
                   Math::EvaluateQuartic(roots[1], a0, a1, a2, a3, a4) / 
                   Math::EvaluateCubic(roots[1], a1, two * a2, three * a3, 
                                       four * a4);
      case 1:
        roots[0] = roots[0] - 
                   Math::EvaluateQuartic(roots[0], a0, a1, a2, a3, a4) / 
                   Math::EvaluateCubic(roots[0], a1, two * a2, three * a3, 
                                       four * a4);
        break;

      case 0:
        return 0;        
    }
  }
  return rootCount;
}
}// namespace Math
