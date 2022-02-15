// MIT Licensed (see LICENSE.md).
#pragma once

#include <xmmintrin.h>
#include <emmintrin.h>

#if defined(WelderTargetOsWindows)
#  define ZeroPreAlign16 __declspec(align(16))
#  define ZeroPostAlign16
#else
#  define ZeroPreAlign16
#  define ZeroPostAlign16 __attribute__((__aligned__(16)))
#endif

namespace Math
{

namespace Simd
{

#define SimInline ZeroForceInline

// wrapper for the shuffle intrinsic so that the rest
// of the library can be platform independent.
// Note: when picking from the two vectors
// v1 = [w1, z1, y1, x1] and
// v2 = [w2, z2, y2, x2]
// with VecShuffle(v1,v2,0,1,2,3), the result is [x2, y2, z1, w1].
// So the first two values are from the second vector and the
// second two values are from the first vector.
#define VecShuffle(v1, v2, v2Index1, v2Index2, v1Index1, v1Index2)                                                     \
  _mm_shuffle_ps(v1, v2, _MM_SHUFFLE(v2Index1, v2Index2, v1Index1, v1Index2))

// Alias class to hold __m128 union to compile union
// operator overloads with Clang/LLVM
ZeroPreAlign16 class M128
{
public:
  M128() : mValue()
  {
  }
  M128(const M128& rhs) : mValue(rhs.mValue)
  {
  }
  M128(__m128 value) : mValue(value)
  {
  }

  operator __m128&()
  {
    return mValue;
  }

  operator const __m128&() const
  {
    return mValue;
  }

  __m128 mValue;
};

typedef float scalar;

// typedef __m128 SimVector;
typedef M128 SimVector;
typedef SimVector SimVec;
typedef SimVec& SimVecRef;
typedef const SimVec& SimVecParam;

struct SimMatrix4
{
  SimVec columns[4];
};

typedef SimMatrix4 SimMat4;
typedef SimMat4& SimMatRef4;
typedef const SimMat4& SimMat4Param;

struct SimMatrix3
{
  SimVec columns[3];
};

typedef SimMatrix3 SimMat3;
typedef SimMat3& SimMatRef3;
typedef const SimMat3& SimMat3Param;

inline uint& MaskInt()
{
  static uint maskInt = 0xffffffff;
  return maskInt;
}

extern const SimVec gSimOne;
extern const SimVec gSimOneVec3;
extern const SimVec gSimZero;
extern const SimVec gSimNegativeOne;
extern const SimVec gSimOneHalf;
extern const SimVec gSimVec3Mask;
extern const SimVec gSimFullMask;
extern const SimVec gSimBasisX;
extern const SimVec gSimBasisY;
extern const SimVec gSimBasisZ;
extern const SimVec gSimBasisW;
extern const SimVec gSimMaskX;
extern const SimVec gSimMaskY;
extern const SimVec gSimMaskZ;
extern const SimVec gSimMaskW;

} // namespace Simd

} // namespace Math
