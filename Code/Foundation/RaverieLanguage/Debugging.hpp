// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{
namespace Events
{
// Sent repeatedly when the program is frozen by the debugger (a breakpoint is
// hit, or it is paused) In general no logic that can affect the state of
// debugging should ever be run within this event handler However, it can be
// used to draw screen overlays and other indicators to the user
RaverieDeclareEvent(DebuggingPauseUpdate, DebuggerEvent);

// Sent when we first pause execution (such as entering a breakpoint or clicking
// pause)
RaverieDeclareEvent(DebuggingPause, DebuggerEvent);

// Sent when we resume execution after being paused
// Note that stepping a single line will resume and then pause again
RaverieDeclareEvent(DebuggingResume, DebuggerEvent);

// Sent when we a breakpoint is added (either locally or by remote client)
RaverieDeclareEvent(DebuggingBreakpointedAdded, DebuggerEvent);

// Sent when we a breakpoint is added (either locally or by remote client)
RaverieDeclareEvent(DebuggingBreakpointedRemoved, DebuggerEvent);

// Sent when we skip a breakpoint or a step due to an externally defined reason
RaverieDeclareEvent(DebuggingBreakNotAllowed, DebuggerEvent);
} // namespace Events

// When the debugger pauses or resumes, we send this event out
class DebuggerEvent : public EventData
{
public:
  RaverieDeclareType(DebuggerEvent, TypeCopyMode::ReferenceType);

  // The debugger we're currently paused inside of
  Debugger* RunningDebugger;

  // The state we're currently paused inside of (last running)
  ExecutableState* State;

  // The location of where we're at in script (generally where we're paused)
  const CodeLocation* Location;
};

// When the debugger needs to send a message
class DebuggerTextEvent : public DebuggerEvent
{
public:
  RaverieDeclareType(DebuggerTextEvent, TypeCopyMode::ReferenceType);

  // The text/message that the debugger is sending
  String Text;
};

// Returned from querying an expression in the debugger
class QueryResult
{
public:
  QueryResult();

  String Name;
  String Value;
  bool Expandable;
};

namespace DebuggerAction
{
enum Enum
{
  Resume,
  Pause,
  StepOver,
  StepIn,
  StepOut
};
}

// The base debugger handles placing breakpoints and signals events when the
// breakpoints are hit. It also handles pausing and stepping.
class Debugger : public EventHandler
{
public:
  // sends DebuggingPauseUpdate : DebuggerEvent;
  // sends DebuggingPause : DebuggerEvent;
  // sends DebuggingResume : DebuggerEvent;
  // sends DebuggingBreakpointedAdded : DebuggerEvent;
  // sends DebuggingBreakpointedRemoved : DebuggerEvent;
  // sends DebuggingBreakNotAllowed : DebuggerTextEvent;

  // Constructor
  Debugger();

  // Destructor
  ~Debugger();

  // Pauses the call stack for whatever is currently executing in Raverie.
  void Pause();

  // If the debugger is currently paused/breakpointed this will resume
  // operation.
  void Resume();

  // If the debugger is currently paused/breakpointed this will step over the
  // current line. If the debugger is not paused this will pause it.
  void StepOver();

  // If the debugger is currently paused/breakpointed this will step into the
  // current line if it is a function, or over if there is nothing to step into.
  // If the debugger is not paused this will pause it.
  void StepIn();

  // If the debugger is currently paused/breakpointed step out of the current
  // function. If there is no Raverie function above this point, this will resume
  // execution. If the debugger is not paused this will pause it.
  void StepOut();

  // Attempts to set a breakpoint at a given code location. Note that the
  // library must be added for this to succeed otherwise the debugger wont know
  // if it's a valid location.
  bool SetBreakpoint(StringParam origin, size_t line, bool breakpoint);

  // Attempts to set a breakpoint at a given code location. Note that the
  // library must be added for this to succeed otherwise the debugger wont know
  // if it's a valid location.
  bool SetBreakpoint(const CodeLocation& location, bool breakpoint);

