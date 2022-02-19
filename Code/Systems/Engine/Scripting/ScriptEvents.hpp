// MIT Licensed (see LICENSE.md).
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
} // namespace Events

class DocumentResource;

class ScriptEvent : public Event
{
public:
  ZilchDeclareType(ScriptEvent, TypeCopyMode::ReferenceType);

  ScriptEvent()
  {
    Script = nullptr;
  }
  CodeLocation Location;
  String Message;
  /// The document resource that should be displayed due to an error
  DocumentResource* Script;
};

} // namespace Zero
