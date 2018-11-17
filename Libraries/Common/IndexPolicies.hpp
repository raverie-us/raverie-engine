///////////////////////////////////////////////////////////////////////////////
///
/// \file IndexPolicies.hpp
/// Declaration of the Lcp policies for various math classes that need to use
/// only index into the vector and matrix to solve.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Math
{

struct IndexDim3Policy
{
  real& operator()(Matrix3& A, uint row, uint col)
  {
    return A(row, col);
  }

  real& operator()(Vector3& v, uint i)
  {
    return v[i];
  }

  uint GetDimension(Vector3& v)
  {
    return 3;
  }
};

struct IndexDim4Policy
{
  real& operator()(Matrix4& A, uint row, uint col)
  {
    return A(row, col);
  }

  real& operator()(Vector4& v, uint i)
  {
    return v[i];
  }

  uint GetDimension(Vector4& v)
  {
    return 4;
  }
};

struct GenericDimIndexPolicy
{
  template <typename MatrixType>
  real& operator()(MatrixType& A, uint row, uint col)
  {
    return A(row,col);
  }

  template <typename VectorType>
  real& operator()(VectorType& v, uint i)
  {
    return v[i];
  }

  template <typename VectorType>
  uint GetDimension(VectorType& v)
  {
    return v.GetSize();
  }
};

}//namespace Math
