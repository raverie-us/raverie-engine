///////////////////////////////////////////////////////////////////////////////
///
///  \file TestContainers.hpp
///  Some containers used in testing LCP solvers.
///  
///  Authors: Joshua Davis
///  Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "LcpStandard.hpp"

/// A Vector of Vec3's that is variable size but only stores the items it has
/// and their corresponding index. Just used for testing.
struct SparseVector
{
  struct SparceCell
  {
    SparceCell() {}
    SparceCell(uint index, Vec3Param value) {mIndex = index; mValue = value;}
    uint mIndex;
    Vec3 mValue;
  };

  Zero::Array<SparceCell> mCells;

  ///Overrides an index if it exists, adds it if it doesn't.
  void Set(uint index, Vec3Param value);
  ///Returns the value at an index if it exists, if it doesn't return the zero vector.
  Vec3 Get(uint index) const;

  Vec3 operator[](uint index) const;
};

/// A Sparse block matrix of Mat3's. Only stores the blocks that actually exist.
/// All non-existant blocks are implicitly the zero matrix.
struct SparseBlockMatrix
{
  struct SparceCell
  {
    SparceCell() {}
    SparceCell(uint index, Mat3Param value) {mIndex = index; mValue = value;}
    uint mIndex;
    Mat3 mValue;
  };

  struct SparseRow
  {
    SparseRow() {}
    SparseRow(uint index) {mIndex = index;}
    uint mIndex;
    Zero::Array<SparceCell> mCells;
  };

  Zero::Array<SparseRow> mRows;

  ///Overrides an index if it exists, adds it if it doesn't.
  void Set(uint r, uint c, Mat3Param value);
  ///Returns the value at an index if it exists, if it doesn't return the zero vector.
  Mat3 Get(uint r, uint c) const;

  ///Transforms a sparse vector by this matrix (aka, vector times matrix)
  SparseVector Transform(const SparseVector& rhs) const;
};

/// A policy used to tell the conjugate gradient solver how to operate
/// on the sparse matrix and vector.
struct SparceBlockCgPolicy
{
  SparseVector Add(const SparseVector& lhs, const SparseVector& rhs);
  SparseVector Subtract(const SparseVector& lhs, const SparseVector& rhs);
  real Dot(const SparseVector& lhs, const SparseVector& rhs);
  SparseVector Scale(const SparseVector& lhs, real rhs);
  SparseVector Transform(const SparseBlockMatrix& mat, const SparseVector& vec);
};

struct MatrixN
{
  MatrixN(uint dimension)
  {
    mDimension = dimension;
    mData.Resize(dimension * dimension);
  }

  real& operator()(uint y, uint x)
  {
    return mData[x + y * mDimension];
  }

  MatrixN SubMatrix(uint row, uint col)
  {
    MatrixN subMat(mDimension - 1);
    for(uint i = 0; i < mDimension; ++i)
    {
      for(uint j = 0; j < mDimension; ++j)
      {
        if(i == row || j == col)
          continue;

        uint rowIndex = i > row ? i - 1 : i;
        uint colIndex = j > col ? j - 1 : j;
        subMat(rowIndex,colIndex) = (*this)(i,j);
      }
    }
    return subMat;
  }

  real Determinant()
  {
    MatrixN& m = *this;
    if(mDimension == 2)
      return m(0,0) * m(1,1) - m(0,1) * m(1,0);

    real determinant = real(0.0);
    //for(uint i = 0; i < mDimension; ++i)
    {
      for(uint j = 0; j < mDimension; ++j)
      {
        real sign = real(1.0);
        if(((0 + j) % 2) == 1)
          sign = -real(1.0);

        MatrixN subMatrix = SubMatrix(0,j);
        real subDet = subMatrix.Determinant();
        determinant += sign * m(0,j) * subDet;
      }
    }
    return determinant;
  }

  Zero::Array<real> mData;
  uint mDimension;
};
