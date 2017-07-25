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

//typedef Math::real        real;
typedef Math::Matrix3     Mat3;
typedef Math::Matrix4     Mat4;
typedef Math::Vector2     Vec2;
typedef Math::Vector3     Vec3;
typedef Math::Vector4     Vec4;
typedef Math::Quaternion  Quat;

using namespace Math;
using namespace Simd;

namespace
{
  //Setting the values of a matrix every time is really irritating, don't change!
  __declspec(align(16))const scalar init3[12]  = { scalar( 2.0), scalar( 3.0), scalar( 5.0), scalar(6.0),
                                                   scalar( 7.0), scalar(11.0), scalar(13.0), scalar(4.0),
                                                   scalar(17.0), scalar(19.0), scalar(23.0), scalar(24.0) };
}

//------------------------------------------------------------- Equal Comparison
TEST(SimMat3_EqualComparison)
{
  SimMat3 mat = LoadMat3(init3);
  SimMat3 tam = LoadMat3(init3);

  SimMat3 result = Equal(mat,tam);
  CheckBool_SimVec4(result.columns[0]);
  CheckBool_SimVec4(result.columns[1]);
  CheckBool_SimVec4(result.columns[2]);
}

//------------------------------------------------------------- Build Scale
TEST(SimMat3_BuildScale3)
{
  SimVec scale = Set4(3,6,9,2);
  SimMat3 mat = BuildScale3(scale);
  SimMat3 tam = SetMat3(3,0,0,
                        0,6,0,
                        0,0,9);

  SimMat3 result = Equal(mat,tam);
  CheckBool_SimMat3(result);
}

//-------------------------------------------------------------- Rotate (vector)
TEST(SimMat3_Rotate_Vector)
{
  //Rotate vector radian
  SimVec axis = Normalize4(Set4(1.0f, 1.0f, 0.0f, 0.0f));
  SimMat3 mtx = BuildRotation3(axis,scalar(.25) * scalar(3.1415926535897932384626433832795));

  SimMat3 expected = SetMat3(0.85355339059327376220042218105242f, 0.14644660940672623779957781894757f,                               0.5f,
                             0.14644660940672623779957781894757f, 0.85355339059327376220042218105242f,                              -0.5f,
                                                           -0.5f,                                0.5f,0.70710678118654752440084436210485f);
  Check_MatSim3_Close(expected, mtx, real(0.000001));
}

//-------------------------------------------------------------- Rotate (vector) 1
TEST(SimMat3_Rotate_Vector_1)
{
  //Rotate vector radian
  SimVec axis = Normalize4(Set4(1.0f, 0.0f, 0.0f, 0.0f));
  SimMat3 mtx = BuildRotation3(axis,scalar(.5) * scalar(3.1415926535897932384626433832795));
  SimVec v = Set4(0,1,0,0);

  SimVec rotatedV = Transform(mtx,v);
  SimVec expected = Set4(0,0,1,0);

  Check_SimVec3_Close(expected,rotatedV,real(.0001));
}

//-------------------------------------------------------------- Rotate (vector) 2
TEST(SimMat3_Rotate_Vector_2)
{
  //Rotate vector radian
  SimVec axis = Normalize4(Set4(0.0f, 1.0f, 0.0f, 0.0f));
  SimMat3 mtx = BuildRotation3(axis,scalar(.5) * scalar(3.1415926535897932384626433832795));
  SimVec v = Set4(0,0,1,0);

  SimVec rotatedV = Transform(mtx,v);
  SimVec expected = Set4(1,0,0,0);

  Check_SimVec3_Close(expected,rotatedV,real(.0001));
}

//-------------------------------------------------------------- Rotate (vector) 3
TEST(SimMat3_Rotate_Vector_3)
{
  //Rotate vector radian
  SimVec axis = Normalize4(Set4(0.0f, 0.0f, 1.0f, 0.0f));
  SimMat3 mtx = BuildRotation3(axis,scalar(.5) * scalar(3.1415926535897932384626433832795));
  SimVec v = Set4(1,0,0,0);

  SimVec rotatedV = Transform(mtx,v);
  SimVec expected = Set4(0,1,0,0);

  Check_SimVec3_Close(expected,rotatedV,real(.0001));
}

