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
#include <cmath>

typedef Math::real    real;
typedef Math::Vector2 Vec2;
typedef Math::Vector3 Vec3;

//----------------------------------------------------------- Zero (Static Data)
TEST(Vector3_Zero_StaticData)
{
  Vec3 expected(real(0.0), real(0.0), real(0.0));
  CHECK_VEC3(expected, Vec3::cZero);
}

//--------------------------------------------------------- X-Axis (Static Data)
TEST(Vector3_XAxis_StaticData)
{
  Vec3 expected(real(1.0), real(0.0), real(0.0));
  CHECK_VEC3(expected, Vec3::cXAxis);
}

//--------------------------------------------------------- Y-Axis (Static Data)
TEST(Vector3_YAxis_StaticData)
{
  Vec3 expected(real(0.0), real(1.0), real(0.0));
  CHECK_VEC3(expected, Vec3::cYAxis);
}

//--------------------------------------------------------- Z-Axis (Static Data)
TEST(Vector3_ZAxis_StaticData)
{
  Vec3 expected(real(0.0), real(0.0), real(1.0));
  CHECK_VEC3(expected, Vec3::cZAxis);
}

//------------------------------------------------------------------ Constructor
TEST(Vector3_Constructor)
{
  //Normal constructor
  real testArray[3] = { real(1.0), real(2.0), real(3.0) };
	Vec3 v1(real(1.0), real(2.0), real(3.0));
  CHECK_VEC3(testArray, v1);
}

//------------------------------------------------------------- Copy Constructor
TEST(Vector3_CopyConstructor)
{
  //Copy constructor
  Vec3 v1(real(1.0), real(2.0), real(3.0));
  Vec3 v2(v1);
  CHECK_VEC3(v1, v2);
}

//------------------------------------------------- Explicit Pointer Constructor
TEST(Vector3_ExplicitPointerConstructor)
{
  //Explicit pointer constructor
  real testData[3] = { real(0.0), real(1.0), real(3.0) };
  Vec3 vec(testData);
  CHECK_VEC3(testData, vec);
}

//----------------------------------------------------------- Initial Assignment
TEST(Vector3_InitialAssignment)
{
  //Initial assignment
  Vec3 vec(real(0.0), real(1.0), real(3.0));
  Vec3 v3 = vec;
  CHECK_VEC3(vec, v3);
}

//------------------------------------------------------- Subscript (Read/Write)
TEST(Vector3_Subscript_ReadWrite)
{
  //Modify
  real testArray[3] = { real(5.0), real(3.0), real(8.0) };
  Vec3 vec;
  vec[0] = real(5.0);
  vec[1] = real(3.0);
  vec[2] = real(8.0);
  CHECK_VEC3(testArray, vec);
}

//-------------------------------------------------------- Subscript (Read Only)
TEST(Vector3_Subscript_ReadOnly)
{
  //Non-modify
  real values[3];
  Vec3 vec(real(1.0), real(2.0), real(3.0));

  values[0] = vec[0];
  values[1] = vec[1];
  values[2] = vec[2];
  CHECK_VEC3(vec, values);
}

//--------------------------------------------------------------- Unary Negation
TEST(Vector3_UnaryOperators)
{
  //Negation
  Vec3 vec(real(9.8), real(26.6), real(73.6));
  Vec3 neg = -vec;
  Vec3 expected(real(-9.8), real(-26.6), real(-73.6));
  CHECK_VEC3(expected, neg);
}

//--------------------------------------------- Scalar Multiplication Assignment
TEST(Vector3_ScalarMultiplicationAssignment)
{
  //Multiplication assignment
  Vec3 vec(real(2.0), real(3.0), real(6.0));
  vec *= real(5.0);
  Vec3 expected(real(10.0), real(15.0), real(30.0));
  CHECK_VEC3(expected, vec);
}

//--------------------------------------------------- Scalar Division Assignment
TEST(Vector3_ScalarDivisionAssignment)
{
  //Division assignment
  Vec3 vec(real(10.0), real(15.0), real(30.0));
  vec /= real(10.0);
  Vec3 expected(real(1.0), real(1.5), real(3.0));
  CHECK_VEC3(expected, vec);
}

//-------------------------------------------------------- Scalar Multiplication
TEST(Vector3_ScalarMultiplication)
{
  //Multiplication with scalar
  Vec3 vec(real(2.0), real(3.0), real(6.0));
  Vec3 scaled = vec * real(5.0);
  CHECK_EQUAL(real(10.0), scaled.x);
  CHECK_EQUAL(real(15.0), scaled.y);
  CHECK_EQUAL(real(30.0), scaled.z);
}

