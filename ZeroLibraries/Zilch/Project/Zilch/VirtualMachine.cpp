/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  // This is just a special identifier that means we jumped, there's really no reason to the number... ;)
  static const int ExceptionJumpResult = 1729;

  //***************************************************************************
  template <>
  void VirtualMachine::GenericPow<Byte>(Byte& out, const Byte& base, const Byte& exponent)
  {
    out = (Byte)IntegralPower(base, exponent);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericPow<Integer>(Integer& out, const Integer& base, const Integer& exponent)
  {
    out = IntegralPower(base, exponent);
  }

  //***************************************************************************
  template <>
  inline void VirtualMachine::GenericPow<Integer2>(Integer2& out, const Integer2& base, const Integer2& exponent)
  {
    out.x = IntegralPower(base.x, exponent.x);
    out.y = IntegralPower(base.y, exponent.y);
  }

  //***************************************************************************
  template <>
  inline void VirtualMachine::GenericPow<Integer3>(Integer3& out, const Integer3& base, const Integer3& exponent)
  {
    out.x = IntegralPower(base.x, exponent.x);
    out.y = IntegralPower(base.y, exponent.y);
    out.z = IntegralPower(base.z, exponent.z);
  }

  //***************************************************************************
  template <>
  inline void VirtualMachine::GenericPow<Integer4>(Integer4& out, const Integer4& base, const Integer4& exponent)
  {
    out.x = IntegralPower(base.x, exponent.x);
    out.y = IntegralPower(base.y, exponent.y);
    out.z = IntegralPower(base.z, exponent.z);
    out.w = IntegralPower(base.w, exponent.w);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericPow<DoubleInteger>(DoubleInteger& out, const DoubleInteger& base, const DoubleInteger& exponent)
  {
    out = IntegralPower(base, exponent);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericPow<Real2>(Real2& out, const Real2& base, const Real2& exponent)
  {
    out.x = std::pow(base.x, exponent.x);
    out.y = std::pow(base.y, exponent.y);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericPow<Real3>(Real3& out, const Real3& base, const Real3& exponent)
  {
    out.x = std::pow(base.x, exponent.x);
    out.y = std::pow(base.y, exponent.y);
    out.z = std::pow(base.z, exponent.z);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericPow<Real4>(Real4& out, const Real4& base, const Real4& exponent)
  {
    out.x = std::pow(base.x, exponent.x);
    out.y = std::pow(base.y, exponent.y);
    out.z = std::pow(base.z, exponent.z);
    out.w = std::pow(base.w, exponent.w);
  }
  
  //***************************************************************************
  template <>
  void VirtualMachine::GenericMod<Real>(Real& out, const Real& value, const Real& mod)
  {
    out = std::fmod(value, mod);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericMod<DoubleReal>(DoubleReal& out, const DoubleReal& value, const DoubleReal& mod)
  {
    out = std::fmod(value, mod);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericMod<Real2>(Real2& out, const Real2& value, const Real2& mod)
  {
    out.x = std::fmod(value.x, mod.x);
    out.y = std::fmod(value.y, mod.y);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericMod<Real3>(Real3& out, const Real3& value, const Real3& mod)
  {
    out.x = std::fmod(value.x, mod.x);
    out.y = std::fmod(value.y, mod.y);
    out.z = std::fmod(value.z, mod.z);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericMod<Real4>(Real4& out, const Real4& value, const Real4& mod)
  {
    out.x = std::fmod(value.x, mod.x);
    out.y = std::fmod(value.y, mod.y);
    out.z = std::fmod(value.z, mod.z);
    out.w = std::fmod(value.w, mod.w);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericScalarPow<Integer2, Integer>(Integer2& out, const Integer2& base, const Integer& exponent)
  {
    out.x = IntegralPower(base.x, exponent);
    out.y = IntegralPower(base.y, exponent);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericScalarPow<Integer3, Integer>(Integer3& out, const Integer3& base, const Integer& exponent)
  {
    out.x = IntegralPower(base.x, exponent);
    out.y = IntegralPower(base.y, exponent);
    out.z = IntegralPower(base.z, exponent);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericScalarPow<Integer4, Integer>(Integer4& out, const Integer4& base, const Integer& exponent)
  {
    out.x = IntegralPower(base.x, exponent);
    out.y = IntegralPower(base.y, exponent);
    out.z = IntegralPower(base.z, exponent);
    out.w = IntegralPower(base.w, exponent);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericScalarPow<Real2, Real>(Real2& out, const Real2& base, const Real& exponent)
  {
    out.x = std::pow(base.x, exponent);
    out.y = std::pow(base.y, exponent);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericScalarPow<Real3, Real>(Real3& out, const Real3& base, const Real& exponent)
  {
    out.x = std::pow(base.x, exponent);
    out.y = std::pow(base.y, exponent);
    out.z = std::pow(base.z, exponent);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericScalarPow<Real4, Real>(Real4& out, const Real4& base, const Real& exponent)
  {
    out.x = std::pow(base.x, exponent);
    out.y = std::pow(base.y, exponent);
    out.z = std::pow(base.z, exponent);
    out.w = std::pow(base.w, exponent);
  }
  

  //***************************************************************************
  template <>
  void VirtualMachine::GenericScalarMod<Real2, Real>(Real2& out, const Real2& value, const Real& mod)
  {
    out.x = std::fmod(value.x, mod);
    out.y = std::fmod(value.y, mod);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericScalarMod<Real3, Real>(Real3& out, const Real3& value, const Real& mod)
  {
    out.x = std::fmod(value.x, mod);
    out.y = std::fmod(value.y, mod);
    out.z = std::fmod(value.z, mod);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericScalarMod<Real4, Real>(Real4& out, const Real4& value, const Real& mod)
  {
    out.x = std::fmod(value.x, mod);
    out.y = std::fmod(value.y, mod);
    out.z = std::fmod(value.z, mod);
    out.w = std::fmod(value.w, mod);
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericIncrement<Real2>(Real2& out)
  {
    ++out.x;
    ++out.y;
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericIncrement<Real3>(Real3& out)
  {
    ++out.x;
    ++out.y;
    ++out.z;
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericIncrement<Real4>(Real4& out)
  {
    ++out.x;
    ++out.y;
    ++out.z;
    ++out.w;
  }
  
  //***************************************************************************
  template <>
  void VirtualMachine::GenericDecrement<Real2>(Real2& out)
  {
    --out.x;
    --out.y;
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericDecrement<Real3>(Real3& out)
  {
    --out.x;
    --out.y;
    --out.z;
  }

  //***************************************************************************
  template <>
  void VirtualMachine::GenericDecrement<Real4>(Real4& out)
  {
    --out.x;
    --out.y;
    --out.z;
    --out.w;
  }

  //***************************************************************************
  template <>
  bool VirtualMachine::GenericIsZero<Integer2>(const Integer2& value)
  {
    return
      value.x == 0 ||
      value.y == 0;
  }

  //***************************************************************************
  template <>
  bool VirtualMachine::GenericIsZero<Integer3>(const Integer3& value)
  {
    return
      value.x == 0 ||
      value.y == 0 ||
      value.z == 0;
  }

  //***************************************************************************
  template <>
  bool VirtualMachine::GenericIsZero<Integer4>(const Integer4& value)
  {
    return
      value.x == 0 ||
      value.y == 0 ||
      value.z == 0 ||
      value.w == 0;
  }
  
  //***************************************************************************
  template <>
  bool VirtualMachine::GenericIsZero<Real2>(const Real2& value)
  {
    return
      value.x == 0.0f ||
      value.y == 0.0f;
  }

  //***************************************************************************
  template <>
  bool VirtualMachine::GenericIsZero<Real3>(const Real3& value)
  {
    return
      value.x == 0.0f ||
      value.y == 0.0f ||
      value.z == 0.0f;
  }

  //***************************************************************************
  template <>
  bool VirtualMachine::GenericIsZero<Real4>(const Real4& value)
  {
    return
      value.x == 0.0f ||
      value.y == 0.0f ||
      value.z == 0.0f ||
      value.w == 0.0f;
  }

  //***************************************************************************
  // Get a reference to a member variable (field), given the place in the registers that the handle exists, and the member index...
  template <typename T>
  ZilchForceInline T& GetField(PerFrameData* stackFrame, PerFrameData* reportFrame, OperandIndex handleOperand, size_t memberOperand)
  {
    // Grab the handle to the object
    Handle& handle = *(Handle*)(stackFrame->Frame + handleOperand);

    // Get a pointer to the data
    byte* data = handle.Dereference();

    // If our data is null
    if (data == nullptr)
    {
      // Throw an exception (we'll need to unwind our stack)
      stackFrame->State->ThrowNullReferenceException(*reportFrame->Report);

      // Unwind our stack
      longjmp(reportFrame->ExceptionJump, ExceptionJumpResult);
    }

    // Return the data (with the member offset)
    return *(T*)(data + memberOperand);
  }

  //***************************************************************************
  // Get a particular constant from a function
  template <typename T>
  ZilchForceInline T& GetConstant(Function* function, OperandIndex constantOperand)
  {
    // Make sure the value we're grabbing is inside the constant buffer (error checking)
    ErrorIf(constantOperand < 0 || constantOperand + sizeof(T) > function->Constants.GetSize(),
      "The constant's position was outside the memory of the function's constants");

    // Grab the constant from the constants array
    return *(T*)(function->Constants.GetElement(constantOperand));
  }

  //***************************************************************************
  // Get a particular local from a function
  template <typename T>
  ZilchForceInline T& GetLocal(byte* frame, OperandIndex localOperand)
  {
    return *(T*)(frame + localOperand);
  }

  //***************************************************************************
  // Get a particular static from an operand
  template <typename T>
  ZilchForceInline T& GetStatic(PerFrameData* stackFrame, PerFrameData* reportFrame, const Operand& operand)
  {
    // Look for the static memory in a map of the fields on our state
    // Static fields are done per executable state, so they get wiped each time we quit
    ExecutableState* state = stackFrame->State;
    return *(T*)(state->GetStaticField(operand.StaticField, *reportFrame->Report) + operand.FieldOffset);
  }

  //***************************************************************************
  // Get an operand (we don't know what type it is)
  template <typename T>
  ZilchForceInline T& GetOperand(PerFrameData* stackFrame, PerFrameData* reportFrame, const Operand& operand)
  {
    // Based on what kind of operand it is...
    switch (operand.Type)
    {
      case OperandType::Field:
        return GetField<T>(stackFrame, reportFrame, operand.HandleConstantLocal, operand.FieldOffset);

      case OperandType::Constant:
        return GetConstant<T>(stackFrame->CurrentFunction, operand.HandleConstantLocal);

      case OperandType::Local:
        return GetLocal<T>(stackFrame->Frame, operand.HandleConstantLocal);

      case OperandType::StaticField:
        return GetStatic<T>(stackFrame, reportFrame, operand);
    }
    
    // This means that something REALLY bad happened...
    // Throw an exception (we'll need to unwind our stack)
    const char* message = "We reached a garbage operand, or the operand was NotSet (something wrong in CodeGeneration?)";
    Error(message);
    stackFrame->State->ThrowException(*reportFrame->Report, message);

    // Unwind our stack
    longjmp(reportFrame->ExceptionJump, ExceptionJumpResult);
  }

  //***************************************************************************
  // Reusable code for the if opcodes
  template <Boolean IfTrue>
  ZilchForceInline void IfHandler(PerFrameData* stackFrame, const Opcode& opcode)
  {
    // Validate the timeout (this will throw an exception if we go beyond the time we need to)
    // This only really needs to be ran in jumps
    if (stackFrame->State->ThrowExceptionOnTimeout(*stackFrame->Report))
    {
      // Unwind our stack
      longjmp(stackFrame->ExceptionJump, ExceptionJumpResult);
    }

    // Grab the rest of the data
    const IfOpcode& op = (const IfOpcode&) opcode;

    // Read the boolean value
    Boolean result = GetOperand<Boolean>(stackFrame, stackFrame, op.Condition);

    // If the register evaluates to true...
    if (result == IfTrue)
    {
      // Move the instruction counter by the given offset
      stackFrame->ProgramCounter += op.JumpOffset;
    }
    // Otherwise, we need to skip it
    else
    {
      // Move the instruction counter past this opcode
      stackFrame->ProgramCounter += sizeof(op);
    }
  }

  //***************************************************************************
  ZilchForceInline void CopyHandlerEx(PerFrameData* ourFrame, PerFrameData* topFrame, const byte*& sourceOut, byte*& destinationOut, const CopyOpcode& op)
  {
    // When we copy to parameters, it's always a destination
    // (we are placing parameters in the place they must go before we call)
    // When we copy a return, it is always the source, as we are getting the result
    // from a called function and storing it in the caller's stack
    // Note: Returns may be removed in the future as we can just directly refer to them

    // Offsets that may change depending on the copy mode
    size_t returnOffset = 0;
    PerFrameData* parameterFrame = ourFrame;

    // Based off the copy mode we're in...
    switch (op.Mode)
    {
      case CopyMode::ToParameter:
        parameterFrame = topFrame;
        break;

      case CopyMode::FromReturn:
        returnOffset = topFrame->NextFrame - ourFrame->Frame;
        break;
    }

    // Get pointers to both the source and destination memory
    sourceOut = &GetOperand<byte>(ourFrame, ourFrame, op.Source) + returnOffset;
    destinationOut = &GetOperand<byte>(parameterFrame, ourFrame, op.Destination);
    
    // Increment the program counter to point past the opcode
    ourFrame->ProgramCounter += sizeof(CopyOpcode);
  }

  //***************************************************************************
  // Note: This function return a pointer to the newly initialized handle
  // (where we copied to) unless we're performing a 'FromReturn' copy, where
  // it will return a pointer to the source handle (for cleanup purposes!)
  template <typename CopyType>
  ZilchForceInline void CopyHandler
  (
    PerFrameData* ourFrame,
    PerFrameData* topFrame,
    CopyType*& sourceTyped,
    CopyType*& destinationTyped,
    const CopyOpcode& op
  )
  {
    const byte* source;
    byte* destination;

    CopyHandlerEx(ourFrame, topFrame, source, destination, op);

    // Now copy from the source to the destination and return a pointer to the new copy
    sourceTyped = (CopyType*)source;

    // If this is an assignment...
    if (op.Mode == CopyMode::Assignment)
    {
      // We know the destination is the same type as us
      destinationTyped = (CopyType*)destination;

      // Perform the direct assignment
      *destinationTyped = *sourceTyped;
    }
    else
    {
      // Otherwise, we need to construct a new one over the memory
      destinationTyped = new (destination) CopyType(*sourceTyped);
    }
  }

  //***************************************************************************
  void VirtualMachine::EnumerationProperty(Call& call, ExceptionReport& report)
  {
    // Get the integral value for this enum value
    Integer integralValue = (Integer)(DoubleInteger)call.GetFunction()->UserData;

    // Return the integral value
    call.Set<Integer>(Call::Return, integralValue);
  }

  //***************************************************************************
  void VirtualMachine::EventsProperty(Call& call, ExceptionReport& report)
  {
    // Get the calling function (the property getter)
    Function* function = call.GetFunction();

    // Read its user data to get the string back that we want to return
    String& eventName = function->ComplexUserData.ReadObject<String>(0);

    // Simply just return the string (there are no parameters or anything else)
    call.Set<String>(Call::Return, eventName);
  }
  
  //***************************************************************************
  void VirtualMachine::NativeConstructor(Call& call, ExceptionReport& report)
  {
    Function* constructor = call.GetFunction();
    Handle& thisHandle = call.Get<Handle&>(Call::This);
    thisHandle.Manager->SetNativeTypeFullyConstructed(thisHandle, true);
    BoundType* oldType = ExecutableState::CallingState->AllocatingType;
    ExecutableState::CallingState->AllocatingType = thisHandle.StoredType;

    // Invoke the actual constructor
    constructor->NativeConstructor(call, report);

    ExecutableState::CallingState->AllocatingType = oldType;
  }
  
  //***************************************************************************
  void VirtualMachine::PatchDummy(Call& call, ExceptionReport& report)
  {
    // Default construct the return value
    byte* returnValue = call.GetReturnUnchecked();
    call.GetFunction()->FunctionType->Return->GenericDefaultConstruct(returnValue);
  }

  //***************************************************************************
  typedef void (*VirtualInstructionFn)(ExecutableState* state, Call& call, ExceptionReport& report, size_t& programCounter, PerFrameData* ourFrame, const Opcode& opcode);
  VirtualInstructionFn InstructionTable[Instruction::Count] = {0};
  #define ZilchVirtualInstruction(Name) void VirtualMachine::Instruction##Name (ExecutableState* state, Call& call, ExceptionReport& report, size_t& programCounter, PerFrameData* ourFrame, const Opcode& opcode)
  
  //*****************************************************************************
  #define ZilchCaseBinaryRValue2(argType1, argType2, resultType, operation, expression)                   \
    ZilchVirtualInstruction(operation##argType1)                                                          \
    {                                                                                                     \
      const BinaryRValueOpcode& op = (const BinaryRValueOpcode&) opcode;                                  \
      const argType1& left = GetOperand<argType1>(ourFrame, ourFrame, op.Left);                           \
      const argType2& right = GetOperand<argType2>(ourFrame, ourFrame, op.Right);                         \
      resultType& output = GetLocal<resultType>(ourFrame->Frame, op.Output);                              \
      expression;                                                                                         \
      programCounter += sizeof(BinaryRValueOpcode);                                                       \
    }

  //*****************************************************************************
  #define ZilchCaseBinaryLValue2(argType1, argType2, operation, expression)                               \
    ZilchVirtualInstruction(operation##argType1)                                                          \
    {                                                                                                     \
      const BinaryLValueOpcode& op = (const BinaryLValueOpcode&) opcode;                                  \
      argType1& output = GetOperand<argType1>(ourFrame, ourFrame, op.Output);                             \
      const argType2& right = GetOperand<argType2>(ourFrame, ourFrame, op.Right);                         \
      expression;                                                                                         \
      programCounter += sizeof(BinaryLValueOpcode);                                                       \
    }

  //*****************************************************************************
  #define ZilchCaseBinaryRValue(argType, resultType, operation, expression)                               \
    ZilchCaseBinaryRValue2(argType, argType, resultType, operation, expression)

  //*****************************************************************************
  #define ZilchCaseBinaryLValue(argType, operation, expression)                                           \
    ZilchCaseBinaryLValue2(argType, argType, operation, expression)

  //*****************************************************************************
  #define ZilchCaseUnaryRValue(argType, resultType, operation, expression)                                \
    ZilchVirtualInstruction(operation##argType)                                                           \
    {                                                                                                     \
      const UnaryRValueOpcode& op = (const UnaryRValueOpcode&) opcode;                                    \
      const argType& operand = GetOperand<argType>(ourFrame, ourFrame, op.SingleOperand);                 \
      resultType& output = GetLocal<resultType>(ourFrame->Frame, op.Output);                              \
      expression;                                                                                         \
      programCounter += sizeof(UnaryRValueOpcode);                                                        \
    }

  //*****************************************************************************
  #define ZilchCaseUnaryLValue(argType, operation, expression)                                            \
    ZilchVirtualInstruction(operation##argType)                                                           \
    {                                                                                                     \
      const UnaryLValueOpcode& op = (const UnaryLValueOpcode&) opcode;                                    \
      argType& operand = GetOperand<argType>(ourFrame, ourFrame, op.SingleOperand);                       \
      expression;                                                                                         \
      programCounter += sizeof(UnaryLValueOpcode);                                                        \
    }

  //*****************************************************************************
  #define ZilchCaseConversion(fromType, toType, expression)                                               \
    ZilchVirtualInstruction(Convert##fromType##To##toType)                                                \
    {                                                                                                     \
      const ConversionOpcode& op = (const ConversionOpcode&) opcode;                                      \
      const fromType& value = GetOperand<fromType>(ourFrame, ourFrame, op.ToConvert);                     \
      toType& output = GetLocal<toType>(ourFrame->Frame, op.Output);                                      \
      expression;                                                                                         \
      programCounter += sizeof(ConversionOpcode);                                                         \
    }

  //*****************************************************************************
  #define ZilchCaseSimpleCopy(T)                                                                          \
    ZilchVirtualInstruction(Copy##T)                                                                      \
    {                                                                                                     \
      PerFrameData* topFrame = state->StackFrames.Back();                                                 \
      T* source;                                                                                          \
      T* destination;                                                                                     \
      CopyHandler<T>                                                                                      \
      (                                                                                                   \
        ourFrame,                                                                                         \
        topFrame,                                                                                         \
        source,                                                                                           \
        destination,                                                                                      \
        (const CopyOpcode&) opcode                                                                        \
      );                                                                                                  \
    }

  //*****************************************************************************
  #define ZilchCaseComplexCopy(T)                                                                         \
    ZilchVirtualInstruction(Copy##T)                                                                      \
    {                                                                                                     \
      /* Grab the rest of the data */                                                                     \
      const CopyOpcode& op = (const CopyOpcode&) opcode;                                                  \
                                                                                                          \
      /* Get the current frame on the top of the stack */                                                 \
      PerFrameData* topFrame = state->StackFrames.Back();                                                 \
                                                                                                          \
      T* source;                                                                                          \
      T* destination;                                                                                     \
      CopyHandler<T>(ourFrame, topFrame, source, destination, op);                                        \
                                                                                                          \
      /* We need to make sure we cleanup any primitives */                                                \
      /* If this is just a standard copy from our stack to our stack... */                                \
      switch (op.Mode)                                                                                    \
      {                                                                                                   \
        case CopyMode::Initialize:                                                                        \
        {                                                                                                 \
          /* For any standard copy, if it's to the stack then */                                          \
          /* we just let our own stack frame clean it up */                                               \
          /* Note: Copies to properties are considered on the stack */                                    \
          /* We don't queue cleans for field initializers because the destructors will clean those up */  \
          OperandType::Enum destType = op.Destination.Type;                                               \
          if (destType != OperandType::Field && destType != OperandType::StaticField)                     \
          {                                                                                               \
            /* Queue the destination to be cleaned up */                                                  \
            ourFrame->Queue##T##Cleanup(destination);                                                     \
          }                                                                                               \
          break;                                                                                          \
        }                                                                                                 \
                                                                                                          \
        case CopyMode::ToParameter:                                                                       \
        {                                                                                                 \
          /* For parameter copies, our stack frame will still clean it up */                              \
          /* but the cleanup must occur right after the function is called */                             \
          /* eg PopFrame (which will pop the top!) */                                                     \
          topFrame->Queue##T##Cleanup(destination);                                                       \
          break;                                                                                          \
        }                                                                                                 \
                                                                                                          \
        case CopyMode::FromReturn:                                                                        \
        {                                                                                                 \
          /* Note: The primitive used here is actually the source primitive! */                           \
          /* See the comment above 'CopyHandler' */                                                       \
          /* For returns, we need to clean up the primitive immediately after */                          \
          /* copy since that space could be reused by anyone else */                                      \
          source->~T();                                                                                   \
                                                                                                          \
          /* We also need to queue our own frame to clean */                                              \
          /* up where we copied it to */                                                                  \
          ourFrame->Queue##T##Cleanup(destination);                                                       \
          break;                                                                                          \
        }                                                                                                 \
      }                                                                                                   \
    }

  // Note: These macros mirror those inside of InstructionEnum and Shared (for generation of instructions)

  // Copy
  #define ZilchCopyCases(WithType)                                                                                                                \
    ZilchCaseSimpleCopy(WithType)

  // Equality and inequality
  #define ZilchEqualityCases(WithType, ResultType)                                                                                                \
    ZilchCaseBinaryRValue(WithType, ResultType, TestInequality,  output = left != right);                                                         \
    ZilchCaseBinaryRValue(WithType, ResultType, TestEquality,    output = left == right);

  // Less and greater comparison
  #define ZilchComparisonCases(WithType, ResultType)                                                                                              \
    ZilchCaseBinaryRValue(WithType, ResultType, TestLessThan,              output = left < right);                                                \
    ZilchCaseBinaryRValue(WithType, ResultType, TestLessThanOrEqualTo,     output = left <= right);                                               \
    ZilchCaseBinaryRValue(WithType, ResultType, TestGreaterThan,           output = left > right);                                                \
    ZilchCaseBinaryRValue(WithType, ResultType, TestGreaterThanOrEqualTo,  output = left >= right);

  // Generic numeric operators, copy, equality
  #define ZilchNumericCases(WithType, ComparisonType)                                                                                             \
    ZilchCopyCases(WithType)                                                                                                                      \
    ZilchEqualityCases(WithType, ComparisonType)                                                                                                  \
    /* No case for unary plus */                                                                                                                  \
    ZilchCaseUnaryRValue (WithType, WithType, Negate,             output = -operand);                                                             \
    ZilchCaseUnaryLValue (WithType,           Increment,          GenericIncrement(operand));                                                     \
    ZilchCaseUnaryLValue (WithType,           Decrement,          GenericDecrement(operand));                                                     \
    ZilchCaseBinaryRValue(WithType, WithType, Add,                output = left + right);                                                         \
    ZilchCaseBinaryRValue(WithType, WithType, Subtract,           output = left - right);                                                         \
    ZilchCaseBinaryRValue(WithType, WithType, Multiply,           output = left * right);                                                         \
    ZilchCaseBinaryRValue(WithType, WithType, Divide,             if (GenericIsZero(right))                                                       \
                                                                  {                                                                               \
                                                                    state->ThrowException(report, "Attempted to divide by zero");                 \
                                                                    longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);                        \
                                                                  }                                                                               \
                                                                  output = left / right);                                                         \
    ZilchCaseBinaryRValue(WithType, WithType, Modulo,             if (GenericIsZero(right))                                                       \
                                                                  {                                                                               \
                                                                    state->ThrowException(report, "Attempted to modulo by zero");                 \
                                                                    longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);                        \
                                                                  }                                                                               \
                                                                  GenericMod(output, left, right));                                               \
    ZilchCaseBinaryRValue(WithType, WithType, Pow,                GenericPow(output, left, right));                                               \
    ZilchCaseBinaryLValue(WithType,           AssignmentAdd,      output += right);                                                               \
    ZilchCaseBinaryLValue(WithType,           AssignmentSubtract, output -= right);                                                               \
    ZilchCaseBinaryLValue(WithType,           AssignmentMultiply, output *= right);                                                               \
    ZilchCaseBinaryLValue(WithType,           AssignmentDivide,   if (GenericIsZero(right))                                                       \
                                                                  {                                                                               \
                                                                    state->ThrowException(report, "Attempted to divide by zero");                 \
                                                                    longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);                        \
                                                                  }                                                                               \
                                                                  output /= right);                                                               \
    ZilchCaseBinaryLValue(WithType,           AssignmentModulo,   if (GenericIsZero(right))                                                       \
                                                                  {                                                                               \
                                                                    state->ThrowException(report, "Attempted to modulo by zero");                 \
                                                                    longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);                        \
                                                                  }                                                                               \
                                                                  GenericMod(output, output, right));                                             \
    ZilchCaseBinaryLValue(WithType,           AssignmentPow,      GenericPow(output, output, right));

  // Generic numeric operators, copy, equality, comparison
  #define ZilchScalarCases(WithType)                                                                                                              \
    ZilchNumericCases(WithType, Boolean)                                                                                                          \
    ZilchComparisonCases(WithType, Boolean)

  // Vector operations, generic numeric operators, copy, equality
  #define ZilchVectorCases(VectorType, ScalarType, ComparisonType)                                                                                \
    ZilchNumericCases(VectorType, Boolean)                                                                                                        \
    ZilchComparisonCases(VectorType, ComparisonType)                                                                                              \
    ZilchCaseBinaryRValue2(VectorType, ScalarType, VectorType,  ScalarMultiply,           output = left * right);                                 \
    ZilchCaseBinaryRValue2(VectorType, ScalarType, VectorType,  ScalarDivide,             if (GenericIsZero(right))                               \
                                                                                          {                                                       \
                                                                                            state->ThrowException(report,                         \
                                                                                              "Attempted to divide by zero");                     \
                                                                                            longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);\
                                                                                          }                                                       \
                                                                                          output = left / right);                                 \
    ZilchCaseBinaryRValue2(VectorType, ScalarType, VectorType,  ScalarModulo,             if (GenericIsZero(right))                               \
                                                                                          {                                                       \
                                                                                            state->ThrowException(report,                         \
                                                                                              "Attempted to modulo by zero");                     \
                                                                                            longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);\
                                                                                          }                                                       \
                                                                                          GenericScalarMod(output, left, right));                 \
    ZilchCaseBinaryRValue2(VectorType, ScalarType, VectorType,  ScalarPow,                GenericScalarPow(output, left, right));                 \
    ZilchCaseBinaryLValue2(VectorType, ScalarType,              AssignmentScalarMultiply, output *= right);                                       \
    ZilchCaseBinaryLValue2(VectorType, ScalarType,              AssignmentScalarDivide,   if (GenericIsZero(right))                               \
                                                                                          {                                                       \
                                                                                            state->ThrowException(report,                         \
                                                                                              "Attempted to divide by zero");                     \
                                                                                            longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);\
                                                                                          }                                                       \
                                                                                          output /= right);                                       \
    ZilchCaseBinaryLValue2(VectorType, ScalarType,              AssignmentScalarModulo,   if (GenericIsZero(right))                               \
                                                                                          {                                                       \
                                                                                            state->ThrowException(report,                         \
                                                                                              "Attempted to modulo by zero");                     \
                                                                                            longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);\
                                                                                          }                                                       \
                                                                                          GenericScalarMod(output, output, right));               \
    ZilchCaseBinaryLValue2(VectorType, ScalarType,              AssignmentScalarPow,      GenericScalarPow(output, output, right));

  // Special integral operators, generic numeric operators, copy, equality, and comparison
  #define ZilchIntegralCases(WithType)                                                                                                            \
    ZilchCaseUnaryRValue (WithType, WithType, BitwiseNot,               output = ~operand);                                                       \
    ZilchCaseBinaryRValue(WithType, WithType, BitshiftLeft,             output = left << right);                                                  \
    ZilchCaseBinaryRValue(WithType, WithType, BitshiftRight,            output = left >> right);                                                  \
    ZilchCaseBinaryRValue(WithType, WithType, BitwiseOr,                output = left | right);                                                   \
    ZilchCaseBinaryRValue(WithType, WithType, BitwiseXor,               output = left ^ right);                                                   \
    ZilchCaseBinaryRValue(WithType, WithType, BitwiseAnd,               output = left & right);                                                   \
    ZilchCaseBinaryLValue(WithType,           AssignmentBitshiftLeft,   output <<= right);                                                        \
    ZilchCaseBinaryLValue(WithType,           AssignmentBitshiftRight,  output >>= right);                                                        \
    ZilchCaseBinaryLValue(WithType,           AssignmentBitwiseOr,      output |= right);                                                         \
    ZilchCaseBinaryLValue(WithType,           AssignmentBitwiseXor,     output ^= right);                                                         \
    ZilchCaseBinaryLValue(WithType,           AssignmentBitwiseAnd,     output &= right);

  //***************************************************************************
  ZilchVirtualInstruction(InternalDebugBreakpoint)
  {
    // Trigger the breakpoint
    ZilchDebugBreak();

    // Move the instruction counter past this opcode
    programCounter += sizeof(Opcode);
  }
  
  //***************************************************************************
  ZilchVirtualInstruction(BeginTimeout)
  {
    // Grab the rest of the data
    const TimeoutOpcode& op = (const TimeoutOpcode&) opcode;

    // Let the executable state know we've started a timeout
    // This can technically throw if a timeout above it expired
    if (state->PushTimeout(ourFrame, op.LengthSeconds))
    {
      // Jump out so we don't run any more code
      longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);
    }

    // Move the instruction counter past this opcode
    programCounter += sizeof(TimeoutOpcode);
  }


  //***************************************************************************
  ZilchVirtualInstruction(EndTimeout)
  {
    // Validate the timeout (this will throw an exception if we go beyond the time we need to)
    // This only really needs to be ran in jumps
    if (state->PopTimeout(ourFrame))
    {
      // Jump out so we don't run any more code
      longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);
    }

    // Move the instruction counter past this opcode
    programCounter += sizeof(Opcode);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(BeginScope)
  {
    // Store a pointer to the newly created (or recycled) scope
    PerScopeData* newScope = state->AllocateScope();

    // Add a new scope so that we can destruct our scope properly
    ourFrame->Scopes.PushBack(newScope);

    // Move the instruction counter past this opcode
    programCounter += sizeof(Opcode);
    return;
  }
  
  //***************************************************************************
  ZilchVirtualInstruction(EndScope)
  {
    // Get the latest scope
    PerScopeData* scope = ourFrame->Scopes.Back();

    // Cleanup any handles and delegates (this also clears for recycling)
    scope->PerformCleanup();

    // Pop this scope since we're exiting it
    ourFrame->Scopes.PopBack();

    // Instead of deleting this scope, we're going to recycle it for speed
    state->RecycledScopes.PushBack(scope);

    // Move the instruction counter past this opcode
    programCounter += sizeof(Opcode);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(ToHandle)
  {
    // Grab the rest of the data
    const ToHandleOpcode& op = (const ToHandleOpcode&) opcode;

    // If the object we're converting into a handle is a local...
    if (op.ToHandle.Type == OperandType::Local)
    {
      // Get the handle that we're going to write to on the stack
      Handle& handle = *new (ourFrame->Frame + op.SaveLocal) Handle();

      // We need to make sure we cleanup this handle
      ourFrame->QueueHandleCleanup(&handle);

      // Initialize the stack handle to point at the given location
      state->InitializeStackHandle(handle, ourFrame->Frame + op.ToHandle.HandleConstantLocal, ourFrame->Scopes.Back(), op.Type);
    }
    else if (op.ToHandle.Type == OperandType::StaticField)
    {
      // Get the handle that we're going to write to on the stack
      Handle& handle = *new (ourFrame->Frame + op.SaveLocal) Handle();

      // We need to make sure we cleanup this handle
      ourFrame->QueueHandleCleanup(&handle);

      // Initialize the stack handle to point at the given location
      byte* fieldPointer = &GetOperand<byte>(ourFrame, ourFrame, op.ToHandle);
      state->InitializePointerHandle(handle, fieldPointer, op.Type);
    }
    else
    {
      // We assume this means we're taking a handle to a field, which should have been copied to the stack
      ErrorIf(op.ToHandle.Type != OperandType::Field,
        "We can only take handles to locals and fields (not constants, for example)");

      // Get the handle that holds the base of the member we're going to make a handle to
      const Handle& baseHandle = *(Handle*)(ourFrame->Frame + op.ToHandle.HandleConstantLocal);

      // Make a copy from the base handle into our handle
      // This should properly handle incrementing a reference, if needed
      Handle& handle = *new (ourFrame->Frame + op.SaveLocal) Handle(baseHandle);

      // We need to make sure we cleanup this handle
      ourFrame->QueueHandleCleanup(&handle);

      // Add the offset to the handle
      handle.Offset += op.ToHandle.FieldOffset;

      // The type we're now referring to is whatever the field is at that offset
      handle.StoredType = op.Type;
    }

    // Move the instruction counter past this opcode
    programCounter += sizeof(op);

    //TODO the rest of the handle initialization
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(CreateStaticDelegate)
  {
    // Grab the rest of the data
    const CreateStaticDelegateOpcode& op = (const CreateStaticDelegateOpcode&) opcode;

    // Get the delegate that we're going to write to on the stack
    Delegate& delegate = *new (ourFrame->Frame + op.SaveLocal) Delegate();

    // Set the delegate's function index to the opcode's index
    delegate.BoundFunction = op.BoundFunction;

    // Even though the 'this' handle is null, we still need to clean it up
    // just to be proper, but also because someone could theoretically Assign to it
    // if we expose it in the future
    ourFrame->QueueDelegateCleanup(&delegate);

    // Move the instruction counter past this opcode
    programCounter += sizeof(op);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(CreateInstanceDelegate)
  {
    // Grab the rest of the data
    const CreateInstanceDelegateOpcode& op = (const CreateInstanceDelegateOpcode&) opcode;

    // Get the delegate that we're going to write to on the stack
    Delegate& delegate = *new (ourFrame->Frame + op.SaveLocal) Delegate();

    // Set the delegate's function index to the opcode's index
    delegate.BoundFunction = op.BoundFunction;

    // Set the handle that will basically act as the 'this' pointer for the delegate
    Handle& thisHandle = GetOperand<Handle>(ourFrame, ourFrame, op.ThisHandle);
    delegate.ThisHandle = thisHandle;

    // If the function we're binding is virtual and we're not calling this function 'non-virtually'
    if (op.BoundFunction->IsVirtual && op.CanBeVirtual && thisHandle.StoredType != nullptr)
    {
      // Find the function on our derived type that matches the signature / name
      Function* function = thisHandle.StoredType->FindFunction(op.BoundFunction->Name, op.BoundFunction->FunctionType, FindMemberOptions::None);
      if (function != nullptr)
        delegate.BoundFunction = function;
      else
        Error("Unable to find the most derived virtual function, we can continue but this should not happen");
    }

    // We need to make sure we cleanup this handle
    ourFrame->QueueDelegateCleanup(&delegate);

    // Move the instruction counter past this opcode
    programCounter += sizeof(op);
    return;
  }
  
  //***************************************************************************
  ZilchVirtualInstruction(IfFalseRelativeGoTo)
  {
    IfHandler<false>(ourFrame, opcode);
    return;
  }
  
  //***************************************************************************
  ZilchVirtualInstruction(IfTrueRelativeGoTo)
  {
    IfHandler<true>(ourFrame, opcode);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(RelativeGoTo)
  {
    // Validate the timeout (this will throw an exception if we go beyond the time we need to)
    // This only really needs to be ran in jumps
    if (state->ThrowExceptionOnTimeout(report))
    {
      // Jump out so we don't run any more code
      longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);
    }

    // Grab the rest of the data
    const RelativeJumpOpcode& op = (const RelativeJumpOpcode&) opcode;

    // Move the instruction counter by the given offset
    programCounter += op.JumpOffset;
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(Return)
  {
  }

  //***************************************************************************
  ZilchVirtualInstruction(PrepForFunctionCall)
  {
    // Validate the timeout (this will throw an exception if we go beyond the time we need to)
    // This only really needs to be ran in jumps
    if (state->ThrowExceptionOnTimeout(report))
    {
      // Jump out so we don't run any more code
      longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);
    }

    // Grab the rest of the data
    const PrepForFunctionCallOpcode& op = (const PrepForFunctionCallOpcode&) opcode;

    // Retrieve the delegate that we have to invoke
    const Delegate& delegate = GetOperand<Delegate>(ourFrame, ourFrame, op.Delegate);

    // Lookup the function in the function array
    Function* functionToInvoke = delegate.BoundFunction;

    // If we attempted to invoke a null delegate then throw an exception
    if (functionToInvoke == nullptr)
    {
      // Throw an exception and bail out
      state->ThrowException(report, "Attempted to invoke a null delegate");
      longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);
    }

    // Get the frame at the top of the stack (it could be ours)
    PerFrameData* topFrame = state->StackFrames.Back();

    // Create a new stack frame for our function
    PerFrameData* newFrame = state->PushFrame(topFrame->NextFrame, functionToInvoke);

    // If the stack frame was created in an error state, then attempt to throw exceptions
    // If this returns true (meaning exceptions were thrown) then we'll jump out
    if (newFrame->AttemptThrowStackExceptions(report))
    {
      longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);
    }

    // Check if the "to be invoked" function is a static function (should we skip the next copy opcode?)
    if (functionToInvoke->IsStatic)
    {
      // Skip copying the 'this' handle (and move past this opcode)
      programCounter += op.JumpOffsetIfStatic;
    }
    else
    {
      // Increment the program counter to point past the opcode
      programCounter += sizeof(PrepForFunctionCallOpcode);
    }
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(FunctionCall)
  {
    // Grab the per frame data from the executable state
    PerFrameData* topFrame = state->StackFrames.Back();
    
    // Create a call (this is not a user call, so it should not push a stack frame)
    // Moreover, none of the debug features should be enabled
    Call subCall(topFrame);

    // Invoke the call (calls the bound function with all the parameters)
    subCall.Invoke(report);

    // Check to see if we threw any exceptions in the above invokation
    if (report.HasThrownExceptions())
    {
      longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);
    }

    // Increment the program counter to point past the opcode
    programCounter += sizeof(Opcode);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(LocalObject)
  {
    // Grab the rest of the data
    const CreateLocalTypeOpcode& op = (const CreateLocalTypeOpcode&) opcode;

    // Get the type that we're creating
    BoundType* createdType = op.CreatedType;

    // Allocate the object
    Handle handle = state->AllocateStackObject(ourFrame->Frame + op.StackLocal, ourFrame->Scopes.Back(), createdType, report);

    // If allocating the stack object threw an exception...
    if (report.HasThrownExceptions())
    {
      longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);
    }

    // Copy the handle to the stack
    Handle* handleOnStack = new (ourFrame->Frame + op.SaveHandleLocal) Handle(handle);

    // We need to make sure we cleanup this handle
    ourFrame->QueueHandleCleanup(handleOnStack);

    // Increment the program counter to point past the opcode
    programCounter += sizeof(CreateLocalTypeOpcode);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(NewObject)
  {
    // Grab the rest of the data
    const CreateTypeOpcode& op = (const CreateTypeOpcode&) opcode;

    // Get the type that we're creating
    BoundType* createdType = op.CreatedType;

    // Allocate the object
    Handle handle = state->AllocateHeapObject(createdType, report, HeapFlags::ReferenceCounted);

    // If allocating the stack object threw an exception...
    if (report.HasThrownExceptions())
    {
      longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);
    }

    // Copy the handle to the stack
    Handle* handleOnStack = new (ourFrame->Frame + op.SaveHandleLocal) Handle(handle);
    
    // We need to make sure we cleanup this handle
    ourFrame->QueueHandleCleanup(handleOnStack);

    // Increment the program counter to point past the opcode
    programCounter += sizeof(CreateTypeOpcode);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(PropertyDelegate)
  {
    // Grab the rest of the data
    const CreatePropertyDelegateOpcode& op = (const CreatePropertyDelegateOpcode&) opcode;

    // Get the type that we're creating
    BoundType* createdType = op.CreatedType;

    // Allocate the object
    Handle handle = state->AllocateDefaultConstructedHeapObject(createdType, report, HeapFlags::ReferenceCounted);

    // If allocating the stack object threw an exception...
    if (report.HasThrownExceptions())
    {
      longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);
    }

    // Copy the handle to the stack
    Handle* handleOnStack = new (ourFrame->Frame + op.SaveHandleLocal) Handle(handle);
    
    // We need to make sure we cleanup this handle
    ourFrame->QueueHandleCleanup(handleOnStack);

    // As long as nothing failed, the object we allocated should be a 'PropertyDelegate' object
    PropertyDelegateTemplate* propertyDelegate = (PropertyDelegateTemplate*)handleOnStack->Dereference();

    // Set the delegate's function index to the opcode's index
    propertyDelegate->Get.BoundFunction = op.ReferencedProperty->Get;
    propertyDelegate->Set.BoundFunction = op.ReferencedProperty->Set;
    propertyDelegate->ReferencedProperty = Handle((const byte*)op.ReferencedProperty, ZilchTypeId(Property), nullptr, state);

    // If this is a static property, then we don't have a this handle
    if (!op.ReferencedProperty->IsStatic)
    {
      // Read the this handle from the stack
      Handle& thisHandle = *(Handle*)(ourFrame->Frame + op.ThisHandleLocal);
      propertyDelegate->Get.ThisHandle = thisHandle;
      propertyDelegate->Set.ThisHandle = thisHandle;
    }

    // Increment the program counter to point past the opcode
    programCounter += sizeof(CreatePropertyDelegateOpcode);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(BeginStringBuilder)
  {
    // Create a new string builder
    state->StringBuilders.PushBack();

    // Increment the program counter to point past the opcode
    programCounter += sizeof(BeginStringBuilderOpcode);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(EndStringBuilder)
  {
    // Grab the rest of the data
    const EndStringBuilderOpcode& op = (const EndStringBuilderOpcode&) opcode;

    // Get the string builder that we're using
    StringBuilder& builder = state->StringBuilders.Back();

    // Convert the concatenations into a single reference counted string
    String result = builder.ToString();

    // We're done with the string builder!
    state->StringBuilders.PopBack();

    // Copy the handle to the stack
    Handle* handle = new (ourFrame->Frame + op.SaveStringHandleLocal) Handle((byte*)&result, ZilchTypeId(String));
    
    // We need to make sure we cleanup this handle
    ourFrame->QueueHandleCleanup(handle);

    // Increment the program counter to point past the opcode
    programCounter += sizeof(EndStringBuilderOpcode);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(AddToStringBuilder)
  {
    // Grab the rest of the data
    const AddToStringBuilderOpcode& op = (const AddToStringBuilderOpcode&) opcode;

    // Get the string builder that we're using
    StringBuilder& builder = state->StringBuilders.Back();

    // Generically get the operand that we want to turn into a string
    byte* data = &GetOperand<byte>(ourFrame, ourFrame, op.Value);

    // Convert the value to a string in the best way we know how to
    String stringifiedValue = op.TypeToConvert->GenericToString(data);

    // Append the stringified value to the string builder
    builder.Append(stringifiedValue);

    // Increment the program counter to point past the opcode
    programCounter += sizeof(AddToStringBuilderOpcode);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(DeleteObject)
  {
    // Grab the rest of the data
    const DeleteObjectOpcode& op = (const DeleteObjectOpcode&) opcode;

    // Get a pointer to the handle we are deleting
    Handle& handle = GetOperand<Handle>(ourFrame, ourFrame, op.Object);
    
    // Delete the handle
    bool result = handle.Delete();

    // If we failed to delete the handle...
    if (result == false)
    {
      // The user handle type could not be deleted!
      state->ThrowException
      (
        report,
        String::Format("We attempted to delete a '%s' handle, but we aren't allowed to", handle.Manager->GetName().c_str())
      );

      // Jump back since we just threw an exception
      longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);
    }

    // Increment the program counter to point past the opcode
    programCounter += sizeof(DeleteObjectOpcode);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(ThrowException)
  {
    // Grab the rest of the data
    const ThrowExceptionOpcode& op = (const ThrowExceptionOpcode&) opcode;

    // Get a pointer to the handle we are deleting
    Handle& handle = GetOperand<Handle>(ourFrame, ourFrame, op.Exception);

    // If the handle itself is null, we actually throw a null reference exception!
    if (handle.IsNull())
    {
      state->ThrowNullReferenceException(report);
    }
    else
    {
      state->ThrowException(report, handle);
    }

    // Jump back since we just threw an exception
    longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);
  }

  //***************************************************************************
  ZilchVirtualInstruction(TypeId)
  {
    const TypeIdOpcode& op = (const TypeIdOpcode&) opcode;

    // This function will access the primitive and attempt to get the most derived type from it
    // For the 'any' type, this will get the type stored inside
    // Note: We MUST be sure that the Syntaxer set the actual expresion result type to the correct type
    // eg: any -> Type (because it could be anything!), bound types -> BoundType, delegates -> DelegateType
    byte* expressionResult = &GetOperand<byte>(ourFrame, ourFrame, op.Expression);
    const Type* virtualType = op.CompileTimeType->GenericGetVirtualType(expressionResult);

    // This may not be necessary, but just in case we don't get a valid type returned, assume its the compile time type
    if (virtualType == nullptr)
      virtualType = op.CompileTimeType;

    // Create a handle in constant space for the type pointer
    byte* handlePointer = &GetLocal<byte>(ourFrame->Frame, op.SaveTypeHandleLocal);
    *new (handlePointer) Handle((byte*)virtualType, ZilchVirtualTypeId(virtualType));

    programCounter += sizeof(TypeIdOpcode);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(ConvertToAny)
  {
    // Grab the rest of the data
    const AnyConversionOpcode& op = (const AnyConversionOpcode&) opcode;
    const byte* value = &GetOperand<byte>(ourFrame, ourFrame, op.ToConvert);

    // Grab the bytes that will hold the 'Any' value
    byte* anyData = &GetLocal<byte>(ourFrame->Frame, op.Output);

    // Construct the any at the given position (this will do a proper copy of data into the any)
    Any* any = new (anyData) Any(value, op.RelatedType);
    
    // We need to make sure we cleanup this 'any'
    ourFrame->QueueAnyCleanup(any);
    programCounter += sizeof(AnyConversionOpcode);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(ConvertFromAny)
  {
    // Grab the rest of the data
    const AnyConversionOpcode& op = (const AnyConversionOpcode&) opcode;
    const Any& any = GetOperand<Any>(ourFrame, ourFrame, op.ToConvert);

    // If we can't directly convert the any into this type...
    Shared& shared = Shared::GetInstance();
    CastOperator cast = shared.GetCastOperator(any.StoredType, op.RelatedType);
    if (cast.IsValid == false || cast.RequiresCodeGeneration)
    {
      // Generate an error string that gives a lot of context clues
      String error = String::Format
      (
        "The 'any' value '%s' of type '%s' cannot be converted to a '%s'. The type must match exactly or be directly convertable",
        any.StoredType->GenericToString(any.Data).c_str(),
        any.StoredType->ToString().c_str(),
        op.RelatedType->ToString().c_str()
      );

      // Throw an exception to let the user know the conversion was invalid
      state->ThrowException(report, error);

      // Jump back since we just threw an exception
      longjmp(ourFrame->ExceptionJump, ExceptionJumpResult);
    }

    // Grab the bytes that will hold the value we copy from the Any
    byte* outputData = &GetLocal<byte>(ourFrame->Frame, op.Output);

    // Copy the value out of the any onto the stack
    any.StoredType->GenericCopyConstruct(outputData, any.Data);
    if (any.StoredType->IsHandle())
      ourFrame->QueueHandleCleanup((Handle*)outputData);
    else if (any.StoredType->IsDelegate())
      ourFrame->QueueDelegateCleanup((Delegate*)outputData);
    // This last case should never happen currently (because we can't put an Any within an Any)
    else if (any.StoredType->IsAny())
      ourFrame->QueueAnyCleanup((Any*)outputData);

    programCounter += sizeof(AnyConversionOpcode);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(ConvertDowncast)
  {
    // Grab the rest of the data
    const DowncastConversionOpcode& op = (const DowncastConversionOpcode&) opcode;
    const Handle& toConvert = GetOperand<Handle>(ourFrame, ourFrame, op.ToConvert);

    // Grab the output that we want to initialize either to a casted handle, or to null
    byte* outputHandle = &GetLocal<byte>(ourFrame->Frame, op.Output);

    // The value we were passed in was null, so just output null
    if (toConvert.IsNull())
    {
      // Initialize the temporary handle to null
      new (outputHandle) Handle();
    }
    // Check to see if this type inherits from the other type
    else if (Type::GenericIsA(op.ToType, toConvert.StoredType))
    {
      // Copy the handle over, it's now the down-casted type!
      new (outputHandle) Handle(toConvert);
    }
    else
    {
      // The downcast failed (not the same) so return null
      new (outputHandle) Handle();
    }

    ourFrame->QueueHandleCleanup((Handle*)outputHandle);
    programCounter += sizeof(DowncastConversionOpcode);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(AnyDynamicMemberGet)
  {
    Error("Not implemented");
  }

  //***************************************************************************
  ZilchVirtualInstruction(AnyDynamicMemberSet)
  {
    Error("Not implemented");
  }

  //***************************************************************************
  ZilchVirtualInstruction(InvalidInstruction)
  {
    Error("This case should never be hit");
  }

  //***************************************************************************
  ZilchVirtualInstruction(ConvertStringToStringRangeExtended)
  {
    const ConversionOpcode& op = (const ConversionOpcode&) opcode;

    const Handle& fromTypeHandle = GetOperand<Handle>(ourFrame, ourFrame, op.ToConvert);
    const String* value = (String*)fromTypeHandle.Dereference();

    // Get the toType's memory
    byte* toTypeMemory = &GetLocal<byte>(ourFrame->Frame, op.Output);
    
    // Deal with null strings
    if (value == nullptr)
    {
      new (toTypeMemory) Handle();
    }
    else
    {
      // Construct a new handle for the string range
      Handle rangeHandle = call.GetState()->AllocateDefaultConstructedHeapObject(ZilchTypeId(StringRangeExtended), report, HeapFlags::ReferenceCounted);
      StringRangeExtended& stringRange = *(StringRangeExtended*)rangeHandle.Dereference();

      stringRange.mRange = value->All();
      stringRange.mOriginalStringReference = *value;
      // Copy the handle over the output data
      Handle* stackHandle = new (toTypeMemory) Handle(rangeHandle);
      ourFrame->QueueHandleCleanup(stackHandle);
    }

    programCounter += sizeof(ConversionOpcode);
  }

  //***************************************************************************
  ZilchVirtualInstruction(CopyValue)
  {
    // Grab the rest of the data
    const CopyOpcode& op = (const CopyOpcode&) opcode;

    const byte* source;
    byte* destination;

    // Get the frame at the top of the stack (it could be ours)
    PerFrameData* topFrame = state->StackFrames.Back();

    CopyHandlerEx(ourFrame, topFrame, source, destination, op);

    memcpy(destination, source, op.Size);
    return;
  }

  //***************************************************************************
  ZilchVirtualInstruction(TestEqualityValue)
  {
    const BinaryRValueOpcode& op = (const BinaryRValueOpcode&) opcode;
    const byte* left = &GetOperand<byte>(ourFrame, ourFrame, op.Left);
    const byte* right = &GetOperand<byte>(ourFrame, ourFrame, op.Right);
    Boolean& output = GetLocal<Boolean>(ourFrame->Frame, op.Output);
    output = (memcmp(left, right, op.Size) == 0);
    programCounter += sizeof(BinaryRValueOpcode);
  }

  //***************************************************************************
  ZilchVirtualInstruction(TestInequalityValue)
  {
    const BinaryRValueOpcode& op = (const BinaryRValueOpcode&) opcode;
    const byte* left = &GetOperand<byte>(ourFrame, ourFrame, op.Left);
    const byte* right = &GetOperand<byte>(ourFrame, ourFrame, op.Right);
    Boolean& output = GetLocal<Boolean>(ourFrame->Frame, op.Output);
    output = (memcmp(left, right, op.Size) != 0);
    programCounter += sizeof(BinaryRValueOpcode);
  }
    
  //***************************************************************************
  ZilchCaseComplexCopy(Handle);
  ZilchCaseComplexCopy(Delegate);
  ZilchCaseComplexCopy(Any);

  // Primitive type instructions
  ZilchIntegralCases(Byte)
  ZilchScalarCases(Byte)
  ZilchIntegralCases(Integer)
  ZilchScalarCases(Integer)
  ZilchVectorCases(Integer2, Integer, Boolean2)
  ZilchVectorCases(Integer3, Integer, Boolean3)
  ZilchVectorCases(Integer4, Integer, Boolean4)
  ZilchIntegralCases(Integer2)
  ZilchIntegralCases(Integer3)
  ZilchIntegralCases(Integer4)
  ZilchScalarCases(Real)
  ZilchVectorCases(Real2, Real, Boolean2)
  ZilchVectorCases(Real3, Real, Boolean3)
  ZilchVectorCases(Real4, Real, Boolean4)
  ZilchScalarCases(DoubleReal)
  ZilchIntegralCases(DoubleInteger)
  ZilchScalarCases(DoubleInteger)

  ZilchEqualityCases(Boolean, Boolean)
  ZilchEqualityCases(Handle, Boolean)
  ZilchEqualityCases(Delegate, Boolean)
  ZilchEqualityCases(Any, Boolean)

  ZilchCopyCases(Boolean)
  // Handle, Delegate, and Value copy (assignment) operators are handled specially above
        
  ZilchCaseUnaryRValue (Boolean, Boolean, LogicalNot, output = !operand);

  ZilchCaseConversion(Byte,           Real,           output = (Real)value);
  ZilchCaseConversion(Byte,           Boolean,        output = (value != 0));
  ZilchCaseConversion(Byte,           Integer,        output = (Integer)value);
  ZilchCaseConversion(Byte,           DoubleInteger,  output = (DoubleInteger)value);
  ZilchCaseConversion(Byte,           DoubleReal,     output = (DoubleReal)value);
  ZilchCaseConversion(Integer,        Real,           output = (Real)value);
  ZilchCaseConversion(Integer,        Boolean,        output = (value != 0));
  ZilchCaseConversion(Integer,        Byte,           output = (Byte)value);
  ZilchCaseConversion(Integer,        DoubleInteger,  output = (DoubleInteger)value);
  ZilchCaseConversion(Integer,        DoubleReal,     output = (DoubleReal)value);
  ZilchCaseConversion(Real,           Integer,        output = (Integer)value);
  ZilchCaseConversion(Real,           Boolean,        output = (value != 0));
  ZilchCaseConversion(Real,           Byte,           output = (Byte)value);
  ZilchCaseConversion(Real,           DoubleInteger,  output = (DoubleInteger)value);
  ZilchCaseConversion(Real,           DoubleReal,     output = (DoubleReal)value);
  ZilchCaseConversion(Boolean,        Integer,        output = (Integer)value);
  ZilchCaseConversion(Boolean,        Real,           output = (Real)value);
  ZilchCaseConversion(Boolean,        Byte,           output = (Byte)value);
  ZilchCaseConversion(Boolean,        DoubleInteger,  output = (DoubleInteger)value);
  ZilchCaseConversion(Boolean,        DoubleReal,     output = (DoubleReal)value);
  ZilchCaseConversion(DoubleInteger,  Real,           output = (Real)value);
  ZilchCaseConversion(DoubleInteger,  Boolean,        output = (value != 0));
  ZilchCaseConversion(DoubleInteger,  Byte,           output = (Byte)value);
  ZilchCaseConversion(DoubleInteger,  Integer,        output = (Integer)value);
  ZilchCaseConversion(DoubleInteger,  DoubleReal,     output = (DoubleReal)value);
  ZilchCaseConversion(DoubleReal,     Real,           output = (Real)value);
  ZilchCaseConversion(DoubleReal,     Boolean,        output = (value != 0));
  ZilchCaseConversion(DoubleReal,     Byte,           output = (Byte)value);
  ZilchCaseConversion(DoubleReal,     Integer,        output = (Integer)value);
  ZilchCaseConversion(DoubleReal,     DoubleInteger,  output = (DoubleInteger)value);

  ZilchCaseConversion(Integer2, Real2,     output = Real2((Real)value.x, (Real)value.y));
  ZilchCaseConversion(Integer2, Boolean2,  output = Boolean2(value.x != 0, value.y != 0));
  ZilchCaseConversion(Real2,    Integer2,  output = Integer2((Integer)value.x, (Integer)value.y));
  ZilchCaseConversion(Real2,    Boolean2,  output = Boolean2(value.x != 0.0f, value.y != 0.0f));
  ZilchCaseConversion(Boolean2, Integer2,  output = Integer2((Integer)value.x, (Integer)value.y));
  ZilchCaseConversion(Boolean2, Real2,     output = Real2((Real)value.x, (Real)value.y));
        
  ZilchCaseConversion(Integer3, Real3,     output = Real3((Real)value.x, (Real)value.y, (Real)value.z));
  ZilchCaseConversion(Integer3, Boolean3,  output = Boolean3(value.x != 0, value.y != 0, value.z != 0));
  ZilchCaseConversion(Real3,    Integer3,  output = Integer3((Integer)value.x, (Integer)value.y, (Integer)value.z));
  ZilchCaseConversion(Real3,    Boolean3,  output = Boolean3(value.x != 0.0f, value.y != 0.0f, value.z != 0.0f));
  ZilchCaseConversion(Boolean3, Integer3,  output = Integer3((Integer)value.x, (Integer)value.y, (Integer)value.z));
  ZilchCaseConversion(Boolean3, Real3,     output = Real3((Real)value.x, (Real)value.y, (Real)value.z));
        
  ZilchCaseConversion(Integer4, Real4,     output = Real4((Real)value.x, (Real)value.y, (Real)value.z, (Real)value.w));
  ZilchCaseConversion(Integer4, Boolean4,  output = Boolean4(value.x != 0, value.y != 0, value.z != 0, value.w != 0));
  ZilchCaseConversion(Real4,    Integer4,  output = Integer4((Integer)value.x, (Integer)value.y, (Integer)value.z, (Integer)value.w));
  ZilchCaseConversion(Real4,    Boolean4,  output = Boolean4(value.x != 0.0f, value.y != 0.0f, value.z != 0.0f, value.w != 0.0f));
  ZilchCaseConversion(Boolean4, Integer4,  output = Integer4((Integer)value.x, (Integer)value.y, (Integer)value.z, (Integer)value.w));
  ZilchCaseConversion(Boolean4, Real4,     output = Real4((Real)value.x, (Real)value.y, (Real)value.z, (Real)value.w));
  
  //***************************************************************************
  void VirtualMachine::InitializeJumpTable()
  {
    #define ZilchEnumValue(Name) InstructionTable[Instruction::Name] = &Instruction##Name;
    #include "InstructionsEnum.inl"
    #undef ZilchEnumValue
  }

  //***************************************************************************
  void VirtualMachine::ExecuteNext(Call& call, ExceptionReport& report)
  {
    // Since we do a raw copy, we always tell the caller to ignore debug checking of the return
    call.DisableReturnChecks();

    // Grab the executable state
    ExecutableState* state = call.GetState();

    // Block vectors are currently only POD
    // Grab the per frame data from the executable state
    PerFrameData* ourFrame = state->StackFrames.Back();

    // The program counter increments with each instruction (instructions can also be variable width, and this number is measured in bytes)
    ourFrame->ProgramCounter = 0;
    size_t& programCounter = ourFrame->ProgramCounter;

    // Setup where we jump to if an exception occurs
    int jumpResult = setjmp(ourFrame->ExceptionJump);

    // If an exception occurred...
    if (jumpResult == ExceptionJumpResult)
      return;

    // Store the compacted opcode as an attempt to bring the opcode into crash reports / mini-dumps
    // Also save it into a non-thread safe global pointer (hopefully it will be pulled in)
    byte* compactedOpcode = ourFrame->CurrentFunction->CompactedOpcode.Data();
    ZilchLastRunningOpcode = compactedOpcode;
    ZilchLastRunningFunction = ourFrame->CurrentFunction;
    ZilchLastRunningOpcodeLength = ourFrame->CurrentFunction->CompactedOpcode.Size();

    // Loop through all the opcodes in the function
    // We don't need to check for the end since the return opcode will exit this function
    ZilchLoop
    {
      // Grab the current opcode that we're executing
      const Opcode& opcode = *(Opcode*)(compactedOpcode + programCounter);

      // If any pre opcode callbacks are set then send the event
      state->SendOpcodeEvent(Events::OpcodePreStep, ourFrame);
      InstructionTable[opcode.Instruction](state, call, report, programCounter, ourFrame, opcode);
      
      // If any post opcode callbacks are set then send the event
      state->SendOpcodeEvent(Events::OpcodePostStep, ourFrame);
      if (opcode.Instruction == Instruction::Return)
        return;
    }
  }

  //***************************************************************************
  void VirtualMachine::PostDestructor(BoundType* boundType, byte* objectData)
  {
    // Loop through all the handles we want to destroy
    for (size_t i = 0; i < boundType->Handles.Size(); ++i)
    {
      // Get the index that the primitive is at
      size_t index = boundType->Handles[i];

      // Destroy the primitive (which may decrement a reference)
      Handle& toDestroy = *(Handle*)(objectData + index);
      toDestroy.~Handle();
    }

    // Loop through all the delegates we want to destroy
    for (size_t i = 0; i < boundType->Delegates.Size(); ++i)
    {
      // Get the index that the primitive is at
      size_t index = boundType->Delegates[i];

      // Destroy the primitive (which may decrement a reference)
      Delegate& toDestroy = *(Delegate*)(objectData + index);
      toDestroy.~Delegate();
    }
  }

  //***************************************************************************
  String VirtualMachine::UnknownEnumerationToString(const BoundType* type, const byte* data)
  {
    Integer inputValue = *(const Integer*)data;
    return String::Format("%s (%d)", type->Name.c_str(), inputValue);
  }

  //***************************************************************************
  String VirtualMachine::EnumerationToString(const BoundType* type, const byte* data)
  {
    // Get the value of the enum
    Integer inputValue = *(const Integer*)data;

    // Loop through all the properties this type defines...
    const PropertyArray& properties = type->AllProperties;
    for (size_t i = 0; i < properties.Size(); ++i)
    {
      // Grab the current property
      Property* property = properties[i];

      // Error checking
      ErrorIf(property->Get == nullptr, "The enum should have no properties that do not have a 'get' function");
      ErrorIf(property->IsStatic == false, "All properties on the enum should be static");

      // If this property's UserData matches our enum's value...
      if (property->Get->UserData == (void*)inputValue)
      {
        return property->Name;
      }
    }

    // Otherwise, this enum is not a known value... (just return the integer for readability)
    return UnknownEnumerationToString(type, data);
  }

  //***************************************************************************
  String VirtualMachine::FlagsToString(const BoundType* type, const byte* data)
  {
    // Get the value of the enum
    Integer inputValue = *(const Integer*)data;

    // With every bit we loop through, we mask the bit off
    // At the end, this value should be 0 unless there were invalid bits that we didn't know what they were
    Integer runningValue = inputValue;

    // Create a string builder so we can concatenate all the flags together
    StringBuilder builder;

    // If we wrote anything to the string buffer, then the next time we need to add the '|'
    bool wroteSomething = false;

    // Loop through all the properties this type defines...
    const PropertyArray& properties = type->AllProperties;
    for (size_t i = 0; i < properties.Size(); ++i)
    {
      // Grab the current property
      Property* property = properties[i];

      // Error checking
      ErrorIf(property->Get == nullptr, "The flags should have no properties that do not have a 'get' function");
      ErrorIf(property->IsStatic == false, "All properties on the flags should be static");

      // Grab the value of this flag (may be multiple bits!)
      Integer flagValue = (Integer)(DoubleInteger)property->Get->UserData;

      // If we have a flag value of 0 (typically None) then ignore it
      // Otherwise every bitfield when printed would also have None set
      if (flagValue == 0)
        continue;

      // Mask off the bits so we know if there were any invalid bits leftover
      runningValue &= ~flagValue;

      // If the input value Contains all these flags...
      if ((inputValue & flagValue) == flagValue)
      {
        // If we've already written anything, we need to add the delimiter (the bitwise OR operator)
        if (wroteSomething)
        {
          builder.Append(" ");
          builder.Append(Grammar::GetKeywordOrSymbol(Grammar::BitwiseOr));
          builder.Append(" ");
        }

        // Append the property name, and note that we've written something now
        // (meaning the next write needs a delimiter)
        builder.Append(property->Name);
        wroteSomething = true;
      }
    }

    // If nothing was written...
    if (wroteSomething == false)
    {
      // Otherwise, this flags is not a known value... (just return the integer for readability)
      return UnknownEnumerationToString(type, data);
    }
    else
    {
      // If we have any bits left that weren't accounted for...
      if (runningValue != 0)
      {
        // Show the user the true value
        builder.Append(" (");
        builder.Append(IntegerToString(inputValue));
        builder.Append(")");
      }

      // Return the bits that were set
      return builder.ToString();
    }
  }
}

//***************************************************************************
// ONLY FOR DEBUGGING CRASH DUMPS
byte* ZilchLastRunningOpcode = nullptr;
Zilch::Function* ZilchLastRunningFunction = nullptr;
size_t ZilchLastRunningOpcodeLength = 0;
