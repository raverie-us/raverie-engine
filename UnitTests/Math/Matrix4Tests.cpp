///////////////////////////////////////////////////////////////////////////////
///
/// \file Matrix4Tests.cpp
/// Unit tests for Matrix4.
///
/// Authors: Benjamin Strukus
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
typedef unsigned int uint;
#include "Math/Reals.hpp"
#include "Math/Matrix3.hpp"
#include "Math/Matrix4.hpp"
#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"
#include "Math/Vector4.hpp"
#include "Math/Quaternion.hpp"
#include "Math/Math.hpp"

typedef Math::real        real;
typedef Math::Matrix3     Mat3;
typedef Math::Matrix4     Mat4;
typedef Math::Vector2     Vec2;
typedef Math::Vector3     Vec3;
typedef Math::Vector4     Vec4;
typedef Math::Quaternion  Quat;

namespace
{
//Setting the values of a matrix every time is really irritating, don't change!
const real init3[9]  = { real( 2.0), real( 3.0), real( 5.0),
                         real( 7.0), real(11.0), real(13.0),
                         real(17.0), real(19.0), real(23.0) };
const real init4[16] = { real( 2.0), real( 3.0), real( 5.0), real( 7.0),
                         real(11.0), real(13.0), real(17.0), real(19.0),
                         real(23.0), real(29.0), real(31.0), real(37.0),
                         real(41.0), real(43.0), real(47.0), real(53.0) };
const real init44[4][4] = { { real( 2.0), real( 3.0), real( 5.0), real( 7.0)  },
                            { real(11.0), real(13.0), real(17.0), real(19.0), },
                            { real(23.0), real(29.0), real(31.0), real(37.0), },
                            { real(41.0), real(43.0), real(47.0), real(53.0)  }
                          };
}

#define MTX3(a,b,c,d,e,f,g,h,i)         \
{ { real((a)), real((b)), real((c)) },  \
  { real((d)), real((e)), real((f)) },  \
  { real((g)), real((h)), real((i)) } }

#define MTX(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)      \
{ { real((a)), real((b)), real((c)), real((d)) }, \
  { real((e)), real((f)), real((g)), real((h)) }, \
  { real((i)), real((j)), real((k)), real((l)) }, \
  { real((m)), real((n)), real((o)), real((p)) } }

//------------------------------------------------------- Identity (Static Data)
TEST(Matrix4_Identity_StaticData)
{
  real expected[4][4] = MTX(1.0, 0.0, 0.0, 0.0,
                            0.0, 1.0, 0.0, 0.0,
                            0.0, 0.0, 1.0, 0.0,
                            0.0, 0.0, 0.0, 1.0);
  CHECK_MAT4(expected, Mat4::cIdentity);
}

//------------------------------------------------------------------ Constructor
TEST(Matrix4_Constructor)
{
  //Normal constructor
  Mat4 norm(real( 1.0), real( 2.0), real( 3.0), real( 4.0),
            real( 5.0), real( 6.0), real( 7.0), real( 8.0),
            real( 9.0), real(10.0), real(11.0), real(12.0),
            real(13.0), real(14.0), real(15.0), real(16.0));
  real expected[4][4] = MTX( 1.0,  2.0,  3.0,  4.0,
                             5.0,  6.0,  7.0,  8.0,
                             9.0, 10.0, 11.0, 12.0,
                            13.0, 14.0, 15.0, 16.0);
  CHECK_MAT4(expected, norm);
}

//------------------------------------------------------------- Copy Constructor
TEST(Matrix4_CopyConstructor)
{
  //Copy constructor
  Mat4 prime(init4);
  Mat4 copy(prime);
  CHECK_MAT4(init44, copy);
}

//------------------------------------------------- Explicit Pointer Constructor
TEST(Matrix4_ExplicitPointerConstructor)
{
  //Data constructor
  Mat4 prime(init4);
  CHECK_MAT4(init44, prime);
}

//----------------------------------------------------------- Initial Assignment
TEST(Matrix4_InitialAssignment)
{
  //Assignment operator
  Mat4 copy(init4);
  Mat4 equal = copy;
  CHECK_MAT4(init44, equal);
}

//------------------------------------------- Scalar Multiplication Assignment 1
TEST(Matrix4_ScalarMultiplicationAssignment_1)
{
  //Multiplication assignment
  Mat4 mtx(init4);
  mtx *= real(2.0);

  real expected[4][4];
  for(int i = 0; i < 4; ++i)
  {
    for(int j = 0; j < 4; ++j)
    {
      expected[i][j] = init44[i][j] * real(2.0);
    }
  }
  CHECK_MAT4(expected, mtx);
}

//------------------------------------------- Scalar Multiplication Assignment 2
TEST(Matrix4_ScalarMultiplicationAssignment_2)
{
  //Multiplication assignment
  Mat4 mtx(init4);
  mtx *= real(0.0);

  real expected[4][4];
  for(int i = 0; i < 4; ++i)
  {
    for(int j = 0; j < 4; ++j)
    {
      expected[i][j] = real(0.0);
    }
  }
  CHECK_MAT4(expected, mtx);
}

//--------------------------------------------------- Scalar Division Assignment
TEST(Matrix4_ScalarDivisionAssignment)
{
  //Multiplication assignment
  Mat4 mtx(init4);
  mtx /= real(10.0);

  real expected[4][4];
  for(int i = 0; i < 4; ++i)
  {
    for(int j = 0; j < 4; ++j)
    {
      expected[i][j] = init44[i][j] / real(10.0);
    }
  }
  CHECK_MAT4_CLOSE(expected, mtx, 0.0001f);
}

//-------------------------------------------------------- Scalar Multiplication
TEST(Matrix4_ScalarMultiplication)
{
  Mat4 mtx(init4);

  //Multiplication with scalar
  Mat4 scaled = mtx * real(2.0);

  real expected[4][4];
  for(int i = 0; i < 4; ++i)
  {
    for(int j = 0; j < 4; ++j)
    {
      expected[i][j] = init44[i][j] * real(2.0);
    }
  }
  CHECK_MAT4(expected, scaled);
}

