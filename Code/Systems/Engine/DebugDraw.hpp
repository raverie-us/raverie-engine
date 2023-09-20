// MIT Licensed (see LICENSE.md).
#pragma once

#define AddDeclarations(DebugObjectType)                                                                               \
  static void Add(Debug::DebugObjectType& debugObject);                                                                \
  static void Add(uint spaceId, Debug::DebugObjectType& debugObject);                                                  \
  static void Add(Space* space, Debug::DebugObjectType& debugObject);

namespace Raverie
{

class DebugDraw
{
public:
  RaverieDeclareType(DebugDraw, TypeCopyMode::ReferenceType);
  typedef DebugDraw self_type;

#define RaverieDebugPrimitive(X) AddDeclarations(X);
#include "Foundation/Geometry/DebugPrimitives.inl"
#undef RaverieDebugPrimitive
};

} // namespace Raverie

#undef AddDeclarations