//-------------------------------------------------------------- Scalar Division
TEST(Vector3_ScalarDivision)
{
  //Division by scalar
  Vec3 vec;
  Vec3 scaled(real(10.0), real(15.0), real(30.0));
  vec = scaled / real(10.0);
  Vec3 expected(real(1.0), real(1.5), real(3.0));
  CHECK_VEC3(expected, vec);
}

//--------------------------------------------------- Vector Addition Assignment
TEST(Vector3_VectorAdditionAssignment)
{
  //Addition assignment
  Vec3 vec(real(5.0), real(2.0), real(3.0));
  Vec3 add(real(1.0), real(2.0), real(3.0));
  vec += add;
  Vec3 expected(real(6.0), real(4.0), real(6.0));
  CHECK_VEC3(expected, vec);
}

//------------------------------------------------ Vector Subtraction Assignment
TEST(Vector3_VectorSubtractionAssignment)
{
  //Subtraction assignment
  Vec3 vec(real(5.0), real(2.0), real(3.0));
  Vec3 add(real(1.0), real(2.0), real(3.0));
  add -= vec;
  Vec3 expected(real(-4.0), real(0.0), real(0.0));
  CHECK_VEC3(expected, add);
}

//-------------------------------------------------------------- Vector Addition
TEST(Vector3_VectorAddition)
{
  //Addition
  Vec3 vec(real(5.0), real(7.0), real(8.0));
  Vec3 add(real(5.0), real(4.0), real(8.0));
  Vec3 result = vec + add;
  Vec3 expected(real(10.0), real(11.0), real(16.0));
  CHECK_VEC3(expected, result);
}

//----------------------------------------------------------- Vector Subtraction
TEST(Vector3_VectorSubtraction)
{
  //Subtraction
  Vec3 vec(real(5.0), real(7.0), real(8.0));
  Vec3 add(real(5.0), real(4.0), real(8.0));
  Vec3 result = vec - add;
  Vec3 expected(real(0.0), real(3.0), real(0.0));
  CHECK_VEC3(expected, result);
}

//------------------------------------------------------------- Equal Comparison
TEST(Vector3_EqualComparison)
{
  //Equality
  Vec3 vec(real(1.0), real(2.0), real(3.0));
  Vec3 cev(real(1.0), real(2.0), real(3.0));
  CHECK(vec == cev);
}

//--------------------------------------------------------- Not Equal Comparison
TEST(Vector3_NotEqualComparison)
{
  //Inequality
  Vec3 vec(real(1.0), real(2.0), real(3.0));
  Vec3 cev(real(3.0), real(2.0), real(3.0));
  CHECK(vec != cev);
}

//------------------------------- Vector Componentwise Multiplication Assignment
TEST(Vector3_VectorComponentwiseMultiplicationAssignment)
{
  //Componentwise multiplication assignment
  Vec3 vec(real(3.0), real(-2.0), real(8.0));
  Vec3 cev(real(9.0), real(12.0), real(5.0));
  vec *= cev;
  Vec3 expected(real(27.0), real(-24.0), real(40.0));
  CHECK_VEC3(expected, vec);
}

//------------------------------------------ Vector Componentwise Multiplication
TEST(Vector3_VectorComponentwiseMultiplication)
{
  //Componentwise multiplication
  Vec3 vec(real(5.0), real(-9.0), real(-23.0));
  Vec3 cev(real(6.0), real(-2.0), real(7.0));
  Vec3 result = vec * cev;
  Vec3 expected(real(30.0), real(18.0), real(-161.0));
  CHECK_VEC3(expected, result);
}

//------------------------------------------------ Vector Componentwise Division
TEST(Vector3_VectorComponentwiseDivision)
{
  //Componentwise division
  Vec3 vec(real(3.0), real(12.0), real(22.0));
  Vec3 cev(real(10.0), real(4.0), real(11.0));
  Vec3 result = vec / cev;
  Vec3 expected(real(0.3), real(3.0), real(2.0));
  CHECK_VEC3(expected, result);
}

//-------------------------------------------------------------------------- Set
TEST(Vector3_Set)
{
  //I find this test funny because I've used Set in all the other tests
  Vec3 vec(real(0.0), real(1.0), real(6.0));
  vec.Set(real(6.0), real(9.0), real(20.0));
  Vec3 expected(real(6.0), real(9.0), real(20.0));
  CHECK_VEC3(expected, vec);
}

//--------------------------------------------------------------------- Zero Out
TEST(Vector3_ZeroOut)
{
  //Test to see if a vector can be zeroed
  Vec3 vec(real(6.0), real(2.0), real(3.0));
  vec.ZeroOut();
  Vec3 expected(real(0.0), real(0.0), real(0.0));
  CHECK_VEC3(expected, vec);
}

