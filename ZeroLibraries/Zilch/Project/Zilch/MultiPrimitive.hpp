/**************************************************************\
* Author: Andrew Colean
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_MULTIPRIMITIVE_HPP
#define ZILCH_MULTIPRIMITIVE_HPP

namespace Zilch
{
  // Represents a type containing multiple primitives (ex. Real3 has 3 Real members)
  class ZeroShared MultiPrimitive
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    MultiPrimitive(BoundType* primitiveMemberType, size_t primitiveMemberCount);

    BoundType* PrimitiveMemberType;
    size_t PrimitiveMemberCount;
  };
}

#endif
