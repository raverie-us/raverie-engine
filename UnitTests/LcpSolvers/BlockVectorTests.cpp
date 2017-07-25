///////////////////////////////////////////////////////////////////////////////
///
///  \file BlockVectorTests.cpp
///  Unit tests for BlockVector and BlockMatrix.
///  
///  Authors: Joshua Davis
///  Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"

#include "LcpStandard.hpp"
#include "Math/ConjugateGradient.hpp"
#include "Math/SimpleCgPolicies.hpp"

#include "TestContainers.hpp"

//----------------------------------------------------------- MatrixNDeterminant_1
TEST(MatrixNDeterminant_1)
{
  MatrixN m(4);
  m(0,0) = real(1.0);
  m(0,1) = real(2.0);
  m(0,2) = real(3.0);
  m(0,3) = real(4.0);

  m(1,0) = real(11.0);
  m(1,1) = real(12.0);
  m(1,2) = real(13.0);
  m(1,3) = real(14.0);

  m(2,0) = real(21.0);
  m(2,1) = real(22.0);
  m(2,2) = real(23.0);
  m(2,3) = real(24.0);

  m(3,0) = real(31.0);
  m(3,1) = real(32.0);
  m(3,2) = real(33.0);
  m(3,3) = real(34.0);

  real result = m.Determinant();
  real expected = real(0.0);
  CHECK_EQUAL(expected,result);
}

//----------------------------------------------------------- MatrixNDeterminant_2
TEST(MatrixNDeterminant_2)
{
  MatrixN m(3);
  m(0,0) = real(3.2);
  m(0,1) = real(8.8);
  m(0,2) = real(4.4);

  m(1,0) = real(1.7);
  m(1,1) = real(3.5);
  m(1,2) = real(6.2);

  m(2,0) = real(5.8);
  m(2,1) = real(6.2);
  m(2,2) = real(3.8);

  real result = m.Determinant();
  real expected = real(136.208);
  CHECK_CLOSE(expected,result,real(.001));
}

//----------------------------------------------------------- MatrixNDeterminant_3
TEST(MatrixNDeterminant_3)
{
  MatrixN m(4);
  m(0,0) = real(2.1);
  m(0,1) = real(1.1);
  m(0,2) = real(8.1);
  m(0,3) = real(3.4);

  m(1,0) = real(5.3);
  m(1,1) = real(3.2);
  m(1,2) = real(8.8);
  m(1,3) = real(4.4);

  m(2,0) = real(7.2);
  m(2,1) = real(1.7);
  m(2,2) = real(3.5);
  m(2,3) = real(6.2);

  m(3,0) = real(6.4);
  m(3,1) = real(5.8);
  m(3,2) = real(6.2);
  m(3,3) = real(3.8);

  real result = m.Determinant();
  real expected = real(-146.917);
  CHECK_CLOSE(expected,result,real(.001));
}

//----------------------------------------------------------- BlockVectorAdd_1
TEST(BlockVectorAdd_1)
{
  BlockVec3 vec;
  vec.SetSize(4);
  vec[0] = Vec3(real(1.0),real(2.0),real(3.0));
  vec[1] = Vec3(real(4.0),real(5.0),real(6.0));
  vec[2] = Vec3(real(7.0),real(8.0),real(9.0));
  vec[3] = Vec3(real(10.0),real(11.0),real(12.0));

  BlockVec3 add;
  add.SetSize(4);
  add[0] = Vec3(real(12.0),real(11.0),real(10.0));
  add[1] = Vec3(real(9.0),real(8.0),real(7.0));
  add[2] = Vec3(real(6.0),real(5.0),real(4.0));
  add[3] = Vec3(real(3.0),real(2.0),real(1.0));

  BlockVec3 expected;
  expected.SetSize(4);
  expected[0] = Vec3(real(13.0),real(13.0),real(13.0));
  expected[1] = Vec3(real(13.0),real(13.0),real(13.0));
  expected[2] = Vec3(real(13.0),real(13.0),real(13.0));
  expected[3] = Vec3(real(13.0),real(13.0),real(13.0));

  vec += add;
  CHECK_BLOCK_VEC3(expected,vec,4);
}

