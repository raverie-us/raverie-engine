// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

void UnTranslatedBoundFunction(Raverie::Call& call, Raverie::ExceptionReport& report)
{
  // This function should never be called via raverie. This is a function that
  // cannot (or hasn't yet) been given an actual implementation and is only used
  // for binding purposes.
  call.GetState()->ThrowException("Un-translatable function was called.");
  Error("Un-translatable function was called.");
}

void DummyBoundFunction(Raverie::Call& call, Raverie::ExceptionReport& report)
{
  // Set the return value to a default constructed type (zero)
  Raverie::DelegateType* functionType = call.GetFunction()->FunctionType;
  Raverie::Type* returnType = functionType->Return;
  size_t byteSize = returnType->GetAllocatedSize();
  byte* returnValue = call.GetReturnUnchecked();
  memset(returnValue, 0, byteSize);
  call.MarkReturnAsSet();
}

void ResolveSimpleFunctionFromOpType(
    RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, OpType opType, RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  RaverieShaderIROp* result = translator->BuildIROpNoBlockAdd(opType, resultType, context);
  for (size_t i = 0; i < functionCallNode->Arguments.Size(); ++i)
  {
    RaverieShaderIROp* arg = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[i], context);
    result->mArguments.PushBack(arg);
  }
  context->GetCurrentBlock()->AddOp(result);
  context->PushIRStack(result);
}

void ResolveVectorTypeCount(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  Raverie::Type* selfType = memberAccessNode->LeftOperand->ResultType;
  RaverieShaderIRType* shaderType = translator->FindType(selfType, memberAccessNode);
  RaverieShaderIROp* intConst = translator->GetIntegerConstant(shaderType->mComponents, context);
  context->PushIRStack(intConst);
}

void ResolvePrimitiveGet(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Since this isn't actually a vector, just return the left operand (ignore
  // the index)
  IRaverieShaderIR* selfInstance = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);

  context->PushIRStack(selfInstance);
}

void ResolvePrimitiveSet(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Since this isn't actually a vector, the "index" result is just the left
  // operand
  IRaverieShaderIR* selfInstance = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  // Get the source value
  IRaverieShaderIR* sourceIR = translator->WalkAndGetResult(functionCallNode->Arguments[1], context);

  // Store the source into the target
  translator->BuildStoreOp(currentBlock, selfInstance, sourceIR, context);
}

void ResolveVectorGet(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  // Get the 'this' vector type and component type
  Raverie::Type* raverieVectorType = memberAccessNode->LeftOperand->ResultType;
  RaverieShaderIRType* vectorType = translator->FindType(raverieVectorType, memberAccessNode->LeftOperand);
  RaverieShaderIRType* componentType = GetComponentType(vectorType);

  // Get the index operator from get (must be a value type)
  IRaverieShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  IRaverieShaderIR* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Generate the access chain to get the element within the vector
  IRaverieShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  RaverieShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  IRaverieShaderIR* accessChainOp = translator->BuildCurrentBlockIROp(OpType::OpAccessChain, componentType->mPointerType, selfInstance, indexOperand, context);

  context->PushIRStack(accessChainOp);
}

void ResolveVectorSet(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  // Get the 'this' vector type and component type
  Raverie::Type* raverieVectorType = memberAccessNode->LeftOperand->ResultType;
  RaverieShaderIRType* vectorType = translator->FindType(raverieVectorType, memberAccessNode->LeftOperand);
  RaverieShaderIRType* componentType = GetComponentType(vectorType);

  // Get the index operator from get (must be a value type)
  IRaverieShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  IRaverieShaderIR* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Generate the access chain to get the element within the vector
  IRaverieShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  RaverieShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  IRaverieShaderIR* accessChainOp = translator->BuildCurrentBlockIROp(OpType::OpAccessChain, componentType->mPointerType, selfInstance, indexOperand, context);

  // Get the source value
  IRaverieShaderIR* sourceIR = translator->WalkAndGetResult(functionCallNode->Arguments[1], context);

  // Store the source into the target
  BasicBlock* currentBlock = context->GetCurrentBlock();
  translator->BuildStoreOp(currentBlock, accessChainOp, sourceIR, context);
}

void ResolveMatrixGet(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  ResolveVectorGet(translator, functionCallNode, memberAccessNode, context);
}

void ResolveMatrixSet(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  ResolveVectorSet(translator, functionCallNode, memberAccessNode, context);
}

void ResolveStaticBinaryFunctionOp(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, OpType opType, RaverieSpirVFrontEndContext* context)
{
  // Get the result type
  RaverieShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  // Walk each operand
  IRaverieShaderIR* operand1 = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  IRaverieShaderIR* operand2 = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[1], context);

  // Generate the fmod op
  IRaverieShaderIR* operationOp = translator->BuildCurrentBlockIROp(opType, resultType, operand1, operand2, context);
  context->PushIRStack(operationOp);
}