//-------------------------------------------------------------- Rotate (vector) 4
TEST(SimMat3_Rotate_Vector_4)
{
  //Rotate vector radian
  SimVec axis = Normalize4(Set4(0.0f, 1.0f, 0.0f, 0.0f));
  SimMat3 mtx = BuildRotation3(axis,scalar(.25) * scalar(3.1415926535897932384626433832795));
  SimVec v = Set4(0,0,1,0);

  SimVec rotatedV = Transform(mtx,v);
  SimVec expected = Normalize3(Set4(1,0,1,0));

  Check_SimVec3_Close(expected,rotatedV,real(.0001));
}

//------------------------------------------ Build Transform (Quaternion)
TEST(SimMat3_BuildTransform3_Quaternion)
{
  SimVec scale = Set3(scalar(-7.9), scalar(0.64), scalar(27.0));
  SimVec rotation = Set4(scalar(0.082165510187), scalar(0.082165510187),
                         scalar(0.082165510187), scalar(0.989821441881));
  rotation = Normalize4(rotation);

  SimMat3 transform = BuildTransform3(rotation,scale);

  SimMat3 expected = SetMat3(-7.68666299437f,-0.095459856258f, 4.75633916083f,
                             -1.39166960632f, 0.622717002076f,-4.02721268589f,
                              1.17833260069f, 0.112742854183f, 26.2708735251f);
  Check_MatSim3_Close(expected, transform, scalar(0.00001));
}

//--------------------------------------------- Build Transform (AxisAngle)
TEST(SimMat3_BuildTransform3_AxisAngle)
{
  SimVec scale = Set3(scalar(2.3), scalar(8.3), scalar(1.4));
  SimVec axis = Set3(0,1,0);
  scalar angle = scalar(.25) * scalar(3.1415926535897932384626433832795);
  SimMat3 matrix = BuildTransform3(axis,angle, scale);

  SimMat3 expected = SetMat3( .707107f * 2.3f,     0.0f, .707107f * 1.4f,
                                         0.0f, 1 * 8.3f,            0.0f,
                             -.707107f * 2.3f,     0.0f, .707107f * 1.4f);
  Check_MatSim3_Close(expected, matrix,scalar(.00001));
}

//--------------------------------------------- Build Transform (Matrix3)
TEST(SimMat3_BuildTransform3_Matrix3)
{
  SimVec scale = Set3(scalar(2.3), scalar(8.3), scalar(1.4));
  SimMat3 rotation = SetMat3(scalar( 0.0), scalar(-1.0), scalar( 0.0),
                             scalar( 0.0), scalar( 0.0), scalar(-1.0),
                             scalar( 1.0), scalar( 0.0), scalar( 0.0));
  SimMat3 matrix = BuildTransform3(rotation, scale);
  SimMat3 expected = SetMat3(0.0f, -8.3f,  0.0f,
                             0.0f,  0.0f, -1.4f,
                             2.3f,  0.0f,  0.0f);
  Check_MatSim3_Close(expected, matrix,real(.00001));
}

//------------------------------------------------------------------- Transposed
TEST(SimMat3_Transpose3)
{
  SimMat3 mtx = SetMat3(scalar( 0.0), scalar( 1.0), scalar( 2.0),
                        scalar( 4.0), scalar( 5.0), scalar( 6.0),
                        scalar( 8.0), scalar( 9.0), scalar(10.0));

  //Transpose
  SimMat3 transpose = Transpose3(mtx);

  SimMat3 expected = SetMat3(0.0f, 4.0f,  8.0f,
                             1.0f, 5.0f,  9.0f,
                             2.0f, 6.0f, 10.0f);
  SimMat3 result = Equal(transpose,expected);
  CheckBool_SimMat3(result);
}

//------------------------------------------------------------------- Affine Inverse3
TEST(SimMat3_AffineInverse3)
{
  SimVec scale = Set3(scalar(1), scalar(1), scalar(1));
  SimVec rotation = Set4(scalar(0.082165510187), scalar(0.082165510187),
                         scalar(0.082165510187), scalar(0.989821441881));
  rotation = Normalize4(rotation);

  SimMat3 transform = BuildTransform3(rotation,scale);
  SimMat3 invTransform = AffineInverse3(transform);

  SimVec expected = Set4(1,2,3,4);
  SimVec tVec = Transform(transform,expected);
  SimVec actual = Transform(invTransform,tVec);

  Check_SimVec3_Close(expected, actual, scalar(0.00001));
}

