///////////////////////////////////////////////////////////////////////////////
///
/// \file JacobiSolver.hpp
/// Declaration of the JacobiSolver class.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Math/IndexPolicies.hpp"
#include "Math/ErrorCallbacks.hpp"

namespace Math
{

///Lcp solver using Jacobi iteration.
struct JacobiSolver
{
  JacobiSolver()
  {
    mMaxIterations = 100;
    mErrorTolerance = real(.001f);
  }

  template <typename MatrixType, typename VectorType, typename PolicyType, typename ErrorCallbackType>
  void Solve(MatrixType& A, VectorType& b, VectorType& x0, PolicyType& policy, ErrorCallbackType& errCallback)
  {
    uint dimension = policy.GetDimension(b);
    uint iteration;
    real convergence = real(0.0);

    VectorType oldX;
    for(iteration = 0; iteration < mMaxIterations; ++iteration)
    {
      oldX = x0;
      convergence = real(0.0);

      for(uint i = 0; i < dimension; ++i)
      {
        real delta = real(0.0);

        for(uint j = 0; j < i; ++j)
          delta += policy(A, i, j) * policy(oldX, j);

        for(uint j = i + 1; j < dimension; ++j)
          delta += policy(A, i, j) * policy(oldX, j);

        policy(x0, i) = (policy(b, i) - delta) / policy(A, i, i);

        real difference = policy(x0, i) - policy(oldX, i);
        convergence += difference * difference;
      }

      if(convergence < mErrorTolerance * mErrorTolerance)
        break;
    }    

    if(iteration == mMaxIterations)
    {
      errCallback(A, b, x0, convergence);
    }
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
