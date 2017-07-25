///////////////////////////////////////////////////////////////////////////////
///
/// \file ConjugateGradient.hpp
/// Declaration of the ConjugateGradientSolver.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Math/Reals.hpp"
#include "Utility/Typedefs.hpp"
#include "Math/ErrorCallbacks.hpp"

namespace Math
{

/// A LCP (Linear Complimentary Problem) solver. This solves the equation Ax = b
/// for the vector x where b is a vector and A is a matrix. Solving for x can be
/// computationally cheaper than computing the inverse of A when A is large and
/// especially when A is very sparse.
/// The conjugate gradient solving method only works with positive-definite matrices,
/// that is if you were to graph the matrix A, it would have to be a 'paraboloid that
/// points up (by traveling down we will find the origin)
struct ConjugateGradientSolver
{
  ConjugateGradientSolver()
  {
    mMaxIterations = 100;
    mErrorTolerance = real(.001f);
  }

  /// X0 is the initial values and also the results (set to 0 if there is no initial)
  template <typename MatrixType, typename VectorType, typename PolicyType, typename ErrorCallbackType>
  void SolveSlow(MatrixType& A, VectorType& b, VectorType& x0, PolicyType& policy, ErrorCallbackType& errCallback)
  {
    VectorType residual = b;
    residual = policy.Subtract(residual, policy.Transform(A, x0));

    VectorType direction = residual;
    real gammaNew = policy.Dot(residual, residual);
    real gamma0 = gammaNew;
    real errorBounds = mErrorTolerance * mErrorTolerance * gamma0;

    uint i;
    for(i = 0; i < mMaxIterations && gammaNew > errorBounds; ++i)
    {
      VectorType q = policy.Transform(A, direction);
      real stepSize = gammaNew / policy.Dot(direction, q);
      //could add a Add by scaled vector for efficiency
      x0 = policy.Add(x0, policy.Scale(direction, stepSize));

      //could add a -= to policy for efficiency
      if((i + 1) % 50)
        residual = policy.Subtract(residual, policy.Scale(q, stepSize));
      else
        residual = policy.Subtract(b, policy.Transform(A, x0));
      

      real gammaOld = gammaNew;
      gammaNew = policy.Dot(residual, residual);
      real beta = gammaNew / gammaOld;
      direction = policy.Add(residual, policy.Scale(direction, beta));
    }
    if(i == mMaxIterations)
    {
      errCallback(A, b, x0, gammaNew);
    }
  }

  template <typename MatrixType, typename VectorType, typename PolicyType, typename ErrorCallbackType>
  void Solve(MatrixType& A, VectorType& b, VectorType& x0, PolicyType& policy, ErrorCallbackType& errCallback)
  {
    VectorType residual = b;
    //r = r Z- A * x0
    policy.NegativeTransformSubtract(A, x0, residual, &residual);

    VectorType direction = residual;
    real gammaNew = policy.Dot(residual, residual);
    real gamma0 = gammaNew;
    real errorBounds = mErrorTolerance * mErrorTolerance * gamma0;

    uint i;
    for(i = 0; i < mMaxIterations && gammaNew > errorBounds; ++i)
    {
      VectorType q = policy.Transform(A, direction);
      real stepSize = gammaNew / policy.Dot(direction, q);
      
      policy.MultiplyAdd(direction, stepSize, x0, &x0);

      //could add a -= to policy for efficiency
      if((i + 1) % 50)
        policy.MultiplySubtract(q, stepSize, residual, &residual);
      else
        policy.NegativeTransformSubtract(A, x0, b, &residual);


      real gammaOld = gammaNew;
      gammaNew = policy.Dot(residual, residual);
      real beta = gammaNew / gammaOld;
      policy.MultiplyAdd(direction, beta, residual, &direction);
    }
    if(i == mMaxIterations)
    {
      errCallback(A, b, x0, gammaNew);
    }
  }

  template <typename MatrixType, typename VectorType, typename PolicyType, typename ErrorCallbackType>
  void SolveGeneric(MatrixType& A, VectorType& b, VectorType& x0, VectorType& residual, VectorType& update, VectorType& direction, PolicyType& policy, ErrorCallbackType& errCallback)
  {
    residual = b;
    //r = r Z- A * x0
    policy.NegativeTransformSubtract(A, x0, residual, &residual);

    direction = residual;
    real gammaNew = policy.Dot(residual, residual);
    real gamma0 = gammaNew;
    real errorBounds = mErrorTolerance * mErrorTolerance * gamma0;

    uint i;
    for(i = 0; i < mMaxIterations && gammaNew > errorBounds; ++i)
    {
      policy.Transform(A, direction, &update);
      real stepSize = gammaNew / policy.Dot(direction, update);

      policy.MultiplyAdd(direction, stepSize, x0, &x0);

      //could add a -= to policy for efficiency
      if((i + 1) % 50)
        policy.NegativeMultiplySubtract(update, stepSize, residual, &residual);
      else
        policy.NegativeTransformSubtract(A, x0, b, &residual);


      real gammaOld = gammaNew;
      gammaNew = policy.Dot(residual, residual);
      real beta = gammaNew / gammaOld;
      policy.MultiplyAdd(direction, beta, residual, &direction);
    }
    if(i == mMaxIterations)
    {
      errCallback(A, b, x0, gammaNew);
    }
  }

  template <typename MatrixType, typename VectorType, typename PolicyType>
  void SolveGeneric(MatrixType& A, VectorType& b, VectorType& x0, VectorType& residual, VectorType& update, VectorType& direction, PolicyType& policy)
  {
    EmptyErrorCallback emptyErr;
    SolveGeneric(A, b, x0, residual, update, direction, policy, emptyErr);
  }

  template <typename MatrixType, typename VectorType, typename PolicyType>
  void Solve(MatrixType& A, VectorType& b, VectorType& x0, PolicyType& policy)
  {
    EmptyErrorCallback emptyErr;
    Solve(A, b, x0, policy, emptyErr);
  }

  uint mMaxIterations;
  real mErrorTolerance;
};

}//namespace Math