//-------------------------------------------------------------- Scalar Division
TEST(Matrix4_ScalarDivision)
{
  //Division by scalar
  Mat4 mtx(init4);
  Mat4 scaled = mtx / real(10.0);

  real expected[4][4];
  for(int i = 0; i < 4; ++i)
  {
    for(int j = 0; j < 4; ++j)
    {
      expected[i][j] = init44[i][j] / real(10.0);
    }
  }
  CHECK_MAT4_CLOSE(expected, scaled, real(0.000001));
}

//--------------------------------------------------- Matrix Addition Assignment
TEST(Matrix4_MatrixAdditionAssignment)
{
  Mat4 mtx(real( 0.0), real( 1.0), real( 2.0), real( 3.0),
           real( 4.0), real( 5.0), real( 6.0), real( 7.0),
           real( 8.0), real( 9.0), real(10.0), real(11.0),
           real(12.0), real(13.0), real(14.0), real(15.0));

  //Addition assignment
  Mat4 add(real( 2.0), real( 4.0), real( 6.0), real( 8.0),
           real(10.0), real(12.0), real(14.0), real(16.0),
           real(18.0), real(20.0), real(22.0), real(24.0),
           real(26.0), real(28.0), real(30.0), real(32.0));
  mtx += add;

  real expected[4][4] = MTX( 2.0,  5.0,  8.0, 11.0,
                            14.0, 17.0, 20.0, 23.0,
                            26.0, 29.0, 32.0, 35.0,
                            38.0, 41.0, 44.0, 47.0);
  CHECK_MAT4(expected, mtx);
}

//------------------------------------------------ Matrix Subtraction Assignment
TEST(Matrix4_MatrixSubtractionAssignment)
{
  Mat4 mtx(real( 2.0), real( 5.0), real( 8.0), real(11.0),
           real(14.0), real(17.0), real(20.0), real(23.0),
           real(26.0), real(29.0), real(32.0), real(35.0),
           real(38.0), real(41.0), real(44.0), real(47.0));

  //Subtraction assignment
  Mat4 add(real( 2.0), real( 4.0), real( 6.0), real( 8.0),
           real(10.0), real(12.0), real(14.0), real(16.0),
           real(18.0), real(20.0), real(22.0), real(24.0),
           real(26.0), real(28.0), real(30.0), real(32.0));
  add -= mtx;

  real expected[4][4] = MTX(  0.0,  -1.0,  -2.0,  -3.0,
                             -4.0,  -5.0,  -6.0,  -7.0,
                             -8.0,  -9.0, -10.0, -11.0, 
                            -12.0, -13.0, -14.0, -15.0);
  CHECK_MAT4(expected, add);
}

//-------------------------------------------------------------- Matrix Addition
TEST(Matrix4_MatrixAddition)
{
  Mat4 mtx(real( 0.0), real( 1.0), real( 2.0), real( 3.0),
           real( 4.0), real( 5.0), real( 6.0), real( 7.0),
           real( 8.0), real( 9.0), real(10.0), real(11.0),
           real(12.0), real(13.0), real(14.0), real(15.0));

  Mat4 add(real( 2.0), real( 4.0), real( 6.0), real( 8.0),
           real(10.0), real(12.0), real(14.0), real(16.0),
           real(18.0), real(20.0), real(22.0), real(24.0),
           real(26.0), real(28.0), real(30.0), real(32.0));

  //Addition
  Mat4 res = mtx + add;

  real expected[4][4] = MTX( 2.0,  5.0,  8.0, 11.0,
                            14.0, 17.0, 20.0, 23.0,
                            26.0, 29.0, 32.0, 35.0,
                            38.0, 41.0, 44.0, 47.0);
  CHECK_MAT4(expected, res);
}

//----------------------------------------------------------- Matrix Subtraction
TEST(Matrix4_MatrixSubtraction)
{
  Mat4 mtx(real( 0.0), real( 1.0), real( 2.0), real( 3.0),
           real( 4.0), real( 5.0), real( 6.0), real( 7.0),
           real( 8.0), real( 9.0), real(10.0), real(11.0),
           real(12.0), real(13.0), real(14.0), real(15.0));

  Mat4 add(real( 2.0), real( 4.0), real( 6.0), real( 8.0),
           real(10.0), real(12.0), real(14.0), real(16.0),
           real(18.0), real(20.0), real(22.0), real(24.0),
           real(26.0), real(28.0), real(30.0), real(32.0));

  //Subtraction
  Mat4 res = add - mtx;

  real expected[4][4] = MTX( 2.0,  3.0,  4.0,  5.0,
                             6.0,  7.0,  8.0,  9.0,
                            10.0, 11.0, 12.0, 13.0,
                            14.0, 15.0, 16.0, 17.0);
  CHECK_MAT4(expected, res);
}

//------------------------------------------------------ Matrix Multiplication 1
TEST(Matrix4_MatrixMultiplication_1)
{
  Mat4 mtx(real( 0.0), real( 1.0), real( 2.0), real( 3.0),
           real( 4.0), real( 5.0), real( 6.0), real( 7.0),
           real( 8.0), real( 9.0), real(10.0), real(11.0),
           real(12.0), real(13.0), real(14.0), real(15.0));

  Mat4 add(real( 2.0), real( 4.0), real( 6.0), real( 8.0),
           real(10.0), real(12.0), real(14.0), real(16.0),
           real(18.0), real(20.0), real(22.0), real(24.0),
           real(26.0), real(28.0), real(30.0), real(32.0));

  //Multiplication
  Mat4 res = add * mtx;

  real expected[4][4] = MTX(160.0, 180.0, 200.0,  220.0,
                            352.0, 404.0, 456.0,  508.0,
                            544.0, 628.0, 712.0,  796.0,
                            736.0, 852.0, 968.0, 1084.0);
  CHECK_MAT4(expected, res);
}