//----------------------------------------------------------- BlockVectorAdd_2
TEST(BlockVectorAdd_2)
{
  BlockVec3 vec;
  vec.SetSize(4);
  vec[0] = Vec3(real(1.0),real(2.0),real(3.0));
  vec[1] = Vec3(real(4.0),real(5.0),real(6.0));
  vec[2] = Vec3(real(7.0),real(8.0),real(9.0));
  vec[3] = Vec3(real(10.0),real(11.0),real(12.0));

  BlockVec3 add;
  add.SetSize(4);
  add[0] = Vec3(real(2.0),real(3.0),real(4.0));
  add[1] = Vec3(real(5.0),real(6.0),real(7.0));
  add[2] = Vec3(real(8.0),real(9.0),real(10.0));
  add[3] = Vec3(real(11.0),real(12.0),real(13.0));

  BlockVec3 expected;
  expected.SetSize(4);
  expected[0] = Vec3(real(3.0),real(5.0),real(7.0));
  expected[1] = Vec3(real(9.0),real(11.0),real(13.0));
  expected[2] = Vec3(real(15.0),real(17.0),real(19.0));
  expected[3] = Vec3(real(21.0),real(23.0),real(25.0));

  vec += add;
  CHECK_BLOCK_VEC3(expected,vec,4);
}

//----------------------------------------------------------- BlockVectorSubtract_1
TEST(BlockVectorSubtract_1)
{
  BlockVec3 vec;
  vec.SetSize(4);
  vec[0] = Vec3(real(1.0),real(2.0),real(3.0));
  vec[1] = Vec3(real(4.0),real(5.0),real(6.0));
  vec[2] = Vec3(real(7.0),real(8.0),real(9.0));
  vec[3] = Vec3(real(10.0),real(11.0),real(12.0));

  BlockVec3 sub;
  sub.SetSize(4);
  sub[0] = Vec3(real(12.0),real(11.0),real(10.0));
  sub[1] = Vec3(real(9.0),real(8.0),real(7.0));
  sub[2] = Vec3(real(6.0),real(5.0),real(4.0));
  sub[3] = Vec3(real(3.0),real(2.0),real(1.0));

  BlockVec3 expected;
  expected.SetSize(4);
  expected[0] = Vec3(real(-11.0),real(-9.0),real(-7.0));
  expected[1] = Vec3(real(-5.0),real(-3.0),real(-1.0));
  expected[2] = Vec3(real(1.0),real(3.0),real(5.0));
  expected[3] = Vec3(real(7.0),real(9.0),real(11.0));

  vec -= sub;
  CHECK_BLOCK_VEC3(expected,vec,4);
}

//----------------------------------------------------------- BlockVectorSubtract_2
TEST(BlockVectorSubtract_2)
{
  BlockVec3 vec;
  vec.SetSize(4);
  vec[0] = Vec3(real(1.0),real(2.0),real(3.0));
  vec[1] = Vec3(real(4.0),real(5.0),real(6.0));
  vec[2] = Vec3(real(7.0),real(8.0),real(9.0));
  vec[3] = Vec3(real(10.0),real(11.0),real(12.0));

  BlockVec3 sub;
  sub.SetSize(4);
  sub[0] = Vec3(real(2.0),real(3.0),real(4.0));
  sub[1] = Vec3(real(5.0),real(6.0),real(7.0));
  sub[2] = Vec3(real(8.0),real(9.0),real(10.0));
  sub[3] = Vec3(real(11.0),real(12.0),real(13.0));

  BlockVec3 expected;
  expected.SetSize(4);
  expected[0] = Vec3(real(-1.0),real(-1.0),real(-1.0));
  expected[1] = Vec3(real(-1.0),real(-1.0),real(-1.0));
  expected[2] = Vec3(real(-1.0),real(-1.0),real(-1.0));
  expected[3] = Vec3(real(-1.0),real(-1.0),real(-1.0));

  vec -= sub;
  CHECK_BLOCK_VEC3(expected,vec,4);
}

