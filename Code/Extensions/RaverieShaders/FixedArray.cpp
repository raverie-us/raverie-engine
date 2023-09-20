// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

void FixedArrayDefaultConstructor(RaverieSpirVFrontEnd* translator,
                                  Raverie::Type* resultType,
                                  RaverieSpirVFrontEndContext* context)
{
  // Just build a variable op for the given fixed array type
  RaverieShaderIRType* shaderType = translator->FindType(resultType, nullptr);
  context->PushIRStack(translator->BuildOpVariable(shaderType->mPointerType, context));
}

void FixedArrayBackupConstructor(RaverieSpirVFrontEnd* translator,
                                 Raverie::FunctionCallNode* fnCallNode,
                                 Raverie::StaticTypeNode* staticTypeNode,
                                 RaverieSpirVFrontEndContext* context)
{
  FixedArrayDefaultConstructor(translator, staticTypeNode->ResultType, context);
}

bool ValidateIndexLiteral(RaverieSpirVFrontEnd* translator,
                          Raverie::ExpressionNode* node,
                          RaverieShaderIROp* indexOperand,
                          int maxValue,
                          RaverieSpirVFrontEndContext* context)
{
  // Check if this is a constant (literal). If not we can't validate.
  if (indexOperand->mOpType != OpType::OpConstant)
    return true;

  // Get the literal value
  RaverieShaderIRConstantLiteral* literal = indexOperand->mArguments[0]->As<RaverieShaderIRConstantLiteral>();
  int indexLiteral = literal->mValue.Get<int>();
  // If it's in the valid range then this index is valid;
  if (0 <= indexLiteral && indexLiteral < maxValue)
    return true;

  // Otherwise the value is invalid so report an error and emit a dummy op-code
  String message = String::Format("Invalid index. Index can only be in the range of [0, %d)", maxValue);
  translator->SendTranslationError(node->Location, message);
  context->PushIRStack(translator->GenerateDummyIR(node, context));
  return false;
}

void ResolveFixedArrayGet(RaverieSpirVFrontEnd* translator,
                          Raverie::FunctionCallNode* functionCallNode,
                          Raverie::MemberAccessNode* memberAccessNode,
                          RaverieSpirVFrontEndContext* context)
{
  // Get the 'this' array type and component type
  Raverie::Type* raverieArrayType = memberAccessNode->LeftOperand->ResultType;
  RaverieShaderIRType* arrayType = translator->FindType(raverieArrayType, memberAccessNode->LeftOperand);
  RaverieShaderIRType* elementType = arrayType->mParameters[0]->As<RaverieShaderIRType>();

  // Get the index operator (must be a value type)
  IRaverieShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  RaverieShaderIROp* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Validate the index if it's a literal
  if (!ValidateIndexLiteral(translator, functionCallNode, indexOperand, arrayType->mComponents, context))
    return;

  // Generate the access chain to get the element within the array
  IRaverieShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  RaverieShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  IRaverieShaderIR* accessChainOp =
      translator->BuildCurrentBlockAccessChain(elementType, selfInstance, indexOperand, context);

  context->PushIRStack(accessChainOp);
}

void ResolveFixedArraySet(RaverieSpirVFrontEnd* translator,
                          Raverie::FunctionCallNode* functionCallNode,
                          Raverie::MemberAccessNode* memberAccessNode,
                          RaverieSpirVFrontEndContext* context)
{
  // Get the 'this' array type and component type
  Raverie::Type* raverieArrayType = memberAccessNode->LeftOperand->ResultType;
  RaverieShaderIRType* arrayType = translator->FindType(raverieArrayType, memberAccessNode->LeftOperand);
  RaverieShaderIRType* elementType = arrayType->mParameters[0]->As<RaverieShaderIRType>();

  // Get the index operator (must be a value type)
  IRaverieShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  RaverieShaderIROp* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Validate the index if it's a literal
  if (!ValidateIndexLiteral(translator, functionCallNode, indexOperand, arrayType->mComponents, context))
    return;

  // Generate the access chain to get the element within the array
  IRaverieShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  RaverieShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  IRaverieShaderIR* accessChainOp =
      translator->BuildCurrentBlockAccessChain(elementType, selfInstance, indexOperand, context);

  // Get the source value
  IRaverieShaderIR* sourceIR = translator->WalkAndGetResult(functionCallNode->Arguments[1], context);

  // Store the source into the target
  BasicBlock* currentBlock = context->GetCurrentBlock();
  translator->BuildStoreOp(currentBlock, accessChainOp, sourceIR, context);
}