void TranslatePrimitiveDefaultConstructor(RaverieSpirVFrontEnd* translator, Raverie::Type* raverieResultType, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  RaverieShaderIRType* resultType = translator->FindType(raverieResultType, nullptr);

  RaverieShaderIRType* componentType = resultType;
  Raverie::Any constantLiteral(componentType->mRaverieType);
  IRaverieShaderIR* constantZero = translator->GetConstant(componentType, constantLiteral, context);
  context->PushIRStack(constantZero);
}

void TranslatePrimitiveDefaultConstructor(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* fnCallNode, Raverie::StaticTypeNode* staticTypeNode, RaverieSpirVFrontEndContext* context)
{
  TranslatePrimitiveDefaultConstructor(translator, fnCallNode->ResultType, context);
}

void TranslateBackupPrimitiveConstructor(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* fnCallNode, Raverie::StaticTypeNode* staticTypeNode, RaverieSpirVFrontEndContext* context)
{
  if (fnCallNode->Arguments.Size() == 0)
    TranslatePrimitiveDefaultConstructor(translator, fnCallNode, staticTypeNode, context);
  else if (fnCallNode->Arguments.Size() == 1)
  {
    BasicBlock* currentBlock = context->GetCurrentBlock();
    IRaverieShaderIR* operand = translator->WalkAndGetResult(fnCallNode->Arguments[0], context);
    context->PushIRStack(operand);
  }
  else
  {
    translator->SendTranslationError(fnCallNode->Location, "Unknown primitive constructor");
    context->PushIRStack(translator->GenerateDummyIR(fnCallNode, context));
  }
}

void TranslateCompositeDefaultConstructor(RaverieSpirVFrontEnd* translator, Raverie::Type* raverieResultType, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  RaverieShaderIRType* resultType = translator->FindType(raverieResultType, nullptr);

  RaverieShaderIRType* componentType = GetComponentType(resultType);
  Raverie::Any constantLiteral(componentType->mRaverieType);
  IRaverieShaderIR* constantZero = translator->GetConstant(componentType, constantLiteral, context);

  RaverieShaderIROp* constructOp = translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, resultType, context);

  for (size_t i = 0; i < resultType->mComponents; ++i)
  {
    constructOp->mArguments.PushBack(constantZero);
  }

  currentBlock->mLines.PushBack(constructOp);
  context->PushIRStack(constructOp);
}

void TranslateCompositeDefaultConstructor(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* fnCallNode, Raverie::StaticTypeNode* staticTypeNode, RaverieSpirVFrontEndContext* context)
{
  TranslateCompositeDefaultConstructor(translator, fnCallNode->ResultType, context);
}

void TranslateBackupCompositeConstructor(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* fnCallNode, Raverie::StaticTypeNode* staticTypeNode, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  RaverieShaderIRType* resultType = translator->FindType(fnCallNode->ResultType, fnCallNode);

  // Create the op for construction but don't add it to the current block yet,
  // we need to walk all arguments first
  RaverieShaderIROp* constructOp = translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, resultType, context);

  // Walk each argument and add it to the constructor call
  for (size_t i = 0; i < fnCallNode->Arguments.Size(); ++i)
  {
    IRaverieShaderIR* argIR = translator->WalkAndGetResult(fnCallNode->Arguments[i], context);
    // CompositeConstruct requires value types
    RaverieShaderIROp* argValueOp = translator->GetOrGenerateValueTypeFromIR(argIR, context);
    constructOp->mArguments.PushBack(argValueOp);
  }

  // Now add the constructor op to the block
  currentBlock->mLines.PushBack(constructOp);
  // Also mark this as the return of this tree
  context->PushIRStack(constructOp);
}

void TranslateMatrixDefaultConstructor(RaverieSpirVFrontEnd* translator, Raverie::Type* raverieResultType, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  RaverieShaderIRType* resultType = translator->FindType(raverieResultType, nullptr);

  // Construct a default composite of the sub-type
  RaverieShaderIRType* componentType = GetComponentType(resultType);
  TranslateCompositeDefaultConstructor(translator, componentType->mRaverieType, context);

  IRaverieShaderIR* constituent = context->PopIRStack();
  // @JoshD: Leave this out for now since it can produce a glsl translation
  // error
  // constituent->mDebugResultName = "constituent";

  // Construct the composite but delay adding it to the stack until we've added
  // all of the arguments to it
  RaverieShaderIROp* constructOp = translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, resultType, context);
  for (size_t i = 0; i < resultType->mComponents; ++i)
  {
    constructOp->mArguments.PushBack(constituent);
  }

  // Now add the matrix constructor the the stack
  currentBlock->mLines.PushBack(constructOp);
  context->PushIRStack(constructOp);
}

void TranslateMatrixDefaultConstructor(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* fnCallNode, Raverie::StaticTypeNode* staticTypeNode, RaverieSpirVFrontEndContext* context)
{
  TranslateMatrixDefaultConstructor(translator, fnCallNode->ResultType, context);
}

