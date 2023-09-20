// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Resolves a binary operator node given the expected return type.
void ResolveBinaryOp(RaverieSpirVFrontEnd* translator, Raverie::BinaryOperatorNode* binaryOpNode, OpType opType, RaverieSpirVFrontEndContext* context)
{
  if (binaryOpNode->OperatorInfo.Io & Raverie::IoMode::WriteLValue)
    translator->PerformBinaryAssignmentOp(binaryOpNode, opType, context);
  else
    translator->PerformBinaryOp(binaryOpNode, opType, context);
}

// Resolves a binary operator node where the lhs and rhs of the node have
// already been resolved. This can be necessary when one of the sides in the
// node has to undergo a transformation first (e.g vector / scalar has to first
// promote the scalar to a vector)
void ResolveBinaryOp(RaverieSpirVFrontEnd* translator, Raverie::BinaryOperatorNode* binaryOpNode, OpType opType, IRaverieShaderIR* lhs, IRaverieShaderIR* rhs, RaverieSpirVFrontEndContext* context)
{
  if (binaryOpNode->OperatorInfo.Io & Raverie::IoMode::WriteLValue)
    translator->PerformBinaryAssignmentOp(binaryOpNode, opType, lhs, rhs, context);
  else
    translator->PerformBinaryOp(binaryOpNode, opType, lhs, rhs, context);
}

void ResolveUnaryOperator(RaverieSpirVFrontEnd* translator, Raverie::UnaryOperatorNode* unaryOpNode, OpType opType, RaverieSpirVFrontEndContext* context)
{
  translator->PerformUnaryOp(unaryOpNode, opType, context);
}

template <OpType opType>
void ResolveIntIncDecUnaryOperator(RaverieSpirVFrontEnd* translator, Raverie::UnaryOperatorNode* unaryOpNode, RaverieSpirVFrontEndContext* context)
{
  // Create the int literal '1'
  IRaverieShaderIR* constantOne = translator->GetIntegerConstant(1, context);
  translator->PerformUnaryIncDecOp(unaryOpNode, constantOne, opType, context);
}

template <OpType opType>
void ResolveFloatIncDecUnaryOperator(RaverieSpirVFrontEnd* translator, Raverie::UnaryOperatorNode* unaryOpNode, RaverieSpirVFrontEndContext* context)
{
  // Create the float literal '1'
  RaverieShaderIRType* floatType = translator->FindType(RaverieTypeId(float), unaryOpNode, context);
  IRaverieShaderIR* constantOne = translator->GetConstant(floatType, 1.0f, context);
  translator->PerformUnaryIncDecOp(unaryOpNode, constantOne, opType, context);
}

void ResolveFMod(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  ResolveStaticBinaryFunctionOp(translator, functionCallNode, spv::OpFMod, context);
}

void ResolveDot(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  ResolveStaticBinaryFunctionOp(translator, functionCallNode, spv::OpDot, context);
}

// Resolves vector op vector(scalar). Needed for some operations like vector /
// scalar which has to turn into vector / vector(scalar) since the componentized
// operations don't exist.
template <OpType opType>
void ResolveVectorOpSplatScalar(RaverieSpirVFrontEnd* translator, Raverie::BinaryOperatorNode* binaryOpNode, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Get the vector operand
  IRaverieShaderIR* vectorOperand = translator->WalkAndGetResult(binaryOpNode->LeftOperand, context);

  // Convert the scalar operand into a vector of the same type as the left hand
  // side
  RaverieShaderIRType* vectorType = translator->FindType(binaryOpNode->LeftOperand->ResultType, binaryOpNode);
  RaverieShaderIROp* scalarOperand = translator->WalkAndGetValueTypeResult(binaryOpNode->RightOperand, context);
  RaverieShaderIROp* splattedScalarOperand = translator->ConstructCompositeFromScalar(currentBlock, vectorType, scalarOperand, context);

  // Perform the op
  ResolveBinaryOp(translator, binaryOpNode, opType, vectorOperand, splattedScalarOperand, context);
}

template <OpType opType>
void ResolveSimpleStaticBinaryFunctionOp(RaverieSpirVFrontEnd* translator,
                                         Raverie::FunctionCallNode* functionCallNode,
                                         Raverie::MemberAccessNode* memberAccessNode,
                                         RaverieSpirVFrontEndContext* context)
{
  ResolveStaticBinaryFunctionOp(translator, functionCallNode, opType, context);
}