//---------------------------------------------------------------------- Reflect
TEST(Vector3_Reflect)
{
  //Just do a 2D plane reflection
  Vec3 vec(real(1.0), real(1.0), real(0.0));
  Vec3 nrm(real(0.0), real(1.0), real(0.0));
  Vec3 reflect = vec.ReflectAcrossVector(nrm);
  Vec3 expected(real(-1.0), real(1.0), real(0.0));
  CHECK_VEC3(expected, reflect);
}

//------------------------------------------------------------ MultiplyAdd
TEST(Vector3_MultiplyAdd)
{
  //Add a vector that is scaled
  Vec3 vec(real(1.0), real(2.0), real(3.0));
  Vec3 cev(real(3.0), real(1.0), real(2.0));
  vec = Math::MultiplyAdd(vec, cev, real(5.0));
  Vec3 expected(real(16.0), real(7.0), real(13.0));
  CHECK_VEC3(expected, vec);
}

//------------------------------------------------------------ MultiplySubtract
TEST(Vector3_MultiplySubtract)
{
  //Add a vector that is scaled
  Vec3 vec(real(1.0), real(2.0), real(3.0));
  Vec3 cev(real(3.0), real(1.0), real(2.0));
  vec = Math::MultiplySubtract(vec, cev, real(5.0));
  Vec3 expected(real(-14.0), real(-3.0), real(-7.0));
  CHECK_VEC3(expected, vec);
}

//------------------------------------------------------------------ Dot Product
TEST(Vector3_DotProduct)
{
  //Dot product!
  Vec3 vec(real(2.0), real(3.0), real(4.0));
  Vec3 cev(real(6.0), real(1.0), real(5.0));
  real dot = vec.Dot(cev);
  CHECK_EQUAL(real(35.0), dot);
}

//--------------------------------------------------------------------- Length 1
TEST(Vector3_Length_1)
{
  //First straight up length
  Vec3 vec(real(2.0), real(5.0), real(3.0));
  real length = vec.Length();
  CHECK_EQUAL(real(6.1644140029689764502501923814542), length);
}

//--------------------------------------------------------------------- Length 2
TEST(Vector3_Length_2)
{
  //More simple length, no tricks here
  Vec3 vec(real(1.0), real(0.0), real(0.0));
  real length = vec.Length();
  CHECK_EQUAL(real(1.0), length);
}

//------------------------------------------------------------- Length Squared 1
TEST(Vector3_LengthSquared_1)
{
  //Squared length
  Vec3 vec(real(2.0), real(5.0), real(3.0));
  real length = vec.LengthSq();
  CHECK_EQUAL(real(38.0), length);
}

//------------------------------------------------------------- Length Squared 2
TEST(Vector3_LengthSquared_2)
{
  //Simple length!
  Vec3 vec(real(1.0), real(0.0), real(0.0));
  real length = vec.LengthSq();
  CHECK_EQUAL(real(1.0), length);
}

//----------------------------------------------------------------- Normalized 1
TEST(Vector3_Normalized_1)
{
  //First norm, the values are SO close that this test should pass. Look at how
  //small that epsilon is!
  Vec3 vec(real(2.0), real(3.0), real(4.0));
  Vec3 norm = vec.Normalized();
  Vec3 expected(real(0.37139067635410372629315244769244),
                real(0.55708601453115558943972867153866),
                real(0.74278135270820745258630489538488));
  CHECK_VEC3_CLOSE(expected, norm, real(0.000001));
}

//----------------------------------------------------------------- Normalized 2
TEST(Vector3_Normalized_2)
{
  //Stupid norm
  Vec3 vec(real(1.0), real(0.0), real(0.0));
  Vec3 norm = vec.Normalized();
  Vec3 expected(real(1.0), real(0.0), real(0.0));
  CHECK_VEC3(expected, norm);
}

//------------------------------------------------------------------ Normalize 1
TEST(Vector3_Normalize_1)
{
  //Normalized this.
  Vec3 vec(real(2.0), real(3.0), real(4.0));
  vec.Normalize();
  Vec3 expected(real(0.37139067635410372629315244769244),
                real(0.55708601453115558943972867153866),
                real(0.74278135270820745258630489538488));
  CHECK_VEC3_CLOSE(expected, vec, real(0.000001));
}

//------------------------------------------------------------------ Normalize 2
TEST(Vector3_Normalize_2)
{
  //Stupid norm this.
  Vec3 vec(real(1.0), real(0.0), real(0.0));
  vec.Normalize();
  Vec3 expected(real(1.0), real(0.0), real(0.0));
  CHECK_VEC3(expected, vec);
}

//---------------------------------------------------------- Attempt Normalize 1
TEST(Vector3_AttemptNormalize_1)
{
  //Now attempt normalize! This should work.
  Vec3 vec(real(2.0), real(3.0), real(4.0));
  vec.AttemptNormalize();
  Vec3 expected(real(0.37139067635410372629315244769244),
                real(0.55708601453115558943972867153866),
                real(0.74278135270820745258630489538488));
  CHECK_VEC3_CLOSE(expected, vec, real(0.000001));
}

