// MIT Licensed (see LICENSE.md).
#pragma once

#include "Reals.hpp"
#include "Vector2.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"

namespace Math
{

/// Solves the quadratic polynomial
///                        a2 * x^2 + a1 * x + a0 = 0
/// returns the number of real roots found and stores the roots (if any) in the
/// last parameter.
uint SolveQuadratic(real a0, real a1, real a2, real* roots);

/// Solves the cubic polynomial
///                 a3 * x^3 + a2 * x^2 + a1 * x + a0 = 0
/// returns the number of real roots found and stores the roots (if any) in the
/// last parameter.
uint SolveCubic(real a0, real a1, real a2, real a3, real* roots);

/// Solves the quartic polynomial
///             a4 * x^4 + a3 * x^3 + a2 * x^2 + a1 * x + a0 = 0
/// returns the number of real roots found and stores the roots (if any) in the
/// last parameter.
uint SolveQuartic(real a0, real a1, real a2, real a3, real a4, real* roots);

/// Evaluates the quadratic polynomial at the given x-value.
///                          a2 * x^2 + a1 * x + a0
real EvaluateQuadratic(real x, real a0, real a1, real a2);

/// Evaluates the cubic polynomial at the given x-value.
///                    a3 * x^3 + a2 * x^2 + a1 * x + a0
real EvaluateCubic(real x, real a0, real a1, real a2, real a3);

/// Evaluates the quartic polynomial at the given x-value.
///               a4 * x^4 + a3 * x^3 + a2 * x^2 + a1 * x + a0
real EvaluateQuartic(real x, real a0, real a1, real a2, real a3, real a4);

/// Evaluates the polynomial at the given x-value.
///             a[count] * x^(count) + ... + a[1] * x + a[0]
real EvaluatePolynomial(real x, real* coefficients, uint coefficientCount);

///

} // namespace Math
