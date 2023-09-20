// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{
namespace Events
{
RaverieDefineEvent(DebuggingPauseUpdate);
RaverieDefineEvent(DebuggingPause);
RaverieDefineEvent(DebuggingResume);
RaverieDefineEvent(DebuggingBreakpointedAdded);
RaverieDefineEvent(DebuggingBreakpointedRemoved);
RaverieDefineEvent(DebuggingBreakNotAllowed);
} // namespace Events

bool Debugger::Enabled = true;

RaverieDefineType(DebuggerEvent, builder, type)
{
}

RaverieDefineType(DebuggerTextEvent, builder, type)
{
}

QueryResult::QueryResult() : Expandable(false)
{
}

Debugger::Debugger() : Action(DebuggerAction::Resume), LastCallStackDepth(0), StepOutOverCallStackDepth(0), IsAttached(false), IsBreakpointed(false), WasLastDisallowedBreak(false)
{
  // We want to know when the console writes anything
  EventConnect(&ScriptConsole::Events, Events::ConsoleWrite, &Debugger::OnConsoleWrite, this);

  // Attach to the state
  ExecutableState* state = ExecutableState::CallingState;
  EventConnect(state, Events::OpcodePreStep, &Debugger::OnOpcodePreStep, this);
  EventConnect(state, Events::EnterFunction, &Debugger::OnEnterFunction, this);
  EventConnect(state, Events::ExitFunction, &Debugger::OnExitFunction, this);
  EventConnect(state, Events::ExceptionUnhandled, &Debugger::OnException, this);
}

Debugger::~Debugger()
{
}

void Debugger::Pause()
{
  this->Action = DebuggerAction::Pause;
  this->UpdateAttach();
}

void Debugger::Resume()
{
  // Make sure we clear out all the data so the program can continue
  this->Action = DebuggerAction::Resume;
  this->UpdateAttach();
}

void Debugger::StepOver()
{
  this->Action = DebuggerAction::StepOver;
  this->StepLocation = this->LastLocation;
  this->StepOutOverCallStackDepth = this->LastCallStackDepth;
  this->UpdateAttach();
}

void Debugger::StepIn()
{
  this->Action = DebuggerAction::StepIn;
  this->StepLocation = this->LastLocation;
  this->UpdateAttach();
}

void Debugger::StepOut()
{
  this->Action = DebuggerAction::StepOut;
  this->StepLocation = this->LastLocation;
  this->StepOutOverCallStackDepth = this->LastCallStackDepth;
  this->UpdateAttach();
}

CodeEntry* Debugger::FindCodeEntry(StringParam origin)
{
  // Loop through all the projects
  for (size_t i = 0; i < this->Libraries.Size(); ++i)
  {
    LibraryRef& library = this->Libraries[i];

    // Loop through all code entries in the library
    for (size_t j = 0; j < library->Entries.Size(); ++j)
    {
      // Grab the current code entry from the project
      CodeEntry* entry = &library->Entries[j];
      if (entry->Origin == origin)
        return entry;
    }
  }

  // We found nothing!
  return nullptr;
}