void ResolveFixedArrayCount(RaverieSpirVFrontEnd* translator,
                            Raverie::FunctionCallNode* functionCallNode,
                            Raverie::MemberAccessNode* memberAccessNode,
                            RaverieSpirVFrontEndContext* context)
{
  // Return the integer constant expression as the array count (could call the
  // intrinsic for count but why not use this)
  Raverie::Type* raverieArrayType = memberAccessNode->LeftOperand->ResultType;
  RaverieShaderIRType* arrayType = translator->FindType(raverieArrayType, memberAccessNode->LeftOperand);
  context->PushIRStack(arrayType->mParameters[1]);
}

void FixedArrayExpressionInitializerResolver(RaverieSpirVFrontEnd* translator,
                                             Raverie::ExpressionInitializerNode*& node,
                                             RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* fixedArrayType = translator->FindType(node->ResultType, node);

  // Validate that the initializer list has the exact number of expected
  // arguments
  size_t statementCount = node->InitializerStatements.Size();
  if (fixedArrayType->mComponents != statementCount)
  {
    String errMsg = String::Format(
        "Array initializer was given %d item(s) and it expected %d.", statementCount, fixedArrayType->mComponents);
    translator->SendTranslationError(node->Location, errMsg);
    context->PushIRStack(translator->GenerateDummyIR(node, context));
    return;
  }

  // Start constructing the array by walking all of the arguments
  RaverieShaderIROp* compositeConstructOp =
      translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, fixedArrayType, context);
  for (size_t i = 0; i < node->InitializerStatements.Size(); ++i)
  {
    Raverie::ExpressionNode* expNode = node->InitializerStatements[i];

    String errorMessage;
    Raverie::ExpressionNode* argumentValueNode = nullptr;

    // Verify that this statement is a call to OperatorInsert (.Add). If it is
    // then walk the argument to get the initial value we set (required to be a
    // value type).
    Raverie::FunctionCallNode* addCallNode = Raverie::Type::DynamicCast<Raverie::FunctionCallNode*>(expNode);
    if (addCallNode != nullptr)
    {
      Raverie::MemberAccessNode* memberAccessNode =
          Raverie::Type::DynamicCast<Raverie::MemberAccessNode*>(addCallNode->LeftOperand);
      if (memberAccessNode != nullptr && memberAccessNode->Name == Raverie::OperatorInsert)
        argumentValueNode = addCallNode->Arguments[0];
      else
        errorMessage = "Array initializer lists can only contain insertion "
                       "operator sub-expressions";
    }
    else
    {
      errorMessage = "It's invalid to have a member initializer statement in "
                     "an array initializer";
    }

    // If we got a valid node then translate it, otherwise send an error and
    // generate a dummy variable
    IRaverieShaderIR* argument = nullptr;
    if (argumentValueNode != nullptr)
    {
      argument = translator->WalkAndGetValueTypeResult(addCallNode->Arguments[0], context);
    }
    else
    {
      translator->SendTranslationError(expNode->Location, errorMessage);
      argument = translator->GenerateDummyIR(expNode, context);
    }

    compositeConstructOp->mArguments.PushBack(argument);
  }

  context->mCurrentBlock->AddOp(compositeConstructOp);
  context->PushIRStack(compositeConstructOp);

  // If the initializer list was put on a variable then we're just
  // attempting to set all of the values in a compact format.
  Raverie::LocalVariableNode* localVariableNode = Raverie::Type::DynamicCast<Raverie::LocalVariableNode*>(node->LeftOperand);
  if (localVariableNode == nullptr)
    return;

  // In this case, the initial value of the initializer node will be the
  // target variable to copy to. If this is a function call node then the this
  // is either the result from a function or a constructor call, both of which
  // require no copy back.
  Raverie::FunctionCallNode* functionCallNode =
      Raverie::Type::DynamicCast<Raverie::FunctionCallNode*>(localVariableNode->InitialValue);
  if (functionCallNode != nullptr)
    return;

  // Otherwise, this is should be either a local variable reference
  // or a member access (e.g. 'this.Array'). In either case this represents
  // where to store back. As long as the result of the initial value is
  // actually a pointer type then we can store to it, otherwise something
  // odd is happening (like array{1, 2} {1, 2}) so we report an error.
  IRaverieShaderIR* target = translator->WalkAndGetResult(localVariableNode->InitialValue, context);
  RaverieShaderIROp* targetOp = target->As<RaverieShaderIROp>();
  if (!targetOp->IsResultPointerType())
  {
    translator->SendTranslationError(node->Location,
                                     "Invalid array initializer list. The left "
                                     "hand side must be an l-value");
    context->PushIRStack(translator->GenerateDummyIR(node, context));
    return;
  }

  translator->BuildStoreOp(targetOp, compositeConstructOp, context);
}

