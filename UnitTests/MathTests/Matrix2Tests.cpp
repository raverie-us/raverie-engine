///////////////////////////////////////////////////////////////////////////////
///
/// \file MatrixTests.cpp
/// Unit tests for Matrix3.
///
/// Authors: Benjamin Strukus
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
typedef unsigned int uint;
typedef uint u32;
#include "Math/Reals.hpp"
#include "Math/Matrix2.hpp"
#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"
#include "Math/Quaternion.hpp"
#include "Math/Math.hpp"

typedef Math::real        real;
typedef Math::Matrix2     Mat2;
typedef Math::Matrix3     Mat3;
typedef Math::Matrix4     Mat4;
typedef Math::Vector2     Vec2;
typedef Math::Vector3     Vec3;
typedef Math::Vector4     Vec4;
typedef Math::Quaternion  Quat;

namespace
{
//Setting the values of a matrix every time is really irritating, don't change!
const real init2[4]  = { real( 2.0), real( 3.0),
                         real( 5.0), real( 7.0)};
const real init22[2][2] = { { real( 2.0), real( 3.0) },
                            { real( 5.0), real( 7.0)}};

}

#define MTX(a,b,c,d)         \
{ { real((a)), real((b)) },  \
{ real((c)), real((d)) } }  

//------------------------------------------------------- Identity (Static Data)
TEST(Matrix2_Identity_StaticData)
{
  Mat2 identity = Mat2::cIdentity;
  real expected[2][2] = MTX(1.0, 0.0, 0.0, 1.0);
  CHECK_MAT2(expected, identity);
}

//------------------------------------------------------------------ Constructor
TEST(Matrix2_Constructor)
{
  //Normal constructor
  Mat2 mtx(real(1.0), real(2.0),
           real(3.0), real(4.0));
  real expected[2][2] = MTX(1.0, 2.0, 3.0, 4.0);
  CHECK_MAT2(expected, mtx);
}

//------------------------------------------------------------- Copy Constructor
TEST(Matrix2_CopyConstructor)
{
  //Copy constructor
  Mat2 mtx(real(1.0), real(2.0),
           real(3.0), real(4.0));
  Mat2 copyMtx(mtx);
  real expected[2][2] = MTX(1.0, 2.0, 3.0, 4.0);
  CHECK_MAT2(expected, copyMtx);
}

//------------------------------------------------- Explicit Pointer Constructor
TEST(Matrix2_ExplicitPointerConstructor)
{
  //Data constructor
  Mat2 dataMtx(init2);
  CHECK_MAT2(init22, dataMtx);
}

//----------------------------------------------------------- Initial Assignment
TEST(Matrix2_InitialAssignment)
{
  //Assignment operator
  Mat2 mtx(real(1.0), real(2.0),
           real(3.0), real(4.0));
  Mat2 equMtx = mtx;
  real expected[2][2] = MTX(1.0, 2.0, 3.0, 4.0);
  CHECK_MAT2(expected, equMtx);
}

//------------------------------------------- Scalar Multiplication Assignment 1
TEST(Matrix2_ScalarMultiplicationAssignment_1)
{
  //Multiplication assignment
  Mat2 mtx(init2);
  mtx *= real(2.0);

  real expected[2][2];
  for(int i = 0; i < 2; ++i)
  {
    for(int j = 0; j < 2; ++j)
    {
      expected[i][j] = init22[i][j] * real(2.0);
    }
  }

  CHECK_MAT2(expected, mtx);
}

//------------------------------------------- Scalar Multiplication Assignment 2
TEST(Matrix2_ScalarMultiplicationAssignment_2)
{
  //Multiplication assignment
  Mat2 mtx(init2);
  mtx *= real(0.0);

  real expected[2][2];
  for(int i = 0; i < 2; ++i)
  {
    for(int j = 0; j < 2; ++j)
    {
      expected[i][j] = real(0.0);
    }
  }

  CHECK_MAT2(expected, mtx);
}

