///////////////////////////////////////////////////////////////////////////////
///
/// \file SimMatrix4.hpp
/// Declaration of the SimMat4 functionality.
/// 
/// Authors: Joshua Davis
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Math
{

namespace Simd
{

//loading
SimMat4 LoadMat4x4(const scalar vals[16]);
SimMat4 SetMat4x4(scalar m00, scalar m01, scalar m02, scalar m03, 
                  scalar m10, scalar m11, scalar m12, scalar m13,
                  scalar m20, scalar m21, scalar m22, scalar m23,
                  scalar m30, scalar m31, scalar m32, scalar m33);
SimMat4 UnAlignedLoadMat4x4(const scalar vals[16]);
//storing
void StoreMat4x4(scalar vals[16], SimMat4Param mat);
void UnAlignedStoreMat4x4(scalar vals[16], SimMat4Param mat);
//default matrix sets
SimMat4 ZeroOutMat4x4();
SimMat4 IdentityMat4x4();
//basis elements
SimVec BasisX(SimMat4Param mat);
SimVec BasisY(SimMat4Param mat);
SimVec BasisZ(SimMat4Param mat);
SimVec BasisW(SimMat4Param mat);
SimMat4 SetBasisX(SimMat4Param mat, SimVecParam value);
SimMat4 SetBasisY(SimMat4Param mat, SimVecParam value);
SimMat4 SetBasisZ(SimMat4Param mat, SimVecParam value);
SimMat4 SetBasisW(SimMat4Param mat, SimVecParam value);
//basic arithmetic
SimMat4 Add(SimMat4Param lhs, SimMat4Param rhs);
SimMat4 Subtract(SimMat4Param lhs, SimMat4Param rhs);
SimMat4 Multiply(SimMat4Param lhs, SimMat4Param rhs);
SimMat4 Scale(SimMat4Param mat, scalar scale);
SimMat4 ComponentScale(SimMat4Param lhs, SimMat4Param rhs);
SimMat4 ComponentDivide(SimMat4Param lhs, SimMat4Param rhs);

//matrix vector arithmetic
SimVec TransformPoint(SimMat4Param mat, SimVecParam vec);
SimVec TransposeTransformPoint(SimMat4Param mat, SimVecParam vec);
SimVec TransformNormal(SimMat4Param mat, SimVecParam vec);
SimVec TransposeTransformNormal(SimMat4Param mat, SimVecParam vec);
SimVec TransformPointProjected(SimMat4Param mat, SimVecParam vec);
//transform building
SimMat4 BuildScale(SimVecParam scale);
SimMat4 BuildRotation(SimVecParam axis, scalar angle);
SimMat4 BuildRotation(SimVecParam quat);
SimMat4 BuildTranslation(SimVecParam translation);
SimMat4 BuildTransform(SimVecParam translation, SimVecParam axis, scalar angle, SimVecParam scale);
SimMat4 BuildTransform(SimVecParam translation, SimVecParam quat, SimVecParam scale);
SimMat4 BuildTransform(SimVecParam translation, SimMat4Param rot, SimVecParam scale);
//transposes the upper 3x3 of the 4x4 and leaves the remaining elements alone
SimMat4 TransposeUpper3x3(SimMat4Param mat);
SimMat4 Transpose4x4(SimMat4Param mat);
SimMat4 AffineInverse4x4(SimMat4Param transform);
SimMat4 AffineInverseWithScale4x4(SimMat4Param transform);
//basic logic
SimMat4 Equal(SimMat4Param mat1, SimMat4Param mat2);


//arithmetic operators
SimMat4 operator+(SimMat4Param lhs, SimMat4Param rhs);
SimMat4 operator-(SimMat4Param lhs, SimMat4Param rhs);
SimMat4 operator*(SimMat4Param lhs, SimMat4Param rhs);

//vector with scalar arithmetic operators
SimMat4 operator*(SimMat4Param lhs, scalar rhs);
SimMat4 operator/(SimMat4Param lhs, scalar rhs);

}//namespace Simd

}//namespace Math


#include "Math/SimMatrix4.inl"