//------------------------------------------------------ Matrix Multiplication 2
TEST(Matrix4_MatrixMultiplication_2)
{
  Mat4 mtx(real( 0.0), real( 1.0), real( 2.0), real( 3.0),
           real( 4.0), real( 5.0), real( 6.0), real( 7.0),
           real( 8.0), real( 9.0), real(10.0), real(11.0),
           real(12.0), real(13.0), real(14.0), real(15.0));

  Mat4 add(real( 2.0), real( 4.0), real( 6.0), real( 8.0),
           real(10.0), real(12.0), real(14.0), real(16.0),
           real(18.0), real(20.0), real(22.0), real(24.0),
           real(26.0), real(28.0), real(30.0), real(32.0));
  Mat4 res = mtx * add;

  real expected[4][4] = MTX(124.0, 136.0,  148.0,  160.0,
                            348.0, 392.0,  436.0,  480.0,
                            572.0, 648.0,  724.0,  800.0,
                            796.0, 904.0, 1012.0, 1120.0);
  CHECK_MAT4(expected, res);
}

//------------------------------------------------------------- Equal Comparison
TEST(Matrix4_EqualComparison)
{
  Mat4 one(init4);
  Mat4 two(init4);
  CHECK(one == two);
}

//--------------------------------------------------------- Not Equal Comparison
TEST(Matrix4_NotEqualComparison)
{
  Mat4 one(init4);
  Mat4 two(init4);
  two.m02 = real(80.0);
  CHECK(one != two);
}

//----------------------------------------------------- "Subscript" (Read/Write)
TEST(Matrix4_Subscript_ReadWrite)
{
  //Modify
  Mat4 mtx(init4);
  for(int i = 0; i < 4; ++i)
  {
    for(int j = 0; j < 4; ++j)
    {
      real foo = mtx(i, j);
      foo *= real(2.0);
      mtx(i, j) = foo;
    }
  }

  real expected[4][4];
  for(int i = 0; i < 4; ++i)
  {
    for(int j = 0; j < 4; ++j)
    {
      expected[i][j] = init44[i][j] * real(2.0);
    }
  }

  CHECK_MAT4(expected, mtx);
}

//------------------------------------------------------ "Subscript" (Read Only)
TEST(Matrix4_Subscript_ReadOnly)
{
  //Non-modify
  Mat4 mtx(init4);
  real values[4][4];

  for(int i = 0; i < 4; ++i)
  {
    for(int j = 0; j < 4; ++j)
    {
      values[i][j] = mtx(i, j);
    }
  }

  CHECK_MAT4(values, mtx);
}

//------------------------------------------------------------------- Transposed
TEST(Matrix4_Transposed)
{
  Mat4 mtx(real( 0.0), real( 1.0), real( 2.0), real( 3.0),
           real( 4.0), real( 5.0), real( 6.0), real( 7.0),
           real( 8.0), real( 9.0), real(10.0), real(11.0),
           real(12.0), real(13.0), real(14.0), real(15.0));

  //Transpose
  Mat4 transpose = mtx.Transposed();

  real expected[4][4] = MTX(0.0, 4.0,  8.0, 12.0,
                            1.0, 5.0,  9.0, 13.0,
                            2.0, 6.0, 10.0, 14.0,
                            3.0, 7.0, 11.0, 15.0);
  CHECK_MAT4(expected, transpose);
}

//-------------------------------------------------------------------- Transpose
TEST(Matrix4_Transpose)
{
  Mat4 mtx(real( 0.0), real( 1.0), real( 2.0), real( 3.0),
           real( 4.0), real( 5.0), real( 6.0), real( 7.0),
           real( 8.0), real( 9.0), real(10.0), real(11.0),
           real(12.0), real(13.0), real(14.0), real(15.0));

  //Transpose this
  mtx.Transpose();
  real expected[4][4] = MTX(0.0, 4.0,  8.0, 12.0,
                            1.0, 5.0,  9.0, 13.0,
                            2.0, 6.0, 10.0, 14.0,
                            3.0, 7.0, 11.0, 15.0);
  CHECK_MAT4(expected, mtx);
}

//--------------------------------------------------------------------- Inverted
TEST(Matrix4_Inverted)
{
  Mat4 mtx(init4);

  //Inverse
  Mat4 inv = mtx.Inverted();

  real res[4][4] = MTX(  3.0 / 11.0,  -12.0 / 55.0,  -1.0 / 5.0,  2.0 / 11.0,
                        -5.0 / 11.0,   -2.0 / 55.0,  3.0 / 10.0, -3.0 / 22.0,
                       -13.0 / 22.0, 307.0 / 440.0, -1.0 / 10.0, -9.0 / 88.0,
                        15.0 / 22.0,  -37.0 / 88.0,         0.0,  7.0 / 88.0);
  CHECK_MAT4_CLOSE(res, inv, real(0.000001));
}

//----------------------------------------------------------------------- Invert
TEST(Matrix4_Invert)
{
  //Inverse this
  Mat4 mtx(init4);
  mtx.Invert();
  real res[4][4] = MTX(  3.0 / 11.0,  -12.0 / 55.0,  -1.0 / 5.0,  2.0 / 11.0,
                        -5.0 / 11.0,   -2.0 / 55.0,  3.0 / 10.0, -3.0 / 22.0,
                       -13.0 / 22.0, 307.0 / 440.0, -1.0 / 10.0, -9.0 / 88.0,
                        15.0 / 22.0,  -37.0 / 88.0,         0.0,  7.0 / 88.0);
  CHECK_MAT4_CLOSE(res, mtx, real(0.000001));
}

//----------------------------------------------------------------------- Multiply
TEST(Matrix4_Multiply)
{
  Mat4 mtx(init4);
  Mat4 mul(real( 2.0), real( 4.0), real( 6.0), real( 8.0),
           real(10.0), real(12.0), real(14.0), real(16.0),
           real(18.0), real(20.0), real(22.0), real(24.0),
           real(26.0), real(28.0), real(30.0), real(32.0));

  //Concat is multiply
  Mat4 result = Mat4::Multiply(mtx, mul);
  real expected[4][4] = MTX( 306.0,  340.0,  374.0,  408.0,
                             952.0, 1072.0, 1192.0, 1312.0,
                            1856.0, 2096.0, 2336.0, 2576.0,
                            2736.0, 3104.0, 3472.0, 3840.0);
  CHECK_MAT4(expected, result);
}