//--------------------------------------------------- Scalar Division Assignment
TEST(Matrix2_ScalarDivisionAssignment)
{
  //Division assignment
  Mat2 mtx(init2);
  mtx /= real(10.0);
  //This looks a little funky but it's to mimic what's happening to the matrix.
  //If you just multiply by 0.2 then you get slightly different values.
  real expected[2][2];
  for(int i = 0; i < 2; ++i)
  {
    for(int j = 0; j < 2; ++j)
    {
      expected[i][j] = init22[i][j] / real(10.0);
    }
  }

  CHECK_MAT2(expected, mtx);
}

//-------------------------------------------------------- Scalar Multiplication
TEST(Matrix2_ScalarMultiplication)
{
  Mat2 mtx(init2);

  //Multiplication with scalar
  Mat2 scaled = mtx * real(2.0);

  real expected[2][2];
  for(int i = 0; i < 2; ++i)
  {
    for(int j = 0; j < 2; ++j)
    {
      expected[i][j] = init22[i][j] * real(2.0);
    }
  }

  CHECK_MAT2(expected, scaled);
}

//-------------------------------------------------------------- Scalar Division
TEST(Matrix2_ScalarDivision)
{
  //Division by scalar
  Mat2 mtx(init2);
  Mat2 scaled = mtx;
  scaled = mtx / real(10.0);

  real expected[2][2];
  for(int i = 0; i < 2; ++i)
  {
    for(int j = 0; j < 2; ++j)
    {
      expected[i][j] = init22[i][j] / real(10.0);
    }
  }
  
  CHECK_MAT2(expected, scaled);
}

//--------------------------------------------------- Matrix Addition Assignment
TEST(Matrix2_MatrixAdditionAssignment)
{
  Mat2 mtx(real(0.0), real(1.0),
           real(2.0), real(3.0));

  //Addition assignment
  Mat2 add(real( 2.0), real( 4.0),
           real( 6.0), real( 8.0));
  mtx += add;

  real expected[2][2] = MTX(2.0, 5.0, 8.0, 11.0);
  CHECK_MAT2(expected, mtx);
}

//------------------------------------------------ Matrix Subtraction Assignment
TEST(Matrix2_MatrixSubtractionAssignment)
{
  //Subtraction assignment
  Mat2 mtx(real( 2.0), real( 5.0), 
           real( 8.0), real(11.0));
  Mat2 add(real( 2.0), real( 4.0),
           real( 6.0), real( 8.0));
  add -= mtx;

  real expected[2][2] = MTX( 0.0, -1.0,
                            -2.0, -3.0);
  CHECK_MAT2(expected, add);
}

//-------------------------------------------------------------- Matrix Addition
TEST(Matrix2_MatrixAddition)
{
  Mat2 mtx(real(0.0), real(1.0),
           real(2.0), real(3.0));
  Mat2 add(real( 2.0), real( 4.0),
           real( 6.0), real( 8.0));

  //Addition
  Mat2 res = mtx + add;

  real expected[2][2] = MTX(2.0, 5.0, 8.0, 11.0);
  CHECK_MAT2(expected, res);
}

//----------------------------------------------------------- Matrix Subtraction
TEST(Matrix2_MatrixSubtraction)
{
  Mat2 mtx(real(0.0), real(1.0),
           real(2.0), real(3.0));
  Mat2 add(real( 2.0), real( 4.0),
           real( 6.0), real( 8.0));

  //Subtraction
  Mat2 res = add - mtx;

  real expected[2][2] = MTX(2.0, 3.0, 4.0, 5.0);

  CHECK_MAT2(expected, res);
}

//------------------------------------------------------ Matrix Multiplication 1
TEST(Matrix2_MatrixMultiplication_1)
{
  Mat2 mtx(real(0.0), real(1.0),
           real(2.0), real(3.0));
  Mat2 add(real( 2.0), real( 4.0),
           real( 6.0), real( 8.0));

  //Multiplication
  Mat2 res = add * mtx;

  real expected[2][2] = MTX(  8.0, 14.0,
                             16.0, 30.0);
  CHECK_MAT2(expected, res);
}

//------------------------------------------------------ Matrix Multiplication 2
TEST(Matrix2_MatrixMultiplication_2)
{
  Mat2 mtx(real(0.0), real(1.0),
           real(2.0), real(3.0));
  Mat2 add(real( 2.0), real( 4.0),
           real( 6.0), real( 8.0));

  Mat2 res = mtx * add;

  real expected[2][2] = MTX(  6.0,  8.0,
                             22.0, 32.0);
  CHECK_MAT2(expected, res);
}

