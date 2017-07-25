///////////////////////////////////////////////////////////////////////////////
///
///  \file MatrixSSETests.cpp
///  Unit tests for MatrixSim.
///  
///  Authors: Joshua Davis
///  Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
#include "Math/MathStandard.hpp"
#include "TestSSE.hpp"

using namespace Math;
using namespace Simd;

namespace
{
  //Setting the values of a matrix every time is really irritating, don't change!
  __declspec(align(16))const scalar init3[9]  = { scalar( 2.0), scalar( 3.0), scalar( 5.0),
    scalar( 7.0), scalar(11.0), scalar(13.0),
    scalar(17.0), scalar(19.0), scalar(23.0) };
  __declspec(align(16))const scalar init4[16] = { scalar( 2.0), scalar( 3.0), scalar( 5.0), scalar( 7.0),
    scalar(11.0), scalar(13.0), scalar(17.0), scalar(19.0),
    scalar(23.0), scalar(29.0), scalar(31.0), scalar(37.0),
    scalar(41.0), scalar(43.0), scalar(47.0), scalar(53.0) };
  __declspec(align(16))const scalar init44[4][4] = { { scalar( 2.0), scalar( 3.0), scalar( 5.0), scalar( 7.0)  },
  { scalar(11.0), scalar(13.0), scalar(17.0), scalar(19.0), },
  { scalar(23.0), scalar(29.0), scalar(31.0), scalar(37.0), },
  { scalar(41.0), scalar(43.0), scalar(47.0), scalar(53.0)  }
  };
}

//------------------------------------------------------------- Equal Comparison
TEST(SimMat_EqualComparison)
{
  SimMat4 mat = LoadMat4x4(init4);
  SimMat4 tam = LoadMat4x4(init4);

  SimMat4 result = Equal(mat,tam);
  CheckBool_SimVec4(result.columns[0]);
  CheckBool_SimVec4(result.columns[1]);
  CheckBool_SimVec4(result.columns[2]);
  CheckBool_SimVec4(result.columns[3]);
}

//------------------------------------------------------------- Build Scale
TEST(SimMat_BuildScale)
{
  SimVec scale = Set4(3,6,9,2);
  SimMat4 mat = BuildScale(scale);
  SimMat4 tam = SetMat4x4(3,0,0,0,
                          0,6,0,0,
                          0,0,9,0,
                          0,0,0,1);

  SimMat4 result = Equal(mat,tam);
  CheckBool_SimMat4(result);
}

//-------------------------------------------------------------- Rotate (vector)
TEST(SimMat_Rotate_Vector)
{
  //StubbedTest(MatSim_Rotate_Vector);
  //return;

  //Rotate vector radian
  SimVec axis = Normalize4(Set4(1.0f, 1.0f, 0.0f, 0.0f));
  SimMat4 mtx = BuildRotation(axis,scalar(.25) * scalar(3.1415926535897932384626433832795));

  SimMat4 expected = SetMat4x4(0.85355339059327376220042218105242f,
    0.14644660940672623779957781894757f,
    0.5f,
    0.0f,
    0.14644660940672623779957781894757f,
    0.85355339059327376220042218105242f,
    -0.5f,
    0.0f,
    -0.5f,
    0.5f,
    0.70710678118654752440084436210485f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f);
  Check_MatSim4_Close(expected, mtx, real(0.000001));
}

//-------------------------------------------------------------- Rotate (vector) 1
TEST(SimMat_Rotate_Vector_1)
{
  //Rotate vector radian
  SimVec axis = Normalize4(Set4(1.0f, 0.0f, 0.0f, 0.0f));
  SimMat4 mtx = BuildRotation(axis,scalar(.5) * scalar(3.1415926535897932384626433832795));
  SimVec v = Set4(0,1,0,0);

  SimVec rotatedV = TransformPoint(mtx,v);
  SimVec expected = Set4(0,0,1,0);

  Check_SimVec4_Close(expected,rotatedV,real(.0001));
}

