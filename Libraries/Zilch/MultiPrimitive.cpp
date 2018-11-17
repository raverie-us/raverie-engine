/**************************************************************\
* Author: Andrew Colean
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Precompiled.hpp"

namespace Zilch
{
  ZilchDefineType(MultiPrimitive, builder, type)
  {
    type->HandleManager = ZilchManagerId(PointerManager);
  }

  MultiPrimitive::MultiPrimitive(BoundType* primitiveMemberType, size_t primitiveMemberCount)
    : PrimitiveMemberType(primitiveMemberType),
      PrimitiveMemberCount(primitiveMemberCount)
  {
  }
}
