////////////////////////////////////////////////////////////////////////////////
//
// Authors: Dane Curbow
// Copyright 2018, DigiPen Institute of Technology
//
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{

namespace Simd
{

const static scalar gMaskScalar = *reinterpret_cast<const float*>(&MaskInt());

const SimVec gSimOne = __m128{ 1.0f, 1.0f, 1.0f, 1.0f};
const SimVec gSimOneVec3 = _mm_setr_ps(1.0f, 1.0f, 1.0f, 0.0f);
const SimVec gSimZero = __m128{ 0.0f, 0.0f, 0.0f, 0.0f};
const SimVec gSimNegativeOne = __m128{ -1.0f, -1.0f, -1.0f, -1.0f};
const SimVec gSimOneHalf = __m128{ 0.5f, 0.5f, 0.5f, 0.5f};
const SimVec gSimVec3Mask = __m128{gMaskScalar, gMaskScalar, gMaskScalar, 0x00000000};
const SimVec gSimFullMask = __m128{gMaskScalar, gMaskScalar, gMaskScalar, gMaskScalar};
const SimVec gSimBasisX = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
const SimVec gSimBasisY = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
const SimVec gSimBasisZ = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
const SimVec gSimBasisW = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
const SimVec gSimMaskX = _mm_setr_ps(gMaskScalar, 0x00000000, 0x00000000, 0x00000000);
const SimVec gSimMaskY = _mm_setr_ps(0x00000000, gMaskScalar, 0x00000000, 0x00000000);
const SimVec gSimMaskZ = _mm_setr_ps(0x00000000, 0x00000000, gMaskScalar, 0x00000000);
const SimVec gSimMaskW = _mm_setr_ps(0x00000000, 0x00000000, 0x00000000, gMaskScalar);

}// namespace Simd

}// namespace Math