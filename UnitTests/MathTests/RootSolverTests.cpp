///////////////////////////////////////////////////////////////////////////////
///
/// \file RootSolverTests.cpp
/// Unit tests for the root solving functions.
///
/// Authors: Benjamin Strukus
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
typedef unsigned int uint;
#include "Math/Reals.hpp"
#include "Math/Numerical.hpp"

typedef Math::real    real;
typedef Math::Vector2 Vec2;
typedef Math::Vector3 Vec3;
typedef Math::Vector4 Vec4;

TEST(QuadraticPolynomial_ZeroRoots)
{
  real roots[2];
  real a = real(3.0);
  real b = real(4.0);
  real c = real(3.0);
  uint count = Math::SolveQuadratic(c, b, a, roots);
  CHECK_EQUAL(0, count);
}

TEST(QuadraticPolynomial_OneRoot)
{
  real roots[2];
  real a = real(2.0);
  real b = real(4.0);
  real c = real(2.0);
  uint count = Math::SolveQuadratic(c, b, a, roots);
  CHECK_EQUAL(1, count);
  CHECK_EQUAL(real(-1.0), roots[0]);
}

TEST(QuadraticPolynomial_TwoRoots)
{
  real roots[2];
  real a = real(2.0);
  real b = real(20.0);
  real c = real(15.0);
  uint count = Math::SolveQuadratic(c, b, a, roots);
  CHECK_EQUAL(2, count);
  CHECK_CLOSE(real(-9.18330013267), roots[0], real(0.00001));
  CHECK_CLOSE(real(-0.81669986733), roots[1], real(0.00001));
}

TEST(CubicPolynomial_OneRoot)
{
  real roots[3];
  real a = real(0.5);
  real b = real(-0.2);
  real c = real(2.0);
  real d = real(-2.0);
  uint count = Math::SolveCubic(d, c, b, a, roots);
  CHECK_EQUAL(1, count);
  CHECK_CLOSE(real(0.89912403), roots[0], real(0.000001));
}

TEST(CubicPolynomial_TwoRoots)
{
  StubbedTest(CubicPolynomial_TwoRoots);
  /*
  real roots[3];
  real a = real(0.5);
  real b = real(-5.0);
  real c = real(2.0);
  real d = -real(2.0) + real(1.795807);
  uint count = Math::SolveCubic(d, c, b, a, roots);
  CHECK_EQUAL(3, count);
  CHECK_CLOSE(real(0.20638948), roots[0], real(0.000001));
  CHECK_CLOSE(real(9.587221), roots[1], real(0.000001));
  //*/
}

TEST(CubicPolynomial_ThreeRoots)
{
  real roots[3];
  real a = real(0.5);
  real b = real(-5.0);
  real c = real(2.0);
  real d = real(1.0);
  uint count = Math::SolveCubic(d, c, b, a, roots);
  CHECK_EQUAL(3, count);
  CHECK_CLOSE(real(-0.2874674), roots[0], real(0.00001));
  CHECK_CLOSE(real(0.72777569), roots[1], real(0.00001));
  CHECK_CLOSE(real(9.5596917), roots[2], real(0.00001));
}

TEST(QuarticPolynomial_ZeroRoots)
{
  real roots[4];
  real a = real(3.0);
  real b = real(-0.5);
  real c = real(-2.0);
  real d = real(2.0);
  real e = real(2.0);
  uint count = Math::SolveQuartic(e, d, c, b, a, roots);
  CHECK_EQUAL(0, count);
}

// TEST(QuarticPolynomial_OneRoot)
// {
//   real roots[4];
//   real a = real(3.0);
//   real b = real(-0.5);
//   real c = real(-2.0);
//   real d = real(2.0);
//   real e = real(2.0 - 0.51171707);
//   uint count = Math::SolveQuartic(e, d, c, b, a, roots);
//   CHECK_EQUAL(1, count);
//   CHECK_CLOSE(real(-0.6967419), roots[0], real(0.00001));
// }

TEST(QuarticPolynomial_TwoRoots)
{
  StubbedTest(QuarticPolynomial_TwoRoots);
//   real roots[4];
//   real a = real(3.0);
//   real b = real(-0.5);
//   real c = real(-5.0);
//   real d = real(2.0);
//   real e = real(1.0);
//   uint count = Math::SolveQuartic(e, d, c, b, a, roots);
//   CHECK_EQUAL(2, count);
//   CHECK_CLOSE(real(-1.326190), roots[0], real(0.00001));
//   CHECK_CLOSE(real(-0.2973087), roots[1], real(0.00001));
}

// TEST(QuarticPolynomial_ThreeRoots)
// {
// //   real roots[4];
// //   real a = real(3.0);
// //   real b = real(-0.1);
// //   real c = real(-5.0);
// //   real d = real(2.0);
// //   real e = real(1.0 - 0.57750882208);
// //   uint count = Math::QuarticPolynomial(a, b, c, d, e, roots);
// //   CHECK_EQUAL(3, count);
// //   CHECK_CLOSE(real(-1.420829), roots[0], real(0.00001));
// //   CHECK_CLOSE(real(-0.153416), roots[1], real(0.00001));
// //   CHECK_CLOSE(real(0.80378906), roots[2], real(0.00001));
//   real roots[4];
//   real a = real(3.0);
//   real b = real(-0.5);
//   real c = real(-8.0);
//   real d = real(2.0);
//   real e = real(2.0 - 2.1247559);
//   uint count = Math::QuarticPolynomial(a, b, c, d, e, roots);
//   CHECK_EQUAL(3, count);
//   CHECK_CLOSE(real(-1.673596), roots[0], real(0.00001));
//   CHECK_CLOSE(real(0.125), roots[1], real(0.00001));
//   CHECK_CLOSE(real(1.590263), roots[2], real(0.00001));
// }

TEST(QuarticPolynomial_FourRoots)
{
  StubbedTest(QuarticPolynomial_FourRoots);
//   real roots[4];
//   real a = real(3.0);
//   real b = real(-4.0);
//   real c = real(-5.0);
//   real d = real(2.0);
//   real e = real(1.0);
//   uint count = Math::SolveQuartic(e, d, c, b, a, roots);
//   CHECK_EQUAL(4, count);
//   CHECK_CLOSE(real(-0.8953482), roots[0], real(0.00001));
//   CHECK_CLOSE(real(-0.3229445), roots[1], real(0.00001));
//   CHECK_CLOSE(real(0.58669339), roots[2], real(0.00001));
//   CHECK_CLOSE(real(1.9649327), roots[3], real(0.00001));
}
