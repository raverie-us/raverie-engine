/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

// Redirection header based on the platform
#ifdef _MSC_VER
  #include "DebuggingWindows.inl"
#else
  #include "DebuggingGeneric.inl"
#endif

namespace Zilch
{
  //***************************************************************************
  namespace Events
  {
    ZilchDefineEvent(DebuggerPauseUpdate);
    ZilchDefineEvent(DebuggerPause);
    ZilchDefineEvent(DebuggerResume);
  }

  //***************************************************************************
  ZilchDefineType(DebuggerEvent, builder, type)
  {
  }

  //***************************************************************************
  Debugger::Debugger() :
    Action(DebuggerAction::Resume),
    LastState(nullptr),
    LastCallStackDepth(0),
    StepOutOverCallStackDepth(0),
    StepOutOverState(nullptr),
    Server(1),
    AllProjectsHashCode(0)
  {
    // We want to know when the console writes anything
    EventConnect(&Console::Events, Events::ConsoleWrite, &Debugger::OnConsoleWrite, this);
    
    // Connect all our event handlers up to the server
    EventConnect(&this->Server, Events::WebSocketAcceptedConnection, &Debugger::OnAcceptedConnection, this);
    EventConnect(&this->Server, Events::WebSocketDisconnected, &Debugger::OnDisconnected, this);
    EventConnect(&this->Server, Events::WebSocketError, &Debugger::OnError, this);
    EventConnect(&this->Server, Events::WebSocketReceivedData, &Debugger::OnReceivedData, this);

    // Register our message handlers
    this->AddMessageHandler("ChangeBreakpoint", OnChangeBreakpoint, this);
    this->AddMessageHandler("Resume", OnResume, this);
    this->AddMessageHandler("Pause", OnPause, this);
    this->AddMessageHandler("StepOver", OnStepOver, this);
    this->AddMessageHandler("StepIn", OnStepIn, this);
    this->AddMessageHandler("StepOut", OnStepOut, this);
    this->AddMessageHandler("QueryExpression", OnQueryExpression, this);
    this->AddMessageHandler("ViewExplorerItem", OnViewExplorerItem, this);
  }

  //***************************************************************************
  Debugger::~Debugger()
  {
  }

  //***************************************************************************
  void Debugger::Host(int port)
  {
    // If the server hasn't been initialized yet, then host the server on a given port
    if (this->Server.IsValid() == false)
      this->Server.Host(port);
  }
  
  //***************************************************************************
  bool Debugger::IsValid()
  {
    return this->Server.IsValid();
  }
  
  //***************************************************************************
  void Debugger::Update()
  {
    // Early out if we're not hosting
    if (this->Server.IsValid() == false)
      return;

    // Make sure we pump incoming messages
    this->Server.Update();

    // Loop through all the projects using a total hash to see if any project has changed (including files)
    unsigned long long projectsHash = 0;
    for (size_t i = 0; i < this->Projects.Size(); ++i)
    {
      // Grab the current project
      Project* project = this->Projects[i];

      // Loop through all the code entries in this project
      for (size_t j = 0; j < project->Entries.Size(); ++j)
      {
        // Grab the current code entry
        CodeEntry& entry = project->Entries[j];
        projectsHash ^= (unsigned long long)entry.GetHash();
        projectsHash *= 5209;
      }
    }

    // If the hash wasn't the same as the last time...
    if (projectsHash != this->AllProjectsHashCode)
    {
      // Update the remote explorer view
      this->UpdateExplorerView();
      this->AllProjectsHashCode = projectsHash;
    }
  }

  //***************************************************************************
  void Debugger::OnPause(const DebuggerMessage& message, void* userData)
  {
    Debugger* self = (Debugger*)userData;
    self->Action = DebuggerAction::Pause;
  }
  
  //***************************************************************************
  void Debugger::OnResume(const DebuggerMessage& message, void* userData)
  {
    Debugger* self = (Debugger*)userData;
    self->Action = DebuggerAction::Resume;
  }
  
  //***************************************************************************
  void Debugger::OnStepOver(const DebuggerMessage& message, void* userData)
  {
    Debugger* self = (Debugger*)userData;
    self->Action = DebuggerAction::StepOver;
    self->StepLocation = self->LastLocation;
    self->StepOutOverCallStackDepth = self->LastCallStackDepth;
    self->StepOutOverState = self->LastState;
  }
  
  //***************************************************************************
  void Debugger::OnStepIn(const DebuggerMessage& message, void* userData)
  {
    Debugger* self = (Debugger*)userData;
    self->Action = DebuggerAction::StepIn;
    self->StepLocation = self->LastLocation;
  }
  
