///////////////////////////////////////////////////////////////////////////////
///
///  \file Vector4Tests.cpp
///  Unit tests for Vector4.
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
#include "Math/Vector4.hpp"


typedef Math::real    real;
typedef Math::Vector2 Vec2;
typedef Math::Vector3 Vec3;
typedef Math::Vector4 Vec4;

//----------------------------------------------------------- Zero (Static Data)
TEST(Vector4_Zero_StaticData)
{
  Vec4 expected(real(0.0), real(0.0), real(0.0), real(0.0));
  CHECK_VEC4(expected, Vec4::cZero);
}

//--------------------------------------------------------- X-Axis (Static Data)
TEST(Vector4_XAxis_StaticData)
{
  Vec4 expected(real(1.0), real(0.0), real(0.0), real(0.0));
  CHECK_VEC4(expected, Vec4::cXAxis);
}

//--------------------------------------------------------- Y-Axis (Static Data)
TEST(Vector4_YAxis_StaticData)
{
  Vec4 expected(real(0.0), real(1.0), real(0.0), real(0.0));
  CHECK_VEC4(expected, Vec4::cYAxis);
}

//--------------------------------------------------------- Z-Axis (Static Data)
TEST(Vector4_ZAxis_StaticData)
{
  Vec4 expected(real(0.0), real(0.0), real(1.0), real(0.0));
  CHECK_VEC4(expected, Vec4::cZAxis);
}

//--------------------------------------------------------- W-Axis (Static Data)
TEST(Vector4_WAxis_StaticData)
{
  Vec4 expected(real(0.0), real(0.0), real(0.0), real(1.0));
  CHECK_VEC4(expected, Vec4::cWAxis);
}
//------------------------------------------------------------------ Constructor
TEST(Vector4_Constructor)
{
  //Normal constructor
  Vec4 v1(real(1.0), real(2.0), real(3.0), real(4.0));
  real expected[4] = { real(1.0), real(2.0), real(3.0), real(4.0) };
  CHECK_VEC4(expected, v1);
}

//------------------------------------------------------------- Copy Constructor
TEST(Vector4_CopyConstructor)
{
  //Copy constructor
  Vec4 v1(real(1.0), real(2.0), real(3.0), real(4.0));
  Vec4 v2(v1);
  CHECK_VEC4(v1, v2);
}

//------------------------------------------------- Explicit Pointer Constructor
TEST(Vector4_ExplicitPointerConstructor)
{
  //Explicit pointer constructor
  real testData[4] = { real(0.0), real(1.0), real(3.0), real(5.0) };
  Vec4 vec(testData);
  CHECK_VEC4(testData, vec);
}

//----------------------------------------------------------- Initial Assignment
TEST(Vector4_InitialAssignment)
{
  //Initial assignment
  Vec4 vec(real(0.0), real(1.0), real(3.0), real(5.0));
  Vec4 v3 = vec;
  CHECK_VEC4(vec, v3);
}

//------------------------------------------------------- Subscript (Read/Write)
TEST(Vector4_Subscript_ReadWrite)
{
  //Modify
  Vec4 vec(real(0.0), real(0.0), real(0.0), real(0.0));
  vec[0] = real(5.0);
  vec[1] = real(3.0);
  vec[2] = real(8.0);
  vec[3] = real(6.0);
  real values[4] = { real(5.0), real(3.0), real(8.0), real(6.0) };
  CHECK_VEC4(values, vec);
}

//-------------------------------------------------------- Subscript (Read Only)
TEST(Vector4_Subscript_ReadOnly)
{
  //Non-modify
  real values[4];
  Vec4 vec(real(1.0), real(2.0), real(3.0), real(4.0));

  values[0] = vec[0];
  values[1] = vec[1];
  values[2] = vec[2];
  values[3] = vec[3];

  CHECK_VEC4(vec, values);
}

//--------------------------------------------------------------- Unary Negation
TEST(Vector4_UnaryNegation)
{
  //Negation
  Vec4 vec(real(9.8), real(26.6), real(73.6), real(24.0));
  Vec4 neg = -vec;
  Vec4 expected(real(-9.8), real(-26.6), real(-73.6), real(-24.0));
  CHECK_VEC4(expected, neg);
}

