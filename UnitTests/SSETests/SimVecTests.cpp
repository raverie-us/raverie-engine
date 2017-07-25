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
#include "Math/MathStandard.hpp"
#include "TestSSE.hpp"

//typedef Math::real    real;
typedef Math::Vector2 Vec2;
typedef Math::Vector3 Vec3;
typedef Math::Vector4 Vec4;
//typedef Math::Simd::VecSim VecSim;

using namespace Math;
using namespace Simd;

//----------------------------------------------------------- BasisX (Static Data)
TEST(SimVec_BasisX_StaticData)
{
  SimVec expected = Set4(scalar(1.0),scalar(0.0),scalar(0.0),scalar(0.0));
  Check_SimVec4(expected, gSimBasisX);
}

//----------------------------------------------------------- BasisY (Static Data)
TEST(SimVec_BasisY_StaticData)
{
  SimVec expected = Set4(scalar(0.0),scalar(1.0),scalar(0.0),scalar(0.0));
  Check_SimVec4(expected, gSimBasisY);
}

//----------------------------------------------------------- BasisZ (Static Data)
TEST(SimVec_BasisZ_StaticData)
{
  SimVec expected = Set4(scalar(0.0),scalar(0.0),scalar(1.0),scalar(0.0));
  Check_SimVec4(expected, gSimBasisZ);
}

//----------------------------------------------------------- BasisW (Static Data)
TEST(SimVec_BasisW_StaticData)
{
  SimVec expected = Set4(scalar(0.0),scalar(0.0),scalar(0.0),scalar(1.0));
  Check_SimVec4(expected, gSimBasisW);
}

//----------------------------------------------------------- MaskX (Static Data)
TEST(SimVec_MaskX_StaticData)
{
  SimVec v1 = Set4(scalar(1.0),scalar(1.0),scalar(1.0),scalar(1.0));
  SimVec result = AndVec(v1,gSimMaskX);
  SimVec expected = Set4(scalar(1.0),scalar(0.0),scalar(0.0),scalar(0.0));
  Check_SimVec4(expected, result);
}

//----------------------------------------------------------- MaskY (Static Data)
TEST(SimVec_MaskY_StaticData)
{
  SimVec v1 = Set4(scalar(1.0),scalar(1.0),scalar(1.0),scalar(1.0));
  SimVec result = AndVec(v1,gSimMaskY);
  SimVec expected = Set4(scalar(0.0),scalar(1.0),scalar(0.0),scalar(0.0));
  Check_SimVec4(expected, result);
}

//----------------------------------------------------------- MaskZ (Static Data)
TEST(SimVec_MaskZ_StaticData)
{
  SimVec v1 = Set4(scalar(1.0),scalar(1.0),scalar(1.0),scalar(1.0));
  SimVec result = AndVec(v1,gSimMaskZ);
  SimVec expected = Set4(scalar(0.0),scalar(0.0),scalar(1.0),scalar(0.0));
  Check_SimVec4(expected, result);
}

//----------------------------------------------------------- MaskW (Static Data)
TEST(SimVec_MaskW_StaticData)
{
  SimVec v1 = Set4(scalar(1.0),scalar(1.0),scalar(1.0),scalar(1.0));
  SimVec result = AndVec(v1,gSimMaskW);
  SimVec expected = Set4(scalar(0.0),scalar(0.0),scalar(0.0),scalar(1.0));
  Check_SimVec4(expected, result);
}

//----------------------------------------------------------- Vec3Mask (Static Data)
TEST(SimVec_Vec3Mask_StaticData)
{
  SimVec v1 = Set4(scalar(1.0),scalar(1.0),scalar(1.0),scalar(1.0));
  SimVec result = AndVec(v1,gSimVec3Mask);
  SimVec expected = Set4(scalar(1.0),scalar(1.0),scalar(1.0),scalar(0.0));
  Check_SimVec4(expected, result);
}

//----------------------------------------------------------- Zero (Static Data)
TEST(SimVec_Zero_StaticData)
{
  SimVec expected = Set4(scalar(0.0),scalar(0.0),scalar(0.0),scalar(0.0));
  Check_SimVec4(expected, gSimZero);
}

//--------------------------------------------------------- X-Axis (Static Data)
TEST(SimVec_XAxis_StaticData)
{
  SimVec expected = Set4(scalar(1.0),scalar(0.0),scalar(0.0),scalar(0.0));
  Check_SimVec4(expected, gSimBasisX);
}

