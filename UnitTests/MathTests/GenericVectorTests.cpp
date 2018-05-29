///////////////////////////////////////////////////////////////////////////////
///
///  \file Vector3Tests.cpp
///  Unit tests for Vector3.
///  
///  Authors: Benjamin Strukus
///  Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
typedef unsigned int uint;
#include "Math/Reals.hpp"
#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"
#include "Math/GenericVector.hpp"
#include <cmath>

typedef Math::real    real;
typedef Math::GenericVector<real,3> GenericVec3;

//------------------------------------------------- Explicit Pointer Constructor
TEST(GenericVec3_ExplicitPointerConstructor)
{
  //Explicit pointer constructor
  real testData[3] = { real(0.0), real(1.0), real(3.0) };
  GenericVec3 vec(testData);
  CHECK_VEC3(testData, vec);
}

//------------------------------------------------------- Subscript (Read/Write)
TEST(GenericVec3_Subscript_ReadWrite)
{
  //Modify
  real testArray[3] = { real(5.0), real(3.0), real(8.0) };
  GenericVec3 vec;
  vec[0] = real(5.0);
  vec[1] = real(3.0);
  vec[2] = real(8.0);
  CHECK_VEC3(testArray, vec);
}

//-------------------------------------------------------- Subscript (Read Only)
TEST(GenericVec3_Subscript_ReadOnly)
{
  //Non-modify
  real values[3];
  real loadValues[3] = { real(1.0), real(2.0), real(3.0) };
  GenericVec3 vec(loadValues);

  values[0] = vec[0];
  values[1] = vec[1];
  values[2] = vec[2];
  CHECK_VEC3(vec, values);
}

//--------------------------------------------------------------- Unary Negation
TEST(GenericVec3_UnaryOperators)
{
  //Negation
  real loadValues1[3] = {real(9.8), real(26.6), real(73.6)};
  GenericVec3 vec(loadValues1);
  GenericVec3 neg = -vec;
  real loadValues2[3] = {real(-9.8), real(-26.6), real(-73.6)};
  GenericVec3 expected(loadValues2);
  CHECK_VEC3(expected, neg);
}

//--------------------------------------------- Scalar Multiplication Assignment
TEST(GenericVec3_ScalarMultiplicationAssignment)
{
  //Multiplication assignment
  real loadValues1[3] = {real(2.0), real(3.0), real(6.0)};
  GenericVec3 vec(loadValues1);
  vec *= real(5.0);
  real loadValues2[3] = {real(10.0), real(15.0), real(30.0)};
  GenericVec3 expected(loadValues2);
  CHECK_VEC3(expected, vec);
}

//--------------------------------------------------- Scalar Division Assignment
TEST(GenericVec3_ScalarDivisionAssignment)
{
  //Division assignment
  real loadValues1[3] = {real(10.0), real(15.0), real(30.0)};
  GenericVec3 vec(loadValues1);
  vec /= real(10.0);
  real loadValues2[3] = {real(1.0), real(1.5), real(3.0)};
  GenericVec3 expected(loadValues2);
  CHECK_VEC3(expected, vec);
}

//-------------------------------------------------------- Scalar Multiplication
TEST(GenericVec3_ScalarMultiplication)
{
  //Multiplication with scalar
  real loadValues1[3] = {real(2.0), real(3.0), real(6.0)};
  GenericVec3 vec(loadValues1);
  GenericVec3 scaled = vec * real(5.0);
  CHECK_EQUAL(real(10.0), scaled[0]);
  CHECK_EQUAL(real(15.0), scaled[1]);
  CHECK_EQUAL(real(30.0), scaled[2]);
}

//-------------------------------------------------------------- Scalar Division
TEST(GenericVec3_ScalarDivision)
{
  //Division by scalar
  GenericVec3 vec;
  real loadValues1[3] = {real(10.0), real(15.0), real(30.0)};
  GenericVec3 scaled(loadValues1);
  vec = scaled / real(10.0);
  real loadValues2[3] = {real(1.0), real(1.5), real(3.0)};
  GenericVec3 expected(loadValues2);
  CHECK_VEC3(expected, vec);
}

//--------------------------------------------------- Vector Addition Assignment
TEST(GenericVec3_VectorAdditionAssignment)
{
  //Addition assignment
  real loadValues1[3] = {real(5.0), real(2.0), real(3.0)};
  GenericVec3 vec(loadValues1);
  real loadValues2[3] = {real(1.0), real(2.0), real(3.0)};
  GenericVec3 add(loadValues2);
  vec += add;
  real loadValues3[3] = {real(6.0), real(4.0), real(6.0)};
  GenericVec3 expected(loadValues3);
  CHECK_VEC3(expected, vec);
}

//------------------------------------------------ Vector Subtraction Assignment
TEST(GenericVec3_VectorSubtractionAssignment)
{
  //Subtraction assignment
  real loadValues1[3] = {real(5.0), real(2.0), real(3.0)};
  GenericVec3 vec(loadValues1);
  real loadValues2[3] = {real(1.0), real(2.0), real(3.0)};
  GenericVec3 add(loadValues2);
  add -= vec;
  real loadValues3[3] = {real(-4.0), real(0.0), real(0.0)};
  GenericVec3 expected(loadValues3);
  CHECK_VEC3(expected, add);
}

