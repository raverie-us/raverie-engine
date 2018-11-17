///////////////////////////////////////////////////////////////////////////////
///
/// \file TransformSupport.cpp
/// Helper functions for the Transform component.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

Mat3 GenerateRotationMatrix(Vec3Param facing)
{
  Mat3 rotation;
  GenerateRotationMatrix(facing, &rotation);
  return rotation;
}

void GenerateRotationMatrix(Vec3Param facing, Mat3Ptr matrix)
{
  //Generate an orthonormal basis from the facing vector
  Vec3 right, up;
  Math::GenerateOrthonormalBasis(facing, &right, &up);
  GenerateRotationMatrix(facing, up, right, matrix);
}

Mat3 GenerateRotationMatrix(Vec3Param facing, Vec3Param up)
{
  Mat3 rotation;
  GenerateRotationMatrix(facing, up, &rotation);
  return rotation;
}

void GenerateRotationMatrix(Vec3Param facing, Vec3Param up, Mat3Ptr matrix)
{
  //Get the right vector
  Vec3 right = Math::Cross(facing, up);
  GenerateRotationMatrix(facing, up, right, matrix);
}

Mat3 GenerateRotationMatrix(Vec3Param facing, Vec3Param up, Vec3Param right)
{
  Mat3 rotation;
  GenerateRotationMatrix(facing, up, right, &rotation);
  return rotation;
}

void GenerateRotationMatrix(Vec3Param facing, Vec3Param up, Vec3Param right, 
                            Mat3Ptr matrix)
{
  matrix->SetBasis(0, right);
  matrix->SetBasis(1, up);
  matrix->SetBasis(2, -facing);
}

Mat3 GenerateRotationMatrix(real x, real y, real z)
{
  Mat3 rotation;
  GenerateRotationMatrix(x, y, z, &rotation);
  return rotation;
}

void GenerateRotationMatrix(real x, real y, real z, Mat3Ptr matrix)
{
  Math::EulerAngles eulerAngles(x, y, z, Math::EulerOrders::XYZs);
  GenerateRotationMatrix(eulerAngles, matrix);
}

Mat3 GenerateRotationMatrix(EulerAnglesParam eulerAngles)
{
  Mat3 rotation;
  GenerateRotationMatrix(eulerAngles, &rotation);
  return rotation;
}

void GenerateRotationMatrix(EulerAnglesParam eulerAngles, Mat3Ptr matrix)
{
  Math::ToMatrix3(eulerAngles, matrix);
}

}// namespace Zero