//---------------------------------------------------------------- SetIdentity 1
TEST(Matrix4_SetIdentity_1)
{
  Mat4 mtx(init4);

  //Identity test
  mtx.SetIdentity();

  real expected[4][4] = MTX(1.0, 0.0, 0.0, 0.0,
                            0.0, 1.0, 0.0, 0.0,
                            0.0, 0.0, 1.0, 0.0,
                            0.0, 0.0, 0.0, 1.0);
  CHECK_MAT4(expected, mtx);
}

//--------------------------------------------------------------- Set Identity 2
TEST(Matrix4_SetIdentity_2)
{
  Mat4 mtx(init4);

  //Identity test
  Mat4 iden = mtx.SetIdentity();

  real expected[4][4] = MTX(1.0, 0.0, 0.0, 0.0,
                            0.0, 1.0, 0.0, 0.0,
                            0.0, 0.0, 1.0, 0.0,
                            0.0, 0.0, 0.0, 1.0);
  CHECK_MAT4(expected, iden);
}

//------------------------------------------------------------------- Zero Out 1
TEST(Matrix4_ZeroOut_1)
{
  Mat4 mtx(init4);

  //Zero test
  mtx.ZeroOut();

  real expected[4][4] = MTX(0.0, 0.0, 0.0, 0.0,
                            0.0, 0.0, 0.0, 0.0,
                            0.0, 0.0, 0.0, 0.0,
                            0.0, 0.0, 0.0, 0.0);
  CHECK_MAT4(expected, mtx);
}

//------------------------------------------------------------------- Zero Out 2
TEST(Matrix4_ZeroOut_2)
{
  Mat4 mtx(init4);

  //Zero test
  Mat4 zero = mtx.ZeroOut();

  real expected[4][4] = MTX(0.0, 0.0, 0.0, 0.0,
                            0.0, 0.0, 0.0, 0.0,
                            0.0, 0.0, 0.0, 0.0,
                            0.0, 0.0, 0.0, 0.0);
  CHECK_MAT4(expected, zero);
}

//------------------------------------------------------------------ Determinant
TEST(Matrix4_Determinant)
{
  Mat4 mtx(init4);

  //Determinant
  real det = mtx.Determinant();
  CHECK_EQUAL(real(880.0), det);
}

//---------------------------------------------------------------------- Valid 1
TEST(Matrix4_Valid_1)
{
  Mat4 mtx(init4);
  CHECK(mtx.Valid());
}

//---------------------------------------------------------------------- Valid 2
TEST(Matrix4_Valid_2)
{
  real zero = real(0.0);
  Mat4 mtx(init4);
  mtx.m00 /= zero;
  CHECK(mtx.Valid() == false);
}

//------------------------------------------------------------------ Scale (xyz)
TEST(Matrix4_Scale_XYZ)
{
  Mat4 mtx;
  //Scale xyz
  mtx.Scale(real(5.0), real(8.0), real(2.0));

  real expected[4][4] = MTX(5.0, 0.0, 0.0, 0.0,
                            0.0, 8.0, 0.0, 0.0,
                            0.0, 0.0, 2.0, 0.0,
                            0.0, 0.0, 0.0, 1.0);
  CHECK_MAT4(expected, mtx);
}

//--------------------------------------------------------------- Scale (vector)
TEST(Matrix4_Scale_Vector)
{
  //Scale vector
  Vec3 scale(real(3.0), real(6.0), real(9.0));
  Mat4 mtx;
  mtx.Scale(scale);

  real expected[4][4] = MTX(3.0, 0.0, 0.0, 0.0,
                            0.0, 6.0, 0.0, 0.0,
                            0.0, 0.0, 9.0, 0.0,
                            0.0, 0.0, 0.0, 1.0);
  CHECK_MAT4(expected, mtx);
}

//----------------------------------------------------------------- Rotate (xyz)
TEST(Matrix4_Rotate_XYZ)
{
  Mat4 mtx;

  //Rotate xyz radian
  mtx.Rotate(real(0.57735026918962576450914878050195),
             real(0.57735026918962576450914878050195),
             real(0.57735026918962576450914878050195),
             Math::cPi);
  real expected[4][4] = MTX(-0.33333333333333333333333333333335,
                             0.66666666666666666666666666666665,
                             0.66666666666666666666666666666665,
                                                            0.0,
                             0.66666666666666666666666666666665,
                            -0.33333333333333333333333333333335,
                             0.66666666666666666666666666666665,
                                                            0.0,
                             0.66666666666666666666666666666665,
                             0.66666666666666666666666666666665,
                            -0.33333333333333333333333333333335,
                                                            0.0,
                                                            0.0,
                                                            0.0,
                                                            0.0,
                                                            1.0);
  CHECK_MAT4_CLOSE(expected, mtx, real(0.000001));
}

//-------------------------------------------------------------- Rotate (vector)
TEST(Matrix4_Rotate_Vector)
{
  //Rotate vector radian
  Vec3 axis(1.0f, 1.0f, 0.0f);
  Normalize(axis);
  Mat4 mtx;
  mtx.Rotate(axis, real(0.25) * Math::cPi);

  real expected[4][4] = MTX(0.85355339059327376220042218105242,
                            0.14644660940672623779957781894757,
                                                           0.5,
                                                           0.0,
                            0.14644660940672623779957781894757,
                            0.85355339059327376220042218105242,
                                                          -0.5,
                                                           0.0,
                                                          -0.5,
                                                           0.5,
                            0.70710678118654752440084436210485,
                                                           0.0,
                                                           0.0,
                                                           0.0,
                                                           0.0,
                                                           1.0);
  CHECK_MAT4_CLOSE(expected, mtx, real(0.000001));
}

//-------------------------------------------------------------- Translate (xyz)
TEST(Matrix4_Translate_XYZ)
{
  Mat4 mtx;

  //Translate xyz
  mtx.Translate(real(5.0), real(20.0), real(6.0));

  real expected[4][4] = MTX(1.0, 0.0, 0.0,  5.0,
                            0.0, 1.0, 0.0, 20.0,
                            0.0, 0.0, 1.0,  6.0,
                            0.0, 0.0, 0.0,  1.0);
  CHECK_MAT4(expected, mtx);
}

