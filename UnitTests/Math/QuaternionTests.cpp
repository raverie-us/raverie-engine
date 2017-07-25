///////////////////////////////////////////////////////////////////////////////
///
///  \file QuaternionTests.cpp
///  Unit tests for Quaternions.
///  
///  Authors: Benjamin Strukus
///  Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
typedef unsigned int uint;
#include "Math/Reals.hpp"
#include "Math/Math.hpp"
#include "Math/Quaternion.hpp"
#include "Math/Matrix3.hpp"
#include "Math/Matrix4.hpp"
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"


typedef Math::real        real;
typedef Math::Matrix3     Mat3;
typedef Math::Matrix4     Mat4;
typedef Math::Vector3     Vec3;
typedef Math::Vector4     Vec4;
typedef Math::Quaternion  Quat;

#define QUAT(a, b, c, d) { real((a)), real((b)), real((c)), real((d)) };

//------------------------------------------------------- Identity (Static Data)
TEST(Quaternion_Identity_StaticData)
{
  Quat identity = Quat::cIdentity;
  real expected[4] = QUAT(0.0, 0.0, 0.0, 1.0);
  CHECK_QUAT(expected, identity);
}

//------------------------------------------------------------------ Constructor
TEST(Quaternion_Constructor)
{
  //Normal constructor
  Quat quat(real(1.0), real(2.0), real(3.0), real(4.0));
  real expected[4] = QUAT(1.0, 2.0, 3.0, 4.0);
  CHECK_QUAT(expected, quat);
}

//------------------------------------------------------------- Copy Constructor
TEST(Quaternion_CopyConstructor)
{
  //Copy constructor
  Quat quat(real(1.0), real(2.0), real(3.0), real(4.0));
  Quat copy(quat);
  real expected[4] = QUAT(1.0, 2.0, 3.0, 4.0);
  CHECK_QUAT(expected, copy);
}

//----------------------------------------------------------- Initial Assignment
TEST(Quaternion_InitialAssignment)
{
  //Assignment operator
  Quat copy(real(1.0), real(2.0), real(3.0), real(4.0));
  Quat assign = copy;
  real expected[4] = QUAT(1.0, 2.0, 3.0, 4.0);
  CHECK_QUAT(expected, assign);
}

//----------------------------------------------- Quaternion Addition Assignment
TEST(Quaternion_QuaternionAdditionAssignment)
{
  Quat quat(real(1.0), real(2.0), real(3.0), real(4.0));
  Quat add(real(5.0), real(6.0), real(7.0), real(8.0));

  //Addition assignment
  quat += add;
  real expected[4] = QUAT(6.0, 8.0, 10.0, 12.0);
  CHECK_QUAT(expected, quat);
}

//-------------------------------------------- Quaternion Subtraction Assignment
TEST(Quaternion_QuaternionSubtractionAssignment)
{
  Quat quat(real(6.0), real(8.0), real(10.0), real(12.0));
  Quat add(real(5.0), real(6.0), real(7.0), real(8.0));

  //Subtraction assignment
  add -= quat;
  real expected[4] = QUAT(-1.0, -2.0, -3.0, -4.0);
}

//----------------------------------------- Quaternion Multiplication Assignment
TEST(Quaternion_QuaternionMultiplicationAssignment)
{
  Quat quat(real(6.0), real(8.0), real(10.0), real(12.0));
  Quat add(real(-1.0), real(-2.0), real(-3.0), real(-4.0));

  //Multiplication assignment
  add *= quat;
  real expected[4] = QUAT(-32.0, -64.0, -72.0, 4.0);
}

//--------------------------------------------- Scalar Multiplication Assignment
TEST(Quaternion_ScalarMultiplicationAssignment)
{
  Quat quat(real(1.0), real(2.0), real(3.0), real(4.0));

  //Multiplication assignment
  quat *= real(5.0);
  real expected[4] = QUAT(5.0, 10.0, 15.0, 20.0);
}

//--------------------------------------------------- Scalar Division Assignment
TEST(Quaternion_ScalarDivisionAssignment)
{
  Quat quat(real(5.0), real(10.0), real(15.0), real(20.0));

  //Division assignment
  quat /= real(10.0);
  real expected[4] = QUAT(0.5, 1.0, 1.5, 2.0);
  CHECK_QUAT(expected, quat);
}

