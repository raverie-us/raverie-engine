///////////////////////////////////////////////////////////////////////////////
///
///  \file Vector2Tests.cpp
///  Unit tests for Vector2.
///  
///  Authors: Benjamin Strukus
///  Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
typedef unsigned int uint;
#include "Math/Reals.hpp"
#include "Math/Vector2.hpp"

typedef Math::real    real;
typedef Math::Vector2 Vec2;

//----------------------------------------------------------- Zero (Static Data)
TEST(Vector2_Zero_StaticData)
{
  Vec2 expected(real(0.0), real(0.0));
  CHECK_VEC2(expected, Vec2::cZero);
}

//--------------------------------------------------------- X-Axis (Static Data)
TEST(Vector2_XAxis_StaticData)
{
  Vec2 expected(real(1.0), real(0.0));
  CHECK_VEC2(expected, Vec2::cXAxis);
}

//--------------------------------------------------------- Y-Axis (Static Data)
TEST(Vector2_YAxis_StaticData)
{
  Vec2 expected(real(0.0), real(1.0));
  CHECK_VEC2(expected, Vec2::cYAxis);
}

//------------------------------------------------------------------ Constructor
TEST(Vector2_Constructor)
{
  //Normal constructor
  real testArray[2] = { real(1.0), real(2.0) };
  Vec2 v1(real(1.0), real(2.0));
  CHECK_VEC2(testArray, v1);
}

//------------------------------------------------------------- Copy Constructor
TEST(Vector2_CopyConstructor)
{
  //Copy constructor
  Vec2 v1(real(1.0), real(2.0));
  Vec2 v2(v1);
  CHECK_VEC2(v1, v2);
}

//------------------------------------------------- Explicit Pointer Constructor
TEST(Vector2_ExplicitPointerConstructor)
{
  //Explicit pointer constructor
  real testData[2] = { real(0.0), real(1.0) };
  Vec2 vec(testData);
  CHECK_VEC2(testData, vec);
}

//----------------------------------------------------------- Initial Assignment
TEST(Vector2_InitialAssignment)
{
  //Initial assignment
  Vec2 vec(real(0.0), real(1.0));
  Vec2 v3 = vec;
  CHECK_VEC2(vec, v3);
}

//------------------------------------------------------- Subscript (Read/Write)
TEST(Vector2_Subscript_ReadWrite)
{
  //Modify
  real values[2] = { real(5.0), real(3.0) };
  Vec2 vec;
  vec[0] = real(5.0);
  vec[1] = real(3.0);

  CHECK_VEC2(values, vec);
}

//-------------------------------------------------------- Subscript (Read Only)
TEST(Vector2_Subscript_ReadOnly)
{
  //Non-modify
  real values[2];
  Vec2 vec(real(1.0), real(2.0));

  values[0] = vec[0];
  values[1] = vec[1];

  real expected[2] = { real(1.0), real(2.0) };
  CHECK_ARRAY_CLOSE(expected, values, 2, real(0.0));
}

//--------------------------------------------------------------- Unary Negation
TEST(Vector2_UnaryNegation)
{
  //Negation
  Vec2 vec(real(9.8), real(26.6));
  Vec2 neg = -vec;
  Vec2 expected(real(-9.8), real(-26.6));
  CHECK_VEC2(expected, neg);
}

//--------------------------------------------- Scalar Multiplication Assignment
TEST(Vector2_ScalarMultiplicationAssignment)
{
  //Multiplication assignment
  Vec2 vec(real(2.0), real(3.0));
  vec *= real(5.0);
  Vec2 expected(real(10.0), real(15.0));
  CHECK_VEC2(expected, vec);
}
//--------------------------------------------------- Scalar Division Assignment
TEST(Vector2_ScalarDivisionAssignment)
{
  //Division assignment
  Vec2 vec(real(10.0), real(15.0));
  vec /= real(10.0);
  Vec2 expected(real(1.0), real(1.5));
  CHECK_VEC2(expected, vec);
}

//-------------------------------------------------------- Scalar Multiplication
TEST(Vector2_ScalarMultiplication)
{
  //Multiplication with scalar
  Vec2 vec(real(2.0), real(3.0));
  Vec2 scaled = vec * real(5.0);
  Vec2 expected(real(10.0), real(15.0));
  CHECK_VEC2(expected, scaled);
}

//-------------------------------------------------------------- Scalar Division
TEST(Vector2_ScalarDivision)
{
  //Division by scalar
  Vec2 vec;
  Vec2 scaled(real(10.0), real(15.0));
  vec = scaled / real(10.0);
  Vec2 expected(real(1.0), real(1.5));
  CHECK_VEC2(expected, vec);
}