//---------------------------------------------------------- Attempt Normalize 2
TEST(Vector3_AttemptNormalize_2)
{
  //This is why attempt normalize was made
  Vec3 vec(real(0.0), real(0.0), real(0.0));
  vec.AttemptNormalize();
  Vec3 expected(real(0.0), real(0.0), real(0.0));
  CHECK_VEC3(expected, vec);
}

//---------------------------------------------------------- Attempt Normalize 3
TEST(Vector3_AttemptNormalize_3)
{
  //Now for a really small vector
  Vec3 vec(real(0.0000001), real(0.0000001), real(0.0));
  vec.AttemptNormalize();
  Vec3 expected(real(0.70710678118654752440084436210485),
                real(0.70710678118654752440084436210485),
                real(0.0));
  CHECK_VEC3_CLOSE(expected, vec, real(0.0000001));
}

//--------------------------------------------------------------------- Negate 1
TEST(Vector3_Negate_1)
{
  Vec3 vec(real(0.25), real(8.0), real(-32.32));
  vec.Negate();
  Vec3 expected(real(-0.25), real(-8.0), real(32.32));
  CHECK_VEC3(expected, vec);
}

//--------------------------------------------------------------------- Negate 2
TEST(Vector3_Negate_2)
{
  Vec3 vec(real(7.66), real(-42.69), real(99.99));
  Vec3 result = vec.Negate();
  Vec3 expected(real(-7.66), real(42.69), real(-99.99));
  CHECK_VEC3(expected, result);
}

//---------------------------------------------------------------------- Valid 1
TEST(Vector3_Valid_1)
{
  //Test to see if a vector is valid.
  Vec3 vec(real(0.0), real(1.0), real(-0.0));  //Should be valid
  CHECK(vec.Valid());
}

//---------------------------------------------------------------------- Valid 2
TEST(Vector3_Valid_2)
{
  //Should not be valid!
  Vec3 vec(real(1.0), real(0.0), real(-1.0));
  real zero = real(0.0);
  vec[0] /= zero;
  CHECK(vec.Valid() == false);
}

//-------------------------------------------------------------- Cross Product 1
TEST(Vector3_CrossProduct_1)
{
  //Cross product!
  Vec3 vec(real(1.0), real(0.0), real(0.0));
  Vec3 cev(real(0.0), real(1.0), real(0.0));
  Vec3 cross = vec.Cross(cev);
  Vec3 expected(real(0.0), real(0.0), real(1.0));
  CHECK_VEC3(expected, cross);
}
//-------------------------------------------------------------- Cross Product 2
TEST(Vector3_CrossProduct_2)
{
  //Zero
  Vec3 vec(real(0.0), real(0.0), real(0.0));
  Vec3 cev(real(0.0), real(1.0), real(0.0));
  Vec3 cross = vec.Cross(cev);
  Vec3 expected(real(0.0), real(0.0), real(0.0));
  CHECK_VEC3(expected, cross);
}

//-------------------------------------------------------------- Cross Product 3
TEST(Vector3_CrossProduct_3)
{
  //Get the y-axis.
  Vec3 vec(real(1.0), real(0.0), real(0.0));
  Vec3 cev(real(0.0), real(0.0), real(-1.0));
  Vec3 cross = vec.Cross(cev);
  Vec3 expected(real(0.0), real(1.0), real(0.0));
  CHECK_VEC3(expected, cross);
}

//------------------------------------------------- Global Scalar Multiplication
TEST(Vector3_Global_ScalarMultiplication)
{
  //Swapped order
  Vec3 vec(real(2.0), real(3.0), real(6.0));
  Vec3 scaled(real(0.0), real(0.0), real(0.0));
  scaled = real(5.0) * vec;
  Vec3 expected(real(10.0), real(15.0), real(30.0));
  CHECK_VEC3(expected, scaled);
}

//----------------------------------------------------------- Global Dot Product
TEST(Vector3_Global_DotProduct)
{
  //Standalone function
  Vec3 vec(real(2.0), real(3.0), real(4.0));
  Vec3 cev(real(6.0), real(1.0), real(5.0));
  real dot = Dot(vec, cev);
  CHECK_EQUAL(real(35.0), dot);
}

//-------------------------------------------------------------- Global Length 1
TEST(Vector3_Global_Length_1)
{
  //Standalone length
  Vec3 vec(real(2.0), real(5.0), real(3.0));
  real length = Length(vec);
  CHECK_EQUAL(real(6.1644140029689764502501923814542), length);
}

//-------------------------------------------------------------- Global Length 2
TEST(Vector3_Global_Length_2)
{
  //More simple length, no tricks here
  Vec3 vec(real(1.0), real(0.0), real(0.0));
  real length = Length(vec);
  CHECK_EQUAL(real(1.0), length);
}