Any Debugger::QueryExpression(StringParam expression, Array<QueryResult>& results)
{
  // Don't query an expression if we're not in a breakpoint
  if (!this->IsBreakpointed)
    return Any();

  // Save away all the state callbacks (we don't want them to get called while
  // we make calls) Because we are swapping with an empty event handler, this
  // also clears out all state callbacks
  ExecutableState* state = ExecutableState::CallingState;
  EventHandler savedEvents;
  EventSwapAll(&savedEvents, state);

  // Temporary space used for traversing object paths and copying Raverie objects
  // (Delegate is the largest object)
  Any currentValue;

  Raverie::StringTokenRange splitter(expression, '.');
  if (splitter.Empty() == false)
  {
    // Grab just the variable name
    String variableName = splitter.Front().Trim();
    splitter.PopFront();

    // Loop through the frames from top to bottom (backwards)
    for (int i = (int)state->StackFrames.Size() - 1; i >= 0; --i)
    {
      // Grab the current frame
      PerFrameData* frame = state->StackFrames[i];

      // Skip any non-active frames (frames in the process of being called)
      if (frame->ProgramCounter == ProgramCounterNotActive)
        continue;

      // Get the current function
      Function* function = frame->CurrentFunction;

      // Loop through all variables in the current function
      for (size_t j = 0; j < function->Variables.Size(); ++j)
      {
        // Grab the current variable
        Variable* variable = function->Variables[j];

        // If the variable's name matches our expression...
        if (variable->Name == variableName)
        {
          // Get the memory that points directly at the variable on the stack
          String valueName = variableName;

          // If the frame is currently initialized
          if (frame->IsVariableInitialized(variable) == false)
            continue;

          // Get the stack location of the variable
          byte* variableStackMemory = frame->Frame + variable->Local;
          currentValue = Any(variableStackMemory, variable->ResultType);

          // If this is the first value, then write out its value (otherwise the
          // parent would have written out our value)
          if (splitter.Empty())
          {
            // Stringify the variable (gets its value)
            String value = currentValue.ToString();
            QueryResult& result = results.PushBack();
            result.Name = valueName;
            result.Value = value;
            result.Expandable = false;
          }

          // Until we run out of sub-strings...
          while (splitter.Empty() == false)
          {
            // Get the current property name
            String propertyName = splitter.Front();
            splitter.PopFront();

            // Grab the property or field by name
            Property* property = nullptr;
            BoundType* boundType = Type::GetBoundType(currentValue.StoredType);
            if (boundType != nullptr)
            {
              property = boundType->GetInstanceGetterSetter(propertyName);
              if (property == nullptr)
                property = boundType->GetInstanceField(propertyName);
            }

            // We allowe debugging of hidden properties, we just don't enumerate
            // them
            if (property != nullptr && property->Get != nullptr)
            {
              valueName = propertyName;
              currentValue = property->Get->Invoke(currentValue);

              // If we didn't get a valid value back (an exception may have
              // happened, then early out)
              if (!currentValue.IsHoldingValue())
                goto END;
            }
            else
            {
              goto END;
            }
          }

          // We want to avoid showing duplicate properties (when they are the
          // exact same value) We do want to however support showing hidden
          // properties
          HashMap<String, String> evaluatedProperties;

          // Check to see if the type is a bound type
          BoundType* boundType = Type::DynamicCast<BoundType*>(currentValue.StoredType);

          // If the type is a bound type...
          while (boundType != nullptr)
          {
            // Loop through all the instance properties
            PropertyArray& properties = boundType->AllProperties;
            for (size_t i = 0; i < properties.Size(); ++i)
            {
              // Grab the current property
              Property* property = properties[i];

              // If it's a static or hidden property, skip it
              if (property->IsStatic || property->IsHidden || property->Get == nullptr)
                continue;

              Any propertyResult = property->Get->Invoke(currentValue);

              // If the property should be hidden when null...
              if (property->IsHiddenWhenNull && propertyResult.IsNull())
                continue;

              // Stringify the variable (gets its value)
              String propertyValue = propertyResult.ToString();

              // If we haven't seen this EXACT property value before...
              String& evaluatedValue = evaluatedProperties[property->Name];
              if (evaluatedValue != propertyValue)
              {
                // Update the value stored in the evaluated property map, so
                // that next time we'll know if we've seen this
                evaluatedValue = propertyValue;

                QueryResult& result = results.PushBack();
                result.Name = property->Name;
                result.Value = propertyValue;
                result.Expandable = HasDebuggableProperties(propertyResult.StoredType);
              }
            }

            // Iterate up to the base class (because we want to access base
            // class properties too)
            boundType = boundType->BaseType;
          }

          // We discovered a valid variable, time to break out of all loops
          goto END;
        }
      }
    }
  }
END:

  // Restore all the state callbacks
  EventSwapAll(&savedEvents, state);

  return currentValue;
}

bool Debugger::HasDebuggableProperties(Type* type)
{
  // Grab the type as a bound type
  BoundType* boundType = Type::DynamicCast<BoundType*>(type);

  // If this is not a bound type, then it has no properties...
  if (boundType == nullptr)
    return false;

  // We have to walk through all properties and check for any non hidden
  // properties
  for (size_t i = 0; i < boundType->AllProperties.Size(); ++i)
  {
    // Grab the current property
    Property* property = boundType->AllProperties[i];

    // As long as this property isn't hidden or marked static, then we can debug
    // it!
    if (property->IsHidden == false && property->IsStatic == false)
      return true;
  }

  // We didn't find a single expandable property...
  return false;
}

bool Debugger::SetBreakpoint(StringParam origin, size_t line, bool breakpoint)
{
  // Grab breakpointed lines by the state and code entry id
  HashSet<size_t>& breakpointedLines = this->Breakpoints[origin];

  DebuggerEvent toSend;
  toSend.RunningDebugger = this;
  toSend.State = ExecutableState::CallingState;
  CodeLocation location;
  location.Origin = origin;
  location.StartLine = line;
  toSend.Location = &location;

  // We need to know whether we're adding or removing a breakpoint
  if (breakpoint)
  {
    // Attempt to add the line and let everyone know if the breakpoint was
    // actually added
    if (breakpointedLines.Insert(line))
      EventSend(this, Events::DebuggingBreakpointedAdded, &toSend);
  }
  else
  {
    // Attempt to remove the line and let everyone know if the breakpoint was
    // actually removed
    if (breakpointedLines.Erase(line))
    {
      EventSend(this, Events::DebuggingBreakpointedRemoved, &toSend);

      if (breakpointedLines.Empty())
        this->Breakpoints.Erase(origin);
    }
  }

  this->UpdateAttach();

  // For the moment, we always bind breakpoints (this may change in the future)
  return breakpoint;
}