//--------------------------------------------------------- Y-Axis (Static Data)
TEST(SimVec_YAxis_StaticData)
{
  SimVec expected = Set4(scalar(0.0),scalar(1.0),scalar(0.0),scalar(0.0));
  Check_SimVec4(expected, gSimBasisY);
}

//--------------------------------------------------------- Z-Axis (Static Data)
TEST(SimVec_ZAxis_StaticData)
{
  SimVec expected = Set4(scalar(0.0),scalar(0.0),scalar(1.0),scalar(0.0));
  Check_SimVec4(expected, gSimBasisZ);
}

//--------------------------------------------------------- W-Axis (Static Data)
TEST(SimVec_WAxis_StaticData)
{
  SimVec expected = Set4(scalar(0.0),scalar(0.0),scalar(0.0),scalar(1.0));
  Check_SimVec4(expected, gSimBasisW);
}

//----------------------------------------------------------- SplatX
TEST(SimVec_SplatX)
{
  SimVec v1 = Set4(scalar(1.0),scalar(2.0),scalar(3.0),scalar(4.0));
  SimVec result = SplatX(v1);
  SimVec expected = Set4(scalar(1.0),scalar(1.0),scalar(1.0),scalar(1.0));
  Check_SimVec4(expected, result);
}

//----------------------------------------------------------- SplatY
TEST(SimVec_SplatY)
{
  SimVec v1 = Set4(scalar(1.0),scalar(2.0),scalar(3.0),scalar(4.0));
  SimVec result = SplatY(v1);
  SimVec expected = Set4(scalar(2.0),scalar(2.0),scalar(2.0),scalar(2.0));
  Check_SimVec4(expected, result);
}

//----------------------------------------------------------- SplatZ
TEST(SimVec_SplatZ)
{
  SimVec v1 = Set4(scalar(1.0),scalar(2.0),scalar(3.0),scalar(4.0));
  SimVec result = SplatZ(v1);
  SimVec expected = Set4(scalar(3.0),scalar(3.0),scalar(3.0),scalar(3.0));
  Check_SimVec4(expected, result);
}

//----------------------------------------------------------- SplatW
TEST(SimVec_SplatW)
{
  SimVec v1 = Set4(scalar(1.0),scalar(2.0),scalar(3.0),scalar(4.0));
  SimVec result = SplatW(v1);
  SimVec expected = Set4(scalar(4.0),scalar(4.0),scalar(4.0),scalar(4.0));
  Check_SimVec4(expected, result);
}

//------------------------------------------------------------------ 4 float Load
TEST(SimVec_Load)
{
  SimVec v1 = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec v2 = { scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0) };
  Check_SimVec4(v1, v2);
}

//------------------------------------------------------------------ float pointer Load
TEST(SimVec_Load_2)
{
  __declspec(align(16))float floatArray[4] = { scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0) };
  SimVec v1 = Load(floatArray);
  SimVec v2 = { scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0) };
  Check_SimVec4(v1, v2);
}

//------------------------------------------------------------------ unaligned float load
TEST(SimVec_UnAlignedLoad)
{
  float floatArray[4] = { scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0) };
  SimVec v1 = UnAlignedLoad(floatArray);
  SimVec v2 = { scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0) };
  Check_SimVec4(v1, v2);
}

//--------------------------------------------------------------- Unary Negation
TEST(SimVec_UnaryNegation)
{
  //Negation
  SimVec vec = Set4(scalar(9.8),scalar(26.6),scalar(-73.6),scalar(-24.0));
  SimVec neg = Negate(vec);
  SimVec expected = Set4(scalar(-9.8), scalar(-26.6), scalar(73.6), scalar(24.0));
  Check_SimVec4(expected, neg);
}

//--------------------------------------------- Scalar Multiplication Assignment
TEST(SimVec_ScalarMultiplicationAssignment)
{  
  //Multiplication assignment
  SimVec vec = Set4(scalar(2.0), scalar(3.0), scalar(6.0), scalar(18.0));
  vec *= scalar(5.0);
  SimVec expected = Set4(scalar(10.0), scalar(15.0), scalar(30.0), scalar(90.0));
  Check_SimVec4(expected, vec);
}

