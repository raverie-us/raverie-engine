// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// Resolves a binary operator node given the expected return type.
void ResolveBinaryOp(ZilchSpirVFrontEnd* translator,
                     Zilch::BinaryOperatorNode* binaryOpNode,
                     OpType opType,
                     ZilchSpirVFrontEndContext* context)
{
  if (binaryOpNode->OperatorInfo.Io & Zilch::IoMode::WriteLValue)
    translator->PerformBinaryAssignmentOp(binaryOpNode, opType, context);
  else
    translator->PerformBinaryOp(binaryOpNode, opType, context);
}

// Resolves a binary operator node where the lhs and rhs of the node have
// already been resolved. This can be necessary when one of the sides in the
// node has to undergo a transformation first (e.g vector / scalar has to first
// promote the scalar to a vector)
void ResolveBinaryOp(ZilchSpirVFrontEnd* translator,
                     Zilch::BinaryOperatorNode* binaryOpNode,
                     OpType opType,
                     IZilchShaderIR* lhs,
                     IZilchShaderIR* rhs,
                     ZilchSpirVFrontEndContext* context)
{
  if (binaryOpNode->OperatorInfo.Io & Zilch::IoMode::WriteLValue)
    translator->PerformBinaryAssignmentOp(binaryOpNode, opType, lhs, rhs, context);
  else
    translator->PerformBinaryOp(binaryOpNode, opType, lhs, rhs, context);
}

template <OpType opType>
void ResolveIntIncDecUnaryOperator(ZilchSpirVFrontEnd* translator,
                                   Zilch::UnaryOperatorNode* unaryOpNode,
                                   ZilchSpirVFrontEndContext* context)
{
  // Create the int literal '1'
  IZilchShaderIR* constantOne = translator->GetIntegerConstant(1, context);
  translator->PerformUnaryIncDecOp(unaryOpNode, constantOne, opType, context);
}

template <OpType opType>
void ResolveFloatIncDecUnaryOperator(ZilchSpirVFrontEnd* translator,
                                     Zilch::UnaryOperatorNode* unaryOpNode,
                                     ZilchSpirVFrontEndContext* context)
{
  // Create the float literal '1'
  ZilchShaderIRType* floatType = translator->FindType(ZilchTypeId(float), unaryOpNode, context);
  IZilchShaderIR* constantOne = translator->GetConstant(floatType, 1.0f, context);
  translator->PerformUnaryIncDecOp(unaryOpNode, constantOne, opType, context);
}

void ResolveFMod(ZilchSpirVFrontEnd* translator,
                 Zilch::FunctionCallNode* functionCallNode,
                 Zilch::MemberAccessNode* memberAccessNode,
                 ZilchSpirVFrontEndContext* context)
{
  ResolveStaticBinaryFunctionOp(translator, functionCallNode, spv::OpFMod, context);
}

void ResolveDot(ZilchSpirVFrontEnd* translator,
                Zilch::FunctionCallNode* functionCallNode,
                Zilch::MemberAccessNode* memberAccessNode,
                ZilchSpirVFrontEndContext* context)
{
  ResolveStaticBinaryFunctionOp(translator, functionCallNode, spv::OpDot, context);
}

// Resolves vector op vector(scalar). Needed for some operations like vector /
// scalar which has to turn into vector / vector(scalar) since the componentized
// operations don't exist.
template <OpType opType>
void ResolveVectorOpSplatScalar(ZilchSpirVFrontEnd* translator,
                                Zilch::BinaryOperatorNode* binaryOpNode,
                                ZilchSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Get the vector operand
  IZilchShaderIR* vectorOperand = translator->WalkAndGetResult(binaryOpNode->LeftOperand, context);

  // Convert the scalar operand into a vector of the same type as the left hand
  // side
  ZilchShaderIRType* vectorType = translator->FindType(binaryOpNode->LeftOperand->ResultType, binaryOpNode);
  ZilchShaderIROp* scalarOperand = translator->WalkAndGetValueTypeResult(binaryOpNode->RightOperand, context);
  ZilchShaderIROp* splattedScalarOperand =
      translator->ConstructCompositeFromScalar(currentBlock, vectorType, scalarOperand, context);

  // Perform the op
  ResolveBinaryOp(translator, binaryOpNode, opType, vectorOperand, splattedScalarOperand, context);
}

template <OpType opType>
void ResolveSimpleStaticBinaryFunctionOp(ZilchSpirVFrontEnd* translator,
                                         Zilch::FunctionCallNode* functionCallNode,
                                         Zilch::MemberAccessNode* memberAccessNode,
                                         ZilchSpirVFrontEndContext* context)
{
  ResolveStaticBinaryFunctionOp(translator, functionCallNode, opType, context);
}