//--------------------------------------------- Scalar Multiplication Assignment
TEST(Vector4_ScalarMultiplicationAssignment)
{  
  //Multiplication assignment
  Vec4 vec(real(2.0), real(3.0), real(6.0), real(18.0));
  vec *= real(5.0);
  Vec4 expected(real(10.0), real(15.0), real(30.0), real(90.0));
  CHECK_VEC4(expected, vec);
}

//--------------------------------------------------- Scalar Division Assignment
TEST(Vector4_ScalarDivisionAssignment)
{
  //Division assignment
  Vec4 vec(real(10.0), real(15.0), real(30.0), real(90.0));
  vec /= real(10.0);
  Vec4 expected(real(1.0), real(1.5), real(3.0), real(9.0));
  CHECK_VEC4_CLOSE(expected, vec, real(0.000001));
}

//-------------------------------------------------------- Scalar Multiplication
TEST(Vector4_ScalarMultiplication)
{
  //Multiplication with scalar
  Vec4 vec(real(2.0), real(3.0), real(6.0), real(7.0));
  Vec4 scaled = vec * real(5.0);
  Vec4 expected(real(10.0), real(15.0), real(30.0), real(35.0));
  CHECK_VEC4(expected, scaled);
}

//-------------------------------------------------------------- Scalar Division
TEST(Vector4_ScalarDivision)
{
  //Division by scalar
  Vec4 vec(real(0.0), real(0.0), real(0.0), real(0.0));
  Vec4 scaled(real(10.0), real(15.0), real(30.0), real(35.0));
  vec = scaled / real(10.0);
  Vec4 expected(real(1.0), real(1.5), real(3.0), real(3.5));
  CHECK_VEC4_CLOSE(expected, vec, real(0.000001));
}

//--------------------------------------------------- Vector Addition Assignment
TEST(Vector4_VectorAdditionAssignment)
{
  //Addition assignment
  Vec4 vec(real(5.0), real(2.0), real(3.0), real(-1.0));
  Vec4 add(real(1.0), real(2.0), real(3.0), real(4.0));
  vec += add;
  Vec4 expected(real(6.0), real(4.0), real(6.0), real(3.0));
  CHECK_VEC4(expected, vec);
}

//------------------------------------------------ Vector Subtraction Assignment
TEST(Vector4_VectorSubtractionAssignment)
{
  //Subtraction assignment
  Vec4 vec(real(5.0), real(2.0), real(3.0), real(-1.0));
  Vec4 add(real(1.0), real(2.0), real(3.0), real(4.0));
  add -= vec;
  Vec4 expected(real(-4.0), real(0.0), real(0.0), real(5.0));
  CHECK_VEC4(expected, add);
}

//-------------------------------------------------------------- Vector Addition
TEST(Vector4_VectorAddition)
{
  //Addition
  Vec4 vec(real(5.0), real(7.0), real(8.0), real(10.0));
  Vec4 add(real(5.0), real(4.0), real(8.0), real(3.0));
  Vec4 result = vec + add;
  Vec4 expected(real(10.0), real(11.0), real(16.0), real(13.0));
  CHECK_VEC4(expected, result);
}

//----------------------------------------------------------- Vector Subtraction
TEST(Vector4_VectorSubtraction)
{
  //Subtraction
  Vec4 vec(real(5.0), real(7.0), real(8.0), real(10.0));
  Vec4 add(real(5.0), real(4.0), real(8.0), real(3.0));
  Vec4 result = vec - add;
  Vec4 expected(real(0.0), real(3.0), real(0.0), real(7.0));
  CHECK_VEC4(expected, result);
}

//------------------------------------------------------------- Equal Comparison
TEST(Vector4_EqualComparison)
{
  //Equality
  Vec4 vec(real(1.0), real(2.0), real(3.0), real(4.0));
  Vec4 cev(real(1.0), real(2.0), real(3.0), real(4.0));
  CHECK(vec == cev);
}

//--------------------------------------------------------- Not Equal Comparison
TEST(Vector4_NotEqualComparison)
{
  //Inequality
  Vec4 vec(real(1.0), real(2.0), real(3.0), real(4.0));
  Vec4 cev(real(3.0), real(2.0), real(3.0), real(8.0));
  CHECK(vec != cev);
}

