// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
DefineEvent(ScriptInitialize);
} // namespace Events

RaverieDefineType(TextEvent, builder, type)
{
  RaverieBindFieldProperty(Text);
}

RaverieDefineType(TextErrorEvent, builder, type)
{
}

RaverieDefineType(BlockingTaskEvent, builder, type)
{
}

} // namespace Raverie
