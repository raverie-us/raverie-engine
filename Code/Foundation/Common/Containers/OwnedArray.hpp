// MIT Licensed (see LICENSE.md).

#pragma once
#include "Array.hpp"

namespace Raverie
{

// OwnedArray

template <typename type, typename Allocator = DefaultAllocator, typename value_tt = StandardTraits<type>>
class OwnedArray : public Array<type, Allocator, value_tt>
{
public:
  // Deletes all elements within the array.
  ~OwnedArray()
  {
    for (size_t i = 0; i < this->Size(); ++i)
    {
      delete (*this)[i];
    }
  }
};

} // namespace Raverie
