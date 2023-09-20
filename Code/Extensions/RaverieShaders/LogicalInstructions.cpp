// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

template <OpType opType>
void ResolveLogicalBinaryOp(RaverieSpirVFrontEnd* translator,
                            Raverie::BinaryOperatorNode* binaryOpNode,
                            RaverieSpirVFrontEndContext* context)
{
  if (binaryOpNode->OperatorInfo.Io & Raverie::IoMode::WriteLValue)
    translator->PerformBinaryAssignmentOp(binaryOpNode, opType, context);
  else
    translator->PerformBinaryOp(binaryOpNode, opType, context);
}

template <OpType opType>
void ResolveLogicalUnaryOp(RaverieSpirVFrontEnd* translator,
                           Raverie::UnaryOperatorNode* unaryOpNode,
                           RaverieSpirVFrontEndContext* context)
{
  translator->PerformUnaryOp(unaryOpNode, opType, context);
}

// Resolves the Any/All NonZero functions which convert a vector of bools to
// a single bool. If the input is a bool type then this is a no-op.
void ResolveAnyAllNonZero(RaverieSpirVFrontEnd* translator,
                          Raverie::FunctionCallNode* functionCallNode,
                          OpType op,
                          RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* boolInputType = translator->FindType(functionCallNode->Arguments[0]->ResultType, functionCallNode);
  RaverieShaderIRType* boolResultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  RaverieShaderIROp* vector = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);

  if (boolInputType->mComponents == 1)
  {
    context->PushIRStack(vector);
    return;
  }

  IRaverieShaderIR* operation = translator->BuildCurrentBlockIROp(op, boolResultType, vector, context);
  context->PushIRStack(operation);
}

void ResolveAllNonZero(RaverieSpirVFrontEnd* translator,
                       Raverie::FunctionCallNode* functionCallNode,
                       Raverie::MemberAccessNode* memberAccessNode,
                       RaverieSpirVFrontEndContext* context)
{
  ResolveAnyAllNonZero(translator, functionCallNode, OpType::OpAll, context);
}

void ResolveAnyNonZero(RaverieSpirVFrontEnd* translator,
                       Raverie::FunctionCallNode* functionCallNode,
                       Raverie::MemberAccessNode* memberAccessNode,
                       RaverieSpirVFrontEndContext* context)
{
  ResolveAnyAllNonZero(translator, functionCallNode, OpType::OpAny, context);
}