void FixedArrayResolver(RaverieSpirVFrontEnd* translator, Raverie::BoundType* raverieFixedArrayType)
{
  RaverieShaderIRLibrary* shaderLibrary = translator->mLibrary;

  // Get the template arguments
  Raverie::Type* raverieElementType = raverieFixedArrayType->TemplateArguments[0].TypeValue;
  // Deal with nested template types that haven't already been resolved
  Raverie::BoundType* raverieElementBoundType = Raverie::Type::GetBoundType(raverieElementType);
  if (!raverieElementBoundType->TemplateBaseName.Empty())
    translator->PreWalkTemplateType(raverieElementBoundType, translator->mContext);
  RaverieShaderIRType* elementType = translator->FindType(raverieElementType, nullptr);
  int length = (int)raverieFixedArrayType->TemplateArguments[1].IntegerValue;

  // Create the array type
  RaverieShaderIRType* fixedArrayType = translator->MakeTypeAndPointer(shaderLibrary,
                                                                     ShaderIRTypeBaseType::FixedArray,
                                                                     raverieFixedArrayType->Name,
                                                                     raverieFixedArrayType,
                                                                     spv::StorageClassFunction);

  // Set the parameters (and currently components for convenience)
  fixedArrayType->mComponents = length;
  fixedArrayType->mParameters.PushBack(elementType);
  // The length has to be an integer constant, not literal
  fixedArrayType->mParameters.PushBack(translator->GetIntegerConstant(length, translator->mContext));
  translator->MakeShaderTypeMeta(fixedArrayType, nullptr);

  Raverie::BoundType* intType = RaverieTypeId(int);
  String intTypeName = intType->Name;

  // Register resolvers for the few functions we care about.
  TypeResolvers& typeResolver = shaderLibrary->mTypeResolvers[raverieFixedArrayType];
  typeResolver.mBackupConstructorResolver = FixedArrayBackupConstructor;
  typeResolver.mDefaultConstructorResolver = FixedArrayDefaultConstructor;
  typeResolver.RegisterFunctionResolver(
      GetMemberOverloadedFunction(raverieFixedArrayType, Raverie::OperatorGet, intTypeName), ResolveFixedArrayGet);
  typeResolver.RegisterFunctionResolver(
      GetMemberOverloadedFunction(raverieFixedArrayType, Raverie::OperatorSet, intTypeName, raverieElementType->ToString()),
      ResolveFixedArraySet);
  typeResolver.RegisterFunctionResolver(GetInstanceProperty(raverieFixedArrayType, "Count")->Get, ResolveFixedArrayCount);
  typeResolver.mExpressionInitializerListResolver = FixedArrayExpressionInitializerResolver;
}

void ResolveRuntimeArrayGet(RaverieSpirVFrontEnd* translator,
                            Raverie::FunctionCallNode* functionCallNode,
                            Raverie::MemberAccessNode* memberAccessNode,
                            RaverieSpirVFrontEndContext* context)
{
  // Get the 'this' array type and component type (from the containing struct
  // type)
  Raverie::Type* raverieArrayType = memberAccessNode->LeftOperand->ResultType;
  RaverieShaderIRType* structArrayType = translator->FindType(raverieArrayType, memberAccessNode->LeftOperand);
  RaverieShaderIRType* spirvArrayType = structArrayType->mParameters[0]->As<RaverieShaderIRType>();
  RaverieShaderIRType* elementType = spirvArrayType->mParameters[0]->As<RaverieShaderIRType>();

  // Get the index operator (must be a value type)
  IRaverieShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  RaverieShaderIROp* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Walk the left operand to get the wrapper struct type
  RaverieShaderIROp* constant0 = translator->GetIntegerConstant(0, context);
  IRaverieShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);

  // To get the actual data we'll do a double access chain. Argument 0 will
  // be the index to get the spirv runtime array (always index 0),
  // then argument 1 will be the index into the array.
  // Note: We have to do a special access chain so the result type is of the
  // correct storage class (uniform)
  RaverieShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  IRaverieShaderIR* accessChainOp =
      translator->BuildCurrentBlockAccessChain(elementType, selfInstance, constant0, indexOperand, context);

  context->PushIRStack(accessChainOp);
}