//----------------------------------------------------------- BlockVectorScale_1
TEST(BlockVectorScale_1)
{
  BlockVec3 vec;
  vec.SetSize(4);
  vec[0] = Vec3(real(1.0),real(2.0),real(3.0));
  vec[1] = Vec3(real(4.0),real(5.0),real(6.0));
  vec[2] = Vec3(real(7.0),real(8.0),real(9.0));
  vec[3] = Vec3(real(10.0),real(11.0),real(12.0));

  real scale = real(2.0);

  BlockVec3 expected;
  expected.SetSize(4);
  expected[0] = Vec3(real(2.0),real(4.0),real(6.0));
  expected[1] = Vec3(real(8.0),real(10.0),real(12));
  expected[2] = Vec3(real(14),real(16.0),real(18.0));
  expected[3] = Vec3(real(20.0),real(22.0),real(24.0));

  vec *= scale;
  CHECK_BLOCK_VEC3(expected,vec,4);
}

//----------------------------------------------------------- BlockVectorScale_2
TEST(BlockVectorScale_2)
{
  BlockVec3 vec;
  vec.SetSize(4);
  vec[0] = Vec3(real(1.0),real(2.0),real(3.0));
  vec[1] = Vec3(real(4.0),real(5.0),real(6.0));
  vec[2] = Vec3(real(7.0),real(8.0),real(9.0));
  vec[3] = Vec3(real(10.0),real(11.0),real(12.0));

  real scale = real(0.0);

  BlockVec3 expected;
  expected.SetSize(4);
  expected[0] = Vec3(real(0.0),real(0.0),real(0.0));
  expected[1] = Vec3(real(0.0),real(0.0),real(0.0));
  expected[2] = Vec3(real(0.0),real(0.0),real(0.0));
  expected[3] = Vec3(real(0.0),real(0.0),real(0.0));

  vec *= scale;
  CHECK_BLOCK_VEC3(expected,vec,4);
}

//----------------------------------------------------------- BlockVectorScale_3
TEST(BlockVectorScale_3)
{
  BlockVec3 vec;
  vec.SetSize(4);
  vec[0] = Vec3(real(1.0),real(2.0),real(3.0));
  vec[1] = Vec3(real(4.0),real(5.0),real(6.0));
  vec[2] = Vec3(real(7.0),real(8.0),real(9.0));
  vec[3] = Vec3(real(10.0),real(11.0),real(12.0));

  real scale = real(0.5);

  BlockVec3 expected;
  expected.SetSize(4);
  expected[0] = Vec3(real(0.5),real(1.0),real(1.5));
  expected[1] = Vec3(real(2.0),real(2.5),real(3.0));
  expected[2] = Vec3(real(3.5),real(4.0),real(4.5));
  expected[3] = Vec3(real(5.0),real(5.5),real(6.0));

  vec *= scale;
  CHECK_BLOCK_VEC3(expected,vec,4);
}

//----------------------------------------------------------- BlockVectorScale_4
TEST(BlockVectorScale_4)
{
  BlockVec3 vec;
  vec.SetSize(4);
  vec[0] = Vec3(real(1.0),real(2.0),real(3.0));
  vec[1] = Vec3(real(4.0),real(5.0),real(6.0));
  vec[2] = Vec3(real(7.0),real(8.0),real(9.0));
  vec[3] = Vec3(real(10.0),real(11.0),real(12.0));

  real scale = real(-2.0);

  BlockVec3 expected;
  expected.SetSize(4);
  expected[0] = Vec3(real(-2.0),real(-4.0),real(-6.0));
  expected[1] = Vec3(real(-8.0),real(-10.0),real(-12));
  expected[2] = Vec3(real(-14),real(-16.0),real(-18.0));
  expected[3] = Vec3(real(-20.0),real(-22.0),real(-24.0));

  vec *= scale;
  CHECK_BLOCK_VEC3(expected,vec,4);
}