  //***************************************************************************
  void Debugger::OnStepOut(const DebuggerMessage& message, void* userData)
  {
    Debugger* self = (Debugger*)userData;
    self->Action = DebuggerAction::StepOut;
    self->StepLocation = self->LastLocation;
    self->StepOutOverCallStackDepth = self->LastCallStackDepth;
    self->StepOutOverState = self->LastState;
  }
  
  //***************************************************************************
  CodeEntry* Debugger::FindCodeEntry(size_t hash)
  {
    // Loop through all the projects
    for (size_t i = 0; i < this->Projects.Size(); ++i)
    {
      // Grab the current project
      Project* project = this->Projects[i];

      // Loop through all code entries in the project
      for (size_t j = 0; j < project->Entries.Size(); ++j)
      {
        // Grab the current code entry from the project
        CodeEntry* entry = &project->Entries[j];
        if (entry->GetHash() == hash)
          return entry;
      }
    }

    // Loop through all the states
    for (size_t i = 0; i < this->States.Size(); ++i)
    {
      // Grab the current executable state
      ExecutableState* state = this->States[i];

      // Look in the current state for the code entry...
      CodeEntry* foundEntry = state->CodeHashToCodeEntry.FindValue(hash, nullptr);
      if (foundEntry != nullptr)
        return foundEntry;
    }

    // We found nothing!
    return nullptr;
  }

  //***************************************************************************
  void Debugger::OnViewExplorerItem(const DebuggerMessage& message, void* userData)
  {
    Debugger* self = (Debugger*)userData;
    
    // With the breakpoint message comes a unique identifier object that describes exact paths to code entries
    // This is the same as the object sent from 'AddState' with each code entry
    JsonValue* codeData = message.JsonRoot->GetMember("CodeData");
    if (codeData == nullptr)
    {
      Error("CodeData object was not specified in the 'AddBreakpoint' message");
      return;
    }
    
    // When looking for any code file that we put a breakpoint in, we can identify the code by an id
    size_t codeHash = (size_t)codeData->MemberAsLongLong("CodeHash");
    
    // Look for the code entry in a map to all code files
    CodeEntry* entry = self->FindCodeEntry(codeHash);
    if (entry == nullptr)
    {
      Error("We couldn't find the code file for the given id");
      return;
    }
    
    JsonBuilder builder;
    builder.Begin(JsonType::Object);
    {
      builder.Key("MessageType");
      builder.Value("ShowCodeEntry");
      builder.Key("Origin");
      builder.Value(entry->Origin);
      builder.Key("Code");
      builder.Value(entry->Code);
    
      builder.Key("CodeData");
      builder.Begin(JsonType::Object);
      {
        // All data sent within here is stored directly on the debugger side
        // and is directly returned to us in messages such as 'AddBreakpoint'
        builder.Key("CodeHash");
        builder.Value(entry->GetHash());
      }
      builder.End();
    }
    builder.End();
    
    String toSend = builder.ToString();
    self->SendPacket(toSend);
  }

