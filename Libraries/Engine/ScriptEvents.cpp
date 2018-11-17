///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(DebuggerPaused);
  DefineEvent(DebuggerResumed);
  DefineEvent(SyntaxError);
  DefineEvent(UnhandledException);
}//namespace Events

ZilchDefineType(ScriptEvent, builder, type)
{
}

}//namespace Zero
