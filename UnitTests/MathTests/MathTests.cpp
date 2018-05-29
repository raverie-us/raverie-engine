///////////////////////////////////////////////////////////////////////////////
///
///  \file MathTests.cpp
///  Unit tests for the general math functions.
///  
///  Authors: Benjamin Strukus
///  Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "CppUnitLite2/CppUnitLite2.h"
typedef unsigned int uint;
#include "Math/Reals.hpp"
#include "Math/Math.hpp"

#include <float.h>

typedef Math::real        real;
typedef Math::Matrix3     Mat3;
typedef Math::Matrix4     Mat4;
typedef Math::Vector3     Vec3;
typedef Math::Vector4     Vec4;
typedef Math::Quaternion  Quat;
typedef Math::EulerAngles EulerAngles;

namespace
{
//Setting the values of a matrix every time is really irritating, don't change!
const real init3[9]  = {  2.0f,  3.0f,  5.0f,
                          7.0f, 11.0f, 13.0f,
                         17.0f, 19.0f, 23.0f };
const real init4[16] = {  2.0f,  3.0f,  5.0f,  7.0f,
                         11.0f, 13.0f, 17.0f, 19.0f,
                         23.0f, 29.0f, 31.0f, 37.0f,
                         41.0f, 43.0f, 47.0f, 53.0f };
}

//------------------------------------------------------------- HELPER FUNCTIONS
TEST(MathRealsEpsilon)
{
  CHECK(real(1.0) > Math::Epsilon());
  CHECK(real(0.0) < Math::Epsilon());
}

TEST(MathRealsPositiveMax)
{
  CHECK_EQUAL(FLT_MAX, Math::PositiveMax());
}

TEST(MathRealsEqual)
{
  CHECK(Math::Equal(real(5.0), real(5.0)));
  CHECK(Math::Equal(real(6.0) - Math::Epsilon(), real(6.0)));
  CHECK(Math::Equal(real(0.0), real(-0.0)));
  CHECK(Math::Equal(Math::Epsilon(), -Math::Epsilon()) == false);
}

TEST(MathRealsNotEqual)
{
  CHECK(Math::NotEqual(real(5.0), real(5.0)) == false);
  CHECK(Math::NotEqual(real(6.0) - Math::Epsilon(), real(6.0)) == false);
  CHECK(Math::NotEqual(real(0.0), real(-0.0)) == false);
  CHECK(Math::NotEqual(Math::Epsilon(), -Math::Epsilon()) == true);
}

TEST(MathRealsIsZero)
{
  CHECK(Math::IsZero(real(0.0)));
  CHECK(Math::IsZero(real(FLT_MIN)));
  CHECK(Math::IsZero(Math::Epsilon()));
}

TEST(MathRealsLessThan)
{
  CHECK(Math::LessThan(5.0f, 6.0f));
//  CHECK(Math::LessThan(5.0f, 5.0f + Math::Epsilon()));
  CHECK(Math::LessThan(1.0f, -1.0f) == false);
}

TEST(MathRealsLessThanOrEqual)
{
  CHECK(Math::LessThanOrEqual(5.0f, 10.0f));
  CHECK(Math::LessThanOrEqual(6.0f, 6.0f));
  CHECK(Math::LessThanOrEqual(3.0f, 2.9f) == false);
}

TEST(MathRealsGreaterThan)
{
  CHECK(Math::GreaterThan(6.0f, 5.0f));
//  CHECK(Math::GreaterThan(5.0f + Math::Epsilon(), 5.0f));
  CHECK(Math::GreaterThan(-1.0f, 1.0f) == false);
}

TEST(MathRealsGreaterThanOrEqual)
{
  CHECK(Math::GreaterThanOrEqual(15.0f, 10.0f));
  CHECK(Math::GreaterThanOrEqual(6.0f, 6.0f));
  CHECK(Math::GreaterThanOrEqual(2.9f, 3.0f) == false);
}

TEST(MathRealsSqrt)
{
  CHECK_EQUAL(real(1.4142135623730950488016887242097), Math::Sqrt(2));
  CHECK_CLOSE(real(3.1415926535897932384626433832795), 
              Math::Sqrt(real(9.8696044010893586188344909998762)),
              real(0.000001));
  CHECK_EQUAL(real(333.0), Math::Sqrt(real(110889.0)));
}