void TranslateMatrixFullConstructor(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* fnCallNode, Raverie::StaticTypeNode* staticTypeNode, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  RaverieShaderIRType* matrixType = translator->FindType(fnCallNode->ResultType, nullptr);
  RaverieShaderIRType* componentType = GetComponentType(matrixType);

  // Construct the matrix type but delay adding it as an instruction until all
  // of the arguments have been created
  RaverieShaderIROp* matrixConstructOp = translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, matrixType, context);
  for (size_t i = 0; i < matrixType->mComponents; ++i)
  {
    // Create each vector type but delay add it for the same reason as the
    // matrix
    RaverieShaderIROp* componentConstructOp = translator->BuildIROpNoBlockAdd(spv::OpCompositeConstruct, componentType, context);
    for (size_t j = 0; j < componentType->mComponents; ++j)
    {
      // Walk the given parameter and add it the the vector
      int argIndex = i * componentType->mComponents + j;
      IRaverieShaderIR* param = translator->WalkAndGetValueTypeResult(fnCallNode->Arguments[argIndex], context);
      componentConstructOp->mArguments.PushBack(param);
    }
    // Now that we've finished constructing the parameters for the vector,
    // actually add it to the the current block and add it as a parameter to the
    // matrix
    currentBlock->mLines.PushBack(componentConstructOp);
    matrixConstructOp->mArguments.PushBack(componentConstructOp);
  }

  // Now the matrix is fully constructed so add it to the block
  currentBlock->mLines.PushBack(matrixConstructOp);
  context->PushIRStack(matrixConstructOp);
}

RaverieShaderIROp* RecursivelyTranslateCompositeSplatConstructor(RaverieSpirVFrontEnd* translator,
                                                                 Raverie::FunctionCallNode* fnCallNode,
                                                                 Raverie::StaticTypeNode* staticTypeNode,
                                                                 RaverieShaderIRType* type,
                                                                 RaverieShaderIROp* splatValueOp,
                                                                 RaverieSpirVFrontEndContext* context)
{
  // Terminate on the base case. Potentially need to handle translating the
  // splat value op to the correct scalar type.
  if (type->mBaseType == ShaderIRTypeBaseType::Float || type->mBaseType == ShaderIRTypeBaseType::Int || type->mBaseType == ShaderIRTypeBaseType::Bool)
    return splatValueOp;

  BasicBlock* currentBlock = context->GetCurrentBlock();
  RaverieShaderIRType* componentType = GetComponentType(type);

  // Construct the composite but delay adding it as an instruction until all of
  // the sub-composites are created
  RaverieShaderIROp* constructOp = translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, type, context);
  for (size_t i = 0; i < type->mComponents; ++i)
  {
    // Construct each constituent
    RaverieShaderIROp* constituentOp = RecursivelyTranslateCompositeSplatConstructor(translator, fnCallNode, staticTypeNode, componentType, splatValueOp, context);
    // @JoshD: Leave this out for now since this produces a glsl translation
    // error
    // constituentOp->mDebugResultName = "constituent";
    constructOp->mArguments.PushBack(constituentOp);
  }
  // Now we can add the instruction to construct this composite since we've
  // created all of the parameters
  currentBlock->mLines.PushBack(constructOp);
  return constructOp;
}

void TranslateCompositeSplatConstructor(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* fnCallNode, Raverie::StaticTypeNode* staticTypeNode, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  RaverieShaderIRType* resultType = translator->FindType(fnCallNode->ResultType, fnCallNode);

  // Get the splat scalar value type
  IRaverieShaderIR* splatValue = translator->WalkAndGetResult(fnCallNode->Arguments[0], context);
  RaverieShaderIROp* splatValueOp = translator->GetOrGenerateValueTypeFromIR(splatValue, context);

  // Recursively construct all composite types with this final scalar type
  RaverieShaderIROp* constructOp = RecursivelyTranslateCompositeSplatConstructor(translator, fnCallNode, staticTypeNode, resultType, splatValueOp, context);

  context->PushIRStack(constructOp);
}

bool IsVectorSwizzle(StringParam memberName)
{
  // Verify that all components are in the valid range
  for (size_t i = 0; i < memberName.SizeInBytes(); ++i)
  {
    byte memberValue = *(memberName.Data() + i);

    if (memberValue < 'W' || memberValue > 'Z')
      return false;
  }
  return true;
}

void ResolveScalarComponentAccess(RaverieSpirVFrontEnd* translator, Raverie::MemberAccessNode* memberAccessNode, byte componentName, RaverieSpirVFrontEndContext* context)
{
  // A scalar component access on a scalar type is just the scalar itself (e.g.
  // a.X => a)
  IRaverieShaderIR* operandResult = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  context->PushIRStack(operandResult);
}