//--------------------------------------------------------------- Unary Negation
TEST(Quaternion_UnaryNegation)
{
  Quat quat(real(1.0), real(2.0), real(3.0), real(4.0));

  //Negation
  quat = -quat;
  real expected[4] = QUAT(-1.0, -2.0, -3.0, -4.0);
  CHECK_QUAT(expected, quat);
}

//---------------------------------------------------- Quaternion Multiplication
TEST(Quaternion_QuaternionMultiplication)
{
  Quat quat(real(5.0), real(3.0), real(9.0), real(10.0));
  Quat tauq(real(2.0), real(8.0), real(4.0), real(7.0));

  //Multiplication
  Quat temp = quat * tauq;
  real expected[4] = QUAT(-5.0, 99.0, 137.0, 0.0);
  CHECK_QUAT(expected, temp);
}

//---------------------------------------------------------- Quaternion Addition
TEST(Quaternion_QuaternionAddition)
{
  Quat quat(real(5.0), real(3.0), real(9.0), real(10.0));
  Quat tauq(real(2.0), real(8.0), real(4.0), real(7.0));

  //Addition
  Quat temp = quat + tauq;
  real expected[4] = QUAT(7.0, 11.0, 13.0, 17.0);
  CHECK_QUAT(expected, temp);
}

//------------------------------------------------------- Quaternion Subtraction
TEST(Quaternion_QuaternionSubtraction)
{
  Quat quat(real(5.0), real(3.0), real(9.0), real(10.0));
  Quat tauq(real(2.0), real(8.0), real(4.0), real(7.0));

  //Subtraction
  Quat temp = tauq - quat;
  real expected[4] = QUAT(-3.0, 5.0, -5.0, -3.0);
  CHECK_QUAT(expected, temp);
}

//-------------------------------------------------------- Scalar Multiplication
TEST(Quaternion_ScalarMultiplication)
{
  Quat quat(real(9.0), real(3.0), real(4.0), real(7.0));

  //Multiplication
  Quat temp = quat * real(5.0);
  real expected[4] = QUAT(45.0, 15.0, 20.0, 35.0);
}

//-------------------------------------------------------------- Scalar Division
TEST(Quaternion_ScalarDivision)
{
  Quat quat(real(9.0), real(3.0), real(4.0), real(7.0));

  //Division
  Quat temp = quat / real(10.0);
  real expected[4] = QUAT(9.0 / 10.0, 3.0 / 10.0, 4.0 / 10.0, 7.0 / 10.0);
  CHECK_QUAT_CLOSE(expected, temp, real(0.000001));
}

//------------------------------------------------------------- Equal Comparison
TEST(Quaternion_EqualComparison)
{
  Quat quat(real(1.0), real(2.0), real(3.0), real(4.0));
  Quat tauq(real(1.0), real(2.0), real(3.0), real(4.0));

  //Equality
  CHECK(quat == tauq);
}

//--------------------------------------------------------- Not Equal Comparison
TEST(Quaternion_NotEqualComparison)
{
  Quat quat(real(1.0), real(2.0), real(3.0), real(4.0));
  Quat tauq(real(5.0), real(2.0), real(3.0), real(4.0));

  //Inequality
  CHECK(quat != tauq);
}

//------------------------------------------------------- Subscript (Read/Write)
TEST(Quaternion_Subscript_ReadWrite)
{
  Quat quat(real(8.0), real(2.0), real(6.0), real(9.0));

  //Write
  quat[0] = real(27.0);
  quat[1] = real(50.0);
  quat[2] = real(91.0);
  quat[3] = real(25.0);

  real expected[4] = QUAT(27.0, 50.0, 91.0, 25.0);
  CHECK_QUAT(expected, quat);
}

//-------------------------------------------------------- Subscript (Read Only)
TEST(Quaternion_Subscript_ReadOnly)
{
  Quat quat(real(8.0), real(2.0), real(6.0), real(9.0));

  //Read
  real values[4] = { quat[0], quat[1], quat[2], quat[3] };
  real expected[4] = QUAT(8.0, 2.0, 6.0, 9.0);
  CHECK_QUAT(expected, values);
}

