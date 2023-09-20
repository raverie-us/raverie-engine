// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{
RaverieDefineType(MultiPrimitive, builder, type)
{
  type->HandleManager = RaverieManagerId(PointerManager);
}

MultiPrimitive::MultiPrimitive(BoundType* primitiveMemberType, size_t primitiveMemberCount) : PrimitiveMemberType(primitiveMemberType), PrimitiveMemberCount(primitiveMemberCount)
{
}
} // namespace Raverie