// Some binary functions are special and have to be flipped due to the column
// vs. row major differences of raverie and spirv.
void ResolveFlippedStaticBinaryFunctionOp(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, OpType opType, RaverieSpirVFrontEndContext* context)
{
  // Get the result type
  RaverieShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  // Walk each operand
  IRaverieShaderIR* operand1 = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  IRaverieShaderIR* operand2 = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[1], context);

  // Generate the fmod op
  IRaverieShaderIR* operationOp = translator->BuildCurrentBlockIROp(opType, resultType, operand2, operand1, context);
  context->PushIRStack(operationOp);
}

void ResolveMatrixTimesVector(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  ResolveFlippedStaticBinaryFunctionOp(translator, functionCallNode, OpType::OpVectorTimesMatrix, context);
}

void ResolveMatrixTimesMatrix(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  ResolveFlippedStaticBinaryFunctionOp(translator, functionCallNode, OpType::OpMatrixTimesMatrix, context);
}

void ResolveMatrixTranspose(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  // Get the result type
  RaverieShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);
  // Walk each operand
  IRaverieShaderIR* operand = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);

  // Generate the transpose op
  IRaverieShaderIR* operationOp = translator->BuildCurrentBlockIROp(OpType::OpTranspose, resultType, operand, context);
  context->PushIRStack(operationOp);
}