void ResolveRuntimeArraySet(RaverieSpirVFrontEnd* translator,
                            Raverie::FunctionCallNode* functionCallNode,
                            Raverie::MemberAccessNode* memberAccessNode,
                            RaverieSpirVFrontEndContext* context)
{
  // Get the 'this' array type and component type
  Raverie::Type* raverieArrayType = memberAccessNode->LeftOperand->ResultType;
  RaverieShaderIRType* structArrayType = translator->FindType(raverieArrayType, memberAccessNode->LeftOperand);
  RaverieShaderIRType* spirvArrayType = structArrayType->mParameters[0]->As<RaverieShaderIRType>();
  RaverieShaderIRType* elementType = spirvArrayType->mParameters[0]->As<RaverieShaderIRType>();

  // Get the index operator (must be a value type)
  IRaverieShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  RaverieShaderIROp* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Generate the access chain to get the element within the array
  RaverieShaderIROp* constant0 = translator->GetIntegerConstant(0, context);
  IRaverieShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  RaverieShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  IRaverieShaderIR* accessChainOp =
      translator->BuildCurrentBlockAccessChain(elementType, selfInstance, constant0, indexOperand, context);

  // Get the source value
  IRaverieShaderIR* sourceIR = translator->WalkAndGetResult(functionCallNode->Arguments[1], context);

  // Store the source into the target
  BasicBlock* currentBlock = context->GetCurrentBlock();
  translator->BuildStoreOp(currentBlock, accessChainOp, sourceIR, context);
}

void ResolveRuntimeArrayCount(RaverieSpirVFrontEnd* translator,
                              Raverie::FunctionCallNode* functionCallNode,
                              Raverie::MemberAccessNode* memberAccessNode,
                              RaverieSpirVFrontEndContext* context)
{
  // The runtime array length instruction is a bit odd as it requires the struct
  // the array is contained in as well as the member index offset into the
  // struct for where the runtime array is actually contained.
  // for where the runtime array is actually contained. Additionally, the return type
  // is required to be an unsigned int (which doesn't exist in raverie).
  // So a hack unsigned int type is returned and immediately casted to a signed int.
  RaverieShaderIRType* intType = translator->FindType(RaverieTypeId(int), functionCallNode);
  RaverieShaderIRType* uintType = translator->FindType(RaverieTypeId(Raverie::UnsignedInt), functionCallNode);
  // Get the wrapper struct instance that contains the real runtime array
  IRaverieShaderIR* structOwnerOp = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  // We create the runtime array wrapper struct such that the real array is always at member index 0
  RaverieShaderIRConstantLiteral* zeroLiteral = translator->GetOrCreateConstantIntegerLiteral(0);
  IRaverieShaderIR* uintLengthResult =
      translator->BuildCurrentBlockIROp(OpType::OpArrayLength, uintType, structOwnerOp, zeroLiteral, context);
  // Cast to a signed int. Note: This will do very bad things if the sign bit is set on the unsigned int.
  IRaverieShaderIR* intLengthResult =
      translator->BuildCurrentBlockIROp(OpType::OpBitcast, intType, uintLengthResult, context);
  context->PushIRStack(intLengthResult);
}