//----------------------------------------------------------- Translate (vector)
TEST(Matrix4_Translate_Vector)
{
  //Translate vector
  Vec3 translate(real(8.0), real(10.0), real(7.0));
  Mat4 mtx;
  mtx.Translate(translate);
  real expected[4][4] = MTX(1.0, 0.0, 0.0,  8.0,
                            0.0, 1.0, 0.0, 10.0,
                            0.0, 0.0, 1.0,  7.0,
                            0.0, 0.0, 0.0,  1.0);
  CHECK_MAT4(expected, mtx);
}

//------------------------------------------------- Build Transform (Quaternion)
TEST(Matrix4_BuildTransform_Quaternion)
{
  Vec3 translate(real(1.0), real(10.0), real(-2.8));
  Vec3 scale(real(8.0), real(-7.0), real(0.5));
  //Rotation about xyz by pi/2 radians
  Quat rotate(real(0.40824829046386301636621401245098),
              real(0.40824829046386301636621401245098),
              real(0.40824829046386301636621401245098),
              real(0.70710678118654752440084436210485));
  Normalize(rotate);
  Mat4 matrix;
  matrix.BuildTransform(translate, rotate, scale);

  real expected[4][4] = MTX(  2.66666666667,  1.70811855099, 
                             0.455351801261,            1.0,
                              7.28546882018,  -2.3333333333,
                            -0.122008467928,           10.0,
                             -1.95213548685, -6.37478521766,
                             0.166666666667,           -2.8,
                                        0.0,            0.0,
                                        0.0,            1.0);
  CHECK_MAT4_CLOSE(expected, matrix, real(0.00001));
}

//---------------------------------------------------- Build Transform (Matrix3)
TEST(Matrix4_BuildTransform_Matrix3)
{
  Vec3 translate(real(753.2), real(-85.2), real(1234.5));
  Vec3 scale(real(8.3), real(7.2), real(5.4));
  Mat3 rotation(real( 0.0), real(1.0), real(0.0),
                real(-1.0), real(0.0), real(0.0),
                real( 0.0), real(0.0), real(1.0));
  Mat4 matrix;
  matrix.BuildTransform(translate, rotation, scale);

  real expected[4][4] = MTX( 0.0, 7.2, 0.0,  753.2,
                            -8.3, 0.0, 0.0,  -85.2,
                             0.0, 0.0, 5.4, 1234.5,
                             0.0, 0.0, 0.0,    1.0);
  CHECK_MAT4(expected, matrix);
}

//------------------------------------------- Decompose (scale/rotate/translate)
TEST(Matrix4_Decompose_ScRoTr)
{
  Mat3 r;
  Vec3 t(real(61.0), real(-975.0), real(0.259));
  Vec3 s(real(7.9), real(0.64), real(27.0));
  Quat q(real(0.082165510187), real(0.082165510187),
         real(0.082165510187), real(0.989821441881));
  Normalize(q);
  ToMatrix3(q, &r);
//   Mat4 matrix(real( 0.220472), real(1.774692), real( 0.000000), real(8.500000),
//               real(-0.102781), real(0.591983), real(-0.174662), real(21.560268),
//               real(-0.091083), real(0.524603), real( 0.079796), real(1.000000),
//               real( 0.000000), real(0.000000), real( 0.000000), real(1.000000));
  Mat4 matrix = BuildTransform(t, r, s);

  Mat3 rotate;
  Vec3 translate;
  Vec3 scale;
  matrix.Decompose(&scale, &rotate, &translate);

  Vec3 expectedS(real(7.9), real(0.64), real(27.0));
  Vec3 expectedT(real(61.0), real(-975.0), real(0.259));
  real expectedR[3][3] = MTX3( 0.97299534, -0.14915602,  0.17616071,
                               0.17616071,  0.97299534, -0.14915602,
                              -0.14915602,  0.17616071,  0.97299534);
  CHECK_VEC3_CLOSE(expectedS, scale, real(0.000001));
  CHECK_VEC3(expectedT, translate);
  CHECK_MAT3_CLOSE(expectedR, rotate, real(0.000001));
}

//------------------------------------- Decompose (scale/shear/rotate/translate)
TEST(Matrix4_Decompose_ScShRoTr)
{
  //I don't really know how to test this...
  StubbedTest(Matrix4_Decompose_ScShRoTr);
}

//---------------------------------------------------------------------- GetBasis 1
TEST(Matrix4_GetBasis_1)
{
  Mat4 mtx(init4);
  Vec4 basis = mtx.GetBasis(0);
  Vec4 expected(mtx(0, 0), mtx(1, 0), mtx(2, 0), mtx(3, 0));
  CHECK_VEC4(expected, basis);
}

//---------------------------------------------------------------------- GetBasis 2
TEST(Matrix4_GetBasis_2)
{
  Mat4 mtx(init4);
  Vec4 basis = mtx.GetBasis(1);
  Vec4 expected(mtx(0, 1), mtx(1, 1), mtx(2, 1), mtx(3, 1));
  CHECK_VEC4(expected, basis);
}

//---------------------------------------------------------------------- GetBasis 3
TEST(Matrix4_GetBasis_3)
{
  Mat4 mtx(init4);
  Vec4 basis = mtx.GetBasis(2);
  Vec4 expected(mtx(0, 2), mtx(1, 2), mtx(2, 2), mtx(3, 2));
  CHECK_VEC4(expected, basis);
}

//---------------------------------------------------------------------- GetBasis 4
TEST(Matrix4_GetBasis_4)
{
  Mat4 mtx(init4);
  Vec4 basis = mtx.GetBasis(3);
  Vec4 expected(mtx(0, 3), mtx(1, 3), mtx(2, 3), mtx(3, 3));
  CHECK_VEC4(expected, basis);
}

//---------------------------------------------------------------------- Basis X
TEST(Matrix4_BasisX)
{
  Mat4 mtx(Mat4::cIdentity);
  Vec4 basisX = mtx.BasisX();
  Vec4 expected(real(1.0), real(0.0), real(0.0), real(0.0));
  CHECK_VEC4(expected, basisX);
}