//------------------------------------------ Vector Componentwise Multiplication
TEST(Vector4_VectorComponentwiseMultiplication)
{
  Vec4 vec(real(3.0), real(-2.0), real(8.0), real(7.0));
  Vec4 cev(real(9.0), real(12.0), real(5.0), real(22.0));
  Vec4 result = vec * cev;
  Vec4 expected(real(27.0), real(-24.0), real(40.0), real(154.0));
  CHECK_VEC4(expected, result);
}

//------------------------------------------------ Vector Componentwise Division
TEST(Vector4_VectorComponentwiseDivision)
{
  //Componentwise division
  Vec4 vec(real(3.0), real(12.0), real(22.0), real(8.0));
  Vec4 cev(real(10.0), real(4.0), real(11.0), real(8.0));
  Vec4 result = vec / cev;
  Vec4 expected(real(0.3), real(3.0), real(2.0), real(1.0));
  CHECK_VEC4_CLOSE(expected, result, real(0.000001));
}

//-------------------------------------------------------------------------- Set
TEST(Vector4_Set)
{
  //I find this test funny because I've used Set in all the other tests
  Vec4 vec(real(0.0), real(1.0), real(6.0), real(98.0));
  vec.Set(real(6.0), real(9.0), real(20.0), real(26.0));
  Vec4 expected(real(6.0), real(9.0), real(20.0), real(26.0));
  CHECK_VEC4(expected, vec);
}

//--------------------------------------------------------------------- Zero Out
TEST(Vector4_ZeroOut)
{
  //Test to see if a vector can be zeroed
  Vec4 vec(real(6.0), real(2.0), real(3.0), real(1.0));
  vec.ZeroOut();
  Vec4 expected(real(0.0), real(0.0), real(0.0), real(0.0));
  CHECK_VEC4(expected, vec);
}

//------------------------------------------------------------------ Dot Product
TEST(Vector4_DotProduct)
{
  //Dot product!
  Vec4 vec(real(2.0), real(3.0), real(4.0), real(5.0));
  Vec4 cev(real(6.0), real(1.0), real(5.0), real(4.0));
  real dot = vec.Dot(cev);
  CHECK_EQUAL(real(55.0), dot);
}

//--------------------------------------------------------------------- Length 1
TEST(Vector4_Length_1)
{
  //First straight up length
  Vec4 vec(real(2.0), real(5.0), real(3.0), real(6.0));
  real length = vec.Length();
  CHECK_EQUAL(real(8.6023252670426267717294735350497), length);
}

//--------------------------------------------------------------------- Length 2
TEST(Vector4_Length_2)
{
  //More simple length, no tricks here
  Vec4 vec(real(1.0), real(0.0), real(0.0), real(0.0));
  real length = vec.Length();
  CHECK_EQUAL(real(1.0), length);
}

//--------------------------------------------------------------------- Length 3
TEST(Vector4_Length_3)
{
  Vec4 vec(real(0.0), real(0.0), real(0.0), real(0.0));
  real length = vec.Length();
  CHECK_EQUAL(real(0.0), length);
}

//------------------------------------------------------------- Length Squared 1
TEST(Vector4_LengthSquared_1)
{
  //Squared length
  Vec4 vec(real(2.0), real(5.0), real(3.0), real(6.0));
  real length = vec.LengthSq();
  CHECK_EQUAL(real(74.0), length);
}

//------------------------------------------------------------- Length Squared 2
TEST(Vector4_LengthSquared_2)
{
  //Simple length!
  Vec4 vec(real(1.0), real(0.0), real(0.0), real(0.0));
  real length = vec.LengthSq();
  CHECK_EQUAL(real(1.0), length);
}

//----------------------------------------------------------------- Normalized 1
TEST(Vector4_Normalized_1)
{
  //First norm, the values are SO close that this test should pass. Look at how
  //small that epsilon is!
  Vec4 vec(real(2.0), real(3.0), real(4.0), real(5.0));
  Vec4 norm = vec.Normalized();
  Vec4 expected(real(0.27216552697590867757747600830065),
                real(0.40824829046386301636621401245098),
                real(0.54433105395181735515495201660131),
                real(0.68041381743977169394369002075163));
  CHECK_VEC4_CLOSE(expected, norm, real(0.000001));
}

//----------------------------------------------------------------- Normalized 2
TEST(Vector4_Normalized_2)
{
  //Stupid norm
  Vec4 vec(real(1.0), real(0.0), real(0.0), real(0.0));
  Vec4 norm = vec.Normalized();
  Vec4 expected(real(1.0), real(0.0), real(0.0), real(0.0));
  CHECK_VEC4_CLOSE(expected, norm, real(0.000001));
}