//--------------------------------------------------- Scalar Division Assignment
TEST(SimVec_ScalarDivisionAssignment)
{
  //Division assignment
  SimVec vec = Set4(scalar(10.0), scalar(15.0), scalar(30.0), scalar(90.0));
  vec /= scalar(10.0);
  SimVec expected = Set4(scalar(1.0), scalar(1.5), scalar(3.0), scalar(9.0));
  Check_SimVec4(expected, vec);
}

//-------------------------------------------------------- Scalar Multiplication
TEST(SimVec_ScalarMultiplication)
{
  //Multiplication with scalar
  SimVec vec = Set4(scalar(2.0), scalar(3.0), scalar(6.0), scalar(7.0));
  SimVec scaled = vec * scalar(5.0);
  SimVec expected = Set4(scalar(10.0), scalar(15.0), scalar(30.0), scalar(35.0));
  Check_SimVec4(expected, scaled);
}

//-------------------------------------------------------------- Scalar Division
TEST(SimVec_ScalarDivision)
{
  //Division by scalar
  SimVec vec = Set4(scalar(0.0), scalar(0.0), scalar(0.0), scalar(0.0));
  SimVec scaled = Set4(scalar(10.0), scalar(15.0), scalar(30.0), scalar(35.0));
  vec = scaled / scalar(10.0);
  SimVec expected = Set4(scalar(1.0), scalar(1.5), scalar(3.0), scalar(3.5));
  Check_SimVec4(expected, vec);
}

//--------------------------------------------------- Vector Addition Assignment
TEST(SimVec_VectorAdditionAssignment)
{
  SimVec vec = Set4(scalar(5.0), scalar(2.0), scalar(3.0), scalar(-1.0));
  SimVec add = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  vec += add;
  SimVec expected = Set4(scalar(6.0), scalar(4.0), scalar(6.0), scalar(3.0));
  Check_SimVec4(expected, vec);
}

//------------------------------------------------ Vector Subtraction Assignment
TEST(SimVec_VectorSubtractionAssignment)
{
  SimVec vec = Set4(scalar(5.0), scalar(2.0), scalar(3.0), scalar(-1.0));
  SimVec add = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  add -= vec;
  SimVec expected = Set4(scalar(-4.0), scalar(0.0), scalar(0.0), scalar(5.0));
  Check_SimVec4(expected, add);
}

//------------------------------------------------ Vector Component Multiplication Assignment
TEST(SimVec_VectorMultiplicationAssignment)
{
  SimVec vec = Set4(scalar(5.0), scalar(2.0), scalar(3.0), scalar(-1.0));
  SimVec add = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  add *= vec;
  SimVec expected = Set4(scalar(5.0), scalar(4.0), scalar(9.0), scalar(-4.0));
  Check_SimVec4(expected, add);
}

//------------------------------------------------ Vector Component Multiplication
TEST(SimVec_VectorMultiplication)
{
  SimVec vec = Set4(scalar(5.0), scalar(2.0), scalar(3.0), scalar(-1.0));
  SimVec add = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec result = add * vec;
  SimVec expected = Set4(scalar(5.0), scalar(4.0), scalar(9.0), scalar(-4.0));
  Check_SimVec4(expected, result);
}

//------------------------------------------------ Vector Component Division Assignment
TEST(SimVec_VectorDivisionAssignment)
{
  SimVec vec = Set4(scalar(5.0), scalar(2.0), scalar(3.0), scalar(-4.0));
  SimVec add = Set4(scalar(10.0), scalar(2.0), scalar(1.0), scalar(4.0));
  add /= vec;
  SimVec expected = Set4(scalar(2.0), scalar(1.0), scalar(.333333), scalar(-1.0));
  Check_SimVec4_Close(expected, add, scalar(.0001));
}

//------------------------------------------------ Vector Component Division
TEST(SimVec_VectorDivision)
{
  SimVec vec = Set4(scalar(5.0), scalar(2.0), scalar(3.0), scalar(-4.0));
  SimVec add = Set4(scalar(10.0), scalar(2.0), scalar(1.0), scalar(4.0));
  SimVec result = add / vec;
  SimVec expected = Set4(scalar(2.0), scalar(1.0), scalar(.333333), scalar(-1.0));
  Check_SimVec4_Close(expected, result, scalar(.0001));
}