void RuntimeArrayResolver(RaverieSpirVFrontEnd* translator, Raverie::BoundType* raverieRuntimeArrayType)
{
  // Runtime arrays have to be created in an odd way. Per the Vulkan spec,
  // OpTypeRuntimeArray must only be used for the last member of an
  // OpTypeStruct. That is, a runtime array must be declared as (glsl sample):
  // buffer StructTypeName
  //{
  //  float ActualRuntimeArray[];
  //} InstanceVarName;
  // To do this the raverie runtime array is translated as the wrapper struct
  // that contains the spirv runtime array since all op codes must go through
  // the struct's instance variable.

  RaverieShaderIRLibrary* shaderLibrary = translator->mLibrary;

  // Get the template arguments
  Raverie::Type* raverieElementType = raverieRuntimeArrayType->TemplateArguments[0].TypeValue;
  // Deal with nested template types that haven't already been resolved
  Raverie::BoundType* raverieElementBoundType = Raverie::Type::GetBoundType(raverieElementType);
  if (!raverieElementBoundType->TemplateBaseName.Empty())
    translator->PreWalkTemplateType(raverieElementBoundType, translator->mContext);

  RaverieShaderIRType* elementType = translator->FindType(raverieElementType, nullptr);

  // The name of the wrapper struct type name must match the raverie runtime
  // array type name since the front end will be searching for the type by
  // this name. Make the internal runtime array type name something unique.
  String raverieTypeName = raverieRuntimeArrayType->Name;
  String internalArrayName = BuildString("SpirV", raverieTypeName);

  // Create the true runtime array type
  RaverieShaderIRType* runtimeArrayType = translator->MakeTypeAndPointer(
      shaderLibrary, ShaderIRTypeBaseType::RuntimeArray, internalArrayName, nullptr, spv::StorageClassStorageBuffer);
  runtimeArrayType->mParameters.PushBack(elementType);
  translator->MakeShaderTypeMeta(runtimeArrayType, nullptr);

  // Now generate the wrapper struct around the runtime array
  RaverieShaderIRType* wrapperStructType =
      translator->MakeStructType(shaderLibrary, raverieTypeName, raverieRuntimeArrayType, spv::StorageClassStorageBuffer);
  wrapperStructType->AddMember(runtimeArrayType, "Data");
  // Always use the actual type name with "Buffer" appended for the wrapper type
  // name
  wrapperStructType->mDebugResultName = BuildString(raverieTypeName, "Buffer");
  translator->MakeShaderTypeMeta(wrapperStructType, nullptr);

  Raverie::BoundType* intType = RaverieTypeId(int);
  String intTypeName = intType->Name;

  // Register resolvers for the few functions we care about.
  // Note: Add is illegal since this is provided by the client
  TypeResolvers& typeResolver = shaderLibrary->mTypeResolvers[raverieRuntimeArrayType];
  typeResolver.RegisterFunctionResolver(
      GetMemberOverloadedFunction(raverieRuntimeArrayType, Raverie::OperatorGet, intTypeName), ResolveRuntimeArrayGet);
  typeResolver.RegisterFunctionResolver(
      GetMemberOverloadedFunction(raverieRuntimeArrayType, Raverie::OperatorSet, intTypeName, raverieElementType->ToString()),
      ResolveRuntimeArraySet);
  typeResolver.RegisterFunctionResolver(GetInstanceProperty(raverieRuntimeArrayType, "Count")->Get,
                                        ResolveRuntimeArrayCount);
}

void GeometryStreamInputResolver(RaverieSpirVFrontEnd* translator, Raverie::BoundType* raverieInputStreamType)
{
  RaverieShaderIRLibrary* shaderLibrary = translator->mLibrary;
  RaverieSpirVFrontEndContext* context = translator->mContext;

  Raverie::GeometryStreamUserData* streamUserData = raverieInputStreamType->Has<Raverie::GeometryStreamUserData>();

  // Get the template arguments
  Raverie::Type* raverieElementType = raverieInputStreamType->TemplateArguments[0].TypeValue;
  RaverieShaderIRType* elementType = translator->FindType(raverieElementType, nullptr);
  int length = streamUserData->mSize;

  // Create the array type
  RaverieShaderIRType* inputStreamType = translator->MakeTypeAndPointer(shaderLibrary,
                                                                      ShaderIRTypeBaseType::FixedArray,
                                                                      raverieInputStreamType->Name,
                                                                      raverieInputStreamType,
                                                                      spv::StorageClassFunction);
  // Set the parameters (and currently components for convenience)
  inputStreamType->mComponents = length;
  inputStreamType->mParameters.PushBack(elementType);
  // The length has to be an integer constant, not literal
  inputStreamType->mParameters.PushBack(translator->GetIntegerConstant(length, context));
  translator->MakeShaderTypeMeta(inputStreamType, nullptr);

  Raverie::BoundType* raverieIntType = RaverieTypeId(int);
  String intTypeName = raverieIntType->Name;

  // Register resolvers for the few functions we care about.
  TypeResolvers& typeResolver = shaderLibrary->mTypeResolvers[raverieInputStreamType];
  typeResolver.mBackupConstructorResolver = FixedArrayBackupConstructor;
  typeResolver.mDefaultConstructorResolver = FixedArrayDefaultConstructor;
  typeResolver.RegisterFunctionResolver(
      GetMemberOverloadedFunction(raverieInputStreamType, Raverie::OperatorGet, intTypeName), ResolveFixedArrayGet);
  typeResolver.RegisterFunctionResolver(
      GetMemberOverloadedFunction(raverieInputStreamType, Raverie::OperatorSet, intTypeName, raverieElementType->ToString()),
      ResolveFixedArraySet);
  typeResolver.RegisterFunctionResolver(GetInstanceProperty(raverieInputStreamType, "Count")->Get,
                                        ResolveFixedArrayCount);
  typeResolver.mExpressionInitializerListResolver = FixedArrayExpressionInitializerResolver;
}