//---------------------------------------------------------------------- Basis Y
TEST(Matrix4_BasisY)
{
  Mat4 mtx(Mat4::cIdentity);
  Vec4 basisY = mtx.BasisY();
  Vec4 expected(real(0.0), real(1.0), real(0.0), real(0.0));
  CHECK_VEC4(expected, basisY);
}

//---------------------------------------------------------------------- Basis Z
TEST(Matrix4_BasisZ)
{
  Mat4 mtx(Mat4::cIdentity);
  Vec4 basisZ = mtx.BasisZ();
  Vec4 expected(real(0.0), real(0.0), real(1.0), real(0.0));
  CHECK_VEC4(expected, basisZ);
}

//---------------------------------------------------------------------- Basis W
TEST(Matrix4_BasisW)
{
  Mat4 mtx(Mat4::cIdentity);
  Vec4 basisW = mtx.BasisW();
  Vec4 expected(real(0.0), real(0.0), real(0.0), real(1.0));
  CHECK_VEC4(expected, basisW);
}

//---------------------------------------------------------------------- GetCross 1
TEST(Matrix4_GetCross_1)
{
  Mat4 mtx(init4);
  Vec4 cross = mtx.GetCross(0);
  Vec4 expected(mtx(0, 0), mtx(0, 1), mtx(0, 2), mtx(0, 3));
  CHECK_VEC4(expected, cross);
}

//---------------------------------------------------------------------- GetCross 2
TEST(Matrix4_GetCross_2)
{
  Mat4 mtx(init4);
  Vec4 cross = mtx.GetCross(1);
  Vec4 expected(mtx(1, 0), mtx(1, 1), mtx(1, 2), mtx(1, 3));
  CHECK_VEC4(expected, cross);
}

//---------------------------------------------------------------------- GetCross 3
TEST(Matrix4_GetCross_3)
{
  Mat4 mtx(init4);
  Vec4 cross = mtx.GetCross(2);
  Vec4 expected(mtx(2, 0), mtx(2, 1), mtx(2, 2), mtx(2, 3));
  CHECK_VEC4(expected, cross);
}

//---------------------------------------------------------------------- GetCross 4
TEST(Matrix4_GetCross_4)
{
  Mat4 mtx(init4);
  Vec4 cross = mtx.GetCross(3);
  Vec4 expected(mtx(3, 0), mtx(3, 1), mtx(3, 2), mtx(3, 3));
  CHECK_VEC4(expected, cross);
}

//--------------------------------------------------------- Set Basis (vector) 1
TEST(Matrix4_SetBasisVector_1)
{
  Mat4 mtx(init4);
  Vec4 basis(real(8.0), real(7.2), real(99.0), real(-32.3));
  mtx.SetBasis(0, basis);
  basis = mtx.GetBasis(0);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, basis);
}

//--------------------------------------------------------- Set Basis (vector) 2
TEST(Matrix4_SetBasisVector_2)
{
  Mat4 mtx(init4);
  Vec4 basis(real(8.0), real(7.2), real(99.0), real(-32.3));
  mtx.SetBasis(1, basis);
  basis = mtx.GetBasis(1);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, basis);
}

//--------------------------------------------------------- Set Basis (vector) 3
TEST(Matrix4_SetBasisVector_3)
{
  Mat4 mtx(init4);
  Vec4 basis(real(8.0), real(7.2), real(99.0), real(-32.3));
  mtx.SetBasis(2, basis);
  basis = mtx.GetBasis(2);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, basis);
}

//--------------------------------------------------------- Set Basis (vector) 4
TEST(Matrix4_SetBasisVector_4)
{
  Mat4 mtx(init4);
  Vec4 basis(real(8.0), real(7.2), real(99.0), real(-32.3));
  mtx.SetBasis(3, basis);
  basis = mtx.GetBasis(3);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, basis);
}

//-------------------------------------------------------- Set Basis (vectorW) 1
TEST(Matrix4_SetBasisVectorW_1)
{
  Mat4 mtx(init4);
  Vec3 basis(real(8.0), real(7.2), real(99.0));
  mtx.SetBasis(0, basis, real(-32.3));
  Vec4 result = mtx.GetBasis(0);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, result);
}

//-------------------------------------------------------- Set Basis (vectorW) 2
TEST(Matrix4_SetBasisVectorW_2)
{
  Mat4 mtx(init4);
  Vec3 basis(real(8.0), real(7.2), real(99.0));
  mtx.SetBasis(1, basis, real(-32.3));
  Vec4 result = mtx.GetBasis(1);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, result);
}

//-------------------------------------------------------- Set Basis (vectorW) 3
TEST(Matrix4_SetBasisVectorW_3)
{
  Mat4 mtx(init4);
  Vec3 basis(real(8.0), real(7.2), real(99.0));
  mtx.SetBasis(2, basis, real(-32.3));
  Vec4 result = mtx.GetBasis(2);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, result);
}

//-------------------------------------------------------- Set Basis (vectorW) 4
TEST(Matrix4_SetBasisVectorW_4)
{
  Mat4 mtx(init4);
  Vec3 basis(real(8.0), real(7.2), real(99.0));
  mtx.SetBasis(3, basis, real(-32.3));
  Vec4 result = mtx.GetBasis(3);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, result);
}

//----------------------------------------------------------- Set Basis (xyzw) 1
TEST(Matrix4_SetBasisXYZW_1)
{
  Mat4 mtx(init4);
  mtx.SetBasis(0, real(8.0), real(7.2), real(99.0), real(-32.3));
  Vec4 basis = mtx.GetBasis(0);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, basis);
}

//----------------------------------------------------------- Set Basis (xyzw) 2
TEST(Matrix4_SetBasisXYZW_2)
{
  Mat4 mtx(init4);
  mtx.SetBasis(1, real(8.0), real(7.2), real(99.0), real(-32.3));
  Vec4 basis = mtx.GetBasis(1);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, basis);
}