// Some binary functions are special and have to be flipped due to the column
// vs. row major differences of zilch and spirv.
void ResolveFlippedStaticBinaryFunctionOp(ZilchSpirVFrontEnd* translator,
                                          Zilch::FunctionCallNode* functionCallNode,
                                          OpType opType,
                                          ZilchSpirVFrontEndContext* context)
{
  // Get the result type
  ZilchShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  // Walk each operand
  IZilchShaderIR* operand1 = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  IZilchShaderIR* operand2 = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[1], context);

  // Generate the fmod op
  IZilchShaderIR* operationOp = translator->BuildCurrentBlockIROp(opType, resultType, operand2, operand1, context);
  context->PushIRStack(operationOp);
}

void ResolveMatrixTimesVector(ZilchSpirVFrontEnd* translator,
                              Zilch::FunctionCallNode* functionCallNode,
                              Zilch::MemberAccessNode* memberAccessNode,
                              ZilchSpirVFrontEndContext* context)
{
  ResolveFlippedStaticBinaryFunctionOp(translator, functionCallNode, OpType::OpVectorTimesMatrix, context);
}

void ResolveMatrixTimesMatrix(ZilchSpirVFrontEnd* translator,
                              Zilch::FunctionCallNode* functionCallNode,
                              Zilch::MemberAccessNode* memberAccessNode,
                              ZilchSpirVFrontEndContext* context)
{
  ResolveFlippedStaticBinaryFunctionOp(translator, functionCallNode, OpType::OpMatrixTimesMatrix, context);
}

void ResolveMatrixTranspose(ZilchSpirVFrontEnd* translator,
                            Zilch::FunctionCallNode* functionCallNode,
                            Zilch::MemberAccessNode* memberAccessNode,
                            ZilchSpirVFrontEndContext* context)
{
  // Get the result type
  ZilchShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);
  // Walk each operand
  IZilchShaderIR* operand = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);

  // Generate the transpose op
  IZilchShaderIR* operationOp = translator->BuildCurrentBlockIROp(OpType::OpTranspose, resultType, operand, context);
  context->PushIRStack(operationOp);
}

