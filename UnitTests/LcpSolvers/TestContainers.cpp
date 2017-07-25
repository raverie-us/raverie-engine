///////////////////////////////////////////////////////////////////////////////
///
///  \file TestContainers.cpp
///  Some containers used in testing LCP solvers.
///  
///  Authors: Joshua Davis
///  Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "TestContainers.hpp"

void SparseVector::Set(uint index, Vec3Param value)
{
  uint actualIndex = uint(-1);
  for(uint i = 0; i < mCells.Size(); ++i)
  {
    if(mCells[i].mIndex == index)
    {
      actualIndex = i;
      break;
    }
  }

  if(actualIndex >= mCells.Size())
  {
    mCells.PushBack(SparceCell(index,value));
    return;
  }
  mCells[actualIndex].mValue = value;
}

Vec3 SparseVector::Get(uint index) const 
{
  uint actualIndex = uint(-1);
  for(uint i = 0; i < mCells.Size(); ++i)
  {
    if(mCells[i].mIndex == index)
    {
      actualIndex = i;
      break;
    }
  }

  if(actualIndex >= mCells.Size())
    return Vec3::cZero;

  return mCells[actualIndex].mValue;
}

Vec3 SparseVector::operator[](uint index) const
{
  return Get(index);
}

void SparseBlockMatrix::Set(uint r, uint c, Mat3Param value)
{
  uint actualRow = uint(-1);
  uint actualColumn = uint(-1);
  for(uint i = 0; i < mRows.Size(); ++i)
  {
    if(mRows[i].mIndex == r)
    {
      actualRow = i;
      break;
    }
  }
  if(actualRow >= mRows.Size())
  {
    SparseRow row(r);
    row.mCells.PushBack(SparceCell(c,value));
    mRows.PushBack(row);
    return;
  }

  for(uint i = 0; i < mRows[actualRow].mCells.Size(); ++i)
  {
    if(mRows[actualRow].mCells[i].mIndex == c)
    {
      actualColumn = i;
      break;
    }
  }

  if(actualColumn >= mRows[actualRow].mCells.Size())
  {
    mRows[actualRow].mCells.PushBack(SparceCell(c,value));
    return;
  }

  mRows[actualRow].mCells[actualColumn].mValue = value;
}

Mat3 SparseBlockMatrix::Get(uint r, uint c) const 
{
  uint actualRow = uint(-1);
  uint actualColumn = uint(-1);
  for(uint i = 0; i < mRows.Size(); ++i)
  {
    if(mRows[i].mIndex == r)
    {
      actualRow = i;
      break;
    }
  }
  if(actualRow >= mRows.Size())
    return Mat3().ZeroOut();

  for(uint i = 0; i < mRows[actualRow].mCells.Size(); ++i)
  {
    if(mRows[actualRow].mCells[i].mIndex == c)
    {
      actualColumn = i;
      break;
    }
  }

  if(actualColumn >= mRows[actualRow].mCells.Size())
    return Mat3().ZeroOut();

  return mRows[actualRow].mCells[actualColumn].mValue;
}

SparseVector SparseBlockMatrix::Transform(const SparseVector& rhs) const 
{
  SparseVector result;

  for(uint row = 0; row < mRows.Size(); ++row)
  {
    const SparseRow& sRow = mRows[row];
    uint actualRow = sRow.mIndex;

    Vec3 cellVal = Vec3::cZero;

    for(uint col = 0; col < sRow.mCells.Size(); ++col)
    {
      const SparceCell& sCell = sRow.mCells[col];
      uint actualCol = sCell.mIndex;

      cellVal += Math::Transform(sCell.mValue,rhs.Get(actualCol));
    }
    result.Set(actualRow,cellVal);
  }
  return result;
}

SparseVector SparceBlockCgPolicy::Add(const SparseVector& lhs, const SparseVector& rhs)
{
  SparseVector result;
  for(uint i = 0; i < lhs.mCells.Size(); ++i)
    result.Set(lhs.mCells[i].mIndex,lhs.mCells[i].mValue);

  for(uint i = 0; i < rhs.mCells.Size(); ++i)
    result.Set(rhs.mCells[i].mIndex, result.Get(i) + rhs.mCells[i].mValue);

  return result;
}

SparseVector SparceBlockCgPolicy::Subtract(const SparseVector& lhs, const SparseVector& rhs)
{
  SparseVector result;
  for(uint i = 0; i < lhs.mCells.Size(); ++i)
    result.Set(lhs.mCells[i].mIndex,lhs.mCells[i].mValue);

  for(uint i = 0; i < rhs.mCells.Size(); ++i)
    result.Set(rhs.mCells[i].mIndex, result.Get(i) - rhs.mCells[i].mValue);

  return result;
}

real SparceBlockCgPolicy::Dot(const SparseVector& lhs, const SparseVector& rhs)
{
  real result = 0;
  for(uint i = 0; i < lhs.mCells.Size(); ++i)
    result += Math::Dot(lhs.mCells[i].mValue,rhs.Get(lhs.mCells[i].mIndex));
  return result;
}

SparseVector SparceBlockCgPolicy::Scale(const SparseVector& lhs, real rhs)
{
  SparseVector result;
  for(uint i = 0; i < lhs.mCells.Size(); ++i)
    result.Set(lhs.mCells[i].mIndex,lhs.mCells[i].mValue * rhs);
  return result;
}

SparseVector SparceBlockCgPolicy::Transform(const SparseBlockMatrix& mat, const SparseVector& vec)
{
  return mat.Transform(vec);
}
