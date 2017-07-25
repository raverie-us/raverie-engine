///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

void ResolveBackupScalarSwizzle(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  String data = memberAccessNode->Name;
  bool invalid = false;
  // We only allow swizzles up to a vector 4 (so anything longer is an error)
  if(data.SizeInBytes() > 4)
    invalid = true;
  // Make sure that every value is X (nothing else is allowed)
  for(StringRange dataRange = data.All(); !dataRange.Empty(); dataRange.PopFront())
  {
    if(data.Front() != 'X')
      invalid = true;
  }

  if(!invalid)
  {
    ShaderType* resultShaderType = translator->FindShaderType(memberAccessNode->ResultType, memberAccessNode);
    
    context->GetBuilder() << resultShaderType->mShaderName << "(";
    context->Walker->Walk(translator, memberAccessNode->LeftOperand, context);
    context->GetBuilder() << ")";
  }
  else
  {
    String leftOpType = memberAccessNode->LeftOperand->ResultType->ToString();
    String msg = String::Format("The member access '%s' on type '%s' is unable to be translated.", data.c_str(), leftOpType.c_str());
    translator->SendTranslationError(memberAccessNode->Location, "Member access cannot be translated.", msg);
  }
}

void ResolveSimpleBackupConstructor(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::StaticTypeNode* staticTypeNode, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();
  context->Walker->Walk(translator, fnCallNode->LeftOperand, context);
  translator->WriteFunctionCall(fnCallNode->Arguments, nullptr, nullptr, context);
}

void ResolveMatrixMultiply(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();
  // For now ensure order of operations by wrapping in parenthesis
  builder << "(";
  context->Walker->Walk(translator, fnCallNode->Arguments[1], context);
  builder << " * ";
  context->Walker->Walk(translator, fnCallNode->Arguments[0], context);
  builder << ")";
}

void ResolveFixedArrayDefaultConstructor(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::StaticTypeNode* staticTypeNode, ZilchShaderTranslatorContext* context)
{
  // Default constructors need to properly construct the array with the pre-initialized value.
  // To do this we have to get the pre-initializer string for one item in the array and then splat that for all paramaters

  // Get the array's type, the first parameter's type, and the size of the array
  Zilch::BoundType* boundType = Zilch::BoundType::GetBoundType(staticTypeNode->ResultType);
  Zilch::Type* templateType = boundType->TemplateArguments[0].TypeValue;
  ShaderType* arrayShaderType = translator->FindShaderType(boundType, fnCallNode);
  size_t fixedSize = arrayShaderType->GetTypeData()->mCount;

  String constructorCall = translator->GenerateDefaultConstructorString(templateType, context);

  ShaderCodeBuilder& builder = context->GetBuilder();
  // Write: "`ArrayType`(`constructorCall`, `constructorCall`, ...)
  builder << arrayShaderType->mShaderName;

  builder.Write("(");
  builder.Write(JoinRepeatedString(constructorCall, ", ", fixedSize));
  builder.Write(")");
}

void ResolveFixedArrayGet(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();
  // Change array.Get(index) into array[index]
  context->Walker->Walk(translator, memberAccessNode->LeftOperand, context);
  builder.Write("[");
  context->Walker->Walk(translator, fnCallNode->Arguments[0], context);
  builder.Write("]");
}

void ResolveFixedArraySet(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();
  // Change array.Set(index, value) into array[index] = value
  context->Walker->Walk(translator, memberAccessNode->LeftOperand, context);
  builder.Write("[");
  context->Walker->Walk(translator, fnCallNode->Arguments[0], context);
  builder.Write("]");
  builder.Write(" = ");
  context->Walker->Walk(translator, fnCallNode->Arguments[1], context);
}

void ResolveFixedArrayAdd(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  // Report an error if the user tries to call the add function. This function has to exist for initializer lists so we can't remove it.
  // We don't actually visit the node when constructing the initializer list so it should be a problem to always make this an error to call.
  translator->SendTranslationError(fnCallNode->Location, "FixedArray's Add function cannot be translated as shaders only support static arrays. The Add function currently exists for initializer lists.");
}

void ResolveFixedArrayCount(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  // Change array.Count into array.Length()
  context->Walker->Walk(translator, memberAccessNode->LeftOperand, context);
  ShaderCodeBuilder& builder = context->GetBuilder();
  builder.Write(".length()");
}