//-------------------------------------------------------------- Vector Addition
TEST(SimVec_VectorAddition)
{
  //Addition
  SimVec vec = Set4(scalar(5.0), scalar(7.0), scalar(8.0), scalar(10.0));
  SimVec add = Set4(scalar(5.0), scalar(4.0), scalar(8.0), scalar(3.0));
  SimVec result = vec + add;
  SimVec expected = Set4(scalar(10.0), scalar(11.0), scalar(16.0), scalar(13.0));
  Check_SimVec4(expected, result);
}

//----------------------------------------------------------- Vector Subtraction
TEST(SimVec_VectorSubtraction)
{
  //Subtraction
  SimVec vec = Set4(scalar(5.0), scalar(7.0), scalar(8.0), scalar(10.0));
  SimVec add = Set4(scalar(5.0), scalar(4.0), scalar(8.0), scalar(3.0));
  SimVec result = vec - add;
  SimVec expected = Set4(scalar(0.0), scalar(3.0), scalar(0.0), scalar(7.0));
  Check_SimVec4(expected, result);
}

//-------------------------------------------------------------- Vector Addition Function
TEST(SimVec_VectorAdditionFunction)
{
  //Addition
  SimVec vec = Set4(scalar(5.0), scalar(7.0), scalar(8.0), scalar(10.0));
  SimVec add = Set4(scalar(5.0), scalar(4.0), scalar(8.0), scalar(3.0));
  SimVec result = Add(vec,add);
  SimVec expected = Set4(scalar(10.0), scalar(11.0), scalar(16.0), scalar(13.0));
  Check_SimVec4(expected, result);
}

//----------------------------------------------------------- Vector Subtraction Function
TEST(SimVec_VectorSubtractionFunction)
{
  //Subtraction
  SimVec vec = Set4(scalar(5.0), scalar(7.0), scalar(8.0), scalar(10.0));
  SimVec add = Set4(scalar(5.0), scalar(4.0), scalar(8.0), scalar(3.0));
  SimVec result = Subtract(vec,add);
  SimVec expected = Set4(scalar(0.0), scalar(3.0), scalar(0.0), scalar(7.0));
  Check_SimVec4(expected, result);
}

//------------------------------------------------ Vector Component Multiplication
TEST(SimVec_VectorComponentMultiplication)
{
  //Subtraction assignment
  SimVec vec = Set4(scalar(5.0), scalar(2.0), scalar(3.0), scalar(-1.0));
  SimVec add = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec result = Multiply(add,vec);
  SimVec expected = Set4(scalar(5.0), scalar(4.0), scalar(9.0), scalar(-4.0));
  Check_SimVec4(expected, result);
}

//------------------------------------------------ Vector Component Division
TEST(SimVec_VectorComponentDivision)
{
  SimVec vec = Set4(scalar(5.0), scalar(2.0), scalar(3.0), scalar(-4.0));
  SimVec add = Set4(scalar(10.0), scalar(2.0), scalar(1.0), scalar(4.0));
  SimVec result = Divide(add,vec);
  SimVec expected = Set4(scalar(2.0), scalar(1.0), scalar(.333333), scalar(-1.0));
  Check_SimVec4_Close(expected, result, scalar(.0001));
}

//------------------------------------------------------------- Equal Comparison
TEST(SimVec_EqualComparison)
{
  //Equality
  SimVec vec = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec cev = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec actual = Equal(vec,cev);
  CheckBool_SimVec4(actual);
}

//--------------------------------------------------------- Not Equal Comparison
TEST(SimVec_NotEqualComparison)
{
  //Inequality
  SimVec vec = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec cev = Set4(scalar(5.0), scalar(6.0), scalar(7.0), scalar(8.0));
  SimVec actual = NotEqual(vec,cev);
  CheckBool_SimVec4(actual);
}

//------------------------------------------ Vector Componentwise Multiplication
TEST(SimVec_VectorComponentwiseMultiplication)
{
  SimVec vec = Set4(scalar(3.0), scalar(-2.0), scalar(8.0), scalar(7.0));
  SimVec cev = Set4(scalar(9.0), scalar(12.0), scalar(5.0), scalar(22.0));
  SimVec result = vec * cev;
  SimVec expected = Set4(scalar(27.0), scalar(-24.0), scalar(40.0), scalar(154.0));
  Check_SimVec4(expected, result);
}

