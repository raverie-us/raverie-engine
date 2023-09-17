// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(ScriptInitialize);
} // namespace Events

ZilchDefineType(TextEvent, builder, type)
{
  ZilchBindFieldProperty(Text);
}

ZilchDefineType(TextErrorEvent, builder, type)
{
}

ZilchDefineType(BlockingTaskEvent, builder, type)
{
}

} // namespace Zero
