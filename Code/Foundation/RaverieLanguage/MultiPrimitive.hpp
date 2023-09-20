// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{
// Represents a type containing multiple primitives (ex. Real3 has 3 Real
// members)
class MultiPrimitive
{
public:
  RaverieDeclareType(MultiPrimitive, TypeCopyMode::ReferenceType);

  MultiPrimitive(BoundType* primitiveMemberType, size_t primitiveMemberCount);

  BoundType* PrimitiveMemberType;
  size_t PrimitiveMemberCount;
};
} // namespace Raverie