//-------------------------------------------------------------------------- Set
TEST(Quaternion_Set)
{
  Quat quat(real(1.0), real(2.0), real(3.0), real(4.0));
  quat.Set(real(8.0), real(32.0), real(85.2), real(5.0));

  real expected[4] = QUAT(8.0, 32.0, 85.2, 5.0);
  CHECK_QUAT(expected, quat);
}

//-------------------------------------------------------------------- Integrate
TEST(Quaternion_Integrate)
{
  Mat3 rotate;
  rotate.Rotate(real(1.0), real(0.0), real(0.0), Math::cPi * real(0.25));

  //Integrate

  //Orientation rotated about x-axis by pi/4
  Quat quat = Math::ToQuaternion(rotate);
  
  //Angular velocity (radians per sec)
  Vec3 deriv(real(1.0), real(0.0), real(0.0));
  quat = Quat::Integrate(quat, deriv, real(0.5));
  
  real expected[4] = QUAT(0.6136533154929114607605057813796, 0.0, 0.0,
                          0.82820867442001431319606819338919);
  CHECK_QUAT_CLOSE(expected, quat, real(0.000001));
}

//-------------------------------------------------------------------- Normalize
TEST(Quaternion_Normalize)
{
  Quat quat(real(1.0), real(2.0), real(3.0), real(4.0));

  //Normalize
  quat.Normalize();

  real expected[4] = QUAT(0.1825741858350553711523232609336,
                          0.3651483716701107423046465218672,
                          0.5477225575051661134569697828008,
                          0.73029674334022148460929304373441);
  CHECK_QUAT_CLOSE(expected, quat, real(0.000001));
}

//------------------------------------------------------------------- Normalized
TEST(Quaternion_Normalized)
{
  //Get normalized
  Quat quat(real(1.0), real(2.0), real(3.0), real(4.0));
  Quat norm = quat.Normalized();

  real expected[4] = QUAT(0.1825741858350553711523232609336,
                          0.3651483716701107423046465218672,
                          0.5477225575051661134569697828008,
                          0.73029674334022148460929304373441);
  CHECK_QUAT_CLOSE(expected, norm, real(0.000001));
}

//------------------------------------------------------------------ Dot Product
TEST(Quaternion_DotProduct)
{
  Quat quat(real(1.0), real(2.0), real(3.0), real(5.0));
  Quat tauq(real(2.0), real(3.0), real(5.0), real(7.0));

  //Dot product
  real dot = quat.Dot(tauq);
  CHECK_EQUAL(real(58.0), dot);
}

//----------------------------------------------------------------------- Length
TEST(Quaternion_Length)
{
  Quat quat(real(10.0), real(9.0), real(8.0), real(7.0));

  //Length
  real length = quat.Length();
  CHECK_EQUAL(real(17.146428199482246687380988522941), length);
}

//--------------------------------------------------------------- Length Squared
TEST(Quaternion_LengthSquared)
{
  Quat quat(real(10.0), real(9.0), real(8.0), real(7.0));

  //Length squared
  real lengthSquared = quat.LengthSq();
  CHECK_EQUAL(real(294.0), lengthSquared);
}

//-------------------------------------------------------------------- Conjugate
TEST(Quaternion_Conjugate)
{
  Quat quat(real(8.0), real(6.0), real(7.0), real(5.0));

  //Conjugate
  quat.Conjugate();
  real expected[4] = QUAT(-8.0, -6.0, -7.0, 5.0);
  CHECK_QUAT(expected, quat);
}

//------------------------------------------------------------------- Conjugated
TEST(Quaternion_Conjugated)
{
  //Get conjugated
  Quat quat(real(8.0), real(6.0), real(7.0), real(5.0));
  Quat tauq = quat.Conjugated();
  real expected[4] = QUAT(-8.0, -6.0, -7.0, 5.0);
  CHECK_QUAT(expected, tauq);
}