//----------------------------------------------------------- BlockVectorDot_1
TEST(BlockVectorDot_1)
{
  BlockVec3 vec;
  vec.SetSize(4);
  vec[0] = Vec3(real(1.0),real(2.0),real(3.0));
  vec[1] = Vec3(real(4.0),real(5.0),real(6.0));
  vec[2] = Vec3(real(7.0),real(8.0),real(9.0));
  vec[3] = Vec3(real(10.0),real(11.0),real(12.0));

  BlockVec3 vec2;
  vec2.SetSize(4);
  vec2[0] = Vec3(real(12.0),real(11.0),real(10.0));
  vec2[1] = Vec3(real(9.0),real(8.0),real(7.0));
  vec2[2] = Vec3(real(6.0),real(5.0),real(4.0));
  vec2[3] = Vec3(real(3.0),real(2.0),real(1.0));

  real expected = real(364);

  real result = vec.Dot(vec2);
  CHECK_EQUAL(expected,result);
}

//----------------------------------------------------------- BlockVectorDot_2
TEST(BlockVectorDot_2)
{
  BlockVec3 vec;
  vec.SetSize(4);
  vec[0] = Vec3(real(1.0),real(2.0),real(3.0));
  vec[1] = Vec3(real(4.0),real(5.0),real(6.0));
  vec[2] = Vec3(real(7.0),real(8.0),real(9.0));
  vec[3] = Vec3(real(10.0),real(11.0),real(12.0));

  BlockVec3 vec2;
  vec2.SetSize(4);
  vec2[0] = Vec3(real(2.0),real(3.0),real(4.0));
  vec2[1] = Vec3(real(5.0),real(6.0),real(7.0));
  vec2[2] = Vec3(real(8.0),real(9.0),real(10.0));
  vec2[3] = Vec3(real(11.0),real(12.0),real(13.0));

  real expected = real(728);

  real result = vec.Dot(vec2);
  CHECK_EQUAL(expected,result);
}

//----------------------------------------------------------- BlockMatrixTransposed_1
TEST(BlockMatrixTransposed_1)
{
  BlockMat3 mat;
  mat.SetSize(2);
  mat(0,0) = Mat3( 1, 2, 3,
                   7, 8, 9,
                  13,14,15);
  mat(0,1) = Mat3( 4, 5, 6,
                  10,11,12,
                  16,17,18);
  mat(1,0) = Mat3(19,20,21,
                  25,26,27,
                  31,32,33);
  mat(1,1) = Mat3(22,23,24,
                  28,29,30,
                  34,35,36);

  BlockMat3 expected;
  expected.SetSize(2);
  expected(0,0) = Mat3( 1, 7,13,
                        2, 8,14,
                        3, 9,15);
  expected(1,0) = Mat3( 4,10,16,
                        5,11,17,
                        6,12,18);
  expected(0,1) = Mat3(19,25,31,
                       20,26,32,
                       21,27,33);
  expected(1,1) = Mat3(22,28,34,
                       23,29,35,
                       24,30,36);

  BlockMat3 result = mat.Transposed();

  CHECK_BLOCK_MAT3(expected,result,2);
}

//----------------------------------------------------------- BlockMatrixMultiply_1
TEST(BlockMatrixMultiply_1)
{
  BlockMat3 mat1;
  mat1.SetSize(2);
  mat1(0,0) = Mat3( 1, 2, 3,
                    7, 8, 9,
                   13,14,15);
  mat1(0,1) = Mat3( 4, 5, 6,
                   10,11,12,
                   16,17,18);
  mat1(1,0) = Mat3(19,20,21,
                   25,26,27,
                   31,32,33);
  mat1(1,1) = Mat3(22,23,24,
                   28,29,30,
                   34,35,36);
  BlockMat3 mat2;
  mat2.SetSize(2);
  mat2(0,0) = Mat3( 1, 7,13,
                    2, 8,14,
                    3, 9,15);
  mat2(0,1) = Mat3(19,25,31,
                   20,26,32,
                   21,27,33);
  mat2(1,0) = Mat3( 4,10,16,
                    5,11,17,
                    6,12,18);
  mat2(1,1) = Mat3(22,28,34,
                   23,29,35,
                   24,30,36);


  BlockMat3 expected;
  expected.SetSize(2);
  expected(0,0) = Mat3(  91, 217, 343,
                        217, 559, 901,
                        343, 901,1459);
  expected(0,1) = Mat3( 469, 595, 721,
                       1243,1585,1927,
                       2017,2575,3133);
  expected(1,0) = Mat3( 469,1243,2017,
                        595,1585,2575,
                        721,1927,3133);
  expected(1,1) = Mat3(2791,3565,4339,
                       3565,4555,5545,
                       4339,5545,6751);

  BlockMat3 result = mat1.Transform(mat2);

  CHECK_BLOCK_MAT3(expected,result,2);
}

