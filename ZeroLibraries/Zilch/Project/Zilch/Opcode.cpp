/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  const char* Instruction::Names[] =
  {
    #define ZilchEnumValue(value) #value,
    #include "InstructionsEnum.inl"
    #undef ZilchEnumValue
  };

  //***************************************************************************
  Operand::Operand() :
    HandleConstantLocal(0),
    FieldOffset(0),
    Type(OperandType::NotSet)
  {
  }

  //***************************************************************************
  Operand::Operand(OperandIndex local) :
    HandleConstantLocal(local),
    FieldOffset(0),
    Type(OperandType::Local)
  {
  }

  //***************************************************************************
  Operand::Operand(OperandIndex handleConstantLocal, size_t field, OperandType::Enum type) :
    HandleConstantLocal(handleConstantLocal),
    FieldOffset(field),
    Type(type)
  {
  }

  //***************************************************************************
  DebugOperand::DebugOperand()
  {
    this->OperandOffset = (size_t)-1;
    this->Primitive = DebugPrimitive::Memory;
    this->IsLocalOnly = false;
  }

  //***************************************************************************
  DebugOperand::DebugOperand(size_t offset, DebugPrimitive::Enum primitive, bool isLocal, StringParam name)
  {
    this->OperandOffset = offset;
    this->Primitive = primitive;
    this->IsLocalOnly = isLocal;
    this->Name = name;
  }

  //***************************************************************************
  DebugInstruction::DebugInstruction() :
    IsCopy(false)
  {
  }

