/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_THREE_ADDRESS_OPCODE_HPP
#define ZILCH_THREE_ADDRESS_OPCODE_HPP

namespace Zilch
{
  // Forward declaration
  class SyntaxTree;

  // The actual instructions that we can execute
  namespace Instruction
  {
    typedef int AlignedEnum;

    enum Enum
    {
      #define ZilchEnumValue(value) value,
      #include "InstructionsEnum.inl"
      #undef ZilchEnumValue
      Count
    };

    // The names of the instructions (for reflection and debugging)
    extern const char* Names[];
  }

  namespace DebugOrigin
  {
    enum Enum
    {
      MemberVariable,
      LocalVariable,
      Scope,
      Timeout,
      If,
      While,
      For,
      DoWhile,
      Loop,
      Break,
      Continue,
      PropertyDelegate,
      DebugBreak,
      BinaryOperation,
      UnaryOperation,
      DataMemberAccess,
      TypeCast,
      ReturnValue,
      FunctionCall,
      FunctionContext,
      DeleteObject,
      ThrowException,
      NewObject,
      LocalObject,
      TypeId,
      FunctionMemberAccess,
      PropertyGetMemberAccess,
      PropertySetMemberAccess,
      FunctionCallConstructor,
      StringInterpolant
    };
  }

  // This is the form that the intermediate three-address opcode will take
  // An opcode is basically a single instruction with its operand (like an assembly command)
  class ZeroShared Opcode
  {
  public:
    // Constructor (gets rid of the annoying warning about POD constructors)
    Opcode() {}

    // The instruction to be generated
    Instruction::AlignedEnum Instruction;

#ifdef ZeroDebug
    // Make the class virtual in debug mode so that we can view a list of opcodes easily
    // Having a vtable in visual studio also gives us a reflected view of the derived class
    virtual ~Opcode() { }
    
    // The origin of the instruction for debugging purposes (who created it)
    DebugOrigin::Enum DebugOrigin;
#endif
  };

  // The type of an operand
  // This generally tells us how we read/write to a particular expression
  // For example, if we're going to a property, we must call set, whereas
  // a local variable we can just write to directly on the stack
  namespace OperandType
  {
    enum Enum
    {
      NotSet,       // If the access type was not set, we know we have a bug
      Constant,     // The value is read only and cannot be assigned to
      Local,        // The value is a local variable and can be assigned to
      Field,        // The value is a member variable (field) and can be assigned to
      StaticField,  // Static values are looked up in a map by the Field*
      Property      // The value is a property and uses get/set (not really used)
    };
  }

  // An operand is used inside of an opcode,
  // for when it needs to refer to any bit of memory
  class ZeroShared Operand
  {
  public:
    // Default constructor
    Operand();

    // Construct from an index on the current stack
    explicit Operand(OperandIndex local);

    // Construct from a primary index, secondary index, and an access type
    Operand(OperandIndex handleConstantLocal, size_t field, OperandType::Enum type);

    // What type of operand are we trying to access?
    OperandType::Enum Type;

    union
    {
      // An offset to:
      //  - A handle on the stack
      //  - A constant within a function's constant space
      //  - A local on the stack
      OperandIndex HandleConstantLocal;

      // A pointer to a field that is static
      Field* StaticField;
    };
    
    // When going through a handle, this can be a field offset onto the derefenced handle (a member)
    // This also works with stack localss and even constants (as well as static fields!)
    size_t FieldOffset;
  };

  // Opcode for the creation of a handle from a local
  class ZeroShared TimeoutOpcode : public Opcode
  {
  public:
    // Even though it might be more efficient to store this in ticks
    // Technically if want this format to be savable and platform independent, it's better
    // to save it in seconds
    size_t LengthSeconds;
  };

  // Opcode for the creation of a handle from a local
  class ZeroShared ToHandleOpcode : public Opcode
  {
  public:
    Operand ToHandle;
    OperandLocal SaveLocal;
    BoundType* Type;
  };

  // Opcode for the creation of generic delegates (never used directly)
  class ZeroShared CreateDelegateOpcode : public Opcode
  {
  public:
    Function* BoundFunction;
    OperandLocal SaveLocal;
  };

  // Opcode for the creation of static delegates
  // Note that this opcode always saves to a local
  // (anyone that wants to store the value just copies it from a local)
  class ZeroShared CreateStaticDelegateOpcode : public CreateDelegateOpcode
  {
  public:
  };

  // Opcode for the creation of instance delegates
  // Note that this opcode always saves to a local
  // (anyone that wants to store the value just copies it from a local)
  class ZeroShared CreateInstanceDelegateOpcode : public CreateDelegateOpcode
  {
  public:
    Operand ThisHandle;
    bool CanBeVirtual;
  };

  // Opcode for the if-instruction
  class ZeroShared IfOpcode : public Opcode
  {
  public:
    Operand Condition;
    ByteCodeOffset JumpOffset;
  };

  // Opcode for the relative jump instruction
  class ZeroShared RelativeJumpOpcode : public Opcode
  {
  public:
    ByteCodeOffset JumpOffset;
  };

  // Opcode for the prep for function call instruction
  class ZeroShared PrepForFunctionCallOpcode : public Opcode
  {
  public:
    Operand Delegate;
    ByteCodeOffset JumpOffsetIfStatic;
  };

  // Creates a fresh string builder that we use for efficient concatenation of strings
  class ZeroShared BeginStringBuilderOpcode : public Opcode
  {
  public:
  };