//------------------------------------------------------------------ Normalize 1
TEST(Vector4_Normalize_1)
{
  //Normalized this.
  Vec4 vec(real(2.0), real(3.0), real(4.0), real(5.0));
  vec.Normalize();
  Vec4 expected(real(0.27216552697590867757747600830065),
                real(0.40824829046386301636621401245098),
                real(0.54433105395181735515495201660131),
                real(0.68041381743977169394369002075163));
  CHECK_VEC4_CLOSE(expected, vec, real(0.000001));
}

//------------------------------------------------------------------ Normalize 2
TEST(Vector4_Normalize_2)
{
  //Stupid norm this.
  Vec4 vec(real(1.0), real(0.0), real(0.0), real(0.0));
  vec.Normalize();
  Vec4 expected(real(1.0), real(0.0), real(0.0), real(0.0));
  CHECK_VEC4_CLOSE(expected, vec, real(0.000001));
}

//---------------------------------------------------------- Attempt Normalize 1
TEST(Vector4_AttemptNormalize_1)
{
  //Now attempt normalize! This should work.
  Vec4 vec(real(2.0), real(3.0), real(4.0), real(5.0));
  vec.AttemptNormalize();
  Vec4 expected(real(0.27216552697590867757747600830065),
                real(0.40824829046386301636621401245098),
                real(0.54433105395181735515495201660131),
                real(0.68041381743977169394369002075163));
  CHECK_VEC4_CLOSE(expected, vec, real(0.000001));
}

//---------------------------------------------------------- Attempt Normalize 2
TEST(Vector4_AttemptNormalize_2)
{
  //This is why attempt normalize was made
  Vec4 vec(real(0.0), real(0.0), real(0.0), real(0.0));
  vec.AttemptNormalize();
  Vec4 expected(real(0.0), real(0.0), real(0.0), real(0.0));
  CHECK_VEC4(expected, vec);
}

//---------------------------------------------------------- Attempt Normalize 3
TEST(Vector4_AttemptNormalize_3)
{
  //Now for a really small vector
  Vec4 vec(real(0.0001), real(0.0001), real(0.0), real(0.0));
  vec.AttemptNormalize();
  Vec4 expected(real(0.70710678118654752440084436210485),
                real(0.70710678118654752440084436210485),
                real(0.0),
                real(0.0));
  CHECK_VEC4_CLOSE(expected, vec, real(0.000001));
}

//--------------------------------------------------------------------- Negate 1
TEST(Vector4_Negate_1)
{
  Vec4 vec(real(67408.4), real(-3967.352), real(-9085.634), real(88.88));
  vec.Negate();
  Vec4 expected(real(-67408.4), real(3967.352), real(9085.634), real(-88.88));
  CHECK_VEC4(expected, vec);
}

//--------------------------------------------------------------------- Negate 2
TEST(Vector4_Negate_2)
{
  Vec4 vec(real(67408.4), real(-3967.352), real(-9085.634), real(88.88));
  Vec4 result = vec.Negate();
  Vec4 expected(real(-67408.4), real(3967.352), real(9085.634), real(-88.88));
  CHECK_VEC4(expected, result);
}

//---------------------------------------------------------------------- Valid 1
TEST(Vector4_Valid_1)
{
  //Test to see if a vector is valid.
  Vec4 vec(real(0.0), real(1.0), real(-0.0), real(5.0));  //Should be valid
  CHECK(vec.Valid());
}

//---------------------------------------------------------------------- Valid 2
TEST(Vector4_Valid_2)
{
  //Should not be valid!
  Vec4 vec(real(1.0), real(0.0), real(-1.0), real(2.0));
  real zero = real(0.0);
  vec[0] /= zero;
  CHECK(vec.Valid() == false);
}

//------------------------------------------------- Global Scalar Multiplication
TEST(Vector4_Global_ScalarMultiplication)
{
  //Swapped order
  Vec4 vec(real(2.0), real(3.0), real(6.0), real(7.0));
  Vec4 scaled(real(0.0), real(0.0), real(0.0), real(0.0));
  scaled = real(5.0) * vec;
  Vec4 expected(real(10.0), real(15.0), real(30.0), real(35.0));
  CHECK_VEC4(expected, scaled);
}

