///////////////////////////////////////////////////////////////////////////////
///
/// \file ExtendableMath.hpp
/// 
/// Authors: Joshua Davis
/// Copyright 2014, DigiPen Institute of Technology
///
//////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Math
{

//-------------------------------------------------------------------ExtendableVector
struct ZeroShared ExtendableVector
{
  void Resize(uint size);

  real& operator[](uint index);
  real operator[](uint index) const;

  uint GetSize() const;

  uint mSize;
  Zero::Array<real> mData;
};

//-------------------------------------------------------------------ExtendableMatrix
struct ZeroShared ExtendableMatrix
{
  void Resize(uint sizeX, uint sizeY);

  real& operator()(uint y, uint x);
  real operator()(uint y, uint x) const;

  uint mSizeX;
  uint mSizeY;
  Zero::Array<real> mData;
};

//-------------------------------------------------------------------FixedVector
// A vector who's max size is compile-time but who's
// working size can be changed up to the fixed size.
// Currently used in position correction.
template <typename DataType, size_t FixedSize>
struct ZeroSharedTemplate FixedVector
{
  void Resize(size_t size)
  {
    if(size > FixedSize)
    {
      Error("Cannot set size greater than the fixed size.");
      size = FixedSize;
    }

    mSize = size;
  }

  DataType& operator[](uint index)
  {
    ErrorIf(index >= mSize, "Access array out of bounds");
    return mData[index];
  }
  DataType operator[](uint index) const
  {
    ErrorIf(index >= mSize, "Access array out of bounds");
    return mData[index];
  }

  size_t GetSize() const
  {
    return mSize;
  }

  size_t mSize;
  DataType mData[FixedSize];
};

//-------------------------------------------------------------------FixedMatrix
template <size_t SizeX, size_t SizeY>
struct ZeroSharedTemplate FixedMatrix
{
  real& operator()(uint y, uint x)
  {
    ErrorIf(y > SizeY || x > SizeX, "Access matrix out of bounds");
    return mData[x + SizeX * y];
  }
  real operator()(uint y, uint x) const
  {
    ErrorIf(y > SizeY || x > SizeX, "Access matrix out of bounds");
    return mData[x + SizeX * y];
  }

  real mData[SizeX * SizeY];
};

}//namespace Math
