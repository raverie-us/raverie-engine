///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Math
{

struct ZeroShared BlockVector3
{
  uint GetSize() const;
  void SetSize(uint size);

  Vector3 operator[](uint index) const;
  Vector3& operator[](uint index);

  real& GlobalIndex(uint index);
  
  //Binary Assignment Operators (reals)
  void operator*=(real rhs);

  //Binary Assignment Operators (vectors)
  void operator+=(const BlockVector3& rhs);
  void operator-=(const BlockVector3& rhs);

  real Dot(const BlockVector3& rhs) const;
  void Scale(const BlockVector3& rhs, BlockVector3& out) const; 

  Zero::Array<Vector3> mBlocks;
};

struct ZeroShared BlockMatrix3
{
  uint GetSize() const;
  void SetSize(uint size);

  Matrix3 operator()(uint row, uint col) const;
  Matrix3& operator()(uint row, uint col);

  real& GlobalIndex(uint row, uint col);

  BlockMatrix3 Transposed() const;
  BlockMatrix3 Transform(const BlockMatrix3& rhs) const;
  void Transform(const BlockVector3& rhs, BlockVector3& out) const; 

  typedef Zero::Array<Matrix3> Cells;
  typedef Zero::Array<Cells> Rows;
  Rows mBlocks;
};

struct ZeroShared BlockCgPolicy
{
  real& operator()(BlockMatrix3& A, uint row, uint col)
  {
    return A.GlobalIndex(row,col);
  }

  real& operator()(BlockVector3& v, uint i)
  {
    return v.GlobalIndex(i);
  }

  uint GetDimension(BlockVector3& v)
  {
    return v.GetSize() * 3;
  }

  BlockVector3 Add(const BlockVector3& lhs, const BlockVector3& rhs)
  {
    BlockVector3 result = lhs;
    result += rhs;
    return result;
  }

  BlockVector3 Subtract(const BlockVector3& lhs, const BlockVector3& rhs)
  {
    BlockVector3 result = lhs;
    result -= rhs;
    return result;
  }

  real Dot(const BlockVector3& lhs, const BlockVector3& rhs)
  {
    return lhs.Dot(rhs);
  }

  BlockVector3 Scale(const BlockVector3& lhs, real rhs)
  {
    BlockVector3 result = lhs;
    result *= rhs;
    return result;
  }

  BlockVector3 Transform(const BlockMatrix3& mat, const BlockVector3& vec)
  {
    BlockVector3 result;
    mat.Transform(vec,result);
    return result;
  }

  //v1 * scalar + v2
  //out is assumed to only ever be aliased as v2
  void MultiplyAdd(const BlockVector3& v1, real scalar, BlockVector3& v2, BlockVector3* out)
  {
    uint size = v1.GetSize();
    for(uint i = 0; i < size; ++i)
    {
      (*out)[i] = v2[i] + v1[i] * scalar;
    }
  }

  //-(v1 * scalar - v2) = v2 - v1 * scalar
  //out is assumed to only ever be aliased as v2
  void MultiplySubtract(const BlockVector3& v1, real scalar, BlockVector3& v2, BlockVector3* out)
  {
    uint size = v1.GetSize();
    for(uint i = 0; i < size; ++i)
    {
      (*out)[i] = v2[i] - v1[i] * scalar;
    }
  }

  //-(mat * v1 - v2) = v2 - mat * v1
  //out is assumed to only ever be aliased as v2
  void NegativeTransformSubtract(const BlockMatrix3& mat, const BlockVector3& v1, BlockVector3& v2, BlockVector3* out)
  {
    uint size = v1.GetSize();
    for(uint i = 0; i < size; ++i)
    {
      Vector3 sum = Vector3::cZero;

      for(uint j = 0; j < size; ++j)
      {
        Matrix3 m = mat(i,j);
        Vector3 v = v1[j];
        sum += Math::Transform(m,v);
      }
      (*out)[i] = v2[i] - sum;
    }
  }
};

}//namespace Math
