/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  ZilchDefineType(Function, builder, type)
  {
    type->HandleManager = ZilchManagerId(PointerManager);
    ZilchFullBindField(builder, type, &Function::FunctionType, "Type", PropertyBinding::Get);
    ZilchFullBindMethod(builder, type, &Function::CreateDelegate, ZilchNoOverload, "CreateDelegate", "instance");
    ZilchFullBindMethod(builder, type, &Function::Invoke, ZilchNoOverload, "Invoke", "instance, arguments");
  }

  //***************************************************************************
  NativeVirtualInfo::NativeVirtualInfo() :
    Index(NonVirtual),
    Thunk(nullptr),
    Guid(InvalidGuid)
  {
  }

  //***************************************************************************
  bool NativeVirtualInfo::Validate()
  {
    // Error checking for the native virtual calls
    ReturnIf
    (
      (this->Index == NonVirtual && this->Thunk != nullptr) ||
      (this->Index != NonVirtual && this->Thunk == nullptr),
      false,
      "You must provide both the virtual index and thunk, or neither"
    );

    // Error check the guid
    ReturnIf
    (
      this->Index != NonVirtual && this->Guid == InvalidGuid,
      false,
      "The guid provided was not valid"
    );

    // Otherwise, we validated
    return true;
  }

  //***************************************************************************
  Function::Function() :
    BoundFunction(nullptr),
    NativeConstructor(nullptr),
    FunctionType(nullptr),
    RequiredStackSpace(0),
    This(nullptr),
    SourceLibrary(nullptr),
    OwningProperty(nullptr),
    IsVirtual(false),
    Hash(0)
  {
  }
  
  //***************************************************************************
  Type* Function::GetTypeOrNull()
  {
    return this->FunctionType;
  }

  //***************************************************************************
  FunctionOptions::Flags Function::GetFunctionOptions()
  {
    FunctionOptions::Flags flags = FunctionOptions::None;
    if (this->IsStatic)
      flags |= FunctionOptions::Static;

    if (this->IsVirtual)
      flags |= FunctionOptions::Virtual;
    return flags;
  }

  //***************************************************************************
  void Function::ComputeHash()
  {
    // Store the resulting hash
    GuidType result = 0;

    // If this function has a this pointer
    if (this->This != nullptr)
    {
      // Take the hash of the 'this' type, and hash it with the delegate type hash
      result = this->This->ResultType->Hash();
    }

    // Add in the delegate type hash
    result ^= this->FunctionType->Hash() * 27697;

    // Add in the hash of the function name
    result ^= this->Name.Hash() * 13;

    // Hash the owner and add that in
    result ^= this->Owner->Hash() * 4738837;

    // Store the resulting hash
    this->Hash = result;
  }

  //***************************************************************************
  String Function::ToString() const
  {
    // The string we build for a function is the name of the class, then the name of the function
    // and then the entire signature, ex: Animal.Attack(damage : Integer) : String
    StringBuilder builder;

    // This should NEVER be null, but we have a case where we're getting a crash
    // For now, we're going to skip writing the name and access symbol, but we'll put an assert in to try and catch it
    if (this->Owner != nullptr)
    {
      builder.Append(this->Owner->Name);
      builder.Append(Grammar::GetKeywordOrSymbol(Grammar::Access));
    }
    else
    {
      Error("The Owner should not be null inside the Function");
    }

    builder.Append(this->Name);
    this->FunctionType->BuildSignatureString(builder);
    return builder.ToString();
  }

  //***************************************************************************
  CodeLocation* Function::GetCodeLocationFromProgramCounter(size_t programCounter)
  {
    // We don't know the location of native and non-active program counters
    if (programCounter == ProgramCounterNative || programCounter == ProgramCounterNotActive)
      return nullptr;

    // Now get the code location for the given opcode inside the function
    CodeLocation* codeLocation = this->OpcodeLocationToCodeLocation.FindPointer(programCounter);
    if (codeLocation == nullptr)
      return nullptr;

    // Return the code location we found
    return codeLocation;
  }

  //***************************************************************************
  Opcode& Function::AllocateArgumentFreeOpcode(Instruction::Enum instruction, DebugOrigin::Enum debugOrigin, const CodeLocation& debugLocation)
  {
    return AllocateOpcode<Opcode>(instruction, debugOrigin, debugLocation);
  }

  //***************************************************************************
  OperandIndex Function::AllocateRegister(size_t size)
  {
    // Get the last index of the registers array
    size_t index = this->RequiredStackSpace;

    // Allocate the spot in the registers
    this->RequiredStackSpace += AlignToBusWidth(size);

    // Return the index
    return (OperandIndex)index;
  }

  //***************************************************************************
  size_t Function::GetCurrentOpcodeIndex()
  {
    return this->OpcodeBuilder.RelativeSize();
  }
  
  //***************************************************************************
  Any Function::CreateDelegate(const Any& instance)
  {
    Handle thisHandle;
    if (this->ValidateInstanceHandle(instance, thisHandle) == false)
      return Any();

    Delegate delegate;
    delegate.BoundFunction = this;
    delegate.ThisHandle = thisHandle;
    return delegate;
  }
  
  //***************************************************************************
  Any Function::Invoke(const Any& instance, ArrayClass<Any>* arguments)
  {
    Handle thisHandle;
    if (this->ValidateInstanceHandle(instance, thisHandle) == false)
      return Any();

    ExecutableState* state = ExecutableState::CallingState;

    // Count how many arguments we were given (null array is fine, but treated as 0 arguments)
    size_t argumentCount = 0;
    if (arguments != nullptr)
      argumentCount = arguments->NativeArray.Size();

    // Make sure we have the correct number of arguments
    ParameterArray& parameters = this->FunctionType->Parameters;
    size_t expectedCount = parameters.Size();
    if (argumentCount != expectedCount)
    {
      String message = String::Format
      (
        "Attempting to invoke a function with an incorrect number of arguments (got %d, expected %d)",
        argumentCount,
        expectedCount
      );
      state->ThrowException(message);
      return Any();
    }

    // Validate that the arguments are of the same type (or raw convertable)
    for (size_t i = 0; i < argumentCount; ++i)
    {
      Type* expectedType = parameters[i].ParameterType;
      Type* argumentType = arguments->NativeArray[i].StoredType;

      // Look up a cast operator between the two types
      // Note that if the types are the same, a cast always technically exists of 'Raw' type
      // This ALSO gives us an invalid cast and handles if the user gave us a empty any (with a null StoredType)
      CastOperator cast = Shared::GetInstance().GetCastOperator(argumentType, expectedType);
      if (cast.IsValid == false || cast.Operation != CastOperation::Raw)
      {
        String message = String::Format
        (
          "Parameter %d expected the type '%s' but was given '%s' (which could not be raw-converted)",
          i,
          expectedType->ToString().c_str(),
          argumentType->ToString().c_str()
        );
        state->ThrowException(message);
        return Any();
      }
    }

    // Now call the function by copying all the arguments to the Zilch stack
    ExceptionReport& report = state->GetCallingReport();
    Call call(this, state);
    if (this->IsStatic == false)
      call.SetHandle(Call::This, thisHandle);

    // The call will assert because we don't bother to mark all parameters as set (we validated already)
    call.DisableParameterChecks();

    // Copy each argument from the any to the Zilch stack (as its actual value)
    for (size_t i = 0; i < argumentCount; ++i)
    {
      byte* stackParameter = call.GetParameterUnchecked(i);
      Any& argument = arguments->NativeArray[i];
      const byte* argumentData = argument.GetData();
      argument.StoredType->GenericCopyConstruct(stackParameter, argumentData);
    }

    // Finally invoke the function and get our result back
    call.Invoke(report);

    if (report.HasThrownExceptions())
      return Any();
    
    // Fetch the return value and construct and any from it
    byte* returnValue = call.GetReturnUnchecked();
    return Any(returnValue, this->FunctionType->Return);
  }
}