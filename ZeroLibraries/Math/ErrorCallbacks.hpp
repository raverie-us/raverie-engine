///////////////////////////////////////////////////////////////////////////////
///
/// \file ErrorCallbacks.hpp
/// Declaration of the EmptyErrorCallback class.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Math
{

/// An empty callback for when an Lcp solver fails to converge.
/// Used when no callback is provided to an lcp solver.
struct EmptyErrorCallback
{
  template<typename MatrixType, typename VectorType>
  void operator()(MatrixType& A, VectorType& b,VectorType& x0, real convergence)
  {

  }
};

struct SimpleErrorCallback
{
  bool mSuccessfullySolved;
  real mConvergence;

  SimpleErrorCallback()
  {
    mSuccessfullySolved = true;
    mConvergence = 0;
  }

  template<typename MatrixType, typename VectorType>
  void operator()(MatrixType& A, VectorType& b, VectorType& x0, real convergence)
  {
    mSuccessfullySolved = false;
    mConvergence = convergence;
  }
};

}//namespace Math