//-------------------------------------------------------------- Rotate (vector) 2
TEST(SimMat_Rotate_Vector_2)
{
  //Rotate vector radian
  SimVec axis = Normalize4(Set4(0.0f, 1.0f, 0.0f, 0.0f));
  SimMat4 mtx = BuildRotation(axis,scalar(.5) * scalar(3.1415926535897932384626433832795));
  SimVec v = Set4(0,0,1,0);

  SimVec rotatedV = TransformPoint(mtx,v);
  SimVec expected = Set4(1,0,0,0);

  Check_SimVec4_Close(expected,rotatedV,real(.0001));
}

//-------------------------------------------------------------- Rotate (vector) 3
TEST(SimMat_Rotate_Vector_3)
{
  //Rotate vector radian
  SimVec axis = Normalize4(Set4(0.0f, 0.0f, 1.0f, 0.0f));
  SimMat4 mtx = BuildRotation(axis,scalar(.5) * scalar(3.1415926535897932384626433832795));
  SimVec v = Set4(1,0,0,0);

  Mat4 m;
  m.Rotate(0.0f, 0.0f, 1.0f,scalar(.5) * scalar(3.1415926535897932384626433832795));

  SimVec rotatedV = TransformPoint(mtx,v);
  SimVec expected = Set4(0,1,0,0);

  Check_SimVec4_Close(expected,rotatedV,real(.0001));
}

//-------------------------------------------------------------- Rotate (vector) 4
TEST(SimMat_Rotate_Vector_4)
{
  //Rotate vector radian
  SimVec axis = Normalize4(Set4(0.0f, 1.0f, 0.0f, 0.0f));
  SimMat4 mtx = BuildRotation(axis,scalar(.25) * scalar(3.1415926535897932384626433832795));
  SimVec v = Set4(0,0,1,0);

  Mat4 r;
  r.Rotate(Vec3(0,1,0),scalar(.25) * scalar(3.1415926535897932384626433832795));

  SimVec rotatedV = TransformPoint(mtx,v);
  SimVec expected = Normalize4(Set4(1,0,1,0));

  Check_SimVec4_Close(expected,rotatedV,real(.0001));
}

//------------------------------------------------------------- Build Translation
TEST(SimMat_BuildTranslation)
{
  SimVec translation = Set4(8.0,10.0,7.0,3.0);
  SimMat4 mat = BuildTranslation(translation);
  SimMat4 tam = SetMat4x4(1.0, 0.0, 0.0,  8.0,
                          0.0, 1.0, 0.0, 10.0,
                          0.0, 0.0, 1.0,  7.0,
                          0.0, 0.0, 0.0,  1.0);

  SimMat4 result = Equal(mat,tam);
  CheckBool_SimMat4(result);
}

//------------------------------------------ Build Transform (Quaternion)
TEST(SimMat_BuildTransform_Quaternion)
{
  SimVec translation = Set3(scalar(61.0), scalar(-975.0), scalar(0.259));
  SimVec scale = Set3(scalar(-7.9), scalar(0.64), scalar(27.0));
  SimVec rotation = Set4(scalar(0.082165510187), scalar(0.082165510187),
                         scalar(0.082165510187), scalar(0.989821441881));
  rotation = Normalize4(rotation);

  SimMat4 transform = BuildTransform(translation,rotation,scale);

  SimMat4 expected = SetMat4x4(-7.68666299437f, -0.095459856258f,
                              4.75633916083f,            61.0f,
                              -1.39166960632f,  0.622717002076f,
                              -4.02721268589f,          -975.0f,
                              1.17833260069f,  0.112742854183f,
                              26.2708735251f,           0.259f,
                              0.0f,             0.0f,
                              0.0f,             1.0f);
  Check_MatSim4_Close(expected, transform, scalar(0.00001));
}

