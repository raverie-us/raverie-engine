// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(DebuggerPaused);
DefineEvent(DebuggerResumed);
DefineEvent(SyntaxError);
DefineEvent(UnhandledException);
} // namespace Events

ZilchDefineType(ScriptEvent, builder, type)
{
}

} // namespace Zero