void ResolveScalarSwizzle(RaverieSpirVFrontEnd* translator, Raverie::MemberAccessNode* memberAccessNode, StringParam memberName, RaverieSpirVFrontEndContext* context)
{
  // Figure out what the result type of this swizzle is
  RaverieShaderIRType* resultType = translator->FindType(memberAccessNode->ResultType, memberAccessNode);

  // The swizzle instruction (vector shuffle) requires value types
  IRaverieShaderIR* operandResult = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  RaverieShaderIROp* operandValueOp = translator->GetOrGenerateValueTypeFromIR(operandResult, context);

  // Vector swizzle doesn't exist on scalar types so just splat construct the
  // relevant vector type
  RaverieShaderIROp* constructOp = translator->BuildCurrentBlockIROp(OpType::OpCompositeConstruct, resultType, context);
  for (size_t i = 0; i < memberName.SizeInBytes(); ++i)
    constructOp->mArguments.PushBack(operandValueOp);
  context->PushIRStack(constructOp);
}

void ScalarBackupFieldResolver(RaverieSpirVFrontEnd* translator, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  String memberName = memberAccessNode->Name;

  // Deal with single component access
  if (memberName.SizeInBytes() == 1)
  {
    byte memberValue = *memberName.Data();
    ResolveScalarComponentAccess(translator, memberAccessNode, memberValue, context);
    return;
  }

  // Deal with swizzles
  if (memberName.SizeInBytes() <= 4 && IsVectorSwizzle(memberName))
  {
    ResolveScalarSwizzle(translator, memberAccessNode, memberName, context);
    return;
  }
}

void ResolveVectorCopyConstructor(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* fnCallNode, Raverie::StaticTypeNode* staticTypeNode, RaverieSpirVFrontEndContext* context)
{
  if (fnCallNode->Arguments.Size() != 1)
  {
    Error("Copy constructor translation can only handle a single element");
    return;
  }

  // Get the value type op
  IRaverieShaderIR* argIR = translator->WalkAndGetResult(fnCallNode->Arguments[0], context);
  RaverieShaderIROp* argValueOp = translator->GetOrGenerateValueTypeFromIR(argIR, context);
  // If the original op was a variable (pointer type) then we can simply load the op to get a value to store or use.
  // If the original op was a value type then we have to generate a copy somehow since we can't just assign ops.
  // The easiest way to do this with a vector is to swizzle it to generate the same vector.
  if (argIR == argValueOp)
  {
    RaverieShaderIRType* resultType = translator->FindType(fnCallNode->ResultType, fnCallNode);
    argValueOp = translator->BuildIROp(context->GetCurrentBlock(), OpType::OpVectorShuffle, resultType, argValueOp, argValueOp, context);
    for (u32 i = 0; i < resultType->mComponents; ++i)
      argValueOp->mArguments.PushBack(translator->GetOrCreateConstantIntegerLiteral(i));
  }

  // Also mark this as the return of this tree
  context->PushIRStack(argValueOp);
}

void ResolveVectorComponentAccess(RaverieSpirVFrontEnd* translator, RaverieShaderIROp* selfInstance, RaverieShaderIRType* componentType, byte componentName, RaverieSpirVFrontEndContext* context)
{
  // Convert the index to [0, 3] using a nice modulus trick
  int index = componentName - 'X';
  index = (index + 4) % 4;

  // If the operand was a pointer type then we have to use an access chain to
  // extract the sub-pointer
  if (selfInstance->IsResultPointerType())
  {
    RaverieShaderIROp* indexOp = translator->GetIntegerConstant(index, context);
    RaverieShaderIROp* accessChainOp = translator->BuildCurrentBlockIROp(OpType::OpAccessChain, componentType->mPointerType, selfInstance, indexOp, context);
    context->PushIRStack(accessChainOp);
  }
  // Otherwise this was a value (temporary, e.g. (Real3() + Real3()).X) so
  // use composite extract instead which works on value types.
  else
  {
    RaverieShaderIRConstantLiteral* indexLiteral = translator->GetOrCreateConstantLiteral(index);
    RaverieShaderIROp* accessChainOp = translator->BuildCurrentBlockIROp(OpType::OpCompositeExtract, componentType, selfInstance, indexLiteral, context);
    context->PushIRStack(accessChainOp);
  }
}

void ResolveVectorComponentAccess(RaverieSpirVFrontEnd* translator, Raverie::MemberAccessNode* memberAccessNode, byte componentName, RaverieSpirVFrontEndContext* context)
{
  // Walk the left hand side of the member access node
  IRaverieShaderIR* operandResult = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  RaverieShaderIROp* operandResultOp = operandResult->As<RaverieShaderIROp>();

  // Get what the result type should be (e.g. Real3.X -> Real)
  RaverieShaderIRType* operandType = translator->FindType(memberAccessNode->LeftOperand->ResultType, memberAccessNode->LeftOperand);
  RaverieShaderIRType* componentType = GetComponentType(operandType);

  ResolveVectorComponentAccess(translator, operandResultOp, componentType, componentName, context);
}