TEST(MathRealsRsqrt)
{
  CHECK_EQUAL(real(0.33333333333333333333333333333333), Math::Rsqrt(9.0));
  CHECK_EQUAL(real(0.25819888974716112567861769331883), Math::Rsqrt(15.0));
  CHECK_EQUAL(real(0.70710678118654752440084436210485), Math::Rsqrt(2.0));
}

TEST(MathRealsAbs)
{
  CHECK_EQUAL(real(1.0), Math::Abs(real(-1.0)));
  CHECK(Math::Abs(real(-40.0)) != real(-40.0));
}

TEST(MathRealsGetSign)
{
  CHECK_EQUAL(real(-1.0), Math::GetSign(real(-1.3)));
  CHECK_EQUAL(real( 1.0), Math::GetSign(real( 2.92)));
}

TEST(MathRealsCos)
{
  CHECK_EQUAL(real(0.98078528040323044912618223613424),
              Math::Cos(Math::cPi / real(16.0)));
  CHECK_EQUAL(real(0.90096886790241912623610231950745),
              Math::Cos(Math::cPi / real(7.0)));
  CHECK_CLOSE(real(0.0), Math::Cos(Math::cPi / real(2.0)), real(0.000001));
  CHECK_EQUAL(real(-1.0), Math::Cos(Math::cPi));
}

TEST(MathRealsSin)
{
  CHECK_CLOSE(real(0.19509032201612826784828486847702), 
              Math::Sin(Math::cPi / real(16.0)), real(0.000001));
  CHECK_CLOSE(real(0.43388373911755812047576833284836),
              Math::Sin(Math::cPi / real(7.0)), real(0.000001));
  CHECK_CLOSE(real(1.0), Math::Sin(Math::cPi / real(2.0)), real(0.000001));
  CHECK_CLOSE(real(0.0), Math::Sin(Math::cPi), real(0.000001));
}

TEST(MathRealsTan)
{
  CHECK_CLOSE(real(0.19891236737965800691159762264468),
              Math::Tan(Math::cPi / real(16.0)), real(0.000001));
  CHECK_CLOSE(real(0.48157461880752864433216235305697), 
              Math::Tan(Math::cPi / real(7.0)), real(0.000001));
  CHECK_CLOSE(real(-12.068205279497753298963981799486), 
              Math::Tan(Math::cPi / real(1.9)), real(0.0001));
  CHECK_CLOSE(real(0.0), Math::Tan(Math::cPi), real(0.000001));
}

TEST(MathRealsArcCos)
{
  CHECK_EQUAL(real(1.0471975511965977461542144610932), 
              Math::ArcCos(real(0.5)));
  CHECK_EQUAL(real(1.1053754499566336297943591098505), 
              Math::ArcCos(Math::cPi / real(7.0)));
  CHECK_CLOSE(real(0.52057988589621129600857943138965), 
              Math::ArcCos(real(0.8675309)), real(0.000001));
  CHECK_EQUAL(real(1.4470245494505612999088642650954), 
              Math::ArcCos(real(0.123456)));
}

TEST(MathRealsArcSin)
{
  CHECK_EQUAL(real(0.52359877559829887307710723054658), 
              Math::ArcSin(real(0.5)));
  CHECK_CLOSE(real(0.46542087683826298943696258178922), 
              Math::ArcSin(Math::cPi / real(7.0)), real(0.000001));
  CHECK_CLOSE(real(1.0502164408986853232227422602501), 
              Math::ArcSin(real(0.8675309)), real(0.000001));
  CHECK_EQUAL(real(0.12377177734433531932245742654434), 
              Math::ArcSin(real(0.123456)));
}

TEST(MathRealsArcTan)
{
  CHECK_EQUAL(real(0.46364760900080611621425623146121), 
              Math::ArcTan(real(0.5)));
  CHECK_CLOSE(real(0.42185468359630030003054701085192), 
              Math::ArcTan(Math::cPi / real(7.0)), real(0.000001));
  CHECK_CLOSE(real(0.7145840218270921185852331228804), 
              Math::ArcTan(real(0.8675309)), real(0.000001));
  CHECK_EQUAL(real(0.12283446061629792519644740907172), 
              Math::ArcTan(real(0.123456)));
}