//------------------------------------------------------------------- Affine InverseWithScale3
TEST(SimMat3_AffineInverseWithScale3)
{
  SimVec scale = Set3(scalar(7.9), scalar(0.64), scalar(27.0));
  SimVec rotation = Set4(scalar(0.082165510187), scalar(0.082165510187),
                         scalar(0.082165510187), scalar(0.989821441881));
  rotation = Normalize4(rotation);

  SimVec expected = Set4(1,2,3,4);

  SimMat3 transform = BuildTransform3(rotation,scale);
  SimVec tVec = Transform(transform,expected);
  SimMat3 invTransform = AffineInverseWithScale3(transform);
  SimVec actual = Transform(invTransform,tVec);

  Check_SimVec3_Close(expected, actual, scalar(0.00001));
}

//------------------------------------------------------------- Transform Normal
TEST(SimMat3_Transform)
{
  SimVec vec = Set4(-1,2,-3,0);
  SimMat3 mat = SetMat3( 2.0f,  3.0f,  5.0f,
                        11.0f, 13.0f, 17.0f,
                        23.0f, 29.0f, 31.0f);
  SimVec result = Transform(mat,vec);
  SimVec expected = Set4(-11,-36,-58,0);
  Check_SimVec3(expected,result);
}

//------------------------------------------------------------- Transposed Transform Normal
TEST(SimMat3_TransposedTransform)
{
  SimVec vec = Set4(-1,2,-3,0);
  SimMat3 mat = SetMat3( 2.0f,  3.0f,  5.0f,
                        11.0f, 13.0f, 17.0f,
                        23.0f, 29.0f, 31.0f);
  SimVec result = TransposeTransform(mat,vec);
  SimVec expected = Set4(-49,-64,-64,0);
  Check_SimVec3(expected,result);
}

//-------------------------------------------------------------- Matrix Addition
TEST(SimMat3_MatrixAddition)
{
  SimMat3 mtx = SetMat3(scalar( 0.0), scalar( 1.0), scalar( 2.0),
                        scalar( 4.0), scalar( 5.0), scalar( 6.0),
                        scalar( 8.0), scalar( 9.0), scalar(10.0));

  SimMat3 add = SetMat3(scalar( 2.0), scalar( 4.0), scalar( 6.0),
                        scalar(10.0), scalar(12.0), scalar(14.0),
                        scalar(18.0), scalar(20.0), scalar(22.0));

  //Addition
  SimMat3 res = Add(mtx,add);
  SimMat3 expected = SetMat3( 2.0f,  5.0f,  8.0f,
                             14.0f, 17.0f, 20.0f,
                             26.0f, 29.0f, 32.0f);

  CheckBool_SimMat3(Equal(res,expected));
}

//----------------------------------------------------------- Matrix Subtraction
TEST(SimMat3_MatrixSubtraction)
{
  SimMat3 mtx = SetMat3(scalar( 0.0), scalar( 1.0), scalar( 2.0),
                        scalar( 4.0), scalar( 5.0), scalar( 6.0),
                        scalar( 8.0), scalar( 9.0), scalar(10.0));

  SimMat3 sub = SetMat3(scalar( 2.0), scalar( 4.0), scalar( 6.0),
                        scalar(10.0), scalar(12.0), scalar(14.0),
                        scalar(18.0), scalar(20.0), scalar(22.0));

  //Subtraction
  SimMat3 res = Subtract(sub,mtx);

  SimMat3 expected = SetMat3( 2.0f,  3.0f,  4.0f,
                              6.0f,  7.0f,  8.0f,
                             10.0f, 11.0f, 12.0f);
  CheckBool_SimMat3(Equal(res,expected));
}


//-------------------------------------------------------------- Matrix ComponentScale
TEST(SimMat3_MatrixComponentScale)
{
  SimMat3 mtx = SetMat3(scalar( 0.0), scalar( 1.0), scalar( 2.0),
                          scalar( 4.0), scalar( 5.0), scalar( 6.0),
                          scalar( 8.0), scalar( 9.0), scalar(10.0));

  SimMat3 mul = SetMat3(scalar( 2.0), scalar( 3.0), scalar( 6.0),
                          scalar(12.0), scalar(15.0), scalar(18.0),
                          scalar(24.0), scalar(27.0), scalar(30.0));

  //Multiplication
  SimMat3 res = ComponentScale(mtx,mul);
  SimMat3 expected = SetMat3(  0.0f,  3.0f, 12.0f,
                                48.0f, 75.0f,108.0f,
                               192.0f,243.0f,300.0f);

  CheckBool_SimMat3(Equal(res,expected));
}

