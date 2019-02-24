// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Zilch
{
ZilchDefineType(MultiPrimitive, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
}

MultiPrimitive::MultiPrimitive(BoundType* primitiveMemberType, size_t primitiveMemberCount) :
    PrimitiveMemberType(primitiveMemberType),
    PrimitiveMemberCount(primitiveMemberCount)
{
}
} // namespace Zilch