TEST(MathRealsArcTan2)
{
  real y = real(5.0);
  real x = real(7.0);
  CHECK_EQUAL(Math::ArcTan(y / x), Math::ArcTan2(y, x));
  
  x = real(-12.0);
  CHECK_EQUAL(Math::cPi + Math::ArcTan(y / x), Math::ArcTan2(y, x));

  y = real(-15.0);
  CHECK_EQUAL(-Math::cPi + Math::ArcTan(y / x), Math::ArcTan2(y, x));

  x = real(0.0);
  y = real(20.0);
  CHECK_EQUAL(Math::cPi / real(2.0), Math::ArcTan2(y, x));

  y = real(-13.0);
  CHECK_EQUAL(-Math::cPi / real(2.0), Math::ArcTan2(y, x));
}

TEST(MathRealsRadToDeg)
{
  CHECK_EQUAL(real(180.0), Math::RadToDeg(Math::cPi));
  CHECK_EQUAL(real(90.0), Math::RadToDeg(Math::cPi / real(2.0)));
  CHECK_EQUAL(real(360.0), Math::RadToDeg(real(2.0) * Math::cPi));
}

TEST(MathRealsDegToRad)
{
  CHECK_EQUAL(Math::cPi, Math::DegToRad(real(180.0)));
  CHECK_EQUAL(Math::cPi / real(2.0), Math::DegToRad(real(90.0)));
  CHECK_EQUAL(real(2.0) * Math::cPi, Math::DegToRad(real(360.0)));
}

TEST(MathRealsIsValid)
{
  real a = real(1.0);
  real b = real(0.0);
  CHECK(Math::IsValid(a / b) == false);
  CHECK(Math::IsValid(b / a));
}

TEST(MathRealsMax)
{
  CHECK_EQUAL(real(5.0), Math::Max(real(1.0), real(5.0)));
  CHECK_EQUAL(real(-5.0), Math::Max(real(-10.0), real(-5.0)));
  CHECK_EQUAL(Math::Epsilon(), Math::Max(-Math::Epsilon(), Math::Epsilon()));
}

TEST(MathRealsMin)
{
  CHECK_EQUAL(real(1.0), Math::Min(real(1.0), real(5.0)));
  CHECK_EQUAL(real(-10.0), Math::Min(real(-10.0), real(-5.0)));
  CHECK_EQUAL(-Math::Epsilon(), Math::Min(-Math::Epsilon(), Math::Epsilon()));
}

TEST(MathRealsClamp)
{
  CHECK_EQUAL(real(20.0), Math::Clamp(real(50.0), real(10.0), real(20.0)));
  CHECK_EQUAL(real(0.0), Math::Clamp(real(-5.0), real(0.0), real(FLT_MAX)));
}

TEST(MathRealsInRange)
{
  CHECK(Math::InRange(real(0.0), real(10.0), real(11.0)) == false);
  CHECK(Math::InRange(real(16.0), real(0.0), real(17.0)));
  CHECK(Math::InRange(real(5.0), real(5.0), real(6.0)));
  CHECK(Math::InRange(real(12.0), real(11.0), real(12.0)));
}

TEST(MathRealsInBounds)
{
  CHECK(Math::InBounds(real(0.0), real(10.0), real(11.0)) == false);
  CHECK(Math::InBounds(real(16.0), real(0.0), real(17.0)));
  CHECK(Math::InBounds(real(5.0), real(5.0), real(6.0)) == false);
  CHECK(Math::InBounds(real(12.0), real(11.0), real(12.0)) == false);
}

TEST(MathRealsWrap)
{
  CHECK_EQUAL(real(1.0), Math::Wrap(real(-1.0), real(0.0), real(2.0)));
  CHECK_EQUAL(real(5.0), Math::Wrap(real(10.0), real(0.0), real(5.0)));
}

TEST(MathRealsSwap)
{
  real five = real(5.0);
  real six  = real(6.0);
  Math::Swap(five, six);
  CHECK_EQUAL(real(6.0), five);
  CHECK_EQUAL(real(5.0), six);
}

//--------------------------------------------------------- CONVERSION FUNCTIONS
TEST(SkewSymmetric)
{
  Vec3 vec(1.0f, 2.0f, 3.0f);
  Mat3 mtx = SkewSymmetric(vec);
  CHECK_EQUAL(real( 0.0), mtx.m00);
  CHECK_EQUAL(real(-3.0), mtx.m01);
  CHECK_EQUAL(real( 2.0), mtx.m02);

  CHECK_EQUAL(real( 3.0), mtx.m10);
  CHECK_EQUAL(real( 0.0), mtx.m11);
  CHECK_EQUAL(real(-1.0), mtx.m12);

  CHECK_EQUAL(real(-2.0), mtx.m20);
  CHECK_EQUAL(real( 1.0), mtx.m21);
  CHECK_EQUAL(real( 0.0), mtx.m22);
}