//-------------------------------------------------------------- Matrix ComponentDivide
TEST(SimMat3_MatrixComponentDivide)
{
  SimMat3 expected = SetMat3(scalar( 0.0), scalar( 1.0), scalar( 2.0),
                             scalar( 4.0), scalar( 5.0), scalar( 6.0),
                             scalar( 8.0), scalar( 9.0), scalar(10.0));

  SimMat3 mtx = SetMat3(  0.0f,  3.0f, 12.0f,
                         48.0f, 75.0f,108.0f,
                         192.0f,243.0f,300.0f);

  SimMat3 div = SetMat3(scalar( 2.0), scalar( 3.0), scalar( 6.0),
                        scalar(12.0), scalar(15.0), scalar(18.0),
                        scalar(24.0), scalar(27.0), scalar(30.0));

  //Division
  SimMat3 res = ComponentDivide(mtx,div);


  CheckBool_SimMat3(Equal(res,expected));
}


//------------------------------------------------------ Matrix Multiplication 1
TEST(SimMat3_MatrixMultiplication_1)
{
  SimMat3 mtx = SetMat3(scalar( 0.0), scalar( 1.0), scalar( 2.0),
                        scalar( 4.0), scalar( 5.0), scalar( 6.0),
                        scalar( 8.0), scalar( 9.0), scalar(10.0));

  SimMat3 add = SetMat3(scalar( 2.0), scalar( 4.0), scalar( 6.0),
                        scalar(10.0), scalar(12.0), scalar(14.0),
                        scalar(18.0), scalar(20.0), scalar(22.0));

  //Multiplication
  SimMat3 res = Multiply(add,mtx);

  SimMat3 expected = SetMat3( 64.0f,  76.0f,  88.0f,
                             160.0f, 196.0f, 232.0f,
                             256.0f, 316.0f, 376.0f);
  CheckBool_SimMat3(Equal(res,expected));
}

//------------------------------------------------------ Matrix Multiplication 2
TEST(SimMat3_MatrixMultiplication_2)
{
  SimMat3 mtx = SetMat3(scalar( 0.0), scalar( 1.0), scalar( 2.0),
                        scalar( 4.0), scalar( 5.0), scalar( 6.0),
                        scalar( 8.0), scalar( 9.0), scalar(10.0));

  SimMat3 add = SetMat3(scalar( 2.0), scalar( 4.0), scalar( 6.0),
                        scalar(10.0), scalar(12.0), scalar(14.0),
                        scalar(18.0), scalar(20.0), scalar(22.0));
  SimMat3 res = Multiply(mtx,add);

  SimMat3 expected = SetMat3( 46.0f,  52.0f,   58.0f,
                             166.0f, 196.0f,  226.0f,
                             286.0f, 340.0f,  394.0f);
  CheckBool_SimMat3(Equal(res,expected));
}


//--------------------------------------------------------------- Set Identity
TEST(SimMat3_SetIdentity)
{
  SimMat3 mtx = LoadMat3(init3);
  mtx = IdentityMat3();

  SimMat3 expected = SetMat3(1.0f, 0.0f, 0.0f,
                             0.0f, 1.0f, 0.0f,
                             0.0f, 0.0f, 1.0f);
  CheckBool_SimMat3(Equal(expected, mtx));
}

//------------------------------------------------------------------- Zero Out
TEST(SimMat3_ZeroOut)
{
  SimMat3 mtx = LoadMat3(init3);
  mtx = ZeroOutMat3();

  SimMat3 expected = SetMat3(0.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.0f);
  CheckBool_SimMat3(Equal(expected, mtx));
}

//---------------------------------------------------------------------- Basis X
TEST(SimMat3_BasisX)
{
  SimMat3 mtx = IdentityMat3();
  SimVec basisX = BasisX(mtx);
  SimVec expected = Set4(scalar(1.0), scalar(0.0), scalar(0.0), scalar(0.0));
  Check_SimVec4(expected, basisX);
}

//---------------------------------------------------------------------- Basis Y
TEST(SimMat3_BasisY)
{
  SimMat3 mtx = IdentityMat3();
  SimVec basisY = BasisY(mtx);
  SimVec expected = Set4(scalar(0.0), scalar(1.0), scalar(0.0), scalar(0.0));
  Check_SimVec4(expected, basisY);
}

//---------------------------------------------------------------------- Basis Z
TEST(SimMat3_BasisZ)
{
  SimMat3 mtx = IdentityMat3();
  SimVec basisZ = BasisZ(mtx);
  SimVec expected = Set4(scalar(0.0), scalar(0.0), scalar(1.0), scalar(0.0));
  Check_SimVec4(expected, basisZ);
}