//--------------------------------------------------- Vector Addition Assignment
TEST(Vector2_VectorAdditionAssignment)
{
  //Addition assignment
  Vec2 vec(real(5.0), real(2.0));
  Vec2 add(real(1.0), real(2.0));
  vec += add;
  Vec2 expected(real(6.0), real(4.0));
  CHECK_VEC2(expected, vec);
}

//------------------------------------------------ Vector Subtraction Assignment
TEST(Vector2_VectorSubtractionAssignment)
{
  //Subtraction assignment
  Vec2 vec(real(5.0), real(2.0));
  Vec2 add(real(1.0), real(2.0));
  add -= vec;
  Vec2 expected(real(-4.0), real(0.0));
  CHECK_VEC2(expected, add);
}

//-------------------------------------------------------------- Vector Addition
TEST(Vector2_VectorAddition)
{
  //Addition
  Vec2 vec(real(5.0), real(7.0));
  Vec2 add(real(5.0), real(4.0));
  Vec2 result = vec + add;
  Vec2 expected(real(10.0), real(11.0));
  CHECK_VEC2(expected, result);
}

//----------------------------------------------------------- Vector Subtraction
TEST(Vector2_VectorSubtraction)
{
  //Subtraction
  Vec2 vec(real(5.0), real(7.0));
  Vec2 add(real(5.0), real(4.0));
  Vec2 result = vec - add;
  Vec2 expected(real(0.0), real(3.0));
  CHECK_VEC2(expected, result);
}

//------------------------------------------------------------- Equal Comparison
TEST(Vector2_EqualComparison)
{
  //Equality
  Vec2 vec(real(1.0), real(2.0));
  Vec2 cev(real(1.0), real(2.0));
  CHECK(vec == cev);
}

//--------------------------------------------------------- Not Equal Comparison
TEST(Vector2_NotEqualComparison)
{
  //Inequality
  Vec2 vec(real(1.0), real(2.0));
  Vec2 cev(real(3.0), real(2.0));
  CHECK(vec != cev);
}

//------------------------------------------------------- Componentwise Multiply
TEST(Vector2_ComponentwiseMultiply)
{
  Vec2 vec(real(8.0), real(-12.0));
  Vec2 cev(real(3.0), real(2.0));
  Vec2 result = vec * cev;
  Vec2 expected(real(24.0), real(-24.0));
  CHECK_VEC2(expected, result);
}

//--------------------------------------------------------- Componentwise Divide
TEST(Vector2_ComponentwiseDivide)
{
  Vec2 vec(real(-9.0), real(20.0));
  Vec2 cev(real(3.0), real(4.0));
  Vec2 result = vec / cev;
  Vec2 expected(real(-3.0), real(5.0));
  CHECK_VEC2(expected, result);
}

//-------------------------------------------------------------------------- Set
TEST(Vector2_Set)
{
  //I find this test funny because I've used Set in all the other tests
  Vec2 vec(real(0.0), real(1.0));
  vec.Set(real(6.0), real(9.0));
  Vec2 expected(real(6.0), real(9.0));
  CHECK_VEC2(expected, vec);
}

//--------------------------------------------------------------------- Zero Out
TEST(Vector2_ZeroOut)
{
  //Test to see if a vector can be zeroed
  Vec2 vec(real(6.0), real(2.0));
  vec.ZeroOut();
  Vec2 expected(real(0.0), real(0.0));
  CHECK_VEC2(expected, vec);
}

//---------------------------------------------------------------------- Reflect
TEST(ReflectAcrossVector)
{
  //Just do a 2D vector reflection
  Vec2 vec(real(1.0), real(1.0));
  Vec2 nrm(real(0.0), real(1.0));
  Vec2 reflect = vec.ReflectAcrossVector(nrm);
  Vec2 expected(real(-1.0), real(1.0));
  CHECK_VEC2(expected, reflect);
}

//---------------------------------------------------------------------- Reflect
TEST(ReflectAcrossPlane)
{
  //Just do a 2D plane reflection
  Vec2 vec(real(1.0), real(1.0));
  Vec2 nrm(real(0.0), real(1.0));
  Vec2 reflect = vec.ReflectAcrossPlane(nrm);
  Vec2 expected(real(1.0), real(-1.0));
  CHECK_VEC2(expected, reflect);
}

