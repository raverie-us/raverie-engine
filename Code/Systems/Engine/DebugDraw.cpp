// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#define AddDefinitions(DebugObjectType)                                                                                \
  void DebugDraw::Add(Debug::DebugObjectType& debugObject)                                                             \
  {                                                                                                                    \
    if (&debugObject != nullptr)                                                                                       \
      gDebugDraw->Add(debugObject);                                                                                    \
    else                                                                                                               \
      DoNotifyException("Null debug object", "You must initialize the debug object.");                                 \
  }                                                                                                                    \
  void DebugDraw::Add(uint spaceId, Debug::DebugObjectType& debugObject)                                               \
  {                                                                                                                    \
    if (&debugObject != nullptr)                                                                                       \
      gDebugDraw->Add(spaceId, debugObject);                                                                           \
    else                                                                                                               \
      DoNotifyException("Null debug object", "You must initialize the debug object.");                                 \
  }                                                                                                                    \
  void DebugDraw::Add(Space* space, Debug::DebugObjectType& debugObject)                                               \
  {                                                                                                                    \
    if (&debugObject != nullptr)                                                                                       \
      gDebugDraw->Add(space->GetRuntimeId(), debugObject);                                                             \
    else                                                                                                               \
      DoNotifyException("Null debug object", "You must initialize the debug object.");                                 \
  }

#define AddBindings(DebugObjectType)                                                                                   \
  RaverieBindOverloadedMethod(Add, (void (*)(Debug::DebugObjectType&)));                                                 \
  RaverieFullBindMethod(builder, type, RaverieSelf::Add, (void (*)(Space*, Debug::DebugObjectType&)), "Add", "space, shape")

namespace Raverie
{
RaverieDefineType(DebugDraw, builder, type)
{
  RaverieBindDocumented();
#define RaverieDebugPrimitive(X) AddBindings(X);
#include "Foundation/Geometry/DebugPrimitives.inl"
#undef RaverieDebugPrimitive
}

#define RaverieDebugPrimitive(X) AddDefinitions(X);
#include "Foundation/Geometry/DebugPrimitives.inl"
#undef RaverieDebugPrimitive

} // namespace Raverie

#undef AddDefinitions
