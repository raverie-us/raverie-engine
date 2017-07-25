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
const real init33[3][3] = { { real( 2.0), real( 3.0), real( 5.0) },
                            { real( 7.0), real(11.0), real(13.0) },
                            { real(17.0), real(19.0), real(23.0) }};

}

#define MTX(a,b,c,d,e,f,g,h,i)          \
{ { real((a)), real((b)), real((c)) },  \
  { real((d)), real((e)), real((f)) },  \
  { real((g)), real((h)), real((i)) } }

//------------------------------------------------------- Identity (Static Data)
TEST(Matrix3_Identity_StaticData)
{
  Mat3 identity = Mat3::cIdentity;
  real expected[3][3] = MTX(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
  CHECK_MAT3(expected, identity);
}

//------------------------------------------------------------------ Constructor
TEST(Matrix3_Constructor)
{
  //Normal constructor
  Mat3 mtx(real(1.0), real(2.0), real(3.0),
           real(4.0), real(5.0), real(6.0),
           real(7.0), real(8.0), real(9.0));
  real expected[3][3] = MTX(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
  CHECK_MAT3(expected, mtx);
}

//------------------------------------------------------------- Copy Constructor
TEST(Matrix3_CopyConstructor)
{
  //Copy constructor
  Mat3 mtx(real(1.0), real(2.0), real(3.0),
           real(4.0), real(5.0), real(6.0),
           real(7.0), real(8.0), real(9.0));
  Mat3 copyMtx(mtx);
  real expected[3][3] = MTX(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
  CHECK_MAT3(expected, copyMtx);
}

//------------------------------------------------- Explicit Pointer Constructor
TEST(Matrix3_ExplicitPointerConstructor)
{
  //Data constructor
  Mat3 dataMtx(init3);
  CHECK_MAT3(init33, dataMtx);
}

//----------------------------------------------------------- Initial Assignment
TEST(Matrix3_InitialAssignment)
{
  //Assignment operator
  Mat3 mtx(real(1.0), real(2.0), real(3.0),
           real(4.0), real(5.0), real(6.0),
           real(7.0), real(8.0), real(9.0));
  Mat3 equMtx = mtx;
  real expected[3][3] = MTX(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
  CHECK_MAT3(expected, equMtx);
}

//------------------------------------------- Scalar Multiplication Assignment 1
TEST(Matrix3_ScalarMultiplicationAssignment_1)
{
  //Multiplication assignment
  Mat3 mtx(init3);
  mtx *= real(2.0);

  real expected[3][3];
  for(int i = 0; i < 3; ++i)
  {
    for(int j = 0; j < 3; ++j)
    {
      expected[i][j] = init33[i][j] * real(2.0);
    }
  }

  CHECK_MAT3(expected, mtx);
}

//------------------------------------------- Scalar Multiplication Assignment 2
TEST(Matrix3_ScalarMultiplicationAssignment_2)
{
  //Multiplication assignment
  Mat3 mtx(init3);
  mtx *= real(0.0);

  real expected[3][3];
  for(int i = 0; i < 3; ++i)
  {
    for(int j = 0; j < 3; ++j)
    {
      expected[i][j] = real(0.0);
    }
  }

  CHECK_MAT3(expected, mtx);
}

//--------------------------------------------------- Scalar Division Assignment
TEST(Matrix3_ScalarDivisionAssignment)
{
  //Division assignment
  Mat3 mtx(init3);
  mtx /= real(10.0);
  //This looks a little funky but it's to mimic what's happening to the matrix.
  //If you just multiply by 0.2 then you get slightly different values.
  real expected[3][3];
  for(int i = 0; i < 3; ++i)
  {
    for(int j = 0; j < 3; ++j)
    {
      expected[i][j] = init33[i][j] / real(10.0);
    }
  }

  CHECK_MAT3_CLOSE(expected, mtx, 0.001f);
}

//-------------------------------------------------------- Scalar Multiplication
TEST(Matrix3_ScalarMultiplication)
{
  Mat3 mtx(init3);

  //Multiplication with scalar
  Mat3 scaled = mtx * real(2.0);

  real expected[3][3];
  for(int i = 0; i < 3; ++i)
  {
    for(int j = 0; j < 3; ++j)
    {
      expected[i][j] = init33[i][j] * real(2.0);
    }
  }

  CHECK_MAT3(expected, scaled);
}

//-------------------------------------------------------------- Scalar Division
TEST(Matrix3_ScalarDivision)
{
  //Division by scalar
  Mat3 mtx(init3);
  Mat3 scaled = mtx;
  scaled = mtx / real(10.0);

  real expected[3][3];
  for(int i = 0; i < 3; ++i)
  {
    for(int j = 0; j < 3; ++j)
    {
      expected[i][j] = init33[i][j] / real(10.0);
    }
  }
  
  CHECK_MAT3_CLOSE(expected, scaled, real(0.000001));
}

//--------------------------------------------------- Matrix Addition Assignment
TEST(Matrix3_MatrixAdditionAssignment)
{
  Mat3 mtx(real(0.0), real(1.0), real(2.0),
           real(3.0), real(4.0), real(5.0),
           real(6.0), real(7.0), real(8.0));

  //Addition assignment
  Mat3 add(real( 2.0), real( 4.0), real( 6.0),
           real( 8.0), real(10.0), real(12.0),
           real(14.0), real(16.0), real(18.0));
  mtx += add;

  real expected[3][3] = MTX(2.0, 5.0, 8.0, 11.0, 14.0, 17.0, 20.0, 23.0, 26.0);
  CHECK_MAT3(expected, mtx);
}

//------------------------------------------------ Matrix Subtraction Assignment
TEST(Matrix3_MatrixSubtractionAssignment)
{
  //Subtraction assignment
  Mat3 mtx(real( 2.0), real( 5.0), real( 8.0), 
           real(11.0), real(14.0), real(17.0), 
           real(20.0), real(23.0), real(26.0));
  Mat3 add(real( 2.0), real( 4.0), real( 6.0),
           real( 8.0), real(10.0), real(12.0),
           real(14.0), real(16.0), real(18.0));
  add -= mtx;

  real expected[3][3] = MTX( 0.0, -1.0, -2.0, 
                            -3.0, -4.0, -5.0, 
                            -6.0, -7.0, -8.0);
  CHECK_MAT3(expected, add);
}

//-------------------------------------------------------------- Matrix Addition
TEST(Matrix3_MatrixAddition)
{
  Mat3 mtx(real(0.0), real(1.0), real(2.0),
           real(3.0), real(4.0), real(5.0),
           real(6.0), real(7.0), real(8.0));
  Mat3 add(real( 2.0), real( 4.0), real( 6.0),
           real( 8.0), real(10.0), real(12.0),
           real(14.0), real(16.0), real(18.0));

  //Addition
  Mat3 res = mtx + add;

  real expected[3][3] = MTX(2.0, 5.0, 8.0, 11.0, 14.0, 17.0, 20.0, 23.0, 26.0);
  CHECK_MAT3(expected, res);
}

//----------------------------------------------------------- Matrix Subtraction
TEST(Matrix3_MatrixSubtraction)
{
  Mat3 mtx(real(0.0), real(1.0), real(2.0),
           real(3.0), real(4.0), real(5.0),
           real(6.0), real(7.0), real(8.0));
  Mat3 add(real( 2.0), real( 4.0), real( 6.0),
           real( 8.0), real(10.0), real(12.0),
           real(14.0), real(16.0), real(18.0));

  //Subtraction
  Mat3 res = add - mtx;

  real expected[3][3] = MTX(2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0);

  CHECK_MAT3(expected, res);
}

//------------------------------------------------------ Matrix Multiplication 1
TEST(Matrix3_MatrixMultiplication_1)
{
  Mat3 mtx(real(0.0), real(1.0), real(2.0),
           real(3.0), real(4.0), real(5.0),
           real(6.0), real(7.0), real(8.0));
  Mat3 add(real( 2.0), real( 4.0), real( 6.0),
           real( 8.0), real(10.0), real(12.0),
           real(14.0), real(16.0), real(18.0));

  //Multiplication
  Mat3 res = add * mtx;

  real expected[3][3] = MTX( 48.0,  60.0,  72.0,
                            102.0, 132.0, 162.0,
                            156.0, 204.0, 252.0);
  CHECK_MAT3(expected, res);
}

//------------------------------------------------------ Matrix Multiplication 2
TEST(Matrix3_MatrixMultiplication_2)
{
  Mat3 mtx(real(0.0), real(1.0), real(2.0),
           real(3.0), real(4.0), real(5.0),
           real(6.0), real(7.0), real(8.0));
  Mat3 add(real( 2.0), real( 4.0), real( 6.0),
           real( 8.0), real(10.0), real(12.0),
           real(14.0), real(16.0), real(18.0));

  Mat3 res = mtx * add;

  real expected[3][3] = MTX( 36.0,  42.0,  48.0,
                            108.0, 132.0, 156.0,
                            180.0, 222.0, 264.0);
  CHECK_MAT3(expected, res);
}

//------------------------------------------------------------- Equal Comparison
TEST(Matrix3_EqualComparison)
{
  Mat3 one(init3);
  Mat3 two(init3);

  //Equality
  CHECK(one == two);
}

//--------------------------------------------------------- Not Equal Comparison
TEST(Matrix3_NotEqualComparison)
{
  Mat3 one(init3);
  Mat3 two(init3);

  //Inequality
  two.m00 = real(50.0);
  CHECK(one != two);
}

//----------------------------------------------------- "Subscript" (Read/Write)
TEST(Matrix3_Subscript_ReadWrite)
{
  //Modify
  Mat3 mtx(init3);
  for(int i = 0; i < 3; ++i)
  {
    for(int j = 0; j < 3; ++j)
    {
      real foo = mtx(i, j);
      foo *= real(2.0);
      mtx(i, j) = foo;
    }
  }

  real expected[3][3];
  for(int i = 0; i < 3; ++i)
  {
    for(int j = 0; j < 3; ++j)
    {
      expected[i][j] = init33[i][j] * real(2.0);
    }
  }

  CHECK_MAT3(expected, mtx);
}

//------------------------------------------------------ "Subscript" (Read Only)
TEST(Matrix3_Subscript_ReadOnly)
{
  //Non-modify
  Mat3 mtx(init3);
  real values[3][3];

  for(int i = 0; i < 3; ++i)
  {
    for(int j = 0; j < 3; ++j)
    {
      values[i][j] = mtx(i, j);
    }
  }

  CHECK_MAT3(values, mtx);
}

//------------------------------------------------------------------- Transposed
TEST(Matrix3_Transposed)
{
  Mat3 mtx(real(0.0), real(1.0), real(2.0),
           real(3.0), real(4.0), real(5.0),
           real(6.0), real(7.0), real(8.0));

  //Transpose
  Mat3 transpose = mtx.Transposed();

  real expected[3][3];
  for(int i = 0; i < 3; ++i)
  {
    for(int j = 0; j < 3; ++j)
    {
      expected[i][j] = mtx(j, i);
    }
  }

  CHECK_MAT3(expected, transpose);
}

//-------------------------------------------------------------------- Transpose
TEST(Matrix3_Transpose)
{
  //Transpose this
  Mat3 mtx(real(0.0), real(1.0), real(2.0),
           real(3.0), real(4.0), real(5.0),
           real(6.0), real(7.0), real(8.0));
  real expected[3][3];
  for(int i = 0; i < 3; ++i)
  {
    for(int j = 0; j < 3; ++j)
    {
      expected[i][j] = mtx(j, i);
    }
  }
  mtx.Transpose();
  CHECK_MAT3(expected, mtx);
}

//--------------------------------------------------------------------- Inverted
TEST(Matrix3_Inverted)
{
  Mat3 mtx(init3);

  //Inverse
  Mat3 inv = mtx.Inverted();
  real expected[3][3] = MTX( -1.0 / 13.0, -1.0 / 3.0,  8.0 / 39.0,
                            -10.0 / 13.0,  1.0 / 2.0, -3.0 / 26.0,
                              9.0 / 13.0, -1.0 / 6.0, -1.0 / 78.0);
  CHECK_MAT3(expected, inv);
}

//----------------------------------------------------------------------- Invert
TEST(Matrix3_Invert)
{
  Mat3 mtx(init3);

  //Inverse this
  mtx.Invert();

  real expected[3][3] = MTX( -1.0 / 13.0, -1.0 / 3.0,  8.0 / 39.0,
                            -10.0 / 13.0,  1.0 / 2.0, -3.0 / 26.0,
                              9.0 / 13.0, -1.0 / 6.0, -1.0 / 78.0);
  CHECK_MAT3(expected, mtx);
}

//----------------------------------------------------------------------- Multiply
TEST(Matrix3_Multiply)
{
  Mat3 mtx(init3);
  Mat3 mul(real( 2.0), real( 4.0), real( 6.0),
           real( 8.0), real(10.0), real(12.0),
           real(14.0), real(16.0), real(18.0));

  //Concat is multiply
  Mat3 result = Mat3::Multiply(mtx, mul);

  real expected[3][3] = MTX( 98.0, 118.0, 138.0,
                            284.0, 346.0, 408.0,
                            508.0, 626.0, 744.0);
  CHECK_MAT3(expected, result);
}

//--------------------------------------------------------------- Set Identity 1
TEST(Matrix3_SetIdentity_1)
{
  Mat3 mtx(init3);

  //Identity test
  mtx.SetIdentity();

  real expected[3][3] = MTX(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
  CHECK_MAT3(expected, mtx);
}

//--------------------------------------------------------------- Set Identity 2
TEST(Matrix3_SetIdentity_2)
{
  Mat3 mtx(init3);

  //Identity test
  Mat3 iden = mtx.SetIdentity();

  real expected[3][3] = MTX(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
  CHECK_MAT3(expected, iden);
}

//------------------------------------------------------------------- Zero Out 1
TEST(Matrix3_ZeroOut_1)
{
  Mat3 mtx(init3);

  //Zero test
  mtx.ZeroOut();

  real expected[3][3] = MTX(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  CHECK_MAT3(expected, mtx);
}

//------------------------------------------------------------------- Zero Out 2
TEST(Matrix3_ZeroOut_2)
{
  Mat3 mtx(init3);

  //Zero test
  Mat3 zero = mtx.ZeroOut();

  real expected[3][3] = MTX(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  CHECK_MAT3(expected, zero);
}

//------------------------------------------------------------------ Determinant
TEST(Matrix3_Determinant)
{
  Mat3 mtx(init3);

  //Determinant
  real det = mtx.Determinant();
  CHECK_EQUAL(real(-78.0), det);
}

//---------------------------------------------------------------------- Valid 1
TEST(Matrix3_Valid_1)
{
  Mat3 mtx(init3);
  CHECK(mtx.Valid());
}

//---------------------------------------------------------------------- Valid 2
TEST(Matrix3_Valid_2)
{
  Mat3 mtx(init3);
  real zero = real(0.0);
  mtx.m00 /= zero;
  CHECK(mtx.Valid() == false);
}

//------------------------------------------------------------------ Scale (xyz)
TEST(Matrix3_Scale_XYZ)
{
  Mat3 mtx;
  //Scale xyz
  mtx.Scale(real(5.0), real(8.0), real(2.0));

  real expected[3][3] = MTX(5.0, 0.0, 0.0, 0.0, 8.0, 0.0, 0.0, 0.0, 2.0);
  CHECK_MAT3(expected, mtx);
}

//--------------------------------------------------------------- Scale (vector)
TEST(Matrix3_Scale_Vector)
{
  Mat3 mtx;

  //Scale vector
  Vec3 scale(real(3.0), real(6.0), real(9.0));
  mtx.Scale(scale);

  real expected[3][3] = MTX(3.0, 0.0, 0.0, 0.0, 6.0, 0.0, 0.0, 0.0, 9.0);
  CHECK_MAT3(expected, mtx);
}

//----------------------------------------------------------------- Rotate (xyz)
TEST(Matrix3_Rotate_XYZ)
{
  Mat3 mtx;

  //Rotate xyz radian
  mtx.Rotate(real(0.57735026918962576450914878050195),
             real(0.57735026918962576450914878050195),
             real(0.57735026918962576450914878050195),
             Math::cPi);
  real expected[3][3] = MTX(-0.33333333333333333333333333333335,
                             0.66666666666666666666666666666665,
                             0.66666666666666666666666666666665,
                             0.66666666666666666666666666666665,
                            -0.33333333333333333333333333333335,
                             0.66666666666666666666666666666665,
                             0.66666666666666666666666666666665,
                             0.66666666666666666666666666666665,
                            -0.33333333333333333333333333333335);
  CHECK_MAT3_CLOSE(expected, mtx, real(0.000001));
}

//-------------------------------------------------------------- Rotate (vector)
TEST(Matrix3_Rotate_Vector)
{
  //Rotate vector radian
  Vec3 axis(real(1.0), real(1.0), real(0.0));
  Normalize(axis);
  Mat3 mtx;
  mtx.Rotate(axis, real(0.25) * Math::cPi);

  real expected[3][3] = MTX( 0.85355339059327376220042218105242,
                             0.14644660940672623779957781894757,
                             0.5,
                             0.14644660940672623779957781894757,
                             0.85355339059327376220042218105242,
                            -0.5,
                            -0.5,
                             0.5,
                             0.70710678118654752440084436210485);
  CHECK_MAT3_CLOSE(expected, mtx, real(0.000001));
}

//--------------------------------------------------------------- Translate (xy)
TEST(Matrix3_Translate_XY)
{
  Mat3 mtx;

  //Translate xy
  mtx.Translate(real(5.0), real(20.0));

  real expected[3][3] = MTX(1.0, 0.0, 5.0, 0.0, 1.0, 20.0, 0.0, 0.0, 1.0);
  CHECK_MAT3(expected, mtx);
}

//----------------------------------------------------------- Translate (vector)
TEST(Matrix3_Translate_Vector)
{
  Mat3 mtx;

  //Translate vector
  Vec2 translate(real(8.0), real(10.0));
  mtx.Translate(translate);

  real expected[3][3] = MTX(1.0, 0.0, 8.0, 0.0, 1.0, 10.0, 0.0, 0.0, 1.0);
  CHECK_MAT3(expected, mtx);
}

//--------------------------------------------------------- Build Transform (2D)
TEST(Matrix3_BuildTransform2D)
{
  Vec2 translate(real(1.0), real(10.0));
  Vec2 scale(real(8.0), real(-7.0));

  Mat3 matrix;
  matrix.BuildTransform(translate, Math::cPi / real(2.0), scale);

  real expected[3][3] = MTX(0.0, 7.0, 1.0, 8.0, 0.0, 10.0, 0.0, 0.0, 1.0);
  CHECK_MAT3_CLOSE(expected, matrix, real(0.00001));
}

//--------------------------------------------------------- Build Transform (3D)
TEST(Matrix3_BuildTransform3D)
{
  Vec3 scale(real(-7.9), real(0.64), real(27.0));
  Quat rotation(real(0.082165510187), real(0.082165510187),
                real(0.082165510187), real(0.989821441881));
  Normalize(rotation);
  Mat3 matrix;
  matrix.BuildTransform(rotation, scale);

  real expected[3][3] = MTX(-7.68666299437, -0.095459856258,  4.75633916083,
                            -1.39166960632,  0.622717002076, -4.02721268589,
                             1.17833260069,  0.112742854183,  26.2708735251);
  CHECK_MAT3_CLOSE(expected, matrix, real(0.00001));
}

//--------------------------------------------------------------- Orthonormalize
TEST(Matrix3_Orthonormalize)
{
  Mat3 mtx;

  //Orthonormalize the matrix
  mtx.Rotate(real(1.0), real(0.0), real(0.0), Math::cPi);
  mtx.m00 += real(0.01);

  mtx.m01  = real(0.01);
  mtx.m11 -= real(0.02);
  mtx.m21  = real(0.02);

  mtx.m02  = real(-0.01);
  mtx.m12  = real(-0.03);
  mtx.m22 += real( 0.03);

  mtx.Orthonormalize();

  real expected[3][3] = MTX(1.0, 0.0, 0.0, 0.0, 
                            -0.99980782165672450923513113130942,
                            -0.01960190163103812389462909872548,
                            0.0, 0.0196040749344455786124535515943,
                            -0.99980786426815382643673019934809);
  CHECK_MAT3_CLOSE(expected, mtx, real(0.00001));
}

//---------------------------------------------------------------------- GetBasis 1
TEST(Matrix3_GetBasis_1)
{
  Mat3 mtx(init3);
  Vec3 basis = mtx.GetBasis(0);
  Vec3 expected(mtx(0, 0), mtx(1, 0), mtx(2, 0));
  CHECK_VEC3(expected, basis);
}

//---------------------------------------------------------------------- GetBasis 2
TEST(Matrix3_GetBasis_2)
{
  Mat3 mtx(init3);
  Vec3 basis = mtx.GetBasis(1);
  Vec3 expected(mtx(0, 1), mtx(1, 1), mtx(2, 1));
  CHECK_VEC3(expected, basis);
}

//---------------------------------------------------------------------- GetBasis 3
TEST(Matrix3_GetBasis_3)
{
  Mat3 mtx(init3);
  Vec3 basis = mtx.GetBasis(2);
  Vec3 expected(mtx(0, 2), mtx(1, 2), mtx(2, 2));
  CHECK_VEC3(expected, basis);
}

//---------------------------------------------------------------------- Basis X
TEST(Matrix3_BasisX)
{
  Mat3 mtx(Mat3::cIdentity);
  Vec3 basisX = mtx.BasisX();
  Vec3 expected(real(1.0), real(0.0), real(0.0));
  CHECK_VEC3(expected, basisX);
}

//---------------------------------------------------------------------- Basis Y
TEST(Matrix3_BasisY)
{
  Mat3 mtx(Mat3::cIdentity);
  Vec3 basisY = mtx.BasisY();
  Vec3 expected(real(0.0), real(1.0), real(0.0));
  CHECK_VEC3(expected, basisY);
}

//---------------------------------------------------------------------- Basis Z
TEST(Matrix3_BasisZ)
{
  Mat3 mtx(Mat3::cIdentity);
  Vec3 basisZ = mtx.BasisZ();
  Vec3 expected(real(0.0), real(0.0), real(1.0));
  CHECK_VEC3(expected, basisZ);
}

//---------------------------------------------------------------------- GetCross 1
TEST(Matrix3_GetCross_1)
{
  Mat3 mtx(init3);
  Vec3 cross = mtx.GetCross(0);
  Vec3 expected(mtx(0, 0), mtx(0, 1), mtx(0, 2));
  CHECK_VEC3(expected, cross);
}

//---------------------------------------------------------------------- GetCross 2
TEST(Matrix3_GetCross_2)
{
  Mat3 mtx(init3);
  Vec3 cross = mtx.GetCross(1);
  Vec3 expected(mtx(1, 0), mtx(1, 1), mtx(1, 2));
  CHECK_VEC3(expected, cross);
}

//---------------------------------------------------------------------- GetCross 3
TEST(Matrix3_GetCross_3)
{
  Mat3 mtx(init3);
  Vec3 cross = mtx.GetCross(2);
  Vec3 expected(mtx(2, 0), mtx(2, 1), mtx(2, 2));
  CHECK_VEC3(expected, cross);
}

//--------------------------------------------------------- Set Basis (vector) 1
TEST(Matrix3_SetBasisVector_1)
{
  Mat3 mtx(init3);
  Vec3 basis(real(8.0), real(7.2), real(99.0));
  mtx.SetBasis(0, basis);
  basis = mtx.GetBasis(0);
  Vec3 expected(real(8.0), real(7.2), real(99.0));
  CHECK_VEC3(expected, basis);
}

//--------------------------------------------------------- Set Basis (vector) 2
TEST(Matrix3_SetBasisVector_2)
{
  Mat3 mtx(init3);
  Vec3 basis(real(8.0), real(7.2), real(99.0));
  mtx.SetBasis(1, basis);
  basis = mtx.GetBasis(1);
  Vec3 expected(real(8.0), real(7.2), real(99.0));
  CHECK_VEC3(expected, basis);
}

//--------------------------------------------------------- Set Basis (vector) 3
TEST(Matrix3_SetBasisVector_3)
{
  Mat3 mtx(init3);
  Vec3 basis(real(8.0), real(7.2), real(99.0));
  mtx.SetBasis(2, basis);
  basis = mtx.GetBasis(2);
  Vec3 expected(real(8.0), real(7.2), real(99.0));
  CHECK_VEC3(expected, basis);
}

//------------------------------------------------------------ Set Basis (xyz) 1
TEST(Matrix3_SetBasisXYZ_1)
{
  Mat3 mtx(init3);
  mtx.SetBasis(0, real(8.0), real(7.2), real(99.0));
  Vec3 basis = mtx.GetBasis(0);
  Vec3 expected(real(8.0), real(7.2), real(99.0));
  CHECK_VEC3(expected, basis);
}

//------------------------------------------------------------ Set Basis (xyz) 2
TEST(Matrix3_SetBasisXYZ_2)
{
  Mat3 mtx(init3);
  mtx.SetBasis(1, real(8.0), real(7.2), real(99.0));
  Vec3 basis = mtx.GetBasis(1);
  Vec3 expected(real(8.0), real(7.2), real(99.0));
  CHECK_VEC3(expected, basis);
}

//------------------------------------------------------------ Set Basis (xyz) 3
TEST(Matrix3_SetBasisXYZ_3)
{
  Mat3 mtx(init3);
  mtx.SetBasis(2, real(8.0), real(7.2), real(99.0));
  Vec3 basis = mtx.GetBasis(2);
  Vec3 expected(real(8.0), real(7.2), real(99.0));
  CHECK_VEC3(expected, basis);
}

//--------------------------------------------------------- Set Cross (vector) 1
TEST(Matrix3_SetCrossVector_1)
{
  Mat3 mtx(init3);
  Vec3 cross(real(8.0), real(7.2), real(99.0));
  mtx.SetCross(0, cross);
  cross = mtx.GetCross(0);
  Vec3 expected(real(8.0), real(7.2), real(99.0));
  CHECK_VEC3(expected, cross);
}

//--------------------------------------------------------- Set Cross (vector) 2
TEST(Matrix3_SetCrossVector_2)
{
  Mat3 mtx(init3);
  Vec3 cross(real(8.0), real(7.2), real(99.0));
  mtx.SetCross(1, cross);
  cross = mtx.GetCross(1);
  Vec3 expected(real(8.0), real(7.2), real(99.0));
  CHECK_VEC3(expected, cross);
}

//--------------------------------------------------------- Set Cross (vector) 3
TEST(Matrix3_SetCrossVector_3)
{
  Mat3 mtx(init3);
  Vec3 cross(real(8.0), real(7.2), real(99.0));
  mtx.SetCross(2, cross);
  cross = mtx.GetCross(2);
  Vec3 expected(real(8.0), real(7.2), real(99.0));
  CHECK_VEC3(expected, cross);
}

//------------------------------------------------------------ Set Cross (xyz) 1
TEST(Matrix3_SetCrossXYZ_1)
{
  Mat3 mtx(init3);
  mtx.SetCross(0, real(8.0), real(7.2), real(99.0));
  Vec3 cross = mtx.GetCross(0);
  Vec3 expected(real(8.0), real(7.2), real(99.0));
  CHECK_VEC3(expected, cross);
}

//------------------------------------------------------------ Set Cross (xyz) 2
TEST(Matrix3_SetCrossXYZ_2)
{
  Mat3 mtx(init3);
  mtx.SetCross(1, real(8.0), real(7.2), real(99.0));
  Vec3 cross = mtx.GetCross(1);
  Vec3 expected(real(8.0), real(7.2), real(99.0));
  CHECK_VEC3(expected, cross);
}

//------------------------------------------------------------ Set Cross (xyz) 3
TEST(Matrix3_SetCrossXYZ_3)
{
  Mat3 mtx(init3);
  mtx.SetCross(2, real(8.0), real(7.2), real(99.0));
  Vec3 cross = mtx.GetCross(2);
  Vec3 expected(real(8.0), real(7.2), real(99.0));
  CHECK_VEC3(expected, cross);
}

//------------------------------------------------- Global Scalar Multiplication
TEST(Matrix3_Global_ScalarMultiplication)
{
  //Now swap the order
  Mat3 mtx(init3);
  Mat3 scaled = mtx;
  scaled = real(5.0) * mtx;

  real expected[3][3];
  for(int i = 0; i < 3; ++i)
  {
    for(int j = 0; j < 3; ++j)
    {
      expected[i][j] = init33[i][j] * real(5.0);
    }
  }

  CHECK_MAT3(expected, scaled);
}

//---------------------------------------------------------------- Global Multiply
TEST(Matrix3_Global_Multiply)
{
  Mat3 mtx(init3);
  Mat3 mul(real( 2.0), real( 4.0), real( 6.0),
           real( 8.0), real(10.0), real(12.0),
           real(14.0), real(16.0), real(18.0));
  //Concat stand alone function
  Mat3 result = Multiply(mul, mtx);

  real expected[3][3] = MTX(134.0, 164.0, 200.0,
                            290.0, 362.0, 446.0,
                            446.0, 560.0, 692.0);
  CHECK_MAT3(expected, result);
}
//-------------------------------------------------- Global Build Transform (2D)
TEST(Matrix3_Global_BuildTransform2D)
{
  Vec2 translate(real(1.0), real(10.0));
  Vec2 scale(real(8.0), real(-7.0));

  Mat3 matrix = BuildTransform(translate, Math::cPi / real(2.0), scale);

  real expected[3][3] = MTX(0.0, 7.0, 1.0, 8.0, 0.0, 10.0, 0.0, 0.0, 1.0);
  CHECK_MAT3_CLOSE(expected, matrix, real(0.00001));
}

//-------------------------------------------------- Global Build Transform (3D)
TEST(Matrix3_Global_BuildTransform3D)
{
  Vec3 scale(real(-7.9), real(0.64), real(27.0));
  Quat rotation(real(0.082165510187), real(0.082165510187),
                real(0.082165510187), real(0.989821441881));
  Normalize(rotation);
  Mat3 matrix = BuildTransform(rotation, scale);

  real expected[3][3] = MTX(-7.68666299437, -0.095459856258,  4.75633916083,
                            -1.39166960632,  0.622717002076, -4.02721268589,
                             1.17833260069,  0.112742854183,  26.2708735251);
  CHECK_MAT3_CLOSE(expected, matrix, real(0.00001));
}

//------------------------------------------------- Global Transform (Read Only)
TEST(Matrix3_Global_Transform_ReadOnly)
{
  Mat3 mtx;
  mtx.Rotate(real(1.0), real(0.0), real(0.0), Math::cPi);
  Vec3 foo(real(1.0), real(1.0), real(1.0));

  //Return vector copy stand alone version
  Vec3 ret = Transform(mtx, foo);

  Vec3 expected(real(1.0), real(-1.0), real(-1.0));
  CHECK_VEC3_CLOSE(expected, ret, real(0.000001));
}

//------------------------------------------------ Global Transform (Read/Write)
TEST(Matrix3_Global_Transform_ReadWrite)
{
  Mat3 mtx;
  mtx.Rotate(real(1.0), real(0.0), real(0.0), Math::cPi);

  Vec3 ret(real(5.0), real(3.0), real(2.0));

  //Modify vector directly
  Transform(mtx, &ret);

  Vec3 expected(real(5.0), real(-3.0), real(-2.0));
  CHECK_VEC3_CLOSE(expected, ret, real(0.000001));
}

//-------------------------------------- Global Transposed Transform (Read Only)
TEST(Matrix3_Global_TransposedTransform_ReadOnly)
{
  Mat3 mtx;
  mtx.Rotate(real(1.0), real(0.0), real(0.0), Math::cPi);
  Vec3 foo(real(1.0), real(1.0), real(1.0));

  //Return vector copy stand alone version
  Vec3 ret = TransposedTransform(mtx, foo);

  Vec3 expected(real(1.0), real(-1.0), real(-1.0));
  CHECK_VEC3_CLOSE(expected, ret, real(0.000001));
}

//------------------------------------- Global Transposed Transform (Read/Write)
TEST(Matrix3_Global_TransposedTransform_ReadWrite)
{
  Mat3 mtx;
  mtx.Rotate(real(1.0), real(0.0), real(0.0), Math::cPi);
  Vec3 foo(real(1.0), real(1.0), real(1.0));

  Vec3 ret(real(5.0), real(3.0), real(2.0));

  //Modify vector directly
  TransposedTransform(mtx, &ret);

  Vec3 expected(real(5.0), real(-3.0), real(-2.0));
}

//-------------------------------------------------------------- Global Cofactor
TEST(Matrix3_Global_Cofactor)
{
  Mat3 mtx(real( 3.0), real(-3.0), real( 4.0),
           real(-2.0), real( 2.0), real(-3.0),
           real( 0.0), real( 1.0), real(-4.0));
  Mat3 result;
  for(uint r = 0; r < 3; ++r)
  {
    for(uint c = 0; c < 3; ++c)
    {
      result(r, c) = Cofactor(mtx, r, c);
    }
  }
  real expected[3][3] = MTX(-5.0,  -8.0, -2.0,
                            -8.0, -12.0, -3.0,
                             1.0,   1.0,  0.0);

  CHECK_MAT3(expected, result);
}

//------------------------------------------------------- Global Diagonalization
TEST(Matrix3_Global_Diagonalization)
{
  Mat3 mtx(real(1.0), real( 7.0), real( 3.0),
           real(7.0), real( 4.0), real(-5.0),
           real(3.0), real(-5.0), real( 6.0));
  Diagonalize(&mtx);
}

#undef MTX