//------------------------------------------------------------ MultiplyAdd
TEST(Vector2_MultiplyAdd)
{
  //Add a vector that is scaled
  Vec2 vec(real(1.0), real(2.0));
  Vec2 cev(real(3.0), real(1.0));
  vec = Vec2::MultiplyAdd(vec, cev, real(5.0));
  Vec2 expected(real(16.0), real(7.0));
  CHECK_VEC2(expected, vec);
}

//------------------------------------------------------------ MultiplySubtract
TEST(Vector2_MultiplySubtract)
{
  //Add a vector that is scaled
  Vec2 vec(real(1.0), real(2.0));
  Vec2 cev(real(3.0), real(1.0));
  vec = Vec2::MultiplySubtract(vec, cev, real(5.0));
  Vec2 expected(real(-14.0), real(-3.0));
  CHECK_VEC2(expected, vec);
}

//------------------------------------------------------------------ Dot Product
TEST(Vector2_DotProduct)
{
  //Dot product!
  Vec2 vec(real(2.0), real(3.0));
  Vec2 cev(real(6.0), real(1.0));
  real dot = vec.Dot(cev);
  CHECK_EQUAL(real(15.0), dot);
}

//--------------------------------------------------------------------- Length 1
TEST(Vector2_Length_1)
{
  //First straight up length
  Vec2 vec(real(2.0), real(5.0));
  real length = vec.Length();
  CHECK_EQUAL(real(5.3851648071345040312507104915403), length);
}

//--------------------------------------------------------------------- Length 2
TEST(Vector2_Length_2)
{
  //More simple length, no tricks here
  Vec2 vec(real(1.0), real(0.0));
  real length = vec.Length();
  CHECK_EQUAL(real(1.0), length);
}

//------------------------------------------------------------- Length Squared 1
TEST(Vector2_LengthSquared_1)
{
  //Squared length
  Vec2 vec(real(2.0), real(5.0));
  real length = vec.LengthSq();
  CHECK_EQUAL(real(29.0), length);
}

//------------------------------------------------------------- Length Squared 2
TEST(Vector2_LengthSquared_2)
{
  //Simple length!
  Vec2 vec(real(1.0), real(0.0));
  real length = vec.LengthSq();
  CHECK_EQUAL(real(1.0), length);
}

//----------------------------------------------------------------- Normalized 1
TEST(Vector2_Normalized_1)
{
  //First norm, the values are SO close that this test should pass. Look at how
  //small that epsilon is!
  Vec2 vec(real(2.0), real(3.0));
  Vec2 norm = vec.Normalized();
  Vec2 expected(real(0.554700196225229122018341733457),
                real(0.8320502943378436830275126001855));
  CHECK_VEC2_CLOSE(expected, norm, real(0.000001));
}

//----------------------------------------------------------------- Normalized 2
TEST(Vector2_Normalized_2)
{
  //Stupid norm
  Vec2 vec(real(1.0), real(0.0));
  Vec2 norm = vec.Normalized();
  Vec2 expected(real(1.0), real(0.0));
  CHECK_VEC2(expected, norm);
}

//------------------------------------------------------------------ Normalize 1
TEST(Vector2_Normalize_1)
{
  //Normalized this.
  Vec2 vec(real(2.0), real(3.0));
  vec.Normalize();
  Vec2 expected(real(0.554700196225229122018341733457),
                real(0.8320502943378436830275126001855));
  CHECK_VEC2_CLOSE(expected, vec, real(0.000001));
}

//------------------------------------------------------------------ Normalize 2
TEST(Vector2_Normalize_2)
{
  //Stupid norm this.
  Vec2 vec(real(1.0), real(0.0));
  vec.Normalize();
  Vec2 expected(real(1.0), real(0.0));
  CHECK_VEC2(expected, vec);
}

//---------------------------------------------------------- Attempt Normalize 1
TEST(Vector2_AttemptNormalize_1)
{
 //Now attempt normalize! This should work.
  Vec2 vec(real(2.0), real(3.0));
  vec.AttemptNormalize();
  Vec2 expected(real(0.554700196225229122018341733457),
                real(0.8320502943378436830275126001855));
  CHECK_VEC2_CLOSE(expected, vec, real(0.000001));
}

//---------------------------------------------------------- Attempt Normalize 2
TEST(Vector2_AttemptNormalize_2)
{
  //This is why attempt normalize was made
  Vec2 vec(real(0.0), real(0.0));
  vec.AttemptNormalize();
  Vec2 expected(real(0.0), real(0.0));
  CHECK_VEC2(expected, vec);
}

