///////////////////////////////////////////////////////////////////////////////
///
/// \file SimMath.hpp
/// Platform dependent defines, typedefs and globals for simd operations.
/// 
/// Authors: Joshua Davis
/// Copyright 2011-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <xmmintrin.h>
#include <emmintrin.h>

namespace Math
{

namespace Simd
{

#define SimInline __forceinline

//wrapper for the shuffle intrinsic so that the rest
//of the library can be platform independent.
//Note: when picking from the two vectors
//v1 = [w1, z1, y1, x1] and
//v2 = [w2, z2, y2, x2]
//with VecShuffle(v1,v2,0,1,2,3), the result is [x2, y2, z1, w1].
//So the first two values are from the second vector and the
//second two values are from the first vector.
#define VecShuffle(v1,v2,v2Index1,v2Index2,v1Index1,v1Index2) \
  _mm_shuffle_ps(v1,v2,_MM_SHUFFLE(v2Index1,v2Index2,v1Index1,v1Index2))

typedef float scalar;
typedef __m128 SimVector;
typedef SimVector SimVec;
typedef SimVec& SimVecRef;
typedef const SimVec& SimVecParam;

//----------------------------------------------------------------- SIM Matrix 4
struct SimMatrix4
{
  SimVec columns[4];
};

typedef SimMatrix4 SimMat4;
typedef SimMat4& SimMatRef4;
typedef const SimMat4& SimMat4Param;

//----------------------------------------------------------------- SIM Matrix 3
struct SimMatrix3
{
  SimVec columns[3];
};

typedef SimMatrix3 SimMat3;
typedef SimMat3& SimMatRef3;
typedef const SimMat3& SimMat3Param;


#define SimVecGlobalConstant extern const __declspec(selectany)

inline uint& MaskInt()
{
  static uint maskInt = 0xffffffff;
  return maskInt;
}

const static scalar gMaskScalar = *reinterpret_cast<const float*>(&MaskInt());

SimVecGlobalConstant SimVec gSimOne = { 1.0f, 1.0f, 1.0f, 1.0f};
SimVecGlobalConstant SimVec gSimOneVec3 = _mm_setr_ps(1.0f, 1.0f, 1.0f, 0.0f);
SimVecGlobalConstant SimVec gSimZero = { 0.0f, 0.0f, 0.0f, 0.0f};
SimVecGlobalConstant SimVec gSimNegativeOne = { -1.0f, -1.0f, -1.0f, -1.0f};
SimVecGlobalConstant SimVec gSimOneHalf = { 0.5f, 0.5f, 0.5f, 0.5f};
SimVecGlobalConstant SimVec gSimVec3Mask = {gMaskScalar, gMaskScalar, gMaskScalar, 0x00000000};
SimVecGlobalConstant SimVec gSimFullMask = {gMaskScalar, gMaskScalar, gMaskScalar, gMaskScalar};
SimVecGlobalConstant SimVec gSimBasisX = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
SimVecGlobalConstant SimVec gSimBasisY = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
SimVecGlobalConstant SimVec gSimBasisZ = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
SimVecGlobalConstant SimVec gSimBasisW = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
SimVecGlobalConstant SimVec gSimMaskX = _mm_setr_ps(gMaskScalar, 0x00000000, 0x00000000, 0x00000000);
SimVecGlobalConstant SimVec gSimMaskY = _mm_setr_ps(0x00000000,gMaskScalar, 0x00000000, 0x00000000);
SimVecGlobalConstant SimVec gSimMaskZ = _mm_setr_ps(0x00000000, 0x00000000, gMaskScalar, 0x00000000);
SimVecGlobalConstant SimVec gSimMaskW = _mm_setr_ps(0x00000000, 0x00000000, 0x00000000, gMaskScalar);

#undef SimVecGlobalConstant

}//namespace Simd

}//namespace Math
