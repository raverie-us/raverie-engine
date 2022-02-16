// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Math
{

void ExtendableVector::Resize(uint size)
{
  mSize = size;
  mData.Resize(mSize);
}

real& ExtendableVector::operator[](uint index)
{
  return mData[index];
}

real ExtendableVector::operator[](uint index) const
{
  return mData[index];
}

uint ExtendableVector::GetSize() const
{
  return mSize;
}

void ExtendableMatrix::Resize(uint sizeX, uint sizeY)
{
  mSizeX = sizeX;
  mSizeY = sizeY;
  mData.Resize(mSizeX * mSizeY);
}

real& ExtendableMatrix::operator()(uint y, uint x)
{
  return mData[x + mSizeX * y];
}

real ExtendableMatrix::operator()(uint y, uint x) const
{
  return mData[x + mSizeX * y];
}

} // namespace Math
