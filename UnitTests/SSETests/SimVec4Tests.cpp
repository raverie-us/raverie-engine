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


//------------------------------------------------------------- Equal Comparison
TEST(SimVec4_EqualComparison)
{
  //Equality
  SimVec vec = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec cev = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec result = Equal(vec,cev);
  CheckBool_SimVec4(result);
}

//--------------------------------------------------------- Not Equal Comparison
TEST(SimVec4_NotEqualComparison)
{
  //Inequality
  SimVec vec = Set4(scalar(1.0), scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec cev = Set4(scalar(3.0), scalar(1.0), scalar(5.0), scalar(8.0));
  SimVec result = NotEqual(vec,cev);
  CheckBool_SimVec4(result);
}

//------------------------------------------------------------------ Dot Product
TEST(SimVec4_DotProduct)
{
  //Dot product!
  SimVec vec = Set4(scalar(2.0), scalar(3.0), scalar(4.0), scalar(5.0));
  SimVec cev = Set4(scalar(6.0), scalar(1.0), scalar(5.0), scalar(4.0));
  SimVec dot = Dot4(vec,cev);
  Check_SimVec4(Set(scalar(55.0)), dot);
}

//--------------------------------------------------------------------- Length 1
TEST(SimVec4_Length_1)
{
  //First straight up length
  SimVec vec = Set4(scalar(2.0), scalar(5.0), scalar(3.0), scalar(6.0));
  SimVec length = Length4(vec);
  Check_SimVec4(Set(scalar(8.6023252670426267717294735350497)), length);
}

//--------------------------------------------------------------------- Length 2
TEST(SimVec4_Length_2)
{
  //More simple length, no tricks here
  SimVec vec = Set4(scalar(1.0), scalar(0.0), scalar(0.0), scalar(0.0));
  SimVec length = Length4(vec);
  Check_SimVec4(Set(scalar(1.0)), length);
}

//--------------------------------------------------------------------- Length 3
TEST(SimVec4_Length_3)
{
  SimVec vec = Set4(scalar(0.0), scalar(0.0), scalar(0.0), scalar(0.0));
  SimVec length = Length4(vec);
  Check_SimVec4(Set(scalar(0.0)), length);
}

//------------------------------------------------------------- Length Squared 1
TEST(SimVec4_LengthSquared_1)
{
  //Squared length
  SimVec vec = Set4(scalar(2.0), scalar(5.0), scalar(3.0), scalar(6.0));
  SimVec length = LengthSq4(vec);
  Check_SimVec4(Set(scalar(74.0)), length);
}

//------------------------------------------------------------- Length Squared 2
TEST(SimVec4_LengthSquared_2)
{
  //Simple length!
  SimVec vec = Set4(scalar(1.0), scalar(0.0), scalar(0.0), scalar(0.0));
  SimVec length = LengthSq4(vec);
  Check_SimVec4(Set(scalar(1.0)), length);
}

//----------------------------------------------------------------- Normalized 1
TEST(SimVec4_Normalized_1)
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
TEST(SimVec4_Normalized_2)
{
  //Stupid norm
  SimVec vec = Set4(scalar(1.0), scalar(0.0), scalar(0.0), scalar(0.0));
  SimVec norm = Normalize4(vec);
  SimVec expected = Set4(scalar(1.0), scalar(0.0), scalar(0.0), scalar(0.0));
  Check_SimVec4(expected, norm);
}