//----------------------------------------------------------- Set Basis (xyzw) 3
TEST(Matrix4_SetBasisXYZW_3)
{
  Mat4 mtx(init4);
  mtx.SetBasis(2, real(8.0), real(7.2), real(99.0), real(-32.3));
  Vec4 basis = mtx.GetBasis(2);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, basis);
}

//----------------------------------------------------------- Set Basis (xyzw) 4
TEST(Matrix4_SetBasisXYZW_4)
{
  Mat4 mtx(init4);
  mtx.SetBasis(3, real(8.0), real(7.2), real(99.0), real(-32.3));
  Vec4 basis = mtx.GetBasis(3);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, basis);
}

//--------------------------------------------------------- Set Cross (vector) 1
TEST(Matrix4_SetCrossVector_1)
{
  Mat4 mtx(init4);
  Vec4 cross(real(8.0), real(7.2), real(99.0), real(-32.3));
  mtx.SetCross(0, cross);
  cross = mtx.GetCross(0);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, cross);
}

//--------------------------------------------------------- Set Cross (vector) 2
TEST(Matrix4_SetCrossVector_2)
{
  Mat4 mtx(init4);
  Vec4 cross(real(8.0), real(7.2), real(99.0), real(-32.3));
  mtx.SetCross(1, cross);
  cross = mtx.GetCross(1);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, cross);
}

//--------------------------------------------------------- Set Cross (vector) 3
TEST(Matrix4_SetCrossVector_3)
{
  Mat4 mtx(init4);
  Vec4 cross(real(8.0), real(7.2), real(99.0), real(-32.3));
  mtx.SetCross(2, cross);
  cross = mtx.GetCross(2);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, cross);
}

//--------------------------------------------------------- Set Cross (vector) 4
TEST(Matrix4_SetCrossVector_4)
{
  Mat4 mtx(init4);
  Vec4 cross(real(8.0), real(7.2), real(99.0), real(-32.3));
  mtx.SetCross(3, cross);
  cross = mtx.GetCross(3);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, cross);
}

//-------------------------------------------------------- Set Cross (vectorW) 1
TEST(Matrix4_SetCrossVectorW_1)
{
  Mat4 mtx(init4);
  Vec3 cross(real(8.0), real(7.2), real(99.0));
  mtx.SetCross(0, cross, real(-32.3));
  Vec4 result = mtx.GetCross(0);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, result);
}

//-------------------------------------------------------- Set Cross (vectorW) 2
TEST(Matrix4_SetCrossVectorW_2)
{
  Mat4 mtx(init4);
  Vec3 cross(real(8.0), real(7.2), real(99.0));
  mtx.SetCross(1, cross, real(-32.3));
  Vec4 result = mtx.GetCross(1);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, result);
}

//-------------------------------------------------------- Set Cross (vectorW) 3
TEST(Matrix4_SetCrossVectorW_3)
{
  Mat4 mtx(init4);
  Vec3 cross(real(8.0), real(7.2), real(99.0));
  mtx.SetCross(2, cross, real(-32.3));
  Vec4 result = mtx.GetCross(2);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, result);
}

//-------------------------------------------------------- Set Cross (vectorW) 4
TEST(Matrix4_SetCrossVectorW_4)
{
  Mat4 mtx(init4);
  Vec3 cross(real(8.0), real(7.2), real(99.0));
  mtx.SetCross(3, cross, real(-32.3));
  Vec4 result = mtx.GetCross(3);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, result);
}

//----------------------------------------------------------- Set Cross (xyzw) 1
TEST(Matrix4_SetCrossXYZ_1)
{
  Mat4 mtx(init4);
  mtx.SetCross(0, real(8.0), real(7.2), real(99.0), real(-32.3));
  Vec4 basis = mtx.GetCross(0);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, basis);
}

//----------------------------------------------------------- Set Cross (xyzw) 2
TEST(Matrix4_SetCrossXYZ_2)
{
  Mat4 mtx(init4);
  mtx.SetCross(1, real(8.0), real(7.2), real(99.0), real(-32.3));
  Vec4 basis = mtx.GetCross(1);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, basis);
}

//----------------------------------------------------------- Set Cross (xyzw) 3
TEST(Matrix4_SetCrossXYZ_3)
{
  Mat4 mtx(init4);
  mtx.SetCross(2, real(8.0), real(7.2), real(99.0), real(-32.3));
  Vec4 basis = mtx.GetCross(2);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, basis);
}

//----------------------------------------------------------- Set Cross (xyzw) 4
TEST(Matrix4_SetCrossXYZ_4)
{
  Mat4 mtx(init4);
  mtx.SetCross(3, real(8.0), real(7.2), real(99.0), real(-32.3));
  Vec4 basis = mtx.GetCross(3);
  Vec4 expected(real(8.0), real(7.2), real(99.0), real(-32.3));
  CHECK_VEC4(expected, basis);
}

//--------------------------------------------------------------------- GetBasis3 1
TEST(Matrix4_GetBasis3_1)
{
  Mat4 mtx(init4);
  Vec3 basis3 = mtx.GetBasis3(0);
  Vec3 expected(mtx(0, 0), mtx(1, 0), mtx(2, 0));
  CHECK_VEC3(expected, basis3);
}

//--------------------------------------------------------------------- GetBasis3 2
TEST(Matrix4_GetBasis3_2)
{
  Mat4 mtx(init4);
  Vec3 basis3 = mtx.GetBasis3(1);
  Vec3 expected(mtx(0, 1), mtx(1, 1), mtx(2, 1));
  CHECK_VEC3(expected, basis3);
}

//--------------------------------------------------------------------- GetBasis3 3
TEST(Matrix4_GetBasis3_3)
{
  Mat4 mtx(init4);
  Vec3 basis3 = mtx.GetBasis3(2);
  Vec3 expected(mtx(0, 2), mtx(1, 2), mtx(2, 2));
  CHECK_VEC3(expected, basis3);
}

//--------------------------------------------------------------------- GetBasis3 4
TEST(Matrix4_GetBasis3_4)
{
  Mat4 mtx(init4);
  Vec3 basis3 = mtx.GetBasis3(3);
  Vec3 expected(mtx(0, 3), mtx(1, 3), mtx(2, 3));
  CHECK_VEC3(expected, basis3);
}