  //***************************************************************************
  void Debugger::OnQueryExpression(const DebuggerMessage& message, void* userData)
  {
    Debugger* self = (Debugger*)userData;
    String expression = message.JsonRoot->MemberAsString("Expression");
    
    // The remote side gives every query and id, so that way when we respond to multiple it can distingquish which is which
    int queryId = message.JsonRoot->MemberAsInteger("QueryId");

    // Assume we're querying an expression for whatever state we're currently debugging
    ExecutableState* state = self->LastState;

    // If we're not debugging a state, return early
    if (state == nullptr)
      return;
    
    // Save away all the state callbacks (we don't want them to get called while we make calls)
    // Because we are swapping with an empty event handler, this also clears out all state callbacks
    EventHandler savedEvents;
    EventSwapAll(&savedEvents, state);

    // Temporary space used for traversing object paths and copying Zilch objects (Delegate is the largest object)
    byte tempSpace[sizeof(Delegate)];

    // Send a message back to answer the expression query
    JsonBuilder builder;
    builder.Begin(JsonType::Object);
    {
      builder.Key("MessageType");
      builder.Value("QueryResult");
      builder.Key("QueryId");
      builder.Value(queryId);
      builder.Key("Expression");
      builder.Value(expression);
      builder.Key("Values");
      builder.Begin(JsonType::ArrayMultiLine);
      {
        Zero::StringTokenRange splitter(expression, '.');
        if (splitter.Empty() == false)
        {
          // Grab just the variable name
          String variableName = splitter.Front();
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
                Type* type = variable->ResultType;

                // If the frame is currently initialized
                if (frame->IsVariableInitialized(variable) == false)
                  continue;

                // Get the stack location of the variable
                byte* variableStackMemory = frame->Frame + variable->Local;
                type->GenericCopyConstruct(tempSpace, variableStackMemory);

                // If this is the first value, then write out its value (otherwise the parent would have written out our value)
                if (splitter.Empty())
                {
                  // Stringify the variable (gets its value)
                  String value = type->GenericToString(tempSpace);
                  builder.Begin(JsonType::Object);
                  {
                    builder.Key("Property");
                    builder.Value(valueName);
                    builder.Key("Value");
                    builder.Value(value);
                    builder.Key("Expandable");
                    builder.Value(false);
                  }
                  builder.End();
                }

                // Until we run out of sub-strings...
                while (splitter.Empty() == false)
                {
                  // Get the most virtual version of that memory (dereferences handles, gets the most derived type, etc)
                  byte* valueMemory = type->GenericGetMemory(tempSpace);
                  type = type->GenericGetVirtualType(tempSpace);

                  // Get the current property name
                  String propertyName = splitter.Front();
                  splitter.PopFront();
                  
                  // Grab the property or field by name
                  Property* property = nullptr;
                  BoundType* boundType = Type::GetBoundType(type);
                  if (boundType != nullptr)
                  {
                    property = boundType->GetInstanceGetterSetter(propertyName);
                    if (property == nullptr)
                      property = boundType->GetInstanceField(propertyName);
                  }

                  // We allowe debugging of hidden properties, we just don't enumerate them
                  if (property != nullptr && property->Get != nullptr)
                  {
                    type->GenericDestruct(tempSpace);

                    valueName = propertyName;
                    type = property->PropertyType;

                    Call call(property->Get, state);

                    // Set the this handle (just as a global pointer...)
                    call.DisableThisChecks();
                    PointerManager* manager = state->GetHandleManager<PointerManager>();
                    BoundType* thisBoundType = Type::GetBoundType(type);
                    Handle* thisHandle = new (call.GetThisUnchecked()) Handle(valueMemory, thisBoundType, manager);

                    ExceptionReport report;
                    call.Invoke(report);

                    if (report.HasThrownExceptions())
                      goto END;

                    byte* returnValue = call.GetReturnUnchecked();
                    type->GenericCopyConstruct(tempSpace, returnValue);
                  }
                  else
                  {
                    // WE FOUND NOTHING! NOOOOOOTHING!!!
                    goto END;
                  }
                }

                // We want to avoid showing duplicate properties (when they are the exact same value)
                // We do want to however support showing hidden properties
                HashMap<String, String> evaluatedProperties;

                // Check to see if the type is a bound type
                BoundType* boundType = Type::DynamicCast<BoundType*>(type);

                // If the type is a bound type...
                while (boundType != nullptr)
                {
                  // Either dereference the handle or get the memory for the value
                  byte* memory = boundType->GenericGetMemory(tempSpace);

                  // Loop through all the instance properties
                  PropertyArray& properties = boundType->AllProperties;
                  for (size_t i = 0; i < properties.Size(); ++i)
                  {
                    // Grab the current property
                    Property* property = properties[i];

                    // If it's a static or hidden property, skip it
                    if (property->IsStatic || property->IsHidden || property->Get == nullptr)
                      continue;

                    Call call(property->Get, state);

                    // Set the this handle (just as a global pointer...)
                    call.DisableThisChecks();
                    PointerManager* manager = state->GetHandleManager<PointerManager>();
                    new (call.GetThisUnchecked()) Handle(memory, boundType, manager);

                    ExceptionReport report;
                    call.Invoke(report);

                    // The value we evaluated for this property (its return value stringified)
                    String propertyValue;

                    if (report.HasThrownExceptions())
                    {
                      // Just treat the property value as the concatenation of all the exceptions thrown
                      propertyValue = report.GetConcatenatedMessages();
                    }
                    else
                    {
                      byte* returnValue = call.GetReturnUnchecked();

                      // If the property should be hidden when null...
                      if (property->IsHiddenWhenNull)
                      {
                        // If the dereferenced property is null, then skip it
                        byte* returnValueDereferenced = property->PropertyType->GenericGetMemory(returnValue);
                        if (returnValueDereferenced == nullptr)
                          continue;
                      }

                      // Stringify the variable (gets its value)
                      propertyValue = property->PropertyType->GenericToString(returnValue);
                    }

                    // If we haven't seen this EXACT property value before...
                    String& evaluatedValue = evaluatedProperties[property->Name];
                    if (evaluatedValue != propertyValue)
                    {
                      // Let the debugger know about this property / value
                      builder.Begin(JsonType::Object);
                      {
                        builder.Key("Property");
                        builder.Value(property->Name);
                        builder.Key("Value");
                        builder.Value(propertyValue);
                        builder.Key("Expandable");
                        builder.Value(HasDebuggableProperties(property->PropertyType));
                      }
                      builder.End();
                    }

                    // Update the value stored in the evaluated property map, so that next time we'll know if we've seen this
                    evaluatedProperties[property->Name] = propertyValue;

                    // TEMPORARY - Because we do not have the ExecutableState in bound C++ functions, we can only
                    // print out fields (because we know their type and know where they exist in memory
                    //if (Field* field = Type::DynamicCast<Field*>(property))
                    //{
                    //  byte* fieldData = memory + field->Offset;
                    //  
                    //  // Stringify the variable (gets its value)
                    //  String fieldValue = field->PropertyType->GenericToString(fieldData);
                    //
                    //  // Let the debugger know about this property / value
                    //  builder.Begin(JsonType::Object);
                    //  {
                    //    builder.Key("Property");
                    //    builder.Value(field->Name);
                    //    builder.Key("Value");
                    //    builder.Value(fieldValue);
                    //    builder.Key("Expandable");
                    //    builder.Value(HasDebuggableProperties(field->PropertyType));
                    //  }
                    //  builder.End();
                    //}
                  }

                  // Iterate up to the base class (because we want to access base class properties too)
                  boundType = boundType->BaseType;
                }
                
                type->GenericDestruct(tempSpace);

                // We discovered a valid variable, time to break out of all loops
                goto END;
              }
            }
          }
        }
      }