//--------------------------------------------- Build Transform (AxisAngle)
TEST(SimMat_BuildTransform3D_AxisAngle)
{
  SimVec translate = Set3(scalar(872.3), scalar(572.11), scalar(867.5));
  SimVec scale = Set3(scalar(2.3), scalar(8.3), scalar(1.4));
  SimVec axis = Set3(0,1,0);
  scalar angle = scalar(.25) * scalar(3.1415926535897932384626433832795);
  SimMat4 matrix = BuildTransform(translate, axis,angle, scale);

  SimMat4 expected = SetMat4x4( .707107f * 2.3f,     0.0f, .707107f * 1.4f, 872.3f,
                                           0.0f, 1 * 8.3f,            0.0f,572.11f,
                               -.707107f * 2.3f,     0.0f, .707107f * 1.4f, 867.5f,
                                           0.0f,     0.0f,            0.0f,     1);
  Check_MatSim4_Close(expected, matrix,scalar(.00001));
}

//--------------------------------------------- Build Transform (Matrix3)
TEST(SimMat_BuildTransform3D_Matrix3)
{
  SimVec translate = Set3(scalar(872.3), scalar(572.11), scalar(867.5));
  SimVec scale = Set3(scalar(2.3), scalar(8.3), scalar(1.4));
  SimMat4 rotation = SetMat4x4(scalar( 0.0), scalar(-1.0), scalar( 0.0), scalar( 2.0),
                               scalar( 0.0), scalar( 0.0), scalar(-1.0), scalar( 3.0),
                               scalar( 1.0), scalar( 0.0), scalar( 0.0), scalar( 4.0),
                               scalar( 2.0), scalar( 3.0), scalar( 4.0), scalar( 1.0));
  SimMat4 matrix = BuildTransform(translate, rotation, scale);
  SimMat4 expected = SetMat4x4(0.0f, -8.3f,  0.0f,  872.3f,
                               0.0f,  0.0f, -1.4f, 572.11f,
                               2.3f,  0.0f,  0.0f,  867.5f,
                               0.0f,  0.0f,  0.0f,    1.0f);
  Check_MatSim4_Close(expected, matrix,real(.00001));
}

//------------------------------------------------------------------- Transposed
TEST(SimMat_Transposed)
{
  SimMat4 mtx = SetMat4x4(scalar( 0.0), scalar( 1.0), scalar( 2.0), scalar( 3.0),
                          scalar( 4.0), scalar( 5.0), scalar( 6.0), scalar( 7.0),
                          scalar( 8.0), scalar( 9.0), scalar(10.0), scalar(11.0),
                          scalar(12.0), scalar(13.0), scalar(14.0), scalar(15.0));

  //Transpose
  SimMat4 transpose = Transpose4x4(mtx);

  SimMat4 expected = SetMat4x4(0.0f, 4.0f,  8.0f, 12.0f,
                               1.0f, 5.0f,  9.0f, 13.0f,
                               2.0f, 6.0f, 10.0f, 14.0f,
                               3.0f, 7.0f, 11.0f, 15.0f);
  SimMat4 result = Equal(transpose,expected);
  CheckBool_SimMat4(result);
}

//------------------------------------------------------------------- Transposed 3x3
TEST(SimMat_Transposed3x3)
{
  SimMat4 mtx = SetMat4x4(scalar( 0.0), scalar( 1.0), scalar( 2.0), scalar( 3.0),
                          scalar( 4.0), scalar( 5.0), scalar( 6.0), scalar( 7.0),
                          scalar( 8.0), scalar( 9.0), scalar(10.0), scalar(11.0),
                          scalar(12.0), scalar(13.0), scalar(14.0), scalar(15.0));

  //Transpose
  SimMat4 transpose = TransposeUpper3x3(mtx);

  SimMat4 expected = SetMat4x4( 0.0f, 4.0f,  8.0f,  3.0f,
                                1.0f, 5.0f,  9.0f,  7.0f,
                                2.0f, 6.0f, 10.0f, 11.0f,
                               12.0f,13.0f, 14.0f, 15.0f);
  SimMat4 result = Equal(transpose,expected);
  CheckBool_SimMat4(result);
}