//------------------------------------------------- Global Scalar Multiplication
TEST(Matrix4_Global_ScalarMultiplication)
{
  //Now swap the order
  Mat4 mtx(init4);
  Mat4 scaled = real(5.0) * mtx;

  real expected[4][4];
  for(int i = 0; i < 4; ++i)
  {
    for(int j = 0; j < 4; ++j)
    {
      expected[i][j] = init44[i][j] * real(5.0);
    }
  }
  CHECK_MAT4(expected, scaled);
}

//---------------------------------------------------------------- Global Multiply
TEST(Matrix4_Global_Multiply)
{
  Mat4 mtx(init4);
  Mat4 mul(real( 2.0), real( 4.0), real( 6.0), real( 8.0),
           real(10.0), real(12.0), real(14.0), real(16.0),
           real(18.0), real(20.0), real(22.0), real(24.0),
           real(26.0), real(28.0), real(30.0), real(32.0));

  //Concat stand alone function
  Mat4 result = Multiply(mul, mtx);

  real expected[4][4] = MTX( 514.0,  576.0,  640.0,  736.0, 
                            1130.0, 1280.0, 1440.0, 1664.0,
                            1746.0, 1984.0, 2240.0, 2592.0,
                            2362.0, 2688.0, 3040.0, 3520.0);
  CHECK_MAT4(expected, result);
}

//------------------------------------------ Global Build Transform (Quaternion)
TEST(Matrix4_Global_BuildTransform_Quaternion)
{
  Vec3 translate(real(61.0), real(-975.0), real(0.259));
  Vec3 scale(real(-7.9), real(0.64), real(27.0));
  Quat rotation(real(0.082165510187), real(0.082165510187),
                real(0.082165510187), real(0.989821441881));
  Normalize(rotation);
  Mat4 matrix = BuildTransform(translate, rotation, scale);

  real expected[4][4] = MTX(-7.68666299437, -0.095459856258,
                             4.75633916083,            61.0,
                            -1.39166960632,  0.622717002076,
                            -4.02721268589,          -975.0,
                             1.17833260069,  0.112742854183,
                             26.2708735251,           0.259,
                                       0.0,             0.0,
                                       0.0,             1.0);
  CHECK_MAT4_CLOSE(expected, matrix, real(0.00001));
}

//--------------------------------------------- Global Build Transform (Matrix3)
TEST(Matrix4_Global_BuildTransform3D_Matrix3)
{
  Vec3 translate(real(872.3), real(572.11), real(867.5));
  Vec3 scale(real(2.3), real(8.3), real(1.4));
  Mat3 rotation(real(0.0), real(-1.0), real( 0.0),
                real(0.0), real( 0.0), real(-1.0),
                real(1.0), real( 0.0), real( 0.0));
  Mat4 matrix = BuildTransform(translate, rotation, scale);
  real expected[4][4] = MTX(0.0, -8.3,  0.0,  872.3,
                            0.0,  0.0, -1.4, 572.11,
                            2.3,  0.0,  0.0,  867.5,
                            0.0,  0.0,  0.0,    1.0);
  CHECK_MAT4(expected, matrix);
}



//------------------------------------------------- Global Transform (Read Only)
TEST(Matrix4_Global_Transform_ReadOnly)
{
  Vec3 translate(real(1.0), real(10.0), real(-2.8));
  Vec3 scale(real(8.0), real(-7.0), real(0.5));
  //Rotation about xyz by pi/2 radians
  Quat rotate(real(0.40824829046386301636621401245098),
              real(0.40824829046386301636621401245098),
              real(0.40824829046386301636621401245098),
              real(0.70710678118654752440084436210485));
  Normalize(rotate);
  Mat4 matrix = BuildTransform(translate, rotate, scale);
  Vec4 foo(real(2.0), real(8.2), real(9.3), real(1.0));
  Vec4 result = Transform(matrix, foo);
  Vec4 expected(real(24.5746), real(4.30293), real(-57.4275), real(1.0));
  CHECK_VEC4_CLOSE(expected, result, real(0.0001));
}

//------------------------------------------------ Global Transform (Read/Write)
TEST(Matrix4_Global_Transform_ReadWrite)
{
  Vec3 translate(real(1.0), real(10.0), real(-2.8));
  Vec3 scale(real(8.0), real(-7.0), real(0.5));
  //Rotation about xyz by pi/2 radians
  Quat rotate(real(0.40824829046386301636621401245098),
              real(0.40824829046386301636621401245098),
              real(0.40824829046386301636621401245098),
              real(0.70710678118654752440084436210485));
  Normalize(rotate);
  Mat4 matrix = BuildTransform(translate, rotate, scale);
  Vec4 foo(real(2.0), real(8.2), real(9.3), real(1.0));
  Transform(matrix, &foo);
  Vec4 expected(real(24.5746), real(4.30293), real(-57.4275), real(1.0));
  CHECK_VEC4_CLOSE(expected, foo, real(0.0001));
}

//------------------------------------------------------- Global Transform Point
TEST(Matrix4_Global_TransformPoint)
{
  StubbedTest(Matrix4_Global_TransformPoint);
}

//------------------------------------------------------ Global Transform Normal
TEST(Matrix4_Global_TransformNormal)
{
  StubbedTest(Matrix4_Global_TransformNormal);
}

//--------------------------------------------- Global Transform Point Projected
TEST(Matrix4_Global_TransfromPointProjected)
{
  StubbedTest(Matrix4_Global_TransfromPointProjected);
}

//----------------------------------------------- Global Transform Normal Column
TEST(Matrix4_Global_TransformNormalColumn)
{
  StubbedTest(Matrix4_Global_TransformNormalColumn);
}

//------------------------------------------------ Global Transform Point Column
TEST(Matrix4_Global_TransformPointColumn)
{
  StubbedTest(Matrix4_Global_TransformPointColumn);
}

//-------------------------------------- Global Transform Point Projected Column
TEST(Matrix4_Global_TransformPointProjectedColumn)
{
  StubbedTest(Matrix4_Global_TransformPointProjectedColumn);
}

#undef MTX