// Resolves the logical or/and operators. This is significantly more complicated
// then it might seem because of short-circuit evaluation. These actually don't
// use the LogicalOr/And instructions but instead convert to a if/else chain.
void ResolveLogicalOrAnd(RaverieSpirVFrontEnd* translator,
                         Raverie::BinaryOperatorNode* binaryOpNode,
                         bool isOr,
                         RaverieSpirVFrontEndContext* context)
{
  // Walk the left hand operator (this can change the current block)
  IRaverieShaderIR* leftIR = translator->WalkAndGetResult(binaryOpNode->LeftOperand, context);

  // In the current block, get the value type result of the left hand side
  RaverieShaderIROp* leftOp = translator->GetOrGenerateValueTypeFromIR(leftIR, context);

  // Logical ors/ands have to generate conditionals due to short circuit
  // evaluation. To store the temporary result we have to either generate a
  // temporary variable or use an OpPhi instruction. For convenience generate a
  // temporary.
  RaverieShaderIRType* boolType = translator->FindType(RaverieTypeId(bool), binaryOpNode);
  RaverieShaderIROp* temp = translator->BuildOpVariable(boolType->mPointerType, context);
  if (isOr)
    temp->mDebugResultName = "tempOr";
  else
    temp->mDebugResultName = "tempAnd";

  // If statements always have 3 new blocks
  BasicBlock* ifTrue = translator->BuildBlockNoStack("ifTrue", context);
  BasicBlock* ifFalse = translator->BuildBlockNoStack("ifFalse", context);
  BasicBlock* mergePoint = translator->BuildBlockNoStack("mergePoint", context);
  // Mark the current block as a selection that merges at the merge block
  BasicBlock* currentBlock = context->GetCurrentBlock();
  currentBlock->mBlockType = BlockType::Selection;
  currentBlock->mMergePoint = mergePoint;

  // Branch to the true or false block depending on the value of the left op.
  // Logical Or/And are the same except for which branch they short-circuit on
  // so simply flip the true/false blocks to differentiate between them.
  if (isOr)
    translator->BuildIROp(currentBlock, OpType::OpBranchConditional, nullptr, leftOp, ifTrue, ifFalse, context);
  else
    translator->BuildIROp(currentBlock, OpType::OpBranchConditional, nullptr, leftOp, ifFalse, ifTrue, context);

  // In the true condition of a LogicalOr, we don't have to walk the left
  // hand side so simply store the result into the temp variable and branch to
  // the merge point.
  context->mCurrentBlock = ifTrue;
  context->mCurrentFunction->mBlocks.PushBack(ifTrue);
  translator->BuildStoreOp(ifTrue, temp, leftOp, context);
  translator->BuildIROp(ifTrue, OpType::OpBranch, nullptr, mergePoint, context);

  // In the false block we have to evaluate the right hand side. If there are
  // nested Ors/Ands they will each generate an if/else chain that has to be
  // evaluated but without actually evaluating a side that needs to be short
  // circuited.
  context->mCurrentBlock = ifFalse;
  context->mCurrentFunction->mBlocks.PushBack(ifFalse);
  IRaverieShaderIR* rightIR = translator->WalkAndGetResult(binaryOpNode->RightOperand, context);

  // Walking the right hand side can change the current block
  currentBlock = context->GetCurrentBlock();
  // Always store the result of the right hand side into our temp
  translator->BuildStoreOp(currentBlock, temp, rightIR, context);
  // And always branch to the merge point
  translator->BuildIROp(currentBlock, OpType::OpBranch, nullptr, mergePoint, context);

  // Now continue control flow from the merge point with the result of this
  // expression as our temporary result
  context->mCurrentBlock = mergePoint;
  context->mCurrentFunction->mBlocks.PushBack(mergePoint);
  context->PushIRStack(temp);
}

void ResolveLogicalOr(RaverieSpirVFrontEnd* translator,
                      Raverie::BinaryOperatorNode* binaryOpNode,
                      RaverieSpirVFrontEndContext* context)
{
  ResolveLogicalOrAnd(translator, binaryOpNode, true, context);
}

void ResolveLogicalAnd(RaverieSpirVFrontEnd* translator,
                       Raverie::BinaryOperatorNode* binaryOpNode,
                       RaverieSpirVFrontEndContext* context)
{
  ResolveLogicalOrAnd(translator, binaryOpNode, false, context);
}