// Register function callbacks for the various arithmetic operators (see
// Arithmetic Instructions in the spir-v spec).
void RegisterArithmeticOps(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, TypeGroups& types)
{
  Zilch::Core& core = Zilch::Core::GetInstance();
  Zilch::BoundType* mathType = core.MathType;
  TypeResolvers& mathTypeResolver = shaderLibrary->mTypeResolvers[mathType];
  OperatorResolvers& opResolvers = shaderLibrary->mOperatorResolvers;

  Zilch::BoundType* realType = core.RealType;
  Zilch::BoundType* intType = core.IntegerType;

  opResolvers.RegisterUnaryOpResolver(
      intType, Zilch::Grammar::Increment, ResolveIntIncDecUnaryOperator<OpType::OpIAdd>);
  opResolvers.RegisterUnaryOpResolver(
      intType, Zilch::Grammar::Decrement, ResolveIntIncDecUnaryOperator<OpType::OpISub>);
  opResolvers.RegisterUnaryOpResolver(
      realType, Zilch::Grammar::Increment, ResolveFloatIncDecUnaryOperator<OpType::OpFAdd>);
  opResolvers.RegisterUnaryOpResolver(
      realType, Zilch::Grammar::Decrement, ResolveFloatIncDecUnaryOperator<OpType::OpFSub>);

  // Register ops that are on all float vector types
  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* zilchType = types.mRealVectorTypes[i]->mZilchType;
    String zilchTypeName = zilchType->ToString();

    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::AssignmentAdd, ResolveBinaryOperator<spv::OpFAdd>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::AssignmentSubtract, ResolveBinaryOperator<OpType::OpFSub>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::AssignmentMultiply, ResolveBinaryOperator<OpType::OpFMul>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::AssignmentDivide, ResolveBinaryOperator<OpType::OpFDiv>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::AssignmentModulo, ResolveBinaryOperator<OpType::OpFMod>);

    opResolvers.RegisterBinaryOpResolver(zilchType, zilchType, Zilch::Grammar::Add, ResolveBinaryOperator<spv::OpFAdd>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::Subtract, ResolveBinaryOperator<OpType::OpFSub>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::Multiply, ResolveBinaryOperator<OpType::OpFMul>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::Divide, ResolveBinaryOperator<OpType::OpFDiv>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::Modulo, ResolveBinaryOperator<OpType::OpFMod>);

    opResolvers.RegisterUnaryOpResolver(zilchType, Zilch::Grammar::Subtract, ResolveUnaryOperator<OpType::OpFNegate>);

    mathTypeResolver.RegisterFunctionResolver(GetStaticFunction(mathType, "FMod", zilchTypeName, zilchTypeName),
                                              ResolveFMod);
  }

  // Register ops that are only on float vector types (no scalars). Some of
  // these are because of zilch and not spirv.
  for (size_t i = 1; i < types.mRealVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* zilchType = types.mRealVectorTypes[i]->mZilchType;
    String zilchTypeName = zilchType->ToString();

    mathTypeResolver.RegisterFunctionResolver(GetStaticFunction(mathType, "Dot", zilchTypeName, zilchTypeName),
                                              ResolveDot);

    opResolvers.RegisterBinaryOpResolver(
        zilchType, realType, Zilch::Grammar::Multiply, ResolveBinaryOperator<spv::OpVectorTimesScalar>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, realType, Zilch::Grammar::AssignmentMultiply, ResolveBinaryOperator<OpType::OpVectorTimesScalar>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, realType, Zilch::Grammar::Divide, ResolveVectorOpSplatScalar<OpType::OpFDiv>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, realType, Zilch::Grammar::AssignmentDivide, ResolveVectorOpSplatScalar<OpType::OpFDiv>);
  }

  // Register ops that are on all integer vector types
  for (size_t i = 0; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* zilchType = types.mIntegerVectorTypes[i]->mZilchType;
    String zilchTypeName = zilchType->ToString();

    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::AssignmentAdd, ResolveBinaryOperator<spv::OpIAdd>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::AssignmentSubtract, ResolveBinaryOperator<OpType::OpISub>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::AssignmentMultiply, ResolveBinaryOperator<OpType::OpIMul>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::AssignmentDivide, ResolveBinaryOperator<OpType::OpSDiv>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::AssignmentModulo, ResolveBinaryOperator<OpType::OpSMod>);

    opResolvers.RegisterBinaryOpResolver(zilchType, zilchType, Zilch::Grammar::Add, ResolveBinaryOperator<spv::OpIAdd>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::Subtract, ResolveBinaryOperator<OpType::OpISub>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::Multiply, ResolveBinaryOperator<OpType::OpIMul>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::Divide, ResolveBinaryOperator<OpType::OpSDiv>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::Modulo, ResolveBinaryOperator<OpType::OpSMod>);

    opResolvers.RegisterUnaryOpResolver(zilchType, Zilch::Grammar::Subtract, ResolveUnaryOperator<OpType::OpSNegate>);
  }

  // Register ops that are only on int vector types (no scalars). Some of these
  // are because of zilch and not spirv.
  // @JoshD: SpirV doesn't have any actual vector operations on integers.
  // Some could be supported using more complicated instructions (e.g. vector *
  // scalar = vector * vector(scalar))
  for (size_t i = 1; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* zilchType = types.mIntegerVectorTypes[i]->mZilchType;
    String zilchTypeName = zilchType->ToString();

    // VectorTimesScalar is only on real types
    // ZilchTypePair vectorScalarTypePair(zilchType, intType);
    // translator->mBinaryOpInstructions[BinaryOpTypeId(vectorScalarTypePair,
    // Zilch::Grammar::Multiply)] = OpType::OpVectorTimesScalar;
    // translator->mBinaryOpInstructions[BinaryOpTypeId(vectorScalarTypePair,
    // Zilch::Grammar::AssignmentMultiply)] = OpType::OpVectorTimesScalar;
  }

  // Register all real matrix instructions.
  for (size_t y = 2; y <= 4; ++y)
  {
    for (size_t x = 2; x <= 4; ++x)
    {
      Zilch::BoundType* zilchType = types.GetMatrixType(y, x)->mZilchType;
      String zilchTypeName = zilchType->ToString();
      Zilch::BoundType* vectorType = types.mRealVectorTypes[x - 1]->mZilchType;

      opResolvers.RegisterBinaryOpResolver(
          zilchType, zilchType, Zilch::Grammar::Multiply, ResolveBinaryOperator<spv::OpMatrixTimesScalar>);
      mathTypeResolver.RegisterFunctionResolver(GetStaticFunction(mathType, "Transpose", zilchTypeName),
                                                ResolveMatrixTranspose);
      // Matrix times vector
      mathTypeResolver.RegisterFunctionResolver(
          GetStaticFunction(mathType, "Multiply", zilchTypeName, vectorType->ToString()), ResolveMatrixTimesVector);

      // Iterate over all of the other matrix dimensions to make the
      // multiplication functions (e.g. Real2x3 * real3x2, Real2x3 * Real3x3,
      // etc...)
      for (size_t z = 2; z <= 4; ++z)
      {
        Zilch::BoundType* rhsMatrixType = types.GetMatrixType(x, z)->mZilchType;
        mathTypeResolver.RegisterFunctionResolver(
            GetStaticFunction(mathType, "Multiply", zilchTypeName, rhsMatrixType->ToString()),
            ResolveMatrixTimesMatrix);
      }
    }
  }
}

} // namespace Zero
