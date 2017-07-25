/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

// Note: These macros mirror those inside of Shared and VirtualMachine (for generation of instructions)

// Copy
#define ZilchCopyInstructions(Type)               \
  ZilchEnumValue(Copy##Type)

// Equality and inequality
#define ZilchEqualityInstructions(Type)           \
  ZilchEnumValue(TestInequality##Type)            \
  ZilchEnumValue(TestEquality##Type)

// Less and greater comparison
#define ZilchComparisonInstructions(Type)         \
  ZilchEnumValue(TestLessThan##Type)              \
  ZilchEnumValue(TestLessThanOrEqualTo##Type)     \
  ZilchEnumValue(TestGreaterThan##Type)           \
  ZilchEnumValue(TestGreaterThanOrEqualTo##Type)

// Generic numeric operators, copy, equality
#define ZilchNumericInstructions(Type)            \
  ZilchCopyInstructions(Type)                     \
  ZilchEqualityInstructions(Type)                 \
  /* No instruction for unary plus */             \
  ZilchEnumValue(Negate##Type)                    \
  ZilchEnumValue(Increment##Type)                 \
  ZilchEnumValue(Decrement##Type)                 \
  ZilchEnumValue(Add##Type)                       \
  ZilchEnumValue(Subtract##Type)                  \
  ZilchEnumValue(Multiply##Type)                  \
  ZilchEnumValue(Divide##Type)                    \
  ZilchEnumValue(Modulo##Type)                    \
  ZilchEnumValue(Pow##Type)                       \
  ZilchEnumValue(AssignmentAdd##Type)             \
  ZilchEnumValue(AssignmentSubtract##Type)        \
  ZilchEnumValue(AssignmentMultiply##Type)        \
  ZilchEnumValue(AssignmentDivide##Type)          \
  ZilchEnumValue(AssignmentModulo##Type)          \
  ZilchEnumValue(AssignmentPow##Type)

// Generic numeric operators, copy, equality, comparison
#define ZilchScalarInstructions(Type)             \
  ZilchNumericInstructions(Type)                  \
  ZilchComparisonInstructions(Type)

// Vector operations, generic numeric operators, copy, equality
#define ZilchVectorInstructions(Type)             \
  ZilchNumericInstructions(Type)                  \
  ZilchComparisonInstructions(Type)               \
  ZilchEnumValue(ScalarMultiply##Type)            \
  ZilchEnumValue(ScalarDivide##Type)              \
  ZilchEnumValue(ScalarModulo##Type)              \
  ZilchEnumValue(ScalarPow##Type)                 \
  ZilchEnumValue(AssignmentScalarMultiply##Type)  \
  ZilchEnumValue(AssignmentScalarDivide##Type)    \
  ZilchEnumValue(AssignmentScalarModulo##Type)    \
  ZilchEnumValue(AssignmentScalarPow##Type)

// Special integral operators, generic numeric operators, copy, equality, and comparison
#define ZilchIntegralInstructions(Type)           \
  ZilchEnumValue(BitwiseNot##Type)                \
  ZilchEnumValue(BitshiftLeft##Type)              \
  ZilchEnumValue(BitshiftRight##Type)             \
  ZilchEnumValue(BitwiseOr##Type)                 \
  ZilchEnumValue(BitwiseXor##Type)                \
  ZilchEnumValue(BitwiseAnd##Type)                \
  ZilchEnumValue(AssignmentBitshiftLeft##Type)    \
  ZilchEnumValue(AssignmentBitshiftRight##Type)   \
  ZilchEnumValue(AssignmentBitwiseOr##Type)       \
  ZilchEnumValue(AssignmentBitwiseXor##Type)      \
  ZilchEnumValue(AssignmentBitwiseAnd##Type)


// Core instructions
ZilchEnumValue(InvalidInstruction)

ZilchEnumValue(InternalDebugBreakpoint)
ZilchEnumValue(ThrowException)
ZilchEnumValue(PropertyDelegate)

ZilchEnumValue(TypeId)

ZilchEnumValue(BeginTimeout)
ZilchEnumValue(EndTimeout)

ZilchEnumValue(BeginScope)
ZilchEnumValue(EndScope)

ZilchEnumValue(ToHandle)

ZilchEnumValue(BeginStringBuilder)
ZilchEnumValue(EndStringBuilder)
ZilchEnumValue(AddToStringBuilder)

ZilchEnumValue(CreateInstanceDelegate)
ZilchEnumValue(CreateStaticDelegate)

ZilchEnumValue(IfFalseRelativeGoTo)
ZilchEnumValue(IfTrueRelativeGoTo)
ZilchEnumValue(RelativeGoTo)

ZilchEnumValue(Return)
ZilchEnumValue(PrepForFunctionCall)
ZilchEnumValue(FunctionCall)

ZilchEnumValue(NewObject)
ZilchEnumValue(LocalObject)
ZilchEnumValue(DeleteObject)

// Primitive type instructions
ZilchIntegralInstructions(Byte)
ZilchScalarInstructions(Byte)
ZilchIntegralInstructions(Integer)
ZilchScalarInstructions(Integer)
ZilchVectorInstructions(Integer2)
ZilchVectorInstructions(Integer3)
ZilchVectorInstructions(Integer4)
ZilchIntegralInstructions(Integer2)
ZilchIntegralInstructions(Integer3)
ZilchIntegralInstructions(Integer4)
ZilchScalarInstructions(Real)
ZilchVectorInstructions(Real2)
ZilchVectorInstructions(Real3)
ZilchVectorInstructions(Real4)
ZilchScalarInstructions(DoubleReal)
ZilchIntegralInstructions(DoubleInteger)
ZilchScalarInstructions(DoubleInteger)

ZilchEqualityInstructions(Boolean)
ZilchEqualityInstructions(Handle)
ZilchEqualityInstructions(Delegate)
ZilchEqualityInstructions(Any)
ZilchEqualityInstructions(Value)

ZilchCopyInstructions(Boolean)
ZilchCopyInstructions(Any)
ZilchCopyInstructions(Handle)
ZilchCopyInstructions(Delegate)
ZilchCopyInstructions(Value)

ZilchEnumValue(LogicalNotBoolean)

ZilchEnumValue(ConvertByteToReal)
ZilchEnumValue(ConvertByteToBoolean)
ZilchEnumValue(ConvertByteToInteger)
ZilchEnumValue(ConvertByteToDoubleInteger)
ZilchEnumValue(ConvertByteToDoubleReal)
ZilchEnumValue(ConvertIntegerToReal)
ZilchEnumValue(ConvertIntegerToBoolean)
ZilchEnumValue(ConvertIntegerToByte)
ZilchEnumValue(ConvertIntegerToDoubleInteger)
ZilchEnumValue(ConvertIntegerToDoubleReal)
ZilchEnumValue(ConvertRealToInteger)
ZilchEnumValue(ConvertRealToBoolean)
ZilchEnumValue(ConvertRealToByte)
ZilchEnumValue(ConvertRealToDoubleInteger)
ZilchEnumValue(ConvertRealToDoubleReal)
ZilchEnumValue(ConvertBooleanToInteger)
ZilchEnumValue(ConvertBooleanToReal)
ZilchEnumValue(ConvertBooleanToByte)
ZilchEnumValue(ConvertBooleanToDoubleInteger)
ZilchEnumValue(ConvertBooleanToDoubleReal)
ZilchEnumValue(ConvertDoubleIntegerToReal)
ZilchEnumValue(ConvertDoubleIntegerToBoolean)
ZilchEnumValue(ConvertDoubleIntegerToByte)
ZilchEnumValue(ConvertDoubleIntegerToInteger)
ZilchEnumValue(ConvertDoubleIntegerToDoubleReal)
ZilchEnumValue(ConvertDoubleRealToReal)
ZilchEnumValue(ConvertDoubleRealToBoolean)
ZilchEnumValue(ConvertDoubleRealToByte)
ZilchEnumValue(ConvertDoubleRealToInteger)
ZilchEnumValue(ConvertDoubleRealToDoubleInteger)

ZilchEnumValue(ConvertInteger2ToReal2)
ZilchEnumValue(ConvertInteger2ToBoolean2)
ZilchEnumValue(ConvertReal2ToInteger2)
ZilchEnumValue(ConvertReal2ToBoolean2)
ZilchEnumValue(ConvertBoolean2ToInteger2)
ZilchEnumValue(ConvertBoolean2ToReal2)

ZilchEnumValue(ConvertInteger3ToReal3)
ZilchEnumValue(ConvertInteger3ToBoolean3)
ZilchEnumValue(ConvertReal3ToInteger3)
ZilchEnumValue(ConvertReal3ToBoolean3)
ZilchEnumValue(ConvertBoolean3ToInteger3)
ZilchEnumValue(ConvertBoolean3ToReal3)

ZilchEnumValue(ConvertInteger4ToReal4)
ZilchEnumValue(ConvertInteger4ToBoolean4)
ZilchEnumValue(ConvertReal4ToInteger4)
ZilchEnumValue(ConvertReal4ToBoolean4)
ZilchEnumValue(ConvertBoolean4ToInteger4)
ZilchEnumValue(ConvertBoolean4ToReal4)

ZilchEnumValue(ConvertStringToStringRangeExtended)

ZilchEnumValue(ConvertDowncast)
ZilchEnumValue(ConvertToAny)
ZilchEnumValue(ConvertFromAny)
ZilchEnumValue(AnyDynamicMemberGet)
ZilchEnumValue(AnyDynamicMemberSet)