bool Debugger::SetBreakpoint(const CodeLocation& location, bool breakpoint)
{
  return this->SetBreakpoint(location.Origin, location.StartLine, breakpoint);
}

bool Debugger::HasBreakpoint(StringParam origin, size_t line)
{
  if (!this->Breakpoints.ContainsKey(origin))
    return false;

  return this->Breakpoints[origin].Contains(line);
}

bool Debugger::HasBreakpoint(const CodeLocation& location)
{
  return this->HasBreakpoint(location.Origin, location.StartLine);
}

void Debugger::ClearBreakpoints(StringParam origin)
{
  this->Breakpoints.Erase(origin);
}

void Debugger::ClearAllBreakpoints()
{
  this->Breakpoints.Clear();
}

void Debugger::OnConsoleWrite(ConsoleEvent* event)
{
}

void Debugger::OnEnterFunction(OpcodeEvent* e)
{
  ErrorIf(this->IsBreakpointed, "We should not be able to get in here when in a breakpoint");
}

void Debugger::OnExitFunction(OpcodeEvent* e)
{
  ErrorIf(this->IsBreakpointed, "We should not be able to get in here when in a breakpoint");
}

void Debugger::OnException(ExceptionEvent* e)
{
  // Ignore exceptions if we're breakpointed
  if (this->IsBreakpointed)
    return;

  this->Pause();
  CodeLocation location = e->ThrownException->Trace.GetMostRecentNonNativeLocation();
  this->Breakpoint(location);
}

void Debugger::OnOpcodePreStep(OpcodeEvent* e)
{
  ErrorIf(this->IsBreakpointed, "We should not be able to get in here when in a breakpoint");

  // Make sure we pump incoming messages
  this->Update();

  // If we didn't get a code location, just skip this (something invalid must
  // have happened)
  if (e->Location == nullptr)
    return;

  // Cache some variables as locals
  ExecutableState* state = e->State;
  CodeLocation& location = *e->Location;

  // If we have a timeout, just basically disable it... we're in the debugger!
  if (state->Timeouts.Empty() == false)
  {
    // Just set the timeout to the max time
    state->Timeouts.Back().LengthTicks = 0x7FFFFFFFFFFFE;
  }

  // Figure out if we changed to a new line by entering this opcode
  bool isNewLineOrFileFromLastLocation = location.StartLine != this->LastLocation.StartLine || location.Origin != this->LastLocation.Origin;
  bool isNewLineOrFileFromStepLocation = location.StartLine != this->StepLocation.StartLine || location.Origin != this->StepLocation.Origin;

  // Store the last code location so we can step by single lines
  this->LastLocation = location;
  this->LastCallStackDepth = state->StackFrames.Size();

  // Lets us know whether the action was handled (so we don't need to check for
  // breakpoints)
  bool actionPausedExecution = false;

  // Based on the action that was last set by the remote client
  switch (this->Action)
  {
  // The user wanted to pause, just pause immediately on the current opcode
  case DebuggerAction::Pause:
  {
    this->Breakpoint(location);
    actionPausedExecution = true;
  }
  break;

  // The user wanted to step into the next function it sees
  // Basically we just break on every new line of opcode
  case DebuggerAction::StepIn:
  {
    // If we changed lines or code entry ids (files), then we want to pause
    if (isNewLineOrFileFromLastLocation)
    {
      this->Breakpoint(location);
      actionPausedExecution = true;
    }
  }
  break;

  // If we're stepping out of a function...
  case DebuggerAction::StepOut:
  {
    // If we're in the same state that we wanted to step out, and the call stack
    // depth is less than what we started at
    if (isNewLineOrFileFromStepLocation && state->StackFrames.Size() < this->StepOutOverCallStackDepth)
    {
      // Not necessary, but lets just clear the state and depth to make things
      // clearer
      this->StepOutOverCallStackDepth = 0;

      // We stepped out of a function!
      this->Breakpoint(location);
      actionPausedExecution = true;
    }
    break;
  }

  // The user wanted to pause on the next line (or next file, etc)...
  case DebuggerAction::StepOver:
  {
    // If we're in the same state that we wanted to step over, and the call
    // stack depth is the same as what we started at
    if (isNewLineOrFileFromStepLocation && state->StackFrames.Size() <= this->StepOutOverCallStackDepth)
    {
      // Not necessary, but lets just clear the state and depth to make things
      // clearer
      this->StepOutOverCallStackDepth = 0;

      // We stepped out of a function!
      this->Breakpoint(location);
      actionPausedExecution = true;
    }
  }
  break;
  // Debugger is attached and looking for a breakpoint location
  case DebuggerAction::Resume:
    break;
  }

  // Note: We always test for breakpoints (when resumed, when stepping in, when
  // stepping out, etc) Even when testing for breakpoints, there may be many
  // opcode associated with one line We only want to break upon the first opcode
  // for that line
  if (isNewLineOrFileFromLastLocation && actionPausedExecution == false)
  {
    // Grab the breakpoint line list by code location/id and state
    HashMap<String, HashSet<size_t>>::range result = this->Breakpoints.Find(location.Origin);
    if (!result.Empty())
    {
      HashSet<size_t>& breakpointedLines = result.Front().second;

      // If the breakpoints has the current line we're hitting
      if (breakpointedLines.Contains(location.StartLine))
        this->Breakpoint(location);
    }
  }
}

