// MIT Licensed (see LICENSE.md).

// Note: These macros mirror those inside of Shared and VirtualMachine (for
// generation of instructions)

// Copy
#define RaverieCopyInstructions(Type) RaverieEnumValue(Copy##Type)

// Equality and inequality
#define RaverieEqualityInstructions(Type) RaverieEnumValue(TestInequality##Type) RaverieEnumValue(TestEquality##Type)

// Less and greater comparison
#define RaverieComparisonInstructions(Type)                                                                              \
  RaverieEnumValue(TestLessThan##Type) RaverieEnumValue(TestLessThanOrEqualTo##Type) RaverieEnumValue(TestGreaterThan##Type) \
      RaverieEnumValue(TestGreaterThanOrEqualTo##Type)

// Generic numeric operators, copy, equality
#define RaverieNumericInstructions(Type)                                                                                 \
  RaverieCopyInstructions(Type) RaverieEqualityInstructions(Type) /* No instruction for unary plus */                      \
      RaverieEnumValue(Negate##Type) RaverieEnumValue(Increment##Type) RaverieEnumValue(Decrement##Type)                     \
          RaverieEnumValue(Add##Type) RaverieEnumValue(Subtract##Type) RaverieEnumValue(Multiply##Type)                      \
              RaverieEnumValue(Divide##Type) RaverieEnumValue(Modulo##Type) RaverieEnumValue(Pow##Type)                      \
                  RaverieEnumValue(AssignmentAdd##Type) RaverieEnumValue(AssignmentSubtract##Type)                         \
                      RaverieEnumValue(AssignmentMultiply##Type) RaverieEnumValue(AssignmentDivide##Type)                  \
                          RaverieEnumValue(AssignmentModulo##Type) RaverieEnumValue(AssignmentPow##Type)

// Generic numeric operators, copy, equality, comparison
#define RaverieScalarInstructions(Type) RaverieNumericInstructions(Type) RaverieComparisonInstructions(Type)

// Vector operations, generic numeric operators, copy, equality
#define RaverieVectorInstructions(Type)                                                                                  \
  RaverieNumericInstructions(Type) RaverieComparisonInstructions(Type) RaverieEnumValue(ScalarMultiply##Type)                \
      RaverieEnumValue(ScalarDivide##Type) RaverieEnumValue(ScalarModulo##Type) RaverieEnumValue(ScalarPow##Type)            \
          RaverieEnumValue(AssignmentScalarMultiply##Type) RaverieEnumValue(AssignmentScalarDivide##Type)                  \
              RaverieEnumValue(AssignmentScalarModulo##Type) RaverieEnumValue(AssignmentScalarPow##Type)

// Special integral operators, generic numeric operators, copy, equality, and
// comparison
#define RaverieIntegralInstructions(Type)                                                                                \
  RaverieEnumValue(BitwiseNot##Type) RaverieEnumValue(BitshiftLeft##Type) RaverieEnumValue(BitshiftRight##Type)              \
      RaverieEnumValue(BitwiseOr##Type) RaverieEnumValue(BitwiseXor##Type) RaverieEnumValue(BitwiseAnd##Type)                \
          RaverieEnumValue(AssignmentBitshiftLeft##Type) RaverieEnumValue(AssignmentBitshiftRight##Type)                   \
              RaverieEnumValue(AssignmentBitwiseOr##Type) RaverieEnumValue(AssignmentBitwiseXor##Type)                     \
                  RaverieEnumValue(AssignmentBitwiseAnd##Type)

// Core instructions
RaverieEnumValue(InvalidInstruction)

    RaverieEnumValue(InternalDebugBreakpoint) RaverieEnumValue(ThrowException) RaverieEnumValue(PropertyDelegate)

        RaverieEnumValue(TypeId)

            RaverieEnumValue(BeginTimeout) RaverieEnumValue(EndTimeout)

                RaverieEnumValue(BeginScope) RaverieEnumValue(EndScope)

                    RaverieEnumValue(ToHandle)

                        RaverieEnumValue(BeginStringBuilder) RaverieEnumValue(EndStringBuilder)
                            RaverieEnumValue(AddToStringBuilder)

                                RaverieEnumValue(CreateInstanceDelegate) RaverieEnumValue(CreateStaticDelegate)

                                    RaverieEnumValue(IfFalseRelativeGoTo) RaverieEnumValue(IfTrueRelativeGoTo)
                                        RaverieEnumValue(RelativeGoTo)

                                            RaverieEnumValue(Return) RaverieEnumValue(PrepForFunctionCall)
                                                RaverieEnumValue(FunctionCall)

                                                    RaverieEnumValue(NewObject) RaverieEnumValue(LocalObject)
                                                        RaverieEnumValue(DeleteObject)

    // Primitive type instructions
    RaverieIntegralInstructions(Byte) RaverieScalarInstructions(Byte) RaverieIntegralInstructions(
        Integer) RaverieScalarInstructions(Integer) RaverieVectorInstructions(Integer2) RaverieVectorInstructions(Integer3)
        RaverieVectorInstructions(Integer4) RaverieIntegralInstructions(Integer2) RaverieIntegralInstructions(
            Integer3) RaverieIntegralInstructions(Integer4) RaverieScalarInstructions(Real) RaverieVectorInstructions(Real2)
            RaverieVectorInstructions(Real3) RaverieVectorInstructions(Real4) RaverieScalarInstructions(
                DoubleReal) RaverieIntegralInstructions(DoubleInteger) RaverieScalarInstructions(DoubleInteger)

                RaverieEqualityInstructions(Boolean) RaverieEqualityInstructions(Handle) RaverieEqualityInstructions(
                    Delegate) RaverieEqualityInstructions(Any) RaverieEqualityInstructions(Value)

                    RaverieCopyInstructions(Boolean) RaverieCopyInstructions(Any) RaverieCopyInstructions(
                        Handle) RaverieCopyInstructions(Delegate) RaverieCopyInstructions(Value)

                        RaverieEnumValue(LogicalNotBoolean)

                            RaverieEnumValue(ConvertByteToReal) RaverieEnumValue(ConvertByteToBoolean) RaverieEnumValue(
                                ConvertByteToInteger) RaverieEnumValue(ConvertByteToDoubleInteger) RaverieEnumValue(ConvertByteToDoubleReal)
                                RaverieEnumValue(ConvertIntegerToReal) RaverieEnumValue(ConvertIntegerToBoolean) RaverieEnumValue(
                                    ConvertIntegerToByte) RaverieEnumValue(ConvertIntegerToDoubleInteger)
                                    RaverieEnumValue(ConvertIntegerToDoubleReal) RaverieEnumValue(ConvertRealToInteger) RaverieEnumValue(
                                        ConvertRealToBoolean) RaverieEnumValue(ConvertRealToByte) RaverieEnumValue(ConvertRealToDoubleInteger)
                                        RaverieEnumValue(ConvertRealToDoubleReal) RaverieEnumValue(ConvertBooleanToInteger) RaverieEnumValue(
                                            ConvertBooleanToReal) RaverieEnumValue(ConvertBooleanToByte)
                                            RaverieEnumValue(ConvertBooleanToDoubleInteger) RaverieEnumValue(
                                                ConvertBooleanToDoubleReal) RaverieEnumValue(ConvertDoubleIntegerToReal)
                                                RaverieEnumValue(ConvertDoubleIntegerToBoolean) RaverieEnumValue(
                                                    ConvertDoubleIntegerToByte) RaverieEnumValue(ConvertDoubleIntegerToInteger)
                                                    RaverieEnumValue(ConvertDoubleIntegerToDoubleReal) RaverieEnumValue(
                                                        ConvertDoubleRealToReal) RaverieEnumValue(ConvertDoubleRealToBoolean)
                                                        RaverieEnumValue(ConvertDoubleRealToByte) RaverieEnumValue(
                                                            ConvertDoubleRealToInteger) RaverieEnumValue(ConvertDoubleRealToDoubleInteger)

                                                            RaverieEnumValue(ConvertInteger2ToReal2) RaverieEnumValue(
                                                                ConvertInteger2ToBoolean2) RaverieEnumValue(ConvertReal2ToInteger2)
                                                                RaverieEnumValue(ConvertReal2ToBoolean2) RaverieEnumValue(
                                                                    ConvertBoolean2ToInteger2) RaverieEnumValue(ConvertBoolean2ToReal2)

                                                                    RaverieEnumValue(ConvertInteger3ToReal3) RaverieEnumValue(
                                                                        ConvertInteger3ToBoolean3) RaverieEnumValue(ConvertReal3ToInteger3)
                                                                        RaverieEnumValue(ConvertReal3ToBoolean3) RaverieEnumValue(
                                                                            ConvertBoolean3ToInteger3) RaverieEnumValue(ConvertBoolean3ToReal3)

                                                                            RaverieEnumValue(ConvertInteger4ToReal4) RaverieEnumValue(
                                                                                ConvertInteger4ToBoolean4)
                                                                                RaverieEnumValue(ConvertReal4ToInteger4) RaverieEnumValue(
                                                                                    ConvertReal4ToBoolean4)
                                                                                    RaverieEnumValue(
                                                                                        ConvertBoolean4ToInteger4)
                                                                                        RaverieEnumValue(
                                                                                            ConvertBoolean4ToReal4)

                                                                                            RaverieEnumValue(
                                                                                                ConvertStringToStringRangeExtended)

                                                                                                RaverieEnumValue(
                                                                                                    ConvertDowncast)
                                                                                                    RaverieEnumValue(
                                                                                                        ConvertToAny)
                                                                                                        RaverieEnumValue(
                                                                                                            ConvertFromAny)
                                                                                                            RaverieEnumValue(
                                                                                                                AnyDynamicMemberGet)
                                                                                                                RaverieEnumValue(
                                                                                                                    AnyDynamicMemberSet)