TEST(Matrix3ToMatrix4)
{
  Mat3 mtx3(init3);
  Mat4 mtx4(init4);
  ToMatrix4(mtx3, &mtx4);

  CHECK_EQUAL(real( 2.0), mtx4.m00);
  CHECK_EQUAL(real( 3.0), mtx4.m01);
  CHECK_EQUAL(real( 5.0), mtx4.m02);
  CHECK_EQUAL(real( 0.0), mtx4.m03);

  CHECK_EQUAL(real( 7.0), mtx4.m10);
  CHECK_EQUAL(real(11.0), mtx4.m11);
  CHECK_EQUAL(real(13.0), mtx4.m12);
  CHECK_EQUAL(real( 0.0), mtx4.m13);

  CHECK_EQUAL(real(17.0), mtx4.m20);
  CHECK_EQUAL(real(19.0), mtx4.m21);
  CHECK_EQUAL(real(23.0), mtx4.m22);
  CHECK_EQUAL(real( 0.0), mtx4.m23);

  CHECK_EQUAL(real( 0.0), mtx4.m30);
  CHECK_EQUAL(real( 0.0), mtx4.m31);
  CHECK_EQUAL(real( 0.0), mtx4.m32);
  CHECK_EQUAL(real( 1.0), mtx4.m33);
}

TEST(QuaternionToEulerAngles)
{
  EulerAngles euler(Vec3::cZero, Math::EulerOrders::XYZs);
  Quat quat(real(0.3826834323650897717284599840304), real(0.0), real(0.0),
            real(0.92387953251128675612818318939679));
  ToEulerAngles(quat, &euler);
  real x = Math::RadToDeg(euler[Math::cX]);
  CHECK_CLOSE(real(45.0), Math::RadToDeg(euler[Math::cX]), real(0.00001));
  CHECK_CLOSE(real(0.0), euler[Math::cY], real(0.000001));
  CHECK_CLOSE(real(0.0), euler[Math::cZ], real(0.000001));
}

TEST(EulerAnglesToQuaternion)
{
  Vec3 angles(Math::DegToRad(real(45.0)), real(0.0), real(0.0));
  EulerAngles euler(angles, Math::EulerOrders::XYZs);
  Quat quat;
  ToQuaternion(euler, &quat);
  CHECK_CLOSE(real(0.3826834323650897717284599840304), quat.x, real(0.000001));
  CHECK_CLOSE(real(0.0), quat.y, real(0.000001));
  CHECK_CLOSE(real(0.0), quat.z, real(0.000001));
  CHECK_CLOSE(real(0.92387953251128675612818318939679), quat.w, real(0.000001));
}

TEST(QuaternionToAxisAngle)
{
  Quat quat(0.0f, 0.0f, 0.0f, 1.0f);
  Vec4 axisAngle;
  ToAxisAngle(quat, &axisAngle);
  CHECK_CLOSE(real(0.0), axisAngle.x, real(0.000001));
  CHECK_CLOSE(real(0.0), axisAngle.y, real(0.000001));
  CHECK_CLOSE(real(0.0), axisAngle.z, real(0.000001));
  CHECK_CLOSE(real(0.0), axisAngle.w, real(0.000001));

  quat.x = real(1.0);
  quat.w = real(0.0);
  ToAxisAngle(quat, &axisAngle);
  CHECK_CLOSE(real(1.0), axisAngle.x, real(0.000001));
  CHECK_CLOSE(real(0.0), axisAngle.y, real(0.000001));
  CHECK_CLOSE(real(0.0), axisAngle.z, real(0.000001));
  CHECK_CLOSE(real(Math::cPi), axisAngle.w, real(0.000001));

  quat.x = real(0.0);
  quat.y = real(0.70710678118654752440084436210485);
  quat.z = real(0.70710678118654752440084436210485);
  quat.w = real(-0.25881904510252076234889883762405);
  ToAxisAngle(quat, &axisAngle);
  CHECK_CLOSE(real(0.0), axisAngle.x, real(0.000001));
  CHECK_CLOSE(real(0.70710678118654752440084436210485), axisAngle.y, real(0.000001));
  CHECK_CLOSE(real(0.70710678118654752440084436210485), axisAngle.z, real(0.000001));
  CHECK_CLOSE(real(7.0 / 6.0) * Math::cPi, axisAngle.w, real(0.1));
}

