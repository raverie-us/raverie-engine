/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_FUNCTION_HPP
#define ZILCH_FUNCTION_HPP

namespace Zilch
{
  // Information we hand to the 
  class ZeroShared NativeVirtualInfo
  {
  public:
    // A special index that means that a function is non-virtual
    static const size_t NonVirtual = (size_t)-1;
    static const GuidType InvalidGuid = (GuidType)-1;

    // Constructor
    NativeVirtualInfo();

    // Validates that the given data is valid (returns false if it's invalid)
    bool Validate();

    // Is this method defined natively as a virtual function? (was it bound as a virtual function?)
    // If so, then this represents the index into the virtual table for the C++ type
    // If this function is not natively virtual, it will be set to 'NonVirtual'
    size_t Index;

    // In the case that this function is considered to be virtual
    // this will be set to the thunk function that actually invokes Zilch
    // from a native virtual call (or null if it's non-virtual)
    TypeBinding::VirtualTableFn Thunk;

    // In order to map a native function thunk back to it's Zilch function, we need
    // every bound virtual function to have it's own guid that we use in a map
    GuidType Guid;
  };

  // Any options we use while building a function
  namespace FunctionOptions
  {
    enum Enum
    {
      // No options
      None = 0,

      // A member that is not part of an instance,
      // but rather part of the type itself
      Static = 1,

      // A function that is virtual can be overridden by
      // another function in a more derived class
      Virtual = 2
    };
    typedef unsigned Flags;
  }

  // A base function
  class ZeroShared Function : public Member
  {
  public:
    // Declare the class for RTTI
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Constructor
    Function();

    // ReflectionObject interface
    Type* GetTypeOrNull() override;

    // A helper to return the flags for a function
    FunctionOptions::Flags GetFunctionOptions();

    // Get the function in string form
    String ToString() const;
    
    // Gets the code location from a given program counter location
    // If the program counter is for a native function (or non active), this will return null
    CodeLocation* GetCodeLocationFromProgramCounter(size_t programCounter);

    // Allocate an argumentless opcode
    Opcode& AllocateArgumentFreeOpcode(Instruction::Enum instruction, DebugOrigin::Enum debugOrigin, const CodeLocation& debugLocation);

    // Allocate an opcode of type T
    template <typename T>
    T& AllocateOpcode(Instruction::Enum instruction, DebugOrigin::Enum debugOrigin, const CodeLocation& debugLocation)
    {
      // Get the compacted index
      size_t compactedIndex = this->OpcodeBuilder.RelativeSize();

      // Get an element of memory with the size of the opcode
      byte* element = this->OpcodeBuilder.RequestElementOfSize(sizeof(T));

      // Get a reference to the opcode
      T& opcode = *new (element) T();

      // Make sure this opcode location is valid...
      ErrorIf(debugLocation.Origin == UnknownOrigin,
        "A code location given for an opcode was from an unknown location (opcode always gets generated from real code!)");

      // For debugging, we need to know from an opcode index where it originated from
      this->OpcodeLocationToCodeLocation.Insert(compactedIndex, debugLocation);

      // We use the compacted indices for debugging
      this->OpcodeCompactedIndices.PushBack(compactedIndex);

#ifdef ZeroDebug
      // Add the debug info to the list
      this->OpcodeDebug.PushBack(&opcode);
      opcode.DebugOrigin = debugOrigin;
#endif

      // Set the instruction
      opcode.Instruction = instruction;

      // Return the opcode to be filled in
      return opcode;
    }

    // Allocates a register and returns its index
    OperandIndex AllocateRegister(size_t size);

    // Allocates a constant and returns its index
    template <typename T>
    T& AllocateConstant(size_t constantSize, OperandIndex& indexOut)
    {
      // Allocate the spot in the constants
      size_t largeIndex;
      T& constant = this->Constants.CreateObject<T>(&largeIndex);
      
      // Get the index where we allocated the constant
      indexOut = (OperandIndex)largeIndex;

      // Return the constant to the user
      return constant;
    }

    // Get the current index into the opcode
    size_t GetCurrentOpcodeIndex();

    // Now that opcode has been compacted, set the
    // debug opcode to point at the compacted opcode
    void SetupCompactedOpcodeDebug();

    // Create a delegate that is castable / callable
    Any CreateDelegate(const Any& instance);

    // Attempts to invoke a function with the specified arguments, or throws an exception
    Any Invoke(const Any& instance, ArrayClass<Any>* arguments);

    // Computes the hash and stores it
    void ComputeHash();

  public:

    // A pre-computed hash for the function
    GuidType Hash;

    // Documentation for each of the parameters (should match the number of parameters in the DelegateType)
    Array<String> ParameterDescriptions;

    // The bound function is the function that gets called when this function is invoked
    BoundFn BoundFunction;

    // When we're binding a native constructor, we need our actual 'BoundFunction' to run
    // special code that lets us know the object reached native construction
    // We store the actual constructor here (in that case, the above BoundFunction just invokes the NativeConstructor)
    // This will be initialized in AddBoundConstructor on the LibraryBuilder
    BoundFn NativeConstructor;

    // The variables associated with this function
    Array<Variable*> Variables;

    // Store the type of the function
    DelegateType* FunctionType;

    // In order to invoke this function, a given amount of stack space is required
    // For example, all the parameters need space on the stack, and the return also needs space
    // Moreover, any local variables in a compiled function also need space on the stack
    // For a user bound function, the required space only includes the parameters and returns,
    // because any locals they will store will be on the actual C++ stack, not on ours
    size_t RequiredStackSpace;

    // A pointer the 'this' variable (or null if it is static)
    // Note that the current calling convention is that the this pointer is the
    // last parameter passed on the stack, which means this should point to the end of
    // the required stack space
    Variable* This;

    // Store the parent library from which the function originated
    // This information is generally used for linking purposes. When we call a
    // function we need the library it came from so we can re-link it back up
    Library* SourceLibrary;

    // Any information related to this function being native and virtual
    NativeVirtualInfo NativeVirtual;

    // If this function is a property getter or setter, then this will point back to the property that owns us
    Property* OwningProperty;

    // If this function is virtual or not (whether it can be overridden)
    // All overriding functions are also marked as virtual
    bool IsVirtual;

    // All the constants used in this function (only used for compiled functions)
    DestructibleBuffer Constants;

    // A temporary buffer for storing opcode
    UntypedBlockArray<1024> OpcodeBuilder;

    // The opcode for this function compacted into a linear array
    Array<byte> CompactedOpcode;

    // Store a list of pointers to opcodes for debugging purposes
    Array<size_t> OpcodeCompactedIndices;

    // Maps from an opcode offset to a code location (so we can determine where we are in debugging)
    HashMap<size_t, CodeLocation> OpcodeLocationToCodeLocation;

#ifdef ZeroDebug
    PodArray<Opcode*> OpcodeDebug;
#endif
  };
}

#endif