void Glsl13FixedArrayInitializerResolver(ZilchShaderTranslator* translator, Zilch::Type* type, Zilch::ExpressionInitializerNode* initializerNode, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();

  Zilch::BoundType* arrayType = Zilch::BoundType::GetBoundType(type);
  // Get the type the array is templated on as well as the size of the fixed array
  Zilch::Type* templateType = arrayType->TemplateArguments[0].TypeValue;
  size_t fixedSize = (size_t)arrayType->TemplateArguments[1].IntegerValue;

  // If there's no initializer expression or it's just the default constructor
  // (meaning there's no initializer statements) then don't try to build up the array initializer.
  size_t statementCount = initializerNode->InitializerStatements.Size();
  if(initializerNode == nullptr || statementCount == 0)
    return;

  // Report an error if the number of statements in the initializer list is not equal to the fixed array's size
  if(statementCount != fixedSize)
  {
    String errMsg = String::Format("Array initializer was only given %d items and it must have %d.", statementCount, fixedSize);
    translator->SendTranslationError(initializerNode->Location, errMsg);
    return;
  }

  // Iterate over all of the initializer statements and build up the array's initializer list
  ShaderCodeBuilder initializerListBuilder;
  for(size_t i = 0; i < initializerNode->InitializerStatements.Size(); ++i)
  {
    Zilch::ExpressionNode* expNode = initializerNode->InitializerStatements[i];

    // Array initializers are just semantic sugar and they actually translate to a whole bunch of array.Add(value) calls.
    // Try to get the function call node for Zilch::OperatorInsert.
    // If this isn't a function call node then we have a problem (throw an error).
    Zilch::FunctionCallNode* addCallNode = Zilch::Type::DynamicCast<Zilch::FunctionCallNode*>(expNode);
    if(addCallNode == nullptr)
    {
      String errMsg = "It's invalid to have a member initializer statement in an array initializer";
      translator->SendTranslationError(expNode->Location, errMsg);
      return;
    }

    // Now that we know it's a function call node, make sure that the function being called 
    // is the OperatorInsert function ("Add"), otherwise throw another error.
    Zilch::MemberAccessNode* memberAccessNode = Zilch::Type::DynamicCast<Zilch::MemberAccessNode*>(addCallNode->LeftOperand);
    if(memberAccessNode == nullptr || memberAccessNode->Name != Zilch::OperatorInsert)
    {
      String errMsg = "Array initializer lists can only contain insertion operator sub-expressions";
      translator->SendTranslationError(expNode->Location, errMsg);
      return;
    }
    
    // Collect the entire sub-expression from the function call (just the first argument)
    ScopedShaderCodeBuilder argumentSubBuilder(context);
    context->Walker->Walk(translator, addCallNode->Arguments[0], context);
    argumentSubBuilder.PopFromStack();
    // Store the argument into our builder for the initializer list
    initializerListBuilder << argumentSubBuilder.ToString();
    
    // Add the separating ',' if we're not at the last item
    if(i != initializerNode->AddValues.Size() - 1)
      initializerListBuilder << ", ";
  }

  // Now write out the initialization. For FixedArray[Real, 2] this will be float[2].
  ShaderType* arrayShaderType = translator->FindShaderType(type, initializerNode);
  builder << arrayShaderType->mShaderName;

  // Now if we built up an initializer expression then write it out. Glsl uses parenthesis for initialization
  // i.e. Array[Real, 2]{1.0, 2.0} -> float[2](1.0, 2.0)
  String initializerExp = initializerListBuilder.ToString();
  if(!initializerExp.Empty())
    builder << "(" << initializerExp << ")";
}

void RegisterFixedArrayTypeAndFunctions(ZilchShaderTranslator* translator, Zilch::BoundType* arrayType, Zilch::SyntaxNode* nodeLocation, String& resultTypeName)
{
  // Mark that we've registered this template's callbacks
  translator->mLibraryTranslator.mRegisteredTemplates.Insert(arrayType->ToString());

  String refKeyword = translator->mSettings->mNameSettings.mReferenceKeyword;

  Zilch::Type* templateType = arrayType->TemplateArguments[0].TypeValue;
  // Get the type the array is templated on as well as the size of the fixed array
  String templateTypeStr = templateType->ToString();
  size_t fixedSize = (size_t)arrayType->TemplateArguments[1].IntegerValue;
    
  // Translate the fixed array's type to the shader type
  ShaderType* fixedArrayTemplateType = translator->FindShaderType(templateType, nodeLocation);
  
  // Build up the glsl name of the fixed array type (i.e. FixedArray[Real, 3] -> float[3])
  String glslFixedArrayType = String::Format("%s[%d]", fixedArrayTemplateType->mShaderName.c_str(), fixedSize);
  // Try and find this array type if it already exists, otherwise create a
  // native type for it (native so we don't try to generate a struct for it)
  ShaderType* fixedArrayShaderType = translator->mCurrentLibrary->FindType(arrayType->ToString());
  if(fixedArrayShaderType == nullptr)
    fixedArrayShaderType = translator->mCurrentLibrary->CreateNativeType(arrayType->ToString(), glslFixedArrayType, refKeyword);
  fixedArrayShaderType->GetTypeData()->mCount = fixedSize;
  // Mark which library owns this array
  fixedArrayShaderType->mOwningLibrary = translator->mCurrentLibrary;
  // Add a dependency to the template argument
  fixedArrayShaderType->AddDependency(fixedArrayTemplateType);
  // Also register the initializer node resolver (so we can translate initializer statements)
  translator->mLibraryTranslator.RegisterInitializerNodeResolver(arrayType, Glsl13FixedArrayInitializerResolver);

  // Register a callback for the default constructor on this type
  // (mainly to just consume the array's default constructor which we don't translate)
  Zilch::Function* defaultConstructor = Zilch::BoundType::GetDefaultConstructor(arrayType->GetOverloadedInheritedConstructors());
  if(defaultConstructor != nullptr)
    translator->mLibraryTranslator.RegisterFunctionCallResolver(defaultConstructor, ResolveFixedArrayDefaultConstructor);

  // Also register any array functions we need to translate
  translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(arrayType, Zilch::OperatorGet, "Integer"), ResolveFixedArrayGet);
  translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(arrayType, Zilch::OperatorSet, "Integer", templateTypeStr), ResolveFixedArraySet);
  translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(arrayType, Zilch::OperatorInsert, templateTypeStr), ResolveFixedArrayAdd);
  translator->mLibraryTranslator.RegisterMemberAccessResolver(GetInstanceProperty(arrayType, "Count"), ResolveFixedArrayCount);
}