TEST(EulerAnglesToMatrix3)
{
  Mat3 matrix;
  EulerAngles euler(Vec3(Math::DegToRad(real(45.0)), real(0.0), real(0.0)), 
                    Math::EulerOrders::XYZs);
  ToMatrix3(euler, &matrix);
  CHECK_CLOSE(real(1.0), matrix.m00, real(0.000001));
  CHECK_CLOSE(real(0.0), matrix.m01, real(0.000001));
  CHECK_CLOSE(real(0.0), matrix.m02, real(0.000001));
  CHECK_CLOSE(real(0.0), matrix.m10, real(0.000001));
  CHECK_CLOSE(real( 0.70710677), matrix.m11, real(0.000001));
  CHECK_CLOSE(real(-0.70710677), matrix.m12, real(0.000001));
  CHECK_CLOSE(real( 0.0), matrix.m20, real(0.000001));
  CHECK_CLOSE(real( 0.70710677), matrix.m21, real(0.000001));
  CHECK_CLOSE(real( 0.70710677), matrix.m22, real(0.000001));
}

TEST(Matrix3ToEulerAngles_1)
{
  Mat3 matrix;
  matrix.Rotate(Vec3::cXAxis, Math::DegToRad(real(45.0)));
  EulerAngles euler(Vec3::cZero, Math::EulerOrders::XYZs);
  ToEulerAngles(matrix, &euler);
  CHECK_CLOSE(real(45.0), Math::RadToDeg(euler[Math::cX]), real(0.000001));
  CHECK_CLOSE(real(0.0), euler[Math::cY], real(0.000001));
  CHECK_CLOSE(real(0.0), euler[Math::cZ], real(0.000001));
}

TEST(Matrix3ToEulerAngles_2)
{
  real root2 = Math::Sqrt(2.0);

  Mat3 matrix;
  real scales[3];
  Vec3 bases[3] = { Vec3(1, 0, 0), 
                    Vec3(0, root2, root2),
                    Vec3(0, -root2, root2), };

  for(int i = 0 ; i < 3; ++i)
  {
    scales[i] = Normalize(bases[i]);
    matrix.SetBasis(i, bases[i]);
  }
  EulerAngles euler(Vec3::cZero, Math::EulerOrders::XYZs);
  ToEulerAngles(matrix, &euler);

  CHECK_CLOSE(real(45.0), Math::RadToDeg(euler[Math::cX]), real(0.000001));
  CHECK_CLOSE(real(0.0), euler[Math::cY], real(0.000001));
  CHECK_CLOSE(real(0.0), euler[Math::cZ], real(0.000001));
}

TEST(AxisAngleToQuaternion)
{
  Vec4 axisAngle(real(0.57735026918962576450914878050196),
                 real(0.57735026918962576450914878050196),
                 real(0.57735026918962576450914878050196),
                 Math::cPi);
  Quat quat;
  ToQuaternion(axisAngle, &quat);
  CHECK_CLOSE(real(0.57735026918962576450914878050196), quat.x, real(0.000001));
  CHECK_CLOSE(real(0.57735026918962576450914878050196), quat.y, real(0.000001));
  CHECK_CLOSE(real(0.57735026918962576450914878050196), quat.z, real(0.000001));
  CHECK_CLOSE(real(0.0), quat.w, real(0.000001));

  axisAngle.w = Math::cPi / real(6.0);
  ToQuaternion(axisAngle, &quat);
  CHECK_CLOSE(real(0.14942924536134225401731517482693), quat.x, real(0.000001));
  CHECK_CLOSE(real(0.14942924536134225401731517482693), quat.y, real(0.000001));
  CHECK_CLOSE(real(0.14942924536134225401731517482693), quat.z, real(0.000001));
  CHECK_CLOSE(real(0.9659258262890682867497431997289), quat.w, real(0.000001));

  axisAngle.Set(real(0.0), real(0.0), real(0.0), real(0.0));
  ToQuaternion(axisAngle, &quat);
  CHECK_CLOSE(real(0.0), quat.x, real(0.000001));
  CHECK_CLOSE(real(0.0), quat.y, real(0.000001));
  CHECK_CLOSE(real(0.0), quat.z, real(0.000001));
  CHECK_CLOSE(real(1.0), quat.w, real(0.000001));

  axisAngle.Set(real(0.0), real(0.70710678118654752440084436210485),
                real(0.70710678118654752440084436210485), 
                real(7.0 / 6.0) * Math::cPi);
  ToQuaternion(axisAngle, &quat);
  CHECK_CLOSE(real(0.0), quat.x, real(0.000001));
  CHECK_CLOSE(real( 0.68301270189221932338186158537647), quat.y, real(0.000001));
  CHECK_CLOSE(real( 0.68301270189221932338186158537647), quat.z, real(0.000001));
  CHECK_CLOSE(real(-0.25881904510252076234889883762405), quat.w, real(0.000001));
}