//------------------------------------------------------------- Equal Comparison
TEST(Matrix2_EqualComparison)
{
  Mat2 one(init2);
  Mat2 two(init2);

  //Equality
  CHECK(one == two);
}

//--------------------------------------------------------- Not Equal Comparison
TEST(Matrix2_NotEqualComparison)
{
  Mat2 one(init2);
  Mat2 two(init2);

  //Inequality
  two.m00 = real(50.0);
  CHECK(one != two);
}

//----------------------------------------------------- "Subscript" (Read/Write)
TEST(Matrix2_Subscript_ReadWrite)
{
  //Modify
  Mat2 mtx(init2);
  for(int i = 0; i < 2; ++i)
  {
    for(int j = 0; j < 2; ++j)
    {
      real foo = mtx(i, j);
      foo *= real(2.0);
      mtx(i, j) = foo;
    }
  }

  real expected[2][2];
  for(int i = 0; i < 2; ++i)
  {
    for(int j = 0; j < 2; ++j)
    {
      expected[i][j] = init22[i][j] * real(2.0);
    }
  }

  CHECK_MAT2(expected, mtx);
}

//------------------------------------------------------ "Subscript" (Read Only)
TEST(Matrix2_Subscript_ReadOnly)
{
  //Non-modify
  Mat3 mtx(init2);
  real values[2][2];

  for(int i = 0; i < 2; ++i)
  {
    for(int j = 0; j < 2; ++j)
    {
      values[i][j] = mtx(i, j);
    }
  }

  CHECK_MAT2(values, mtx);
}

//------------------------------------------------------------------- Transposed
TEST(Matrix2_Transposed)
{
  Mat2 mtx(real(0.0), real(1.0),
           real(2.0), real(3.0));

  //Transpose
  Mat2 transpose = mtx.Transposed();

  real expected[2][2];
  for(int i = 0; i < 2; ++i)
  {
    for(int j = 0; j < 2; ++j)
    {
      expected[i][j] = mtx(j, i);
    }
  }

  CHECK_MAT2(expected, transpose);
}

//--------------------------------------------------------------------- Inverted
TEST(Matrix2_Inverted)
{
  Mat2 mtx(init2);

  //Inverse
  Mat2 inv = mtx.Inverted();
  real expected[2][2] = MTX( -7, 3, 5, -2);
  CHECK_MAT2(expected, inv);
}

//----------------------------------------------------------------------- Multiply
TEST(Matrix2_Multiply)
{
  Mat2 mtx(init2);
  Mat2 mul(real( 2.0), real( 4.0),
           real( 6.0), real( 8.0));

  //Concat is multiply
  Mat2 result = Math::Multiply(mtx, mul);

  real expected[2][2] = MTX( 22.0, 32.0,
                             52.0, 76.0);
  CHECK_MAT2(expected, result);
}

//--------------------------------------------------------------- Set Identity 1
TEST(Matrix2_SetIdentity_1)
{
  Mat2 mtx(init2);

  //Identity test
  mtx.SetIdentity();

  real expected[2][2] = MTX(1.0, 0.0, 0.0, 1.0);
  CHECK_MAT2(expected, mtx);
}

//--------------------------------------------------------------- Set Identity 2
TEST(Matrix2_SetIdentity_2)
{
  Mat2 mtx(init2);

  //Identity test
  Mat2 iden = mtx.SetIdentity();

  real expected[2][2] = MTX(1.0, 0.0, 0.0, 1.0);
  CHECK_MAT2(expected, iden);
}

//------------------------------------------------------------------- Zero Out 1
TEST(Matrix2_ZeroOut_1)
{
  Mat2 mtx(init2);

  //Zero test
  mtx.ZeroOut();

  real expected[2][2] = MTX(0.0, 0.0, 0.0, 0.0);
  CHECK_MAT2(expected, mtx);
}

//------------------------------------------------------------------- Zero Out 2
TEST(Matrix2_ZeroOut_2)
{
  Mat2 mtx(init2);

  //Zero test
  Mat2 zero = mtx.ZeroOut();

  real expected[2][2] = MTX(0.0, 0.0, 0.0, 0.0);
  CHECK_MAT2(expected, zero);
}