void GenerateDummyClass(ZilchShaderTranslator* translator, ShaderType* shaderType, Zilch::SyntaxNode* nodeLocation)
{
  NameSettings& nameSettings = translator->mSettings->mNameSettings;
  String thisKeyword = nameSettings.mThisKeyword;

  // Create the default constructor shader function
  ShaderFunction* constructor = shaderType->FindOrCreateFunction(nameSettings.mConstructorName, ZilchShaderSettings::GetDefaultConstructorKey());
  constructor->mShaderReturnType = shaderType->mShaderName;
  constructor->mShaderSignature = "()";
  constructor->mShaderName = translator->MangleName(nameSettings.mConstructorName, shaderType);
  
  // Build the constructor's translation (just construct and return the type, no preconstructor)
  ShaderCodeBuilder constructorBodyBuilder;
  constructorBodyBuilder.BeginScope();
  constructorBodyBuilder << constructorBodyBuilder.EmitIndent() << shaderType->mShaderName << " " << thisKeyword << ";" << constructorBodyBuilder.EmitLineReturn();
  constructorBodyBuilder << constructorBodyBuilder.EmitIndent() << "return " << thisKeyword << ";" << constructorBodyBuilder.EmitLineReturn();
  constructorBodyBuilder.EndScope();
  constructor->mShaderBodyCode = constructorBodyBuilder.ToString();

  // Glsl doesn't allow empty structs so generate a dummy variable
  translator->GenerateDummyMemberVariable(shaderType);
}

ShaderType* CreateBasicTemplateType(ZilchShaderTranslator* translator, Zilch::BoundType* outputType, Zilch::SyntaxNode* nodeLocation)
{
  // Create the shader type for the bound shader type if it didn't exist
  ShaderType* templateShaderType = translator->mCurrentLibrary->FindType(outputType->ToString());
  if(templateShaderType == nullptr)
    templateShaderType = translator->mCurrentLibrary->CreateType(outputType);
  templateShaderType->SetNative(true);

  // Translate the template class' name
  templateShaderType->mShaderName = translator->FixClassName(outputType);
  // Register dependencies on all template parameters
  for(size_t i = 0; i < outputType->TemplateArguments.Size(); ++i)
  {
    Zilch::Type* templateType = outputType->TemplateArguments[i].TypeValue;
    ShaderType* templateShaderType = translator->FindShaderType(templateType, nodeLocation);
    templateShaderType->AddDependency(templateShaderType);
  }
  
  GenerateDummyClass(translator, templateShaderType, nodeLocation);
  return templateShaderType;
}

void RegisterGeometryInputTypeAndFunctions(ZilchShaderTranslator* translator, Zilch::BoundType* boundType, Zilch::SyntaxNode* nodeLocation, String& resultTypeName, int count, StringParam primitiveType)
{
  // Geometry inputs register like a fixed array that's size is determined by the type (ie. TriangleOutput is of size 3).

  LibraryTranslator& libraryTranslator = translator->mLibraryTranslator;
  ZilchShaderLibrary* currentLibrary = translator->mCurrentLibrary;
  String refKeyword = translator->mSettings->mNameSettings.mReferenceKeyword;
  // Mark that we've registered this template's callbacks
  libraryTranslator.mRegisteredTemplates.Insert(boundType->ToString());

  // Get the shader type for the input struct type (the template parameter)
  Zilch::Type* inputDataType = boundType->TemplateArguments[0].TypeValue;
  ShaderType* inputDataShaderType = translator->FindShaderType(inputDataType, nodeLocation);
  
  // Build up the glsl name of the input type (i.e. TriangleOutput[Struct] -> Struct[3])
  String zilchInputStreamTypeName = boundType->ToString();
  String glslInputStreamTypeName = String::Format("%s[%d]", inputDataShaderType->mShaderName.c_str(), count);

  // Find the input stream type if it existed, otherwise create it as a native type (native so no struct is created)
  ShaderType* inputStreamShaderType = currentLibrary->FindType(zilchInputStreamTypeName);
  if(inputStreamShaderType == nullptr)
    inputStreamShaderType = currentLibrary->CreateNativeType(zilchInputStreamTypeName, glslInputStreamTypeName, refKeyword);

  // Set some meta information about this type
  ShaderTypeData* typeData = inputStreamShaderType->GetTypeData();
  typeData->mType = ShaderVarType::GeometryInput;
  typeData->mCount = count;
  typeData->mExtraData = primitiveType;

  // Mark that the input stream is dependent on its template parameter
  inputStreamShaderType->AddDependency(inputDataShaderType);

  // Register the main function translations
  Zilch::Function* defaultConstructor = Zilch::BoundType::GetDefaultConstructor(boundType->GetOverloadedInheritedConstructors());
  if(defaultConstructor != nullptr)
    libraryTranslator.RegisterFunctionCallResolver(defaultConstructor, ResolveFixedArrayDefaultConstructor);
  libraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(boundType, "Get", "Integer"), ResolveFixedArrayGet);
  libraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(boundType, "Set", "Integer", inputDataShaderType->mZilchName), ResolveFixedArraySet);
  libraryTranslator.RegisterMemberAccessResolver(GetInstanceProperty(boundType, "Count"), ResolveFixedArrayCount);
}

