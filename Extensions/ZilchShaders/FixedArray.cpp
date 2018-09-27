///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

void FixedArrayDefaultConstructor(ZilchSpirVFrontEnd* translator, Zilch::Type* resultType, ZilchSpirVFrontEndContext* context)
{
  // Just build a variable op for the given fixed array type
  ZilchShaderIRType* shaderType = translator->FindType(resultType, nullptr);
  context->PushIRStack(translator->BuildOpVariable(shaderType->mPointerType, context));
}

void FixedArrayBackupConstructor(ZilchSpirVFrontEnd* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::StaticTypeNode* staticTypeNode, ZilchSpirVFrontEndContext* context)
{
  FixedArrayDefaultConstructor(translator, staticTypeNode->ResultType, context);
}

bool ValidateIndexLiteral(ZilchSpirVFrontEnd* translator, Zilch::ExpressionNode* node, ZilchShaderIROp* indexOperand, int maxValue, ZilchSpirVFrontEndContext* context)
{
  // Check if this is a constant (literal). If not we can't validate.
  if(indexOperand->mOpType != OpType::OpConstant)
    return true;

  // Get the literal value
  ZilchShaderIRConstantLiteral* literal = indexOperand->mArguments[0]->As<ZilchShaderIRConstantLiteral>();
  int indexLiteral = literal->mValue.Get<int>();
  // If it's in the valid range then this index is valid;
  if(0 <= indexLiteral && indexLiteral < maxValue)
    return true;

  // Otherwise the value is invalid so report an error and emit a dummy op-code
  String message = String::Format("Invalid index. Index can only be in the range of [0, %d)", maxValue);
  translator->SendTranslationError(node->Location, message);
  context->PushIRStack(translator->GenerateDummyIR(node, context));
  return false;
}

void ResolveFixedArrayGet(ZilchSpirVFrontEnd* translator, Zilch::FunctionCallNode* functionCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchSpirVFrontEndContext* context)
{
  // Get the 'this' array type and component type
  Zilch::Type* zilchArrayType = memberAccessNode->LeftOperand->ResultType;
  ZilchShaderIRType* arrayType = translator->FindType(zilchArrayType, memberAccessNode->LeftOperand);
  ZilchShaderIRType* elementType = arrayType->mParameters[0]->As<ZilchShaderIRType>();

  // Get the index operator (must be a value type)
  IZilchShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  ZilchShaderIROp* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Validate the index if it's a literal
  if(!ValidateIndexLiteral(translator, functionCallNode, indexOperand, arrayType->mComponents, context))
    return;

  // Generate the access chain to get the element within the array
  IZilchShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  ZilchShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  IZilchShaderIR* accessChainOp = translator->BuildCurrentBlockIROp(OpType::OpAccessChain, elementType->mPointerType, selfInstance, indexOperand, context);

  context->PushIRStack(accessChainOp);
}

void ResolveFixedArraySet(ZilchSpirVFrontEnd* translator, Zilch::FunctionCallNode* functionCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchSpirVFrontEndContext* context)
{
  // Get the 'this' array type and component type
  Zilch::Type* zilchArrayType = memberAccessNode->LeftOperand->ResultType;
  ZilchShaderIRType* arrayType = translator->FindType(zilchArrayType, memberAccessNode->LeftOperand);
  ZilchShaderIRType* elementType = arrayType->mParameters[0]->As<ZilchShaderIRType>();

  // Get the index operator (must be a value type)
  IZilchShaderIR* indexArgument = translator->WalkAndGetResult(functionCallNode->Arguments[0], context);
  ZilchShaderIROp* indexOperand = translator->GetOrGenerateValueTypeFromIR(indexArgument, context);

  // Validate the index if it's a literal
  if(!ValidateIndexLiteral(translator, functionCallNode, indexOperand, arrayType->mComponents, context))
    return;

  // Generate the access chain to get the element within the array
  IZilchShaderIR* leftOperand = translator->WalkAndGetResult(memberAccessNode->LeftOperand, context);
  ZilchShaderIROp* selfInstance = translator->GetOrGeneratePointerTypeFromIR(leftOperand, context);
  IZilchShaderIR* accessChainOp = translator->BuildCurrentBlockIROp(OpType::OpAccessChain, elementType->mPointerType, selfInstance, indexOperand, context);

  // Get the source value
  IZilchShaderIR* sourceIR = translator->WalkAndGetResult(functionCallNode->Arguments[1], context);

  // Store the source into the target
  BasicBlock* currentBlock = context->GetCurrentBlock();
  translator->BuildStoreOp(currentBlock, accessChainOp, sourceIR, context);
}