// Register function callbacks for the various arithmetic operators (see
// Arithmetic Instructions in the spir-v spec).
void RegisterArithmeticOps(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, RaverieTypeGroups& types)
{
  Raverie::Core& core = Raverie::Core::GetInstance();
  Raverie::BoundType* mathType = core.MathType;
  TypeResolvers& mathTypeResolver = shaderLibrary->mTypeResolvers[mathType];
  OperatorResolvers& opResolvers = shaderLibrary->mOperatorResolvers;

  Raverie::BoundType* realType = core.RealType;
  Raverie::BoundType* intType = core.IntegerType;

  opResolvers.RegisterUnaryOpResolver(intType, Raverie::Grammar::Increment, ResolveIntIncDecUnaryOperator<OpType::OpIAdd>);
  opResolvers.RegisterUnaryOpResolver(intType, Raverie::Grammar::Decrement, ResolveIntIncDecUnaryOperator<OpType::OpISub>);
  opResolvers.RegisterUnaryOpResolver(realType, Raverie::Grammar::Increment, ResolveFloatIncDecUnaryOperator<OpType::OpFAdd>);
  opResolvers.RegisterUnaryOpResolver(realType, Raverie::Grammar::Decrement, ResolveFloatIncDecUnaryOperator<OpType::OpFSub>);

  // Register ops that are on all float vector types
  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mRealVectorTypes[i];
    String raverieTypeName = raverieType->ToString();

    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::AssignmentAdd, ResolveBinaryOperator<spv::OpFAdd>);
    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::AssignmentSubtract, ResolveBinaryOperator<OpType::OpFSub>);
    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::AssignmentMultiply, ResolveBinaryOperator<OpType::OpFMul>);
    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::AssignmentDivide, ResolveBinaryOperator<OpType::OpFDiv>);
    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::AssignmentModulo, ResolveBinaryOperator<OpType::OpFMod>);

    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::Add, ResolveBinaryOperator<spv::OpFAdd>);
    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::Subtract, ResolveBinaryOperator<OpType::OpFSub>);
    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::Multiply, ResolveBinaryOperator<OpType::OpFMul>);
    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::Divide, ResolveBinaryOperator<OpType::OpFDiv>);
    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::Modulo, ResolveBinaryOperator<OpType::OpFMod>);

    opResolvers.RegisterUnaryOpResolver(raverieType, Raverie::Grammar::Subtract, ResolveUnaryOperator<OpType::OpFNegate>);

    mathTypeResolver.RegisterFunctionResolver(GetStaticFunction(mathType, "FMod", raverieTypeName, raverieTypeName), ResolveFMod);
  }

  // Register ops that are only on float vector types (no scalars). Some of
  // these are because of raverie and not spirv.
  for (size_t i = 1; i < types.mRealVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mRealVectorTypes[i];
    String raverieTypeName = raverieType->ToString();

    mathTypeResolver.RegisterFunctionResolver(GetStaticFunction(mathType, "Dot", raverieTypeName, raverieTypeName), ResolveDot);

    opResolvers.RegisterBinaryOpResolver(raverieType, realType, Raverie::Grammar::Multiply, ResolveBinaryOperator<spv::OpVectorTimesScalar>);
    opResolvers.RegisterBinaryOpResolver(raverieType, realType, Raverie::Grammar::AssignmentMultiply, ResolveBinaryOperator<OpType::OpVectorTimesScalar>);
    opResolvers.RegisterBinaryOpResolver(raverieType, realType, Raverie::Grammar::Divide, ResolveVectorOpSplatScalar<OpType::OpFDiv>);
    opResolvers.RegisterBinaryOpResolver(raverieType, realType, Raverie::Grammar::AssignmentDivide, ResolveVectorOpSplatScalar<OpType::OpFDiv>);
  }

  // Register ops that are on all integer vector types
  for (size_t i = 0; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mIntegerVectorTypes[i];
    String raverieTypeName = raverieType->ToString();

    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::AssignmentAdd, ResolveBinaryOperator<spv::OpIAdd>);
    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::AssignmentSubtract, ResolveBinaryOperator<OpType::OpISub>);
    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::AssignmentMultiply, ResolveBinaryOperator<OpType::OpIMul>);
    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::AssignmentDivide, ResolveBinaryOperator<OpType::OpSDiv>);
    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::AssignmentModulo, ResolveBinaryOperator<OpType::OpSMod>);

    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::Add, ResolveBinaryOperator<spv::OpIAdd>);
    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::Subtract, ResolveBinaryOperator<OpType::OpISub>);
    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::Multiply, ResolveBinaryOperator<OpType::OpIMul>);
    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::Divide, ResolveBinaryOperator<OpType::OpSDiv>);
    opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::Modulo, ResolveBinaryOperator<OpType::OpSMod>);

    opResolvers.RegisterUnaryOpResolver(raverieType, Raverie::Grammar::Subtract, ResolveUnaryOperator<OpType::OpSNegate>);
  }

  // Register ops that are only on int vector types (no scalars). Some of these
  // are because of raverie and not spirv.
  // @JoshD: SpirV doesn't have any actual vector operations on integers.
  // Some could be supported using more complicated instructions (e.g. vector *
  // scalar = vector * vector(scalar))
  for (size_t i = 1; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mIntegerVectorTypes[i];
    String raverieTypeName = raverieType->ToString();

    // VectorTimesScalar is only on real types
    // RaverieTypePair vectorScalarTypePair(raverieType, intType);
    // translator->mBinaryOpInstructions[BinaryOpTypeId(vectorScalarTypePair,
    // Raverie::Grammar::Multiply)] = OpType::OpVectorTimesScalar;
    // translator->mBinaryOpInstructions[BinaryOpTypeId(vectorScalarTypePair,
    // Raverie::Grammar::AssignmentMultiply)] = OpType::OpVectorTimesScalar;
  }

  // Register all real matrix instructions.
  for (size_t y = 2; y <= 4; ++y)
  {
    for (size_t x = 2; x <= 4; ++x)
    {
      Raverie::BoundType* raverieType = types.GetMatrixType(y, x);
      String raverieTypeName = raverieType->ToString();
      Raverie::BoundType* vectorType = types.mRealVectorTypes[x - 1];

      opResolvers.RegisterBinaryOpResolver(raverieType, raverieType, Raverie::Grammar::Multiply, ResolveBinaryOperator<spv::OpMatrixTimesScalar>);
      mathTypeResolver.RegisterFunctionResolver(GetStaticFunction(mathType, "Transpose", raverieTypeName), ResolveMatrixTranspose);
      // Matrix times vector
      mathTypeResolver.RegisterFunctionResolver(GetStaticFunction(mathType, "Multiply", raverieTypeName, vectorType->ToString()), ResolveMatrixTimesVector);

      // Iterate over all of the other matrix dimensions to make the
      // multiplication functions (e.g. Real2x3 * real3x2, Real2x3 * Real3x3,
      // etc...)
      for (size_t z = 2; z <= 4; ++z)
      {
        Raverie::BoundType* rhsMatrixType = types.GetMatrixType(x, z);
        mathTypeResolver.RegisterFunctionResolver(GetStaticFunction(mathType, "Multiply", raverieTypeName, rhsMatrixType->ToString()), ResolveMatrixTimesMatrix);
      }
    }
  }
}

} // namespace Raverie
