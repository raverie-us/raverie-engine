//////////////////////////////////////////////////////////////////////////////
///
/// \file TransformSupport.hpp
/// Helper functions for the Transform component.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
///Generates an orthonormal basis from the facing vector.
Mat3 GenerateRotationMatrix(Vec3Param facing);
void GenerateRotationMatrix(Vec3Param facing, Mat3Ptr matrix);

///Generates a rotation matrix from the given facing and up vectors.
Mat3 GenerateRotationMatrix(Vec3Param facing, Vec3Param up);
void GenerateRotationMatrix(Vec3Param facing, Vec3Param up, Mat3Ptr matrix);

///Generates a rotation matrix from the given bases.
Mat3 GenerateRotationMatrix(Vec3Param facing, Vec3Param up, Vec3Param right);
void GenerateRotationMatrix(Vec3Param facing, Vec3Param up, Vec3Param right, 
                            Mat3Ptr matrix);

///Generates a rotation matrix from the given Euler angles.
Mat3 GenerateRotationMatrix(Math::real x, Math::real y, Math::real z);
void GenerateRotationMatrix(Math::real x, Math::real y, Math::real z, 
                            Mat3Ptr matrix);
Mat3 GenerateRotationMatrix(EulerAnglesParam eulerAngles);
void GenerateRotationMatrix(EulerAnglesParam eulerAngles, Mat3Ptr matrix);

}// namespace Zero