//-------------------------------------------------------------- Global Length 1
TEST(Vector4_Global_Length_1)
{
  //Standalone length
  Vec4 vec(real(2.0), real(5.0), real(3.0), real(6.0));
  real length = Length(vec);
  CHECK_EQUAL(real(8.6023252670426267717294735350497), length);
}

//-------------------------------------------------------------- Global Length 2
TEST(Vector4_Global_Length_2)
{
  //More simple length, no tricks here
  Vec4 vec(real(1.0), real(0.0), real(0.0), real(0.0));
  real length = vec.Length();
  CHECK_EQUAL(real(1.0), length);
}

//------------------------------------------------------ Global Length Squared 1
TEST(Vector4_Global_LengthSquared_1)
{
  //Squared length
  Vec4 vec(real(2.0), real(5.0), real(3.0), real(6.0));
  real length = LengthSq(vec);
  CHECK_EQUAL(real(74.0), length);
}

//------------------------------------------------------ Global Length Squared 2
TEST(Vector4_Global_LengthSquared_2)
{
  //Simple length!
  Vec4 vec(real(1.0), real(0.0), real(0.0), real(0.0));
  real length = LengthSq(vec);
  CHECK_EQUAL(real(1.0), length);
}

//----------------------------------------------------------- Global Dot Product
TEST(Vector4_Global_DotProduct)
{
  //Dot product!
  Vec4 vec(real(2.0), real(3.0), real(4.0), real(5.0));
  Vec4 cev(real(6.0), real(1.0), real(5.0), real(4.0));
  real dot = Dot(vec, cev);
  CHECK_EQUAL(real(55.0), dot);
}

//----------------------------------------------------------- Global Normalize 1
TEST(Vector4_Global_Normalize_1)
{
  //Normalized this using standalone function
  Vec4 vec(real(2.0), real(3.0), real(4.0), real(5.0));
  Normalize(vec);
  Vec4 expected(real(0.27216552697590867757747600830065),
                real(0.40824829046386301636621401245098),
                real(0.54433105395181735515495201660131),
                real(0.68041381743977169394369002075163));
  CHECK_VEC4_CLOSE(expected, vec, real(0.000001));
}

//----------------------------------------------------------- Global Normalize 2
TEST(Vector4_Global_Normalize_2)
{
  //Stupid norm this.
  Vec4 vec(real(1.0), real(0.0), real(0.0), real(0.0));
  Normalize(vec);
  Vec4 expected(real(1.0), real(0.0), real(0.0), real(0.0));
  CHECK_VEC4_CLOSE(expected, vec, real(0.000001));
}

//---------------------------------------------------------- Global Normalized 1
TEST(Vector4_Global_Normalized_1)
{
  //Normalized using the standalone function
  Vec4 vec(real(2.0), real(3.0), real(4.0), real(5.0));
  Vec4 norm = Normalized(vec);
  Vec4 expected(real(0.27216552697590867757747600830065),
                real(0.40824829046386301636621401245098),
                real(0.54433105395181735515495201660131),
                real(0.68041381743977169394369002075163));
  CHECK_VEC4_CLOSE(expected, norm, real(0.000001));
}

//---------------------------------------------------------- Global Normalized 2
TEST(Vector4_Global_Normalized_2)
{
  //Stupid norm
  Vec4 vec(real(1.0), real(0.0), real(0.0), real(0.0));
  Vec4 norm = Normalized(vec);
  Vec4 expected(real(1.0), real(0.0), real(0.0), real(0.0));
  CHECK_VEC4_CLOSE(expected, norm, real(0.000001));
}

//--------------------------------------------------- Global Attempt Normalize 1
TEST(Vector4_Global_AttemptNormalize_1)
{
  //Now attempt normalize! This should work.
  Vec4 vec(real(2.0), real(3.0), real(4.0), real(5.0));
  AttemptNormalize(vec);
  Vec4 expected(real(0.27216552697590867757747600830065),
                real(0.40824829046386301636621401245098),
                real(0.54433105395181735515495201660131),
                real(0.68041381743977169394369002075163));
  CHECK_VEC4_CLOSE(expected, vec, real(0.000001));
}