void GenerateOutputTypeAppend(ZilchShaderTranslator* translator, Zilch::BoundType* outputStreamBoundType, Zilch::SyntaxNode* nodeLocation, ShaderType* outputStreamShaderType)
{
  Zilch::BoundType* outputDataType = Zilch::BoundType::GetBoundType(outputStreamBoundType->TemplateArguments[0].TypeValue);
  ShaderType* outputDataShaderType = translator->FindShaderType(outputDataType, nodeLocation);
  
  // Find the append function
  String integerTypeName = Zilch::Core::GetInstance().IntegerType->ToString();
  Zilch::Function* appendZilchFunction = GetMemberOverloadedFunction(outputStreamBoundType, "Append", outputDataType->ToString(), integerTypeName);
  // Create the shader version of the append function
  ShaderFunction* appendShaderFunction = outputStreamShaderType->FindOrCreateFunction("Append", appendZilchFunction);
  appendShaderFunction->mShaderReturnType = "void";
  appendShaderFunction->mShaderName = translator->MangleName("Append", outputStreamShaderType);

  String outputDataParamName = outputDataShaderType->mShaderName.ToLower();
  String thisKeyword = translator->mSettings->mNameSettings.mThisKeyword;
  // Write out the function signature
  ShaderCodeBuilder signatureBuilder;
  signatureBuilder.Write("(");
  signatureBuilder << outputDataShaderType->mShaderName << " " << outputDataParamName;
  signatureBuilder << ", int index, ";
  signatureBuilder << outputStreamShaderType->mShaderName << " " << thisKeyword;
  signatureBuilder.Write(")");
  appendShaderFunction->mShaderSignature = signatureBuilder.ToString();

  // Write out the function body
  ShaderCodeBuilder codeBuilder;
  codeBuilder.BeginScope();
  codeBuilder << codeBuilder.EmitIndent() << "WriteToVertex(index, " << outputDataParamName << ");" << codeBuilder.EmitLineReturn();
  codeBuilder << codeBuilder.EmitIndent() << "EmitVertex();" << codeBuilder.EmitLineReturn();
  codeBuilder.EndScope();
  appendShaderFunction->mShaderBodyCode = codeBuilder.ToString();
}

void ResolveGeometryOutputRestart(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();
  builder.Write("EndPrimitive()");
}

void RegisterGeometryOutputTypeAndFunctions(ZilchShaderTranslator* translator, Zilch::BoundType* boundType, Zilch::SyntaxNode* nodeLocation, String& resultTypeName, int count, StringParam primitiveType)
{
  ShaderType* outputStreamShaderType = CreateBasicTemplateType(translator, boundType, nodeLocation);

  // Set some meta information about this type
  ShaderTypeData* typeData = outputStreamShaderType->GetTypeData();
  typeData->mType = ShaderVarType::GeometryOutput;
  typeData->mCount = count;
  typeData->mExtraData = primitiveType;

  // The append function is special and needs to actually be generated instead of just replaced with a native intrinsic.
  GenerateOutputTypeAppend(translator, boundType, nodeLocation, outputStreamShaderType);
  // Register translation for the Restart function
  translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(boundType, "Restart"), ResolveGeometryOutputRestart);

  // Mark that we've registered this template's callbacks
  translator->mLibraryTranslator.mRegisteredTemplates.Insert(boundType->ToString());
}

void RegisterGeometryStreamMoverTypeAndFunctions(ZilchShaderTranslator* translator, Zilch::BoundType* boundType, Zilch::SyntaxNode* nodeLocation, String& resultTypeName)
{
  // Register that we've registered this template's callbacks
  translator->mLibraryTranslator.mRegisteredTemplates.Insert(boundType->ToString());

  ShaderType* shaderOutputType = CreateBasicTemplateType(translator, boundType, nodeLocation);
  // The move function isn't registered for translation because it should normally never be called.
  // The move function sits under the variable declaration node. In some languages this entire variable declaration needs to go away.
}