void ResolveVectorSwizzle(RaverieSpirVFrontEnd* translator, IRaverieShaderIR* selfInstance, RaverieShaderIRType* resultType, StringParam memberName, RaverieSpirVFrontEndContext* context)
{
  // The swizzle instruction (vector shuffle) requires value types
  RaverieShaderIROp* operandValueOp = translator->GetOrGenerateValueTypeFromIR(selfInstance, context);

  // Build the base instruction
  RaverieShaderIROp* swizzleOp = translator->BuildCurrentBlockIROp(OpType::OpVectorShuffle, resultType, operandValueOp, operandValueOp, context);

  // For every swizzle element, figure out what index the sub-item is and add
  // that as an argument
  for (size_t i = 0; i < memberName.SizeInBytes(); ++i)
  {
    byte memberValue = *(memberName.Data() + i);
    int index = (memberValue - 'X' + 4) % 4;
    RaverieShaderIRConstantLiteral* indexLiteral = translator->GetOrCreateConstantLiteral(index);
    swizzleOp->mArguments.PushBack(indexLiteral);
  }
  context->PushIRStack(swizzleOp);
}

void ResolveVectorSwizzle(RaverieSpirVFrontEnd* translator, Raverie::MemberAccessNode* memberAccessNode, StringParam memberName, RaverieSpirVFrontEndContext* context)
{
  // Figure out what the result type of this swizzle is
  RaverieShaderIRType* resultType = translator->FindType(memberAccessNode->ResultType, memberAccessNode);

  // Get the operand to swizzle
  IRaverieShaderIR* operandResult = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  ResolveVectorSwizzle(translator, operandResult, resultType, memberName, context);
}

void VectorBackupFieldResolver(RaverieSpirVFrontEnd* translator, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  String memberName = memberAccessNode->Name;

  // Deal with single component access
  if (memberName.SizeInBytes() == 1)
  {
    byte memberValue = *memberName.Data();
    ResolveVectorComponentAccess(translator, memberAccessNode, memberValue, context);
    return;
  }

  // Deal with swizzles
  if (memberName.SizeInBytes() <= 4 && IsVectorSwizzle(memberName))
  {
    ResolveVectorSwizzle(translator, memberAccessNode, memberName, context);
    return;
  }

  // Otherwise we don't know how to translate this so report an error
  translator->SendTranslationError(memberAccessNode->Location, "Cannot resolve vector field");
  context->PushIRStack(translator->GenerateDummyIR(memberAccessNode, context));
}

void ResolverVectorSwizzleSetter(
    RaverieSpirVFrontEnd* translator, IRaverieShaderIR* selfInstance, RaverieShaderIROp* resultValue, RaverieShaderIRType* resultType, StringParam memberName, RaverieSpirVFrontEndContext* context)
{
  // To generate the new vector we need to perform vector shuffle which requires
  // a value type.
  RaverieShaderIROp* instanceValue = translator->GetOrGenerateValueTypeFromIR(selfInstance, context);

  // The easiest way to set via a swizzle is by constructing a brand new vector
  // from elements of the old and new vector using the shuffle instruction
  // (parameters set below)
  RaverieShaderIROp* shuffleOp = translator->BuildCurrentBlockIROp(OpType::OpVectorShuffle, resultType, instanceValue, resultValue, context);

  // The shuffle operator picks components from the two vectors by index as if
  // they were laid out in one contiguous block of memory. By default assume
  // that all components will come from the same location in the original
  // vector. To copy elements from the new vector we loop over the member access
  // and set the relevant set index (by name) to the memory index of the new
  // vector.

  // Keep track of how many components this vector has
  int instanceComponentCount = instanceValue->mResultType->mComponents;
  // Hard-coded max of 4 (vectors cannot be bigger)
  int indices[4] = {0, 1, 2, 3};

  for (size_t i = 0; i < memberName.SizeInBytes(); ++i)
  {
    byte memberValue = *(memberName.Data() + i);
    int index = (memberValue - 'X' + 4) % 4;
    indices[index] = i + instanceComponentCount;
  }

  // Actually create all of the arguments (have to create literals)
  for (size_t i = 0; i < (size_t)instanceComponentCount; ++i)
  {
    RaverieShaderIRConstantLiteral* literal = translator->GetOrCreateConstantIntegerLiteral(indices[i]);
    shuffleOp->mArguments.PushBack(literal);
  }

  // Store the result back into the instance
  translator->BuildStoreOp(selfInstance, shuffleOp, context);
}

void ResolverVectorSwizzleSetter(RaverieSpirVFrontEnd* translator, Raverie::MemberAccessNode* memberAccessNode, RaverieShaderIROp* resultValue, RaverieSpirVFrontEndContext* context)
{
  String memberName = memberAccessNode->Name;
  RaverieShaderIRType* resultType = translator->FindType(memberAccessNode->LeftOperand->ResultType, memberAccessNode->LeftOperand);

  // Get the instance we will be setting to
  IRaverieShaderIR* instance = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  ResolverVectorSwizzleSetter(translator, instance, resultValue, resultType, memberName, context);
}

