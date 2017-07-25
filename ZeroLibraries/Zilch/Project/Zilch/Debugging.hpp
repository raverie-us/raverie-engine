/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_DEBUGGING_HPP
#define ZILCH_DEBUGGING_HPP

namespace Zilch
{
  namespace Events
  {
    // Sent repeatedly when the program is frozen by the debugger (a breakpoint is hit, or it is paused)
    // In general no logic that can affect the state of debugging should ever be run within this event handler
    // However, it can be used to draw screen overlays and other indicators to the user
    ZilchDeclareEvent(DebuggerPauseUpdate, DebuggerEvent);

    // Sent when we first pause execution (such as entering a breakpoint or clicking pause)
    ZilchDeclareEvent(DebuggerPause, DebuggerEvent);

    // Sent when we resume execution after being paused
    // Note that stepping a single line will resume and then pause again
    ZilchDeclareEvent(DebuggerResume, DebuggerEvent);
  }

  // When the debugger pauses or resumes, we send this event out
  class ZeroShared DebuggerEvent : public EventData
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // The debugger we're currently paused inside of
    Debugger* RunningDebugger;

    // The state we're currently paused inside of (last running)
    ExecutableState* State;

    // The location of where we're at in script (generally where we're paused)
    CodeLocation* Location;
  };

  // Every platform should define an error handler
  ZeroShared bool DebugErrorHandler(ErrorSignaler::ErrorData& errorData);

  // Any data we receive from the debugger
  class ZeroShared DebuggerMessage
  {
  public:
    // The root of the json tree
    JsonValue* JsonRoot;

    // The message that we're currently processing
    String Type;
  };

  // The debugger message handler function
  typedef void (*MessageFn)(const DebuggerMessage& message, void* userData);

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

  // The debugger hosts a web-socket connection and allows an external program to
  // place breakpoints, step over lines, see the call stack, inspect variables, etc
  // The debugger is NOT thread safe, so only ExecutableStates from the same thread
  // should be added to the debugger. Note however that you can create multiple
  // debuggers hosted on different ports for different threads
  // Note: The debugger must be periodically updated
  class ZeroShared Debugger : public EventHandler
  {
  public:
    // sends DebuggerPauseUpdate : DebuggerEvent;
    // sends DebuggerPause : DebuggerEvent;
    // sends DebuggerResume : DebuggerEvent;

    // Constructor
    Debugger();

    // Destructor
    ~Debugger();

    // Starts the debugger hosting on a given port
    void Host(int port);

    // Tells us if the debugger has been initialized (basically if we started hosting or not)
    bool IsValid();

    // The debugger must be periodically updated to ensure that it receives remote messages
    void Update();

    // Adds a project whose files we track
    // We use the hashes of the code from each file to show code entries in the debugger (as well as file name)
    void AddProject(Project* project);

    // Remove a project that we no longer want to track
    void RemoveProject(Project* project);

    // Adds a state to be debugged (this will inform any running debuggers of the new state)
    // Be sure to remove any states that get deleted (they will not be automatically removed!)
    // A state can only be added once (multiple times will be ignored)
    void AddState(ExecutableState* state);

    // Removes a state from the list of states to be debugged
    // It is safe to call this more than once (and can be called even when the state was not added)
    void RemoveState(ExecutableState* state);

    // When we receive a custom json message, this will attempt to handle it
    void AddMessageHandler(StringParam type, MessageFn callback, void* userData);

  private:

    // Send a single message
    void SendPacket(StringParam message);

    // Send a single json built message (helper function)
    void SendPacket(const JsonBuilder& message);

    // When we resume execution, we want to tell the remote client to clear the execution point
    void SetExecutionPoint(CodeLocation* codeLocation, ExecutableState* state);

    // When we resume execution, we want to tell the remote client to clear the execution point
    void ClearExecutionPoint();

    // Updates the view of executable states and their files
    void UpdateExplorerView();

    // The break loop will pause all execution on this thread, only processing debugger messages
    void PauseExecution(CodeLocation* codeLocation, ExecutableState* state);

    // Checks if a type has any debuggable properties (expandable)
    static bool HasDebuggableProperties(Type* type);

    // Called when the remote debugger connects to us
    void OnAcceptedConnection(WebSocketEvent* event);

    // Called by the same thread the debugger / states are on
    // This will process all incoming messages, such as breakpoint, step, continue, etc
    void OnReceivedData(WebSocketEvent* event);

    // Called when we encounter any errors with a connection or the listener (server)
    void OnError(WebSocketEvent* event);

    // Called when the remote debugger disconnects
    void OnDisconnected(WebSocketEvent* event);

    // Attempts to find a code entry by hash (first starting with the project, then with each executable state)
    CodeEntry* FindCodeEntry(size_t hash);

    // Messages from the client:
    // When we receive a remote message to add a breakpoint for a code file and line
    static void OnChangeBreakpoint(const DebuggerMessage& message, void* userData);

    // When we receive a remote message to remove a breakpoint for a code file and line
    static void OnRemoveBreakpoint(const DebuggerMessage& message, void* userData);

    // When the client sends us a common command like pause, resume, step over, etc
    static void OnPause(const DebuggerMessage& message, void* userData);
    static void OnResume(const DebuggerMessage& message, void* userData);
    static void OnStepOver(const DebuggerMessage& message, void* userData);
    static void OnStepIn(const DebuggerMessage& message, void* userData);
    static void OnStepOut(const DebuggerMessage& message, void* userData);
    
    // When the user attempted to view an item in the explorer, we're repsonsible for sending what to show
    static void OnViewExplorerItem(const DebuggerMessage& message, void* userData);
    
    // When the debugger attempts to query an expression (such as when hovering over a variable or watching an expression)
    static void OnQueryExpression(const DebuggerMessage& message, void* userData);

    // Callbacks from the state:
    // Every time the executable state steps into an opcode, this function is called
    void OnOpcodePreStep(OpcodeEvent* e);

    // Every time the executable state steps into a function, this function is called
    void OnEnterFunction(OpcodeEvent* e);

    // Every time the executable state steps out of a function, this function is called
    void OnExitFunction(OpcodeEvent* e);

    // Every time the executable state steps out of a function, this function is called
    void OnException(ExceptionEvent* e);

    // Whenever we print anything out using the console, we want to know about it
    void OnConsoleWrite(ConsoleEvent* event);

  private:

    // This data must be cleared properly upon the client disconnecting
    //******** BEGIN CLEARED DATA ********//
    
    // The last action that was queued up by the debugger for the current state
    DebuggerAction::Enum Action;

    // As we walk over lines of code (callbacks from any running ExecutableState)
    // we will check to see if the line exists in this breakpoints map
    // The map maps from code hash values to line numbers
    HashMap<size_t, HashSet<size_t> > Breakpoints;

    // Store the last location id and line
    CodeLocation LastLocation;

    // The last call stack position (how deep we were)
    size_t LastCallStackDepth;

    // The last state that we were accessing
    ExecutableState* LastState;

    // When we're doing stepping, we need to save the last location here (but not update it with each opcode step)
    CodeLocation StepLocation;

    // The call stack depth where we're stepping out of (state context relative operations)
    size_t StepOutOverCallStackDepth;

    // The state we were using when stepping out / over (state context relative operations)
    ExecutableState* StepOutOverState;

    //******** END CLEARED DATA ********//

    // The states we are currently debugging
    Array<ExecutableState*> States;

    // The projects whose code we are currently viewing
    Array<Project*> Projects;

    // We need to check if any projects change files, then update the remote end if that happens
    // Currently we just scan the projects each update for any changes (we detect changes by looking for hash code changes)
    unsigned long long AllProjectsHashCode;

    // When we recieve messages from the remote client, we look here to handle any messages
    class DebuggerMessageDelegate
    {
    public:
      MessageFn MessageCallback;
      void* UserData;
    };
    HashMap<String, DebuggerMessageDelegate> MessageHandlers;

    // Accepts connections and manages our remote connection
    ThreadedWebSocketServer Server;
  };

  // All information the debugger needs to know per state
  class ZeroShared DebuggerState
  {
  public:
  };
}

#endif
