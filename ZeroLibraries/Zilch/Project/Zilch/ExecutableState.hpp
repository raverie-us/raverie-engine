/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_EXECUTABLE_STATE_HPP
#define ZILCH_EXECUTABLE_STATE_HPP

namespace Zilch
{
  namespace Events
  {
    // Sent on ExecutableState when a fatal error occurs, such as
    // out of memory, stack overflow inside another overflow, etc
    // The program will be immediately aborted after this event is run
    ZilchDeclareEvent(FatalError, FatalErrorEvent);

    // Sent on ExecutableState when an exception occurs and isn't handled (can be user thrown, etc)
    ZilchDeclareEvent(UnhandledException, ExceptionEvent);

    // Sent on ExecutableState when an exception occurs and is caught by the user
    ZilchDeclareEvent(HandledException, ExceptionEvent);

    // Debugger and profiler events
    ZilchDeclareEvent(OpcodePreStep, OpcodeEvent);
    ZilchDeclareEvent(OpcodePostStep, OpcodeEvent);
    ZilchDeclareEvent(EnterFunction, OpcodeEvent);
    ZilchDeclareEvent(ExitFunction, OpcodeEvent);

    // Whenever an scripted object is leaked, it is reported when the executable state is torn down
    ZilchDeclareEvent(MemoryLeak, MemoryLeakEvent);
  }