void VectorBackupPropertySetter(RaverieSpirVFrontEnd* translator, Raverie::MemberAccessNode* memberAccessNode, RaverieShaderIROp* resultValue, RaverieSpirVFrontEndContext* context)
{
  String memberName = memberAccessNode->Name;

  // Deal with scalar setters (fallback to the component access)
  if (memberName.SizeInBytes() == 1)
  {
    ResolveVectorComponentAccess(translator, memberAccessNode, *memberName.Data(), context);
    translator->BuildStoreOp(context->GetCurrentBlock(), context->PopIRStack(), resultValue, context);
    return;
  }

  // Resolve swizzles
  if (IsVectorSwizzle(memberName))
  {
    ResolverVectorSwizzleSetter(translator, memberAccessNode, resultValue, context);
    return;
  }

  // Otherwise we don't know how to translate this so report an error
  translator->SendTranslationError(memberAccessNode->Location, "Cannot set non-vector swizzle");
  context->PushIRStack(translator->GenerateDummyIR(memberAccessNode, context));
}

bool MatrixElementAccessResolver(RaverieSpirVFrontEnd* translator, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context, Raverie::MatrixUserData& matrixUserData)
{
  // We only translate Myx here.
  String data = memberAccessNode->Name;
  if (data.SizeInBytes() != 3)
    return false;

  StringRange range = data.All();
  Rune first = range.Front();
  if (first != 'M')
    return false;

  range.PopFront();
  Rune indexYRune = range.Front();
  range.PopFront();
  Rune indexXRune = range.Front();

  // Get the indices of the access and validate them
  size_t indexY = indexYRune.value - '0';
  size_t indexX = indexXRune.value - '0';
  if (indexX >= matrixUserData.SizeX || indexY >= matrixUserData.SizeY)
    return false;

  // Figure out what the result type of the member access is
  RaverieShaderIRType* resultType = translator->FindType(memberAccessNode->ResultType, memberAccessNode);
  // Construct constants for the sub-access
  RaverieShaderIROp* indexYConstant = translator->GetIntegerConstant((int)indexY, context);
  RaverieShaderIROp* indexXConstant = translator->GetIntegerConstant((int)indexX, context);
  // Get the matrix instance
  IRaverieShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  RaverieShaderIROp* operandResult = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  // Construct the op to access the matrix element
  // @JoshD: Revisit when looking over matrix column/row order
  RaverieShaderIROp* accessOp = translator->BuildCurrentBlockIROp(OpType::OpAccessChain, resultType->mPointerType, operandResult, indexYConstant, indexXConstant, context);
  context->PushIRStack(accessOp);
  return true;
}

void MatrixBackupFieldResolver(RaverieSpirVFrontEnd* translator, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  Raverie::Type* matrixType = memberAccessNode->LeftOperand->ResultType;
  Raverie::MatrixUserData& userData = matrixType->ComplexUserData.ReadObject<Raverie::MatrixUserData>(0);
  // Handle accessing an element (e.g. M12) from the matrix
  if (MatrixElementAccessResolver(translator, memberAccessNode, context, userData))
    return;

  // @JoshD: Handle the rest of matrix translation later
  translator->SendTranslationError(memberAccessNode->Location, "Member access cannot be translated");
  context->PushIRStack(translator->GenerateDummyIR(memberAccessNode, context));
}

void TranslateQuaternionDefaultConstructor(RaverieSpirVFrontEnd* translator, Raverie::Type* raverieResultType, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  // Get all of the types related to quaternions
  RaverieShaderIRType* quaternionType = translator->FindType(raverieResultType, nullptr);
  RaverieShaderIRType* vec4Type = quaternionType->GetSubType(0);
  RaverieShaderIRType* realType = GetComponentType(vec4Type);

  // Construct the default vec4 for the quaternion (identity)
  IRaverieShaderIR* constantZero = translator->GetConstant(realType, 0.0f, context);
  IRaverieShaderIR* constantOne = translator->GetConstant(realType, 1.0f, context);
  RaverieShaderIROp* vec4ConstructOp = translator->BuildCurrentBlockIROp(OpType::OpCompositeConstruct, vec4Type, context);
  vec4ConstructOp->mArguments.PushBack(constantZero);
  vec4ConstructOp->mArguments.PushBack(constantZero);
  vec4ConstructOp->mArguments.PushBack(constantZero);
  vec4ConstructOp->mArguments.PushBack(constantOne);

  // Construct the quaternion from the vec4
  RaverieShaderIROp* constructOp = translator->BuildIROp(context->GetCurrentBlock(), OpType::OpCompositeConstruct, quaternionType, vec4ConstructOp, context);
  context->PushIRStack(constructOp);
}

void TranslateQuaternionDefaultConstructor(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* fnCallNode, Raverie::StaticTypeNode* staticTypeNode, RaverieSpirVFrontEndContext* context)
{
  TranslateQuaternionDefaultConstructor(translator, fnCallNode->ResultType, context);
}