//------------------------------------------------ Vector Componentwise Division
TEST(SimVec_VectorComponentwiseDivision)
{
  ////Componentwise division
  SimVec vec = Set4(scalar(3.0), scalar(12.0), scalar(22.0), scalar(8.0));
  SimVec cev = Set4(scalar(10.0), scalar(4.0), scalar(11.0), scalar(8.0));
  SimVec result = vec / cev;
  SimVec expected = Set4(scalar(0.3), scalar(3.0), scalar(2.0), scalar(1.0));
  Check_SimVec4(expected, result);
}

//-------------------------------------------------------------------------- Set 1
TEST(SimVec_Set_1)
{
  SimVec vec = Set4(scalar(0.0), scalar(1.0), scalar(6.0), scalar(98.0));
  vec = Set(scalar(1));
  SimVec expected = Set4(scalar(1), scalar(1), scalar(1), scalar(1));
  Check_SimVec4(expected, vec);
}

//-------------------------------------------------------------------------- Set 2
TEST(SimVec_Set_2)
{
  SimVec vec = Set4(scalar(0.0), scalar(1.0), scalar(6.0), scalar(98.0));
  vec = Set4(scalar(6.0), scalar(9.0), scalar(20.0), scalar(26.0));
  SimVec expected = Set4(scalar(6.0), scalar(9.0), scalar(20.0), scalar(26.0));
  Check_SimVec4(expected, vec);
}

//-------------------------------------------------------------------------- GetXYZ
TEST(SimVec_GetXYZ)
{
  SimVec vec = Set4(scalar(0.0), scalar(1.0), scalar(6.0), scalar(98.0));
  vec = GetXYZ(vec);
  SimVec expected = Set4(scalar(0.0),scalar(1.0),scalar(6.0),scalar(0.0));
  Check_SimVec4(expected, vec);
}

//--------------------------------------------------------------------- Zero Out
TEST(SimVec_ZeroOut)
{
  //Test to see if a vector can be zeroed
  SimVec vec = Set4(scalar(6.0), scalar(2.0), scalar(3.0), scalar(1.0));
  vec = ZeroOutVec();
  SimVec expected = Set4(scalar(0.0), scalar(0.0), scalar(0.0), scalar(0.0));
  Check_SimVec4(expected, vec);
}

//------------------------------------------------------------------ Dot Product
TEST(SimVec_DotProduct)
{
  ////Dot product!
  SimVec vec = Set4(scalar(2.0), scalar(3.0), scalar(4.0), scalar(5.0));
  SimVec cev = Set4(scalar(6.0), scalar(1.0), scalar(5.0), scalar(4.0));
  SimVec dot = Dot4(vec,cev);
  Check_SimVec4(Set(55.0), dot);
}

//--------------------------------------------------------------------- Length 1
TEST(SimVec_Length_1)
{
  //First straight up length
  SimVec vec = Set4(scalar(2.0), scalar(5.0), scalar(3.0), scalar(6.0));
  SimVec length = Length4(vec);
  Check_SimVec4(Set(scalar(8.6023252670426267717294735350497)), length);
}

//--------------------------------------------------------------------- Length 2
TEST(SimVec_Length_2)
{
  //More simple length, no tricks here
  SimVec vec = Set4(scalar(1.0), scalar(0.0), scalar(0.0), scalar(0.0));
  SimVec length = Length4(vec);
  Check_SimVec4(Set(scalar(1.0)), length);
}

//--------------------------------------------------------------------- Length 3
TEST(SimVec_Length_3)
{
  SimVec vec = Set4(scalar(0.0), scalar(0.0), scalar(0.0), scalar(0.0));
  SimVec length = Length4(vec);
  Check_SimVec4(Set(scalar(0.0)), length);
}

//------------------------------------------------------------- Length Squared 1
TEST(SimVec_LengthSquared_1)
{
  //Squared length
  SimVec vec = Set4(scalar(2.0), scalar(5.0), scalar(3.0), scalar(6.0));
  SimVec length = LengthSq4(vec);
  Check_SimVec4(Set(scalar(74.0)), length);
}

//------------------------------------------------------------- Length Squared 2
TEST(SimVec_LengthSquared_2)
{
  //Simple length!
  SimVec vec = Set4(scalar(1.0), scalar(0.0), scalar(0.0), scalar(0.0));
  SimVec length = LengthSq4(vec);
  Check_SimVec4(Set(scalar(1.0)), length);
}