bool Glsl13TemplateTypeResolver(ZilchShaderTranslator* translator, Zilch::Type* type, Zilch::SyntaxNode* nodeLocation, String& resultTypeName)
{
  // @JoshD: Add this to the pre-walk phase so that I don't have worry about an order issue...
  Zilch::BoundType* boundType = Zilch::BoundType::GetBoundType(type);
  String templateTypeName = boundType->ToString();
  if(translator->mLibraryTranslator.mRegisteredTemplates.Contains(templateTypeName))
    return true;

  // If this is an array type then register all array functions and types
  if(boundType->TemplateBaseName == "FixedArray")
  {
    Zilch::BoundType* boundType = Zilch::BoundType::GetBoundType(type);
    RegisterFixedArrayTypeAndFunctions(translator, boundType, nodeLocation, resultTypeName);
    return true;
  }
  if(boundType->TemplateBaseName == "PointInput")
  {
    Zilch::BoundType* boundType = Zilch::BoundType::GetBoundType(type);
    RegisterGeometryInputTypeAndFunctions(translator, boundType, nodeLocation, resultTypeName, 1, "points");
    return true;
  }
  if(boundType->TemplateBaseName == "LineInput")
  {
    Zilch::BoundType* boundType = Zilch::BoundType::GetBoundType(type);
    RegisterGeometryInputTypeAndFunctions(translator, boundType, nodeLocation, resultTypeName, 2, "lines");
    return true;
  }
  if(boundType->TemplateBaseName == "TriangleInput")
  {
    Zilch::BoundType* boundType = Zilch::BoundType::GetBoundType(type);
    RegisterGeometryInputTypeAndFunctions(translator, boundType, nodeLocation, resultTypeName, 3, "triangles");
    return true;
  }
  if(boundType->TemplateBaseName == "PointOutput")
  {
    Zilch::BoundType* boundType = Zilch::BoundType::GetBoundType(type);
    RegisterGeometryOutputTypeAndFunctions(translator, boundType, nodeLocation, resultTypeName, 1, "points");
    return true;
  }

  if(boundType->TemplateBaseName == "LineOutput")
  {
    Zilch::BoundType* boundType = Zilch::BoundType::GetBoundType(type);
    RegisterGeometryOutputTypeAndFunctions(translator, boundType, nodeLocation, resultTypeName, 2, "line_strip");
    return true;
  }
  if(boundType->TemplateBaseName == "TriangleOutput")
  {
    Zilch::BoundType* boundType = Zilch::BoundType::GetBoundType(type);
    RegisterGeometryOutputTypeAndFunctions(translator, boundType, nodeLocation, resultTypeName, 3, "triangle_strip");
    return true;
  }
  
  if(boundType->TemplateBaseName == "GeometryStreamMover")
  {
    Zilch::BoundType* boundType = Zilch::BoundType::GetBoundType(type);
    RegisterGeometryStreamMoverTypeAndFunctions(translator, boundType, nodeLocation, resultTypeName);
    return true;
  }
  // Make arrays illegal to have
  if(boundType->TemplateBaseName == "Array")
  {
    if(nodeLocation != nullptr)
      translator->SendTranslationError(nodeLocation->Location, "Type 'Array' is not valid for use in shaders. Use 'FixedArray' instead.");
    return false;
  }

  // We didn't do anything special with this template type, translate as normal
  return false;
}

void ResolveZilchVector3Equality(ZilchShaderTranslator* translator, Zilch::BinaryOperatorNode* node, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();
  builder << "all(equal(";
  context->Walker->Walk(translator, node->LeftOperand, context);
  builder.Write(", ");
  context->Walker->Walk(translator, node->RightOperand, context);
  builder.Write("))");
}

void ResolveZilchVector3Inequality(ZilchShaderTranslator* translator, Zilch::BinaryOperatorNode* node, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();
  builder << "any(notEqual(";
  context->Walker->Walk(translator, node->LeftOperand, context);
  builder.Write(", ");
  context->Walker->Walk(translator, node->RightOperand, context);
  builder.Write("))");
}

// Small helper to build a vector of a certain size with one value being special (mainly to build up Real3.YAxis -> Real3(0, 1, 0))
String BuildVectorAxis(StringParam type, size_t dimension, size_t axis, StringParam axisValue, StringParam offAxisValue)
{
  StringBuilder builder;
  builder << type;
  builder << "(";
  for(size_t i = 0; i < dimension; ++i)
  {
    if(i == axis)
      builder << axisValue;
    else
      builder << offAxisValue;

    if(i != dimension - 1)
      builder << ", ";
  }
  builder << ")";
  return builder.ToString();
}

void ParseCoreLibrary(ZilchShaderTranslator* translator, ZilchShaderLibrary* coreLibrary)
{
  String refType = translator->mSettings->mNameSettings.mReferenceKeyword;
  ShaderType* unknownType = coreLibrary->CreateType("[Unknown]");
  unknownType->mShaderName = "[error]";

  coreLibrary->CreateNativeType("Void", "void", refType);
  coreLibrary->CreateNativeType("Real", "float", refType, ShaderVarType::Scalar);
  coreLibrary->CreateNativeType("Integer", "int", refType, ShaderVarType::Scalar);
  coreLibrary->CreateNativeType("Boolean", "bool", refType, ShaderVarType::Scalar);


  char* zilchVectorTypes[3] = {"Real%d", "Integer%d", "Boolean%d"};
  char* glslVectorTypes[3] = {"vec%d", "ivec%d", "bvec%d"};
  for(size_t typeIndex = 0; typeIndex < 3; ++typeIndex)
  {
    for(size_t dim = 2; dim <= 4; ++dim)
    {
      String zilchTypeStr = String::Format(zilchVectorTypes[typeIndex], dim);
      String glslTypeStr = String::Format(glslVectorTypes[typeIndex], dim);
      coreLibrary->CreateNativeType(zilchTypeStr, glslTypeStr, refType, ShaderVarType::Vector);
    }
  }

  char* zilchMatrixTypes[1] = {"Real%dx%d"};
  char* glslMatrixTypes[1] = {"mat%dx%d"};
  for(size_t type = 0; type < 1; ++type)
  {
    for(size_t y = 2; y <= 4; ++y)
    {
      for(size_t x = 2; x <= 4; ++x)
      {
        // Format the names of both the zilch and glsl types
        String zilchTypeStr = String::Format(zilchMatrixTypes[type], y, x);
        String glslTypeStr = String::Format(glslMatrixTypes[type], y, x);
        ShaderType* resultType = coreLibrary->CreateNativeType(zilchTypeStr, glslTypeStr, refType, ShaderVarType::Matrix);
        ShaderTypeData* typeData = resultType->GetTypeData();
        typeData->mSizeY = y;
        typeData->mSizeX = x;
      }
    }
  }

  // Create the quaternion type (just maps to a vector4 with special functions)
  {
    ShaderType* quatShaderType = coreLibrary->CreateNativeType("Quaternion", "vec4", refType, ShaderVarType::Vector);
    ShaderTypeData* typeData = quatShaderType->GetTypeData();
    typeData->mCount = 4;
  }

  coreLibrary->CreateNativeType("Math", "Math", refType);
}