void Debugger::AttachCallbacks()
{
  if (this->IsAttached)
    return;

  this->IsAttached = true;

  ExecutableState* state = ExecutableState::CallingState;
  ErrorIf(state->EnableDebugEvents,
          "Debug events were enabled for the ExecutableState, are there two "
          "debuggers running?");

  // Enable debug events
  state->EnableDebugEvents = true;
}

void Debugger::DetachCallbacks()
{
  if (!this->IsAttached)
    return;

  this->IsAttached = false;

  ExecutableState* state = ExecutableState::CallingState;
  ErrorIf(!state->EnableDebugEvents,
          "Debug events were not enabled for the ExecutableState, are there "
          "two debuggers running?");

  // Disable debug events
  state->EnableDebugEvents = false;
}

void Debugger::UpdateAttach()
{
  if (this->Breakpoints.Empty() && this->Action == DebuggerAction::Resume || this->IsBreakpointed)
    this->DetachCallbacks();
  else
    this->AttachCallbacks();
}

void Debugger::Breakpoint(const CodeLocation& codeLocation)
{
  if (!codeLocation.IsValid())
    return;

  if (!Enabled)
  {
    this->Resume();
    return;
  }

  ErrorIf(this->IsBreakpointed, "We should not be able to get in here when in a breakpoint");

  ExecutableState* state = ExecutableState::CallingState;

  // If we're currently set to not allow breakpoints...
  if (!this->DoNotAllowBreakReason.Empty())
  {
    // We only want to do this once...
    if (!this->WasLastDisallowedBreak)
    {
      this->WasLastDisallowedBreak = true;

      DebuggerTextEvent toSend;
      toSend.RunningDebugger = this;
      toSend.State = state;
      toSend.Location = &codeLocation;
      toSend.Text = this->DoNotAllowBreakReason;
      EventSend(this, Events::DebuggingBreakNotAllowed, &toSend);
    }
    return;
  }

  // Since we got in here, breakpoints must be allowed
  this->WasLastDisallowedBreak = false;

  // We hit a breakpoint, so pause execution
  this->Action = DebuggerAction::Pause;
  this->IsBreakpointed = true;
  this->UpdateAttach();

  // Just create a single empty event that we send just once
  DebuggerEvent toSend;
  toSend.RunningDebugger = this;
  toSend.State = state;
  toSend.Location = &codeLocation;

  // Let the user know that we just paused...
  EventSend(this, Events::DebuggingPause, &toSend);
  this->DebuggingPause(codeLocation);

  // Loop until we hit an again that causes us to resume
  while (this->Action == DebuggerAction::Pause)
  {
    // Send out an event that lets the hosting application update its visuals
    // (it's being debugged!)
    EventSend(this, Events::DebuggingPauseUpdate, &toSend);
    this->DebuggingPauseUpdate(codeLocation);

    // While we're paused just pump messages (could be a breakpoint, after a
    // step, or a true pause)
    this->Update();
  }

  // Let the user know that we just resumed...
  EventSend(this, Events::DebuggingResume, &toSend);
  this->DebuggingResume();

  this->IsBreakpointed = false;
  this->UpdateAttach();
}

void Debugger::DebuggingPause(const CodeLocation& codeLocation)
{
}

void Debugger::DebuggingPauseUpdate(const CodeLocation& codeLocation)
{
}

void Debugger::DebuggingResume()
{
}

void Debugger::Update()
{
}
} // namespace Raverie