TEST(QuaternionToMatrix3)
{
  Quat quat(1.0f, 0.0f, 0.0f, 0.0f);
  Mat3 mtx;
  ToMatrix3(quat, &mtx);
  CHECK_CLOSE(real( 1.0), mtx.m00, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m01, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m02, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m10, real(0.000001));
  CHECK_CLOSE(real(-1.0), mtx.m11, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m12, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m20, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m21, real(0.000001));
  CHECK_CLOSE(real(-1.0), mtx.m22, real(0.000001));

  //Rotation about the xy-axis by 7/6 pi radians
  quat.x = real(0.0);
  quat.y = real(0.68301270189221932338186158537647);
  quat.z = real(0.68301270189221932338186158537647);
  quat.w = real(-0.25881904510252076234889883762405);
  ToMatrix3(quat, &mtx);
  CHECK_CLOSE(real(-0.86602540378443864676372317075294), mtx.m00, real(0.000001));
  CHECK_CLOSE(real( 0.35355339059327376220042218105242), mtx.m01, real(0.000001));
  CHECK_CLOSE(real(-0.35355339059327376220042218105242), mtx.m02, real(0.000001));
  CHECK_CLOSE(real(-0.35355339059327376220042218105242), mtx.m10, real(0.000001));
  CHECK_CLOSE(real( 0.06698729810778067661813841462353), mtx.m11, real(0.000001));
  CHECK_CLOSE(real( 0.93301270189221932338186158537647), mtx.m12, real(0.000001));
  CHECK_CLOSE(real( 0.35355339059327376220042218105242), mtx.m20, real(0.000001));
  CHECK_CLOSE(real( 0.93301270189221932338186158537647), mtx.m21, real(0.000001));
  CHECK_CLOSE(real( 0.06698729810778067661813841462353), mtx.m22, real(0.000001));
}