//----------------------------------------------------------------- Normalized 1
TEST(SimVec_Normalized_1)
{
  //First norm, the values are SO close that this test should pass. Look at how
  //small that epsilon is!
  SimVec vec = Set4(scalar(2.0), scalar(3.0), scalar(4.0), scalar(5.0));
  SimVec norm = Normalize4(vec);
  SimVec expected = Set4(scalar(0.27216552697590867757747600830065),
    scalar(0.40824829046386301636621401245098),
    scalar(0.54433105395181735515495201660131),
    scalar(0.68041381743977169394369002075163));
  Check_SimVec4_Close(expected, norm, scalar(0.000001));
}

//----------------------------------------------------------------- Normalized 2
TEST(SimVec_Normalized_2)
{
  //Stupid norm
  SimVec vec = Set4(scalar(1.0), scalar(0.0), scalar(0.0), scalar(0.0));
  SimVec norm = Normalize4(vec);
  SimVec expected = Set4(scalar(1.0), scalar(0.0), scalar(0.0), scalar(0.0));
  Check_SimVec4(expected, norm);
}

//---------------------------------------------------------- Attempt Normalize 1
TEST(SimVec_AttemptNormalize_1)
{
  //Now attempt normalize! This should work.
  SimVec vec = Set4(scalar(2.0), scalar(3.0), scalar(4.0), scalar(5.0));
  SimVec actual = AttemptNormalize4(vec);
  SimVec expected = Set4(scalar(0.27216552697590867757747600830065),
                         scalar(0.40824829046386301636621401245098),
                         scalar(0.54433105395181735515495201660131),
                         scalar(0.68041381743977169394369002075163));
  Check_SimVec4_Close(expected, actual, scalar(0.000001));
}

//---------------------------------------------------------- Attempt Normalize 2
TEST(SimVec_AttemptNormalize_2)
{
  //This is why attempt normalize was made
  SimVec vec = Set4(scalar(0.0), scalar(0.0), scalar(0.0), scalar(0.0));
  SimVec actual = AttemptNormalize4(vec);
  SimVec expected = Set4(scalar(0.0), scalar(0.0), scalar(0.0), scalar(0.0));
  Check_SimVec4(expected, actual);
}

//---------------------------------------------------------- Attempt Normalize 3
TEST(SimVec_AttemptNormalize_3)
{
  //Now for a scalarly small vector
  SimVec vec = Set4(scalar(0.0001), scalar(0.0001), scalar(0.0), scalar(0.0));
  SimVec actual = AttemptNormalize4(vec);
  SimVec expected = Set4(scalar(0.70710678118654752440084436210485),
                         scalar(0.70710678118654752440084436210485),
                         scalar(0.0),
                         scalar(0.0));
  Check_SimVec4_Close(expected, actual, scalar(0.000001));
}

//--------------------------------------------------------------------- Negate 1
TEST(SimVec_Negate_1)
{
  SimVec vec = Set4(scalar(67408.4), scalar(-3967.352), scalar(-9085.634), scalar(88.88));
  vec = Negate(vec);
  SimVec expected = Set4(scalar(-67408.4), scalar(3967.352), scalar(9085.634), scalar(-88.88));
  Check_SimVec4(expected, vec);
}

//--------------------------------------------------------------------- Negate 2
TEST(SimVec_Negate_2)
{
  SimVec vec = Set4(scalar(67408.4), scalar(-3967.352), scalar(-9085.634), scalar(88.88));
  SimVec result = Negate(vec);
  SimVec expected = Set4(scalar(-67408.4), scalar(3967.352), scalar(9085.634), scalar(-88.88));
  Check_SimVec4(expected, result);
}

//------------------------------------------------------------------- Global Abs
TEST(SimVec_Global_Abs)
{
  SimVec vec = Set4(scalar(-1.5), scalar(8.3), scalar(-27.5), scalar(-54.32));
  SimVec result = Abs(vec);
  SimVec expected = Set4(scalar(1.5), scalar(8.3), scalar(27.5), scalar(54.32));
  Check_SimVec4(expected, result);
}

