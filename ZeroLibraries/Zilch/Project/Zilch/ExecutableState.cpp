/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  ZilchDefineType(ExecutableState, builder, type)
  {
    type->HandleManager = ZilchManagerId(PointerManager);

    ZilchFullBindField(builder, type, &ExecutableState::CallingState, "CallingState", PropertyBinding::Get);
    ZilchFullBindMethod(builder, type, &ExecutableState::ExecuteStatement, ZilchNoOverload, "ExecuteStatement", ZilchNoNames);
  }

  //***************************************************************************
  ZilchDefineType(MemoryLeakEvent, builder, type)
  {
  }

  //***************************************************************************
  ZilchDefineType(OpcodeEvent, builder, type)
  {
  }

  //***************************************************************************
  ZilchDefineType(FatalErrorEvent, builder, type)
  {
  }

  //***************************************************************************
  namespace Events
  {
    ZilchDefineEvent(FatalError);
    ZilchDefineEvent(UnhandledException);
    ZilchDefineEvent(HandledException);
    ZilchDefineEvent(OpcodePreStep);
    ZilchDefineEvent(OpcodePostStep);
    ZilchDefineEvent(EnterFunction);
    ZilchDefineEvent(ExitFunction);
    ZilchDefineEvent(MemoryLeak);
  }
  
  //***************************************************************************
  MemoryLeakEvent::MemoryLeakEvent() :
    State(nullptr),
    AllocatedLocation(nullptr)
  {
  }

  //***************************************************************************
  OpcodeEvent::OpcodeEvent() :
    State(nullptr),
    CurrentFunction(nullptr),
    ProgramCounter(InvalidOpcodeLocation),
    StackOffset(0),
    Location(nullptr)
  {
  }

  //***************************************************************************
  ZilchDefineType(ExceptionEvent, builder, type)
  {
  }
  
  //***************************************************************************
  ExceptionEvent::ExceptionEvent() :
    State(nullptr),
    ThrownException(nullptr)
  {
  }

  //***************************************************************************
  FatalErrorEvent::FatalErrorEvent() :
    ErrorCode(FatalError::Invalid)
  {
  }

  //***************************************************************************
  void PerScopeData::PerformCleanup()
  {
    // Get the things we need to cleanup
    Array<Handle*>& handleCleanup = this->HandlesToBeCleaned;
    Array<Delegate*>& delegateCleanup = this->DelegatesToBeCleaned;
    Array<Any*>& anyCleanup = this->AnysToBeCleaned;

    // We need to go through all handles on the stack and destroy them
    for (size_t i = 0; i < handleCleanup.Size(); ++i)
    {
      // Invoke the destructor (which should also decrement any reference counts)
      handleCleanup[i]->~Handle();
    }

    // We need to go through all delegates on the stack and destroy them
    for (size_t i = 0; i < delegateCleanup.Size(); ++i)
    {
      // Invoke the destructor (which should also destroy the 'this' handle)
      delegateCleanup[i]->~Delegate();
    }

    // We need to go through all anys on the stack and destroy them
    for (size_t i = 0; i < anyCleanup.Size(); ++i)
    {
      // Invoke the destructor (which should also destroy the stored value)
      anyCleanup[i]->~Any();
    }

    // Because this type is recycled, we need to clear the arrays for its next use
    handleCleanup.Clear();
    delegateCleanup.Clear();
    anyCleanup.Clear();
  }

  //***************************************************************************
  PerFrameData::PerFrameData(ExecutableState* state) :
    Frame(nullptr),
    NextFrame(nullptr),
    CurrentFunction(nullptr),
    ProgramCounter(ProgramCounterNotActive),
    State(state),
    Debug(CallDebug::None),
    Report(nullptr),
    Timeouts(0),
    ErrorState(StackErrorState::Normal)
  {
  }

  //***************************************************************************
  void PerFrameData::QueueAnyCleanup(Any* any)
  {
    // Error checking
    ErrorIf(this->Scopes.Back()->AnysToBeCleaned.FindIndex(any) != Array<Any*>::InvalidIndex,
      "We should not be queuing the same address twice for cleanup!");

    this->Scopes.Back()->AnysToBeCleaned.PushBack(any);
  }

  //***************************************************************************
  void PerFrameData::QueueHandleCleanup(Handle* handle)
  {
    // Error checking
    ErrorIf(this->Scopes.Back()->HandlesToBeCleaned.FindIndex(handle) != Array<Handle*>::InvalidIndex,
      "We should not be queuing the same address twice for cleanup!");

    this->Scopes.Back()->HandlesToBeCleaned.PushBack(handle);
  }

  //***************************************************************************
  void PerFrameData::QueueDelegateCleanup(Delegate* delegate)
  {
    // Error checking
    ErrorIf(this->Scopes.Back()->DelegatesToBeCleaned.FindIndex(delegate) != Array<Delegate*>::InvalidIndex,
      "We should not be queuing the same address twice for cleanup!");

    this->Scopes.Back()->DelegatesToBeCleaned.PushBack(delegate);
  }
  
  //***************************************************************************
  bool PerFrameData::IsVariableInitialized(Variable* variable)
  {
    // Get the stack location of the variable
    byte* variableStackMemory = this->Frame + variable->Local;

    // If this is a value type...
    if (variable->ResultType->IsCopyComplex() == false)
    {
      // Consider it initialized... for now
      return true;
    }
    else
    {
      // Grab the curren't function's type, because we keep reusing it
      DelegateType* delegateType = this->CurrentFunction->FunctionType;

      // If the variable is the this handle...
      byte* thisLocation = delegateType->ThisHandleStackOffset + this->Frame;
      if (thisLocation == variableStackMemory)
        return true;

      // If the pointer is any of the parameters...
      ParameterArray& parameters = delegateType->Parameters;
      for (size_t k = 0; k < parameters.Size(); ++k)
      {
        // Get the current offset of the parameter...
        byte* parameterLocation = parameters[k].StackOffset + this->Frame;
                    
        // If the parameter is the current variable we're looking at...
        if (parameterLocation == variableStackMemory)
          return true;
      }

      // Walk through all scopes
      for (size_t k = 0; k < this->Scopes.Size(); ++k)
      {
        // Grab the current scope
        PerScopeData* scope = this->Scopes[k];
                    
        // Loop through all handles, see if this pointer matches...
        for (size_t m = 0; m < scope->HandlesToBeCleaned.Size(); ++m)
        {
          if (scope->HandlesToBeCleaned[m] == (Handle*)variableStackMemory)
            return true;
        }
                    
        // Loop through all delegates, see if this pointer matches...
        for (size_t m = 0; m < scope->DelegatesToBeCleaned.Size(); ++m)
        {
          if (scope->DelegatesToBeCleaned[m] == (Delegate*)variableStackMemory)
            return true;
        }
                    
        // Loop through all any objects, see if this pointer matches...
        for (size_t m = 0; m < scope->AnysToBeCleaned.Size(); ++m)
        {
          if (scope->AnysToBeCleaned[m] == (Any*)variableStackMemory)
            return true;
        }
      }
    }

    // We didn't find that it was initialized... it must not be
    return false;
  }

  //***************************************************************************
  bool PerFrameData::AttemptThrowStackExceptions(ExceptionReport& report)
  {
    // If we already know about the stack overflow, then ignore it
    if (this->State->HitStackOverflow)
      return false;
    
    // If we reached the max recursion depth (not necessarily an overflow, but definitely a limit)
    if (this->ErrorState == StackErrorState::MaxRecursionReached)
    {
      this->State->HitStackOverflow = true;
      this->State->ThrowException(report, "Maximum recursion depth reached (too many function called inside of other functions)");
      return true;
    }
    // If we truly ran out of stack space...
    else if (this->ErrorState == StackErrorState::Overflowed)
    {
      this->State->HitStackOverflow = true;
      this->State->ThrowException(report, "The stack ran out of space (too many function called inside of other functions)");
      return true;
    }

    // No exceptions were thrown if we got here
    return false;
  }

  //***************************************************************************
  OperandLocation::OperandLocation() :
    Type(OperandType::NotSet),
    Memory(nullptr),
    MemorySize(0),
    Offset(0)
  {
  }

  //***************************************************************************
  Timeout::Timeout() :
    LengthTicks(0),
    AccumulatedTicks(0)
  {
  }
    
  //***************************************************************************
  ExceptionReport::ExceptionReport() :
    ForceThrownExceptions(false)
  {
  }
  
  //***************************************************************************
  void ExceptionReport::Clear()
  {
    this->ForceThrownExceptions = false;
    this->Exceptions.Clear();
    this->ExceptionsForDebugOnly.Clear();
  }

  //***************************************************************************
  bool ExceptionReport::HasThrownExceptions()
  {
    return this->ForceThrownExceptions || this->Exceptions.Empty() == false;
  }
  
  //***************************************************************************
  String ExceptionReport::GetConcatenatedMessages()
  {
    // Walk through all the exceptions and concatenate them
    StringBuilderExtended builder;
    for (size_t i = 0; i < this->Exceptions.Size(); ++i)
    {
      // Read each of the exceptions from the handle
      Exception* exception = (Exception*)this->Exceptions[i].Dereference();

      // Write out the exception message
      builder.Write(exception->Message);

      // If this isn't the last exception...
      if (i != (this->Exceptions.Size() - 1))
      {
        // Write out something that joins them together
        builder.Write(" / ");
      }
    }
    
    // Output the concatenated exceptions
    return builder.ToString();
  }

  //***************************************************************************
  void DefaultExceptionCallback(ExceptionEvent* e)
  {
    // Print out the standard formatted error message to the console
    printf("%s\n", e->ThrownException->GetFormattedMessage(MessageFormat::Zilch).c_str());
  }

  //***************************************************************************
  static const size_t DefaultStackSize = 2097152;
  static const String DefaultName("ExecutableState");
  ExecutableState* ExecutableState::CallingState = nullptr;

  //***************************************************************************
  ExecutableState::ExecutableState() :
    StackSize(DefaultStackSize),
    OverflowStackSize(DefaultStackSize),
    UserData(nullptr),
    MaxRecursionDepth(40),
    HitStackOverflow(false),
    TimeoutSeconds(0),
    Name(DefaultName),
    PatchId(0),
    EnableDebugEvents(false),
    DoNotAllowAllocation(0),
    UniqueIdScopeCounter(1),
    AllocatingType(nullptr)
  {
    ZilchErrorIfNotStarted(ExecutableState);

    // Reserve space for frames and scopes
    this->StackFrames.Reserve(128);
    this->RecycledFrames.Reserve(128);
    this->RecycledScopes.Reserve(128);

    // Then add the rest of our handle managers
    this->HeapObjects = this->GetHandleManager<HeapManager>();
    this->StackObjects = this->GetHandleManager<StackManager>();
    this->PointerObjects = this->GetHandleManager<PointerManager>();

    // Clear the stack (including reserved space)
    size_t totalStackSize = this->StackSize + this->OverflowStackSize;
    this->Stack = new byte[totalStackSize];
    memset(this->Stack, 0xCD, totalStackSize);

    // Create a frame data that points to the beginning of the stack
    // This frame should never be popped (FrameData should have a minimum size of 1)
    PerFrameData* baseFrame = new PerFrameData(this);
    this->StackFrames.PushBack(baseFrame);

    // Generate an empty function to help reduce code paths
    // When any function wants to get the next stack position,
    // they generally look at the current function + required space
    // The 'empty function' has a required space of zero (0) always
    Function* emptyFunction = new Function();

    // Initialize the frame data to defaults, except the stack pointer
    baseFrame->CurrentFunction = emptyFunction;
    baseFrame->Frame = this->Stack;
    baseFrame->NextFrame = this->Stack;
    baseFrame->ProgramCounter = ProgramCounterNotActive;
    baseFrame->State = this;
    baseFrame->Debug = CallDebug::None;
  }

  //***************************************************************************
  ExecutableState::~ExecutableState()
  {
    // We're not allowed to delete an executable state that isn't done executing
    ErrorIf(this->StackFrames.Size() > 1, "Illegal to delete an ExecutableState that has a running stack frame");

    // We should always have the base frame
    ErrorIf(this->StackFrames.Size() == 0, "Base frame should always exist (this is bad)");

    // In general no objects should still be existing by this point in time unless the user allocated and stored
    // handles to objects, especially non-reference counted objects
    
    // Because the entire handle manager gets torn down and will no longer allow objects to be allocated, then static memory
    // must be cleaned up first because it could access the static memory (we don't delete the memory yet, we just null it and release handles)
    // We actually delete the memory for statics AFTER destructing the handle managers
    // We could either do this, or we could destruct statics afterwards but make it throw if you try to allocate
    // We currently destruct handled objects first, and then we destruct statics (but allocations are not allowed during destructors)
    // and then afterward we actually tear down the memory for both statics and handle managers

    // We need to clean up memory for static variables (release handles/delegates)
    // Techincally code could run during this phase (even code that accesses other deleted statics)
    // To be entirely safe, we wait to delete the static memory (so we don't magically attempt to allocate it again on access)
    typedef Pair<Field*, byte*> StaticFieldPair;
    ZilchForEach(StaticFieldPair& pair, this->StaticFieldToMemory)
    {
      // Pull out the two values from the pair for convenience
      Field* field = pair.first;
      byte* staticMemory = pair.second;

      // Destruct the memory in place and then delete the memory
      // Note: When we actually flag statics as being initialized, keep that flag on
      // so we don't try and reinitialize it in case someone refers to it below
      field->PropertyType->GenericDestruct(staticMemory);

      // We only really need to memset the object to zero if it has a complex copy
      if (field->PropertyType->IsCopyComplex())
        memset(staticMemory, 0, field->PropertyType->GetCopyableSize());
    }
    
    // Tell all the handle managers to delete their objects (only the ones owned by this ExecutableState)
    ZilchForEach(HandleManager* manager, this->UniqueManagers.Values())
    {
      // Walk through all allocated objects in this manager and delete their objects
      manager->DeleteAll(this);
    }

    // Loop through all native v-tables we created
    HashMap<BoundType*, byte*>::valuerange virtualTables = this->NativeVirtualTables.Values();
    while (virtualTables.Empty() == false)
    {
      // Clean up each v-table and move on
      delete virtualTables.Front();
      virtualTables.PopFront();
    }

    // Delete all handle managers
    ZilchForEach(HandleManager* manager, this->UniqueManagers.Values())
    {
      // Delete the current handle manager
      delete manager;
    }

    // Now delete all static memory
    ZilchForEach(StaticFieldPair& pair, this->StaticFieldToMemory)
    {
      // Get the static memory and delete it
      byte* staticMemory = pair.second;
      delete[] staticMemory;
    }

    // Destroy the stack
    delete[] this->Stack;

    // We created a fake function as a 'base frame' function (need to clean it up)
    PerFrameData* baseFrame = this->StackFrames.Back();
    delete baseFrame->CurrentFunction;
    delete baseFrame;
  }

  //***************************************************************************
  byte* ExecutableState::GetCurrentStackFrame()
  {
    return this->StackFrames.Back()->Frame;
  }

  //***************************************************************************
  byte* ExecutableState::GetNextStackFrame()
  {
    // Get the current frame data
    PerFrameData* current = this->StackFrames.Back();

    // Compute the frame from the current position
    // (moved forward by the used stack space)
    return current->NextFrame;
  }

  //***************************************************************************
  PerFrameData* ExecutableState::PushFrame(Function* function)
  {
    // Get the next frame before we push our own frame data
    byte* frame = this->GetNextStackFrame();

    // Call the internal function
    return this->PushFrame(frame, function);
  }

  //***************************************************************************
  PerFrameData* ExecutableState::PushFrame(byte* frame, Function* function)
  {
    // Unfortunately we incur an overhead for patched functions, however, this should
    // be descently quick if the patched functions hash table is empty (it 'early outs' internally)
    // This either finds a patched function, or returns the same function
    function = this->PatchedFunctions.FindValue(function, function);

    // Store a pointer to the newly created stack frame
    PerFrameData* newFrame = nullptr;

    // If we have no recycled stack frames...
    if (this->RecycledFrames.Empty())
    {
      // There was nothing to be pulled from recycling,
      // so just make a new one (will be recycled later)
      newFrame = new PerFrameData(this);
    }
    else
    {
      // Otherwise, we just pop the latest one from the recycled list
      newFrame = this->RecycledFrames.Back();
      this->RecycledFrames.PopBack();
      newFrame->Report = nullptr;
    }

    // Clear any error state set on the frame data
    newFrame->ErrorState = StackErrorState::Normal;

    // If we got more calls then the max recursion depth...
    if (this->StackFrames.Size() >= this->MaxRecursionDepth)
    {
      newFrame->ErrorState = StackErrorState::MaxRecursionReached;
    }

    // Compute the next frame on the stack
    byte* nextFrame = frame + function->RequiredStackSpace;

    // Compute the end of the real stack
    byte* endOfStack = this->Stack + this->StackSize;

    // If the next frame exceeds our max stack size...
    if (nextFrame >= endOfStack)
    {
      newFrame->ErrorState = StackErrorState::Overflowed;
    }

    // If we actually hit the end of the stack, including reserve size, then there's basically nothing we can do!
    if (nextFrame >= endOfStack + this->OverflowStackSize || this->StackFrames.Size() >= this->MaxRecursionDepth * 2)
    {
      // Throw a fatal error!
      FatalErrorEvent toSend;
      toSend.ErrorCode = FatalError::StackReserveOverflow;
      EventSend(this, Events::FatalError, &toSend);

      // Time to quit the process!
      Error("We hit a stack overflow more than once (used up the reserved stack space). The fatal callback was called, and now we're aborting");
      abort();
    }

    // If this is the first call being made into the state and we have a default timeout set
    // Note: This must be done before we push the stack frame below
    if (this->TimeoutSeconds != 0 && this->IsInCallStack() == false)
    {
      // Push the timeout with the number of seconds that the user set
      this->PushTimeout(newFrame, this->TimeoutSeconds);
    }

    // Take the newly allocated (or recycled) stack frame and push it onto the stack frames list
    this->StackFrames.PushBack(newFrame);

    // Make sure we cleared out any scopes from before
    ErrorIf(newFrame->Scopes.Empty() == false, "Improperly recycled stack frame");

    // Setup the frame data before we call the function
    // We need to initialize ALL members since this is recycled (also avoids constructors)
    // Note we don't need to set the State, as it should never change
    newFrame->Frame = frame;
    newFrame->NextFrame = nextFrame;
    newFrame->CurrentFunction = function;
    newFrame->ProgramCounter = ProgramCounterNotActive;
    newFrame->Debug = CallDebug::None;

    // Since every function gets an implicit scope, we'll add one to start
    newFrame->Scopes.PushBack(this->AllocateScope());

    // Return the data we just created
    return newFrame;
  }

  //***************************************************************************
  PerFrameData* ExecutableState::PopFrame()
  {
    // Get the frame we're about to pop
    PerFrameData* frame = this->StackFrames.Back();

    // We MUST save the timeout count as a temporary, because popping timeouts will decrement it
    size_t timeoutCount = frame->Timeouts;

    // Loop through all timeouts that still exist
    // Note: It is VERY important that we do this before cleaning up scopes (delegates/handles)
    // because destructors could get invoked and end up throwing exceptions again due to the timeout still existing
    for (size_t i = 0; i < timeoutCount; ++i)
    {
      // Pop any timeouts for this frame
      this->PopTimeout(frame);
    }

    // Loop through all the scopes
    for (size_t i = 0; i < frame->Scopes.Size(); ++i)
    {
      // Get the current scope
      PerScopeData* scope = frame->Scopes[i];

      // Cleanup everything from this scope
      scope->PerformCleanup();

      // Recycle this scope (we will clear the scopes below)
      this->RecycledScopes.PushBack(scope);
    }

    // We need to clean out all scopes (including the implicitly created first one)
    // because we are going to be recycling the frame
    frame->Scopes.Clear();

    // Recycle the stack frame
    this->RecycledFrames.PushBack(frame);

    // Pop the stack frame
    this->StackFrames.PopBack();

    // If we hit the stack overflow...
    if (this->HitStackOverflow)
    {
      // If we popped enough frames to come out of stack overlfow mode...
      byte* endOfStack = this->Stack + this->StackSize;
      if (frame->NextFrame < endOfStack && this->StackFrames.Size() < this->MaxRecursionDepth)
      {
        // Clear the flag that lets us use extra stack space
        this->HitStackOverflow = false;
      }
    }

    // Make sure the dummy always exists
    ErrorIf(this->StackFrames.Empty(), "We popped the dummy stack frame and were not supposed to!");
    
    // Validation of timeouts
    ErrorIf(this->StackFrames.Size() == 1 && this->Timeouts.Empty() == false,
      "If we popped the last stack frame (except the dummy) then all timeouts should be gone!");

    // Even though the frame is invalid, we should return it for debugging
    return frame;
  }

  //***************************************************************************
  Any ExecutableState::ExecuteStatement(StringParam code)
  {
    Project project;

    // If an error occurs, this callback will output the error directly into this string
    String errorMessage;
    EventConnect(&project, Events::CompilationError, OutputErrorStringCallback, &errorMessage);

    // Add the code in and treat it as a statement
    project.AddCodeFromString(code);
    LibraryRef library = project.Compile(ExpressionLibrary, this->Dependencies, EvaluationMode::Expression);
    
    // If we failed to compile the library, output whatever error occurred
    if (library == nullptr)
      return Any(errorMessage, this);

    // Patch the state with the new library
    // Ideally we wouldn't have to do this, but if the user ever does anything to refer to the calling function
    // and say, bind it to a delegate, attach it to an event handler, etc... then calling that function could crash
    // Patching it will keep it alive for the remainder of our executable state
    this->ForcePatchLibrary(library);

    static const String ProgramError("Failed to find the program type (internal error)");
    static const String MainError("Failed to find the main function (internal error)");

    // Try and find the program type
    BoundType* program = library->BoundTypes.FindValue(ExpressionProgram, nullptr);
    if (program == nullptr)
      return Any(ProgramError, this);

    // Try and find the main function
    Function* main = program->FindFunction(ExpressionMain, Array<Type*>(), ZilchTypeId(Any), FindMemberOptions::Static);
    if (main == nullptr)
      return Any(MainError, this);

    // Invoke the main function and let it do whatever its going to do
    ExceptionReport report;
    Call call(main, this);
    call.Invoke(report);

    // If an exception was thrown, concatenate all the messages into a string
    if (report.HasThrownExceptions())
    {
      String messages = report.GetConcatenatedMessages();
      return Any(messages, this);
    }

    return call.Get<Any>(Call::Return);
  }

  //***************************************************************************
  void ExecutableState::InitializeStackHandle(Handle& handle, byte* location, PerScopeData* scope, BoundType* type)
  {
    // Set the handle's manager
    handle.Manager = this->StackObjects;

    // Set the type of this handle
    handle.StoredType = type;

    // We are always guaranteed that the handle data is cleared before we get the user data portion
    StackHandleData& data = *(StackHandleData*)handle.Data;
    data.StackLocation = location;
    data.UniqueId = scope->UniqueId;
    data.Scope = scope;
  }
  
  //***************************************************************************
  void ExecutableState::InitializePointerHandle(Handle& handle, byte* location, BoundType* type)
  {
    // Set the handle's manager
    handle.Manager = this->PointerObjects;

    // Set the type of this handle
    handle.StoredType = type;

    // We are always guaranteed that the handle data is cleared before we get the user data portion
    this->PointerObjects->ObjectToHandle(location, type, handle);
  }

  //***************************************************************************
  bool ExecutableState::PushTimeout(PerFrameData* frame, size_t seconds)
  {
    // When pushing the timeout we do a validation for any previous timeouts
    // Note: This is very important because it also updates the timer's last query
    // This will early out and skip pushing a timer
    if (this->ThrowExceptionOnTimeout(*frame->Report))
      return true;

    // Create a timeout structure...
    Timeout& timeout = this->Timeouts.PushBack();
          
    // Set the length of the timeout based on the constant stored in the opcode
    timeout.LengthTicks = seconds * Timer::TicksPerSecond;

    // Increment the timeouts stored on that frame, so if an exception happens
    // or we need to unroll (return) then we know how many timeouts to pop
    ++frame->Timeouts;

#if ZeroDebug
    // This is not necessary, but it's nice for debugging
    // We use this to verify that 
    timeout.Frame = frame;
#endif

    // We didn't throw an exception above, so return false
    return false;
  }

  //***************************************************************************
  bool ExecutableState::PopTimeout(PerFrameData* frame)
  {
    // We assume we're not going to throw any exceptions, see below
    bool result = false;

    // If we already have an exception that we're dealing with, then don't try and throw another!
    if (frame->Report->HasThrownExceptions() == false)
    {
      // When popping the timeout we do one last validation
      result = this->ThrowExceptionOnTimeout(*frame->Report);
    }

#if ZeroDebug
    // Verify that the frame is the same one that pushed this timeout
    ErrorIf(this->Timeouts.Back().Frame != frame,
      "An incorrect frame was used to pop a timeout (not the one that pushed it!)");
#endif

    // Pop the last timeout off the stack
    this->Timeouts.PopBack();

    // We no longer need to pop this timeout if the stack unrolls
    --frame->Timeouts;

    // Return whether we threw an exception or not
    return result;
  }

  //***************************************************************************
  ExceptionReport& ExecutableState::GetCallingReport()
  {
    ExecutableState* state = CallingState;
    Array<PerFrameData*>& frames = state->StackFrames;
    for (int i = (int)(frames.Size() - 1); i >= 0; --i)
    {
      ExceptionReport* report = frames[i]->Report;
      if (report != nullptr)
        return *report;
    }

    state->DefaultReport.Clear();
    return state->DefaultReport;
  }
  
  //***************************************************************************
  ExecutableState* ExecutableState::GetCallingState()
  {
    return CallingState;
  }

  //***************************************************************************
  void ExecutableState::SendOpcodeEvent(StringParam eventId, PerFrameData* frame)
  {
    // If the user didn't enable debug events, then early out
    if (this->EnableDebugEvents == false)
      return;

    // If anyone is listening to the callback...
    EventDelegateList* delegates = this->OutgoingPerEventName.FindValue(eventId, nullptr);
    if (delegates != nullptr)
    {
      // Get the stack offset that we're at
      size_t stackOffset = (size_t)(frame->Frame - frame->State->Stack);
      
      // Get the program counter (we use it also to lookup the opcode location)
      size_t lookupProgramCounter = frame->ProgramCounter;
      if (lookupProgramCounter == ProgramCounterNotActive)
        lookupProgramCounter = 0;
      
      // Let the user know that we've stepped once
      OpcodeEvent toSend;
      toSend.State = this;
      toSend.CurrentFunction = frame->CurrentFunction;
      toSend.ProgramCounter = frame->ProgramCounter;
      toSend.StackOffset = stackOffset;
      toSend.Location = frame->CurrentFunction->GetCodeLocationFromProgramCounter(lookupProgramCounter);
      delegates->Send(&toSend);
    }
  }

  //***************************************************************************
  PerScopeData* ExecutableState::AllocateScope()
  {
    // The new scope we'll return
    PerScopeData* newScope = nullptr;

    // If we have no recycled scopes...
    if (this->RecycledScopes.Empty())
    {
      // There was nothing to be pulled from recycling,
      // so just make a new one (will be recycled later)
      newScope = new PerScopeData();
    }
    else
    {
      // Otherwise, we just pop the latest one from the recycled list
      newScope = this->RecycledScopes.Back();
      this->RecycledScopes.PopBack();
    }

    // Every scope must be assigned a new id so that any references we form
    // to stack variables inside the scope will be safely pointed at
    // If the scope goes away, this unique id gets set back to 0, or if it gets re-used
    // then we make sure it will never have the same id it had before
    newScope->UniqueId = this->UniqueIdScopeCounter;
    ++this->UniqueIdScopeCounter;

    // Return the newly created or recycled scope
    return newScope;
  }

  //***************************************************************************
  void ExecutableState::InvokePreConstructorOrRelease(Handle& handle, ExceptionReport& report)
  {
    // Error checking
    ErrorIf(report.HasThrownExceptions(),
      "Exceptions still set on the report when attempting to invoke the pre-constructor");

    // Iterate through this type and all it's base types, so we can call pre-constructors on each of them
    // Technically, the order should not matter!
    BoundType* type = handle.StoredType;
    while (type != nullptr)
    {
      // If we have a pre-constructor...
      Function* preConstructor = type->PreConstructor;
      if (preConstructor != nullptr)
      {
        // Call the preconstructor
        Call call(preConstructor, this);
        call.SetHandle(Call::This, handle);
        call.Invoke(report);

        // If we failed to execute the pre constructor, return a null handle
        if (report.HasThrownExceptions())
        {
          // Clear the handle to a null handle (which should also delete the object)
          handle = Handle();
          return;
        }
      }
      
      // Iterate to the next base type
      type = type->BaseType;
    }
  }

  //***************************************************************************
  void ExecutableState::UpdateCppVirtualTable(byte* objectWithBaseVTable, BoundType* cppBaseType, BoundType* derivedType)
  {
    // Error checking
    ErrorIf(cppBaseType->BoundNativeVirtualCount > cppBaseType->RawNativeVirtualCount,
      "We should never have more bound native virtual functions than the actual v-table size");

    // First, check to see if this object is native or not
    if (cppBaseType->RawNativeVirtualCount == 0 || cppBaseType->BoundNativeVirtualCount == 0)
    {
      return;
    }

    // Get a pointer to the virtual table
    TypeBinding::VirtualTableFn*& virtualTable = *(TypeBinding::VirtualTableFn**)objectWithBaseVTable;

    // Check to see if we've already mapped up this virtual table
    byte* foundVirtualTable = this->NativeVirtualTables.FindValue(cppBaseType, nullptr);

    // If we did find it (already made one)
    if (foundVirtualTable != nullptr)
    {
      // Just map the object's virtual table to this one (plus offset)
      virtualTable = (TypeBinding::VirtualTableFn*)(foundVirtualTable + sizeof(ExecutableState*));
      return;
    }

    // Compute the exact size of the native v-table in bytes
    size_t nativeVTableSizeBytes = cppBaseType->RawNativeVirtualCount * sizeof(TypeBinding::VirtualTableFn);

    // Create the new virtual table to store the executable state,
    // and a copy (with replacements) of the native virtual table
    byte* fullVirtualTable = new byte[sizeof(BoundType*) + sizeof(ExecutableState*) + nativeVTableSizeBytes];

    // Insert the type at the front of the v-table so we can resolve the virtual function
    // This must be properly destructed with the executable state
    new (fullVirtualTable) BoundType*(derivedType);

    // Insert the executable state at the front
    new (fullVirtualTable + sizeof(BoundType*)) ExecutableState*(this);

    // The full virtual table stores extra data at the front, but we only
    // want a view that looks like the native virtual table
    TypeBinding::VirtualTableFn* newVirtualTable = (TypeBinding::VirtualTableFn*)(fullVirtualTable + sizeof(BoundType*) + sizeof(ExecutableState*));

    // Now copy the actual v-table from the object into the new virtual table
    memcpy(newVirtualTable, virtualTable, nativeVTableSizeBytes);

    // We need to walk all parent types to generate the table
    BoundType* parentCppType = cppBaseType;

    // Loop until we hit the end of the parent types
    while (parentCppType != nullptr)
    {
      // Get all the instance functions
      FunctionMultiValueRange allInstanceFunctions = parentCppType->InstanceFunctions.Values();

      // Loop until all the instance functions are empty
      while (allInstanceFunctions.Empty() == false)
      {
        // Grab the current array of instance functions
        FunctionArray& functionArray = allInstanceFunctions.Front();
        allInstanceFunctions.PopFront();

        // Loop through all instance functions (those are the only virtuals!)
        for (size_t i = 0; i < functionArray.Size(); ++i)
        {
          // Grab the current instance function
          Function* function = functionArray[i];

          // If the function is native virtual...
          if (function->NativeVirtual.Index != NativeVirtualInfo::NonVirtual)
          {
            // Map the thunk mixed with the bound function
            GuidType guid = derivedType->Hash() ^ function->NativeVirtual.Guid;

            BoundType* parentZilchType = derivedType;
            Function* mostDerived = nullptr;

            do
            {
              const FunctionArray* functionArray = parentZilchType->GetOverloadedInstanceFunctions(function->Name);

              if (functionArray != nullptr)
              {
                for (size_t j = 0; j < functionArray->Size(); ++j)
                {
                  Function* currentFunction = (*functionArray)[j];

                  if (Type::IsSame(currentFunction->FunctionType, function->FunctionType))
                  {
                    mostDerived = currentFunction;
                  }
                }
              }

              parentZilchType = parentZilchType->BaseType;

              if (parentZilchType == nullptr)
              {
                Error("We should never reach a null base class before hitting the Cpp type");
                break;
              }
            }
            while (parentZilchType != cppBaseType);
            
            // If we found that the user overwrote the vtable entry...
            if (mostDerived != nullptr)
            {
              this->ThunksToFunctions.InsertOrError(guid, mostDerived);

              // Error checking for v-table indices
              ErrorIf(function->NativeVirtual.Index >= cppBaseType->RawNativeVirtualCount,
                "We should never have a native virtual whose index is greater than the total v-table indices");

              // Override the function in the v-table with our own thunk
              newVirtualTable[function->NativeVirtual.Index] = function->NativeVirtual.Thunk;
            }
          }
        }
      }

      // Move to the next parent type
      parentCppType = parentCppType->BaseType;
    }

    // Add the virtual table to our known v-tables
    this->NativeVirtualTables.InsertOrError(cppBaseType, fullVirtualTable);

    // Finally, point this object instance at the new virtual table
    virtualTable = newVirtualTable;
  }

  //***************************************************************************
  class RemappedField
  {
  public:
    byte* OldMemory;
    byte* NewMemory;
    Type* SameType;
  };
  
  //***************************************************************************
  void ExecutableState::ForcePatchLibrary(LibraryRef newLibrary)
  {
    // If the library is the core, just early out (it will never change)
    if (newLibrary == Core::GetInstance().GetLibrary())
      return;

    // Figure out which old library we're currently patching (walk our dependencies)
    LibraryRef oldLibrary = nullptr;
    for (size_t i = 0; i < this->Dependencies.Size(); ++i)
    {
      // If any library has the same name as the one we're patching with, we'll patch it (otherwise skip it!)
      LibraryRef library = this->Dependencies[i];
      if (library->Name == newLibrary->Name)
      {
        oldLibrary = library;
        break;
      }
    }

    // If we didn't find an old library to patch by name, then early out
    if (oldLibrary == nullptr)
      return;

    // Increment the patch id, so that the user can re-enable certain features (for example, event connections)
    ++this->PatchId;

    // We need to store the new library on ourselves because we're going to directly store their functions (need to keep them alive!)
    this->PatchedLibraries.PushBack(newLibrary);

    // Loop through all bound types in the old library
    BoundTypeValueRange oldBoundTypes = oldLibrary->BoundTypes.Values();
    while (oldBoundTypes.Empty() == false)
    {
      // Grab the current type (it may exist in the new version)
      BoundType* oldType = oldBoundTypes.Front();
      oldBoundTypes.PopFront();

      // Look for a type by the same name in the new library (this can totally be null!)
      BoundType* newTypeOrNull = newLibrary->BoundTypes.FindValue(oldType->Name, nullptr);
        
      // If we found a new type, we need to scan through all instances of that type in memory and modify them
      if (newTypeOrNull != nullptr)
      {
        // Let the state know that we're patching
        this->PatchedBoundTypes.Insert(oldType, newTypeOrNull);
        
        ZilchTodo("We MUST respect the HeapManagerExtraPatchSize to make sure we don't go outside! What do we do in that case though... fail patching?");

        // Loop through all heap objects and check if any of them are the old type
        ZilchForEach(const byte* object, this->HeapObjects->LiveObjects)
        {
          // Just behind the allocated object is the header
          ObjectHeader& header = *(ObjectHeader*)(object - sizeof(ObjectHeader));

          // Remember, we only compare names, which means the oldHeapType can actually be different than oldType
          // This is especially true after we've patched multiple times (the object gets updated to a newer object, but still isn't the original!)
          BoundType* oldHeapType = header.Type;
          BoundType* newHeapType = newTypeOrNull;

          // If the name matches the object's name... (if not, skip it)
          if (oldHeapType->Name != newTypeOrNull->Name)
            continue;

          // Update the type on the slot's header
          header.Type = newTypeOrNull;

          // Create a temporary buffer to copy all the values from the old heap type over
          size_t oldSize = oldHeapType->GetAllocatedSize();
          byte* temporaryBuffer = new byte[oldSize];
          memset(temporaryBuffer, 0x00, oldSize);

          byte* memory = (byte*)object;

          Array<RemappedField> remappedFields;
          HashSet<Field*> handledNewFields;

          // We have to destruct or copy all old fields to a temporary location before remapping them to the new type
          FieldMapValueRange oldInstanceFields = oldHeapType->InstanceFields.Values();
          while (oldInstanceFields.Empty() == false)
          {
            Field* oldInstanceField = oldInstanceFields.Front();
            oldInstanceFields.PopFront();

            Field* newInstanceField = newHeapType->InstanceFields.FindValue(oldInstanceField->Name, nullptr);

            byte* oldInstanceFieldMemory = memory + oldInstanceField->Offset;

            // If there's a new field AND it is of the same type...
            if (newInstanceField != nullptr && Type::IsSame(oldInstanceField->PropertyType, newInstanceField->PropertyType))
            {
              byte* temporaryOldInstanceFieldMemory = temporaryBuffer + oldInstanceField->Offset;
              newInstanceField->PropertyType->GenericCopyConstruct(temporaryOldInstanceFieldMemory, oldInstanceFieldMemory);

              handledNewFields.InsertOrError(newInstanceField);

              RemappedField& remappedField = remappedFields.PushBack();
              remappedField.NewMemory = memory + newInstanceField->Offset;
              remappedField.OldMemory = temporaryOldInstanceFieldMemory;
              remappedField.SameType = newInstanceField->PropertyType;
            }

            // We need to destruct the old field memory (regardless of whether we're remapping it)
            // Remember, we copied it to a temporary buffer in the case we did a remap (above)
            oldInstanceField->PropertyType->GenericDestruct(oldInstanceFieldMemory);
          }

          // Walk through only the values that need to be remapped
          for (size_t i = 0; i < remappedFields.Size(); ++i)
          {
            RemappedField& remappedField = remappedFields[i];
              
            remappedField.SameType->GenericCopyConstruct(remappedField.NewMemory, remappedField.OldMemory);
            remappedField.SameType->GenericDestruct(remappedField.OldMemory);
          }

          delete[] temporaryBuffer;
            
          // Now walk through all new instance fields, running initialization functions on them...
          FieldMapValueRange newInstanceFields = newHeapType->InstanceFields.Values();
          while (newInstanceFields.Empty() == false)
          {
            Field* newInstanceField = newInstanceFields.Front();
            newInstanceFields.PopFront();

            // Skip any fields we already handled (those that were copied over, for example)
            if (handledNewFields.Contains(newInstanceField))
              continue;

            byte* newInstanceFieldMemory = memory + newInstanceField->Offset;
            newInstanceField->PropertyType->GenericDefaultConstruct(newInstanceFieldMemory);
          }
        }
      }
        
      // Walk through all old defined functions
      FunctionArrayRange oldFunctions = oldType->AllFunctions.All();
      while (oldFunctions.Empty() == false)
      {
        // Grab the current function from the old type
        Function* oldFunction = oldFunctions.Front();
        oldFunctions.PopFront();
          
        Function* newFunctionOrNull = nullptr;

        // If we actually found a new type (that maps from the old type)
        // Then we're also going to attempt to find a new function that matches the signature
        if (newTypeOrNull != nullptr)
        {
          // Attempt to find a function with the exact same delegate signature and the same name (as well as being an instance or static)
          FindMemberOptions::Enum options = FindMemberOptions::DoNotIncludeBaseClasses;
          if (oldFunction->IsStatic)
            options = (FindMemberOptions::Enum)(options | FindMemberOptions::Static);
          newFunctionOrNull = newTypeOrNull->FindFunction(oldFunction->Name, oldFunction->FunctionType, options);
        }

        // If we didn't find a new function that patches over the old function (we're going to have to create one!)
        if (newFunctionOrNull == nullptr)
        {
          // The patch function directly refers to members of the other function, because we know the lifetime
          // of those primitives should match the lifetime of the patch
          // For example, the 'Type' DelegateType is owned by the oldLibrary, same with 'This'
          Function* patchDummy = new Function();
          patchDummy->Owner               = oldFunction->Owner;
          patchDummy->Name                = oldFunction->Name;
          patchDummy->FunctionType        = oldFunction->FunctionType;
          patchDummy->OwningProperty      = oldFunction->OwningProperty;
          patchDummy->SourceLibrary       = oldFunction->SourceLibrary;
          patchDummy->This                = oldFunction->This;
          patchDummy->SourceLibrary       = oldFunction->SourceLibrary;
          patchDummy->Description         = oldFunction->Description;
          patchDummy->Remarks             = oldFunction->Remarks;
          patchDummy->Attributes          = oldFunction->Attributes;
          patchDummy->IsHidden            = oldFunction->IsHidden;
          patchDummy->IsVirtual           = oldFunction->IsVirtual;
          patchDummy->Location            = oldFunction->Location;
          // NativeVirtual
            
          // The patch dummy is a special function that returns default constructed values (runs no code)
          patchDummy->BoundFunction = VirtualMachine::PatchDummy;

          // We technically only need enough stack space to match the delegate call, and nothing else (no local variables, etc)
          // However, because we want to match the amount of data for parameter passing (and the this handle), we just use the old function's space
          patchDummy->RequiredStackSpace = oldFunction->RequiredStackSpace;

          // Map the old function to the new function (currently this overwrites!)
          this->PatchedFunctions.Insert(oldFunction, patchDummy);
        }
        else
        {
          // NOTE: These are all the members we DO NOT need to patch over:
          // Owner is not copied because the type that owns this is still the same (just patching the function, not the type yet)
          // The type will be patched on its own anyways!

          // The name is not copied because it should be the exact same
          ErrorIf(oldFunction->Name != newFunctionOrNull->Name, "A function we were patching did not match its new name");
          // We found this function by type, therefore the types should be the same (maybe not the same pointer, but type identity!)
          ErrorIf(Type::IsSame(oldFunction->FunctionType, newFunctionOrNull->FunctionType) == false, "A function we were patching did not match the new delegate type");
          // A function can't suddenly change from being a property delegate to not being one...
          if (oldFunction->OwningProperty && newFunctionOrNull->OwningProperty)
            ErrorIf(oldFunction->OwningProperty->Name != newFunctionOrNull->OwningProperty->Name, "A function we were patching did not match IsPropertyGetOrSet");
          else // This check is making sure that if one is null the other is also null
            ErrorIf((oldFunction->OwningProperty != nullptr) || (newFunctionOrNull->OwningProperty != nullptr), "A function we were patching changed the state of having an owning property");

          // Map the old function to the new function (currently this overwrites!)
          this->PatchedFunctions.Insert(oldFunction, newFunctionOrNull);
        }
      }
    }

    // Now we've gone through all the new types and all their functions,
    // its time to go back through old types and stub out their functions
  }

  //***************************************************************************
  void ExecutableState::PatchLibrary(LibraryRef newLibrary)
  {
    // We cannot patch while an executable state is currently running
    ReturnIf(this->StackFrames.Size() > 1,, "Illegal to patch a library in an ExecutableState that has a running stack frame");
    this->ForcePatchLibrary(newLibrary);
  }

  //***************************************************************************
  void ExecutableState::SetTimeout(size_t seconds)
  {
    // Error checking
    ReturnIf(this->IsInCallStack(),,
      "You cannot set a timeout while inside a call-stack");

    // Store the timeout
    this->TimeoutSeconds = seconds;
  }

  //***************************************************************************
  bool ExecutableState::IsInCallStack()
  {
    // Error checking
    ErrorIf(this->StackFrames.Empty(),
      "The stack frames should never be empty (we should always have a dummy!)");

    // Technically we always have 1 dummy frame that exists
    return (this->StackFrames.Size() != 1);
  }

  //***************************************************************************
  Handle ExecutableState::AllocateStackObject(byte* stackLocation, PerScopeData* scope, BoundType* type, ExceptionReport& report)
  {
    // Verify that the given pointer is within our stack
    ErrorIf(stackLocation < this->Stack || stackLocation > this->Stack + this->StackSize,
      "The given stack location for allocating a stack object was not within our stack");

    // Clear the memory of the stack location
    memset(stackLocation, 0, type->Size);

    // Create a stack handle to point at the given location
    Handle handle;
    this->InitializeStackHandle(handle, stackLocation, scope, type);

    // Pre-construct the handle (initialize memory) or clear it to null if we fail
    this->InvokePreConstructorOrRelease(handle, report);

    // Return the stack handle
    return handle;
  }

  //***************************************************************************
  Handle ExecutableState::AllocateDefaultConstructedHeapObject(BoundType* type, ExceptionReport& report, HeapFlags::Enum flags)
  {
    // If we were given an invalid type to allocate, return early
    if (type == nullptr)
    {
      Error("Given an invalid type to 'AllocateDefaultConstructedHeapObject' (null type)");
      return Handle();
    }

    // Grab all the constructors available
    const FunctionArray* constructors = type->GetOverloadedInheritedConstructors();

    // As long as we have constructors (could be inherited!)
    if (constructors != nullptr && constructors->Empty() == false)
    {
      // If the default constructor is null...
      Function* defaultConstructor = BoundType::GetDefaultConstructor(constructors);
      if (defaultConstructor != nullptr)
      {
        // Allocate the heap object
        Handle handle = this->AllocateHeapObject(type, report, flags);

        // If allocating the heap object with just the pre-constructor threw an exception, early out
        if (report.HasThrownExceptions())
        {
          handle.Delete();
          return Handle();
        }

        // Let base classes know (especially in plugins) what the derived class is that is being constructed
        // Most likely the plugin will use this type to construct a handle
        BoundType* oldAllocatingType = this->AllocatingType;
        this->AllocatingType = type;

        // Execute the constructor which returns a call
        Call call(defaultConstructor, this);
        call.SetHandle(Call::This, handle);
        call.Invoke(report);

        this->AllocatingType = oldAllocatingType;

        // If we failed to execute the default constructor, return a null handle
        if (report.HasThrownExceptions())
          return Handle();

        // Return the handle, even if we couldn't construct the object
        return handle;
      }
      else
      {
        // Show an error since we couldn't construct the object
        Error("The default constructor could not be found for type '%s' but constructors were provided", type->ToString().c_str());
        return Handle();
      }
    }
    // Otherwise, we have no constructors (this is ok so long as we aren't native!)
    else if (type->Native == false)
    {
      // Just pre-construct the object
      return this->AllocateHeapObject(type, report, flags);
    }
    
    // Show an error since we couldn't construct the object
    Error("The default constructor could not be found for type '%s' and native types require a default constructor", type->ToString().c_str());
    return Handle();
  }

  //***************************************************************************
  Handle ExecutableState::AllocateDefaultConstructedHeapObject(BoundType* type, HeapFlags::Enum flags)
  {
    ExceptionReport report;
    Handle resultHandle = this->AllocateDefaultConstructedHeapObject(type, report, flags);
    return resultHandle;
  }

  //***************************************************************************
  Handle ExecutableState::AllocateCopyConstructedHeapObject(BoundType* type, ExceptionReport& report, HeapFlags::Enum flags, const Handle& fromObject)
  {
    type->IsInitializedAssert();

    // If we were given an invalid type to allocate, return early
    if (type == nullptr)
    {
      Error("Given an invalid type to 'AllocateCopyConstructedHeapObject' (null type)");
      return Handle();
    }

    // Grab all the constructors available
    const FunctionArray* constructors = type->GetOverloadedInheritedConstructors();

    // As long as we have constructors (could be inherited!)
    if (constructors != nullptr && constructors->Empty() == false)
    {
      // If the copy constructor was found...
      Function* copyConstructor = BoundType::GetCopyConstructor(constructors);
      if (copyConstructor != nullptr)
      {
        // Allocate the heap object
        Handle handle = this->AllocateHeapObject(type, report, flags);

        // If allocating the heap object with just the pre-constructor threw an exception, early out
        if (report.HasThrownExceptions())
        {
          handle.Delete();
          return Handle();
        }

        // Execute the constructor which returns a call
        Call call(copyConstructor, this);
        call.SetHandle(Call::This, handle);
        call.SetHandle(0, fromObject);
        call.Invoke(report);

        // If we failed to execute the default constructor, return a null handle
        if (report.HasThrownExceptions())
          return Handle();

        // Return the handle, even if we couldn't construct the object
        return handle;
      }
    }

    // Show an error since we couldn't construct the object
    Error
    (
      "The copy constructor could not be found for type '%s'. "
      "This is often required for when an object is returned to Zilch "
      "via binding if the object has no safe handle manager itself. "
      "If you are using automatic binding call ZilchBindConstructor(const %s&);",
      type->ToString().c_str(),
      type->ToString().c_str()
    );
    return Handle();
  }

  //***************************************************************************
  Handle ExecutableState::AllocateHeapObject(BoundType* type, ExceptionReport& report, HeapFlags::Enum flags)
  {
    // If we were given an invalid type to allocate, return early
    if (type == nullptr)
    {
      Error("Given an invalid type to 'AllocateHeapObject' (null type)");
      return Handle();
    }
    
    if (type->IsInitializedAssert() == false)
      return Handle();

    // If we currently are not allowing allocation, then throw an exception
    // This is sort of strange, because we can't even allocate the exception...
    if (this->DoNotAllowAllocation != 0)
    {
      // Even though the exception cannot be allocated, it will still be reported to C++ callbacks and will still unroll
      ZilchTodo("We should make this throw an exception, but then it cannot be allocated currently... InternalException?");
      return Handle();
    }

    // Let base classes know (especially in plugins) what the derived class is that is being constructed
    // Most likely the plugin will use this type to construct a handle
    BoundType* oldAllocatingType = this->AllocatingType;
    this->AllocatingType = type;

    // Let the heap objects initialize the handle
    Handle handle;
    handle.StoredType = type;
    handle.Manager = HandleManagers::GetInstance().GetManager(type->HandleManager, this);
    handle.Manager->Allocate(type, handle, flags);

    //HACK (forces all handles to be direct pointers)
    //byte* obj = handle.Dereference();
    //handle.Manager = this->PointerObjects;
    //this->PointerObjects->ObjectToHandle(obj, handle);

    // Pre-construct the handle (initialize memory) or clear it to null if we fail
    this->InvokePreConstructorOrRelease(handle, report);

    this->AllocatingType = oldAllocatingType;

    // Return the handle
    return handle;
  }

  //***************************************************************************
  bool ExecutableState::ThrowExceptionOnTimeout(ExceptionReport& report)
  {
    // Get the ticks since last check (this also updates the timer to now)
    // This MUST be called before the early out so that we don't accumulate up time not spent in Zilch
    long long ticksSinceLastCheck = this->TimeoutTimer.GetAndUpdateTicks();

    // Reset the timer so the the timer always returns us 'time passed since last check'
    this->TimeoutTimer.Reset();

    // Early out if we don't have any timeouts to abide by
    if (this->Timeouts.Empty())
      return false;

    // Get the time that we expect to timeout at
    Timeout& timeout = this->Timeouts.Back();

    // Accumulate ticks for our timer
    timeout.AccumulatedTicks += ticksSinceLastCheck;
    
    // If we exceed the timeout, we need to throw an exception and stop everything
    if (timeout.AccumulatedTicks > timeout.LengthTicks)
    {
      // Throw an exception to say we timed out
      this->ThrowException
      (
        report,
        String::Format
        (
          "Exceeded the allowed execution time of %d second(s). Use the timeout statement to increase allowed time",
          timeout.LengthTicks / Timer::TicksPerSecond
        )
      );

      // We threw the exception and thus we return 'true', the timeout occurred
      return true;
    }

    // Otherwise, we got here so nothing bad happened
    return false;
  }

  //***************************************************************************
  void ExecutableState::ThrowNullReferenceException(ExceptionReport& report)
  {
    // Throw a null reference exception
    this->ThrowException(report, "Attempted to access a member of a null object");
  }

  //***************************************************************************
  void ExecutableState::ThrowNullReferenceException(ExceptionReport& report, StringParam customMessage)
  {
    // Throw a null reference exception
    this->ThrowException
    (
      report,
      String::Format("Attempted to access a member of a null object: %s", customMessage.c_str())
    );
  }

  //***************************************************************************
  void ExecutableState::ThrowNotImplementedException()
  {
    this->ThrowException("This method is not implemented (its implementation may be abstract and a virutal function should overwrite it)");
  }

  //***************************************************************************
  void ExecutableState::ThrowException(StringParam message)
  {
    // Grab the report from the latest stack frame
    ExceptionReport& report = this->GetCallingReport();
    this->ThrowException(report, message);
  }

  //***************************************************************************
  void ExecutableState::ThrowException(ExceptionReport& report, StringParam message)
  {
    Core& core = Core::GetInstance();

    // Allocate a default constructed base exception
    // Note: We only allocate the exception and DO NOT default construct it
    // This is because we know the exception will be fully initialized, and moreover if we run out of allocation space
    // then returning a null handle is considered ok!
    // The StackTrace allocator originally did not support memset to zero, but we made a special one called MemsetZeroDefaultAllocator
    ExceptionReport defaultExceptionReport;
    Handle handle = this->AllocateHeapObject(core.ExceptionType, defaultExceptionReport, HeapFlags::ReferenceCounted);
    if (handle.Manager != nullptr)
      handle.Manager->SetNativeTypeFullyConstructed(handle, true);

    // Error testing
    ReturnIf(defaultExceptionReport.HasThrownExceptions(),,
      "Allocating the default exception object should NEVER throw an exception");

    // Dereference a handle and grab a pointer to the exception object
    byte* memory = handle.Dereference();

    // Because an exception can be null if we truly run out of memory, then just skip this portion
    if (memory != nullptr)
    {
      // Grab a reference to the exception memory
      Exception& exception = *(Exception*)memory;

      // Set the friendly and exact error messages of the exception
      exception.Message = message;
    }
    
    // Forward this to the normal function that throws exceptions
    this->ThrowException(report, handle);
  }

  //***************************************************************************
  void ExecutableState::ThrowException(ExceptionReport& report, Handle& handle)
  {
    ZilchTodo("We need to verify that this handle is indeed a handle to an Exception type");
    
    // Dereference a handle and grab a pointer to the exception object
    Exception* exception = (Exception*)handle.Dereference();

    // If the exception was unable to be allocated we use this instead (max stack depth, out of objects, etc)
    Exception unableToAllocateException;

    // Only add the exception to the report if it exists and was allocated
    if (exception != nullptr)
    {
      // Add the current exception to the list of active exceptions
      report.Exceptions.PushBack(handle);

      // The user should never touch this, it's only for debug!
      report.ExceptionsForDebugOnly.PushBack(exception);
    }
    else
    {
      // We couldn't allocate the exception, so use a dummy one on the stack with a custom message
      exception = &unableToAllocateException;
      unableToAllocateException.Message = "The exception could not be allocated (most likely we hit the max stack depth, ran out of memory, or an exception was thrown in a destructor)";

      // We still want exception behavior, but we can't actually add the exception to the report (so force it!)
      report.ForceThrownExceptions = true;
    }

    // Generate a stack trace for the exception
    this->GetStackTrace(exception->Trace);

    // Inform the user that an exception occurred
    ExceptionEvent toSend;
    toSend.State = this;
    toSend.ThrownException = exception;
    EventSend(this, Events::UnhandledException, &toSend);
  }
  
  //***************************************************************************
  void ExecutableState::GetStackTrace(StackTrace& trace)
  {
    // Walk the stack frames from the bottom to top, building a call stack
    for (size_t i = 1; i < this->StackFrames.Size(); ++i)
    {
      // Grab the current frame
      PerFrameData* frame = this->StackFrames[i];

      // Detect if this is the last stack frame
      bool isLastFrame = (i == (this->StackFrames.Size() - 1));

      // As long as this frame's program counter is active in some sort...
      if (frame->ProgramCounter != ProgramCounterNotActive || (isLastFrame && trace.Stack.Empty()))
      {
        // Create a new stack entry for this frame
        StackEntry& stackEntry = trace.Stack.PushBack();

        // Add the current function to the stack entry
        Function* function = frame->CurrentFunction;
        stackEntry.ExecutingFunction = function;

        // If this is a frame that we're managing (so we can get more call stack information)
        // (Non-Active-Functions): For right now, since we don't have better location information about where the
        // function is (using the first opcode is WRONG and points at the wrong location)
        // We just opt to print the function out as if it's native, the user can search for it...
        size_t programCounter = frame->ProgramCounter;
        if (programCounter != ProgramCounterNative && programCounter != ProgramCounterNotActive)
        {
          // Get the code location (or null if for some reason we can't find it...)
          CodeLocation* codeLocation = function->OpcodeLocationToCodeLocation.FindPointer(programCounter);

          // If we found the code location
          if (codeLocation != nullptr)
          {
            // Set the code location on the stack entry
            stackEntry.Location = *codeLocation;
          }
          else
          {
            Error("Unable to find code location for an opcode that belongs to a function (invalid opcode offset?)");
          }
        }
        else
        {
          // Initialize the location to the bound function
          stackEntry.Location.Origin = function->SourceLibrary->Name;
          stackEntry.Location.Library = function->SourceLibrary->Name;
          stackEntry.Location.Class = function->Owner->Name;
          stackEntry.Location.Function = function->Name;
        }
      }
    }
  }

  //***************************************************************************
  const byte* ExecutableState::GetRawStack()
  {
    return this->Stack;
  }
  
  //***************************************************************************
  byte* ExecutableState::GetStaticField(Field* field, ExceptionReport& report)
  {
    // Look for the static memory in a map of the fields on our state
    // Static fields are done per executable state, so they get wiped each time we quit
    byte*& staticMemory = this->StaticFieldToMemory[field];

    // If no memory was allocated yet, then allocate some and clear it
    if (staticMemory == nullptr)
    {
      // Allocate enough memory to store the field
      size_t fieldSize = field->PropertyType->GetCopyableSize();
      staticMemory = new byte[field->PropertyType->GetCopyableSize()];
      
      // All handles, value types, etc support being memset to 0
      memset(staticMemory, 0, fieldSize);

      // If an initializer exists, then invoke that
      if (field->Initializer != nullptr)
      {
        // Unless an exception gets thrown, the field should be initialized after this
        // Note that user code may cause cycles in initialization (which is why we memset first)
        Call call(field->Initializer, this);
        call.Invoke(report);
      }
    }

    // Return the pointer to the static memory
    return staticMemory;
  }

  //***************************************************************************
  void Call::PerformStandardChecks(size_t size, Type* userType, Type* actualType, CheckPrimitive::Enum primitive, Direction::Enum io)
  {
    // Check that the size is the same
    ErrorIf(AlignToBusWidth(size) < actualType->GetCopyableSize(),
      "The size of the types did not match");

    // If this is a value type...
    if (primitive == CheckPrimitive::Value)
    {
      // Check that the size of the value we're writing to is correct
      ErrorIf(actualType->IsCopyComplex(),
        "The type must be a value type");

      if (userType != nullptr)
      {
        if (Type::IsSame(userType, actualType) == false)
        {
          if (!((Type::IsEnumOrFlagsType(actualType)) && Type::IsSame(userType, ZilchTypeId(Integer))))
          {
            // Make sure we're trying to set the same type as the parameter
            Error("The user's type and the parameter/return/this type are not compatible");
          }
        }
        
      }

    }
    // If this is a reference/handle type...
    else if (primitive == CheckPrimitive::Handle)
    {
      // If the user type is a bound type
      BoundType* boundUserType = Type::DynamicCast<BoundType*>(userType);
      if (boundUserType != nullptr)
      {
        // If this is a value type being pointed at by a handle...
        if (boundUserType->CopyMode == TypeCopyMode::ValueType)
        {
          IndirectionType* indirectionActualType = Type::DynamicCast<IndirectionType*>(actualType);

          if (indirectionActualType != nullptr)
          {
            // For now, we can't do much else
            userType = nullptr;
          }
          else
          {
            Error("Handles that refer to value types can only be passed in when the type is a ref type (indirect type)");
          }
        }
      }

      // Make sure the return type is a handle type
      ErrorIf(Type::IsHandleType(actualType) == false,
        "The parameter/return/this type is not a handle type");

      // If there was a provided user type to check
      if (userType != nullptr)
      {
        // Make sure the given type is a handle type
        ErrorIf(Type::IsHandleType(userType) == false,
          "The user's type is not a handle type");
        
        // We need to verify that the parameter is the same type
        // If we're getting a value...
        if (io == Direction::Get)
        {
          // Error checking
          ErrorIf(actualType->IsRawCastableTo(userType) == false,
            "When getting a handle, the parameter/return/this must either "
            "derive from or be the same type as what we're trying to get");
        }
        // Otherwise we're setting a value
        else
        {
          // Error checking
          ErrorIf(userType->IsRawCastableTo(actualType) == false,
            "When setting a handle, the handle must either derive "
            "from or be the same type as the parameter/return/this");
        }
      }
    }
    // If this is a delegate type...
    else if (primitive == CheckPrimitive::Delegate)
    {
      // Check that the delegate types are the same
      ErrorIf(Type::IsDelegateType(actualType) == false,
        "The parameter/return/this type is not a delegate type");

      // Check that the delegate types are the same
      ErrorIf(userType != nullptr && Type::IsDelegateType(userType) == false,
        "The user's type is not a delegate type");

      // Check that the delegate types are the same
      ErrorIf(userType != nullptr && Shared::GetInstance().GetCastOperator(userType, actualType).Operation != CastOperation::Raw,
        "Attempting to pass in a delegate of an incorrect type");
    }
    // It must be the 'any' type...
    else
    {
      // Make sure this is the 'any type'
      ErrorIf(Type::IsAnyType(actualType) == false,
        "The parameter/return/this type is not the 'any' type");

      // If the user type was provided, make sure it's also the any type
      ErrorIf(userType != nullptr && Type::IsAnyType(userType) == false,
        "The user's type is not the 'any' type");
    }
  }

  //***************************************************************************
  byte* Call::GetChecked(size_t index, size_t size, Type* userType, CheckPrimitive::Enum primitive, Direction::Enum io)
  {
    // If we're setting this... we need to mark that we did for debugging!
    if (io == Direction::Set)
    {
      // Mark and perform debug checks
      this->MarkAsSet(index);
    }

    // Based on the type of index...
    switch (index)
    {
    // It's an index that represents the return
    case Return:
      return this->GetReturnChecked(size, userType, primitive, io);
      
    // It's an index that represents the 'this' handle
    case This:
      return this->GetThisChecked(size, userType, primitive, io);
      
    // It's an index that represents any parameter
    default:
      return this->GetParameterChecked(index, size, userType, primitive, io);
    }
  }

  //***************************************************************************
  byte* Call::GetUnchecked(size_t index)
  {
    // Based on the type of index...
    switch(index)
    {
    // It's an index that represents the return
    case Return:
      return this->GetReturnUnchecked();

    // It's an index that represents the 'this' handle
    case This:
      return this->GetThisUnchecked();

    // It's an index that represents any parameter
    default:
      return this->GetParameterUnchecked(index);
    }
  }

  //***************************************************************************
  byte* Call::GetThisChecked(size_t size, Type* userType, CheckPrimitive::Enum primitive, Direction::Enum io)
  {
    // As long as the user didn't disable checks...
    if (!(this->Data->Debug & CallDebug::NoThisChecking))
    {
      // Make sure we don't try to request the 'this' handle as something weird
      ErrorIf(primitive != CheckPrimitive::Handle,
        "The 'this' handle for a function can only be retrieved as"
        " a handle/reference type, not as a value or delegate");

      // Get a reference to the this variable
      Variable* thisVariable = this->Data->CurrentFunction->This;

      // Error checking for if we're static
      ErrorIf(thisVariable == nullptr,
        "Cannot get the 'this' handle stack location for a static function");
    
      // Run a series of checks that tries to verify anything we can about what the user is doing
      this->PerformStandardChecks(size, userType, thisVariable->ResultType, primitive, io);
    }

    // Get the stack pointer where the 'this' should go
    return this->GetThisUnchecked();
  }

  //***************************************************************************
  byte* Call::GetReturnChecked(size_t size, Type* userType, CheckPrimitive::Enum primitive, Direction::Enum io)
  {
    // As long as the user didn't disable checks...
    if (!(this->Data->Debug & CallDebug::NoReturnChecking))
    {
      // Verify that this is not a void type
      ErrorIf(Core::GetInstance().VoidType == this->Data->CurrentFunction->FunctionType->Return,
        "The return type is void and cannot be get/set");
    
      // Run a series of checks that tries to verify anything we can about what the user is doing
      this->PerformStandardChecks(size, userType, this->Data->CurrentFunction->FunctionType->Return, primitive, io);
    }
    
    // Get the stack pointer where the return should go
    return this->GetReturnUnchecked();
  }

  //***************************************************************************
  byte* Call::GetParameterChecked(size_t parameterIndex, size_t size, Type* userType, CheckPrimitive::Enum primitive, Direction::Enum io)
  {
    // Get a reference to the parameters
    ParameterArray& parameters = this->Data->CurrentFunction->FunctionType->Parameters;
    
    // Error checking for the index
    ErrorIf(parameterIndex >= parameters.Size(),
      "Attempting to access an invalid parameter by index (out of bounds)");
    
    // Get the current parameter
    DelegateParameter& parameter = parameters[parameterIndex];

    // As long as the user didn't disable checks...
    if (!(this->Data->Debug & CallDebug::NoParameterChecking))
    {
      // Run a series of checks that tries to verify anything we can about what the user is doing
      this->PerformStandardChecks(size, userType, parameter.ParameterType, primitive, io);
    }
    
    // Get the stack pointer where the parameter should go
    return this->Data->Frame + parameter.StackOffset;
  }

  //***************************************************************************
  void Call::SetValue(size_t index, const byte* input, size_t size)
  {
    // Get the stack location and perform checks
    byte* stack = this->GetChecked(index, size, nullptr, CheckPrimitive::Value, Direction::Set);

    // Copy the input into the stack position
    memcpy(stack, input, size);
  }
  
  //***************************************************************************
  void Call::SetHandle(size_t index, const Handle& value)
  {
    // Get the stack location and perform checks
    byte* stack = this->GetChecked(index, sizeof(Handle), value.StoredType, CheckPrimitive::Handle, Direction::Set);

    // Now copy the handle to the stack
    new (stack) Handle(value);
  }
  
  //***************************************************************************
  void Call::SetDelegate(size_t index, const Delegate& value)
  {
    // Get the exact function we're referencing
    Function* function = value.BoundFunction;

    // Get the stack location and perform checks
    byte* stack = this->GetChecked(index, sizeof(Delegate), function->FunctionType, CheckPrimitive::Delegate, Direction::Set);

    // Now copy the handle to the stack
    new (stack) Delegate(value);
  }
  
  //***************************************************************************
  void Call::GetValue(size_t index, byte* output, size_t size)
  {
    // Get the stack location and perform checks
    byte* stack = this->GetChecked(index, size, nullptr, CheckPrimitive::Value, Direction::Get);
    
    // Copy the stack into the output
    memcpy(output, stack, size);
  }
    
  //***************************************************************************
  Handle& Call::GetHandle(size_t index)
  {
    // Get the stack location and perform checks
    byte* stack = this->GetChecked(index, sizeof(Handle), nullptr, CheckPrimitive::Handle, Direction::Get);

    // Return a reference to the stack
    return *(Handle*)stack;
  }
    
  //***************************************************************************
  byte* Call::GetHandlePointer(size_t index)
  {
    // Get the stack location and perform checks
    return this->GetChecked(index, sizeof(Handle), nullptr, CheckPrimitive::Handle, Direction::Get);
  }
    
  //***************************************************************************
  Delegate& Call::GetDelegate(size_t index)
  {
    // Get the stack location and perform checks
    byte* stack = this->GetChecked(index, sizeof(Delegate), nullptr, CheckPrimitive::Delegate, Direction::Get);

    // Return a reference to the stack
    return *(Delegate*)stack;
  }

  //***************************************************************************
  byte* Call::GetDelegatePointer(size_t index)
  {
    // Get the stack location and perform checks
    return this->GetChecked(index, sizeof(Delegate), nullptr, CheckPrimitive::Delegate, Direction::Get);
  }

  //***************************************************************************
  ExecutableState* Call::GetState()
  {
    return this->Data->State;
  }

  //***************************************************************************
  byte* Call::GetStackUnchecked()
  {
    return this->Data->Frame;
  }
    
  //***************************************************************************
  byte* Call::GetThisUnchecked()
  {
    // Get the stack offsetted by to the 'this' handle location
    return this->Data->Frame + this->Data->CurrentFunction->FunctionType->ThisHandleStackOffset;
  }

  //***************************************************************************
  byte* Call::GetParametersUnchecked()
  {
    // Get the stack offsetted by the size of the return (to where the parameters are)
    return this->Data->Frame + this->Data->CurrentFunction->FunctionType->Return->GetCopyableSize();
  }

  //***************************************************************************
  byte* Call::GetParameterUnchecked(size_t parameterIndex)
  {
    // Get a reference to the parameters
    ParameterArray& parameters = this->Data->CurrentFunction->FunctionType->Parameters;

    // Error checking for the index
    ErrorIf(parameterIndex >= parameters.Size(),
      "Attempting to access an invalid parameter index (out of bounds)");

    // Return the stack offsetted by the current parameter's position
    return this->Data->Frame + parameters[parameterIndex].StackOffset;
  }

  //***************************************************************************
  byte* Call::GetReturnUnchecked()
  {
    // General error checking for our own assumptions
    ErrorIf(this->Data->CurrentFunction->FunctionType->ReturnStackOffset != 0,
      "Unexpected stack return location (internal error)");

    // The returns always exist at the beginning
    return this->Data->Frame;
  }

  //***************************************************************************
  Function* Call::GetFunction()
  {
    return this->Data->CurrentFunction;
  }

  //***************************************************************************
  Property* Call::GetProperty()
  {
    return this->Data->CurrentFunction->OwningProperty;
  }
  
  //***************************************************************************
  void Call::DisableParameterChecks()
  {
    this->Data->Debug = (CallDebug::Enum)(this->Data->Debug | CallDebug::NoParameterChecking);
  }
  
  //***************************************************************************
  void Call::DisableThisChecks()
  {
    this->Data->Debug = (CallDebug::Enum)(this->Data->Debug | CallDebug::NoThisChecking);
  }
  
  //***************************************************************************
  void Call::DisableReturnChecks()
  {
    this->Data->Debug = (CallDebug::Enum)(this->Data->Debug | CallDebug::NoReturnChecking);
  }
  
  //***************************************************************************
  void Call::DisableParameterDestruction()
  {
    this->Data->Debug = (CallDebug::Enum)(this->Data->Debug | CallDebug::NoParameterDestruction);
  }
  
  //***************************************************************************
  void Call::DisableThisDestruction()
  {
    this->Data->Debug = (CallDebug::Enum)(this->Data->Debug | CallDebug::NoThisDestruction);
  }
  
  //***************************************************************************
  void Call::DisableReturnDestruction()
  {
    this->Data->Debug = (CallDebug::Enum)(this->Data->Debug | CallDebug::NoReturnDestruction);
  }

  //***************************************************************************
  void Call::MarkAsSet(size_t index)
  {
    // Based on the type of index...
    switch (index)
    {
    // It's an index that represents the return
    case Return:
      this->MarkReturnAsSet();
      break;
      
    // It's an index that represents the 'this' handle
    case This:
      this->MarkThisAsSet();
      break;
      
    // It's an index that represents any parameter
    default:
      this->MarkParameterAsSet(index);
      break;
    }
  }

  //***************************************************************************
  void Call::MarkReturnAsSet()
  {
    // Make sure we only call this once
    ErrorIf((this->Data->Debug & CallDebug::SetReturn) != 0,
      "Attempting to set the return twice");
      
    // For debugging, mark that we set the return
    this->Data->Debug = (CallDebug::Enum)(this->Data->Debug | CallDebug::SetReturn);
  }

  //***************************************************************************
  void Call::MarkThisAsSet()
  {
    // Make sure we only call this once
    ErrorIf((this->Data->Debug & CallDebug::SetThis) != 0,
      "Attempting to set the this handle twice");
      
    // For debugging, mark that we set the 'this' handle
    this->Data->Debug = (CallDebug::Enum)(this->Data->Debug | CallDebug::SetThis);
  }

  //***************************************************************************
  void Call::MarkParameterAsSet(size_t parameterIndex)
  {
    // We have a bit for each parameter we set
    CallDebug::Enum parameterFlag = (CallDebug::Enum)(1 << parameterIndex);

    // Make sure we only call this once
    ErrorIf((this->Data->Debug & parameterFlag) != 0,
      "Attempting to set the parameter twice");
      
    // For debugging, mark that we set this parameter
    this->Data->Debug = (CallDebug::Enum)(this->Data->Debug | parameterFlag);
  }

  //***************************************************************************
  Call::Call(Function* function, ExecutableState* state)
  {
    // If this is call being generated for the user, then we have to push the frame
    // The virtual machine will do these steps for us when it's performing the call
    ErrorIf(function == nullptr, "Attempting to invoke a null function in Call");

    // Push a new stack frame for the function we want to invoke
    this->Data = state->PushFrame(function);
  }

  //***************************************************************************
  Call::Call(const Delegate& delegate, ExecutableState* state)
  {
    // The delegate directly stores the function to be executed on it
    Function* function = delegate.BoundFunction;
    ErrorIf(function == nullptr, "Attempting to invoke a null delegate in Call");

    // Push a new stack frame for the function we want to invoke
    this->Data = state->PushFrame(function);

    // If this is a non-static delegate
    if (function->This != nullptr)
    {
      // Set the this handle
      this->SetHandle(Call::This, delegate.ThisHandle);
    }
  }

  //***************************************************************************
  Call::Call(PerFrameData* data)
  {
    // We still need to grab the stack
    this->Data = data;

    // For calls being made by the VM, we don't care about parameters or 'this' being checked
    // We also don't want anything to be destructed
    // Having said that, we still want the return to be checked as it's set by the called
    this->Data->Debug = (CallDebug::Enum)
      (CallDebug::NoParameterChecking     |
       CallDebug::NoThisChecking          |
       CallDebug::NoParameterDestruction  |
       CallDebug::NoThisDestruction       |
       CallDebug::NoReturnDestruction);
  }

  //***************************************************************************
  Call::~Call()
  {
    //HACK WE CURRENTLY DONT HANDLE EXCEPTIONS
    ZilchTodo("Make sure we handle exceptions here (could have thrown before returning)");

    // For convenience, get the current function
    Function* function = this->Data->CurrentFunction;
    ExecutableState* state = this->Data->State;

    // Get the stack offset that we're at
    size_t stackOffset = (size_t)(this->Data->Frame - this->Data->State->Stack);

    // Grab the parameters of the function type
    ParameterArray& parameters = function->FunctionType->Parameters;

    // If parameter destruction is enabled...
    if ((this->Data->Debug & CallDebug::NoParameterDestruction) == 0)
    {
      // Loop through all the parameters and destruct anything that needs to be destructed
      for (size_t i = 0; i < parameters.Size(); ++i)
      {
        // Get the current parameter
        DelegateParameter& parameter = parameters[i];

        // Get the stack offset of that parameter
        byte* stack = this->Data->Frame + parameter.StackOffset;

        // Release / destruct that parameter
        parameter.ParameterType->GenericDestruct(stack);
      }
    }

    // If 'this' handle destruction is enabled...
    if ((this->Data->Debug & CallDebug::NoThisDestruction) == 0)
    {
      // If this is an instance (non-static) function
      if (function->This != nullptr)
      {
        // Get the stack offset of the this handle
        byte* thisStack = this->Data->Frame + function->FunctionType->ThisHandleStackOffset;

        // Destroy the this handle
        ((Handle*)thisStack)->~Handle();
      }
    }
    
    // If we're ignoring outputs (return)
    if ((this->Data->Debug & CallDebug::NoReturnDestruction) == 0)
    {
      // Get the core library just for the void type
      Core& core = Core::GetInstance();

      // As long as our return isn't void...
      if (function->FunctionType->Return != core.VoidType)
      {
        // Get the return stack frame (should be right at the front)
        byte* returnStack = this->Data->Frame;
      
        // Verify that return position is always at the front
        ErrorIf(function->FunctionType->ReturnStackOffset != 0,
          "Internal error, the return stack position was not 0");

        // Finally, release/destruct the return value
        function->FunctionType->Return->GenericDestruct(returnStack);
      }
    }

    // If the call was invoked, we need to pop
    if (this->Data->Debug & CallDebug::Invoked)
    {
      state->SendOpcodeEvent(Events::ExitFunction, this->Data);
    }

    // Pop all frames up to our own
    // Note: This is very important since it's possible that other frames may exist that have no Call owner
    // If an exception gets thrown between PrepForCall / FunctionCall opcodes, we will have an extra frame on the stack
    PerFrameData* poppedFrame = nullptr;
    do
    {
      // Pop the frame and get back what we just popped
      poppedFrame = state->PopFrame();
    }
    // Loop until we pop our own
    while (poppedFrame != this->Data);

    // Clear out our data, just for safety
    this->Data = nullptr;
  }
  
  //***************************************************************************
  bool Call::Invoke()
  {
    return this->Invoke(this->GetState()->GetCallingReport());
  }

  //***************************************************************************
  bool Call::Invoke(ExceptionReport& report)
  {
    // If this stack frame needed to throw any exceptions (such as stack overflow or maximum recursion depth...)
    if (this->Data->AttemptThrowStackExceptions(report))
    {
      // Early out, there's no need to do anything else from this call
      return false;
    }

    // Make sure we're not doing a call inside a call
    // At some point in time a code refactor happened that introduced the PerFrameData* onto the Call object
    // The scary part about this was that we were still getting the topFrame using 'state->StackFrames.Back()',
    // instead of just accessing the PerFrameData* we stored (currently called Data), not sure why this was
    // as maybe it was just missed code in a cleanup, but if we ever falsely get this assert this may be why!
    ErrorIf(this->Data != this->Data->State->StackFrames.Back(),
      "The function being invoked should always be the top of the frame (it is illegal to run a Call inside another Call)");

    // Store the top frame locally for efficiency
    PerFrameData* topFrame = this->Data;

    // Check if any exceptions are left on the state
    ErrorIf(report.HasThrownExceptions(),
      "Attempting to call another function when there are exceptions present in the report");

    // Make sure we don't call this twice
    ErrorIf((topFrame->Debug & CallDebug::Invoked) != 0,
      "Attempting to invoke the function twice via the same call");

    // Make sure the 'this' handle was set
    ErrorIf(!(topFrame->Debug & CallDebug::NoThisChecking) &&
      topFrame->CurrentFunction->This != nullptr &&
      (topFrame->Debug & CallDebug::SetThis) == 0,
      "The 'this' handle was not set before invoking the function");

    // Make a bit mask that includes 1s for all parameters that we have
    CallDebug::Enum allParameters = (CallDebug::Enum)((1 << topFrame->CurrentFunction->FunctionType->Parameters.Size()) - 1);

    // Make sure all parameters have been set
    ErrorIf(!(topFrame->Debug & CallDebug::NoParameterChecking) &&
      allParameters != (topFrame->Debug & allParameters),
      "Not all of the parameters were set");

    // Set the exception reporter on the top frame
    topFrame->Report = &report;

    // Get a reference to the state for convenience
    ExecutableState* state = topFrame->State;

    // Also grab the current function we're executing
    Function* function = topFrame->CurrentFunction;
    size_t stackOffset = (size_t)(topFrame->Frame - state->Stack);

    // If this function is a non-static function
    if (function->This != nullptr)
    {
      // We need to check to see if the 'this' handle is valid
      // Technically this is slower and maybe we should only do it for functions that are not the VM function
      // It does make it a lot safer (so people writing external functions can always assume 'this' is valid)
      Handle& thisHandle = *(Handle*)(topFrame->Frame + function->This->Local);

      // Dereference the handle and get a pointer back (may be null)
      byte* thisData = thisHandle.Dereference();

      // If the 'this' handle was actually null...
      if (thisData == nullptr)
      {
        // Throw the null reference exception and jump out
        state->ThrowNullReferenceException(report, "Attempted to call a member function on a null object");

        // We didn't even invoke the function, don't destruct the return type
        this->DisableReturnDestruction();
        return false;
      }
    }

    // If anyone wants to know if we entered a function...
    state->SendOpcodeEvent(Events::EnterFunction, this->Data);

    // Set the flag that says we've invoked the function
    topFrame->Debug = (CallDebug::Enum)(topFrame->Debug | CallDebug::Invoked);
    
    // Get the bound function we're going to run
    BoundFn boundFunction = function->BoundFunction;

    // If the function is not our own...
    if (boundFunction != VirtualMachine::ExecuteNext)
    {
      // We need to let any debugging/exceptions know that we're
      // entering a C++ bound function (for building the call stack)
      topFrame->ProgramCounter = ProgramCounterNative;
    }

    // Set the thread local calling state to the current state invoking
    // this function (so the function always knows the caller!)
    ExecutableState* lastCallingState = ExecutableState::CallingState;
    ExecutableState::CallingState = state;

    // Actually execute the function
    boundFunction(*this, report);

    // Get whether the return value was set
    bool returnWasSet = (this->Data->Debug & CallDebug::SetReturn) != 0;

    // If we threw any exceptions and the return value was not set, we will want to disable return destruction
    if (report.HasThrownExceptions() && returnWasSet == false)
      this->DisableReturnDestruction();
    
    // Reset the calling state back to the last one
    ExecutableState::CallingState = lastCallingState;

    // Get a reference to the core library
    Core& core = Core::GetInstance();

    // Make sure the return value was set (we can ignore this if an exception gets thrown)
    ErrorIf(!(this->Data->Debug & CallDebug::NoReturnChecking) &&
      (this->Data->Debug & CallDebug::SetReturn) == 0 &&
      !report.HasThrownExceptions() && 
      Type::IsSame(function->FunctionType->Return, core.VoidType) == false,
      "The return value was not set after a call (ignored when exceptions are thrown)");
    return report.HasThrownExceptions() == false;
  }

  //***************************************************************************
  void CallHelper<Any>::Set(Call& call, size_t index, const Any& value)
  {
    // Get the stack location and perform checks
    byte* stack = call.GetChecked(index, sizeof(Any), ZilchTypeId(Any), CheckPrimitive::Any, Direction::Set);
      
    // Write the value to the stack
    new (stack) Any(value);
  }

  //***************************************************************************
  void CallHelper<      Any*>::Set(Call& call, size_t index,       Any* const& value) { CallHelper<Any>::Set(call, index, *value); }
  void CallHelper<      Any&>::Set(Call& call, size_t index,       Any&        value) { CallHelper<Any>::Set(call, index,  value); }
  void CallHelper<const Any*>::Set(Call& call, size_t index, const Any* const& value) { CallHelper<Any>::Set(call, index, *value); }
  void CallHelper<const Any&>::Set(Call& call, size_t index, const Any&        value) { CallHelper<Any>::Set(call, index,  value); }
  
  //***************************************************************************
  Any* CallHelper<Any*>::Get(Call& call, size_t index)
  {
    // Get the stack location and perform checks
    byte* stack = call.GetChecked(index, sizeof(Any), ZilchTypeId(Any), CheckPrimitive::Any, Direction::Get);
      
    // Read the value from the stack and return it (or convert it)
    return (Any*)stack;
  }
  
  //***************************************************************************
        Any  CallHelper<      Any >::Get(Call& call, size_t index) { return *CallHelper<Any*>::Get(call, index); }
        Any& CallHelper<      Any&>::Get(Call& call, size_t index) { return *CallHelper<Any*>::Get(call, index); }
  const Any* CallHelper<const Any*>::Get(Call& call, size_t index) { return  CallHelper<Any*>::Get(call, index); }
  const Any& CallHelper<const Any&>::Get(Call& call, size_t index) { return *CallHelper<Any*>::Get(call, index); }
  
  //***************************************************************************
  byte* CallHelper<Any*>::GetArgumentPointer(Call& call, size_t index)
  {
    // Get the stack location and perform checks
    return call.GetChecked(index, sizeof(Any), ZilchTypeId(Any), CheckPrimitive::Any, Direction::Get);
  }
  
  //***************************************************************************
  byte* CallHelper<      Any >::GetArgumentPointer(Call& call, size_t index) { return CallHelper<Any*>::GetArgumentPointer(call, index); }
  byte* CallHelper<      Any&>::GetArgumentPointer(Call& call, size_t index) { return CallHelper<Any*>::GetArgumentPointer(call, index); }
  byte* CallHelper<const Any*>::GetArgumentPointer(Call& call, size_t index) { return CallHelper<Any*>::GetArgumentPointer(call, index); }
  byte* CallHelper<const Any&>::GetArgumentPointer(Call& call, size_t index) { return CallHelper<Any*>::GetArgumentPointer(call, index); }
  
  //***************************************************************************
  Any* CallHelper<Any*>::CastArgumentPointer(byte* stackPointer)
  {
    // Read the value from the stack and return it (or convert it)
    return (Any*)stackPointer;
  }
  
  //***************************************************************************
        Any  CallHelper<      Any >::CastArgumentPointer(byte* stackPointer) { return *CallHelper<Any*>::CastArgumentPointer(stackPointer); }
        Any& CallHelper<      Any&>::CastArgumentPointer(byte* stackPointer) { return *CallHelper<Any*>::CastArgumentPointer(stackPointer); }
  const Any* CallHelper<const Any*>::CastArgumentPointer(byte* stackPointer) { return  CallHelper<Any*>::CastArgumentPointer(stackPointer); }
  const Any& CallHelper<const Any&>::CastArgumentPointer(byte* stackPointer) { return *CallHelper<Any*>::CastArgumentPointer(stackPointer); }
  
  //***************************************************************************
  void CallHelper<Handle>::Set(Call& call, size_t index, const Handle& value)
  {
    call.SetHandle(index, value);
  }
  
  //***************************************************************************
  void CallHelper<      Handle*>::Set(Call& call, size_t index,       Handle* const& value) { CallHelper<Handle>::Set(call, index, *value); }
  void CallHelper<      Handle&>::Set(Call& call, size_t index,       Handle&        value) { CallHelper<Handle>::Set(call, index,  value); }
  void CallHelper<const Handle*>::Set(Call& call, size_t index, const Handle* const& value) { CallHelper<Handle>::Set(call, index, *value); }
  void CallHelper<const Handle&>::Set(Call& call, size_t index, const Handle&        value) { CallHelper<Handle>::Set(call, index,  value); }
  
  //***************************************************************************
  Handle* CallHelper<Handle*>::Get(Call& call, size_t index)
  {
    return &call.GetHandle(index);
  }
  
  //***************************************************************************
        Handle  CallHelper<      Handle >::Get(Call& call, size_t index) { return *CallHelper<Handle*>::Get(call, index); }
        Handle& CallHelper<      Handle&>::Get(Call& call, size_t index) { return *CallHelper<Handle*>::Get(call, index); }
  const Handle* CallHelper<const Handle*>::Get(Call& call, size_t index) { return  CallHelper<Handle*>::Get(call, index); }
  const Handle& CallHelper<const Handle&>::Get(Call& call, size_t index) { return *CallHelper<Handle*>::Get(call, index); }
  
  //***************************************************************************
  byte* CallHelper<Handle*>::GetArgumentPointer(Call& call, size_t index)
  {
    return call.GetHandlePointer(index);
  }
  
  //***************************************************************************
  byte* CallHelper<      Handle >::GetArgumentPointer(Call& call, size_t index) { return CallHelper<Handle*>::GetArgumentPointer(call, index); }
  byte* CallHelper<      Handle&>::GetArgumentPointer(Call& call, size_t index) { return CallHelper<Handle*>::GetArgumentPointer(call, index); }
  byte* CallHelper<const Handle*>::GetArgumentPointer(Call& call, size_t index) { return CallHelper<Handle*>::GetArgumentPointer(call, index); }
  byte* CallHelper<const Handle&>::GetArgumentPointer(Call& call, size_t index) { return CallHelper<Handle*>::GetArgumentPointer(call, index); }
  
  //***************************************************************************
  Handle* CallHelper<Handle*>::CastArgumentPointer(byte* stackPointer)
  {
    return (Handle*)stackPointer;
  }
  
  //***************************************************************************
        Handle  CallHelper<      Handle >::CastArgumentPointer(byte* stackPointer) { return *CallHelper<Handle*>::CastArgumentPointer(stackPointer); }
        Handle& CallHelper<      Handle&>::CastArgumentPointer(byte* stackPointer) { return *CallHelper<Handle*>::CastArgumentPointer(stackPointer); }
  const Handle* CallHelper<const Handle*>::CastArgumentPointer(byte* stackPointer) { return  CallHelper<Handle*>::CastArgumentPointer(stackPointer); }
  const Handle& CallHelper<const Handle&>::CastArgumentPointer(byte* stackPointer) { return *CallHelper<Handle*>::CastArgumentPointer(stackPointer); }
  
  //***************************************************************************
  void CallHelper<Delegate>::Set(Call& call, size_t index, const Delegate& value)
  {
    call.SetDelegate(index, value);
  }
  
  //***************************************************************************
  void CallHelper<      Delegate*>::Set(Call& call, size_t index,       Delegate* const& value) { CallHelper<Delegate>::Set(call, index, *value); }
  void CallHelper<      Delegate&>::Set(Call& call, size_t index,       Delegate&        value) { CallHelper<Delegate>::Set(call, index,  value); }
  void CallHelper<const Delegate*>::Set(Call& call, size_t index, const Delegate* const& value) { CallHelper<Delegate>::Set(call, index, *value); }
  void CallHelper<const Delegate&>::Set(Call& call, size_t index, const Delegate&        value) { CallHelper<Delegate>::Set(call, index,  value); }
  
  //***************************************************************************
  Delegate* CallHelper<Delegate*>::Get(Call& call, size_t index)
  {
    return &call.GetDelegate(index);
  }
  
  //***************************************************************************
        Delegate  CallHelper<      Delegate >::Get(Call& call, size_t index) { return *CallHelper<Delegate*>::Get(call, index); }
        Delegate& CallHelper<      Delegate&>::Get(Call& call, size_t index) { return *CallHelper<Delegate*>::Get(call, index); }
  const Delegate* CallHelper<const Delegate*>::Get(Call& call, size_t index) { return  CallHelper<Delegate*>::Get(call, index); }
  const Delegate& CallHelper<const Delegate&>::Get(Call& call, size_t index) { return *CallHelper<Delegate*>::Get(call, index); }

  //***************************************************************************
  byte* CallHelper<Delegate*>::GetArgumentPointer(Call& call, size_t index)
  {
    return call.GetDelegatePointer(index);
  }
  
  //***************************************************************************
  byte* CallHelper<      Delegate >::GetArgumentPointer(Call& call, size_t index) { return CallHelper<Delegate*>::GetArgumentPointer(call, index); }
  byte* CallHelper<      Delegate&>::GetArgumentPointer(Call& call, size_t index) { return CallHelper<Delegate*>::GetArgumentPointer(call, index); }
  byte* CallHelper<const Delegate*>::GetArgumentPointer(Call& call, size_t index) { return CallHelper<Delegate*>::GetArgumentPointer(call, index); }
  byte* CallHelper<const Delegate&>::GetArgumentPointer(Call& call, size_t index) { return CallHelper<Delegate*>::GetArgumentPointer(call, index); }
  
  //***************************************************************************
  Delegate* CallHelper<Delegate*>::CastArgumentPointer(byte* stackPointer)
  {
    return (Delegate*)stackPointer;
  }
  
  //***************************************************************************
        Delegate  CallHelper<      Delegate >::CastArgumentPointer(byte* stackPointer) { return *CallHelper<Delegate*>::CastArgumentPointer(stackPointer); }
        Delegate& CallHelper<      Delegate&>::CastArgumentPointer(byte* stackPointer) { return *CallHelper<Delegate*>::CastArgumentPointer(stackPointer); }
  const Delegate* CallHelper<const Delegate*>::CastArgumentPointer(byte* stackPointer) { return  CallHelper<Delegate*>::CastArgumentPointer(stackPointer); }
  const Delegate& CallHelper<const Delegate&>::CastArgumentPointer(byte* stackPointer) { return *CallHelper<Delegate*>::CastArgumentPointer(stackPointer); }  
}