//------------------------------------------------------------------- Affine Inverse4x4
TEST(SimMat_AffineInverse4x4)
{
  SimVec translation = Set3(scalar(61.0), scalar(-975.0), scalar(0.259));
  SimVec scale = Set3(scalar(1), scalar(1), scalar(1));
  SimVec rotation = Set4(scalar(0.082165510187), scalar(0.082165510187),
                         scalar(0.082165510187), scalar(0.989821441881));
  rotation = Normalize4(rotation);

  //Transpose
  SimMat4 transform = BuildTransform(translation,rotation,scale);
  SimMat4 invTransform = AffineInverse4x4(transform);

  SimVec expected = Set4(1,2,3,4);
  SimVec tVec = TransformPoint(transform,expected);
  SimVec actual = TransformPoint(invTransform,tVec);

  Check_SimVec4_Close(expected, actual, scalar(0.00001));
}

//------------------------------------------------------------------- Affine InverseWithScale4x4
TEST(SimMat_AffineInverseWithScale4x4)
{
  SimVec translation = Set3(scalar(61.0), scalar(-975.0), scalar(0.259));
  SimVec scale = Set3(scalar(7.9), scalar(0.64), scalar(27.0));
  SimVec rotation = Set4(scalar(0.082165510187), scalar(0.082165510187),
    scalar(0.082165510187), scalar(0.989821441881));
  rotation = Normalize4(rotation);

  SimVec expected = Set4(1,2,3,4);

  SimMat4 transform = BuildTransform(translation,rotation,scale);
  SimVec tVec = TransformPoint(transform,expected);
  SimMat4 invTransform = AffineInverseWithScale4x4(transform);
  SimVec actual = TransformPoint(invTransform,tVec);

  Check_SimVec4_Close(expected, actual, scalar(0.00001));
}

//------------------------------------------------------------- Transform Point
TEST(SimMat_TransformPoint)
{
  SimVec vec = Set4(-1,2,-3,4);
  SimMat4 mat = LoadMat4x4(init4);
  SimVec result = TransformPoint(mat,vec);
  SimVec expected = Set4(115,108,124,132);
  Check_SimVec4(expected,result);
}

//------------------------------------------------------------- Transposed Transform Point
TEST(SimMat_TransposedTransformPoint)
{
  SimVec vec = Set4(-1,2,-3,4);
  SimMat4 mat = LoadMat4x4(init4);
  SimVec result = TransposeTransformPoint(mat,vec);
  SimVec expected = Set4(17,40,90,116);
  Check_SimVec4(expected,result);
}

//------------------------------------------------------------- Transform Normal
TEST(SimMat_TransformNormal)
{
  SimVec vec = Set4(-1,2,-3,0);
  SimMat4 mat = SetMat4x4( 2.0f,  3.0f,  5.0f,  7.0f,
                          11.0f, 13.0f, 17.0f, 19.0f,
                          23.0f, 29.0f, 31.0f, 37.0f,
                           0.0f,  0.0f,  0.0f,  1.0f);
  SimVec result = TransformNormal(mat,vec);
  SimVec expected = Set4(-11,-36,-58,0);
  Check_SimVec4(expected,result);
}

//------------------------------------------------------------- Transposed Transform Normal
TEST(SimMat_TransposedTransformNormal)
{
  SimVec vec = Set4(-1,2,-3,0);
  SimMat4 mat = SetMat4x4( 2.0f,  3.0f,  5.0f,  7.0f,
                          11.0f, 13.0f, 17.0f, 19.0f,
                          23.0f, 29.0f, 31.0f, 37.0f,
                           0.0f,  0.0f,  0.0f,  1.0f);
  SimVec result = TransposeTransformNormal(mat,vec);
  SimVec expected = Set4(-49,-64,-64,0);
  Check_SimVec4(expected,result);
}