//------------------------------------------------------ Global Length Squared 1
TEST(Vector3_Global_LengthSquared_1)
{
  //Squared length
  Vec3 vec(real(2.0), real(5.0), real(3.0));
  real length = LengthSq(vec);
  CHECK_EQUAL(real(38.0), length);
}

//------------------------------------------------------ Global Length Squared 2
TEST(Vector3_Global_LengthSquared_2)
{
  //Simple length!
  Vec3 vec(real(1.0), real(0.0), real(0.0));
  real length = LengthSq(vec);
  CHECK_EQUAL(real(1.0), length);
}

//---------------------------------------------------------- Global Normalized 1
TEST(Vector3_Global_Normalized_1)
{
  //Normalized using the standalone function
  Vec3 vec(real(2.0), real(3.0), real(4.0));
  Vec3 norm = Normalized(vec);
  Vec3 expected(real(0.37139067635410372629315244769244),
                real(0.55708601453115558943972867153866),
                real(0.74278135270820745258630489538488));
  CHECK_VEC3_CLOSE(expected, norm, real(0.000001));
}

//---------------------------------------------------------- Global Normalized 2
TEST(Vector3_Global_Normalized_2)
{
  //Stupid norm
  Vec3 vec(real(1.0), real(0.0), real(0.0));
  Vec3 norm = Normalized(vec);
  Vec3 expected(real(1.0), real(0.0), real(0.0));
  CHECK_VEC3(expected, norm);
}

//----------------------------------------------------------- Global Normalize 1
TEST(Vector3_Global_Normalize_1)
{
  //Normalized this using standalone function
  Vec3 vec(real(2.0), real(3.0), real(4.0));
  Normalize(vec);
  Vec3 expected(real(0.37139067635410372629315244769244),
                real(0.55708601453115558943972867153866),
                real(0.74278135270820745258630489538488));
  CHECK_VEC3_CLOSE(expected, vec, real(0.000001));
}

//----------------------------------------------------------- Global Normalize 2
TEST(Vector3_Global_Normalize_2)
{
  //Stupid norm this.
  Vec3 vec(real(1.0), real(0.0), real(0.0));
  Normalize(vec);
  Vec3 expected(real(1.0), real(0.0), real(0.0));
  CHECK_VEC3(expected, vec);
}

//--------------------------------------------------- Global Attempt Normalize 1
TEST(Vector3_Global_AttemptNormalize_1)
{
  //Now attempt normalize! This should work.
  Vec3 vec(real(2.0), real(3.0), real(4.0));
  AttemptNormalize(vec);
  Vec3 expected(real(0.37139067635410372629315244769244),
                real(0.55708601453115558943972867153866),
                real(0.74278135270820745258630489538488));
  CHECK_VEC3_CLOSE(expected, vec, real(0.000001));
}

//--------------------------------------------------- Global Attempt Normalize 2
TEST(Vector3_Global_AttemptNormalize_2)
{
  //This is why attempt normalize was made
  Vec3 vec(real(0.0), real(0.0), real(0.0));
  AttemptNormalize(vec);
  Vec3 expected(real(0.0), real(0.0), real(0.0));
  CHECK_VEC3(expected, vec);
}

//--------------------------------------------------- Global Attempt Normalize 3
TEST(Vector3_Global_AttemptNormalize_3)
{
  //Now for a really small vector
  Vec3 vec(real(0.0000001), real(0.0000001), real(0.0));
  AttemptNormalize(vec);
  Vec3 expected(real(0.70710678118654752440084436210485),
                real(0.70710678118654752440084436210485),
                real(0.0));
  CHECK_VEC3_CLOSE(expected, vec, real(0.0000001));
}

//------------------------------------------------------- Global Cross Product 1
TEST(Vector3_Global_CrossProduct_1)
{
  //Cross product using standalone function
  Vec3 vec(real(1.0), real(0.0), real(0.0));
  Vec3 cev(real(0.0), real(1.0), real(0.0));
  Vec3 cross = Cross(vec, cev);
  Vec3 expected(real(0.0), real(0.0), real(1.0));
  CHECK_VEC3(expected, cross);
}

//------------------------------------------------------- Global Cross Product 2
TEST(Vector3_Global_CrossProduct_2)
{
  //Zero
  Vec3 vec(real(0.0), real(0.0), real(0.0));
  Vec3 cev(real(0.0), real(1.0), real(0.0));
  Vec3 cross = Cross(vec, cev);
  Vec3 expected(real(0.0), real(0.0), real(0.0));
  CHECK_VEC3(expected, cross);
}

