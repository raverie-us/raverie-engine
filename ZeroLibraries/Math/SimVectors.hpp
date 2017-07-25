///////////////////////////////////////////////////////////////////////////////
///
/// \file SimVectors.hpp
/// Declaration of the SimVec3 and SimVec3 functionality.
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

//platform specific

//loading
SimVec Load(const scalar vals[4]);
SimVec UnAlignedLoad(const scalar vals[4]);
//storing
void Store(SimVecParam vec, scalar result[4]);
void UnAlignedStore(SimVecParam vec, scalar result[4]);
//setting
SimVec Set(scalar val);
SimVec Set4(scalar x, scalar y, scalar z, scalar w);
SimVec ZeroOutVec();
//basic arithmetic
SimVec Add(SimVecParam lhs, SimVecParam rhs);
SimVec Subtract(SimVecParam lhs, SimVecParam rhs);
SimVec Multiply(SimVecParam lhs, SimVecParam rhs);
SimVec Divide(SimVecParam lhs, SimVecParam rhs);
SimVec ReciprocalEst(SimVecParam vec);
SimVec Sqrt(SimVecParam vec);
SimVec ReciprocalSqrtEst(SimVecParam vec);
//bitwise
SimVec OrVec(SimVecParam vec, SimVecParam mask);
SimVec AndVec(SimVecParam vec, SimVecParam mask);
//r = ~vec & mask
SimVec AndNotVec(SimVecParam vec, SimVecParam mask);
SimVec XorVec(SimVecParam vec, SimVecParam mask);
//logic
//returns a simvec that has all bits set if an element was equal and all bits cleared otherwise
SimVec Equal(SimVecParam lhs, SimVecParam rhs);
SimVec NotEqual(SimVecParam lhs, SimVecParam rhs);
SimVec Greater(SimVecParam lhs, SimVecParam rhs);
SimVec GreaterEqual(SimVecParam lhs, SimVecParam rhs);
SimVec Less(SimVecParam lhs, SimVecParam rhs);
SimVec LessEqual(SimVecParam lhs, SimVecParam rhs);
//bounds logic/arithmetic
SimVec Min(SimVecParam vec, SimVecParam minVec);
SimVec Max(SimVecParam vec, SimVecParam maxVec);
//larger primitives (multiple sse instructions, 1 vmx instruction)
//Selects v0 if the element mask is empty, v1 if it is not
SimVec Select(SimVecParam v0, SimVecParam v1, SimVecParam select);
SimVec MultiplyAdd(SimVecParam v0, SimVecParam v1, SimVecParam v2);
SimVec MultiplySubtract(SimVecParam v0, SimVecParam v1, SimVecParam v2);

//platform independent

SimVec GetXYZ(SimVecParam vec);
//various selecting/setting
SimVec SplatX(SimVecParam vec);
SimVec SplatY(SimVecParam vec);
SimVec SplatZ(SimVecParam vec);
SimVec SplatW(SimVecParam vec);
//basic arithmetic
SimVec Scale(SimVecParam lhs, scalar rhs);
SimVec Negate(SimVecParam lhs);
SimVec Reciprocal(SimVecParam vec);
SimVec ReciprocalSqrt(SimVecParam vec);
//logic
SimVec Clamp(SimVecParam vec, SimVecParam minVec, SimVecParam maxVec);
SimVec Abs(SimVecParam vec);

//random stuff
SimVec Lerp(SimVecParam start, SimVecParam end, SimVecParam t);
SimVec Lerp(SimVecParam start, SimVecParam end, scalar t);
SimVec BaryCentric(SimVecParam p0, SimVecParam p1, SimVecParam p2, scalar u, scalar v);

//arithmetic operators
SimVec operator+(SimVecParam lhs, SimVecParam rhs);
SimVec operator-(SimVecParam lhs, SimVecParam rhs);
SimVec operator*(SimVecParam lhs, SimVecParam rhs);
SimVec operator/(SimVecParam lhs, SimVecParam rhs);
void operator+=(SimVecRef lhs, SimVecParam rhs);
void operator-=(SimVecRef lhs, SimVecParam rhs);
void operator*=(SimVecRef lhs, SimVecParam rhs);
void operator/=(SimVecRef lhs, SimVecParam rhs);

//vector with scalar arithmetic operators
SimVec operator*(SimVecParam lhs, scalar rhs);
SimVec operator/(SimVecParam lhs, scalar rhs);
void operator*=(SimVecRef lhs, scalar rhs);
void operator/=(SimVecRef lhs, scalar rhs);

//vector3 specific
SimVec Set3(scalar x, scalar y, scalar z);
SimVec InnerSum3(SimVec vec);
SimVec Dot3(SimVecParam lhs, SimVecParam rhs);
SimVec Cross3(SimVecParam lhs, SimVecParam rhs);
SimVec LengthSq3(SimVecParam vec);
SimVec Length3(SimVecParam vec);
SimVec Normalize3(SimVecParam vec);
SimVec NormalizeEst3(SimVecParam vec);
SimVec AttemptNormalize4(SimVecParam lhs);
SimVec Reflect3(SimVecParam incident, SimVecParam normal);
SimVec Refract3(SimVecParam incident, SimVecParam normal, SimVecParam refractionIndex);

//vector4 specific
//the scalar value is splatted across all values of the result
SimVec InnerSum4(SimVec vec);
SimVec Dot4(SimVecParam lhs, SimVecParam rhs);
SimVec LengthSq4(SimVecParam lhs);
SimVec Length4(SimVecParam lhs);
SimVec Normalize4(SimVecParam lhs);
SimVec NormalizeEst4(SimVecParam lhs);
SimVec AttemptNormalize4(SimVecParam lhs);
SimVec Reflect4(SimVecParam incident, SimVecParam normal);
SimVec Refract4(SimVecParam incident, SimVecParam normal, SimVecParam refractionIndex);

}//namespace Simd

}//namespace Math

#include "Math/SimVectorSpecific.inl"
#include "Math/SimVectors.inl"