//----------------------------------------------------------- BlockMatrixTransform_1
TEST(BlockMatrixTransform_1)
{
  BlockVec3 vec;
  vec.SetSize(4);
  vec[0] = Vec3(real(1.0),real(2.0),real(3.0));
  vec[1] = Vec3(real(4.0),real(5.0),real(6.0));
  vec[2] = Vec3(real(7.0),real(8.0),real(9.0));
  vec[3] = Vec3(real(10.0),real(11.0),real(12.0));

  BlockMat3 mat;
  mat.SetSize(4);
  for(uint i = 0; i < 4; ++i)
     mat(i,i) = Mat3::cIdentity;

  BlockVec3 expected = vec;

  BlockVec3 result;
  result.SetSize(4);
  mat.Transform(vec,result);

  CHECK_BLOCK_VEC3(expected,result,4);
}

//----------------------------------------------------------- BlockMatrixTransform_2
TEST(BlockMatrixTransform_2)
{
  BlockVec3 vec;
  vec.SetSize(4);
  vec[0] = Vec3(real(1.0),real(2.0),real(3.0));
  vec[1] = Vec3(real(4.0),real(5.0),real(6.0));
  vec[2] = Vec3(real(7.0),real(8.0),real(9.0));
  vec[3] = Vec3(real(10.0),real(11.0),real(12.0));

  BlockMat3 mat;
  mat.SetSize(4);
  for(uint i = 0; i < 4; ++i)
    mat(i,i) = Mat3::cIdentity * real(2.0);

  BlockVec3 expected;
  expected.SetSize(4);
  expected[0] = Vec3(real(2.0),real(4.0),real(6.0));
  expected[1] = Vec3(real(8.0),real(10.0),real(12.0));
  expected[2] = Vec3(real(14.0),real(16.0),real(18.0));
  expected[3] = Vec3(real(20.0),real(22.0),real(24.0));

  BlockVec3 result;
  result.SetSize(4);
  mat.Transform(vec,result);

  CHECK_BLOCK_VEC3(expected,result,4);
}

//----------------------------------------------------------- BlockMatrixTransform_3
TEST(BlockMatrixTransform_3)
{
  BlockVec3 vec;
  vec.SetSize(4);
  vec[0] = Vec3(real(1.0),real(2.0),real(3.0));
  vec[1] = Vec3(real(4.0),real(5.0),real(6.0));
  vec[2] = Vec3(real(7.0),real(8.0),real(9.0));
  vec[3] = Vec3(real(10.0),real(11.0),real(12.0));

  BlockMat3 mat;
  mat.SetSize(4);
  for(uint i = 0; i < 4; ++i)
    for(uint j = 0; j < 4; ++j)
    mat(i,j) = Mat3::cIdentity;

  BlockVec3 expected;
  expected.SetSize(4);
  expected[0] = Vec3(real(22.0),real(26.0),real(30.0));
  expected[1] = Vec3(real(22.0),real(26.0),real(30.0));
  expected[2] = Vec3(real(22.0),real(26.0),real(30.0));
  expected[3] = Vec3(real(22.0),real(26.0),real(30.0));

  BlockVec3 result;
  result.SetSize(4);
  mat.Transform(vec,result);

  CHECK_BLOCK_VEC3(expected,result,4);
}

