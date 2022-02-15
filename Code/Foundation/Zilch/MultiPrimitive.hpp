// MIT Licensed (see LICENSE.md).

#pragma once
#ifndef ZILCH_MULTIPRIMITIVE_HPP
#  define ZILCH_MULTIPRIMITIVE_HPP

namespace Zilch
{
// Represents a type containing multiple primitives (ex. Real3 has 3 Real
// members)
class ZeroShared MultiPrimitive
{
public:
  ZilchDeclareType(MultiPrimitive, TypeCopyMode::ReferenceType);

  MultiPrimitive(BoundType* primitiveMemberType, size_t primitiveMemberCount);

  BoundType* PrimitiveMemberType;
  size_t PrimitiveMemberCount;
};
} // namespace Zilch

#endif
