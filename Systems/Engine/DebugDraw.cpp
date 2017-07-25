////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Nathan Carlson
/// Copyright 2016, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#define AddDefinitions(DebugObjectType)                                  \
  void DebugDraw::Add(Debug::DebugObjectType& debugObject)               \
  {                                                                      \
    if (&debugObject != nullptr)                                         \
      gDebugDraw->Add(debugObject);                                      \
    else                                                                 \
      DoNotifyException("Null debug object",                             \
                        "You must initialize the debug object.");        \
  }                                                                      \
  void DebugDraw::Add(uint spaceId, Debug::DebugObjectType& debugObject) \
  {                                                                      \
    if(&debugObject != nullptr)                                          \
      gDebugDraw->Add(spaceId, debugObject);                             \
    else                                                                 \
      DoNotifyException("Null debug object",                             \
                        "You must initialize the debug object.");        \
  }                                                                      \
  void DebugDraw::Add(Space* space, Debug::DebugObjectType& debugObject) \
  {                                                                      \
    if(&debugObject != nullptr)                                          \
      gDebugDraw->Add(space->GetRuntimeId(), debugObject);               \
    else                                                                 \
      DoNotifyException("Null debug object",                             \
                        "You must initialize the debug object.");        \
  }


#define AddBindings(DebugObjectType)                                                                                     \
  ZilchBindOverloadedMethod(Add, (void (*)(Debug::DebugObjectType&)));                                                   \
  ZilchFullBindMethod(builder, type, ZilchSelf::Add, (void(*)(Space*, Debug::DebugObjectType&)), "Add", "space, shape")

namespace Zero
{
ZilchDefineType(DebugDraw, builder, type)
{
  #define ZeroDebugPrimitive(X) AddBindings(X);
  #include "Geometry/DebugPrimitives.inl"
  #undef ZeroDebugPrimitive
}

#define ZeroDebugPrimitive(X) AddDefinitions(X);
#include "Geometry/DebugPrimitives.inl"
#undef ZeroDebugPrimitive

} // namespace Zero

#undef AddDefinitions