//---------------------------------------------------------- Attempt Normalize 3
TEST(Vector2_AttemptNormalize_3)
{
  //Now for a really small vector
  Vec2 vec(real(0.0001), real(0.0001));
  vec.AttemptNormalize();
  Vec2 expected(real(0.70710678118654752440084436210485),
                real(0.70710678118654752440084436210485));
  CHECK_VEC2_CLOSE(expected, vec, real(0.000001));
}

//--------------------------------------------------------------------- Negate 1
TEST(Vector2_Negate_1)
{
  Vec2 vec(real(-1.0), real(20.0));
  vec.Negate();
  Vec2 expected(real(1.0), real(-20.0));
  CHECK_VEC2(expected, vec);
}

//--------------------------------------------------------------------- Negate 2
TEST(Vector2_Negate_2)
{
  Vec2 vec(real(87.0), real(92.0));
  Vec2 cev = vec.Negate();
  Vec2 expected(real(-87.0), real(-92.0));
  CHECK_VEC2(expected, cev);
}

//---------------------------------------------------------------------- Valid 1
TEST(Vector2_Valid_1)
{
  //Test to see if a vector is valid.
  Vec2 vec(real(0.0), real(1.0));  //Should be valid
  CHECK(vec.Valid());
}

//---------------------------------------------------------------------- Valid 2
TEST(Vector2_Valid_2)
{
  //Should not be valid!
  Vec2 vec(real(1.0), real(0.0));
  real zero = real(0.0);
  vec[0] /= zero;
  CHECK(vec.Valid() == false);
}

//------------------------------------------------- Global Scalar Multiplication
TEST(Vector2_Global_ScalarMultiplication)
{
  //Swapped order
  Vec2 vec(real(2.0), real(3.0));
  Vec2 scaled(real(0.0), real(0.0));
  scaled = real(5.0) * vec;
  Vec2 expected(real(10.0), real(15.0));
  CHECK_VEC2(expected, scaled);
}

//----------------------------------------------------------- Global Dot Product
TEST(Vector2_Global_DotProduct)
{
  //Standalone function
  Vec2 vec(real(2.0), real(3.0));
  Vec2 cev(real(6.0), real(1.0));
  real dot = Dot(vec, cev);
  CHECK_EQUAL(real(15.0), dot);
}

//-------------------------------------------------------------- Global Length 1
TEST(Vector2_Global_Length_1)
{
  //Standalone length
  Vec2 vec(real(2.0), real(5.0));
  real length = Length(vec);
  CHECK_EQUAL(real(5.3851648071345040312507104915403), length);
}

//-------------------------------------------------------------- Global Length 2
TEST(Vector2_Global_Length_2)
{
  //More simple length, no tricks here
  Vec2 vec(real(1.0), real(0.0));
  real length = vec.Length();
  CHECK_EQUAL(real(1.0), length);
}

//------------------------------------------------------ Global Length Squared 1
TEST(Vector2_Global_LengthSquared_1)
{
  //Squared length
  Vec2 vec(real(2.0), real(5.0));
  real length = LengthSq(vec);
  CHECK_EQUAL(real(29.0), length);
}

//------------------------------------------------------ Global Length Squared 2
TEST(Vector2_Global_LengthSquared_2)
{
  //Simple length!
  Vec2 vec(real(1.0), real(0.0));
  real length = LengthSq(vec);
  CHECK_EQUAL(real(1.0), length);
}

//---------------------------------------------------------- Global Normalized 1
TEST(Vector2_Global_Normalized_1)
{
  //Normalized using the standalone function
  Vec2 vec(real(2.0), real(3.0));
  Vec2 norm = Normalized(vec);
  Vec2 expected(real(0.554700196225229122018341733457),
                real(0.8320502943378436830275126001855));
  CHECK_VEC2_CLOSE(expected, norm, real(0.000001));
}

//---------------------------------------------------------- Global Normalized 2
TEST(Vector2_Global_Normalized_2)
{
  //Stupid norm
  Vec2 vec(real(1.0), real(0.0));
  Vec2 norm = Normalized(vec);
  Vec2 expected(real(1.0), real(0.0));
  CHECK_VEC2(expected, norm);
}

//----------------------------------------------------------- Global Normalize 1
TEST(Vector2_Global_Normalize_1)
{
  //Normalized this using standalone function
  Vec2 vec(real(2.0), real(3.0));
  Normalize(vec);
  Vec2 expected(real(0.554700196225229122018341733457),
                real(0.8320502943378436830275126001855));
  CHECK_VEC2_CLOSE(expected, vec, real(0.000001));
}

