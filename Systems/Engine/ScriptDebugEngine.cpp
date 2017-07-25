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
  DefineEvent(DebugBreak);
  DefineEvent(DebugException);
  DefineEvent(SyntaxError);
  DefineEvent(UnhandledException);
}//namespace Events


ZilchDefineType(DebugEngineEvent, builder, type)
{
}

}//namespace Zero