//------------------------------------------------------------------- Global Min
TEST(SimVec_Global_Min)
{
  SimVec vecA = Set4(scalar(79.3), scalar(-32.4), scalar(2526.3), scalar(999.9));
  SimVec vecB = Set4(scalar(2384.66), scalar(0.0), scalar(728.2), scalar(998.9));
  SimVec result = Min(vecA, vecB);
  SimVec expected = Set4(scalar(79.3), scalar(-32.4), scalar(728.2), scalar(998.9));
  Check_SimVec4(expected, result);
}

//------------------------------------------------------------------- Global Max
TEST(SimVec_Global_Max)
{
  SimVec vecA = Set4(scalar(99.3), scalar(27.6), scalar(-7682.43), scalar(-33.2));
  SimVec vecB = Set4(scalar(333.3), scalar(26.7), scalar(-3.2), scalar(8.0));
  SimVec result = Max(vecA, vecB);
  SimVec expected = Set4(scalar(333.3), scalar(27.6), scalar(-3.2), scalar(8.0));
  Check_SimVec4(expected, result);
}

//---------------------------------------------------------------- Global Lerp 1
TEST(SimVec_Global_Lerp_1)
{
  SimVec vecA = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec vecB = Set4(scalar(8.0), scalar(1.0), scalar(12.0), scalar(7.0));
  SimVec result = Lerp(vecA, vecB, scalar(0.5));
  SimVec expected = Set4(scalar(4.5), scalar(1.5), scalar(7.5), scalar(5.5));
  Check_SimVec4(expected, result);
}

//---------------------------------------------------------------- Global Lerp 2
TEST(SimVec_Global_Lerp_2)
{
  SimVec vecA = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec vecB = Set4(scalar(8.0), scalar(1.0), scalar(12.0), scalar(7.0));
  SimVec result = Lerp(vecA, vecB, scalar(0.0));
  SimVec expected = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  Check_SimVec4(expected, result);
}

//---------------------------------------------------------------- Global Lerp 3
TEST(SimVec_Global_Lerp_3)
{
  SimVec vecA = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec vecB = Set4(scalar(8.0), scalar(1.0), scalar(12.0), scalar(7.0));
  SimVec result = Lerp(vecA, vecB, scalar(1.0));
  SimVec expected = Set4(scalar(8.0), scalar(1.0), scalar(12.0), scalar(7.0));
  Check_SimVec4(expected, result);
}

//---------------------------------------------------------------- Select 1
TEST(SimVec_Select_1)
{
  SimVec vecA = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec vecB = Set4(scalar(5.0), scalar(6.0), scalar(7.0), scalar(8.0));
  SimVec actual = Select(vecA,vecB,OrVec(gSimMaskX,gSimMaskZ));
  SimVec expected = Set4(scalar(5.0), scalar(2.0), scalar(7.0), scalar(4.0));
  Check_SimVec4(expected, actual);
}

//---------------------------------------------------------------- Select 2
TEST(SimVec_Select_2)
{
  SimVec vecA = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec vecB = Set4(scalar(5.0), scalar(6.0), scalar(7.0), scalar(8.0));
  SimVec actual = Select(vecA,vecB,OrVec(gSimMaskY,gSimMaskW));
  SimVec expected = Set4(scalar(1.0), scalar(6.0), scalar(3.0), scalar(8.0));
  Check_SimVec4(expected, actual);
}

//---------------------------------------------------------------- Multiply Add
TEST(SimVec_MultiplyAdd)
{
  SimVec vecA = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec vecB = Set4(scalar(5.0), scalar(6.0), scalar(7.0), scalar(8.0));
  SimVec vecC = Set4(scalar(9.0),scalar(10.0),scalar(11.0),scalar(12.0));
  SimVec actual = MultiplyAdd(vecA,vecB,vecC);
  SimVec expected = Set4(scalar(14), scalar(22.0), scalar(32.0), scalar(44.0));
  Check_SimVec4(expected, actual);
}

//---------------------------------------------------------------- NegativeMultiplySubtract
TEST(SimVec_NegativeMultiplySubtract)
{
  SimVec vecA = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec vecB = Set4(scalar(5.0), scalar(6.0), scalar(7.0), scalar(8.0));
  SimVec vecC = Set4(scalar(9.0),scalar(10.0),scalar(11.0),scalar(12.0));
  SimVec actual = MultiplySubtract(vecA,vecB,vecC);
  SimVec expected = Set4(scalar(4), scalar(-2.0), scalar(-10.0), scalar(-20.0));
  Check_SimVec4(expected, actual);
}