//------------------------------------------------------- Global Cross Product 3
TEST(Vector3_Global_CrossProduct_3)
{
  //Get the y-axis.
  Vec3 vec(real(1.0), real(0.0), real(0.0));
  Vec3 cev(real(0.0), real(0.0), real(-1.0));
  Vec3 cross = Cross(vec, cev);
  Vec3 expected(real(0.0), real(1.0), real(0.0));
  CHECK_VEC3(expected, cross);
}

//---------------------------------------------------------------- Global Negate
TEST(Vector3_Global_Negate)
{
  Vec3 vec(real(-75.0), real(27.5), real(88.8));
  Negate(&vec);
  Vec3 expected(real(75.0), real(-27.5), real(-88.8));
  CHECK_VEC3(expected, vec);
}

//--------------------------------------------------------------- Global Negated
TEST(Vector3_Global_Negated)
{
  Vec3 vec(real(98230.3), real(-90673.2), real(-25734.678));
  Vec3 result = Negated(vec);
  Vec3 expected(real(-98230.3), real(90673.2), real(25734.678));
  CHECK_VEC3(expected, result);
}

//------------------------------------------------------------------- Global Abs
TEST(Vector3_Global_Abs)
{
  Vec3 vec(real(-1.5), real(8.3), real(-27.5));
  Vec3 result = Abs(vec);
  Vec3 expected(real(1.5), real(8.3), real(27.5));
  CHECK_VEC3(expected, result);
}

//------------------------------------------------------------------- Global Min
TEST(Vector3_Global_Min)
{
  Vec3 vecA(real(79.3), real(-32.4), real(2526.3));
  Vec3 vecB(real(2384.66), real(0.0), real(728.2));
  Vec3 result = Min(vecA, vecB);
  Vec3 expected(real(79.3), real(-32.4), real(728.2));
  CHECK_VEC3(expected, result);
}

//------------------------------------------------------------------- Global Max
TEST(Vector3_Global_Max)
{
  Vec3 vecA(real(99.3), real(27.6), real(-7682.43));
  Vec3 vecB(real(333.3), real(26.7), real(-3.2));
  Vec3 result = Max(vecA, vecB);
  Vec3 expected(real(333.3), real(27.6), real(-3.2));
  CHECK_VEC3(expected, result);
}

//---------------------------------------------------------------- Global Lerp 1
TEST(Vector3_Global_Lerp_1)
{
  Vec3 vecA(real(1.0), real(1.0), real(0.0));
  Vec3 vecB(real(0.0), real(2.0), real(1.0));
  Vec3 result = Lerp(vecA, vecB, real(0.5));
  Vec3 expected(real(0.5), real(1.5), real(0.5));
  CHECK_VEC3(expected, result);
}

//---------------------------------------------------------------- Global Lerp 2
TEST(Vector3_Global_Lerp_2)
{
  Vec3 vecA(real(1.0), real(1.0), real(0.0));
  Vec3 vecB(real(0.0), real(2.0), real(1.0));
  Vec3 result = Lerp(vecA, vecB, real(0.0));
  Vec3 expected(real(1.0), real(1.0), real(0.0));
  CHECK_VEC3(expected, result);
}

//---------------------------------------------------------------- Global Lerp 3
TEST(Vector3_Global_Lerp_3)
{
  Vec3 vecA(real(1.0), real(1.0), real(0.0));
  Vec3 vecB(real(0.0), real(2.0), real(1.0));
  Vec3 result = Lerp(vecA, vecB, real(1.0));
  Vec3 expected(real(0.0), real(2.0), real(1.0));
  CHECK_VEC3(expected, result);
}

//---------------------------------------------------------------- Slerp 1
TEST(Vector3_Slerp_1)
{
  Vec3 start(1, 0, 0);
  Vec3 end(0, 1, 0);
  real t = 0.25f;

  Vec3 rSlerp = Vec3::Slerp(start, end, t);
  Vec3 rSlerpUnnormalized = Vec3::SlerpUnnormalized(start, end, t);

  Vec3 nStart = start.AttemptNormalized();
  Vec3 nEnd = end.AttemptNormalized();

  Vec3 rSlerpFast = Vec3::SlerpFast(nStart, nEnd, t);

  Vec3 eNormalized = Vec3(0.92388f, 0.382683f, 0);
  Vec3 eUnNormalized = eNormalized;
  CHECK_VEC3_CLOSE(eNormalized, rSlerpFast, 0.001f);
  CHECK_VEC3_CLOSE(eNormalized, rSlerp, 0.001f);
  CHECK_VEC3_CLOSE(eUnNormalized, rSlerpUnnormalized, 0.001f);
}