//----------------------------------------------------------- Global Normalize 2
TEST(Vector2_Global_Normalize_2)
{
  //Stupid norm this.
  Vec2 vec(real(1.0), real(0.0));
  Normalize(vec);
  Vec2 expected(real(1.0), real(0.0));
  CHECK_VEC2(expected, vec);
}

//--------------------------------------------------- Global Attempt Normalize 1
TEST(Vector2_Global_AttemptNormalize_1)
{
  //Now attempt normalize! This should work.
  Vec2 vec(real(2.0), real(3.0));
  AttemptNormalize(vec);
  Vec2 expected(real(0.554700196225229122018341733457),
                real(0.8320502943378436830275126001855));
  CHECK_VEC2_CLOSE(expected, vec, real(0.000001));
}

//--------------------------------------------------- Global Attempt Normalize 2
TEST(Vector2_Global_AttemptNormalize_2)
{
  //This is why attempt normalize was made
  Vec2 vec(real(0.0), real(0.0));
  AttemptNormalize(vec);
  Vec2 expected(real(0.0), real(0.0));
  CHECK_VEC2(expected, vec);
}

//--------------------------------------------------- Global Attempt Normalize 3
TEST(Vector2_Global_AttemptNormalize_3)
{
  //Now for a really small vector
  Vec2 vec(real(0.0001), real(0.0001));
  AttemptNormalize(vec);
  Vec2 expected(real(0.70710678118654752440084436210485),
                real(0.70710678118654752440084436210485));
  CHECK_VEC2_CLOSE(expected, vec, real(0.000001));
}

//---------------------------------------------------------------- Global Negate
TEST(Vector2_Global_Negate)
{
  Vec2 vec(real(-2.5), real(8.3));
  Negate(&vec);
  Vec2 expected(real(2.5), real(-8.3));
  CHECK_VEC2(expected, vec);
}

//--------------------------------------------------------------- Global Negated
TEST(Vector2_Global_Negated)
{
  Vec2 vec(real(93.2), real(-77.7));
  Vec2 cev = Negated(vec);
  Vec2 expected(real(-93.2), real(77.7));
  CHECK_VEC2(expected, cev);
}

//------------------------------------------------------------------- Global Abs
TEST(Vector2_Global_Abs)
{
  Vec2 vec(real(-2.0), real(30.0));
  Vec2 cev = Abs(vec);
  Vec2 expected(real(2.0), real(30.0));
  CHECK_VEC2(expected, cev);
}

//------------------------------------------------------------------- Global Min
TEST(Vector2_Global_Min)
{
  Vec2 vecA(real(-3.0), real(20.0));
  Vec2 vecB(real(-2.0), real(10.0));
  Vec2 result = Min(vecA, vecB);
  Vec2 expected(real(-3.0), real(10.0));
  CHECK_VEC2(expected, result);
}

//------------------------------------------------------------------- Global Max
TEST(Vector2_Global_Max)
{
  Vec2 vecA(real(33.3), real(-12.8));
  Vec2 vecB(real(33.33), real(7.6));
  Vec2 result = Max(vecA, vecB);
  Vec2 expected(real(33.33), real(7.6));
  CHECK_VEC2(expected, result);
}

//---------------------------------------------------------------- Global Lerp 1
TEST(Vector2_Global_Lerp_1)
{
  Vec2 vecA(real(1.0), real(0.0));
  Vec2 vecB(real(0.0), real(1.0));
  Vec2 result = Lerp(vecA, vecB, real(0.5));
  Vec2 expected(real(0.5), real(0.5));
  CHECK_VEC2(expected, result);
}

//---------------------------------------------------------------- Global Lerp 2
TEST(Vector2_Global_Lerp_2)
{
  Vec2 vecA(real(1.0), real(0.0));
  Vec2 vecB(real(0.0), real(1.0));
  Vec2 result = Lerp(vecA, vecB, real(0.0));
  Vec2 expected(real(1.0), real(0.0));
  CHECK_VEC2(expected, result);
}

//---------------------------------------------------------------- Global Lerp 3
TEST(Vector2_Global_Lerp_3)
{
  Vec2 vecA(real(1.0), real(0.0));
  Vec2 vecB(real(0.0), real(1.0));
  Vec2 result = Lerp(vecA, vecB, real(1.0));
  Vec2 expected(real(0.0), real(1.0));
  CHECK_VEC2(expected, result);
}