void ParseShaderLibrary(ZilchShaderTranslator* translator, ZilchShaderLibrary* shaderIntrinsicLibrary)
{
  shaderIntrinsicLibrary->CreateNativeType("Shader", "Shader");
}

void Glsl13NativeLibraryParser(ZilchShaderTranslator* translator, ZilchShaderLibrary* shaderLibrary)
{
  if(shaderLibrary->mZilchLibrary->Name == "Core")
    ParseCoreLibrary(translator, shaderLibrary);
  else if(shaderLibrary->mZilchLibrary->Name == "ShaderIntrinsicsLibrary")
    ParseShaderLibrary(translator, shaderLibrary);
  else
  {
    Error("Unknown native library");
  }
}

void SetupGlsl_1_3(ZilchShaderTranslator* translator)
{
  SetupGenericLibraryTranslation(translator);

  Zilch::Core& core = Zilch::Core::GetInstance();
  Zilch::LibraryRef coreLibrary = core.GetLibrary();
  LibraryTranslator& libraryTranslator = translator->mLibraryTranslator;
  libraryTranslator.RegisterNativeLibraryParser(Glsl13NativeLibraryParser);

  // Set the template type resolver (to detect and register arrays)
  libraryTranslator.RegisterTemplateTypeResolver(Glsl13TemplateTypeResolver);

  // Store what the default value is for the basic types in zilch (needed later for vector types as well)
  char* glslDefaultValues[3] = {"0.0", "0", "false"};

  // Make a mapping of zilch types to glsl types
  char* zilchBasicTypes[3] = {"Real", "Integer", "Boolean"};
  char* glslBasicTypes[3] = {"float", "int", "bool"};
  // Add translation mappings for all basic zilch->glsl types
  for(size_t type = 0; type < 3; ++type)
  {
    // Look up the zilch type and register its replacement type
    Zilch::BoundType* zilchBoundType = coreLibrary->BoundTypes.FindValue(zilchBasicTypes[type], nullptr);

    // Find the default constructor and register its default value as (value)]
    // (it's in parenthesis because it'll always be part of a function call such as float(0.0))
    const Zilch::FunctionArray* constructors = zilchBoundType->GetOverloadedInheritedConstructors();
    if(constructors != nullptr && constructors->Empty() == false)
    {
      Zilch::Function* defaultConstructor = Zilch::BoundType::GetDefaultConstructor(constructors);
      String parameters = glslDefaultValues[type];
      String defaultValue = BuildString(glslBasicTypes[type], "(", parameters, ")");
      libraryTranslator.RegisterFunctionCallResolver(defaultConstructor, defaultValue);
    }

    // Register a callback for all other constructors (simply assume we can translate all of the arguments.
    // This is mostly to translate things such as Real(1.0) which mostly show up with generate code across lots of types (for a splat).
    libraryTranslator.RegisterBackupConstructorResolver(zilchBoundType, ResolveSimpleBackupConstructor);

    libraryTranslator.RegisterMemberAccessBackupResolver(zilchBoundType, ResolveBackupScalarSwizzle);
  }

  // Now register the vector type replacements
  char* zilchVectorTypes[3] = {"Real%d", "Integer%d", "Boolean%d"};
  char* glslVectorTypes[3] = {"vec%d", "ivec%d", "bvec%d"};
  char* axesNames[4] = {"XAxis", "YAxis", "ZAxis", "WAxis"};
  for(size_t type = 0; type < 3; ++type)
  {
    // Assume for now that all vector types are of dimension 2 -> 4
    for(size_t dimension = 2; dimension <= 4; ++dimension)
    {
      // Build up the names of both the zilch and glsl types
      String zilchType = String::Format(zilchVectorTypes[type], dimension);
      String glslType = String::Format(glslVectorTypes[type], dimension);

      // Find the zilch type and register the replacement
      Zilch::BoundType* zilchBoundType = coreLibrary->BoundTypes.FindValue(zilchType, nullptr);

      // Find the default constructor of the type and register its default value
      const Zilch::FunctionArray* constructors = zilchBoundType->GetOverloadedInheritedConstructors();
      if(constructors != nullptr && constructors->Empty() == false)
      {
        Zilch::Function* defaultConstructor = Zilch::BoundType::GetDefaultConstructor(constructors);
        String parameters = JoinRepeatedString(glslDefaultValues[type], ", ", dimension);
        String defaultValue = BuildString(glslType, "(", parameters, ")");
        libraryTranslator.RegisterFunctionCallResolver(defaultConstructor, defaultValue);
      }

      // For now assume that any other constructor can translate as normal, this is a bit of a dangerous
      // thing to do as any random construction that is added to zilch could fail to translate, however it's
      // a bit more complicated to look up every possible constructor to translate right now (such as Real4(float),
      // Real4(float, float, float, float), Real4(Real2, Real2), Real4(Real, Real3), etc...)
      libraryTranslator.RegisterBackupConstructorResolver(zilchBoundType, ResolveSimpleBackupConstructor);

      libraryTranslator.RegisterBinaryOpResolver(zilchBoundType, Zilch::Grammar::Equality, zilchBoundType, ResolveZilchVector3Equality);
      libraryTranslator.RegisterBinaryOpResolver(zilchBoundType, Zilch::Grammar::Inequality, zilchBoundType, ResolveZilchVector3Inequality);
      libraryTranslator.RegisterBinaryOpResolver(zilchBoundType, Zilch::Grammar::LessThan, zilchBoundType, "lessThan");
      libraryTranslator.RegisterBinaryOpResolver(zilchBoundType, Zilch::Grammar::LessThanOrEqualTo, zilchBoundType, "lessThanEqual");
      libraryTranslator.RegisterBinaryOpResolver(zilchBoundType, Zilch::Grammar::GreaterThan, zilchBoundType, "greaterThan");
      libraryTranslator.RegisterBinaryOpResolver(zilchBoundType, Zilch::Grammar::GreaterThanOrEqualTo, zilchBoundType, "greaterThanEqual");

      // Bind all of the axis properties (Real3.XAxis, etc...)
      for(size_t axis = 0; axis < dimension; ++axis)
      {
        String vectorValue = BuildVectorAxis(glslType, dimension, axis, "1", "0");
        
        libraryTranslator.RegisterMemberAccessResolver(GetStaticProperty(zilchBoundType, axesNames[axis]), vectorValue);
      }
      // Also bind the Zero property (Real3.Zero)
      String zeroVectorValue = BuildVectorAxis(glslType, dimension, 0, "0", "0");
      libraryTranslator.RegisterMemberAccessResolver(GetStaticProperty(zilchBoundType, "Zero"), zeroVectorValue);
    }
  }

  {
    // Find the zilch type and register the replacement
    Zilch::BoundType* zilchQuatBoundType = coreLibrary->BoundTypes.FindValue("Quaternion", nullptr);

    String quatIdentity = "vec4(0.0, 0.0, 0.0, 1.0)";
    // Find the default constructor of the type and register its default value
    const Zilch::FunctionArray* constructors = zilchQuatBoundType->GetOverloadedInheritedConstructors();
    if(constructors != nullptr && constructors->Empty() == false)
    {
      Zilch::Function* defaultConstructor = Zilch::BoundType::GetDefaultConstructor(constructors);
      libraryTranslator.RegisterFunctionCallResolver(defaultConstructor, quatIdentity);
    }

    // Register a callback for all other constructors (simply assume we can translate all of the arguments.
    // This is mostly to translate things such as Real(1.0) which mostly show up with generate code across lots of types (for a splat).
    libraryTranslator.RegisterBackupConstructorResolver(zilchQuatBoundType, ResolveSimpleBackupConstructor);
    // Register the identity property (Extension/implements doesn't work on properties yet)
    libraryTranslator.RegisterMemberAccessResolver(GetStaticProperty(zilchQuatBoundType, "Identity"), quatIdentity);
  }

  // Now translate the matrix types
  char* zilchMatrixTypes[1] = {"Real%dx%d"};
  char* glslMatrixTypes[1] = {"mat%dx%d"};
  for(size_t type = 0; type < 1; ++type)
  {
    for(size_t y = 2; y <= 4; ++y)
    {
      for(size_t x = 2; x <= 4; ++x)
      {
        // Format the names of both the zilch and glsl types
        // glsl is column by row (x by y) which is opposite zilch and hlsl
        String zilchType = String::Format(zilchMatrixTypes[type], y, x);
        String glslType = String::Format(glslMatrixTypes[type], y, x);

        // Lookup the zilch type
        Zilch::BoundType* zilchBoundType = coreLibrary->BoundTypes.FindValue(zilchType, nullptr);

        String transposedZilchMatrixTypeStr = String::Format(zilchMatrixTypes[type], x, y);
        libraryTranslator.RegisterFunctionCallResolver(GetStaticFunction(core.MathType, "Multiply", zilchType, transposedZilchMatrixTypeStr), ResolveMatrixMultiply);

        String zilchVectorTypeStr = String::Format(zilchVectorTypes[type], x);
        libraryTranslator.RegisterFunctionCallResolver(GetStaticFunction(core.MathType, "Multiply", zilchType, zilchVectorTypeStr), ResolveMatrixMultiply);

        // Find and register the default constructors
        const Zilch::FunctionArray* constructors = zilchBoundType->GetOverloadedInheritedConstructors();
        if(constructors != nullptr && constructors->Empty() == false)
        {
          Zilch::Function* defaultConstructor = Zilch::BoundType::GetDefaultConstructor(constructors);
          String parameters = JoinRepeatedString(glslDefaultValues[type], ", ", x * y);
          String defaultValue = BuildString(glslType, "(", parameters, ")");
          libraryTranslator.RegisterFunctionCallResolver(defaultConstructor, defaultValue);
        }

        // For now assume that any other constructor can translate as normal, this is a bit of a dangerous
        // thing to do as any random construction that is added to zilch could fail to translate, however it's
        // a bit more complicated to look up every possible constructor to translate right now (such as Real2x2(float),
        // Real2x2(float, float, float, float), Real2x2(Real2, Real2), Real2x2(Real, Real3), etc...)
        libraryTranslator.RegisterBackupConstructorResolver(zilchBoundType, ResolveSimpleBackupConstructor);
      }
    }
  }


  Array<Zilch::BoundType*>& allRealTypes = core.AllRealTypes;
  Array<Zilch::BoundType*>& allIntegerTypes = core.AllIntegerTypes;
  Array<Zilch::BoundType*>& allBooleanTypes = core.AllBooleanTypes;
  Array<Zilch::BoundType*> allIntegerAndRealTypes;
  allIntegerAndRealTypes.Insert(allIntegerAndRealTypes.End(), allRealTypes.All());
  allIntegerAndRealTypes.Insert(allIntegerAndRealTypes.End(), allIntegerTypes.All());
  Array<Zilch::BoundType*> allPrimitiveTypes;
  allPrimitiveTypes.Insert(allPrimitiveTypes.End(), allRealTypes.All());
  allPrimitiveTypes.Insert(allPrimitiveTypes.End(), allIntegerTypes.All());
  allPrimitiveTypes.Insert(allPrimitiveTypes.End(), allBooleanTypes.All());

  // Bind the splatted math functions for all types
  for(size_t i = 0; i < allPrimitiveTypes.Size(); ++i)
  {
    Zilch::BoundType* boundType = allPrimitiveTypes[i];
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "AllNonZero", boundType->ToString()), "all");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "AnyNonZero", boundType->ToString()), "any");
  }

  // Bind the splatted math functions shared for real and integer
  for(size_t i = 0; i < allIntegerAndRealTypes.Size(); ++i)
  {
    Zilch::BoundType* boundType = allIntegerAndRealTypes[i];
    String boundTypeStr = boundType->ToString();
    String realTypeStr = core.RealType->ToString();

    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Abs", boundType->ToString()), "abs");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Clamp", boundTypeStr, boundTypeStr, boundTypeStr), "clamp");
    // Glsl 4.0
    //libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "CountBits", boundTypeStr), "bitCount");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Max", boundTypeStr, boundTypeStr), "max");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Min", boundTypeStr, boundTypeStr), "min");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Sign", boundTypeStr), "sign");
  }

  // Find the rest of the splatted math functions on real (the commented out ones don't exist in glsl, eventually
  // these should generate functions that perform operation from whatever primitives are available (if possible))
  for(size_t i = 0; i < allRealTypes.Size(); ++i)
  {
    Zilch::BoundType* boundType = allRealTypes[i];
    String boundTypeStr = boundType->ToString();
    String realTypeStr = core.RealType->ToString();
    String integerTypeStr = allIntegerTypes[i]->ToString();

    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "ACos", boundType->ToString()), "acos");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "ASin", boundTypeStr), "asin");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "ATan", boundTypeStr), "atan");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "ATan2", boundTypeStr, boundTypeStr), "atan");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Ceil", boundTypeStr), "ceil");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Cos", boundTypeStr), "cos");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Cosh", boundTypeStr), "cosh");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Exp", boundTypeStr), "exp");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Exp2", boundTypeStr), "exp2");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Floor", boundTypeStr), "floor");
    //libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "FMod", boundTypeStr), "fmod");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Frac", boundTypeStr), "fract");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Lerp", boundTypeStr, boundTypeStr, realTypeStr), "mix");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Lerp", boundTypeStr, boundTypeStr, boundTypeStr), "mix");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Log", boundTypeStr), "log");
    //libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Log10", boundTypeStr), "mix");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Log2", boundTypeStr), "log2");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Pow", boundTypeStr, boundTypeStr), "pow");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "ReflectAcrossPlane", boundTypeStr, boundTypeStr), "reflect");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Refract", boundTypeStr, boundTypeStr, realTypeStr), "refract");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Round", boundTypeStr), "round");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "RSqrt", boundTypeStr), "inversesqrt");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Sin", boundTypeStr), "sin");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Sinh", boundTypeStr), "sinh");
    //libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "SmoothStep", boundTypeStr, boundTypeStr, realTypeStr), "smoothstep");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "SmoothStep", boundTypeStr, boundTypeStr, boundTypeStr), "smoothstep");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Sqrt", boundTypeStr), "sqrt");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Step", boundTypeStr, boundTypeStr), "step");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Tan", boundTypeStr), "tan");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Tanh", boundTypeStr), "tanh");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "ToDegrees", boundTypeStr), "degrees");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "ToRadians", boundTypeStr), "radians");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Truncate", boundTypeStr), "trunc");
    //libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "ApproximatelyEqual", boundTypeStr, boundTypeStr), "approxEqual");
  }


  // Stuff for shader values (samplers, etc...)
  Zilch::Library* shaderLibrary = Zilch::ShaderIntrinsicsLibrary::GetInstance().GetLibrary();
  Zilch::BoundType* shaderType = ZilchTypeId(Zilch::Shader);
}

}//namespace Zero
