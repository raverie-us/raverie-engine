///////////////////////////////////////////////////////////////////////////////
///
/// \file ConstraintFragmentsSse.hpp
/// 
/// Authors: Joshua Davis
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Math/SimMath.hpp"
#include "Math/SimVectors.hpp"
#include "Math/SimMatrix3.hpp"

namespace Zero
{

namespace Physics
{

using Math::Simd::SimVec;
using Math::Simd::SimVecRef;
using Math::Simd::SimVecParam;
using Math::Simd::SimMat3;
using Math::Simd::SimMat3Param;
using namespace Math;

__declspec(align(16))
struct Vec3_16 : public Vec3
{
  Vec3_16& operator=(Vec3Param data)
  {
    Vec3::operator=(data);
    return *this;
  }
};

__declspec(align(16))
struct Vec4_16 : public Vec4
{
  Vec4_16& operator=(Vec4Param data)
  {
    Vec4::operator=(data);
    return *this;
  }
};

SimInline real SimVecToReal(SimVecParam v)
{
  Vec4_16 vals;
  Math::Simd::Store(v, vals.array);
  return vals[0];
}

SimInline SimVec ComputeLambdaSse(SimVecParam v0, SimVecParam w0, SimVecParam v1, SimVecParam w1, 
                                  SimVecParam L0, SimVecParam A0, SimVecParam L1, SimVecParam A1, 
                                  ConstraintMolecule& mol)
{
  SimVec cDot = Simd::Multiply(v0, L0);
  cDot = Simd::MultiplyAdd(v1, L1, cDot);
  cDot = Simd::MultiplyAdd(w0, A0, cDot);
  cDot = Simd::MultiplyAdd(w1, A1, cDot);

  cDot = Simd::InnerSum3(cDot);

  SimVec mass = Simd::Set(mol.mMass);
  SimVec bias = Simd::Set(mol.mBias);
  SimVec gamma = Simd::Set(mol.mGamma);
  SimVec impulse = Simd::Set(mol.mImpulse);
  SimVec minImpulse = Simd::Set(mol.mMinImpulse);
  SimVec maxImpulse = Simd::Set(mol.mMaxImpulse);

  cDot = Simd::Add(cDot, bias);
  cDot = Simd::MultiplyAdd(gamma, impulse, cDot);
  SimVec lambda = Simd::Negate(Simd::Multiply(mass, cDot));

  SimVec oldImpulse = impulse;
  impulse = Simd::Clamp(Simd::Add(oldImpulse, lambda), minImpulse, maxImpulse);
  lambda = Simd::Subtract(impulse, oldImpulse);

  //save the accumulated impulse
  mol.mImpulse = SimVecToReal(impulse);
  //return the lambda to apply
  return lambda;
}

SimInline void SolveConstraintSse(SimVecRef v0, SimVecRef w0, SimVecRef v1, SimVecRef w1, 
                                  SimVecParam m0, SimMat3Param i0, SimVecParam m1, SimMat3Param i1,
                                  ConstraintMolecule& mol)
{
  SimVec L0 = Simd::UnAlignedLoad(mol.mJacobian.Linear[0].array);
  SimVec L1 = Simd::UnAlignedLoad(mol.mJacobian.Linear[1].array);
  SimVec A0 = Simd::UnAlignedLoad(mol.mJacobian.Angular[0].array);
  SimVec A1 = Simd::UnAlignedLoad(mol.mJacobian.Angular[1].array);

  SimVec lambda = ComputeLambdaSse(v0, w0, v1, w1, L0, A0, L1, A1, mol);

  v0 = Simd::MultiplyAdd(Simd::Multiply(m0, L0), lambda, v0);
  v1 = Simd::MultiplyAdd(Simd::Multiply(m1, L1), lambda, v1);
  w0 = Simd::MultiplyAdd(Simd::Transform(i0, A0), lambda, w0);
  w1 = Simd::MultiplyAdd(Simd::Transform(i1, A1), lambda, w1);
}

}//namespace Physics

}//namespace Zero
