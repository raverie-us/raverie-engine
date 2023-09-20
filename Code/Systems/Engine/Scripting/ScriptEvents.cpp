// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
DefineEvent(DebuggerPaused);
DefineEvent(DebuggerResumed);
DefineEvent(SyntaxError);
DefineEvent(UnhandledException);
} // namespace Events

RaverieDefineType(ScriptEvent, builder, type)
{
}

} // namespace Raverie