#define ZilchOperand(array, type, member, primitive, isLocal) \
  array.PushBack(DebugOperand(offsetof(type, member), primitive, isLocal, #member));



  //***************************************************************************
  void GenerateDebugInstructionInfo(Array<DebugInstruction>& debugOut)
  {
    debugOut.Resize(Instruction::Count);

    // ToHandle
    {
      DebugInstruction& info = debugOut[Instruction::ToHandle];
      ZilchOperand(info.ReadOperands, ToHandleOpcode, ToHandle, DebugPrimitive::Memory, false);
      ZilchOperand(info.WriteOperands, ToHandleOpcode, SaveLocal, DebugPrimitive::Memory, true);
    }

    // CreateStaticDelegate
    {
      DebugInstruction& info = debugOut[Instruction::CreateStaticDelegate];
      info.FunctionPointers.PushBack(offsetof(CreateStaticDelegateOpcode, BoundFunction));
      ZilchOperand(info.WriteOperands, CreateStaticDelegateOpcode, SaveLocal, DebugPrimitive::Delegate, true);
    }

    // CreateInstanceDelegate
    {
      DebugInstruction& info = debugOut[Instruction::CreateInstanceDelegate];
      info.FunctionPointers.PushBack(offsetof(CreateInstanceDelegateOpcode, BoundFunction));
      ZilchOperand(info.ReadOperands, CreateInstanceDelegateOpcode, ThisHandle, DebugPrimitive::Handle, false);
      ZilchOperand(info.WriteOperands, CreateInstanceDelegateOpcode, SaveLocal, DebugPrimitive::Delegate, true);
    }

    
    // IfFalseRelativeGoTo / IfTrueRelativeGoTo
    {
      Instruction::Enum instructions[] = 
      {
        Instruction::IfFalseRelativeGoTo,
        Instruction::IfTrueRelativeGoTo
      };

      for (size_t i = 0; i < ZilchCArrayCount(instructions); ++i)
      {
        Instruction::Enum instruction = instructions[i];
        DebugInstruction& info = debugOut[instruction];
        ZilchOperand(info.ReadOperands, IfOpcode, Condition, DebugPrimitive::Boolean, false);
        info.OpcodeOffsets.PushBack(offsetof(IfOpcode, JumpOffset));
      }
    }

    // IfTrueRelativeGoTo
    {
      DebugInstruction& info = debugOut[Instruction::IfTrueRelativeGoTo];
      ZilchOperand(info.ReadOperands, IfOpcode, Condition, DebugPrimitive::Boolean, false);
      info.OpcodeOffsets.PushBack(offsetof(IfOpcode, JumpOffset));
    }
    
    
    // RelativeGoTo
    {
      DebugInstruction& info = debugOut[Instruction::RelativeGoTo];
      info.OpcodeOffsets.PushBack(offsetof(RelativeJumpOpcode, JumpOffset));
    }
    
    // PrepForFunctionCall
    {
      DebugInstruction& info = debugOut[Instruction::PrepForFunctionCall];
      ZilchOperand(info.ReadOperands, PrepForFunctionCallOpcode, Delegate, DebugPrimitive::Delegate, false);
      info.OpcodeOffsets.PushBack(offsetof(PrepForFunctionCallOpcode, JumpOffsetIfStatic));
    }
    
    // NewObject
    {
      DebugInstruction& info = debugOut[Instruction::NewObject];
      info.TypePointers.PushBack(offsetof(CreateTypeOpcode, CreatedType));
      ZilchOperand(info.WriteOperands, CreateTypeOpcode, SaveHandleLocal, DebugPrimitive::Handle, true);
    }

    // LocalObject
    {
      DebugInstruction& info = debugOut[Instruction::LocalObject];
      info.TypePointers.PushBack(offsetof(CreateLocalTypeOpcode, CreatedType));
      ZilchOperand(info.WriteOperands, CreateLocalTypeOpcode, SaveHandleLocal, DebugPrimitive::Handle, true);
      ZilchOperand(info.WriteOperands, CreateLocalTypeOpcode, StackLocal, DebugPrimitive::Memory, true);
    }
    
    // DeleteObject
    {
      DebugInstruction& info = debugOut[Instruction::DeleteObject];
      ZilchOperand(info.WriteOperands, DeleteObjectOpcode, Object, DebugPrimitive::Handle, false);
    }
    

    // [UnaryRValueOpcode]
    {
      Instruction::Enum instructions[] = 
      {
        Instruction::NegateInteger,
        Instruction::NegateReal,
        Instruction::NegateReal2,
        Instruction::NegateReal3,
        Instruction::NegateReal4,
        Instruction::LogicalNotBoolean,
        Instruction::BitwiseNotInteger
      };

      DebugPrimitive::Enum types[] =
      {
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Boolean,
        DebugPrimitive::Integer
      };

      for (size_t i = 0; i < ZilchCArrayCount(instructions); ++i)
      {
        Instruction::Enum instruction = instructions[i];
        DebugPrimitive::Enum primitive = types[i];
        DebugInstruction& info = debugOut[instruction];
        ZilchOperand(info.ReadOperands, UnaryRValueOpcode, SingleOperand, primitive, false);
        ZilchOperand(info.WriteOperands, UnaryRValueOpcode, Output, primitive, true);
      }
    }

    // [UnaryLValueOpcode]
    {
      Instruction::Enum instructions[] = 
      {
        Instruction::IncrementInteger,
        Instruction::IncrementReal,
        Instruction::DecrementInteger,
        Instruction::DecrementReal,
      };

      DebugPrimitive::Enum types[] =
      {
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Integer,
        DebugPrimitive::Real
      };

      for (size_t i = 0; i < ZilchCArrayCount(instructions); ++i)
      {
        Instruction::Enum instruction = instructions[i];
        DebugPrimitive::Enum primitive = types[i];
        DebugInstruction& info = debugOut[instruction];
        ZilchOperand(info.WriteOperands, UnaryLValueOpcode, SingleOperand, primitive, false);
      }
    }

    // [BinaryRValueOpcode] (Result + Operands all the same type)
    {
      Instruction::Enum instructions[] = 
      {
        Instruction::BitshiftLeftInteger,
        Instruction::BitshiftRightInteger,
        Instruction::BitwiseOrInteger,
        Instruction::BitwiseXorInteger,
        Instruction::BitwiseAndInteger,
        Instruction::AddInteger,
        Instruction::AddReal,
        Instruction::AddReal2,
        Instruction::AddReal3,
        Instruction::AddReal4,
        Instruction::SubtractInteger,
        Instruction::SubtractReal,
        Instruction::SubtractReal2,
        Instruction::SubtractReal3,
        Instruction::SubtractReal4,
        Instruction::MultiplyInteger,
        Instruction::MultiplyReal,
        Instruction::MultiplyReal2,
        Instruction::MultiplyReal3,
        Instruction::MultiplyReal4,
        Instruction::DivideInteger,
        Instruction::DivideReal,
        Instruction::DivideReal2,
        Instruction::DivideReal3,
        Instruction::DivideReal4,
        Instruction::ModuloInteger,
        Instruction::ModuloReal,
        Instruction::ModuloReal2,
        Instruction::ModuloReal3,
        Instruction::ModuloReal4,
        Instruction::PowInteger,
        Instruction::PowReal,
        Instruction::PowReal2,
        Instruction::PowReal3,
        Instruction::PowReal4
      };

      DebugPrimitive::Enum types[] =
      {
        DebugPrimitive::Integer,
        DebugPrimitive::Integer,
        DebugPrimitive::Integer,
        DebugPrimitive::Integer,
        DebugPrimitive::Integer,
        DebugPrimitive::Boolean,
        DebugPrimitive::Boolean,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
      };

      for (size_t i = 0; i < ZilchCArrayCount(instructions); ++i)
      {
        Instruction::Enum instruction = instructions[i];
        DebugPrimitive::Enum primitive = types[i];
        DebugInstruction& info = debugOut[instruction];
        ZilchOperand(info.ReadOperands, BinaryRValueOpcode, Left, primitive, false);
        ZilchOperand(info.ReadOperands, BinaryRValueOpcode, Right, primitive, false);
        ZilchOperand(info.WriteOperands, BinaryRValueOpcode, Output, primitive, true);
      }
    }

    // [BinaryRValueOpcode] (Operands all the same type, Result is a Boolean)
    {
      Instruction::Enum instructions[] = 
      {
        Instruction::TestLessThanInteger,
        Instruction::TestLessThanReal,
        Instruction::TestLessThanOrEqualToInteger,
        Instruction::TestLessThanOrEqualToReal,
        Instruction::TestGreaterThanInteger,
        Instruction::TestGreaterThanReal,
        Instruction::TestGreaterThanOrEqualToInteger,
        Instruction::TestGreaterThanOrEqualToReal,
        Instruction::TestInequalityInteger,
        Instruction::TestInequalityReal,
        Instruction::TestInequalityBoolean,
        Instruction::TestInequalityHandle,
        Instruction::TestInequalityReal2,
        Instruction::TestInequalityReal3,
        Instruction::TestInequalityReal4,
        Instruction::TestEqualityInteger,
        Instruction::TestEqualityReal,
        Instruction::TestEqualityBoolean,
        Instruction::TestEqualityHandle,
        Instruction::TestEqualityReal2,
        Instruction::TestEqualityReal3,
        Instruction::TestEqualityReal4
      };

      DebugPrimitive::Enum types[] =
      {
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Boolean,
        DebugPrimitive::Handle,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Boolean,
        DebugPrimitive::Handle,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
      };

      for (size_t i = 0; i < ZilchCArrayCount(instructions); ++i)
      {
        Instruction::Enum instruction = instructions[i];
        DebugPrimitive::Enum primitive = types[i];
        DebugInstruction& info = debugOut[instruction];
        ZilchOperand(info.ReadOperands, BinaryRValueOpcode, Left, primitive, false);
        ZilchOperand(info.ReadOperands, BinaryRValueOpcode, Right, primitive, false);
        ZilchOperand(info.WriteOperands, BinaryRValueOpcode, Output, DebugPrimitive::Boolean, true);
      }
    }

    // [BinaryRValueOpcode] (Scalar + Vector operations)
    {
      Instruction::Enum instructions[] = 
      {
        Instruction::ScalarMultiplyReal2,
        Instruction::ScalarMultiplyReal3,
        Instruction::ScalarMultiplyReal4,
        Instruction::ScalarDivideReal2,
        Instruction::ScalarDivideReal3,
        Instruction::ScalarDivideReal4,
        Instruction::ScalarModuloReal2,
        Instruction::ScalarModuloReal3,
        Instruction::ScalarModuloReal4,
        Instruction::ScalarPowReal2,
        Instruction::ScalarPowReal3,
        Instruction::ScalarPowReal4
      };

      DebugPrimitive::Enum types[] =
      {
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
      };

      for (size_t i = 0; i < ZilchCArrayCount(instructions); ++i)
      {
        Instruction::Enum instruction = instructions[i];
        DebugPrimitive::Enum primitive = types[i];
        DebugInstruction& info = debugOut[instruction];
        ZilchOperand(info.ReadOperands, BinaryRValueOpcode, Left, primitive, false);
        ZilchOperand(info.ReadOperands, BinaryRValueOpcode, Right, DebugPrimitive::Real, false);
        ZilchOperand(info.WriteOperands, BinaryRValueOpcode, Output, primitive, true);
      }
    }

    // [BinaryLValueOpcode] (Operands all the same type)
    {
      Instruction::Enum instructions[] = 
      {
        Instruction::AssignmentBitshiftLeftInteger,
        Instruction::AssignmentBitshiftRightInteger,
        Instruction::AssignmentBitwiseOrInteger,
        Instruction::AssignmentBitwiseXorInteger,
        Instruction::AssignmentBitwiseAndInteger,
        Instruction::AssignmentAddInteger,
        Instruction::AssignmentAddReal,
        Instruction::AssignmentAddReal2,
        Instruction::AssignmentAddReal3,
        Instruction::AssignmentAddReal4,
        Instruction::AssignmentSubtractInteger,
        Instruction::AssignmentSubtractReal,
        Instruction::AssignmentSubtractReal2,
        Instruction::AssignmentSubtractReal3,
        Instruction::AssignmentSubtractReal4,
        Instruction::AssignmentMultiplyInteger,
        Instruction::AssignmentMultiplyReal,
        Instruction::AssignmentMultiplyReal2,
        Instruction::AssignmentMultiplyReal3,
        Instruction::AssignmentMultiplyReal4,
        Instruction::AssignmentDivideInteger,
        Instruction::AssignmentDivideReal,
        Instruction::AssignmentDivideReal2,
        Instruction::AssignmentDivideReal3,
        Instruction::AssignmentDivideReal4,
        Instruction::AssignmentModuloInteger,
        Instruction::AssignmentModuloReal,
        Instruction::AssignmentModuloReal2,
        Instruction::AssignmentModuloReal3,
        Instruction::AssignmentModuloReal4,
        Instruction::AssignmentPowInteger,
        Instruction::AssignmentPowReal,
        Instruction::AssignmentPowReal2,
        Instruction::AssignmentPowReal3,
        Instruction::AssignmentPowReal4
      };

      DebugPrimitive::Enum types[] =
      {
        DebugPrimitive::Integer,
        DebugPrimitive::Integer,
        DebugPrimitive::Integer,
        DebugPrimitive::Integer,
        DebugPrimitive::Integer,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4
      };

      for (size_t i = 0; i < ZilchCArrayCount(instructions); ++i)
      {
        Instruction::Enum instruction = instructions[i];
        DebugPrimitive::Enum primitive = types[i];
        DebugInstruction& info = debugOut[instruction];
        ZilchOperand(info.ReadOperands, BinaryLValueOpcode, Right, primitive, false);
        ZilchOperand(info.WriteOperands, BinaryLValueOpcode, Output, primitive, false);
      }
    }

    // [BinaryLValueOpcode] (Scalar + Vector assignment operations)
    {
      Instruction::Enum instructions[] = 
      {
        Instruction::AssignmentScalarMultiplyReal2,
        Instruction::AssignmentScalarMultiplyReal3,
        Instruction::AssignmentScalarMultiplyReal4,
        Instruction::AssignmentScalarDivideReal2,
        Instruction::AssignmentScalarDivideReal3,
        Instruction::AssignmentScalarDivideReal4,
        Instruction::AssignmentScalarModuloReal2,
        Instruction::AssignmentScalarModuloReal3,
        Instruction::AssignmentScalarModuloReal4,
        Instruction::AssignmentScalarPowReal2,
        Instruction::AssignmentScalarPowReal3,
        Instruction::AssignmentScalarPowReal4
      };

      DebugPrimitive::Enum types[] =
      {
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4
      };

      for (size_t i = 0; i < ZilchCArrayCount(instructions); ++i)
      {
        Instruction::Enum instruction = instructions[i];
        DebugPrimitive::Enum primitive = types[i];
        DebugInstruction& info = debugOut[instruction];
        ZilchOperand(info.ReadOperands, BinaryLValueOpcode, Right, DebugPrimitive::Real, false);
        ZilchOperand(info.WriteOperands, BinaryLValueOpcode, Output, primitive, false);
      }
    }

    // [ConversionOpcode]
    {
      Instruction::Enum instructions[] = 
      {
        Instruction::ConvertIntegerToReal,
        Instruction::ConvertIntegerToBoolean,
        Instruction::ConvertRealToInteger,
        Instruction::ConvertRealToBoolean,
        Instruction::ConvertBooleanToInteger,
        Instruction::ConvertBooleanToReal
      };

      DebugPrimitive::Enum fromTypes[] =
      {
        DebugPrimitive::Integer,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Real,
        DebugPrimitive::Boolean,
        DebugPrimitive::Boolean,
      };

      DebugPrimitive::Enum toTypes[] =
      {
        DebugPrimitive::Real,
        DebugPrimitive::Boolean,
        DebugPrimitive::Integer,
        DebugPrimitive::Boolean,
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
      };

      for (size_t i = 0; i < ZilchCArrayCount(instructions); ++i)
      {
        Instruction::Enum instruction = instructions[i];
        DebugPrimitive::Enum fromPrimitive = fromTypes[i];
        DebugPrimitive::Enum toPrimitive = toTypes[i];
        DebugInstruction& info = debugOut[instruction];
        ZilchOperand(info.ReadOperands, ConversionOpcode, ToConvert, fromPrimitive, false);
        ZilchOperand(info.WriteOperands, ConversionOpcode, Output, toPrimitive, true);
      }
    }

    // String to StringRange conversion
    {
      Instruction::Enum instruction = Instruction::ConvertStringToStringRangeExtended;
      DebugPrimitive::Enum fromPrimitive = DebugPrimitive::Handle;
      DebugPrimitive::Enum toPrimitive = DebugPrimitive::Handle;
      DebugInstruction& info = debugOut[instruction];
      ZilchOperand(info.ReadOperands, ConversionOpcode, ToConvert, fromPrimitive, false);
      ZilchOperand(info.WriteOperands, ConversionOpcode, Output, toPrimitive, true);
    }

    // [CopyOpcode]
    {
      Instruction::Enum instructions[] = 
      {
        Instruction::CopyInteger,
        Instruction::CopyReal,
        Instruction::CopyReal2,
        Instruction::CopyReal3,
        Instruction::CopyReal4,
        Instruction::CopyBoolean,
        Instruction::CopyHandle,
        Instruction::CopyDelegate,
        Instruction::CopyValue
      };

      DebugPrimitive::Enum types[] =
      {
        DebugPrimitive::Integer,
        DebugPrimitive::Real,
        DebugPrimitive::Real2,
        DebugPrimitive::Real3,
        DebugPrimitive::Real4,
        DebugPrimitive::Boolean,
        DebugPrimitive::Handle,
        DebugPrimitive::Delegate,
        DebugPrimitive::Memory,
      };

      for (size_t i = 0; i < ZilchCArrayCount(instructions); ++i)
      {
        Instruction::Enum instruction = instructions[i];
        DebugPrimitive::Enum primitive = types[i];
        DebugInstruction& info = debugOut[instruction];
        info.IsCopy = true;
        ZilchOperand(info.ReadOperands, CopyOpcode, Source, primitive, false);
        ZilchOperand(info.WriteOperands, CopyOpcode, Destination, primitive, false);
        info.Sizes.PushBack(offsetof(CopyOpcode, Size));
        info.Options.PushBack(offsetof(CopyOpcode, Mode));
      }
    }
  }
}