  // Finishes off a string builder and outputs the string to a given stack local
  class ZeroShared EndStringBuilderOpcode : public Opcode
  {
  public:
    OperandLocal SaveStringHandleLocal;
  };

  // Creates a fresh string builder that we use for efficient concatenation of strings
  class ZeroShared AddToStringBuilderOpcode : public Opcode
  {
  public:
    const Type* TypeToConvert;
    Operand Value;
  };

  // Gets the virtual type (most derived) of an expression
  class ZeroShared TypeIdOpcode : public Opcode
  {
  public:
    const Type* CompileTimeType;
    OperandLocal SaveTypeHandleLocal;
    Operand Expression;
  };

  // Opcode for generic creation of an object
  // Note that this opcode always creates a handle at the given local position
  class ZeroShared CreateTypeOpcode : public Opcode
  {
  public:
    BoundType* CreatedType;
    OperandLocal SaveHandleLocal;
  };

  // Opcode for local creation of an object
  class ZeroShared CreateLocalTypeOpcode : public CreateTypeOpcode
  {
  public:
    OperandLocal StackLocal;
  };

  // Opcode for the creation of a property delegate (a reference object)
  class ZeroShared CreatePropertyDelegateOpcode : public CreateTypeOpcode
  {
  public:
    OperandLocal ThisHandleLocal;
    Property* ReferencedProperty;
  };

  // Opcode for the delete object instruction
  class ZeroShared DeleteObjectOpcode : public Opcode
  {
  public:
    Operand Object;
  };

  // Opcode for the throw exception instruction
  class ZeroShared ThrowExceptionOpcode : public Opcode
  {
  public:
    Operand Exception;
  };

  // Binary operation between two operands (this instruction has no side effects
  // and is only used with value types, and therefore the output is always local)
  class ZeroShared BinaryRValueOpcode : public Opcode
  {
  public:
    Operand Left;
    Operand Right;
    OperandLocal Output;
    size_t Size;
  };

  // A side effect operator (such as assignment)
  class ZeroShared BinaryLValueOpcode : public Opcode
  {
  public:
    Operand Output;
    Operand Right;
  };

  // Unary operation for a single operand (this instruction has no side effects
  // and is only used with value types, and therefore the output is always local)
  class ZeroShared UnaryRValueOpcode : public Opcode
  {
  public:
    Operand SingleOperand;
    OperandLocal Output;
  };

  // A side effect operator (such as increment)
  class ZeroShared UnaryLValueOpcode : public Opcode
  {
  public:
    Operand SingleOperand;
  };

  // Convert one value to another (this instruction has no side effects and
  // is only used with value types, and therefore the output is always local)
  class ZeroShared ConversionOpcode : public Opcode
  {
  public:
    Operand ToConvert;
    OperandLocal Output;
  };

  // Convert a type into the 'Any' type (which means copying it's value into the variant)
  class ZeroShared AnyConversionOpcode : public ConversionOpcode
  {
  public:
    // For ConvertToAny:
    // The type we're going to be putting into the Any
    // Note that we may actually do extra introspection to find a more derived type
    // As an example, if we attempt to store an Animal into the Any, the type stored on
    // this opcode would be the Animal, even if the underlying value was really a Cat
    // however, when we actually do the operation, we'll know if it's a handle type
    // and then we'll pull the derived type Cat out and store that on the Any

    // For ConvertFromAny:
    // We compare the type stored within the Any to this type to ensure that
    // we only pull out the correct type, otherwise we throw an exception
    Type* RelatedType;
  };

  // When we cast between a base type and derived handles
  class ZeroShared DowncastConversionOpcode : public ConversionOpcode
  {
  public:
    // We check to make sure the type stored in the handle is a type that is either
    // more derived or the same as this related type
    Type* ToType;
  };

  // Describes how to get a value out of a value stored by the 'any' type
  class ZeroShared AnyDynamicGet : public Opcode
  {
  public:
    // An index into the constant table where the member's name lives (as a string)
    OperandIndex StringConstant;
  };

  namespace CopyMode
  {
    enum Enum
    {
      Assignment,
      Initialize,
      ToParameter,
      FromReturn,
      ToReturn,
    };
  }

  // Copy a value from one place to another
  class ZeroShared CopyOpcode : public Opcode
  {
  public:
    Operand Source;
    Operand Destination;
    size_t Size;
    CopyMode::Enum Mode;
  };

  namespace DebugPrimitive
  {
    enum Enum
    {
      Integer,
      Boolean,
      Real,
      Real2,
      Real3,
      Real4,
      Handle,
      Delegate,
      Memory
    };
  }

  class ZeroShared DebugOperand
  {
  public:
    DebugOperand();
    DebugOperand(size_t offset, DebugPrimitive::Enum primitive, bool isLocal, StringParam name);
    size_t OperandOffset;
    DebugPrimitive::Enum Primitive;
    bool IsLocalOnly;
    String Name;
  };

  class ZeroShared DebugInstruction
  {
  public:
    DebugInstruction();

    Array<DebugOperand> ReadOperands;
    Array<DebugOperand> WriteOperands;
    Array<size_t> Sizes;
    Array<size_t> OpcodeOffsets;
    Array<size_t> FunctionPointers;
    Array<size_t> TypePointers;
    Array<size_t> Options;
    bool IsCopy;
  };

  // Generate debug info per instruction
  ZeroShared void GenerateDebugInstructionInfo(Array<DebugInstruction>& debugOut);
}

#endif
