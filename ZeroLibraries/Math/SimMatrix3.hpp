///////////////////////////////////////////////////////////////////////////////
///
/// \file SimMatrix3.hpp
/// Declaration of the SimMat3 functionality.
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
SimMat3 LoadMat3(const scalar vals[12]);
SimMat3 SetMat3(scalar m00, scalar m01, scalar m02, 
                scalar m10, scalar m11, scalar m12,
                scalar m20, scalar m21, scalar m22);
SimMat3 UnAlignedLoadMat3(const scalar vals[12]);
//storing
void StoreMat3(scalar vals[12], SimMat3Param mat);
void UnAlignedStoreMat3(scalar vals[12], SimMat3Param mat);
//default matrix sets
SimMat3 ZeroOutMat3();
SimMat3 IdentityMat3();
//basis elements
SimVec BasisX(SimMat3Param mat);
SimVec BasisY(SimMat3Param mat);
SimVec BasisZ(SimMat3Param mat);
SimMat3 SetBasisX(SimMat3Param mat, SimVecParam value);
SimMat3 SetBasisY(SimMat3Param mat, SimVecParam value);
SimMat3 SetBasisZ(SimMat3Param mat, SimVecParam value);
//basic arithmetic
SimMat3 Add(SimMat3Param lhs, SimMat3Param rhs);
SimMat3 Subtract(SimMat3Param lhs, SimMat3Param rhs);
SimMat3 Multiply(SimMat3Param lhs, SimMat3Param rhs);
SimMat3 Scale(SimMat3Param mat, scalar scale);
SimMat3 ComponentScale(SimMat3Param lhs, SimMat3Param rhs);
SimMat3 ComponentDivide(SimMat3Param lhs, SimMat3Param rhs);
//matrix vector arithmetic
SimVec Transform(SimMat3Param mat, SimVecParam vec);
SimVec TransposeTransform(SimMat3Param mat, SimVecParam vec);
//transform building
SimMat3 BuildScale3(SimVecParam scale);
SimMat3 BuildRotation3(SimVecParam axis, scalar angle);
SimMat3 BuildRotation3(SimVecParam quat);
SimMat3 BuildTransform3(SimVecParam axis, scalar angle, SimVecParam scale);
SimMat3 BuildTransform3(SimVecParam quat, SimVecParam scale);
SimMat3 BuildTransform3(SimMat3Param rot, SimVecParam scale);
//transposes the upper 3x3 of the 4x4 and leaves the remaining elements alone
SimMat3 Transpose3(SimMat3Param mat);
SimMat3 AffineInverse3(SimMat3Param transform);
SimMat3 AffineInverseWithScale3(SimMat3Param transform);
//basic logic
SimMat3 Equal(SimMat3Param mat1, SimMat3Param mat2);

}//namespace Simd

}//namespace Math

#include "Math/SimMatrix3.inl"