void OutputStreamRestart(RaverieSpirVFrontEnd* translator,
                         Raverie::FunctionCallNode* functionCallNode,
                         Raverie::MemberAccessNode* memberAccessNode,
                         RaverieSpirVFrontEndContext* context)
{
  translator->BuildCurrentBlockIROp(OpType::OpEndPrimitive, nullptr, context);
}

void GeometryStreamOutputResolver(RaverieSpirVFrontEnd* translator, Raverie::BoundType* raverieFixedArrayType)
{
  RaverieShaderIRLibrary* shaderLibrary = translator->mLibrary;
  RaverieSpirVFrontEndContext* context = translator->mContext;

  // Get the template arguments
  Raverie::Type* raverieElementType = raverieFixedArrayType->TemplateArguments[0].TypeValue;
  RaverieShaderIRType* elementType = translator->FindType(raverieElementType, nullptr);

  Raverie::BoundType* raverieIntType = RaverieTypeId(int);
  String intTypeName = raverieIntType->Name;
  RaverieShaderIRType* intType = translator->mLibrary->FindType(raverieIntType);

  // Create the array type
  RaverieShaderIRType* fixedArrayType = translator->MakeTypeAndPointer(shaderLibrary,
                                                                     ShaderIRTypeBaseType::Struct,
                                                                     raverieFixedArrayType->Name,
                                                                     raverieFixedArrayType,
                                                                     spv::StorageClassFunction);
  fixedArrayType->mDebugResultName = "OutputStream";
  // Add the element type
  fixedArrayType->AddMember(elementType, "Output");
  translator->MakeShaderTypeMeta(fixedArrayType, nullptr);

  // Create the append function. We need this to be an actual function that will
  // be late bound later via the entry point.
  Raverie::Function* raverieAppendFn =
      GetMemberOverloadedFunction(raverieFixedArrayType, "Append", raverieElementType->ToString(), intTypeName);
  RaverieShaderIRFunction* appendFn =
      translator->GenerateIRFunction(nullptr, nullptr, fixedArrayType, raverieAppendFn, raverieAppendFn->Name, context);
  // Add the parameters for the function
  translator->BuildIROp(&appendFn->mParameterBlock, OpType::OpFunctionParameter, fixedArrayType->mPointerType, context)
      ->mDebugResultName = "stream";
  translator->BuildIROp(&appendFn->mParameterBlock, OpType::OpFunctionParameter, elementType, context)
      ->mDebugResultName = "outputData";
  translator->BuildIROp(&appendFn->mParameterBlock, OpType::OpFunctionParameter, intType, context)->mDebugResultName =
      "vertexId";
  // Make this a valid function by adding the first block with a return
  // statement
  BasicBlock* firstBlock = translator->BuildBlockNoStack(String(), context);
  appendFn->mBlocks.PushBack(firstBlock);
  translator->BuildIROp(firstBlock, OpType::OpReturn, nullptr, context);

  // Register resolvers for the few functions we care about.
  TypeResolvers& typeResolver = shaderLibrary->mTypeResolvers[raverieFixedArrayType];
  typeResolver.mBackupConstructorResolver = FixedArrayBackupConstructor;
  typeResolver.mDefaultConstructorResolver = FixedArrayDefaultConstructor;
  typeResolver.RegisterFunctionResolver(GetMemberOverloadedFunction(raverieFixedArrayType, "Restart"),
                                        OutputStreamRestart);
  typeResolver.mExpressionInitializerListResolver = FixedArrayExpressionInitializerResolver;
}

} // namespace Raverie