//----------------------------------------------------------- BlockMatrixTransform_4
TEST(BlockMatrixTransform_4)
{
  BlockVec3 vec;
  vec.SetSize(4);
  vec[0] = Vec3(real(1.0),real(2.0),real(3.0));
  vec[1] = Vec3(real(4.0),real(5.0),real(6.0));
  vec[2] = Vec3(real(7.0),real(8.0),real(9.0));
  vec[3] = Vec3(real(10.0),real(11.0),real(12.0));

  BlockMat3 mat;
  mat.SetSize(4);
  mat(0,0) = Mat3(real(.1),real(.2),real(.3),
                  real(.13),real(.14),real(.15),
                  real(.25),real(.26),real(.27));
  mat(0,1) = Mat3(real(.4),real(.5),real(.6),
                  real(.16),real(.17),real(.18),
                  real(.28),real(.29),real(.30));
  mat(0,2) = Mat3(real(.7),real(.8),real(.9),
                  real(.19),real(.20),real(.21),
                  real(.31),real(.32),real(.33));
  mat(0,3) = Mat3(real(.10),real(.11),real(.12),
                  real(.22),real(.23),real(.24),
                  real(.34),real(.35),real(.36));
  mat(1,0) = Mat3(real(.37),real(.38),real(.39),
                  real(.49),real(.50),real(.51),
                  real(.61),real(.62),real(.63));
  mat(1,1) = Mat3(real(.40),real(.41),real(.42),
                  real(.52),real(.53),real(.54),
                  real(.64),real(.65),real(.66));
  mat(1,2) = Mat3(real(.43),real(.44),real(.45),
                  real(.55),real(.56),real(.57),
                  real(.67),real(.68),real(.69));
  mat(1,3) = Mat3(real(.46),real(.47),real(.48),
                  real(.58),real(.59),real(.60),
                  real(.70),real(.71),real(.72));
  mat(2,0) = Mat3(real(.73),real(.74),real(.75),
                  real(.85),real(.86),real(.87),
                  real(.97),real(.98),real(.99));
  mat(2,1) = Mat3(real(.76),real(.77),real(.78),
                  real(.88),real(.89),real(.90),
                  real(1.00),real(1.01),real(1.02));
  mat(2,2) = Mat3(real(.79),real(.80),real(.81),
                  real(.91),real(.92),real(.93),
                  real(1.03),real(1.04),real(1.05));
  mat(2,3) = Mat3(real(.82),real(.83),real(.84),
                  real(.94),real(.95),real(.96),
                  real(1.06),real(1.07),real(1.08));
  mat(3,0) = Mat3(real(1.09),real(1.10),real(1.11),
                  real(1.21),real(1.22),real(1.23),
                  real(1.33),real(1.34),real(1.35));
  mat(3,1) = Mat3(real(1.12),real(1.13),real(1.14),
                  real(1.24),real(1.25),real(1.26),
                  real(1.36),real(1.37),real(1.38));
  mat(3,2) = Mat3(real(1.15),real(1.16),real(1.17),
                  real(1.27),real(1.28),real(1.29),
                  real(1.39),real(1.40),real(1.41));
  mat(3,3) = Mat3(real(1.18),real(1.19),real(1.20),
                  real(1.30),real(1.31),real(1.32),
                  real(1.42),real(1.43),real(1.44));

  BlockVec3 expected;
  expected.SetSize(4);
  expected[0] = Vec3(real(32.15),real(15.86),real(25.22));
  expected[1] = Vec3(real(34.58),real(43.94),real(53.3));
  expected[2] = Vec3(real(62.66),real(72.02),real(81.38));
  expected[3] = Vec3(real(90.74),real(100.1),real(109.46));

  BlockVec3 result;
  result.SetSize(4);
  mat.Transform(vec,result);

  CHECK_BLOCK_VEC3_CLOSE(expected,result,4,real(.0001));
}

TEST(BlockMatrixMultiply_5)
{
  Math::BlockMatrix3 A;
  A.SetSize(2);
  A(0,0) = Mat3(real(9.5),real(2.2),real(5.4),
                real(2.2),real(4.2),real(3.1),
                real(5.4),real(3.1),real(7.7));
  A(1,1) = Mat3::cIdentity;

  Math::BlockVector3 x;
  x.SetSize(2);
  x[0] = Vec3(real(2.0),real(0.0),real(1.1));
  x[1] = Vec3(-real(1.2),real(3.1),real(9.0));

  Math::BlockVector3 expected;
  expected.SetSize(2);
  expected[0] = Vec3(real(24.94),real(7.81),real(19.27));
  expected[1] = Vec3(-real(1.2),real(3.1),real(9.0));

  Math::BlockVector3 result;
  result.SetSize(2);
  A.Transform(x,result);

  CHECK_BLOCK_VEC3_CLOSE(expected, result, 2, real(.01));
}