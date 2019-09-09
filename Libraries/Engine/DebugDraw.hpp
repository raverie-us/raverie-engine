// MIT Licensed (see LICENSE.md).
#pragma once

#define AddDeclarations(DebugObjectType)                                                                               \
  static void Add(Debug::DebugObjectType& debugObject);                                                                \
  static void Add(uint spaceId, Debug::DebugObjectType& debugObject);                                                  \
  static void Add(Space* space, Debug::DebugObjectType& debugObject);

namespace Zero
{

class DebugDraw
{
public:
  ZilchDeclareType(DebugDraw, TypeCopyMode::ReferenceType);
  typedef DebugDraw self_type;

#define ZeroDebugPrimitive(X) AddDeclarations(X);
#include "Geometry/DebugPrimitives.inl"
#undef ZeroDebugPrimitive
};

} // namespace Zero

#undef AddDeclarations