//------------------------------------------------------------------ Determinant
TEST(Matrix2_Determinant)
{
  Mat2 mtx(init2);

  //Determinant
  real det = mtx.Determinant();
  CHECK_EQUAL(real(-1), det);
}

//---------------------------------------------------------------------- Valid 1
TEST(Matrix2_Valid_1)
{
  Mat2 mtx(init2);
  CHECK(mtx.Valid());
}

//---------------------------------------------------------------------- Valid 2
TEST(Matrix2_Valid_2)
{
  Mat2 mtx(init2);
  real zero = real(0.0);
  mtx.m00 /= zero;
  CHECK(mtx.Valid() == false);
}

//------------------------------------------------------------------ Scale (xyz)
TEST(Matrix2_Scale_XYZ)
{
  Mat2 mtx;
  //Scale xyz
  mtx.Scale(real(5.0), real(8.0));

  real expected[2][2] = MTX(5.0, 0.0, 0.0, 8.0);
  CHECK_MAT2(expected, mtx);
}

//--------------------------------------------------------------- Scale (vector)
TEST(Matrix2_Scale_Vector)
{
  Mat2 mtx;

  //Scale vector
  Vec2 scale(real(3.0), real(6.0));
  mtx.Scale(scale);

  real expected[3][3] = MTX(3.0, 0.0, 0.0, 6.0);
  CHECK_MAT2(expected, mtx);
}

//----------------------------------------------------------------- Rotate (xyz)
TEST(Matrix2_Rotate)
{
  Mat2 mtx;

  //Rotate xyz radian
  mtx.Rotate(Math::cPi);
  real expected[2][2] = MTX(-1, 0, 0, -1);
  CHECK_MAT2_CLOSE(expected, mtx, real(0.000001));
}

//---------------------------------------------------------------------- GetBasis 1
TEST(Matrix2_GetBasis_1)
{
  Mat2 mtx(init2);
  Vec2 basis = mtx.GetBasis(0);
  Vec2 expected(mtx(0, 0), mtx(1, 0));
  CHECK_VEC2(expected, basis);
}

//---------------------------------------------------------------------- GetBasis 2
TEST(Matrix2_GetBasis_2)
{
  Mat2 mtx(init2);
  Vec2 basis = mtx.GetBasis(1);
  Vec2 expected(mtx(0, 1), mtx(1, 1));
  CHECK_VEC2(expected, basis);
}

//---------------------------------------------------------------------- GetCross 1
TEST(Matrix2_GetCross_1)
{
  Mat2 mtx(init2);
  Vec2 cross = mtx.GetCross(0);
  Vec2 expected(mtx(0, 0), mtx(0, 1));
  CHECK_VEC2(expected, cross);
}

//---------------------------------------------------------------------- GetCross 2
TEST(Matrix2_GetCross_2)
{
  Mat2 mtx(init2);
  Vec2 cross = mtx.GetCross(1);
  Vec2 expected(mtx(1, 0), mtx(1, 1));
  CHECK_VEC2(expected, cross);
}

//------------------------------------------------- Global Scalar Multiplication
TEST(Matrix2_Global_ScalarMultiplication)
{
  //Now swap the order
  Mat2 mtx(init2);
  Mat2 scaled = mtx;
  scaled = real(5.0) * mtx;

  real expected[2][2];
  for(int i = 0; i < 2; ++i)
  {
    for(int j = 0; j < 2; ++j)
    {
      expected[i][j] = init22[i][j] * real(5.0);
    }
  }

  CHECK_MAT2(expected, scaled);
}

//---------------------------------------------------------------- Global Concat
TEST(Matrix2_Global_Concat)
{
  Mat2 mtx(init2);
  Mat2 mul(real( 2.0), real( 4.0),
           real( 6.0), real( 8.0));
  //Concat stand alone function
  Mat2 result = Multiply(mul, mtx);

  real expected[2][2] = MTX( 24.0, 34.0,
                             52.0, 74.0);
  CHECK_MAT2(expected, result);
}


#undef MTX