//--------------------------------------------------------------------- Invert 1
TEST(Quaternion_Invert_1)
{
  Quat quat(real(4.0), real(4.0), real(2.0), real(6.0));

  //Invert the quaternion
  quat.Invert();
  real expected[4] = QUAT(-0.05555555555555555555555555555556,
                          -0.05555555555555555555555555555556,
                          -0.02777777777777777777777777777778,
                           0.08333333333333333333333333333333);
  CHECK_QUAT(expected, quat);
}

//--------------------------------------------------------------------- Invert 2
TEST(Quaternion_Invert_2)
{
  Quat quat(real(1.0), real(0.0), real(0.0), real(0.0));

  //Invert the quaternion
  quat.Invert();
  real expected[4] = QUAT(-1.0, 0.0, 0.0, 0.0);
  CHECK_QUAT(expected, quat);
}

//--------------------------------------------------------------------- Inverted
TEST(Quaternion_Inverted)
{
  Quat quat(real(2.0), real(-3.0), real(5.0), real(7.0));

  //Get the inverse of the quaternion
  Quat tauq = quat.Inverted();

  real expected[4] = QUAT( -0.0229885057471264367816091954023,
                           0.03448275862068965517241379310345,
                          -0.05747126436781609195402298850575,
                           0.08045977011494252873563218390805);
  CHECK_QUAT(expected, tauq);
}

//--------------------------------------------------------------------- Exponent
TEST(Quaternion_Exponent)
{
  Quat quat(real(1.0), real(2.0), real(3.0), real(0.0));

  //Exponent
  Quat exp = Quat::Exponent(quat);
  real expected[4] = QUAT(-0.15092132721996450687853623013008,
                          -0.30184265443992901375707246026016,
                          -0.45276398165989352063560869039024,
                          -0.82529906207525863498963598672094);
  CHECK_QUAT_CLOSE(expected, exp, real(0.000001));
}

//------------------------------------------------------------------ Logarithm 1
TEST(Quaternion_Logarithm_1)
{
  Quat quat(real(1.0), real(2.0), real(3.0), real(0.5));

  //Logarithm
  Quat log = Quat::Logarithm(quat);
  real expected[4] = QUAT(1.2091995761561452337293855050948,
                          2.4183991523122904674587710101896,
                          3.6275987284684357011881565152844,
                          0.0);
  CHECK_QUAT_CLOSE(expected, log, real(0.000001));
}

//------------------------------------------------------------------ Logarithm 2
TEST(Quaternion_Logarithm_2)
{
  //Just to check...
  Quat quat(real(1.0), real(1.0), real(1.0), real(0.0));
  Quat exp = Quat::Exponent(quat);
  quat = Quat::Logarithm(exp);
  real expected[4] = QUAT(1.0, 1.0, 1.0, 0.0);
  CHECK_QUAT(expected, quat);
}

//---------------------------------------------------------------- Multiply
TEST(Quaternion_MultiplyVector1)
{
  Quat quat(real(0.0), real(1.0), real(0.0), real(0.0));
  Vec3 vec(real(1.0), real(1.0), real(1.0));

  //Rotate given vector
  vec = Math::Multiply(quat, vec);

  Vec3 expected(real(-1.0), real(1.0), real(-1.0));
  CHECK_VEC3(expected, vec);
}

//--------------------------------------------------------------- Multiply
TEST(Quaternion_MultiplyVector2)
{
  Quat quat(real(1.0), real(0.0), real(0.0), real(0.0));
  Vec3 vec(real(1.0), real(1.0), real(1.0));

  //Get rotated vector
  Vec3 cev = Math::Multiply(quat, vec);
  Vec3 expected(real(1.0), real(-1.0), real(-1.0));
  CHECK_VEC3(expected, cev);
}

//--------------------------------------------------------------------- Zero Out
TEST(Quaternion_SetIdentity)
{
  Quat quat(real(1.0), real(2.0), real(3.0), real(4.0));

  //Zero out the quaternion
  quat.SetIdentity();

  real expected[4] = QUAT(0.0, 0.0, 0.0, 1.0);
  CHECK_QUAT(expected, quat);
}

//---------------------------------------------------------------------- Valid 1
TEST(Quaternion_Valid_1)
{
  Quat quat(real(1.0), real(2.0), real(3.0), real(4.0));

  //Test to see if the quaternion is valid
  CHECK(quat.Valid());
}