// Register function callbacks for all logical operations (see Relational and
// Logical Instructions in the spir-v spec). Some functions aren't implemented
// here as raverie doesn't have a corresponding function. Everything else should
// be implemented on the ShaderIntrinsics type.
void RegisterLogicalOps(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, RaverieTypeGroups& types)
{
  Raverie::Core& core = Raverie::Core::GetInstance();
  Raverie::BoundType* mathType = core.MathType;
  TypeResolvers& mathTypeResolver = shaderLibrary->mTypeResolvers[mathType];
  OperatorResolvers& opResolvers = shaderLibrary->mOperatorResolvers;

  Raverie::BoundType* realType = core.RealType;
  Raverie::BoundType* intType = core.IntegerType;
  Raverie::BoundType* boolType = core.BooleanType;

  // Register ops that are on all float vector types
  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mRealVectorTypes[i];

    opResolvers.RegisterBinaryOpResolver(
        raverieType, raverieType, Raverie::Grammar::Equality, ResolveLogicalBinaryOp<OpType::OpFOrdEqual>);
    opResolvers.RegisterBinaryOpResolver(
        raverieType, raverieType, Raverie::Grammar::Inequality, ResolveLogicalBinaryOp<OpType::OpFOrdNotEqual>);
    opResolvers.RegisterBinaryOpResolver(
        raverieType, raverieType, Raverie::Grammar::GreaterThan, ResolveLogicalBinaryOp<OpType::OpFOrdGreaterThan>);
    opResolvers.RegisterBinaryOpResolver(raverieType,
                                         raverieType,
                                         Raverie::Grammar::GreaterThanOrEqualTo,
                                         ResolveLogicalBinaryOp<OpType::OpFOrdGreaterThanEqual>);
    opResolvers.RegisterBinaryOpResolver(
        raverieType, raverieType, Raverie::Grammar::LessThan, ResolveLogicalBinaryOp<OpType::OpFOrdLessThan>);
    opResolvers.RegisterBinaryOpResolver(
        raverieType, raverieType, Raverie::Grammar::LessThanOrEqualTo, ResolveLogicalBinaryOp<OpType::OpFOrdLessThanEqual>);
  }

  // Register ops that are on all integer vector types
  for (size_t i = 0; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mIntegerVectorTypes[i];

    opResolvers.RegisterBinaryOpResolver(
        raverieType, raverieType, Raverie::Grammar::Equality, ResolveLogicalBinaryOp<OpType::OpIEqual>);
    opResolvers.RegisterBinaryOpResolver(
        raverieType, raverieType, Raverie::Grammar::Inequality, ResolveLogicalBinaryOp<OpType::OpINotEqual>);
    opResolvers.RegisterBinaryOpResolver(
        raverieType, raverieType, Raverie::Grammar::GreaterThan, ResolveLogicalBinaryOp<OpType::OpSGreaterThan>);
    opResolvers.RegisterBinaryOpResolver(raverieType,
                                         raverieType,
                                         Raverie::Grammar::GreaterThanOrEqualTo,
                                         ResolveLogicalBinaryOp<OpType::OpSGreaterThanEqual>);
    opResolvers.RegisterBinaryOpResolver(
        raverieType, raverieType, Raverie::Grammar::LessThan, ResolveLogicalBinaryOp<OpType::OpSLessThan>);
    opResolvers.RegisterBinaryOpResolver(
        raverieType, raverieType, Raverie::Grammar::LessThanOrEqualTo, ResolveLogicalBinaryOp<OpType::OpSLessThanEqual>);
  }

  // Register bool scalar ops
  opResolvers.RegisterBinaryOpResolver(
      boolType, boolType, Raverie::Grammar::Equality, ResolveLogicalBinaryOp<OpType::OpLogicalEqual>);
  opResolvers.RegisterBinaryOpResolver(
      boolType, boolType, Raverie::Grammar::Inequality, ResolveLogicalBinaryOp<OpType::OpLogicalNotEqual>);
  opResolvers.RegisterBinaryOpResolver(boolType, boolType, Raverie::Grammar::LogicalOr, ResolveLogicalOr);
  opResolvers.RegisterBinaryOpResolver(boolType, boolType, Raverie::Grammar::LogicalAnd, ResolveLogicalAnd);

  // Register ops that are on all boolean vector types
  for (size_t i = 0; i < types.mBooleanVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mBooleanVectorTypes[i];
    String raverieTypeName = raverieType->ToString();

    opResolvers.RegisterUnaryOpResolver(
        raverieType, Raverie::Grammar::LogicalNot, ResolveLogicalUnaryOp<OpType::OpLogicalNot>);

    mathTypeResolver.RegisterFunctionResolver(GetStaticFunction(mathType, "AllNonZero", raverieTypeName),
                                              ResolveAllNonZero);
    mathTypeResolver.RegisterFunctionResolver(GetStaticFunction(mathType, "AnyNonZero", raverieTypeName),
                                              ResolveAnyNonZero);
  }
}

} // namespace Raverie