//--------------------------------------------------- Global Attempt Normalize 2
TEST(Vector4_Global_AttemptNormalize_2)
{
  //This is why attempt normalize was made
  Vec4 vec(real(0.0), real(0.0), real(0.0), real(0.0));
  AttemptNormalize(vec);
  Vec4 expected(real(0.0), real(0.0), real(0.0), real(0.0));
  CHECK_VEC4(expected, vec);
}

//--------------------------------------------------- Global Attempt Normalize 3
TEST(Vector4_Global_AttemptNormalize_3)
{
  //Now for a really small vector
  Vec4 vec(real(0.0001), real(0.0001), real(0.0), real(0.0));
  AttemptNormalize(vec);
  Vec4 expected(real(0.70710678118654752440084436210485),
                real(0.70710678118654752440084436210485),
                real(0.0),
                real(0.0));
  CHECK_VEC4_CLOSE(expected, vec, real(0.000001));
}

//---------------------------------------------------------------- Global Negate
TEST(Vector4_Global_Negate)
{
  Vec4 vec(real(-75.0), real(27.5), real(88.8), real(-2698.6));
  Negate(&vec);
  Vec4 expected(real(75.0), real(-27.5), real(-88.8), real(2698.6));
  CHECK_VEC4(expected, vec);
}

//--------------------------------------------------------------- Global Negated
TEST(Vector4_Global_Negated)
{
  Vec4 vec(real(98230.3), real(-90673.2), real(-25734.678), real(2823.33));
  Vec4 result = Negated(vec);
  Vec4 expected(real(-98230.3), real(90673.2), real(25734.678), real(-2823.33));
  CHECK_VEC4(expected, result);
}

//------------------------------------------------------------------- Global Abs
TEST(Vector4_Global_Abs)
{
  Vec4 vec(real(-1.5), real(8.3), real(-27.5), real(-54.32));
  Vec4 result = Abs(vec);
  Vec4 expected(real(1.5), real(8.3), real(27.5), real(54.32));
  CHECK_VEC4(expected, result);
}

//------------------------------------------------------------------- Global Min
TEST(Vector4_Global_Min)
{
  Vec4 vecA(real(79.3), real(-32.4), real(2526.3), real(999.9));
  Vec4 vecB(real(2384.66), real(0.0), real(728.2), real(998.9));
  Vec4 result = Min(vecA, vecB);
  Vec4 expected(real(79.3), real(-32.4), real(728.2), real(998.9));
  CHECK_VEC4(expected, result);
}

//------------------------------------------------------------------- Global Max
TEST(Vector4_Global_Max)
{
  Vec4 vecA(real(99.3), real(27.6), real(-7682.43), real(-33.2));
  Vec4 vecB(real(333.3), real(26.7), real(-3.2), real(8.0));
  Vec4 result = Max(vecA, vecB);
  Vec4 expected(real(333.3), real(27.6), real(-3.2), real(8.0));
  CHECK_VEC4(expected, result);
}

//---------------------------------------------------------------- Global Lerp 1
TEST(Vector4_Global_Lerp_1)
{
  Vec4 vecA(real(1.0), real(2.0), real(3.0), real(4.0));
  Vec4 vecB(real(8.0), real(1.0), real(12.0), real(7.0));
  Vec4 result = Lerp(vecA, vecB, real(0.5));
  Vec4 expected(real(4.5), real(1.5), real(7.5), real(5.5));
  CHECK_VEC4(expected, result);
}

//---------------------------------------------------------------- Global Lerp 2
TEST(Vector4_Global_Lerp_2)
{
  Vec4 vecA(real(1.0), real(2.0), real(3.0), real(4.0));
  Vec4 vecB(real(8.0), real(1.0), real(12.0), real(7.0));
  Vec4 result = Lerp(vecA, vecB, real(0.0));
  Vec4 expected(real(1.0), real(2.0), real(3.0), real(4.0));
  CHECK_VEC4(expected, result);
}

//---------------------------------------------------------------- Global Lerp 3
TEST(Vector4_Global_Lerp_3)
{
  Vec4 vecA(real(1.0), real(2.0), real(3.0), real(4.0));
  Vec4 vecB(real(8.0), real(1.0), real(12.0), real(7.0));
  Vec4 result = Lerp(vecA, vecB, real(1.0));
  Vec4 expected(real(8.0), real(1.0), real(12.0), real(7.0));
  CHECK_VEC4(expected, result);
}
