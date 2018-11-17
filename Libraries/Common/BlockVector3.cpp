///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Math
{

uint BlockVector3::GetSize() const
{
  return mBlocks.Size();
}

void BlockVector3::SetSize(uint size)
{
  mBlocks.Resize(size,Vector3::cZero);
}

Vector3& BlockVector3::operator[](uint index)
{
  ErrorIf(index > mBlocks.Size(), "Math::BlockVector3 - Subscript out of range.");
  return mBlocks[index];
}

Vector3 BlockVector3::operator[](uint index) const
{
  ErrorIf(index > mBlocks.Size(), "Math::BlockVector3 - Subscript out of range.");
  return mBlocks[index];
}

real& BlockVector3::GlobalIndex(uint index)
{
  int blockIndex = index / 3;
  int finalIndex = index % 3;

  return mBlocks[blockIndex][finalIndex];
}

void BlockVector3::operator*=(real rhs)
{
  for(uint i = 0; i < mBlocks.Size(); ++i)
    mBlocks[i] *= rhs;
}

void BlockVector3::operator+=(const BlockVector3& rhs)
{
  ErrorIf(rhs.mBlocks.Size() != mBlocks.Size(),
          "Cannot add two block vectors of different dimensions");

  for(uint i = 0; i < mBlocks.Size(); ++i)
    mBlocks[i] += rhs.mBlocks[i];
}

void BlockVector3::operator-=(const BlockVector3& rhs)
{
  ErrorIf(rhs.mBlocks.Size() != mBlocks.Size(),
          "Cannot subtract two block vectors of different dimensions");

  for(uint i = 0; i < mBlocks.Size(); ++i)
    mBlocks[i] -= rhs.mBlocks[i];
}

real BlockVector3::Dot(const BlockVector3& rhs) const
{
  ErrorIf(rhs.mBlocks.Size() != mBlocks.Size(),
          "Cannot perform the dot product between two block "
          "vectors of different dimensions");

  real result = real(0.0);
  for(uint i = 0; i < mBlocks.Size(); ++i)
    result += Math::Dot(mBlocks[i], rhs.mBlocks[i]);
  return result;
}

void BlockVector3::Scale(const BlockVector3& rhs, BlockVector3& out) const
{
  ErrorIf(rhs.mBlocks.Size() != mBlocks.Size(),
          "Cannot perform vector scale between two block "
          "vectors of different dimensions");

  for(uint i = 0; i < mBlocks.Size(); ++i)
    out[i] = mBlocks[i] * rhs[i];
}

uint BlockMatrix3::GetSize() const
{
  return mBlocks.Size();
}

void BlockMatrix3::SetSize(uint size)
{
  mBlocks.Resize(size);

  Matrix3 zero;
  zero.ZeroOut();
  for(uint i = 0; i < size; ++i)
    mBlocks[i].Resize(size,zero);
}

Matrix3 BlockMatrix3::operator()(uint row, uint col) const
{
  ErrorIf(row > mBlocks.Size() || col > mBlocks.Size(),
          "Math::BlockMatrix3 - Subscript out of range.");
  
  return mBlocks[row][col];
}

Matrix3& BlockMatrix3::operator()(uint row, uint col)
{
  ErrorIf(row > mBlocks.Size() || col > mBlocks.Size(),
          "Math::BlockMatrix3 - Subscript out of range.");

  return mBlocks[row][col];
}

real& BlockMatrix3::GlobalIndex(uint row, uint col)
{
  uint size = mBlocks.Size() * 3;
  ErrorIf(row > size || col > size,
    "Math::BlockMatrix3 - Subscript out of range.");

  int blockRow = row / 3;
  int blockCol = col / 3;
  int finalRow = row % 3;
  int finalCol = col % 3;
  return mBlocks[blockRow][blockCol](finalRow,finalCol);
}

BlockMatrix3 BlockMatrix3::Transposed() const
{
  uint size = mBlocks.Size();
  BlockMatrix3 result;
  result.SetSize(size);
  for(uint j = 0; j < size; ++j)
  {
    for(uint i = 0; i < size; ++i)
    {
      result(i,j) = (*this)(j,i).Transposed();
    }
  }
  return result;
}

BlockMatrix3 BlockMatrix3::Transform(const BlockMatrix3& rhs) const
{
  uint size = mBlocks.Size();
  BlockMatrix3 result;
  result.SetSize(size);
  for(uint j = 0; j < size; ++j)
  {
    for(uint i = 0; i < size; ++i)
    {
      for(uint x = 0; x < size; ++x)
      {
        Matrix3 left = (*this)(i,x);
        Matrix3 right = rhs(x,j);
        result(i,j) += left * right;
      }
    }
  }
  return result;
}

void BlockMatrix3::Transform(const BlockVector3& rhs, BlockVector3& out) const
{
  out.SetSize(rhs.GetSize());

  for(uint i = 0; i < mBlocks.Size(); ++i)
  {
    const Cells& cell = mBlocks[i];
    Vector3 sum = Vector3::cZero;
    
    for(uint j = 0; j < cell.Size(); ++j)
    {
      Matrix3 m = cell[j];
      Vector3 v = rhs[j];
      sum += Math::Transform(m,v);
    }

    out[i] = sum;
  }
}

}//namespace Math