//---------------------------------------------------------------------- Valid 2
TEST(Quaternion_Valid_2)
{
  //Test to see if the quaternion is invalid
  Quat quat(real(1.0), real(2.0), real(3.0), real(4.0));
  real zero = real(0.0);
  quat.x /= zero;
  CHECK(quat.Valid() == false);
}

//------------------------------------------------- Global Scalar Multiplication
TEST(Quaternion_Global_ScalarMultiplication)
{
  Quat quat(real(9.0), real(3.0), real(4.0), real(7.0));
  Quat temp = real(6.0) * quat;
  real expected[4] = QUAT(54.0, 18.0, 24.0, 42.0);
  CHECK_QUAT(expected, temp);
}

//------------------------------------------------------------- Global Normalize
TEST(Quaternion_Global_Normalize)
{
  //Normalize (stand alone function)
  Quat quat(real(2.0), real(3.0), real(5.0), real(7.0));
  Normalize(quat);
  real expected[4] = QUAT(0.21442250696755896656447070212231,
                          0.32163376045133844984670605318347,
                          0.53605626741889741641117675530578,
                          0.75047877438645638297564745742809);
  CHECK_QUAT_CLOSE(expected, quat, real(0.000001));
}

//------------------------------------------------------------ Global Normalized
TEST(Quaternion_Global_Normalized)
{
  //Get normalized (stand alone function)
  Quat quat(real(2.0), real(3.0), real(5.0), real(7.0));
  Quat norm = Normalized(quat);
  real expected[4] = QUAT(0.21442250696755896656447070212231,
                          0.32163376045133844984670605318347,
                          0.53605626741889741641117675530578,
                          0.75047877438645638297564745742809);
  CHECK_QUAT_CLOSE(expected, norm, real(0.000001));
}

//----------------------------------------------------------- Global Dot Product
TEST(Quaternion_Global_DotProduct)
{
  //Dot product (stand alone function)
  Quat quat(real(1.0), real(2.0), real(3.0), real(5.0));
  Quat tauq(real(2.0), real(3.0), real(5.0), real(7.0));

  real dot = Dot(tauq, quat);
  CHECK_EQUAL(real(58.0), dot);
}

//---------------------------------------------------------------- Global Length
TEST(Quaternion_Global_Length)
{
  //Length (stand alone function)
  Quat quat(real(10.0), real(9.0), real(8.0), real(7.0));
  real length = Length(quat);
  CHECK_EQUAL(real(17.146428199482246687380988522941), length);
}

//-------------------------------------------------------- Global Length Squared
TEST(Quaternion_Global_LengthSquared)
{
  Quat quat(real(10.0), real(9.0), real(8.0), real(7.0));
  //Length squared (stand alone function)
  real length = LengthSq(quat);
  CHECK_EQUAL(real(294.0), length);
}

//------------------------------------------------------------------ Global Lerp
TEST(Quaternion_Global_Lerp)
{
  //Linear interpolation
  Quat quat(real(1.0), real(2.0), real(3.0), real(4.0));
  Quat tauq(real(5.0), real(6.0), real(7.0), real(8.0));
  Quat lerp = Lerp(quat, tauq, real(0.25));
  real expected[4] = QUAT(0.27216552697590867757747600830065,
                          0.40824829046386301636621401245098,
                          0.54433105395181735515495201660131,
                          0.68041381743977169394369002075163);
  CHECK_QUAT(expected, lerp);
}

//----------------------------------------------------------------- Global Slerp
TEST(Quaternion_Global_Slerp)
{
  //Spherical linear interpolation
  Quat quat(real(0.0), real(0.0), real(0.70710678118654752440084436210485), 
            real(0.70710678118654752440084436210485));
  Quat tauq(real(0.0), real(0.0), real(0.0), real(1.0));
  Quat slerp = Slerp(quat, tauq, real(0.75));

  real expected[4] = QUAT(0.0, 0.0, 0.19509032201612826784828486847702,
                          0.98078528040323044912618223613423);
  CHECK_QUAT_CLOSE(expected, slerp,real(.00001));
}

#undef QUAT