//------------------------------------------------------------- Transform Projected
TEST(SimMat_TransformProjected)
{
  SimVec expected = Set4(-1,2,-3,1);
  SimMat4 mat = SetMat4x4(2.3119643f,       0.0f,       0.0f, 0.0f,
                                0.0f, 2.4142134f,       0.0f, 0.0f,
                                0.0f,       0.0f,-1.0050125f,-1.0f,
                                0.0f,       0.0f,-1.0025063f, 0.0f);
  SimMat4 inv = SetMat4x4(.432533f,    0.0f,         0.0f,   0.0f,
                              0.0f,.414214f,         0.0f,   0.0f,
                              0.0f,    0.0f,-1.11022e-16f,-.9975f,
                              0.0f,    0.0f,        -1.0f,1.0025f);
  SimVec result = TransformPointProjected(mat,expected);
  SimVec actual = TransformPointProjected(inv,result);

  Check_SimVec4_Close(expected,actual,scalar(.0001));
}

//-------------------------------------------------------------- Matrix Addition
TEST(SimMat_MatrixAddition)
{
  SimMat4 mtx = SetMat4x4(scalar( 0.0), scalar( 1.0), scalar( 2.0), scalar( 3.0),
                          scalar( 4.0), scalar( 5.0), scalar( 6.0), scalar( 7.0),
                          scalar( 8.0), scalar( 9.0), scalar(10.0), scalar(11.0),
                          scalar(12.0), scalar(13.0), scalar(14.0), scalar(15.0));

  SimMat4 add = SetMat4x4(scalar( 2.0), scalar( 4.0), scalar( 6.0), scalar( 8.0),
                          scalar(10.0), scalar(12.0), scalar(14.0), scalar(16.0),
                          scalar(18.0), scalar(20.0), scalar(22.0), scalar(24.0),
                          scalar(26.0), scalar(28.0), scalar(30.0), scalar(32.0));
   
  //Addition
  SimMat4 res = Add(mtx,add);
  SimMat4 expected = SetMat4x4( 2.0f,  5.0f,  8.0f, 11.0f,
                               14.0f, 17.0f, 20.0f, 23.0f,
                               26.0f, 29.0f, 32.0f, 35.0f,
                               38.0f, 41.0f, 44.0f, 47.0f);

  CheckBool_SimMat4(Equal(res,expected));
}

//----------------------------------------------------------- Matrix Subtraction
TEST(SimMat_MatrixSubtraction)
{
  SimMat4 mtx = SetMat4x4(scalar( 0.0), scalar( 1.0), scalar( 2.0), scalar( 3.0),
                          scalar( 4.0), scalar( 5.0), scalar( 6.0), scalar( 7.0),
                          scalar( 8.0), scalar( 9.0), scalar(10.0), scalar(11.0),
                          scalar(12.0), scalar(13.0), scalar(14.0), scalar(15.0));

  SimMat4 sub = SetMat4x4(scalar( 2.0), scalar( 4.0), scalar( 6.0), scalar( 8.0),
                          scalar(10.0), scalar(12.0), scalar(14.0), scalar(16.0),
                          scalar(18.0), scalar(20.0), scalar(22.0), scalar(24.0),
                          scalar(26.0), scalar(28.0), scalar(30.0), scalar(32.0));

  //Subtraction
  SimMat4 res = Subtract(sub,mtx);

  SimMat4 expected = SetMat4x4( 2.0f,  3.0f,  4.0f,  5.0f,
                                6.0f,  7.0f,  8.0f,  9.0f,
                               10.0f, 11.0f, 12.0f, 13.0f,
                               14.0f, 15.0f, 16.0f, 17.0f);
  CheckBool_SimMat4(Equal(res,expected));
}


