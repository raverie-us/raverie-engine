///////////////////////////////////////////////////////////////////////////////
///
/// \file GaussSeidelSolver.hpp
/// Declaration of the GaussSeidelSolver class.
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

///Lcp solver using Gauss-Seidel iteration.
struct GaussSeidelSolver
{
  GaussSeidelSolver()
  {
    mMaxIterations = 100;
    mErrorTolerance = real(.001f);
  }

  template <typename MatrixType, typename VectorType, typename PolicyType, typename ErrorCallbackType>
  void Solve(MatrixType& A, VectorType& b, VectorType& x0, PolicyType& policy, ErrorCallbackType& errCallback)
  {
    uint dimension = policy.GetDimension(b);
    uint iteration;
    real convergence = real(0);
    real toleranceThresholdSq = mErrorTolerance * mErrorTolerance;

    for(iteration = 0; iteration < mMaxIterations; ++iteration)
    {
      convergence = real(0);

      for(uint i = 0; i < dimension; ++i)
      {
        real delta = real(0);

        for(uint j = 0; j < i; ++j)
          delta += policy(A, i, j) * policy(x0, j);
        
        for(uint j = i + 1; j < dimension; ++j)
          delta += policy(A, i, j) * policy(x0, j);

        real& newElement = policy(x0, i);
        real oldElement = newElement;

        real aii = policy(A, i, i);
        if(aii != 0)
          newElement = (policy(b, i) - delta) / aii;

        real difference = newElement - oldElement;
        convergence += difference * difference;
      }

      if(convergence < toleranceThresholdSq)
        break;
    }
    

    if(convergence >= toleranceThresholdSq)
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