TEST(Matrix3ToQuaternion)
{
  Quat quat(1.0f, 0.0f, 0.0f, 0.0f);
  Mat3 mtx;
  mtx.m00 = real(1.0 / 17.0) - real((8.0 * Math::Sqrt(3.0)) / 17.0);
  mtx.m01 = real((2.0 * (Math::Sqrt(3.0) + 2.0)) / 17.0);
  mtx.m02 = real((2.0 * Math::Sqrt(17.0)) / 17.0);
  mtx.m10 = real((2.0 * (Math::Sqrt(3.0) + 2.0)) / 17.0);
  mtx.m11 = real(16.0 / 17.0) - real(Math::Sqrt(3.0) / 34.0);
  mtx.m12 = real(-Math::Sqrt(17.0) / 34.0);
  mtx.m20 = real((-2.0 * Math::Sqrt(17.0)) / 17.0);
  mtx.m21 = real(Math::Sqrt(17.0) / 34.0);
  mtx.m22 = real(-Math::Sqrt(3.0) / 2.0);
  quat.y = quat.z = quat.w = quat.x = real(0.0);
  ToQuaternion(mtx, &quat);
  Normalize(quat);
  CHECK_CLOSE(real(0.23427142401775556506231674697964), quat.x, real(0.000001));
  CHECK_CLOSE(real(0.93708569607102226024926698791854), quat.y, real(0.000001));
  CHECK_CLOSE(real(0.0), quat.z, real(0.000001));
  CHECK_CLOSE(real(0.25881904510252076234889883762405), quat.w, real(0.000001));


  mtx.m00 = real(-0.86602540378443864676372317075294);
  mtx.m01 = real( 0.35355339059327376220042218105242);
  mtx.m02 = real(-0.35355339059327376220042218105242);
  mtx.m10 = real(-0.35355339059327376220042218105242);
  mtx.m11 = real( 0.06698729810778067661813841462353);
  mtx.m12 = real( 0.93301270189221932338186158537647);
  mtx.m20 = real( 0.35355339059327376220042218105242);
  mtx.m21 = real( 0.93301270189221932338186158537647);
  mtx.m22 = real( 0.06698729810778067661813841462353);

  quat.y = quat.z = quat.w = real(0.0);
  ToQuaternion(mtx, &quat);
  CHECK_CLOSE(real(0.0), quat.x, real(0.000001));
  CHECK_CLOSE(real( 0.68301270189221932338186158537647), quat.y, real(0.000001));
  CHECK_CLOSE(real( 0.68301270189221932338186158537647), quat.z, real(0.000001));
  CHECK_CLOSE(real(-0.25881904510252076234889883762405), quat.w, real(0.000001));

  mtx.Rotate(1.0f, 0.0f, 0.0f, Math::cPi);
  ToQuaternion(mtx, &quat);
  CHECK_EQUAL(real(1.0), quat.x);
  CHECK_EQUAL(real(0.0), quat.y);
  CHECK_EQUAL(real(0.0), quat.z);
  CHECK_CLOSE(real(0.0), quat.w, real(0.000001));

  //Assigned to a Matrix3
  mtx.Rotate(real(0.57735026918962576450914878050195),
             real(0.57735026918962576450914878050195),
             real(0.57735026918962576450914878050195),
             Math::cPi * real(0.5));
  ToQuaternion(mtx, &quat);
  CHECK_CLOSE(real(0.40824829046386301636621401245098), quat.x, real(0.000001));
  CHECK_CLOSE(real(0.40824829046386301636621401245098), quat.y, real(0.000001));
  CHECK_CLOSE(real(0.40824829046386301636621401245098), quat.z, real(0.000001));
  CHECK_CLOSE(real(0.70710678118654752440084436210485), quat.w, real(0.000001));
}

TEST(QuaternionToMatrix4)
{
  Quat quat(1.0f, 0.0f, 0.0f, 0.0f);
  Mat4 mtx;
  ToMatrix4(quat, &mtx);
  CHECK_CLOSE(real( 1.0), mtx.m00, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m01, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m02, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m03, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m10, real(0.000001));
  CHECK_CLOSE(real(-1.0), mtx.m11, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m12, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m13, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m20, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m21, real(0.000001));
  CHECK_CLOSE(real(-1.0), mtx.m22, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m23, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m30, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m31, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m32, real(0.000001));
  CHECK_CLOSE(real( 1.0), mtx.m33, real(0.000001));

  //Rotation about the xy-axis by 7/6 pi radians
  quat.x = real( 0.0);
  quat.y = real( 0.68301270189221932338186158537647);
  quat.z = real( 0.68301270189221932338186158537647);
  quat.w = real(-0.25881904510252076234889883762405);
  ToMatrix4(quat, &mtx);
  CHECK_CLOSE(real(-0.86602540378443864676372317075294), mtx.m00, real(0.000001));
  CHECK_CLOSE(real( 0.35355339059327376220042218105242), mtx.m01, real(0.000001));
  CHECK_CLOSE(real(-0.35355339059327376220042218105242), mtx.m02, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m03, real(0.000001));
  CHECK_CLOSE(real(-0.35355339059327376220042218105242), mtx.m10, real(0.000001));
  CHECK_CLOSE(real( 0.06698729810778067661813841462353), mtx.m11, real(0.000001));
  CHECK_CLOSE(real( 0.93301270189221932338186158537647), mtx.m12, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m13, real(0.000001));
  CHECK_CLOSE(real( 0.35355339059327376220042218105242), mtx.m20, real(0.000001));
  CHECK_CLOSE(real( 0.93301270189221932338186158537647), mtx.m21, real(0.000001));
  CHECK_CLOSE(real( 0.06698729810778067661813841462353), mtx.m22, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m23, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m30, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m31, real(0.000001));
  CHECK_CLOSE(real( 0.0), mtx.m32, real(0.000001));
  CHECK_CLOSE(real( 1.0), mtx.m33, real(0.000001));
}