//-------------------------------------------------------------- Matrix ComponentScale
TEST(SimMat_MatrixComponentScale)
{
  SimMat4 mtx = SetMat4x4(scalar( 0.0), scalar( 1.0), scalar( 2.0), scalar( 3.0),
                          scalar( 4.0), scalar( 5.0), scalar( 6.0), scalar( 7.0),
                          scalar( 8.0), scalar( 9.0), scalar(10.0), scalar(11.0),
                          scalar(12.0), scalar(13.0), scalar(14.0), scalar(15.0));

  SimMat4 mul = SetMat4x4(scalar( 2.0), scalar( 3.0), scalar( 6.0), scalar( 9.0),
                          scalar(12.0), scalar(15.0), scalar(18.0), scalar(21.0),
                          scalar(24.0), scalar(27.0), scalar(30.0), scalar(33.0),
                          scalar(36.0), scalar(39.0), scalar(42.0), scalar(45.0));

  //Multiplication
  SimMat4 res = ComponentScale(mtx,mul);
  SimMat4 expected = SetMat4x4(  0.0f,  3.0f, 12.0f, 27.0f,
                                48.0f, 75.0f,108.0f,147.0f,
                               192.0f,243.0f,300.0f,363.0f,
                               432.0f,507.0f,588.0f,675.0f);

  CheckBool_SimMat4(Equal(res,expected));
}

//-------------------------------------------------------------- Matrix ComponentDivide
TEST(SimMat_MatrixComponentDivide)
{
  SimMat4 expected = SetMat4x4(scalar( 0.0), scalar( 1.0), scalar( 2.0), scalar( 3.0),
                               scalar( 4.0), scalar( 5.0), scalar( 6.0), scalar( 7.0),
                               scalar( 8.0), scalar( 9.0), scalar(10.0), scalar(11.0),
                               scalar(12.0), scalar(13.0), scalar(14.0), scalar(15.0));

  SimMat4 mtx = SetMat4x4(  0.0f,  3.0f, 12.0f, 27.0f,
                           48.0f, 75.0f,108.0f,147.0f,
                          192.0f,243.0f,300.0f,363.0f,
                          432.0f,507.0f,588.0f,675.0f);

  SimMat4 div = SetMat4x4(scalar( 2.0), scalar( 3.0), scalar( 6.0), scalar( 9.0),
                          scalar(12.0), scalar(15.0), scalar(18.0), scalar(21.0),
                          scalar(24.0), scalar(27.0), scalar(30.0), scalar(33.0),
                          scalar(36.0), scalar(39.0), scalar(42.0), scalar(45.0));

  //Division
  SimMat4 res = ComponentDivide(mtx,div);
  

  CheckBool_SimMat4(Equal(res,expected));
}


//------------------------------------------------------ Matrix Multiplication 1
TEST(SimMat_MatrixMultiplication_1)
{
  SimMat4 mtx = SetMat4x4(scalar( 0.0), scalar( 1.0), scalar( 2.0), scalar( 3.0),
    scalar( 4.0), scalar( 5.0), scalar( 6.0), scalar( 7.0),
    scalar( 8.0), scalar( 9.0), scalar(10.0), scalar(11.0),
    scalar(12.0), scalar(13.0), scalar(14.0), scalar(15.0));

  SimMat4 add = SetMat4x4(scalar( 2.0), scalar( 4.0), scalar( 6.0), scalar( 8.0),
    scalar(10.0), scalar(12.0), scalar(14.0), scalar(16.0),
    scalar(18.0), scalar(20.0), scalar(22.0), scalar(24.0),
    scalar(26.0), scalar(28.0), scalar(30.0), scalar(32.0));

  //Multiplication
  SimMat4 res = Multiply(add,mtx);

  SimMat4 expected = SetMat4x4(160.0f, 180.0f, 200.0f,  220.0f,
                               352.0f, 404.0f, 456.0f,  508.0f,
                               544.0f, 628.0f, 712.0f,  796.0f,
                               736.0f, 852.0f, 968.0f, 1084.0f);
  CheckBool_SimMat4(Equal(res,expected));
}