  // Get if a breakpoint exists at a given location.
  bool HasBreakpoint(StringParam origin, size_t line);

  // Get if a breakpoint exists at a given location.
  bool HasBreakpoint(const CodeLocation& location);

  // Clear breakpoints for a given document/origin
  void ClearBreakpoints(StringParam origin);

  // Clear all breakpoints for all documents/origins
  void ClearAllBreakpoints();

  // Checks if a type has any debuggable properties (expandable)
  static bool HasDebuggableProperties(Type* type);

  // Attempts to find a code entry by origin
  CodeEntry* FindCodeEntry(StringParam origin);

  // Evaluates an expression and outputs the value of the expression as well as
  // sub-properties. Returns the actual value of the expression an an Any.
  Any QueryExpression(StringParam expression, Array<QueryResult>& results);

protected:
  // Lets us know when we stopped an execution point
  virtual void DebuggingPause(const CodeLocation& codeLocation);

  // Lets us know when we stopped an execution point and is updating
  virtual void DebuggingPauseUpdate(const CodeLocation& codeLocation);

  // When we resume execution, we want to tell the remote client to clear the
  // execution point
  virtual void DebuggingResume();

  // Callbacks from the state:
  // Every time the executable state steps into an opcode, this function is
  // called
  virtual void OnOpcodePreStep(OpcodeEvent* e);

  // Every time the executable state steps into a function, this function is
  // called
  virtual void OnEnterFunction(OpcodeEvent* e);

  // Every time the executable state steps out of a function, this function is
  // called
  virtual void OnExitFunction(OpcodeEvent* e);

  // Every time the executable state steps out of a function, this function is
  // called
  virtual void OnException(ExceptionEvent* e);

  // Whenever we print anything out using the console, we want to know about it
  virtual void OnConsoleWrite(ConsoleEvent* event);

  // Called after state changes and when we're paused on a breakpoint
  virtual void Update();

private:
  // The break loop will pause all execution on this thread, only processing
  // debugger messages
  void Breakpoint(const CodeLocation& codeLocation);

  // Attach callbacks to the executable state
  void AttachCallbacks();

  // Detach callbacks from the executable state
  void DetachCallbacks();

  // Attaches or detaches depending on if the Action or we have breakpoints.
  void UpdateAttach();

public:
  static bool Enabled;

  // As we walk over lines of code (callbacks from any running ExecutableState)
  // we will check to see if the line exists in this breakpoints map
  // The map maps from origin to line numbers
  HashMap<String, HashSet<size_t>> Breakpoints;

  // Whether we've attached to the executable state
  bool IsAttached;

  // If this string is not empty we will send an event with this message when we
  // hit the breakpoint and continue execution
  String DoNotAllowBreakReason;

  // If we're already in a breakpoint (prevents breaking within a breakpoint
  // when we send external updates)
  bool IsBreakpointed;

  // The last action that was queued up by the debugger for the current state
  DebuggerAction::Enum Action;

private:
  // Store the last location id and line
  CodeLocation LastLocation;

  // The last call stack position (how deep we were)
  size_t LastCallStackDepth;

  // When we're doing stepping, we need to save the last location here (but not
  // update it with each opcode step)
  CodeLocation StepLocation;

  // The call stack depth where we're stepping out of (state context relative
  // operations)
  size_t StepOutOverCallStackDepth;

  // The libraries whose code we are currently viewing
  Array<LibraryRef> Libraries;

  // If the last opcode we hit was a dis-allowed breakpoint, then we use this to
  // make sure we don't keep sending events over and over for every opcode we
  // execute (just do it the first time until we clear this flag)
  bool WasLastDisallowedBreak;
};

// Every platform should define an error handler
bool DebugErrorHandler(ErrorSignaler::ErrorData& errorData);
} // namespace Raverie