//-------------------------------------------------------------- Vector Addition
TEST(GenericVec3_VectorAddition)
{
  //Addition
  real loadValues1[3] = {real(5.0), real(7.0), real(8.0)};
  GenericVec3 vec(loadValues1);
  real loadValues2[3] = {real(5.0), real(4.0), real(8.0)};
  GenericVec3 add(loadValues2);
  GenericVec3 result = vec + add;
  real loadValues3[3] = {real(10.0), real(11.0), real(16.0)};
  GenericVec3 expected(loadValues3);
  CHECK_VEC3(expected, result);
}

//----------------------------------------------------------- Vector Subtraction
TEST(GenericVec3_VectorSubtraction)
{
  //Subtraction
  real loadValues1[3] = {real(5.0), real(7.0), real(8.0)};
  GenericVec3 vec(loadValues1);
  real loadValues2[3] = {real(5.0), real(4.0), real(8.0)};
  GenericVec3 add(loadValues2);
  GenericVec3 result = vec - add;
  real loadValues3[3] = {real(0.0), real(3.0), real(0.0)};
  GenericVec3 expected(loadValues3);
  CHECK_VEC3(expected, result);
}

//------------------------------- Vector Componentwise Multiplication Assignment
TEST(GenericVec3_VectorComponentwiseMultiplicationAssignment)
{
  //Componentwise multiplication assignment
  real loadValues1[3] = {real(3.0), real(-2.0), real(8.0)};
  GenericVec3 vec(loadValues1);
  real loadValues2[3] = {real(9.0), real(12.0), real(5.0)};
  GenericVec3 cev(loadValues2);
  vec *= cev;
  real loadValues3[3] = {real(27.0), real(-24.0), real(40.0)};
  GenericVec3 expected(loadValues3);
  CHECK_VEC3(expected, vec);
}

//------------------------------------------ Vector Componentwise Multiplication
TEST(GenericVec3_VectorComponentwiseMultiplication)
{
  //Componentwise multiplication
  real loadValues1[3] = {real(5.0), real(-9.0), real(-23.0)};
  GenericVec3 vec(loadValues1);
  real loadValues2[3] = {real(6.0), real(-2.0), real(7.0)};
  GenericVec3 cev(loadValues2);
  GenericVec3 result = vec * cev;
  real loadValues3[3] = {real(30.0), real(18.0), real(-161.0)};
  GenericVec3 expected(loadValues3);
  CHECK_VEC3(expected, result);
}

//------------------------------------------------ Vector Componentwise Division
TEST(GenericVec3_VectorComponentwiseDivision)
{
  //Componentwise division
  real loadValues1[3] = {real(3.0), real(12.0), real(22.0)};
  GenericVec3 vec(loadValues1);
  real loadValues2[3] = {real(10.0), real(4.0), real(11.0)};
  GenericVec3 cev(loadValues2);
  GenericVec3 result = vec / cev;
  real loadValues3[3] = {real(0.3), real(3.0), real(2.0)};
  GenericVec3 expected(loadValues3);
  CHECK_VEC3(expected, result);
}

//------------------------------------------------------------------ Dot Product
TEST(GenericVec3_DotProduct)
{
  //Dot product!
  real loadValues1[3] = {real(2.0), real(3.0), real(4.0)};
  GenericVec3 vec(loadValues1);
  real loadValues2[3] = {real(6.0), real(1.0), real(5.0)};
  GenericVec3 cev(loadValues2);
  real dot = vec.Dot(cev);
  CHECK_EQUAL(real(35.0), dot);
}

//------------------------------------------------------------- Length Squared 1
TEST(GenericVec3_LengthSquared_1)
{
  //Squared length
  real loadValues1[3] = {real(2.0), real(5.0), real(3.0)};
  GenericVec3 vec(loadValues1);
  real length = vec.LengthSq();
  CHECK_EQUAL(real(38.0), length);
}

//------------------------------------------------------------- Length Squared 2
TEST(GenericVec3_LengthSquared_2)
{
  //Simple length!
  real loadValues1[3] = {real(1.0), real(0.0), real(0.0)};
  GenericVec3 vec(loadValues1);
  real length = vec.LengthSq();
  CHECK_EQUAL(real(1.0), length);
}

//--------------------------------------------------------------------- Zero Out
TEST(GenericVec3_ZeroOut)
{
  //Test to see if a vector can be zeroed
  real loadValues1[3] = {real(6.0), real(2.0), real(3.0)};
  GenericVec3 vec(loadValues1);
  vec.ZeroOut();
  real loadValues2[3] = {real(0.0), real(0.0), real(0.0)};
  GenericVec3 expected(loadValues2);
  CHECK_VEC3(expected, vec);
}

//------------------------------------------------------------- Splat
TEST(GenericVec3_Splat)
{
  real testArray[3] = { real(5.0), real(5.0), real(5.0) };
  GenericVec3 vec;
  vec.Splat(real(5.0));
  CHECK_VEC3(testArray, vec);
}