//---------------------------------------------------------------- Slerp 2
TEST(Vector3_Slerp_2)
{
  Vec3 start(0, 0, 1);
  Vec3 end(0, 1, 0);
  real t = 0.25f;

  Vec3 rSlerp = Vec3::Slerp(start, end, t);
  Vec3 rSlerpUnnormalized = Vec3::SlerpUnnormalized(start, end, t);

  Vec3 nStart = start.AttemptNormalized();
  Vec3 nEnd = end.AttemptNormalized();

  Vec3 rSlerpFast = Vec3::SlerpFast(nStart, nEnd, t);

  Vec3 eNormalized = Vec3(0, 0.382683f, 0.92388f);
  Vec3 eUnNormalized = eNormalized;
  CHECK_VEC3_CLOSE(eNormalized, rSlerpFast, 0.001f);
  CHECK_VEC3_CLOSE(eNormalized, rSlerp, 0.001f);
  CHECK_VEC3_CLOSE(eUnNormalized, rSlerpUnnormalized, 0.001f);
}

//---------------------------------------------------------------- Slerp 3
TEST(Vector3_Slerp_3)
{
  Vec3 start(1, 0.5f, 1);
  Vec3 end(0, 0.5f, -1);
  real t = 0.75f;

  Vec3 rSlerp = Vec3::Slerp(start, end, t);
  Vec3 rSlerpUnnormalized = Vec3::SlerpUnnormalized(start, end, t);

  Vec3 nStart = start.AttemptNormalized();
  Vec3 nEnd = end.AttemptNormalized();

  Vec3 rSlerpFast = Vec3::SlerpFast(nStart, nEnd, t);

  Vec3 eNormalized = Vec3(0.362962f, 0.680976f, -0.636027f);
  Vec3 eUnNormalized = Vec3(0.544443f, 0.830673f, -0.572461f);
  CHECK_VEC3_CLOSE(eNormalized, rSlerpFast, 0.001f);
  CHECK_VEC3_CLOSE(eNormalized, rSlerp, 0.001f);
  CHECK_VEC3_CLOSE(eUnNormalized, rSlerpUnnormalized, 0.001f);
}

//---------------------------------------------------------------- Slerp 4
TEST(Vector3_Slerp_4)
{
  Vec3 start(-0.5, 0, 1);
  Vec3 end(-0.5, 0, -2);
  real t = 0.75f;

  Vec3 rSlerp = Vec3::Slerp(start, end, t);
  Vec3 rSlerpUnnormalized = Vec3::SlerpUnnormalized(start, end, t);

  Vec3 nStart = start.AttemptNormalized();
  Vec3 nEnd = end.AttemptNormalized();

  Vec3 rSlerpFast = Vec3::SlerpFast(nStart, nEnd, t);

  Vec3 eNormalized = Vec3(-0.753402f, 0, -0.657561f);
  Vec3 eUnNormalized = Vec3(-1.18268f, 0, -2.09659f);
  CHECK_VEC3_CLOSE(eNormalized, rSlerpFast, 0.001f);
  CHECK_VEC3_CLOSE(eNormalized, rSlerp, 0.001f);
  CHECK_VEC3_CLOSE(eUnNormalized, rSlerpUnnormalized, 0.001f);
}

//---------------------------------------------------------------- Slerp 5
TEST(Vector3_Slerp_5)
{
  // Degenerate case for Fast (that'll crash)
  Vec3 start(1, 0, 0);
  Vec3 end(-1, 0, 0);
  real t = 0.5f;

  Vec3 rSlerp = Vec3::Slerp(start, end, t);
  Vec3 rSlerpUnnormalized = Vec3::SlerpUnnormalized(start, end, t);

  CHECK(rSlerp != Vec3::cZero);
  CHECK(rSlerpUnnormalized != Vec3::cZero);
}

//---------------------------------------------------------------- Slerp 6
TEST(Vector3_Slerp_6)
{
  // Degenerate case for Fast (that'll crash)
  Vec3 start(0, 1, 0);
  Vec3 end(0, -1, 0);
  real t = 0.5f;

  Vec3 rSlerp = Vec3::Slerp(start, end, t);
  Vec3 rSlerpUnnormalized = Vec3::SlerpUnnormalized(start, end, t);

  CHECK(rSlerp != Vec3::cZero);
  CHECK(rSlerpUnnormalized != Vec3::cZero);
}

//---------------------------------------------------------------- Slerp 7
TEST(Vector3_Slerp_7)
{
  // Degenerate case for Fast (that'll crash)
  Vec3 start(0, 0, 1);
  Vec3 end(0, 0, -1);
  real t = 0.5f;

  Vec3 rSlerp = Vec3::Slerp(start, end, t);
  Vec3 rSlerpUnnormalized = Vec3::SlerpUnnormalized(start, end, t);

  CHECK(rSlerp != Vec3::cZero);
  CHECK(rSlerpUnnormalized != Vec3::cZero);
}

