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

ZilchDefineType(ProgressEvent, builder, type)
{
}

ZilchDefineType(BlockingTaskEvent, builder, type)
{
}

ProgressEvent::ProgressEvent()
{
  ProgressType = ProgressType::Normal;
  Percentage = 0.0f;
}

} // namespace Zero