TEST(Matrix4ToQuaternion)
{
  Quat quat(1.0f, 0.0f, 0.0f, 0.0f);
  Mat4 mtx;
  mtx.m00 = real(1.0 / 17.0) - real((8.0 * Math::Sqrt(3.0)) / 17.0);
  mtx.m01 = real((2.0 * (Math::Sqrt(3.0) + 2.0)) / 17.0);
  mtx.m02 = real((2.0 * Math::Sqrt(17.0)) / 17.0);
  mtx.m03 = real(0.0);
  mtx.m10 = real((2.0 * (Math::Sqrt(3.0) + 2.0)) / 17.0);
  mtx.m11 = real(16.0 / 17.0) - real(Math::Sqrt(3.0) / 34.0);
  mtx.m12 = real(-Math::Sqrt(17.0) / 34.0);
  mtx.m13 = real(0.0);
  mtx.m20 = real((-2.0 * Math::Sqrt(17.0)) / 17.0);
  mtx.m21 = real(Math::Sqrt(17.0) / 34.0);
  mtx.m22 = real(-Math::Sqrt(3.0) / 2.0);
  mtx.m23 = real(0.0);
  mtx.m30 = real(0.0);
  mtx.m31 = real(0.0);
  mtx.m32 = real(0.0);
  mtx.m33 = real(1.0);
  quat.y = quat.z = quat.w = quat.x = real(0.0);
  ToQuaternion(mtx, &quat);
  Normalize(quat);
  CHECK_CLOSE(real(0.23427142401775556506231674697964), quat.x, real(0.000001));
  CHECK_CLOSE(real(0.93708569607102226024926698791854), quat.y, real(0.000001));
  CHECK_CLOSE(real(0.0), quat.z, real(0.000001));
  CHECK_CLOSE(real(0.25881904510252076234889883762405), quat.w, real(0.000001));


  mtx.m00 = real(-0.86602540378443864676372317075294);
  mtx.m01 = real( 0.35355339059327376220042218105242);
  mtx.m02 = real(-0.35355339059327376220042218105242);
  mtx.m10 = real(-0.35355339059327376220042218105242);
  mtx.m11 = real( 0.06698729810778067661813841462353);
  mtx.m12 = real( 0.93301270189221932338186158537647);
  mtx.m20 = real( 0.35355339059327376220042218105242);
  mtx.m21 = real( 0.93301270189221932338186158537647);
  mtx.m22 = real( 0.06698729810778067661813841462353);

  quat.y = quat.z = quat.w = real(0.0);
  ToQuaternion(mtx, &quat);
  CHECK_CLOSE(real(0.0), quat.x, real(0.000001));
  CHECK_CLOSE(real( 0.68301270189221932338186158537647), quat.y, real(0.000001));
  CHECK_CLOSE(real( 0.68301270189221932338186158537647), quat.z, real(0.000001));
  CHECK_CLOSE(real(-0.25881904510252076234889883762405), quat.w, real(0.000001));

  mtx.Rotate(1.0f, 0.0f, 0.0f, Math::cPi);
  ToQuaternion(mtx, &quat);
  CHECK_EQUAL(real(1.0), quat.x);
  CHECK_EQUAL(real(0.0), quat.y);
  CHECK_EQUAL(real(0.0), quat.z);
  CHECK_CLOSE(real(0.0), quat.w, real(0.000001));

  //Assigned to a Matrix3
  mtx.Rotate(real(0.57735026918962576450914878050195),
             real(0.57735026918962576450914878050195),
             real(0.57735026918962576450914878050195),
             Math::cPi * real(0.5));
  ToQuaternion(mtx, &quat);
  CHECK_CLOSE(real(0.40824829046386301636621401245098), quat.x, real(0.000001));
  CHECK_CLOSE(real(0.40824829046386301636621401245098), quat.y, real(0.000001));
  CHECK_CLOSE(real(0.40824829046386301636621401245098), quat.z, real(0.000001));
  CHECK_CLOSE(real(0.70710678118654752440084436210485), quat.w, real(0.000001));
}

TEST(ToHalf)
{
  MARK_AS_UNIMPLEMENTED();
}

TEST(ToFloat)
{
  MARK_AS_UNIMPLEMENTED();
}
