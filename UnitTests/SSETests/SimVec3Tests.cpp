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

//------------------------------------------------------------------ Dot Product
TEST(SimVec3_DotProduct)
{
  //Dot product!
  SimVec vec = Set3(scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec cev = Set3(scalar(6.0), scalar(1.0), scalar(5.0));
  SimVec dot = Dot3(vec,cev);
  Check_SimVec3(Set(scalar(35.0)), dot);
}

//--------------------------------------------------------------------- Length 1
TEST(SimVec3_Length_1)
{
  //First straight up length
  SimVec vec = Set3(scalar(2.0), scalar(5.0), scalar(3.0));
  SimVec length = Length3(vec);
  Check_SimVec3(Set(scalar(6.1644140029689764502501923814542)), length);
}

//--------------------------------------------------------------------- Length 2
TEST(SimVec3_Length_2)
{
  //More simple length, no tricks here
  SimVec vec = Set3(scalar(1.0), scalar(0.0), scalar(0.0));
  SimVec length = Length3(vec);
  Check_SimVec3(Set(scalar(1.0)), length);
}

//------------------------------------------------------------- Length Squared 1
TEST(SimVec3_LengthSquared_1)
{
  //Squared length
  SimVec vec = Set3(scalar(2.0), scalar(5.0), scalar(3.0));
  SimVec length = LengthSq3(vec);
  Check_SimVec3(Set(scalar(38.0)), length);
}

//------------------------------------------------------------- Length Squared 2
TEST(SimVec3_LengthSquared_2)
{
  //Simple length!
  SimVec vec = Set3(scalar(1.0), scalar(0.0), scalar(0.0));
  SimVec length = LengthSq3(vec);
  Check_SimVec3(Set(scalar(1.0)), length);
}

//----------------------------------------------------------------- Normalized 1
TEST(SimVec3_Normalized_1)
{
  //First norm, the values are SO close that this test should pass. Look at how
  //small that epsilon is!
  SimVec vec = Set3(scalar(2.0), scalar(3.0), scalar(4.0));
  SimVec norm = Normalize3(vec);
  SimVec expected = Set3(scalar(0.37139067635410372629315244769244),
                          scalar(0.55708601453115558943972867153866),
                          scalar(0.74278135270820745258630489538488));
  Check_SimVec3_Close(expected, norm, scalar(0.000001));
}

//----------------------------------------------------------------- Normalized 2
TEST(SimVec3_Normalized_2)
{
  //Stupid norm
  SimVec vec = Set3(scalar(1.0), scalar(0.0), scalar(0.0));
  SimVec norm = Normalize3(vec);
  SimVec expected = Set3(scalar(1.0), scalar(0.0), scalar(0.0));
  Check_SimVec3(expected, norm);
}

//-------------------------------------------------------------- Cross Product 1
TEST(SimVec3_CrossProduct_1)
{
  //Cross product!
  SimVec vec = Set3(scalar(1.0), scalar(0.0), scalar(0.0));
  SimVec cev = Set3(scalar(0.0), scalar(1.0), scalar(0.0));
  SimVec cross = Cross3(vec,cev);
  SimVec expected = Set3(scalar(0.0), scalar(0.0), scalar(1.0));
  Check_SimVec3(expected, cross);
}
//-------------------------------------------------------------- Cross Product 2
TEST(SimVec3_CrossProduct_2)
{
  //Zero
  SimVec vec = Set3(scalar(0.0), scalar(0.0), scalar(0.0));
  SimVec cev = Set3(scalar(0.0), scalar(1.0), scalar(0.0));
  SimVec cross = Cross3(vec,cev);
  SimVec expected = Set3(scalar(0.0), scalar(0.0), scalar(0.0));
  Check_SimVec3(expected, cross);
}

//-------------------------------------------------------------- Cross Product 3
TEST(SimVec3_CrossProduct_3)
{
  //Get the y-axis.
  SimVec vec = Set3(scalar(1.0), scalar(0.0), scalar(0.0));
  SimVec cev = Set3(scalar(0.0), scalar(0.0), scalar(-1.0));
  SimVec cross = Cross3(vec,cev);
  SimVec expected = Set3(scalar(0.0), scalar(1.0), scalar(0.0));
  Check_SimVec3(expected, cross);
}