void QuaternionBackupFieldResolver(RaverieSpirVFrontEnd* translator, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  String memberName = memberAccessNode->Name;

  IRaverieShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  RaverieShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  RaverieShaderIRType* vec4Type = selfInstance->mResultType->mDereferenceType->GetSubType(0);
  RaverieShaderIRType* realType = GetComponentType(vec4Type);

  // Get the vec4 of the quaternion
  IRaverieShaderIR* vec4OffsetConstant = translator->GetIntegerConstant(0, context);
  RaverieShaderIROp* vec4Instance = translator->BuildCurrentBlockIROp(OpType::OpAccessChain, vec4Type->mPointerType, selfInstance, vec4OffsetConstant, context);

  // Deal with single component access
  if (memberName.SizeInBytes() == 1)
  {
    byte memberValue = *memberName.Data();
    ResolveVectorComponentAccess(translator, vec4Instance, realType, memberValue, context);
    return;
  }

  // Deal with swizzles
  if (memberName.SizeInBytes() <= 4 && IsVectorSwizzle(memberName))
  {
    RaverieShaderIRType* resultType = translator->FindType(memberAccessNode->ResultType, memberAccessNode);
    ResolveVectorSwizzle(translator, vec4Instance, resultType, memberName, context);
    return;
  }

  // Deal with the remainder later
  translator->SendTranslationError(memberAccessNode->Location, "Cannot resolve quaternion field");
  context->PushIRStack(translator->GenerateDummyIR(memberAccessNode, context));
}

void QuaternionBackupPropertySetter(RaverieSpirVFrontEnd* translator, Raverie::MemberAccessNode* memberAccessNode, RaverieShaderIROp* resultValue, RaverieSpirVFrontEndContext* context)
{
  String memberName = memberAccessNode->Name;

  IRaverieShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  RaverieShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  RaverieShaderIRType* vec4Type = selfInstance->mResultType->mDereferenceType->GetSubType(0);
  RaverieShaderIRType* realType = GetComponentType(vec4Type);

  // Get the vec4 of the quaternion
  IRaverieShaderIR* vec4OffsetConstant = translator->GetIntegerConstant(0, context);
  RaverieShaderIROp* vec4Instance = translator->BuildCurrentBlockIROp(OpType::OpAccessChain, vec4Type->mPointerType, selfInstance, vec4OffsetConstant, context);

  // Deal with single component access
  if (memberName.SizeInBytes() == 1)
  {
    byte memberValue = *memberName.Data();
    ResolveVectorComponentAccess(translator, vec4Instance, realType, memberValue, context);
    translator->BuildStoreOp(context->GetCurrentBlock(), context->PopIRStack(), resultValue, context);
    return;
  }

  // Resolve swizzles
  if (IsVectorSwizzle(memberName))
  {
    RaverieShaderIRType* resultType = translator->FindType(memberAccessNode->ResultType, memberAccessNode);
    ResolverVectorSwizzleSetter(translator, vec4Instance, resultValue, vec4Type, memberAccessNode->Name, context);
    return;
  }

  // Otherwise we don't know how to translate this so report an error
  translator->SendTranslationError(memberAccessNode->Location, "Cannot set non-vector swizzle");
  context->PushIRStack(translator->GenerateDummyIR(memberAccessNode, context));
}

void ResolveQuaternionTypeCount(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  // Quaternion count is always 4
  RaverieShaderIROp* intConst = translator->GetIntegerConstant(4, context);
  context->PushIRStack(intConst);
}

void ResolveQuaternionGet(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  IRaverieShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  RaverieShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  RaverieShaderIRType* vec4Type = selfInstance->mResultType->mDereferenceType->GetSubType(0);
  RaverieShaderIRType* realType = GetComponentType(vec4Type);

  // Get the vec4 of the quaternion
  IRaverieShaderIR* vec4OffsetConstant = translator->GetIntegerConstant(0, context);
  RaverieShaderIROp* vec4Instance = translator->BuildCurrentBlockIROp(OpType::OpAccessChain, vec4Type->mPointerType, selfInstance, vec4OffsetConstant, context);

  // Get the index operator from get (must be a value type)
  IRaverieShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  IRaverieShaderIR* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Generate the access chain to get the element within the vector
  IRaverieShaderIR* accessChainOp = translator->BuildCurrentBlockIROp(OpType::OpAccessChain, realType->mPointerType, vec4Instance, indexOperand, context);

  context->PushIRStack(accessChainOp);
}

void ResolveQuaternionSet(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  IRaverieShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  RaverieShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  RaverieShaderIRType* vec4Type = selfInstance->mResultType->mDereferenceType->GetSubType(0);
  RaverieShaderIRType* realType = GetComponentType(vec4Type);

  // Get the vec4 of the quaternion
  IRaverieShaderIR* offsetConstant = translator->GetIntegerConstant(0, context);
  RaverieShaderIROp* vec4Instance = translator->BuildCurrentBlockIROp(OpType::OpAccessChain, vec4Type->mPointerType, selfInstance, offsetConstant, context);

  // Get the index operator from get (must be a value type)
  IRaverieShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  IRaverieShaderIR* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Generate the access chain to get the element within the vector
  IRaverieShaderIR* accessChainOp = translator->BuildCurrentBlockIROp(OpType::OpAccessChain, realType->mPointerType, vec4Instance, indexOperand, context);

  // Get the source value
  IRaverieShaderIR* sourceIR = translator->WalkAndGetResult(functionCallNode->Arguments[1], context);

  // Store the source into the target
  BasicBlock* currentBlock = context->GetCurrentBlock();
  translator->BuildStoreOp(currentBlock, accessChainOp, sourceIR, context);
}

