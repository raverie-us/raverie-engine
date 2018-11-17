///////////////////////////////////////////////////////////////////////////////
///
/// \file ScriptEvents.hpp
/// Declaration of the DebugValue and ScriptDebugEngine classes.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Events sent by the DebugEngine.
namespace Events
{
  DeclareEvent(DebuggerPaused);
  DeclareEvent(DebuggerResumed);
  DeclareEvent(SyntaxError);
  DeclareEvent(UnhandledException);
}//namespace Events

//---------------------------------------------------------------- Code Location
class DocumentResource;

//----------------------------------------------------------- Debug Engine Event
class ScriptEvent : public Event
{
public:
  ZilchDeclareType(ScriptEvent, TypeCopyMode::ReferenceType);

  ScriptEvent() { Script = nullptr; }
  CodeLocation Location;
  String Message;
  /// The document resource that should be displayed due to an error
  DocumentResource* Script;
};

}//namespace Zero
