/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  // Operator Precedence:
  // Left to right:
  //  0. As
  //     BeginFunctionCall & EndFunctionCall
  //     BeginIndex & EndIndex
  //     Access
  // Right to left:
  //  1. Positive
  //     Negative
  //     Increment
  //     Decrement
  //     LogicalNot
  //     BitwiseNot
  //     PropertyDelegate
  // Left to right:
  //  2. Exponent
  //  3. Multiply
  //     Divide
  //     Modulo
  //  4. Add
  //     Subtract
  //  5. BitshiftLeft
  //     BitshiftRight
  //  6. LessThan
  //     LessThanOrEqualTo
  //     GreaterThan
  //     GreaterThanOrEqualTo
  //  7. Equality
  //     Inequality
  //  8. BitwiseAnd
  //  9. BitwiseXor
  // 10. BitwiseOr
  // 11. LogicalAnd
  // 12. LogicalOr
  // Right to left:
  // 13. Assignment
  //     AssignmentAdd
  //     AssignmentSubtract
  //     AssignmentMultiply
  //     AssignmentDivide
  //     AssignmentModulo
  //     AssignmentExponent
  //     AssignmentLeftShift
  //     AssignmentRightShift
  //     AssignmentBitwiseAnd
  //     AssignmentBitwiseXor
  //     AssignmentBitwiseOr

  //***************************************************************************
  BinaryOperator::BinaryOperator() :
    IsValid(false),
    Lhs(nullptr),
    Rhs(nullptr),
    Result(nullptr),
    Operator(Grammar::Invalid),
    Instruction(Instruction::InvalidInstruction),
    FlipArguments(false),
    Io(IoMode::NotSet),
    CastLhsTo(nullptr),
    CastRhsTo(nullptr)
  {
  }

  //***************************************************************************
  size_t BinaryOperator::Hash() const
  {
    // Start off with a cleared out hash
    size_t result = 0;

    // Hash both the left and right (add a random prime because they could be the same)
    result ^= (size_t)(this->Lhs->Hash());
    result ^= (size_t)(this->Rhs->Hash() * 1276478784635841471);

    // Now hash the operator and include that 
    result ^= (size_t)(this->Operator * 5463458053);

    // Return the resulting hash
    return result;
  }

  //***************************************************************************
  bool BinaryOperator::operator==(const BinaryOperator& rhs) const
  {
    // Compare the operators
    if (this->Operator != rhs.Operator)
      return false;

    // Compare the left and right operands
    return (this->Lhs == rhs.Lhs) && (this->Rhs == rhs.Rhs);
  }


  //***************************************************************************
  UnaryOperator::UnaryOperator() :
    IsValid(false),
    Operand(nullptr),
    Result(nullptr),
    Operator(Grammar::Invalid),
    Instruction(Instruction::InvalidInstruction),
    Io(IoMode::NotSet)
  {
  }

  //***************************************************************************
  size_t UnaryOperator::Hash() const
  {
    // Start off with a cleared out hash
    size_t result = 0;

    // Hash the operand and add that to the result
    result ^= (size_t)this->Operand->Hash();

    // Now hash the operator and include that 
    result ^= (size_t)(this->Operator * 5463458053);

    // Return the resulting hash
    return result;
  }

  //***************************************************************************
  bool UnaryOperator::operator==(const UnaryOperator& rhs) const
  {
    // Compare the operators
    if (this->Operator != rhs.Operator)
      return false;

    // Compare the operands
    return this->Operand == rhs.Operand;
  }

  //***************************************************************************
  UntypedOperator::UntypedOperator() :
    IsValid(false),
    Operator(Grammar::Invalid),
    Precedence(0),
    Associativity(OperatorAssociativity::LeftToRight)
  {
  }

  //***************************************************************************
  CastOperator::CastOperator() :
    IsValid(false),
    From(nullptr),
    To(nullptr),
    Operation(CastOperation::Invalid),
    PrimitiveInstruction(Instruction::InvalidInstruction),
    CanBeImplicit(false),
    RequiresCodeGeneration(false)
  {
  }

  //***************************************************************************
  size_t CastOperator::Hash() const
  {
    return (size_t)(this->From->Hash() ^ this->To->Hash() * 33679033);
  }

  //***************************************************************************
  bool CastOperator::operator==(const CastOperator& rhs) const
  {
    return this->From == rhs.From && this->To == rhs.To;
  }

  //***************************************************************************
  Shared::Shared()
  {
    // Get the instance of the type database
    Core& core = Core::GetInstance();

    // Reserve room for operators
    this->PrecedenceToOperators.Reserve(32);
    
    // Add all generic information about operator precedence and associativity
    size_t precedence = 1;
    OperatorAssociativity::Enum associativity = OperatorAssociativity::LeftToRight;
    this->AddPrecedence(precedence, associativity, OperatorArity::Unary, Grammar::BeginFunctionCall);
    this->AddPrecedence(precedence, associativity, OperatorArity::Unary, Grammar::EndFunctionCall);
    this->AddPrecedence(precedence, associativity, OperatorArity::Unary, Grammar::BeginIndex);
    this->AddPrecedence(precedence, associativity, OperatorArity::Unary, Grammar::EndIndex);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::Access);
    precedence = 2;
    associativity = OperatorAssociativity::RightToLeft;
    this->AddPrecedence(precedence, associativity, OperatorArity::Unary, Grammar::Positive);
    this->AddPrecedence(precedence, associativity, OperatorArity::Unary, Grammar::Negative);
    this->AddPrecedence(precedence, associativity, OperatorArity::Unary, Grammar::Increment);
    this->AddPrecedence(precedence, associativity, OperatorArity::Unary, Grammar::Decrement);
    this->AddPrecedence(precedence, associativity, OperatorArity::Unary, Grammar::LogicalNot);
    this->AddPrecedence(precedence, associativity, OperatorArity::Unary, Grammar::BitwiseNot);
    this->AddPrecedence(precedence, associativity, OperatorArity::Unary, Grammar::PropertyDelegate);
    precedence = 3;
    associativity = OperatorAssociativity::LeftToRight;
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::Exponent);
    precedence = 4;
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::Multiply);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::Divide);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::Modulo);
    precedence = 5;
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::Add);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::Subtract);
    precedence = 6;
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::BitshiftLeft);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::BitshiftRight);
    precedence = 7;
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::LessThan);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::LessThanOrEqualTo);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::GreaterThan);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::GreaterThanOrEqualTo);
    precedence = 8;
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::Equality);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::Inequality);
    precedence = 9;
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::BitwiseAnd);
    precedence = 10;
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::BitwiseXor);
    precedence = 11;
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::BitwiseOr);
    precedence = 12;
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::LogicalAnd);
    precedence = 13;
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::LogicalOr);
    precedence = 14;
    associativity = OperatorAssociativity::RightToLeft;
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::Assignment);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::AssignmentAdd);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::AssignmentSubtract);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::AssignmentMultiply);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::AssignmentDivide);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::AssignmentModulo);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::AssignmentExponent);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::AssignmentLeftShift);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::AssignmentRightShift);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::AssignmentBitwiseAnd);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::AssignmentBitwiseXor);
    this->AddPrecedence(precedence, associativity, OperatorArity::Binary, Grammar::AssignmentBitwiseOr);

    // HandleAssignment
    {
      BinaryOperator& info = this->HandleAssignment;
      info.Result = core.VoidType;
      info.Operator = Grammar::Assignment;
      info.Instruction = Instruction::CopyHandle;
      info.Io = IoMode::WriteLValue;
      info.IsValid = true;
    }
    // HandleEquality
    {
      BinaryOperator& info = this->HandleEquality;
      info.Result = core.BooleanType;
      info.Operator = Grammar::Equality;
      info.Instruction = Instruction::TestEqualityHandle;
      info.Io = IoMode::ReadRValue;
      info.IsValid = true;
    }
    // HandleInequality
    {
      BinaryOperator& info = this->HandleInequality;
      info.Result = core.BooleanType;
      info.Operator = Grammar::Inequality;
      info.Instruction = Instruction::TestInequalityHandle;
      info.Io = IoMode::ReadRValue;
      info.IsValid = true;
    }
    // DelegateAssignment
    {
      BinaryOperator& info = this->DelegateAssignment;
      info.Result = core.VoidType;
      info.Operator = Grammar::Assignment;
      info.Instruction = Instruction::CopyDelegate;
      info.Io = IoMode::WriteLValue;
      info.IsValid = true;
    }
    // DelegateEquality
    {
      BinaryOperator& info = this->DelegateEquality;
      info.Result = core.BooleanType;
      info.Operator = Grammar::Equality;
      info.Instruction = Instruction::TestEqualityDelegate;
      info.Io = IoMode::ReadRValue;
      info.IsValid = true;
    }
    // DelegateInequality
    {
      BinaryOperator& info = this->DelegateInequality;
      info.Result = core.BooleanType;
      info.Operator = Grammar::Inequality;
      info.Instruction = Instruction::TestInequalityDelegate;
      info.Io = IoMode::ReadRValue;
      info.IsValid = true;
    }
    // AnyAssignment
    {
      BinaryOperator& info = this->AnyAssignment;
      info.Result = core.VoidType;
      info.Operator = Grammar::Assignment;
      info.Instruction = Instruction::CopyAny;
      info.Io = IoMode::WriteLValue;
      info.IsValid = true;
    }
    // AnyEquality
    {
      BinaryOperator& info = this->AnyEquality;
      info.Result = core.BooleanType;
      info.Operator = Grammar::Equality;
      info.Instruction = Instruction::TestEqualityAny;
      info.Io = IoMode::ReadRValue;
      info.IsValid = true;
    }
    // AnyInequality
    {
      BinaryOperator& info = this->AnyInequality;
      info.Result = core.BooleanType;
      info.Operator = Grammar::Inequality;
      info.Instruction = Instruction::TestInequalityAny;
      info.Io = IoMode::ReadRValue;
      info.IsValid = true;
    }
    // ValueAssignment
    {
      BinaryOperator& info = this->ValueAssignment;
      info.Result = core.VoidType;
      info.Operator = Grammar::Assignment;
      info.Instruction = Instruction::CopyValue;
      info.Io = IoMode::WriteLValue;
      info.IsValid = true;
    }
    // ValueEquality
    {
      BinaryOperator& info = this->ValueEquality;
      info.Result = core.BooleanType;
      info.Operator = Grammar::Equality;
      info.Instruction = Instruction::TestEqualityValue;
      info.Io = IoMode::ReadRValue;
      info.IsValid = true;
    }
    // ValueInequality
    {
      BinaryOperator& info = this->ValueInequality;
      info.Result = core.BooleanType;
      info.Operator = Grammar::Inequality;
      info.Instruction = Instruction::TestInequalityValue;
      info.Io = IoMode::ReadRValue;
      info.IsValid = true;
    }
    
    // RawImplicitCast, (same-cast, up-cast, null-cast, any-delegate-cast)
    {
      CastOperator& info = this->RawImplicitCast;
      info.Operation = CastOperation::Raw;
      info.CanBeImplicit = true;
      info.RequiresCodeGeneration = false;
      info.IsValid = true;
    }
    // DynamicDownCast
    {
      CastOperator& info = this->DynamicDownCast;
      info.Operation = CastOperation::DynamicDown;
      info.CanBeImplicit = false;
      info.RequiresCodeGeneration = true;
      info.IsValid = true;
    }
    // ToAnyCast
    {
      CastOperator& info = this->ToAnyCast;
      info.Operation = CastOperation::ToAny;
      info.CanBeImplicit = true;
      info.RequiresCodeGeneration = true;
      info.IsValid = true;
    }
    // FromAnyCast
    {
      CastOperator& info = this->FromAnyCast;
      info.Operation = CastOperation::FromAny;
      info.CanBeImplicit = true;
      info.RequiresCodeGeneration = true;
      info.IsValid = true;
    }
    // FromAnyHandleCast
    {
      CastOperator& info = this->FromAnyHandleCast;
      info.Operation = CastOperation::DynamicDown;
      info.CanBeImplicit = true;
      info.RequiresCodeGeneration = true;
      info.IsValid = true;
    }
    // EnumIntegerCast
    {
      CastOperator& info = this->EnumIntegerCast;
      info.Operation = CastOperation::Raw;
      info.CanBeImplicit = true;
      info.RequiresCodeGeneration = false;
      info.IsValid = true;
    }
    // IntegerEnumCast
    {
      CastOperator& info = this->IntegerEnumCast;
      info.Operation = CastOperation::Raw;
      info.CanBeImplicit = false;
      info.RequiresCodeGeneration = false;
      info.IsValid = true;
    }
    // NullToDelegate
    {
      CastOperator& info = this->NullToDelegate;
      info.Operation = CastOperation::NullToDelegate;
      info.CanBeImplicit = true;
      info.RequiresCodeGeneration = false;
      info.IsValid = true;
      //this->CastOperatorsFrom[core.NullType].PushBack(info);
    }

    // Handle our built in primitive casts
    // The 1-dimensional cases
    this->AddPrimitiveCast(core.ByteType,           core.RealType,          Instruction::ConvertByteToReal,                 true );
    this->AddPrimitiveCast(core.ByteType,           core.BooleanType,       Instruction::ConvertByteToBoolean,              false);
    this->AddPrimitiveCast(core.ByteType,           core.IntegerType,       Instruction::ConvertByteToInteger,              true );
    this->AddPrimitiveCast(core.ByteType,           core.DoubleIntegerType, Instruction::ConvertByteToDoubleInteger,        true );
    this->AddPrimitiveCast(core.ByteType,           core.DoubleRealType,    Instruction::ConvertByteToDoubleReal,           true );

    this->AddPrimitiveCast(core.IntegerType,        core.RealType,          Instruction::ConvertIntegerToReal,              true );
    this->AddPrimitiveCast(core.IntegerType,        core.BooleanType,       Instruction::ConvertIntegerToBoolean,           false);
    this->AddPrimitiveCast(core.IntegerType,        core.ByteType,          Instruction::ConvertIntegerToByte,              false);
    this->AddPrimitiveCast(core.IntegerType,        core.DoubleIntegerType, Instruction::ConvertIntegerToDoubleInteger,     true );
    this->AddPrimitiveCast(core.IntegerType,        core.DoubleRealType,    Instruction::ConvertIntegerToDoubleReal,        true );
    this->AddPrimitiveCast(core.RealType,           core.IntegerType,       Instruction::ConvertRealToInteger,              false);
    this->AddPrimitiveCast(core.RealType,           core.BooleanType,       Instruction::ConvertRealToBoolean,              false);
    this->AddPrimitiveCast(core.RealType,           core.ByteType,          Instruction::ConvertRealToByte,                 false);
    this->AddPrimitiveCast(core.RealType,           core.DoubleIntegerType, Instruction::ConvertRealToDoubleInteger,        false);
    this->AddPrimitiveCast(core.RealType,           core.DoubleRealType,    Instruction::ConvertRealToDoubleReal,           true );
    this->AddPrimitiveCast(core.BooleanType,        core.IntegerType,       Instruction::ConvertBooleanToInteger,           false);
    this->AddPrimitiveCast(core.BooleanType,        core.RealType,          Instruction::ConvertBooleanToReal,              false);
    this->AddPrimitiveCast(core.BooleanType,        core.ByteType,          Instruction::ConvertBooleanToByte,              false);
    this->AddPrimitiveCast(core.BooleanType,        core.DoubleIntegerType, Instruction::ConvertBooleanToDoubleInteger,     false);
    this->AddPrimitiveCast(core.BooleanType,        core.DoubleRealType,    Instruction::ConvertBooleanToDoubleReal,        false);
    this->AddPrimitiveCast(core.DoubleIntegerType,  core.RealType,          Instruction::ConvertDoubleIntegerToReal,        false);
    this->AddPrimitiveCast(core.DoubleIntegerType,  core.BooleanType,       Instruction::ConvertDoubleIntegerToBoolean,     false);
    this->AddPrimitiveCast(core.DoubleIntegerType,  core.ByteType,          Instruction::ConvertDoubleIntegerToByte,        false);
    this->AddPrimitiveCast(core.DoubleIntegerType,  core.IntegerType,       Instruction::ConvertDoubleIntegerToInteger,     false);
    this->AddPrimitiveCast(core.DoubleIntegerType,  core.DoubleRealType,    Instruction::ConvertDoubleIntegerToDoubleReal,  true );
    this->AddPrimitiveCast(core.DoubleRealType,     core.RealType,          Instruction::ConvertDoubleRealToReal,           false);
    this->AddPrimitiveCast(core.DoubleRealType,     core.BooleanType,       Instruction::ConvertDoubleRealToBoolean,        false);
    this->AddPrimitiveCast(core.DoubleRealType,     core.ByteType,          Instruction::ConvertDoubleRealToByte,           false);
    this->AddPrimitiveCast(core.DoubleRealType,     core.IntegerType,       Instruction::ConvertDoubleRealToInteger,        false);
    this->AddPrimitiveCast(core.DoubleRealType,     core.DoubleIntegerType, Instruction::ConvertDoubleRealToDoubleInteger,  false);

    // The 2-dimensional cases
    this->AddPrimitiveCast(core.Integer2Type,  core.Real2Type,    Instruction::ConvertInteger2ToReal2,    true);
    this->AddPrimitiveCast(core.Integer2Type,  core.Boolean2Type, Instruction::ConvertInteger2ToBoolean2, false);
    this->AddPrimitiveCast(core.Real2Type,     core.Integer2Type, Instruction::ConvertReal2ToInteger2,    false);
    this->AddPrimitiveCast(core.Real2Type,     core.Boolean2Type, Instruction::ConvertReal2ToBoolean2,    false);
    this->AddPrimitiveCast(core.Boolean2Type,  core.Integer2Type, Instruction::ConvertBoolean2ToInteger2, false);
    this->AddPrimitiveCast(core.Boolean2Type,  core.Real2Type,    Instruction::ConvertBoolean2ToReal2,    false);

    // The 3-dimensional cases
    this->AddPrimitiveCast(core.Integer3Type,  core.Real3Type,    Instruction::ConvertInteger3ToReal3,    true);
    this->AddPrimitiveCast(core.Integer3Type,  core.Boolean3Type, Instruction::ConvertInteger3ToBoolean3, false);
    this->AddPrimitiveCast(core.Real3Type,     core.Integer3Type, Instruction::ConvertReal3ToInteger3,    false);
    this->AddPrimitiveCast(core.Real3Type,     core.Boolean3Type, Instruction::ConvertReal3ToBoolean3,    false);
    this->AddPrimitiveCast(core.Boolean3Type,  core.Integer3Type, Instruction::ConvertBoolean3ToInteger3, false);
    this->AddPrimitiveCast(core.Boolean3Type,  core.Real3Type,    Instruction::ConvertBoolean3ToReal3,    false);

    // The 4-dimensional cases
    this->AddPrimitiveCast(core.Integer4Type,  core.Real4Type,    Instruction::ConvertInteger4ToReal4,    true);
    this->AddPrimitiveCast(core.Integer4Type,  core.Boolean4Type, Instruction::ConvertInteger4ToBoolean4, false);
    this->AddPrimitiveCast(core.Real4Type,     core.Integer4Type, Instruction::ConvertReal4ToInteger4,    false);
    this->AddPrimitiveCast(core.Real4Type,     core.Boolean4Type, Instruction::ConvertReal4ToBoolean4,    false);
    this->AddPrimitiveCast(core.Boolean4Type,  core.Integer4Type, Instruction::ConvertBoolean4ToInteger4, false);
    this->AddPrimitiveCast(core.Boolean4Type,  core.Real4Type,    Instruction::ConvertBoolean4ToReal4,    false);

    // String to StringRange
    this->AddPrimitiveCast(core.StringType,  core.StringRangeType,    Instruction::ConvertStringToStringRangeExtended, true);

    // Note: These macros mirror those inside of InstructionEnum and VirtualMachine (for generation of instructions)

    // Copy
    #define ZilchCopyOperators(WithType)                                                                                                                                                                                \
      {                                                                                                                                                                                                                 \
        BoundType* type =  core.WithType##Type;                                                                                                                                                                         \
        this->AddBinary(type, core.VoidType, Grammar::Assignment, Instruction::Copy##WithType, IoMode::WriteLValue);                                                                                                    \
      }

    // Equality and inequality
    #define ZilchEqualityOperators(WithType, ResultType)                                                                                                                                                                            \
      {                                                                                                                                                                                                                 \
        BoundType* type =  core.WithType##Type;                                                                                                                                                                         \
        this->AddBinary(type, core.ResultType##Type, Grammar::Inequality, Instruction::TestInequality##WithType, IoMode::ReadRValue);                                                                                        \
        this->AddBinary(type, core.ResultType##Type, Grammar::Equality, Instruction::TestEquality##WithType, IoMode::ReadRValue);                                                                                            \
      }

    // Less and greater comparison
    #define ZilchComparisonOperators(WithType, ResultType)                                                                                                                                                              \
      {                                                                                                                                                                                                                 \
        BoundType* type =  core.WithType##Type;                                                                                                                                                                         \
        this->AddBinary(type, core.ResultType##Type, Grammar::LessThan, Instruction::TestLessThan##WithType, IoMode::ReadRValue);                                                                                       \
        this->AddBinary(type, core.ResultType##Type, Grammar::LessThanOrEqualTo, Instruction::TestLessThanOrEqualTo##WithType, IoMode::ReadRValue);                                                                     \
        this->AddBinary(type, core.ResultType##Type, Grammar::GreaterThan, Instruction::TestGreaterThan##WithType, IoMode::ReadRValue);                                                                                 \
        this->AddBinary(type, core.ResultType##Type, Grammar::GreaterThanOrEqualTo, Instruction::TestGreaterThanOrEqualTo##WithType, IoMode::ReadRValue);                                                               \
      }

    // Generic numeric operators, copy, equality
    #define ZilchNumericOperators(WithType, ComparisonType)                                                                                                                                                                             \
      ZilchCopyOperators(WithType)                                                                                                                                                                                      \
      ZilchEqualityOperators(WithType, ComparisonType)                                                                                                                                                                                  \
      {                                                                                                                                                                                                                 \
        BoundType* type =  core.WithType##Type;                                                                                                                                                                         \
        this->AddUnary(type, type, Grammar::Positive, Instruction::InvalidInstruction, IoMode::ReadRValue);                                                                                                             \
        this->AddUnary(type, type, Grammar::Negative, Instruction::Negate##WithType, IoMode::ReadRValue);                                                                                                               \
        this->AddUnary(type, core.VoidType, Grammar::Increment, Instruction::Increment##WithType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue));                                                            \
        this->AddUnary(type, core.VoidType, Grammar::Decrement, Instruction::Decrement##WithType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue));                                                            \
        this->AddBinary(type, type, Grammar::Add, Instruction::Add##WithType, IoMode::ReadRValue);                                                                                                                      \
        this->AddBinary(type, type, Grammar::Subtract, Instruction::Subtract##WithType, IoMode::ReadRValue);                                                                                                            \
        this->AddBinary(type, type, Grammar::Multiply, Instruction::Multiply##WithType, IoMode::ReadRValue);                                                                                                            \
        this->AddBinary(type, type, Grammar::Divide, Instruction::Divide##WithType, IoMode::ReadRValue);                                                                                                                \
        this->AddBinary(type, type, Grammar::Modulo, Instruction::Modulo##WithType, IoMode::ReadRValue);                                                                                                                \
        this->AddBinary(type, type, Grammar::Exponent, Instruction::Pow##WithType, IoMode::ReadRValue);                                                                                                                 \
        this->AddBinary(type, core.VoidType, Grammar::AssignmentAdd, Instruction::AssignmentAdd##WithType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue));                                                   \
        this->AddBinary(type, core.VoidType, Grammar::AssignmentSubtract, Instruction::AssignmentSubtract##WithType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue));                                         \
        this->AddBinary(type, core.VoidType, Grammar::AssignmentMultiply, Instruction::AssignmentMultiply##WithType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue));                                         \
        this->AddBinary(type, core.VoidType, Grammar::AssignmentDivide, Instruction::AssignmentDivide##WithType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue));                                             \
        this->AddBinary(type, core.VoidType, Grammar::AssignmentModulo, Instruction::AssignmentModulo##WithType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue));                                             \
        this->AddBinary(type, core.VoidType, Grammar::AssignmentExponent, Instruction::AssignmentPow##WithType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue));                                              \
      }

    // Generic numeric operators, copy, equality, comparison
    #define ZilchScalarOperators(WithType)                                                                                                                                                                              \
      ZilchNumericOperators(WithType, Boolean)                                                                                                                                                                          \
      ZilchComparisonOperators(WithType, Boolean)

    // Vector operations, generic numeric operators, copy, equality
    #define ZilchVectorOperators(VectorType, ScalarType, ComparisonType)                                                                                                                                                \
      ZilchNumericOperators(VectorType, Boolean)                                                                                                                                                                 \
      ZilchComparisonOperators(VectorType, ComparisonType)                                                                                                                                                              \
      {                                                                                                                                                                                                                 \
        BoundType* vectorType = core.VectorType##Type;                                                                                                                                                                  \
        BoundType* scalarType = core.ScalarType##Type;                                                                                                                                                                  \
        this->AddBinaryCommunative(vectorType, scalarType, vectorType, Grammar::Multiply, Instruction::ScalarMultiply##VectorType, IoMode::ReadRValue);                                                                 \
        this->AddBinaryNonCommunative(vectorType, scalarType, vectorType, Grammar::Divide, Instruction::ScalarDivide##VectorType, IoMode::ReadRValue);                                                                  \
        this->AddBinaryNonCommunative(vectorType, scalarType, vectorType, Grammar::Modulo, Instruction::ScalarModulo##VectorType, IoMode::ReadRValue);                                                                  \
        this->AddBinaryNonCommunative(vectorType, scalarType, vectorType, Grammar::Exponent, Instruction::ScalarPow##VectorType, IoMode::ReadRValue);                                                                   \
        this->AddBinaryNonCommunative(vectorType, scalarType, core.VoidType, Grammar::AssignmentMultiply, Instruction::AssignmentScalarMultiply##VectorType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue)); \
        this->AddBinaryNonCommunative(vectorType, scalarType, core.VoidType, Grammar::AssignmentDivide, Instruction::AssignmentScalarDivide##VectorType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue));     \
        this->AddBinaryNonCommunative(vectorType, scalarType, core.VoidType, Grammar::AssignmentModulo, Instruction::AssignmentScalarModulo##VectorType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue));     \
        this->AddBinaryNonCommunative(vectorType, scalarType, core.VoidType, Grammar::AssignmentExponent, Instruction::AssignmentScalarPow##VectorType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue));      \
      }

    // Special integral operators, generic numeric operators, copy, equality, and comparison
    #define ZilchIntegralOperators(WithType)                                                                                                                                                                            \
      {                                                                                                                                                                                                                 \
        BoundType* type =  core.WithType##Type;                                                                                                                                                                         \
        this->AddUnary(type, type, Grammar::BitwiseNot, Instruction::BitwiseNot##WithType, IoMode::ReadRValue);                                                                                                         \
        this->AddBinary(type, type, Grammar::BitshiftLeft, Instruction::BitshiftLeft##WithType, IoMode::ReadRValue);                                                                                                    \
        this->AddBinary(type, type, Grammar::BitshiftRight, Instruction::BitshiftRight##WithType, IoMode::ReadRValue);                                                                                                  \
        this->AddBinary(type, type, Grammar::BitwiseOr, Instruction::BitwiseOr##WithType, IoMode::ReadRValue);                                                                                                          \
        this->AddBinary(type, type, Grammar::BitwiseXor, Instruction::BitwiseXor##WithType, IoMode::ReadRValue);                                                                                                        \
        this->AddBinary(type, type, Grammar::BitwiseAnd, Instruction::BitwiseAnd##WithType, IoMode::ReadRValue);                                                                                                        \
        this->AddBinary(type, core.VoidType, Grammar::AssignmentLeftShift, Instruction::AssignmentBitshiftLeft##WithType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue));                                    \
        this->AddBinary(type, core.VoidType, Grammar::AssignmentRightShift, Instruction::AssignmentBitshiftRight##WithType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue));                                  \
        this->AddBinary(type, core.VoidType, Grammar::AssignmentBitwiseOr, Instruction::AssignmentBitwiseOr##WithType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue));                                       \
        this->AddBinary(type, core.VoidType, Grammar::AssignmentBitwiseXor, Instruction::AssignmentBitwiseXor##WithType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue));                                     \
        this->AddBinary(type, core.VoidType, Grammar::AssignmentBitwiseAnd, Instruction::AssignmentBitwiseAnd##WithType, (IoMode::Enum)(IoMode::ReadRValue | IoMode::WriteLValue));                                     \
      }
    
    ZilchIntegralOperators(Byte);
    ZilchScalarOperators(Byte);
    ZilchIntegralOperators(Integer);
    ZilchScalarOperators(Integer);
    ZilchVectorOperators(Integer2, Integer, Boolean2);
    ZilchVectorOperators(Integer3, Integer, Boolean3);
    ZilchVectorOperators(Integer4, Integer, Boolean4);
    ZilchIntegralOperators(Integer2);
    ZilchIntegralOperators(Integer3);
    ZilchIntegralOperators(Integer4);
    ZilchScalarOperators(Real);
    ZilchVectorOperators(Real2, Real, Boolean2);
    ZilchVectorOperators(Real3, Real, Boolean3);
    ZilchVectorOperators(Real4, Real, Boolean4);
    ZilchScalarOperators(DoubleReal);
    ZilchIntegralOperators(DoubleInteger);
    ZilchScalarOperators(DoubleInteger);

    ZilchEqualityOperators(Boolean, Boolean);
    // Handle, Delegate, and Value equality operators are handled specially above

    ZilchCopyOperators(Boolean);
    // Handle, Delegate, and Value copy (assignment) operators are handled specially above

    // Boolean operators
    this->AddUnary(core.BooleanType, core.BooleanType, Grammar::LogicalNot, Instruction::LogicalNotBoolean, IoMode::ReadRValue);

    // Note: These operators have instructions marked as invalid because short circuit is handled specially
    // There is not actually an opcode/instruction that performs logical or/and
    this->AddBinary(core.BooleanType, core.BooleanType, Grammar::LogicalAnd, Instruction::InvalidInstruction, IoMode::ReadRValue);
    this->AddBinary(core.BooleanType, core.BooleanType, Grammar::LogicalOr, Instruction::InvalidInstruction, IoMode::ReadRValue);
  }

  //***************************************************************************
  void Shared::AddBinary(Type* lhs, Type* rhs, Type* result, Grammar::Enum oper, Instruction::Enum instruction, IoMode::Enum io, bool flip)
  {
    // Generate the operator information
    BinaryOperator info;
    info.Lhs = lhs;
    info.Rhs = rhs;
    info.Result = result;
    info.Operator = oper;
    info.Instruction = instruction;
    info.Io = io;
    info.FlipArguments = flip;
    info.IsValid = true;

    // Insert it into the set
    this->BinaryOperators.InsertOrError(info, "Two unary operators inserted with the same types and operator");
  }

  //***************************************************************************
  void Shared::AddBinaryCommunative(Type* type1, Type* type2, Type* result, Grammar::Enum oper, Instruction::Enum instruction, IoMode::Enum io)
  {
    // The arguments only need flipping if they're not of the same type
    bool needsFlip = !Type::IsSame(type1, type2);

    // Since the order they added it was type1, type2, then it needs no flip
    this->AddBinary(type1, type2, result, oper, instruction, io, false);

    // When we reverse the types, a flip could be necessary for the opcode
    this->AddBinary(type2, type1, result, oper, instruction, io, needsFlip);
  }

  //***************************************************************************
  void Shared::AddBinaryNonCommunative(Type* lhs, Type* rhs, Type* result, Grammar::Enum oper, Instruction::Enum instruction, IoMode::Enum io)
  {
    // Since the order they added it was type1, type2, then it needs no flip
    this->AddBinary(lhs, rhs, result, oper, instruction, io, false);
  }
  
  //***************************************************************************
  void Shared::AddBinary(Type* sameType, Type* result, Grammar::Enum oper, Instruction::Enum instruction, IoMode::Enum io)
  {
    this->AddBinary(sameType, sameType, result, oper, instruction, io, false);
  }

  //***************************************************************************
  void Shared::AddUnary(Type* operand, Type* result, Grammar::Enum oper, Instruction::Enum instruction, IoMode::Enum io)
  {
    // Generate the operator information
    UnaryOperator info;
    info.Operand = operand;
    info.Result = result;
    info.Operator = oper;
    info.Instruction = instruction;
    info.Io = io;
    info.IsValid = true;

    // Insert it into the set
    this->UnaryOperators.InsertOrError(info, "Two binary operators inserted with the same types and operator");
  }

  //***************************************************************************
  void Shared::AddPrimitiveCast(Type* fromType, Type* toType, Instruction::Enum instruction, bool canBeImplicit)
  {
    CastOperator info;
    info.From = fromType;
    info.To = toType;
    info.Operation = CastOperation::Primitive;
    info.PrimitiveInstruction = instruction;
    info.CanBeImplicit = canBeImplicit;
    info.IsValid = true;

    // All primitive operations require code generation (an instruction)
    info.RequiresCodeGeneration = true;
    
    // Insert it into the set
    this->CastOperators.InsertOrError(info, "Two cast operators inserted with the same from/to types");
    this->PrimitiveCastOperatorsFrom[fromType].PushBack(info);
  }

  //***************************************************************************
  void Shared::AddPrecedence(size_t precedence, OperatorAssociativity::Enum associativity, OperatorArity::Enum arity, Grammar::Enum oper)
  {
    // Create a structure that describes everything we need to know generically about the operator
    UntypedOperator info;
    info.Associativity = associativity;
    info.Operator = oper;
    info.Precedence = precedence;
    info.Arity = arity;
    info.IsValid = true;

    // It turns out since sometimes we use the same symbol for an operator,
    // but in some cases it can be unary or binary, then we need to key off the 'arity'
    OperatorWithArity key;
    key.Operator = oper;
    key.Arity = arity;

    // Map the operator to its precedence level, which is useful for code formatters
    this->OperatorToPrecedence.InsertOrError(key, info, "The operator was inserted twice");

    // Map the precedence level to any operators on that level, which is useful for documentation
    if (precedence >= this->PrecedenceToOperators.Size())
    {
      // Make sure we can store the precedence up to this level
      this->PrecedenceToOperators.Resize(precedence + 1);
    }

    // Get all the operators at the given precedence level (or create an empty array)
    Array<UntypedOperator>& operators = this->PrecedenceToOperators[precedence];

    // Add the operator to the precedence list, and we're done!
    operators.PushBack(info);
  }

  //***************************************************************************
  bool Shared::OperatorWithArity::operator==(const OperatorWithArity& rhs) const
  {
    return this->Operator == rhs.Operator && this->Arity == rhs.Arity;
  }

  //***************************************************************************
  size_t Shared::OperatorWithArity::Hash() const
  {
    return (((size_t)this->Operator) * 234059) ^ (this->Arity * 98361);
  }

  //***************************************************************************
  Shared& Shared::GetInstance()
  {
    // Singleton pattern
    static Shared instance;
    return instance;
  }
  
  //***************************************************************************
  BinaryOperator Shared::GetBinaryOperator(Type* lhs, Type* rhs, Grammar::Enum oper, bool allowRecursiveLookup)
  {
    // First attempt to find the operator in the specialized place
    // This MUST be first, otherwise value assignment for primitives will be a slower memcpy
    BinaryOperator finder;
    finder.Lhs = lhs;
    finder.Rhs = rhs;
    finder.Operator = oper;

    // Look for the operator in the pre-defined operator set
    BinaryOperator* result = this->BinaryOperators.FindPointer(finder);
    if (result != nullptr)
    {
      // We found it, so return it!
      return *result;
    }

    // Make sure at least one is an enum
    Type* integerType = ZilchTypeId(Integer);
    bool lhsIsEnum = Type::IsEnumOrFlagsType(lhs);
    bool rhsIsEnum = Type::IsEnumOrFlagsType(rhs);
    if (lhsIsEnum || rhsIsEnum)
    {
      // Check to see if the operation is between an enum and an integer
      // We Assign to the left, meaning the type we're converting to would be on the left
      bool isEnumToInteger = Type::IsSame(lhs, integerType);
      bool isIntegerToEnum = Type::IsSame(rhs, integerType);

      // Check to see if the operation is between the enum and another enum of the same kind
      bool isSameEnumWithEnum = lhsIsEnum && rhsIsEnum && Type::IsSame(lhs, rhs);

      // Don't allow assignments with integer to enum (otheriwse if it's between enum/enum or enum to integer, let it through)
      if (isSameEnumWithEnum || isEnumToInteger || (isIntegerToEnum && oper != Grammar::Assignment))
      {
        // Treat both as integers, and see if the operation would have been valid
        BinaryOperator integerOperator = this->GetBinaryOperator(integerType, integerType, oper);

        // If the operator is valid
        if (integerOperator.IsValid)
        {
          // Modify the operator slightly to make the inputs the same, and also make the result the enum type
          if (Type::IsSame(integerOperator.Result, integerType))
          {
            // Set the result to the enum type
            if (lhsIsEnum)
              integerOperator.Result = lhs;
            else
              integerOperator.Result = rhs;
          }

          // Return the re-purposed integer operator
          return integerOperator;
        }
      }
    }

    // Are both types handle types?
    if (Type::IsHandleType(lhs) && Type::IsHandleType(rhs))
    {
      // If we can convert the right to the left hand side (or they are the same)
      CastOperator castRightToLeft = this->GetCastOperator(rhs, lhs);
      if (castRightToLeft.IsValid && castRightToLeft.CanBeImplicit && castRightToLeft.RequiresCodeGeneration == false)
      {
        // Based on the operation...
        switch (oper)
        {
          // It is only legal for us to do assignment here, since the right
          // hand side must convert to the left hand side (eg, upcasting or null)
          case Grammar::Assignment:
            return this->HandleAssignment;
          case Grammar::Equality:
            return this->HandleEquality;
          case Grammar::Inequality:
            return this->HandleInequality;
        }
      }
      
      // If we can convert the left to the right hand side (we already know they aren't the same from above)
      CastOperator castLeftToRight = this->GetCastOperator(lhs, rhs);
      if (castLeftToRight.IsValid && castLeftToRight.CanBeImplicit && castLeftToRight.RequiresCodeGeneration == false)
      {
        // Based on the operation...
        switch (oper)
        {
          case Grammar::Equality:
            return this->HandleEquality;
          case Grammar::Inequality:
            return this->HandleInequality;
        }
      }
    }
    // Are both types the same type?
    else if (Type::IsSame(lhs, rhs))
    {
      // Note: In all of these  checks below we only need to
      // check one of them because we know they are the same!

      // Are both types value types?
      if (Type::IsValueType(lhs))
      {
        // Based on the operation...
        switch (oper)
        {
          case Grammar::Assignment:
            return this->ValueAssignment;
          case Grammar::Equality:
            return this->ValueEquality;
          case Grammar::Inequality:
            return this->ValueInequality;
        }
      }

      // Are both types delegate types?
      if (Type::IsDelegateType(lhs))
      {
        // Based on the operation...
        switch (oper)
        {
          case Grammar::Assignment:
            return this->DelegateAssignment;
          case Grammar::Equality:
            return this->DelegateEquality;
          case Grammar::Inequality:
            return this->DelegateInequality;
        }
      }

      // Are both types any types?
      if (Type::IsAnyType(lhs))
      {
        // Based on the operation...
        switch (oper)
        {
          case Grammar::Assignment:
            return this->AnyAssignment;
          case Grammar::Equality:
            return this->AnyEquality;
          case Grammar::Inequality:
            return this->AnyInequality;
        }
      }
    }

    // We got to this point and didn't find any binary operators that worked without implicit casting
    // When testing for implicit casting, it is common for us to look into our own casts
    if (allowRecursiveLookup == false)
      return BinaryOperator();

    // Lets take a look and see if implicit casting can solve our problems!
    // First, attempt to cast the right argument into the left argument type
    {
      CastOperator rightCast = this->GetCastOperator(rhs, lhs);

      // Only accept valid implicit casts
      if (rightCast.IsValid && rightCast.CanBeImplicit)
      {
        // We only look for direct 'primitive' binary operators
        BinaryOperator binaryOperatorWithCast = this->GetBinaryOperator(lhs, lhs, oper, false);
        if (binaryOperatorWithCast.IsValid)
        {
          // Let the user know that they need to cast to make this operator work
          binaryOperatorWithCast.Rhs = rhs;
          binaryOperatorWithCast.CastRhsTo = lhs;
          return binaryOperatorWithCast;
        }
      }
    }

    // We enumerate all the values the right argument can be cast into
    Array<CastOperator> rightCasts = this->GetPrimitiveCastOperatorsFrom(rhs);
    for (size_t i = 0; i < rightCasts.Size(); ++i)
    {
      CastOperator& rightCast = rightCasts[i];
      
      // We only consider implicit casts
      if (rightCast.CanBeImplicit == false)
        continue;

      // We only look for direct 'primitive' binary operators
      BinaryOperator binaryOperatorWithCast = this->GetBinaryOperator(lhs, rightCast.To, oper, false);
      if (binaryOperatorWithCast.IsValid)
      {
        // Let the user know that they need to cast to make this operator work
        binaryOperatorWithCast.Rhs = rhs;
        binaryOperatorWithCast.CastRhsTo = rightCast.To;
        return binaryOperatorWithCast;
      }
    }

    // We enumerate all the values the left argument can be cast into
    Array<CastOperator> leftCasts = this->GetPrimitiveCastOperatorsFrom(lhs);
    for (size_t i = 0; i < leftCasts.Size(); ++i)
    {
      CastOperator& leftCast = leftCasts[i];
      
      // We only consider implicit casts
      if (leftCast.CanBeImplicit == false)
        continue;

      // We only look for direct 'primitive' binary operators
      BinaryOperator binaryOperatorWithCast = this->GetBinaryOperator(leftCast.To, rhs, oper, false);
      if (binaryOperatorWithCast.IsValid)
      {
        // We can't allow implicit casting of the left argument when the operator is an l-value operator
        // For example, we never want Integer = Real to attempt to cast Integer to Real to make it work
        // This is partially mitigated by attempting the right operand first
        if ((binaryOperatorWithCast.Io & IoMode::WriteLValue) != 0)
          continue;

        // Let the user know that they need to cast to make this operator work
        binaryOperatorWithCast.Lhs = lhs;
        binaryOperatorWithCast.CastLhsTo = leftCast.To;
        return binaryOperatorWithCast;
      }
    }

    // We were unable to find anything, return an invalid operator
    return BinaryOperator();
  }
  
  //***************************************************************************
  UnaryOperator Shared::GetUnaryOperator(Type* type, Grammar::Enum oper)
  {
    // Attempt to find the result in the pre-made set of instructions
    UnaryOperator finder;
    finder.Operand = type;
    finder.Operator = oper;
    UnaryOperator* result = this->UnaryOperators.FindPointer(finder);

    // If we found any operator, return it
    if (result != nullptr)
      return *result;

    // Check to see if the type we're operating on is an enum...
    Type* integerType = ZilchTypeId(Integer);
    if (Type::IsEnumOrFlagsType(type))
    {
      // Check to see if we have an integer operator of the same kind...
      UnaryOperator integerOperator = this->GetUnaryOperator(integerType, oper);

      // If the operator is valid
      if (integerOperator.IsValid)
      {
        // Modify the operator slightly to make the inputs the same, and also make the result the enum type
        if (Type::IsSame(integerOperator.Result, integerType))
            integerOperator.Result = type;

        // Return the re-purposed integer operator
        return integerOperator;
      }
    }

    // Otherwise, we didn't find the unary operator for this type...
    return UnaryOperator();
  }

  //***************************************************************************
  Array<CastOperator> Shared::GetPrimitiveCastOperatorsFrom(Type* from)
  {
    return this->PrimitiveCastOperatorsFrom[from];
  }

  //***************************************************************************
  CastOperator Shared::GetCastOperator(Type* from, Type* to)
  {
    // If either argument is null, return an invalid cast
    if (from == nullptr || to == nullptr)
      return CastOperator();

    // Get the core library
    Core& core = Core::GetInstance();

    // If the types are the exact same, then we require no conversion at all!
    // Note: This check should always come first, to avoid situations like 'ToAny' when it's 'Any' to 'Any'
    if (Type::IsSame(from, to))
      return this->RawImplicitCast;

    // If we're attempting to convert to the 'any' type...
    if (Type::IsAnyType(to))
      return this->ToAnyCast;

    // If we're attempting to convert from the 'any' type...
    if (Type::IsAnyType(from))
      return this->FromAnyCast;

    // If we're attempting to convert from the 'AnyHandle' type to another handle...
    if (from == ZilchTypeId(Handle) && Type::IsHandleType(to))
      return this->FromAnyHandleCast;

    // If we're casting from an Integer to an enum...
    if (Type::IsSame(from, ZilchTypeId(Integer)) && Type::IsEnumOrFlagsType(to))
      return this->IntegerEnumCast;

    // If we're casting from an enum to an Integer... (can be implicit)
    if (Type::IsEnumOrFlagsType(from) && Type::IsSame(to, ZilchTypeId(Integer)))
      return this->EnumIntegerCast;

    // First attempt to find the operator in the specialized place
    CastOperator finder;
    finder.From = from;
    finder.To = to;

    // Look for the operator in the pre-defined operator set
    CastOperator* result = this->CastOperators.FindPointer(finder);
    if (result != nullptr)
      return *result;

    // If we're converting from null to any delegate type...
    if (Type::IsSame(ZilchTypeId(NullPointerType), from) && Type::IsDelegateType(to))
      return this->NullToDelegate;

    // If we're converting from null to any other handle type...
    if (Type::IsSame(ZilchTypeId(NullPointerType), from) && Type::IsHandleType(to))
      return this->RawImplicitCast;

    // If we are converting from a delegate to the 'any delegate'...
    if (Type::IsDelegateType(from) && to == core.AnyDelegateType)
      return this->RawImplicitCast;

    // If we are converting from a reference type (handle type) to the 'any handle'...
    if (Type::IsHandleType(from) && to == core.AnyHandleType)
      return this->RawImplicitCast;

    // Attempt to grab both types as reference types
    BoundType* fromBoundType = Type::GetHandleType(from);
    BoundType* toBoundType   = Type::GetHandleType(to);

    // If both types are reference types (only one level of indirection)...
    if (fromBoundType != nullptr && toBoundType != nullptr)
    {
      // If the 'from' type is a 'to' type, meaning 'from' is either the same or more derived...
      if (Type::BoundIsA(fromBoundType, toBoundType))
        return this->RawImplicitCast;

      // If the 'to' type is a 'from' type, meaning 'from' is either the same or more base... ('to' is more derived)
      if (Type::BoundIsA(toBoundType, fromBoundType))
        return this->DynamicDownCast;
    }

    // Otherwise I was not able to find the cast operator
    return CastOperator();
  }
  
  //***************************************************************************
  UntypedOperator Shared::GetOperatorPrecedence(Grammar::Enum oper, OperatorArity::Enum arity)
  {
    OperatorWithArity finder;
    finder.Operator = oper;
    finder.Arity = arity;
    return this->OperatorToPrecedence.FindValue(finder, UntypedOperator());
  }
  
  //***************************************************************************
  const Array<Array<UntypedOperator> >& Shared::GetPrecedences()
  {
    return this->PrecedenceToOperators;
  }
}