END:
      builder.End();
    }
    builder.End();
    
    String toSend = builder.ToString();
    self->SendPacket(toSend);

    // Restore all the state callbacks
    EventSwapAll(&savedEvents, state);
  }

  //***************************************************************************
  bool Debugger::HasDebuggableProperties(Type* type)
  {
    // Grab the type as a bound type
    BoundType* boundType = Type::DynamicCast<BoundType*>(type);

    // If this is not a bound type, then it has no properties...
    if (boundType == nullptr)
      return false;
    
    // We have to walk through all properties and check for any non hidden properties
    for (size_t i = 0; i < boundType->AllProperties.Size(); ++i)
    {
      // Grab the current property
      Property* property = boundType->AllProperties[i];

      // As long as this property isn't hidden or marked static, then we can debug it!
      if (property->IsHidden == false && property->IsStatic == false)
        return true;
    }

    // We didn't find a single expandable property...
    return false;
  }

  //***************************************************************************
  void Debugger::OnChangeBreakpoint(const DebuggerMessage& message, void* userData)
  {
    Debugger* self = (Debugger*)userData;

    // With the breakpoint message comes a unique identifier object that describes exact paths to code entries
    // This is the same as the object sent from 'AddState' with each code entry
    JsonValue* codeData = message.JsonRoot->GetMember("CodeData");
    if (codeData == nullptr)
    {
      Error("CodeData object was not specified in the 'AddBreakpoint' message");
      return;
    }

    // When looking for any code file that we put a breakpoint in, we can identify the code by an id
    size_t codeHash = (size_t)codeData->MemberAsLongLong("CodeHash");
    
    // Look for the code entry in a map to all code files
    CodeEntry* entry = self->FindCodeEntry(codeHash);
    if (entry == nullptr)
    {
      Error("We couldn't find the code file for the given id");
      return;
    }
    
    // Get the line that this is associated with
    size_t line = (size_t)message.JsonRoot->MemberAsLongLong("Line");

    // Grab breakpointed lines by the state and code entry id
    HashSet<size_t>& breakpointedLines = self->Breakpoints[entry->GetHash()];

    // We need to know whether we're adding or removing a breakpoint
    if (message.JsonRoot->MemberAsString("Action") == "Add")
      breakpointedLines.Insert(line);
    else
      breakpointedLines.Erase(line);
  }

  //***************************************************************************
  void Debugger::SendPacket(const JsonBuilder& message)
  {
    this->SendPacket(message.ToString());
  }

  //***************************************************************************
  void Debugger::SendPacket(StringParam message)
  {
    // Send the packet to 'all' (the only client we have connected, or drop the packet if nobody is connected)
    this->Server.SendPacketToAll(message, WebSocketPacketType::Text);
  }

  //***************************************************************************
  void Debugger::OnConsoleWrite(ConsoleEvent* event)
  {
    // The code location this is called from (may not be set if we're being called directly from C++, see below)
    CodeLocation location;

    // We want to know where this console write came from
    // Obviously we need to skip the actual call to Write or WriteLine
    // Warning: If the write was called from C++, it may not have a valid executable state
    // Moreover, if the call was made from C++ which was being called from Zilch, we'll only show the Zilch location
    if (event->State != nullptr)
    {
      // Grab the entire stack trace, possibly not the most efficient but it works
      StackTrace trace;
      event->State->GetStackTrace(trace);
      StackEntry* zilchStackEntry = trace.GetMostRecentNonNativeStackEntry();

      // If there was indeed a some Zilch function calling this (or called C++ that called this)
      if (zilchStackEntry != nullptr)
        location = zilchStackEntry->Location;
    }

    // Send a message back that we hit a breakpoint
    JsonBuilder builder;
    builder.Begin(JsonType::Object);
    {
      builder.Key("MessageType");
      builder.Value("Output");
      builder.Key("Text");
      builder.Value(event->Text);

      // If the location was set to something (otherwise we don't know where the call came from)
      if (location.IsValid())
      {
        builder.Key("Origin");
        builder.Value(location.Origin);
        builder.Key("Line");
        builder.Value(location.StartLine);
        builder.Key("CodeData");
        builder.Begin(JsonType::Object);
        {
          // This data allows the debugger to uniquely identify the
          // file this came from (so they can click on it and such)
          builder.Key("CodeHash");
          builder.Value(location.GetHash());
        }
        builder.End();
      }
    }
    builder.End();
    
    String message = builder.ToString();
    this->SendPacket(message);
  }

  //***************************************************************************
  void Debugger::OnEnterFunction(OpcodeEvent* e)
  {
    // Make sure we pump incoming messages
    this->Server.Update();

    if (e->Location == nullptr)
      return;

    // What do we do here?
  }

  //***************************************************************************
  void Debugger::OnExitFunction(OpcodeEvent* e)
  {
    // Make sure we pump incoming messages
    this->Server.Update();

    if (e->Location == nullptr)
      return;
  }
  
  //***************************************************************************
  void Debugger::OnException(ExceptionEvent* e)
  {
    CodeLocation location = e->ThrownException->Trace.GetMostRecentNonNativeLocation();
    ConsoleEvent exceptionEvent;
    exceptionEvent.Text = String::Format("Exception: %s\n", e->ThrownException->Message.c_str());
    exceptionEvent.State = e->State;
    this->OnConsoleWrite(&exceptionEvent);
    this->PauseExecution(&location, e->State);
  }

  //***************************************************************************
  void Debugger::OnOpcodePreStep(OpcodeEvent* e)
  {
    // Make sure we pump incoming messages
    this->Server.Update();

    // Cache some variables as locals
    CodeLocation* location = e->Location;
    ExecutableState* state = e->State;

    // If we didn't get a code location, just skip this (something invalid must have happened)
    if (location == nullptr)
      return;

    // If we have a timeout, just basically disable it... we're in the debugger!
    if (state->Timeouts.Empty() == false)
    {
      // Just set the timeout to the max time
      state->Timeouts.Back().LengthTicks = 0x7FFFFFFFFFFFE;
    }

    // Figure out if we changed to a new line by entering this opcode
    bool isNewLineOrFileFromLastLocation = location->StartLine != this->LastLocation.StartLine || location->Code != this->LastLocation.Code || state != this->LastState;
    bool isNewLineOrFileFromStepLocation = location->StartLine != this->StepLocation.StartLine || location->Code != this->StepLocation.Code;

    // Store the last code location so we can step by single lines
    this->LastLocation = *location;
    this->LastState = state;
    this->LastCallStackDepth = state->StackFrames.Size();

    // Lets us know whether the action was handled (so we don't need to check for breakpoints)
    bool actionPausedExecution = false;
    
    // Based on the action that was last set by the remote client
    switch (this->Action)
    {
      // The user wanted to pause, just pause immediately on the current opcode
      case DebuggerAction::Pause:
      {
        this->PauseExecution(location, state);
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
          this->PauseExecution(location, state);
          actionPausedExecution = true;
        }
      }
      break;

      // If we're stepping out of a function...
      case DebuggerAction::StepOut:
      {
        // If we're in the same state that we wanted to step out, and the call stack depth is less than what we started at
        if (isNewLineOrFileFromStepLocation && state == this->StepOutOverState && state->StackFrames.Size() < this->StepOutOverCallStackDepth)
        {
          // Not necessary, but lets just clear the state and depth to make things clearer
          this->StepOutOverState = nullptr;
          this->StepOutOverCallStackDepth = 0;

          // We stepped out of a function!
          this->PauseExecution(location, state);
          actionPausedExecution = true;
        }
        break;
      }

      // The user wanted to pause on the next line (or next file, etc)...
      case DebuggerAction::StepOver:
      {
        // If we're in the same state that we wanted to step over, and the call stack depth is the same as what we started at
        if (isNewLineOrFileFromStepLocation && state == this->StepOutOverState && state->StackFrames.Size() <= this->StepOutOverCallStackDepth)
        {
          // Not necessary, but lets just clear the state and depth to make things clearer
          this->StepOutOverState = nullptr;
          this->StepOutOverCallStackDepth = 0;

          // We stepped out of a function!
          this->PauseExecution(location, state);
          actionPausedExecution = true;
        }
      }
      break;
    }

    // Note: We always test for breakpoints (when resumed, when stepping in, when stepping out, etc)
    // Even when testing for breakpoints, there may be many opcode associated with one line
    // We only want to break upon the first opcode for that line
    if (isNewLineOrFileFromLastLocation && actionPausedExecution == false)
    {
      // Grab the breakpoint line list by code location/id and state
      HashSet<size_t>& breakpointedLines = this->Breakpoints[location->GetHash()];
          
      // If the breakpoints has the current line we're hitting
      if (breakpointedLines.Contains(location->StartLine))
        this->PauseExecution(location, state);
    }
  }
  
  //***************************************************************************
  void Debugger::SetExecutionPoint(CodeLocation* codeLocation, ExecutableState* state)
  {
    // Send a message back that we hit a breakpoint
    JsonBuilder builder;
    builder.Begin(JsonType::Object);
    {
      builder.Key("MessageType");
      builder.Value("SetExecutionPoint");
      builder.Key("Line");
      builder.Value(codeLocation->StartLine);
      builder.Key("CodeData");
      builder.Begin(JsonType::Object);
      {
        // All data sent within here is stored directly on the debugger side
        // and is directly returned to us in messages such as 'AddBreakpoint'
        builder.Key("CodeHash");
        builder.Value(codeLocation->GetHash());
      }
      builder.End();

      builder.Key("CallStack");
      builder.Begin(JsonType::ArrayMultiLine);
      {
        // Perform a stack trace so we can grab the entire stack
        StackTrace trace;
        state->GetStackTrace(trace);

        // Loop through the entire stack trace and send it in a message
        // Note: We loop backwards so that the most recent entries appear at the top (typical for debuggers)
        for (int i = (int)trace.Stack.Size() - 1; i >= 0; --i)
        {
          builder.Begin(JsonType::Object);
          {
            // Grab the current stack entry and emit a message to the debugger
            StackEntry& entry = trace.Stack[i];
            builder.Key("Text");
            builder.Value(entry.ExecutingFunction->ToString());
            builder.Key("Language");
            if (codeLocation->IsNative)
            {
              builder.Value("Native");
            }
            else
            {
              builder.Value("Zilch");
              builder.Key("Line");
              builder.Value(entry.Location.StartLine);
              builder.Key("CodeData");
              builder.Begin(JsonType::Object);
              {
                // This data allows the debugger to identify exactly which file is being shown
                builder.Key("CodeHash");
                builder.Value(entry.Location.GetHash());
              }
              builder.End();
            }
          }
          builder.End();
        }
      }
      builder.End();
    }
    builder.End();
      
    String message = builder.ToString();
    this->SendPacket(message);
  }

  //***************************************************************************
  void Debugger::ClearExecutionPoint()
  {
    // Send a message back that we hit a breakpoint
    JsonBuilder builder;
    builder.Begin(JsonType::Object);
      builder.Key("MessageType");
      builder.Value("ClearExecutionPoint");
    builder.End();
      
    String message = builder.ToString();
    this->SendPacket(message);
  }
  
  //***************************************************************************
  void AddCodeEntryToExplorerviewUpdate(HashSet<size_t>& processedCodeHashes, JsonBuilder& builder, CodeEntry& entry)
  {
    // We want to skip any code entries we've already seen
    size_t entryHash = entry.GetHash();
    if (processedCodeHashes.Contains(entryHash))
      return;

    // Next time we see this hash, we'll know to skip it
    processedCodeHashes.Insert(entryHash);
              
    // Write out this code entry (the id is the index combined with the state id)
    builder.Begin(JsonType::Object);
    {
      builder.Key("Id");
      builder.Value(entryHash);
      builder.Key("Name");
      builder.Value(entry.Origin);
      builder.Key("CodeData");
      builder.Begin(JsonType::Object);
      {
        // All data sent within here is stored directly on the debugger side
        // and is directly returned to us in messages such as 'ViewExplorerItem'
        builder.Key("CodeHash");
        builder.Value(entry.GetHash());
      }
      builder.End();
    }
    builder.End();
  }

  //***************************************************************************
  void Debugger::UpdateExplorerView()
  {
    JsonBuilder builder;
    builder.Begin(JsonType::Object);
    {
      builder.Key("MessageType");
      builder.Value("UpdateExplorer");

      // A simple set to see if we've already processed a code entry
      HashSet<size_t> processedCodeHashes;
      
      builder.Key("Roots");
      builder.Begin(JsonType::ArrayMultiLine);
      {
        // Loop through all the projects
        for (size_t i = 0; i < this->Projects.Size(); ++i)
        {
          // Grab the current project
          Project* project = this->Projects[i];

          // Loop through all code entries in the project
          for (size_t j = 0; j < project->Entries.Size(); ++j)
          {
            // Grab the current code entry from the project
            CodeEntry& entry = project->Entries[j];
            AddCodeEntryToExplorerviewUpdate(processedCodeHashes, builder, entry);
          }
        }

        // Loop through all the states
        for (size_t i = 0; i < this->States.Size(); ++i)
        {
          // Grab the current executable state
          ExecutableState* state = this->States[i];

          // Loop through all dependent libraries so we can get the original source code files
          for (size_t j = 0; j < state->Dependencies.Size(); ++j)
          {
            // Get the current dependent library
            LibraryRef& library = state->Dependencies[j];
                
            // We only process this library if it has any entries (otheriwse we would always show core!)
            if (library->Entries.Empty())
              continue;

            // Loop through the original source code entries that built this library
            // Note: The entries may be empty if it was a generated library
            for (size_t k = 0; k < library->Entries.Size(); ++k)
            {
              // Grab the current code entry
              CodeEntry& entry = library->Entries[k];
              AddCodeEntryToExplorerviewUpdate(processedCodeHashes, builder, entry);
            }
          }
        }
      }
      builder.End();
    }
    builder.End();

    String message = builder.ToString();
    this->SendPacket(message);
  }

  
  //***************************************************************************
  void Debugger::AddProject(Project* project)
  {
    // If the project already exists within the project array, then don't add it again
    if (this->Projects.FindIndex(project) != Array<Project*>::InvalidIndex)
      return;

    this->Projects.PushBack(project);
    this->UpdateExplorerView();
  }
  
  //***************************************************************************
  void Debugger::RemoveProject(Project* project)
  {
    // Attempt to find the project in the array of all project we're viewing
    size_t index = this->Projects.FindIndex(project);

    // If it didn't exist in the array, then there's nothing for us to do!
    if (index == Array<Project*>::InvalidIndex)
      return;

    // Remove the project from the array
    this->Projects.EraseAt(index);

    // Lastly, if any client is connected then update the explorer view they have because the project is now gone
    this->UpdateExplorerView();
  }

  //***************************************************************************
  void Debugger::AddState(ExecutableState* state)
  {
    // If the state already exists within the state array, then don't add it again
    if (this->States.FindIndex(state) != Array<ExecutableState*>::InvalidIndex)
      return;

    this->States.PushBack(state);
    state->EnableDebugEvents = true;

    EventConnect(state, Events::OpcodePreStep, &Debugger::OnOpcodePreStep, this);
    EventConnect(state, Events::EnterFunction, &Debugger::OnEnterFunction, this);
    EventConnect(state, Events::ExitFunction, &Debugger::OnExitFunction, this);
    EventConnect(state, Events::UnhandledException, &Debugger::OnException, this);
    
    this->UpdateExplorerView();
  }
  
  //***************************************************************************
  void Debugger::RemoveState(ExecutableState* state)
  {
    // Attempt to find the state in the array of all states we're debugging
    size_t index = this->States.FindIndex(state);

    // If it didn't exist in the array, then there's nothing for us to do!
    if (index == Array<ExecutableState*>::InvalidIndex)
      return;

    // Remove the state from the array
    this->States.EraseAt(index);

    // If the last state we were stepping out of was this state, then clear the step out info
    if (this->StepOutOverState == state)
    {
      this->StepOutOverState = nullptr;
      this->StepOutOverCallStackDepth = 0;
    }

    // If the last state we were debugging was this state, then clear it
    if (this->LastState == state)
      this->LastState = nullptr;

    // Disable debug events
    state->EnableDebugEvents = false;
    EventDisconnect(state, this, Events::OpcodePreStep, this);
    EventDisconnect(state, this, Events::EnterFunction, this);
    EventDisconnect(state, this, Events::ExitFunction, this);
    EventDisconnect(state, this, Events::UnhandledException, this);

    // Lastly, if any client is connected then update the explorer view they have because the state is now gone
    this->UpdateExplorerView();
  }

  //***************************************************************************
  void Debugger::AddMessageHandler(StringParam type, MessageFn callback, void* userData)
  {
    // Add the delegate to the message handlers
    DebuggerMessageDelegate delegate;
    delegate.MessageCallback = callback;
    delegate.UserData = userData;
    this->MessageHandlers.InsertOrError(type, delegate);
  }

  //***************************************************************************
  void Debugger::PauseExecution(CodeLocation* codeLocation, ExecutableState* state)
  {
    // We hit a breakpoint, so pause execution
    this->Action = DebuggerAction::Pause;

    // Inform the client where we stopped in code execution
    this->SetExecutionPoint(codeLocation, state);

    // Just create a single empty event that we send just once
    DebuggerEvent toSend;
    toSend.RunningDebugger = this;
    toSend.State = state;
    toSend.Location = codeLocation;

    // Let the user know that we just paused...
    EventSend(this, Events::DebuggerPause, &toSend);

    // Loop until we hit an again that causes us to resume
    while (this->Action == DebuggerAction::Pause)
    {
      // We should probably wait on a signal here (typically we don't want to wait inside of processing received messages
      // because we often want to continue, even if there are no messages
      // For now, sleep just to prevent this from using all the cpu
      Zero::Os::Sleep(1);

      // Send out an event that lets the hosting application update its visuals (it's being debugged!)
      EventSend(this, Events::DebuggerPauseUpdate, &toSend);

      // While we're paused just pump messages (could be a breakpoint, after a step, or a true pause)
      this->Server.Update();
    }
    
    // Since we're resuming, clear execution
    this->ClearExecutionPoint();

    // Let the user know that we just resumed...
    EventSend(this, Events::DebuggerResume, &toSend);
  }

  //***************************************************************************
  void Debugger::OnAcceptedConnection(WebSocketEvent* event)
  {
    // Update the explorer view for the newly connected client
    this->UpdateExplorerView();
  }
  
  //***************************************************************************
  void Debugger::OnError(WebSocketEvent* event)
  {
    Error("Debugger Error: %s", event->ErrorStatus.Message.c_str());
  }
  
  //***************************************************************************
  void Debugger::OnDisconnected(WebSocketEvent* event)
  {
    // Make sure we clear out all the data so the program can continue
    this->Action = DebuggerAction::Resume;
    this->Breakpoints.Clear();
    this->LastLocation = CodeLocation();
    this->LastCallStackDepth = 0;
    this->LastState = nullptr;
    this->StepLocation = CodeLocation();
    this->StepOutOverState = nullptr;
  }

  //***************************************************************************
  void Debugger::OnReceivedData(WebSocketEvent* event)
  {
    // Now parse the string into a json tree
    static const String Origin("Received Json");
    CompilationErrors errors;
    JsonValue* root = JsonReader::ReadIntoTreeFromString(errors, event->Data, Origin, nullptr);
    ReturnIf(root == nullptr,, "We weren't able to parse the JSON that we received");

    // Read the message type from the json message
    String messageType = root->MemberAsString("MessageType");

    // Grab the current handler by message type
    DebuggerMessageDelegate* delegate = this->MessageHandlers.FindPointer(messageType);
    
    // As long as we registered a delegate to handle this message...
    if (delegate != nullptr)
    {
      // Invoke the debugger message callback
      DebuggerMessage message;
      message.Type = messageType;
      message.JsonRoot = root;
      delegate->MessageCallback(message, delegate->UserData);
    }
    else
    {
      Error("Unknown message type '%s'", messageType.c_str());
    }

    // Get rid of the json tree
    delete root;
  }
}
