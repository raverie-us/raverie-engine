// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

/// A debug step is a collection of debug drawing shapes that should all
/// be drawn together. This is an array of each debug draw type.
class DebugDrawStep
{
public:
#define ZeroDebugPrimitive(DebugType)                                                                                  \
  Array<Debug::DebugType> m##DebugType##List;                                                                          \
  void Add(const Debug::DebugType& value)                                                                              \
  {                                                                                                                    \
    m##DebugType##List.PushBack(value);                                                                                \
  }
#include "DebugPrimitives.inl"
#undef ZeroDebugPrimitive

  void Draw();
};

/// A stack of debug draw steps. This maintains a collection of sub-steps for
/// debug drawing to make it easier to debug an algorithm without having to
/// early-out or make it re-entrant. This does come at a severe memory overhead
/// so some algorithms can fail when large enough.
class DebugDrawStack
{
public:
  void Add(const DebugDrawStep& step);
  void Clear();
  void Draw(int index);

  // The list of all steps to be drawn
  Array<DebugDrawStep> mSteps;

  // Sometimes it can be difficult/annoying to store a bunch of information
  // about how a sub-portion of an algorithm should draw, such as the location
  // of any debug text. To work around this, base debug shapes are stored here
  // that can be modified by the top level of the algorithm. Any sub-step should
  // clone a debug shape and only set the relevant properties, this leaves all
  // pre-establish properties the same.
#define ZeroDebugPrimitive(DebugType) Debug::DebugType mBase##DebugType;
#include "DebugPrimitives.inl"
#undef ZeroDebugPrimitive
};

} // namespace Zero