//------------------------------------------------------ Matrix Multiplication 2
TEST(SimMat_MatrixMultiplication_2)
{
  SimMat4 mtx = SetMat4x4(scalar( 0.0), scalar( 1.0), scalar( 2.0), scalar( 3.0),
    scalar( 4.0), scalar( 5.0), scalar( 6.0), scalar( 7.0),
    scalar( 8.0), scalar( 9.0), scalar(10.0), scalar(11.0),
    scalar(12.0), scalar(13.0), scalar(14.0), scalar(15.0));

  SimMat4 add = SetMat4x4(scalar( 2.0), scalar( 4.0), scalar( 6.0), scalar( 8.0),
    scalar(10.0), scalar(12.0), scalar(14.0), scalar(16.0),
    scalar(18.0), scalar(20.0), scalar(22.0), scalar(24.0),
    scalar(26.0), scalar(28.0), scalar(30.0), scalar(32.0));
  SimMat4 res = Multiply(mtx,add);

  SimMat4 expected = SetMat4x4(124.0f, 136.0f,  148.0f,  160.0f,
                               348.0f, 392.0f,  436.0f,  480.0f,
                               572.0f, 648.0f,  724.0f,  800.0f,
                               796.0f, 904.0f, 1012.0f, 1120.0f);
  CheckBool_SimMat4(Equal(res,expected));
}


//--------------------------------------------------------------- Set Identity
TEST(SimMat_SetIdentity)
{
  SimMat4 mtx = LoadMat4x4(init4);
  mtx = IdentityMat4x4();

  SimMat4 expected = SetMat4x4(1.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, 1.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 1.0f, 0.0f,
                               0.0f, 0.0f, 0.0f, 1.0f);
  CheckBool_SimMat4(Equal(expected, mtx));
}

//------------------------------------------------------------------- Zero Out
TEST(SimMat_ZeroOut)
{
  SimMat4 mtx = LoadMat4x4(init4);
  mtx = ZeroOutMat4x4();

  SimMat4 expected = SetMat4x4(0.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.0f, 0.0f);
  CheckBool_SimMat4(Equal(expected, mtx));
}

//---------------------------------------------------------------------- Basis X
TEST(SimMat_BasisX)
{
  SimMat4 mtx = IdentityMat4x4();
  SimVec basisX = BasisX(mtx);
  SimVec expected = Set4(scalar(1.0), scalar(0.0), scalar(0.0), scalar(0.0));
  Check_SimVec4(expected, basisX);
}

//---------------------------------------------------------------------- Basis Y
TEST(SimMat_BasisY)
{
  SimMat4 mtx = IdentityMat4x4();
  SimVec basisY = BasisY(mtx);
  SimVec expected = Set4(scalar(0.0), scalar(1.0), scalar(0.0), scalar(0.0));
  Check_SimVec4(expected, basisY);
}

//---------------------------------------------------------------------- Basis Z
TEST(SimMat_BasisZ)
{
  SimMat4 mtx = IdentityMat4x4();
  SimVec basisZ = BasisZ(mtx);
  SimVec expected = Set4(scalar(0.0), scalar(0.0), scalar(1.0), scalar(0.0));
  Check_SimVec4(expected, basisZ);
}

//---------------------------------------------------------------------- Basis W
TEST(SimMat_BasisW)
{
  SimMat4 mtx = IdentityMat4x4();
  SimVec basisW = BasisW(mtx);
  SimVec expected = Set4(scalar(0.0), scalar(0.0), scalar(0.0), scalar(1.0));
  Check_SimVec4(expected, basisW);
}