//---------------------------------------------------------------- Slerp 8
TEST(Vector3_Slerp_8)
{
  // Degenerate case (start is zero)
  Vec3 start(0, 0, 0);
  Vec3 end(0, 0, -1);
  real t = 0.5f;

  // Simply make sure these don't crash
  Vec3 rSlerp = Vec3::Slerp(start, end, t);
  Vec3 rSlerpUnnormalized = Vec3::SlerpUnnormalized(start, end, t);
}

//---------------------------------------------------------------- Slerp 9
TEST(Vector3_Slerp_9)
{
  // Degenerate case (start is zero)
  Vec3 start(0, 0, 1);
  Vec3 end(0, 0, 0);
  real t = 0.5f;

  // Simply make sure these don't crash
  Vec3 rSlerp = Vec3::Slerp(start, end, t);
  Vec3 rSlerpUnnormalized = Vec3::SlerpUnnormalized(start, end, t);
}

//---------------------------------------------------------------- Slerp 10
TEST(Vector3_Slerp_10)
{
  // Checking near parallel case
  Vec3 start(1, 0, 0);
  Vec3 end(1, 0.001f, 0);
  real t = 0.25f;

  Vec3 rSlerp = Vec3::Slerp(start, end, t);
  Vec3 rSlerpUnnormalized = Vec3::SlerpUnnormalized(start, end, t);

  Vec3 nStart = start.AttemptNormalized();
  Vec3 nEnd = end.AttemptNormalized();

  Vec3 rSlerpFast = Vec3::SlerpFast(nStart, nEnd, t);

  Vec3 eNormalized = start;
  Vec3 eUnNormalized = eNormalized;
  CHECK_VEC3_CLOSE(eNormalized, rSlerpFast, 0.001f);
  CHECK_VEC3_CLOSE(eNormalized, rSlerp, 0.001f);
  CHECK_VEC3_CLOSE(eUnNormalized, rSlerpUnnormalized, 0.001f);
}

//---------------------------------------------------------------- Slerp 11
TEST(Vector3_Slerp_11)
{
  // Checking near parallel case
  Vec3 start(0, 1, 0);
  Vec3 end(0, 1, 0.001f);
  real t = 0.25f;

  Vec3 rSlerp = Vec3::Slerp(start, end, t);
  Vec3 rSlerpUnnormalized = Vec3::SlerpUnnormalized(start, end, t);

  Vec3 nStart = start.AttemptNormalized();
  Vec3 nEnd = end.AttemptNormalized();

  Vec3 rSlerpFast = Vec3::SlerpFast(nStart, nEnd, t);

  Vec3 eNormalized = start;
  Vec3 eUnNormalized = eNormalized;
  CHECK_VEC3_CLOSE(eNormalized, rSlerpFast, 0.001f);
  CHECK_VEC3_CLOSE(eNormalized, rSlerp, 0.001f);
  CHECK_VEC3_CLOSE(eUnNormalized, rSlerpUnnormalized, 0.001f);
}

//---------------------------------------------------------------- Slerp 12
TEST(Vector3_Slerp_12)
{
  // Checking near parallel case
  Vec3 start(0, 0, 1);
  Vec3 end(0.001f, 0, 1);
  real t = 0.25f;

  Vec3 rSlerp = Vec3::Slerp(start, end, t);
  Vec3 rSlerpUnnormalized = Vec3::SlerpUnnormalized(start, end, t);

  Vec3 nStart = start.AttemptNormalized();
  Vec3 nEnd = end.AttemptNormalized();

  Vec3 rSlerpFast = Vec3::SlerpFast(nStart, nEnd, t);

  Vec3 eNormalized = start;
  Vec3 eUnNormalized = eNormalized;
  CHECK_VEC3_CLOSE(eNormalized, rSlerpFast, 0.001f);
  CHECK_VEC3_CLOSE(eNormalized, rSlerp, 0.001f);
  CHECK_VEC3_CLOSE(eUnNormalized, rSlerpUnnormalized, 0.001f);
}

//---------------------------------------------------------------- Slerp 13
TEST(Vector3_Slerp_13)
{
  // Checking near parallel case
  Vec3 start(0, 0, 1);
  Vec3 end = start;
  real t = 0.25f;

  Vec3 rSlerp = Vec3::Slerp(start, end, t);
  Vec3 rSlerpUnnormalized = Vec3::SlerpUnnormalized(start, end, t);

  Vec3 nStart = start.AttemptNormalized();
  Vec3 nEnd = end.AttemptNormalized();

  Vec3 rSlerpFast = Vec3::SlerpFast(nStart, nEnd, t);

  Vec3 eNormalized = start;
  Vec3 eUnNormalized = eNormalized;
  CHECK_VEC3_CLOSE(eNormalized, rSlerpFast, 0.001f);
  CHECK_VEC3_CLOSE(eNormalized, rSlerp, 0.001f);
  CHECK_VEC3_CLOSE(eUnNormalized, rSlerpUnnormalized, 0.001f);
}