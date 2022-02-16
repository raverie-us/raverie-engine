// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
DataBlock::DataBlock() : Data(nullptr), Size(0)
{
}

DataBlock::DataBlock(byte* data, size_t size) : Data(data), Size(size)
{
}

DataBlock::operator bool()
{
  return Data != nullptr;
}

bool DataBlock::operator==(DataBlock& lhs)
{
  if (Size != lhs.Size)
    return false;

  return memcmp(Data, lhs.Data, Size) == 0;
}

bool DataBlock::operator!=(DataBlock& lhs)
{
  return !(*this == lhs);
}

size_t DataBlock::Hash()
{
  return HashString((char*)Data, Size);
}

} // namespace Zero