void TranslateQuaternionSplatConstructor(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* fnCallNode, Raverie::StaticTypeNode* staticTypeNode, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  RaverieShaderIRType* quaternionType = translator->FindType(fnCallNode->ResultType, fnCallNode);
  RaverieShaderIRType* vec4Type = quaternionType->GetSubType(0);

  // Get the splat scalar value type
  IRaverieShaderIR* splatValue = translator->WalkAndGetResult(fnCallNode->Arguments[0], context);
  RaverieShaderIROp* splatValueOp = translator->GetOrGenerateValueTypeFromIR(splatValue, context);

  // Construct the vec4 type from the splat value
  RaverieShaderIROp* subConstructOp = RecursivelyTranslateCompositeSplatConstructor(translator, fnCallNode, staticTypeNode, vec4Type, splatValueOp, context);

  // Construct the quaternion from the vec4
  RaverieShaderIROp* vec4Value = translator->GetOrGenerateValueTypeFromIR(subConstructOp, context);
  RaverieShaderIROp* quaternionConstructOp = translator->BuildCurrentBlockIROp(OpType::OpCompositeConstruct, quaternionType, vec4Value, context);

  context->PushIRStack(quaternionConstructOp);
}

void TranslateBackupQuaternionConstructor(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* fnCallNode, Raverie::StaticTypeNode* staticTypeNode, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  RaverieShaderIRType* quaternionType = translator->FindType(fnCallNode->ResultType, fnCallNode);
  RaverieShaderIRType* vec4Type = quaternionType->GetSubType(0);

  // Create the op for construction but don't add it to the current block yet,
  // we need to walk all arguments first
  RaverieShaderIROp* vec4ConstructOp = translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, vec4Type, context);

  // Walk each argument and add it to the constructor call
  for (size_t i = 0; i < fnCallNode->Arguments.Size(); ++i)
  {
    IRaverieShaderIR* argIR = translator->WalkAndGetResult(fnCallNode->Arguments[i], context);
    // CompositeConstruct requires value types
    RaverieShaderIROp* argValueOp = translator->GetOrGenerateValueTypeFromIR(argIR, context);
    vec4ConstructOp->mArguments.PushBack(argValueOp);
  }

  // Now add the constructor op to the block
  currentBlock->mLines.PushBack(vec4ConstructOp);

  RaverieShaderIROp* quaternionConstructOp = translator->BuildCurrentBlockIROp(OpType::OpCompositeConstruct, quaternionType, vec4ConstructOp, context);
  // Also mark this as the return of this tree
  context->PushIRStack(quaternionConstructOp);
}

void ResolveColor(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* /*functionCallNode*/, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  Raverie::Property* property = memberAccessNode->AccessedProperty;
  RaverieShaderIRType* resultType = translator->FindType(property->PropertyType, memberAccessNode);
  RaverieShaderIRType* componentType = GetComponentType(resultType);

  // Read the color value from the complex user data
  Raverie::Real4 propertyValue = property->ComplexUserData.ReadObject<Raverie::Real4>(0);

  // Composite construct the color
  RaverieShaderIROp* constructOp = translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, resultType, context);
  for (size_t i = 0; i < 4; ++i)
  {
    Raverie::Any constantLiteral(propertyValue[i]);
    IRaverieShaderIR* component = translator->GetConstant(componentType, constantLiteral, context);
    constructOp->mArguments.PushBack(component);
  }
  context->mCurrentBlock->AddOp(constructOp);
  context->PushIRStack(constructOp);
}

void RegisterColorsOps(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, RaverieTypeGroups& types)
{
  Raverie::Core& core = Raverie::Core::GetInstance();
  Raverie::Library* coreLibrary = core.GetLibrary();
  // Create the colors type
  Raverie::BoundType* colors = RaverieTypeId(Raverie::ColorsClass);
  RaverieShaderIRType* colorsShaderType = translator->MakeStructType(shaderLibrary, colors->Name, colors, spv::StorageClass::StorageClassGeneric);
  TypeResolvers& typeResolver = shaderLibrary->mTypeResolvers[colors];

  // the only way to get the value for each color during translation is to
  // compile and execute raverie.
  Raverie::Type* real4Type = RaverieTypeId(Raverie::Real4);
  forRange (Raverie::Property* raverieProperty, colors->GetProperties())
  {
    // Skip non-static real4 properties (should only be the actual colors)
    if (!raverieProperty->IsStatic || raverieProperty->PropertyType != real4Type)
      continue;

    typeResolver.RegisterFunctionResolver(raverieProperty->Get, ResolveColor);
  }
}

} // namespace Raverie