void ResolveFixedArrayCount(ZilchSpirVFrontEnd* translator, Zilch::FunctionCallNode* functionCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchSpirVFrontEndContext* context)
{
  // Return the integer constant expression as the array count (could call the intrinsic for count but why not use this)
  Zilch::Type* zilchArrayType = memberAccessNode->LeftOperand->ResultType;
  ZilchShaderIRType* arrayType = translator->FindType(zilchArrayType, memberAccessNode->LeftOperand);
  context->PushIRStack(arrayType->mParameters[1]);
}

void FixedArrayExpressionInitializerResolver(ZilchSpirVFrontEnd* translator, Zilch::ExpressionInitializerNode*& node, ZilchSpirVFrontEndContext* context)
{
  ZilchShaderIRType* fixedArrayType = translator->FindType(node->ResultType, node);

  // Validate that the initializer list has the exact number of expected arguments
  size_t statementCount = node->InitializerStatements.Size();
  if(fixedArrayType->mComponents != statementCount)
  {
    String errMsg = String::Format("Array initializer was given %d item(s) and it expected %d.", statementCount, fixedArrayType->mComponents);
    translator->SendTranslationError(node->Location, errMsg);
    context->PushIRStack(translator->GenerateDummyIR(node, context));
    return;
  }

  // Start constructing the array by walking all of the arguments
  ZilchShaderIROp * compositeConstructOp = translator->BuildIROpNoBlockAdd(OpType::OpCompositeConstruct, fixedArrayType, context);
  for(size_t i = 0; i < node->InitializerStatements.Size(); ++i)
  {
    Zilch::ExpressionNode* expNode = node->InitializerStatements[i];

    String errorMessage;
    Zilch::ExpressionNode* argumentValueNode = nullptr;

    // Verify that this statement is a call to OperatorInsert (.Add). If it is then walk the
    // argument to get the initial value we set (required to be a value type).
    Zilch::FunctionCallNode* addCallNode = Zilch::Type::DynamicCast<Zilch::FunctionCallNode*>(expNode);
    if(addCallNode != nullptr)
    {
      Zilch::MemberAccessNode* memberAccessNode = Zilch::Type::DynamicCast<Zilch::MemberAccessNode*>(addCallNode->LeftOperand);
      if(memberAccessNode != nullptr && memberAccessNode->Name == Zilch::OperatorInsert)
        argumentValueNode = addCallNode->Arguments[0];
      else
        errorMessage = "Array initializer lists can only contain insertion operator sub-expressions";
    }
    else
    {
      errorMessage = "It's invalid to have a member initializer statement in an array initializer";
    }

    // If we got a valid node then translate it, otherwise send an error and generate a dummy variable
    IZilchShaderIR* argument = nullptr;
    if(argumentValueNode != nullptr)
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
  Zilch::LocalVariableNode* localVariableNode = Zilch::Type::DynamicCast<Zilch::LocalVariableNode*>(node->LeftOperand);
  if(localVariableNode == nullptr)
    return;

  // In this case, the initial value of the initializer node will be the
  // target variable to copy to. If this is a function call node then the this is 
  // either the result from a function or a constructor call, both of which require no copy back.
  Zilch::FunctionCallNode* functionCallNode = Zilch::Type::DynamicCast<Zilch::FunctionCallNode*>(localVariableNode->InitialValue);
  if(functionCallNode != nullptr)
    return;

  // Otherwise, this is should be either a local variable reference
  // or a member access (e.g. 'this.Array'). In either case this represents
  // where to store back. As long as the result of the initial value is 
  // actually a pointer type then we can store to it, otherwise something
  // odd is happening (like array{1, 2} {1, 2}) so we report an error.
  IZilchShaderIR* target = translator->WalkAndGetResult(localVariableNode->InitialValue, context);
  ZilchShaderIROp* targetOp = target->As<ZilchShaderIROp>();
  if(!targetOp->IsResultPointerType())
  {
    translator->SendTranslationError(node->Location, "Invalid array initializer list. The left hand side must be an l-value");
    context->PushIRStack(translator->GenerateDummyIR(node, context));
    return;
  }
    
  translator->BuildStoreOp(targetOp, compositeConstructOp, context);
}

void FixedArrayResolver(ZilchSpirVFrontEnd* translator, Zilch::BoundType* zilchFixedArrayType)
{
  ZilchShaderIRLibrary* shaderLibrary = translator->mLibrary;

  // Get the template arguments
  Zilch::Type* zilchElementType = zilchFixedArrayType->TemplateArguments[0].TypeValue;
  ZilchShaderIRType* elementType = translator->FindType(zilchElementType, nullptr);
  int length = (int)zilchFixedArrayType->TemplateArguments[1].IntegerValue;

  // Create the array type
  ZilchShaderIRType* fixedArrayType = translator->MakeTypeAndPointer(shaderLibrary, ShaderIRTypeBaseType::FixedArray, zilchFixedArrayType->Name, zilchFixedArrayType, spv::StorageClassFunction);

  // Set the parameters (and currently components for convenience)
  fixedArrayType->mComponents = length;
  fixedArrayType->mParameters.PushBack(elementType);
  // The length has to be an integer constant, not literal
  fixedArrayType->mParameters.PushBack(translator->GetIntegerConstant(length, translator->mContext));
  translator->MakeShaderTypeMeta(fixedArrayType, nullptr);

  Zilch::BoundType* intType = ZilchTypeId(int);
  String intTypeName = intType->Name;

  // Register resolvers for the few functions we care about.
  TypeResolvers& typeResolver = shaderLibrary->mTypeResolvers[zilchFixedArrayType];
  typeResolver.mBackupConstructorResolver = FixedArrayBackupConstructor;
  typeResolver.mDefaultConstructorResolver = FixedArrayDefaultConstructor;
  typeResolver.RegisterFunctionResolver(GetMemberOverloadedFunction(zilchFixedArrayType, Zilch::OperatorGet, intTypeName), ResolveFixedArrayGet);
  typeResolver.RegisterFunctionResolver(GetMemberOverloadedFunction(zilchFixedArrayType, Zilch::OperatorSet, intTypeName, zilchElementType->ToString()), ResolveFixedArraySet);
  typeResolver.RegisterFunctionResolver(GetInstanceProperty(zilchFixedArrayType, "Count")->Get, ResolveFixedArrayCount);
  typeResolver.mExpressionInitializerListResolver = FixedArrayExpressionInitializerResolver;
}

void GeometryStreamInputResolver(ZilchSpirVFrontEnd* translator, Zilch::BoundType* zilchInputStreamType)
{
  ZilchShaderIRLibrary* shaderLibrary = translator->mLibrary;
  ZilchSpirVFrontEndContext* context = translator->mContext;

  Zilch::GeometryStreamUserData* streamUserData = zilchInputStreamType->Has<Zilch::GeometryStreamUserData>();

  // Get the template arguments
  Zilch::Type* zilchElementType = zilchInputStreamType->TemplateArguments[0].TypeValue;
  ZilchShaderIRType* elementType = translator->FindType(zilchElementType, nullptr);
  int length = streamUserData->mSize;

  // Create the array type
  ZilchShaderIRType* inputStreamType = translator->MakeTypeAndPointer(shaderLibrary, ShaderIRTypeBaseType::FixedArray, zilchInputStreamType->Name, zilchInputStreamType, spv::StorageClassFunction);
  // Set the parameters (and currently components for convenience)
  inputStreamType->mComponents = length;
  inputStreamType->mParameters.PushBack(elementType);
  // The length has to be an integer constant, not literal
  inputStreamType->mParameters.PushBack(translator->GetIntegerConstant(length, context));
  translator->MakeShaderTypeMeta(inputStreamType, nullptr);

  Zilch::BoundType* zilchIntType = ZilchTypeId(int);
  String intTypeName = zilchIntType->Name;

  // Register resolvers for the few functions we care about.
  TypeResolvers& typeResolver = shaderLibrary->mTypeResolvers[zilchInputStreamType];
  typeResolver.mBackupConstructorResolver = FixedArrayBackupConstructor;
  typeResolver.mDefaultConstructorResolver = FixedArrayDefaultConstructor;
  typeResolver.RegisterFunctionResolver(GetMemberOverloadedFunction(zilchInputStreamType, Zilch::OperatorGet, intTypeName), ResolveFixedArrayGet);
  typeResolver.RegisterFunctionResolver(GetMemberOverloadedFunction(zilchInputStreamType, Zilch::OperatorSet, intTypeName, zilchElementType->ToString()), ResolveFixedArraySet);
  typeResolver.RegisterFunctionResolver(GetInstanceProperty(zilchInputStreamType, "Count")->Get, ResolveFixedArrayCount);
  typeResolver.mExpressionInitializerListResolver = FixedArrayExpressionInitializerResolver;
}

void OutputStreamRestart(ZilchSpirVFrontEnd* translator, Zilch::FunctionCallNode* functionCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchSpirVFrontEndContext* context)
{
  translator->BuildCurrentBlockIROp(OpType::OpEndPrimitive, nullptr, context);
}

void GeometryStreamOutputResolver(ZilchSpirVFrontEnd* translator, Zilch::BoundType* zilchFixedArrayType)
{
  ZilchShaderIRLibrary* shaderLibrary = translator->mLibrary;
  ZilchSpirVFrontEndContext* context = translator->mContext;

  // Get the template arguments
  Zilch::Type* zilchElementType = zilchFixedArrayType->TemplateArguments[0].TypeValue;
  ZilchShaderIRType* elementType = translator->FindType(zilchElementType, nullptr);

  Zilch::BoundType* zilchIntType = ZilchTypeId(int);
  String intTypeName = zilchIntType->Name;
  ZilchShaderIRType* intType = translator->mLibrary->FindType(zilchIntType);

  // Create the array type
  ZilchShaderIRType* fixedArrayType = translator->MakeTypeAndPointer(shaderLibrary, ShaderIRTypeBaseType::Struct, zilchFixedArrayType->Name, zilchFixedArrayType, spv::StorageClassFunction);
  fixedArrayType->mDebugResultName = "OutputStream";
  // Add the element type
  fixedArrayType->AddMember(elementType, "Output");
  translator->MakeShaderTypeMeta(fixedArrayType, nullptr);

  // Create the append function. We need this to be an actual function that will be late bound later via the entry point.
  Zilch::Function* zilchAppendFn = GetMemberOverloadedFunction(zilchFixedArrayType, "Append", zilchElementType->ToString(), intTypeName);
  ZilchShaderIRFunction* appendFn = translator->GenerateIRFunction(nullptr, nullptr, fixedArrayType, zilchAppendFn, zilchAppendFn->Name, context);
  // Add the parameters for the function
  translator->BuildIROp(&appendFn->mParameterBlock, OpType::OpFunctionParameter, fixedArrayType->mPointerType, context)->mDebugResultName = "stream";
  translator->BuildIROp(&appendFn->mParameterBlock, OpType::OpFunctionParameter, elementType, context)->mDebugResultName = "outputData";
  translator->BuildIROp(&appendFn->mParameterBlock, OpType::OpFunctionParameter, intType, context)->mDebugResultName = "vertexId";
  // Make this a valid function by adding the first block with a return statement
  BasicBlock* firstBlock = translator->BuildBlockNoStack(String(), context);
  appendFn->mBlocks.PushBack(firstBlock);
  translator->BuildIROp(firstBlock, OpType::OpReturn, nullptr, context);

  // Register resolvers for the few functions we care about.
  TypeResolvers& typeResolver = shaderLibrary->mTypeResolvers[zilchFixedArrayType];
  typeResolver.mBackupConstructorResolver = FixedArrayBackupConstructor;
  typeResolver.mDefaultConstructorResolver = FixedArrayDefaultConstructor;
  typeResolver.RegisterFunctionResolver(GetMemberOverloadedFunction(zilchFixedArrayType, "Restart"), OutputStreamRestart);
  typeResolver.mExpressionInitializerListResolver = FixedArrayExpressionInitializerResolver;
}

}//namespace Zero
