// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{

// User data for a single matrix so that functions can be generic
// to matrices of different sizes and types
class MatrixUserData
{
public:
  MatrixUserData() : SizeX(4), SizeY(4), ElementTypeIndex(0)
  {
  }

  MatrixUserData(size_t sizeX, size_t sizeY, size_t elementTypeIndex) :
      SizeX(sizeX),
      SizeY(sizeY),
      ElementTypeIndex(elementTypeIndex)
  {
  }

  size_t SizeX;
  size_t SizeY;
  // What kind of matrix this is (Real, Integer, Boolean).
  // This is an index into Core::MatrixElementTypes.
  size_t ElementTypeIndex;
};

// Create all of the matrix types and their functions on the math class
void CreateMatrixTypes(LibraryBuilder& builder);

} // namespace Raverie