  // Used when an object leaks when an executable state is torn down
  // Members may be null depending on the type of leak (always check for null)
  class ZeroShared MemoryLeakEvent : public EventData
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    MemoryLeakEvent();
    ExecutableState* State;
    Handle* LeakedObject;
    CodeLocation* AllocatedLocation;
  };

  // An event intended for debuggers and profilers (used when we enter/exit functions and step opcodes)
  class ZeroShared OpcodeEvent : public EventData
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    OpcodeEvent();
    ExecutableState* State;
    Function* CurrentFunction;
    size_t ProgramCounter;
    size_t StackOffset;
    CodeLocation* Location;
  };

  // An event sent out when an exception occurs
  class ZeroShared ExceptionEvent : public EventData
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);
    ExceptionEvent();
    ExecutableState* State;
    Exception* ThrownException;
  };
  
  // The types of fatal errors that can occur inside of Zilch (non recoverable errors)
  namespace FatalError
  {
    enum Enum
    {
      Invalid,
      OutOfMemory,
      StackReserveOverflow
    };
  }
  
  // The event sent out when a fatal error occurs
  class ZeroShared FatalErrorEvent : public EventData
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    FatalErrorEvent();
    FatalError::Enum ErrorCode; 
  };

  // Two constants we use to note the state of our debug program counter pointer
  const size_t ProgramCounterNotActive = (size_t)-1;
  const size_t ProgramCounterNative = (size_t)-2;

  // The data that exists per scope (per stack frame)
  // Every function has an implicit scope
  class ZeroShared PerScopeData
  {
  public:
    // Invoke the destructors of anything we need to cleanup
    // and clear out the arrays for recycling
    void PerformCleanup();

    // Handles on the stack that need to be cleaned up
    // This includes 'this' handles inside delegates, any handles that get copied to the stack,
    // and or any handles that are created (such as via local/new, or stack/member handles)
    Array<Any*> AnysToBeCleaned;
    Array<Handle*> HandlesToBeCleaned;
    Array<Delegate*> DelegatesToBeCleaned;

    // A special unique id we use to specify keep track of stack handles
    Uid UniqueId;
  };

  // Debug flags that we use for making and receiving calls
  namespace CallDebug
  {
    enum Enum
    {
      None                      = 0,
      SetParameter0             = 1 << 0,
      SetParameter1             = 1 << 1,
      SetParameter2             = 1 << 2,
      SetParameter3             = 1 << 3,
      SetParameter4             = 1 << 4,
      SetParameter5             = 1 << 5,
      SetParameter6             = 1 << 6,
      SetParameter7             = 1 << 7,
      SetParameter8             = 1 << 8,
      SetParameter9             = 1 << 9,
      SetParameter10            = 1 << 10,
      SetParameter11            = 1 << 11,
      SetParameter12            = 1 << 12,
      SetParameter13            = 1 << 13,
      SetParameter14            = 1 << 14,
      SetParameter15            = 1 << 15,
      SetParameter16            = 1 << 16,
      SetParameter17            = 1 << 17,
      SetParameter18            = 1 << 18,
      SetParameter19            = 1 << 19,
      SetParameter20            = 1 << 20,
      SetParameter21            = 1 << 21,
      SetReturn                 = 1 << 22,
      SetThis                   = 1 << 23,
      Invoked                   = 1 << 24,
      NoReturnChecking          = 1 << 25,
      NoReturnDestruction       = 1 << 26,
      NoParameterChecking       = 1 << 27,
      NoParameterDestruction    = 1 << 28,
      NoThisChecking            = 1 << 29,
      NoThisDestruction         = 1 << 30
    };
  }

  // The state of a stack frame
  namespace StackErrorState
  {
    enum Enum
    {
      Normal,
      Overflowed,
      MaxRecursionReached
    };
  }

  // The data that exists per stack frame
  class ZeroShared PerFrameData
  {
  public:
    // Constructor
    PerFrameData(ExecutableState* state);

    // Adds an 'any type' that needs cleanup
    void QueueAnyCleanup(Any* any);

    // Adds a handle that needs cleanup
    void QueueHandleCleanup(Handle* handle);

    // Adds a delegate that needs cleanup
    void QueueDelegateCleanup(Delegate* delegate);

    // If the stack overflowed or we reached the max recursion depth,
    // this will throw an exception and return true
    // Otherwise this will return false if no exception was thrown
    bool AttemptThrowStackExceptions(ExceptionReport& report);

    // Checks if a variable is initialized within the current frame
    // Note: This is only intended to be used by a debugger, and may be incorrect for value types
    bool IsVariableInitialized(Variable* variable);

    // For the current stack frame, this is where all the data lies
    byte* Frame;
    byte* NextFrame;

    // We need a pointer back to the state to do certain operations
    ExecutableState* State;

    // The active program counter for this call stack
    // By default, this is set to 'ProgramCounterNotActive' which indicates it should not be used
    // For C++ bound functions, this is 'ProgramCounterNative' which means it is active but more information is needed
    size_t ProgramCounter;

    // The current function that is being executed
    Function* CurrentFunction;

    // Any per scope data (this data is generally 'things to be destructed')
    // This is also used to destruct things when exceptions are thrown
    Array<PerScopeData*> Scopes;

    // Used for debugging (we can't invoke a function until all parameters are written)
    CallDebug::Enum Debug;

    // Where we report exceptions to
    // This is only set when actually entering the call, not when the frame is created
    ExceptionReport* Report;

    // When we're inside the VM, this is the location that we jump to if an exception occurs
    // This is only set when actually inside the VM's 'ExecuteNext' function
    jmp_buf ExceptionJump;

    // The number of timeouts we have associated with this stack frame
    // When this frame gets destroyed/unrolled, we need to pop these timeouts
    size_t Timeouts;

    // The frame itself could have been created past the recursion depth or in an overflowed state
    StackErrorState::Enum ErrorState;
  };

  namespace CheckPrimitive
  {
    enum Enum
    {
      Handle,
      Value,
      Delegate,
      Any
    };
  }

  namespace Direction
  {
    enum Enum
    {
      Get,
      Set
    };
  }

  // Describes where an operand is pointing to =
  // (a location on the stack, or an object, etc)
  class ZeroShared OperandLocation
  {
  public:
    // Constructor
    OperandLocation();

    // The type will either be set to Member, Constant, or Local
    OperandType::Enum Type;

    // If the operand is a Member, then this is a pointer to the base of
    // the object where the member exists. If the operand is a Constant,
    // this points at the base of the constant buffer for the current function.
    // If the operand is a Local, then this is a pointer to the stack.
    const char* Memory;

    // The size of the memory. In the case of the Member, this is the size
    // of the object. In the case of the Constant, this is the size of the
    // constant buffer. In the case of the Local, this is the size of the stack
    size_t MemorySize;

    // The offset into the memory where the Member, Constant, or Local exists
    size_t Offset;
  };

  // Tells us when a function or timeout scope started, and how
  // long it has until it exceeds its time and throws an exception
  class ZeroShared Timeout
  {
  public:
    // Constructor
    Timeout();

    // The length of the timeout in ticks (once the accumulation reaches this we throw an exception)
    // We also use this to print a meaningful exception message
    long long LengthTicks;

    // Stores the amount of ticks accumulated under this timeout (based on the Timer class)
    // Every timeout is exclusive, which means while a timeout is active it's parents DO NOT accumulate time
    long long AccumulatedTicks;

#if ZeroDebug
    // Only used to verify that the correct frame that pushed us is the only one to pop us
    PerFrameData* Frame;
#endif
  };

  // With any function call, an exception can occur that we need to catch
  class ZeroShared ExceptionReport
  {
  public:
    // Friends
    friend class ExecutableState;

    // Constructor
    ExceptionReport();

    // Clear the exception report (will allow code to continue with execution)
    void Clear();

    // Tests if any exceptions were thrown
    bool HasThrownExceptions();

    // Gets all the exceptions concatenated in a list
    String GetConcatenatedMessages();
    
    // The exceptions that are being thrown (as we unravel the stack)
    Array<Handle> Exceptions;

    // In the event that an exception could not be allocated (or other fringe reasons)
    // we must set this flag (which forces 'HasThrownExceptions' to return true, even when Exceptions is empty)
    bool ForceThrownExceptions;

  private:
    // An array of exception pointers, only for debugging purposes
    Array<Exception*> ExceptionsForDebugOnly;
  };

  // A callback that prints to stderr whenever an exception occurs
  ZeroShared void DefaultExceptionCallback(ExceptionEvent* e);

  // Stores the generated functions, and anything else needed for a VM to execute
  class ZeroShared ExecutableState : public EventHandler
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Friends
    friend class VirtualMachine;
    friend class Module;
    friend class Handle;
    friend class CodeGenerator;
    friend class HeapManager;
    friend class StackManager;
    friend class Call;
    friend class PerFrameData;
    friend class Debugger;

    // Because users often need to access the state in their own bound functions, we provide a thread local
    // that is the last running state (set before each call to Zilch, and reset to the previous after the call)
    static ExecutableState* CallingState;
    static ExecutableState* GetCallingState();

    // Constructor
    ExecutableState();

    // Destructor
    ~ExecutableState();
    
    // Patch a library that already exists on the same state (by name)
    // The executable state must not be running inside any code (no stack frame) and currently
    // the library must have no other libraries that depend upon it (a leaf library)
    void PatchLibrary(LibraryRef newLibrary);

    // Set the timeout for this state in seconds (equivalent of a timeout statement in language)
    // Any code that runs for longer than this amount of time will throw an exception saying
    // that it timed out. This is mainly used to prevent user code from infinite looping/recursing
    // This can only be set while we're not inside a call-stack
    // A value of 0 seconds will clear the timeout
    void SetTimeout(size_t seconds);

    // Checks if we are currently inside a call stack
    bool IsInCallStack();

    // Allocates an object on the stack at the given stack
    // position and returns a stack handle to the object
    // Note that the handle will become null when we leave the scope
    // that the object was allocated in (return from a function, etc)
    Handle AllocateStackObject(byte* stackLocation, PerScopeData* scope, BoundType* type, ExceptionReport& report);

    // Allocates an object on the heap and returns a handle to the object
    // No constructors are called, but the object will be pre-constructed
    // Note that the memory will be managed by the language itself
    Handle AllocateHeapObject(BoundType* type, ExceptionReport& report, HeapFlags::Enum flags);

    // Allocates an object on the heap and returns a handle to the object
    // Both the pre-constructor and default constructor will be called (or an exception will be thrown)
    // Note that the memory will be managed by the language itself
    Handle AllocateDefaultConstructedHeapObject(BoundType* type, ExceptionReport& report, HeapFlags::Enum flags);
    Handle AllocateDefaultConstructedHeapObject(BoundType* type, HeapFlags::Enum flags = HeapFlags::ReferenceCounted);

    // Allocates an object on the heap and returns a handle to the object
    // The object will be pre-constructed and then the copy constructor will be invoked on it
    // Note that the memory will be managed by the language itself
    Handle AllocateCopyConstructedHeapObject(BoundType* type, ExceptionReport& report, HeapFlags::Enum flags, const Handle& fromObject);

    template <typename T>
    HandleOf<T> AllocateDefaultConstructed(BoundType* typeToAllocate = nullptr, HeapFlags::Enum flags = HeapFlags::ReferenceCounted)
    {
      // If the user provided no specific type to allocate, then assume its the T type
      if (typeToAllocate == nullptr)
        typeToAllocate = ZilchTypeId(T);

      // Allocate the heap object default constructed based on the specified type
      ExceptionReport report;
      Handle resultHandle = this->AllocateDefaultConstructedHeapObject(typeToAllocate, report, flags);
      return resultHandle;
    }

    template <typename T>
    void Delete(T* instance)
    {
      if (instance == nullptr)
        return;

      Handle handle(this, ZilchTypeId(T), (const byte*)instance);
      handle.Delete();
    }

    // Get a pointer to the current stack for the current function
    // If the function has a return value, then it is the first value on the stack
    // After the return value comes the parameters, in tightly packed order
    // If the function is an instance method, then the last value is the 'this' handle,
    // whose type is actually 'Handle'; handles can be dereferenced to get a
    // direct pointer to the object
    // Only ever read up to the size of your return, parameters, and this handle
    byte* GetCurrentStackFrame();

    // Executes a statement or expression and returns the result
    // The dependent libraries are assumed to be the same libraries as the state compiled with (not including patches)
    // If the statement fails to compile, it will return a string that is the compilation error
    // If the statement results in an exception, this will return the a string with all the exceptions concatenated
    // This uses the 'patch library' logic to Append a patch to the running state
    Any ExecuteStatement(StringParam code);

    // Throws a standard null reference exception
    void ThrowNullReferenceException(ExceptionReport& report);

    // Throws a standard null reference exception with a custom message
    void ThrowNullReferenceException(ExceptionReport& report, StringParam customMessage);

    // If a function is not implemented, this is a standard exception that lets the user know
    void ThrowNotImplementedException();

    // Throws a standard exception with the given message using the ExceptionReport from the latest stack frame
    void ThrowException(StringParam message);

    // Throws a standard exception with the given message
    void ThrowException(ExceptionReport& report, StringParam message);

    // Throws an exception allocated by the user
    void ThrowException(ExceptionReport& report, Handle& exception);

    // Ensures that a scope does not exceed a time limit (throws an exception if it does)
    // This should be periodically called in native C++ code for proper timeout protection
    // Returns true if we threw a timeout exception, false otherwise
    bool ThrowExceptionOnTimeout(ExceptionReport& report);

    // Gets the latest exception report via the thread local 'CallingState'
    static ExceptionReport& GetCallingReport();

    // Build a stack trace into the stack array
    void GetStackTrace(StackTrace& trace);

    // Gets a handle manager by type
    template <typename T>
    T* GetHandleManager()
    {
      return (T*)HandleManagers::GetInstance().GetManager(ZilchManagerId(T), this);
    }

    // Update the virtual table of a native C++ object, or do nothing if it's not virtual or native
    void UpdateCppVirtualTable(byte* objectWithBaseVTable, BoundType* cppBaseType, BoundType* derivedType);

    // Get the raw stack array
    const byte* GetRawStack();

    // Get a static field from this state (must be a field in one of the dependent libraries)
    // If the field is not initialized, it will run the initializer code (user defined code)
    // to attempt to initialize the value, though it is first initialized to zero memory
    // An exception can be thrown from user code when attempting to initialize the field
    byte* GetStaticField(Field* field, ExceptionReport& report);

    // A pointer to any data the user wants to attach
    mutable const void* UserData;

    // Any user data that cant simply be represented by a pointer
    // Data can be written to the buffer and will be properly destructed
    // when this object is destroyed (must be read in the order it's written)
    mutable DestructibleBuffer ComplexUserData;

  private:

    // Applies a patch, but skips some checks (used when we know patching a library is safe)
    void ForcePatchLibrary(LibraryRef newLibrary);

    // Gets a pointer to the next stack frame
    // If we're currently in a function, this represents the
    // position ahead of our current function. If we're not
    // in any functions, it should be the front of the stack
    // Always call this BEFORE you push any frame data
    byte* GetNextStackFrame();

    // Push a new stack frame and returns the location on the stack
    ZilchForceInline PerFrameData* PushFrame(Function* function);

    // A slightly more optimal version of pushing a stack frame (used internally in execution)
    ZilchForceInline PerFrameData* PushFrame(byte* frame, Function* function);

    // Pops a stack frame and return a pointer to where the return value should be
    ZilchForceInline PerFrameData* PopFrame();

    // Initialize a handle to point at a location on the stack
    void InitializeStackHandle(Handle& handle, byte* location, PerScopeData* scope, BoundType* type);

    // Initialize a handle with a direct pointer value
    // Generally unsafe, but used in cases such as statics which are guaranteed to exist and therefore safe
    void InitializePointerHandle(Handle& handle, byte* location, BoundType* type);

    // Invokes the pre-constructor (which initializes memory) on a handle
    void InvokePreConstructorOrRelease(Handle& handle, ExceptionReport& report);

    // Allocates or recycles a scope
    PerScopeData* AllocateScope();

    // A timeout is a low level construct that allows us to ensure code does not run beyond a certain time
    // Timeouts do not work while calling native code (except upon native code's return)
    // This will push a timeout based on a given number of seconds
    // Returns true if we threw a timeout exception, false otherwise
    bool PushTimeout(PerFrameData* frame, size_t seconds);

    // Exits a timeout scope and validates that the timeout was not reached
    // Returns true if we threw a timeout exception, false otherwise
    bool PopTimeout(PerFrameData* frame);

    // Send an opcode event (generally used for debuggers or profilers)
    ZilchForceInline void SendOpcodeEvent(StringParam eventId, PerFrameData* frame);

  public:

    // Enables debug events (opcode step, enter/exit function, etc)
    bool EnableDebugEvents;
    
    // Maps old functions to the new functions they were patched with (only if any library was patched in the state)
    HashMap<Function*, Function*> PatchedFunctions;
    
    // Maps old types to the new types they were patched with (only if any library was patched in the state)
    HashMap<BoundType*, BoundType*> PatchedBoundTypes;

    // Libraries that we need to keep alive because we were patched using their types and functions
    //HashMap<LibraryRef, LibraryRef> PatchedLibraries;
    Array<LibraryRef> PatchedLibraries;

    // An id that is guaranteed to start at 0 and counts up every time we are patched
    // This can be used to re-enable features after a patch occurs on the state (such as event handlers)
    size_t PatchId;

    // Externally set breakpoints will overwrite the instruction, so we remap the opcode's index
    // to its original instruction here (if a breakpoint gets unset, we use this to write back the original instruction)
    HashMap<size_t, Instruction::Enum> ExternalBreakpoints;

    // Static variables are currently just looked up by their pointer
    // If the static field memory does not exist, it will be created and zeroed out
    // In the future, we'll also run the initializer upon the memory the first time it is accessed
    // We may reserve a header byte to know whether the memory has been fully initialized,
    // because accessing a field that has been created but not fully initialized means there is a cycle
    HashMap<Field*, byte*> StaticFieldToMemory;

    // We need a way to map virtual function ids into our function
    HashMap<GuidType, Function*> ThunksToFunctions;

    // The size of the stack
    const size_t StackSize;

    // Reserved space after the stack, this is only used when we reach a stack overflow
    const size_t OverflowStackSize;

    // If this reference count is greater than 0, then we do not allow allocation
    // This is true when running any destructors (destructors must not allocate objects, or call any functions that allocate)
    size_t DoNotAllowAllocation;

    // The maximum number of recursive calls we allow in this state
    size_t MaxRecursionDepth;

    // Used for debugging (the name will show up in the debugger, and so on)
    String Name;

    // We want to hold references to the libraries that we were compiled with
    Module Dependencies;

    // Pointers to our global handle managers
    HeapManager* HeapObjects;
    StackManager* StackObjects;
    PointerManager* PointerObjects;

    // The current type we are allocating (set by AllocateHeapObject)
    // This is useful for when we need to access the 'virtual type' from a base class of a constructing object
    BoundType* AllocatingType;

    // All the virtual tables (of varying sizes) for each native type that has virtual methods bound
    HashMap<BoundType*, byte*> NativeVirtualTables;

    // The handle managers we use to dereference and setup handles
    mutable HashMap<HandleManagerId, HandleManager*> UniqueManagers;

    // Map code entry ids to the code entry itself (the entry will be alive as long as the library is alive)
    // This maintains a list of all code entries used by this state
    HashMap<size_t, CodeEntry*> CodeHashToCodeEntry;

    // Data that we need each time we jump into a new stack frame
    Array<PerFrameData*> StackFrames;
    
    // Every time we allocate a stack frame, we don't actually delete it, but rather put it into this free list of stack frames
    // These are destroyed along with the executable state
    OwnedArray<PerFrameData*> RecycledFrames;

    // Recycled for memory efficiency (see RecycledFrames)
    OwnedArray<PerScopeData*> RecycledScopes;
    
    // Every time we allocate a scope (even for saved versions) we give it a unique
    // id so that way references formed to stack variables can be correctly used
    // It is very important that the scopes be recycled, because handles continue to point at old scopes
    // We use 0 as a special value that means a scope is not valid (in the recycle list) so the counter starts at 1
    Uid UniqueIdScopeCounter;

    // This stack of string builders that we use for efficient concatenation of strings
    Array<StringBuilder> StringBuilders;

    // In the case where we have no globally set timeout, and we have no timeout statements,
    // this array will be empty. Otherwise if a timeout exists it will be checked by the
    // virtual machine
    // Note: Timeouts are closely related to stack frames, as they can only be pushed
    // or popped by the same stack frame
    Array<Timeout> Timeouts;

    // The timer we use to measure timeouts
    // Ideally this timer would have a high fidelity to prevent timing inaccuracies
    Timer TimeoutTimer;

    // The amount of time we require when executing a timeout
    // A value of 0 means that we don't perform any timeouts
    // This value is only used when we enter the first function on the call stack
    size_t TimeoutSeconds;

    // The stack data used by the executable state
    // This is the base of the stack, NOT the current stack
    // Note that the stack is of a fixed size, and should never be reallocated
    byte* Stack;

    // Once we hit a stack overflow we no longer have stack space to invoke anything
    // such as destructors, or constructing the exception itself! To fix this issue,
    // we use an extra reserve of space at the end of the stack, however it is only
    // valid to access that stack space when this flag is set
    // If a destructor then stack overflows while another stack overflow occurred,
    // we hit a fatal error
    bool HitStackOverflow;

    // When no exception reports exist on the call stack, this one will be returned
    // This report is always cleared upon its request
    ExceptionReport DefaultReport;

    // Not copyable
    ZilchNoCopy(ExecutableState);
  };
  
  // Grab the next value from the stack frame
  // This will push the stack frame pointer forward
  // You can get the current stack frame by calling 'GetCurrentStackFrame'
  // See 'GetCurrentStackFrame' for the calling conventions
  template <typename T>
  ZeroSharedTemplate T InternalReadValue(byte* stackFrame)
  {
    // Return the read in value and advance the stack forward by the value's size
    typedef typename TypeBinding::StaticTypeId<T>::ReadType ReadType;
    ReadType readValue = TypeBinding::StaticTypeId<T>::Read(stackFrame);
    return TypeBinding::ReferenceCast<ReadType, T>::Cast(readValue);
  }

  // Pushes a value onto the stack frame
  // This will push the stack frame pointer forward
  // You can get the current stack frame by calling 'GetCurrentStackFrame'
  // See 'GetCurrentStackFrame' for the calling conventions
  template <typename T>
  ZeroSharedTemplate void InternalWriteValue(const T& value, byte* stackFrame)
  {
    // Write the value directly to the stack frame
    typedef typename TypeBinding::StaticTypeId<T>::UnqualifiedType& ToType;
    typedef typename TypeBinding::StripConst<T>::Type& FromType;
    ZilchStaticType(T)::Write(TypeBinding::ReferenceCast<FromType, ToType>::Cast((FromType)value), stackFrame);
  }

  // Grab the next reference type from the stack frame
  // This will push the stack frame pointer forward
  // You can get the current stack frame by calling 'GetCurrentStackFrame'
  // See 'GetCurrentStackFrame' for the calling conventions
  template <typename T>
  ZeroSharedTemplate T InternalReadRef(byte* stackFrame)
  {
    // Read the handle that will point at the string from the stack
    Handle& handle = *(Handle*)stackFrame;

    // Read the data from the handle by dereferencing it
    byte* data = handle.Dereference();

    // Read the value from the handle data
    return InternalReadValue<T>(data);
  }

  template <typename T>
  ZeroSharedTemplate void InternalWriteRef(const T& value, byte* stackFrame, ExecutableState* state)
  {
    // Get the type we're trying to write
    ZilchStrip(T)* pointerToValue = ZilchToPointer(value);
    BoundType* type = ZilchVirtualTypeId(pointerToValue);

    if (type->IsInitializedAssert() == false)
    {
      new (stackFrame) Handle();
      return;
    }

    // Grab the handle manager via the state
    HandleManager* manager = HandleManagers::GetInstance().GetManager(type->HandleManager, state);

    // Create a handle that goes with the given manager index
    Handle* handle = new (stackFrame) Handle();
    handle->Manager = manager;

    // If this is a redirected type...
    if (ZilchStaticType(T)::DirectRead)
    {
      // Get a raw pointer to the value (removes all const, reference, and other qualifiers)
      handle->StoredType = type;

      // Setup the newly created handle
      manager->ObjectToHandle((const byte*)pointerToValue, handle->StoredType, *handle);
    }
    else
    {
      // Write the value to a temporary buffer
      size_t size = sizeof(typename ZilchStaticType(T)::RepresentedType);
      byte* data = (byte*)alloca(size);

      // This was a redirect, so just take the type of the redirect and put it on the handle
      handle->StoredType = type;

      // Convert the value to the redirected type within our temporary buffer
      InternalWriteValue<T>(value, data);

      // Setup the newly created handle
      manager->ObjectToHandle(data, type, *handle);

      // Invoke the destructor on the temporary memory
      typedef typename ZilchStaticType(T)::RepresentedType RepresentedType;
      ((RepresentedType*)data)->~RepresentedType();
    }
  }

  template <typename T>
  ZeroSharedTemplate class CallHelper
  {
  public:
    static T Get(Call& call, size_t index)
    {
      // If the type is a reference type... (this is always a handle)
      if (ZilchTypeId(T)->CopyMode == TypeCopyMode::ReferenceType || index == Call::This)
      {
        return call.GetHandle<T>(index);
      }
      // Otherwise it must be a value type...
      else
      {
        return call.GetValue<T>(index);
      }
    }

    static void Set(Call& call, size_t index, const T& value)
    {
      // If the type is a reference type... (this is always a handle)
      if (ZilchTypeId(T)->CopyMode == TypeCopyMode::ReferenceType || index == Call::This)
      {
        call.SetHandle<T>(index, value);
      }
      // Otherwise it must be a value type...
      else
      {
        call.SetValue<T>(index, value);
      }
    }

    static byte* GetArgumentPointer(Call& call, size_t index)
    {
      if (ZilchTypeId(T)->CopyMode == TypeCopyMode::ReferenceType || index == Call::This)
      {
        // Read the handle from the stack
        Handle& handle = *(Handle*)call.GetHandlePointer<T>(index);

        // Read the data from the handle by dereferencing it
        byte* stackPointer = handle.Dereference();

        // Throw exception if there's null for a value or reference type
        if (stackPointer == nullptr && !Zero::is_pointer<T>::value)
          ExecutableState::GetCallingState()->ThrowException(String::Format("Error: Argument %d cannot be null.", index));

        return stackPointer;
      }
      else
      {
        return call.GetValuePointer<T>(index);
      }
    }

    static T CastArgumentPointer(byte* stackPointer)
    {
      return InternalReadValue<T>(stackPointer);
    }
  };

  #define ZilchCallHelperSpecialization(T, SetT)                              \
    class ZeroShared CallHelper<T>                                            \
    {                                                                         \
    public:                                                                   \
      static T Get(Call& call, size_t index);                                 \
      static void Set(Call& call, size_t index, SetT value);                  \
      static byte* GetArgumentPointer(Call& call, size_t index);              \
      static T CastArgumentPointer(byte* stackPointer);                       \
    };

  #define ZilchCallHelperTemplateSpecialization(T, SetT)                      \
    ZeroSharedTemplate class CallHelper<T>                                    \
    {                                                                         \
    public:                                                                   \
      static T Get(Call& call, size_t index);                                 \
      static void Set(Call& call, size_t index, SetT value);                  \
      static byte* GetArgumentPointer(Call& call, size_t index);              \
      static T CastArgumentPointer(byte* stackPointer);                       \
    };

  // Facilitates invoking Zilch functions including parameter passing and grabbing return values
  // Also is passed into each call when implementing a custom function that is bound to Zilch
  class ZeroShared Call
  {
  public:
    friend class ExecutableState;
    friend class VirtualMachine;

    // Constructor for calling a function
    Call(Function* function, ExecutableState* state = ExecutableState::CallingState);

    // Constructor for calling a delegate (automatically sets the this handle)
    Call(const Delegate& delegate, ExecutableState* state = ExecutableState::CallingState);

    // Destructor (constructor is private so only the ExecutableState can create it)
    ~Call();

    // All getters and setters below perform checks on the size and type (if possible)
    // These are special constants that represent the 'this' handle and return value
    static const size_t Return  = (size_t)-1;
    static const size_t This    = (size_t)-2;

    // Set either a parameter or return for the call (value types only, this not allowed)
    void SetValue(size_t index, const byte* input, size_t size);

    // Set either a parameter, return, or this handle for the call
    void SetHandle(size_t index, const Handle& value);

    // Set either a parameter or return for the call (this not allowed)
    void SetDelegate(size_t index, const Delegate& value);
    
    // Set either a parameter or return for the call (value types only, this not allowed)
    template <typename T>
    void SetValue(size_t index, const T& value)
    {
      typedef typename TypeBinding::StripQualifiers<T>::Type UnqualifiedType;
      const UnqualifiedType* pointer = TypeBinding::ReferenceCast<T&, const UnqualifiedType*>::Cast((T&)value);
      BoundType* valueType = ZilchVirtualTypeId(pointer);

      // Get the stack location and perform checks
      byte* stack = this->GetChecked(index, sizeof(typename ZilchStaticType(T)::RepresentedType), valueType, CheckPrimitive::Value, Direction::Set);

      // Finally, copy the input into the stack position
      InternalWriteValue<T>(value, stack);
    }
    
    // Set either a parameter, return, or this handle for the call (reference types only)
    template <typename T>
    void SetHandle(size_t index, const T& value)
    {
      typedef typename TypeBinding::StripQualifiers<T>::Type UnqualifiedType;
      const UnqualifiedType* pointer = TypeBinding::ReferenceCast<T&, const UnqualifiedType*>::Cast((T&)value);
      BoundType* valueType = ZilchVirtualTypeId(pointer);

      // Get the stack location and perform checks
      byte* stack = this->GetChecked(index, sizeof(Handle), valueType, CheckPrimitive::Handle, Direction::Set);

      // Finally, copy the input into the stack position
      InternalWriteRef<T>(value, stack, this->Data->State);
    }

    // Set either a parameter, return, or this handle for the call
    // This method auto determines whether it's reference or value type
    template <typename T>
    void Set(size_t index, const T& value)
    {
      return CallHelper<T>::Set(*this, index, value);
    }

    // Get either a parameter or return from the call (value types only, this not allowed)
    void GetValue(size_t index, byte* output, size_t size);
    
    // Get either a parameter, return, or this handle from the call
    Handle& GetHandle(size_t index);
    
    // Get either a parameter, return, or this handle from the call
    byte* GetHandlePointer(size_t index);
    
    // Get either a parameter or return from the call (this not allowed)
    Delegate& GetDelegate(size_t index);

    // Get either a parameter or return from the call (this not allowed)
    byte* GetDelegatePointer(size_t index);

    // Get either a parameter or return from the call (value types only, this not allowed)
    template <typename T>
    T GetValue(size_t index)
    {
      // Get the stack location and perform checks
      byte* stack = this->GetChecked(index, sizeof(typename ZilchStaticType(T)::RepresentedType), ZilchTypeId(T), CheckPrimitive::Value, Direction::Get);
      
      // Read the value from the stack and return it (or convert it)
      return InternalReadValue<T>(stack);
    }
    
    // Get either a parameter, return, or this handle from the call (reference types only)
    template <typename T>
    T GetHandle(size_t index)
    {
      // Get the stack location and perform checks
      byte* stack = this->GetChecked(index, sizeof(Handle), ZilchTypeId(T), CheckPrimitive::Handle, Direction::Get);
      
      // Read the value from the stack and return it (or convert it)
      return InternalReadRef<T>(stack);
    }

    // Get either a parameter or return from the call (value types only, this not allowed)
    template <typename T>
    byte* GetValuePointer(size_t index)
    {
      // Get the stack location and perform checks
      return this->GetChecked(index, sizeof(typename ZilchStaticType(T)::RepresentedType), ZilchTypeId(T), CheckPrimitive::Value, Direction::Get);
    }
    
    // Get either a parameter, return, or this handle from the call (reference types only)
    template <typename T>
    byte* GetHandlePointer(size_t index)
    {
      // Get the stack location and perform checks
      return this->GetChecked(index, sizeof(Handle), ZilchTypeId(T), CheckPrimitive::Handle, Direction::Get);
    }

    // Get either a parameter, return, or this handle from the call
    // This method auto determines whether it's reference or value type
    // Does not check for a null value cast
    template <typename T>
    T Get(size_t index)
    {
      return CallHelper<T>::Get(*this, index);
    }

    // Same as Get but without the type cast
    // Throws exception if casting the data from the stack to the given type will result in a null value cast
    template <typename T>
    byte* GetArgumentPointer(size_t index)
    {
      return CallHelper<T>::GetArgumentPointer(*this, index);
    }

    // Does the type cast for the pointer returned by GetArgumentPointer, given type must be the same
    template <typename T>
    T CastArgumentPointer(byte* stackPointer)
    {
      return CallHelper<T>::CastArgumentPointer(stackPointer);
    }
    
    // Invoke the function / call
    // All parameters must be set before invoking (and the 'this' if it's an instance method)
    // Returns true if it succeeded, false if any exceptions were thrown
    bool Invoke(ExceptionReport& report);

    // The same as the above invoke, but it uses the last exception report on the stack
    // Returns true if it succeeded, false if any exceptions were thrown
    bool Invoke();

    // Get a reference to the executable state
    ExecutableState* GetState();

    // Get a raw pointer to the stack
    byte* GetStackUnchecked();
    
    // Get a raw pointer to the stack where the 'this' handle is placed
    byte* GetThisUnchecked();

    // Get a raw pointer to the stack where the parameters are placed
    byte* GetParametersUnchecked();

    // Get a raw pointer to the stack where a particular parameter is placed
    byte* GetParameterUnchecked(size_t parameterIndex);

    // Get a raw pointer to the stack where the return is placed
    byte* GetReturnUnchecked();
    
    // Get the function involved in the call
    Function* GetFunction();

    // Get the property involved in the call (or null if this is not a property)
    Property* GetProperty();

    // Makes it so we ignore debug checking for parameters
    // Only set this flag if you plan on raw manipulating the stack
    // If raw manipulation is done, make sure you also clean up the stack manually
    void DisableParameterChecks();

    // Makes it so we ignore debug checking for the 'this' handle
    // Only set this flag if you plan on raw manipulating the stack
    // If raw manipulation is done, make sure you also clean up the stack manually
    void DisableThisChecks();
    
    // Makes it so we ignore debug checking for the return value
    // Only set this flag if you plan on raw manipulating the stack
    // If raw manipulation is done, make sure you also clean up the stack manually
    void DisableReturnChecks();

    // Disables the automatic destruction of parameters at the end of the call
    // Only disable this feature if you plan on manually cleaning up the stack
    void DisableParameterDestruction();
    
    // Disables the automatic destruction of the 'this' handle at the end of the call
    // Only disable this feature if you plan on manually cleaning up the stack
    void DisableThisDestruction();
    
    // Disables the automatic destruction of the return value at the end of the call
    // Only disable this feature if you plan on manually cleaning up the stack
    void DisableReturnDestruction();

    // Tells the debugging features that the parameter, return, or this handle was set
    // Only use this if you set the return via direct stack memory
    void MarkAsSet(size_t index);

    // Tells the debugging features that the return value was set
    // Only use this if you set the return via direct stack memory
    void MarkReturnAsSet();

    // Tells the debugging features that the 'this' handle was set
    // Only use this if you set the return via direct stack memory
    void MarkThisAsSet();

    // Tells the debugging features that the 'this' handle was set
    // Only use this if you set the return via direct stack memory
    void MarkParameterAsSet(size_t parameterIndex);

    // Get a generic stack location and do error checking
    byte* GetChecked(size_t index, size_t size, Type* userType, CheckPrimitive::Enum primitive, Direction::Enum io);

    // Get a generic stack location and don't do any error checking
    byte* GetUnchecked(size_t index);

  private:
    
    // Run a set of checks on the given type / size
    void PerformStandardChecks(size_t size, Type* userType, Type* actualType, CheckPrimitive::Enum primitive, Direction::Enum io);

    // Get a stack location to the 'this' handle and do error checking
    byte* GetThisChecked(size_t size, Type* userType, CheckPrimitive::Enum primitive, Direction::Enum io);

    // Get a stack location to the return and do error checking
    byte* GetReturnChecked(size_t size, Type* userType, CheckPrimitive::Enum primitive, Direction::Enum io);

    // Get a stack location to the given parameter and do error checking
    byte* GetParameterChecked(size_t parameterIndex, size_t size, Type* userType, CheckPrimitive::Enum primitive, Direction::Enum io);

    // Constructor for the virtual machine call
    Call(PerFrameData* data);

    // Make sure we can't copy this around
    ZilchNoCopy(Call);

  private:

    // Every call corresponds with per frame data
    PerFrameData* Data;
  };

  // A helper for allocating a type within Zilch using the current executable state
  #define ZilchAllocate(T, ...) (ZZ::ExecutableState::GetCallingState()->AllocateDefaultConstructed<T>(__VA_ARGS__))
  #define ZilchAllocateUntyped(...) (ZZ::ExecutableState::GetCallingState()->AllocateDefaultConstructedHeapObject(__VA_ARGS__))

  template <> ZilchCallHelperSpecialization(      Any*,       Any* const&);
  template <> ZilchCallHelperSpecialization(      Any , const Any&);
  template <> ZilchCallHelperSpecialization(      Any&,       Any&);
  template <> ZilchCallHelperSpecialization(const Any*, const Any* const&);
  template <> ZilchCallHelperSpecialization(const Any&, const Any&);
  
  template <> ZilchCallHelperSpecialization(      Handle*,       Handle* const&);
  template <> ZilchCallHelperSpecialization(      Handle , const Handle&);
  template <> ZilchCallHelperSpecialization(      Handle&,       Handle&);
  template <> ZilchCallHelperSpecialization(const Handle*, const Handle* const&);
  template <> ZilchCallHelperSpecialization(const Handle&, const Handle&);

  template <> ZilchCallHelperSpecialization(      Delegate*,       Delegate* const&);
  template <> ZilchCallHelperSpecialization(      Delegate , const Delegate&);
  template <> ZilchCallHelperSpecialization(      Delegate&,       Delegate&);
  template <> ZilchCallHelperSpecialization(const Delegate*, const Delegate* const&);
  template <> ZilchCallHelperSpecialization(const Delegate&, const Delegate&);

  template <typename T> ZilchCallHelperTemplateSpecialization(      HandleOf<T>*,       HandleOf<T>* const&);
  template <typename T> ZilchCallHelperTemplateSpecialization(      HandleOf<T> , const HandleOf<T>&);
  template <typename T> ZilchCallHelperTemplateSpecialization(      HandleOf<T>&,       HandleOf<T>&);
  template <typename T> ZilchCallHelperTemplateSpecialization(const HandleOf<T>*, const HandleOf<T>* const&);
  template <typename T> ZilchCallHelperTemplateSpecialization(const HandleOf<T>&, const HandleOf<T>&);

  //***************************************************************************
  template <typename T>
  HandleOf<T>* CallHelper<HandleOf<T>*>::Get(Call& call, size_t index)
  {
    return (HandleOf<T>*)&call.GetHandle(index);
  }

  //***************************************************************************
  template <typename T>       HandleOf<T>  CallHelper<      HandleOf<T> >::Get(Call& call, size_t index) { return *CallHelper<HandleOf<T>*>::Get(call, index); }
  template <typename T>       HandleOf<T>& CallHelper<      HandleOf<T>&>::Get(Call& call, size_t index) { return *CallHelper<HandleOf<T>*>::Get(call, index); }
  template <typename T> const HandleOf<T>* CallHelper<const HandleOf<T>*>::Get(Call& call, size_t index) { return  CallHelper<HandleOf<T>*>::Get(call, index); }
  template <typename T> const HandleOf<T>& CallHelper<const HandleOf<T>&>::Get(Call& call, size_t index) { return *CallHelper<HandleOf<T>*>::Get(call, index); }

  //***************************************************************************
  template <typename T>
  void CallHelper<const HandleOf<T>*>::Set(Call& call, size_t index, const HandleOf<T>* const& value)
  {
    call.SetHandle(index, *static_cast<const Handle*>(value));
  }

  //***************************************************************************
  template <typename T> void CallHelper<      HandleOf<T> >::Set(Call& call, size_t index, const HandleOf<T>&        value) { CallHelper<const HandleOf<T>*>::Set(call, index, &value); }
  template <typename T> void CallHelper<      HandleOf<T>&>::Set(Call& call, size_t index,       HandleOf<T>&        value) { CallHelper<const HandleOf<T>*>::Set(call, index, &value); }
  template <typename T> void CallHelper<      HandleOf<T>*>::Set(Call& call, size_t index,       HandleOf<T>* const& value) { CallHelper<const HandleOf<T>*>::Set(call, index,  value); }
  template <typename T> void CallHelper<const HandleOf<T>&>::Set(Call& call, size_t index, const HandleOf<T>&        value) { CallHelper<const HandleOf<T>*>::Set(call, index, &value); }

  //***************************************************************************
  template <typename T>
  byte* CallHelper<HandleOf<T>*>::GetArgumentPointer(Call& call, size_t index)
  {
    return call.GetHandlePointer(index);
  }

  //***************************************************************************
  template <typename T> byte* CallHelper<      HandleOf<T> >::GetArgumentPointer(Call& call, size_t index) { return CallHelper<HandleOf<T>*>::GetArgumentPointer(call, index); }
  template <typename T> byte* CallHelper<      HandleOf<T>&>::GetArgumentPointer(Call& call, size_t index) { return CallHelper<HandleOf<T>*>::GetArgumentPointer(call, index); }
  template <typename T> byte* CallHelper<const HandleOf<T>*>::GetArgumentPointer(Call& call, size_t index) { return CallHelper<HandleOf<T>*>::GetArgumentPointer(call, index); }
  template <typename T> byte* CallHelper<const HandleOf<T>&>::GetArgumentPointer(Call& call, size_t index) { return CallHelper<HandleOf<T>*>::GetArgumentPointer(call, index); }

  //***************************************************************************
  template <typename T>
  HandleOf<T>* CallHelper<HandleOf<T>*>::CastArgumentPointer(byte* stackPointer)
  {
    return (HandleOf<T>*)stackPointer;
  }

  //***************************************************************************
  template <typename T>       HandleOf<T>  CallHelper<      HandleOf<T> >::CastArgumentPointer(byte* stackPointer) { return *CallHelper<HandleOf<T>*>::CastArgumentPointer(stackPointer); }
  template <typename T>       HandleOf<T>& CallHelper<      HandleOf<T>&>::CastArgumentPointer(byte* stackPointer) { return *CallHelper<HandleOf<T>*>::CastArgumentPointer(stackPointer); }
  template <typename T> const HandleOf<T>* CallHelper<const HandleOf<T>*>::CastArgumentPointer(byte* stackPointer) { return  CallHelper<HandleOf<T>*>::CastArgumentPointer(stackPointer); }
  template <typename T> const HandleOf<T>& CallHelper<const HandleOf<T>&>::CastArgumentPointer(byte* stackPointer) { return *CallHelper<HandleOf<T>*>::CastArgumentPointer(stackPointer); }
}

#endif
