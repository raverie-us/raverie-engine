///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "HlslTranslator.hpp"
#include "ShaderLibrary.hpp"
#include "GenericTranslation.hpp"
#include "LibraryTranslationHelpers.hpp"
#include "ShaderIntrinsicTypes.hpp"

namespace Zero
{

void ResolveVectorSplatConstructor(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::StaticTypeNode* staticTypeNode, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();

  builder << "(";
  context->Walker->Walk(translator, staticTypeNode, context);
  builder << ")";
  builder << "(";
  context->Walker->Walk(translator, fnCallNode->Arguments[0], context);
  builder << ")";
}

void ResolveHlslBackupScalarSwizzle(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  String data = memberAccessNode->Name;

  bool invalid = false;
  // We only allow swizzles up to a vector 4 (so anything longer is an error)
  if(data.size() > 4)
    invalid = true;
  // Make sure that every value is X (nothing else is allowed)
  for(size_t i = 0; i < data.size(); ++i)
  {
    if(data[i] != 'X')
      invalid = true;
  }

  if(!invalid)
  {
    ShaderType* resultShaderType = translator->FindShaderType(memberAccessNode->ResultType, memberAccessNode);

    context->GetBuilder() << "(" << resultShaderType->mShaderName << ")" << "(";
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

void ResolveHlsl11FixedArrayDefaultConstructor(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::StaticTypeNode* staticTypeNode, ZilchShaderTranslatorContext* context)
{
  // just consume the default constructor for an array
}

void ResolveHlsl11FixedArrayGet(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();
  // Change array.Get(index) into array[index]
  context->Walker->Walk(translator, memberAccessNode->LeftOperand, context);
  builder.Write("[");
  context->Walker->Walk(translator, fnCallNode->Arguments[0], context);
  builder.Write("]");
}

void ResolveHlsl11FixedArraySet(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
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

void ResolveHlsl11FixedArrayAdd(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  // Report an error if the user tries to call the add function. This function has to exist for initializer lists so we can't remove it.
  // We don't actually visit the node when constructing the initializer list so it should be a problem to always make this an error to call.
  translator->SendTranslationError(fnCallNode->Location, "FixedArray's Add function cannot be translated as shaders only support static arrays. The Add function currently exists for initializer lists.");
}

void ResolveHlsl11FixedArrayCount(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  // Change array.Count into array.length()
  context->Walker->Walk(translator, memberAccessNode->LeftOperand, context);
  ShaderCodeBuilder& builder = context->GetBuilder();
  builder.Write(".length()");
}

void Hlsl11FixedArrayInitializerResolver(ZilchShaderTranslator* translator, Zilch::Type* type, Zilch::ExpressionInitializerNode* initializerNode, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();

  Zilch::BoundType* arrayType = Zilch::BoundType::GetBoundType(type);
  // Get the type the array is templated on as well as the size of the fixed array
  Zilch::Type* templateType = arrayType->TemplateArguments[0].TypeValue;
  size_t fixedSize = (size_t)arrayType->TemplateArguments[1].IntegerValue;

  // If there's no initializer expression or it's just the default constructor
  // (meaning there's no initializer statements) then don't try to build up the array initializer.
  size_t statementCount = initializerNode->InitializerStatements.size();
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
  for(size_t i = 0; i < initializerNode->InitializerStatements.size(); ++i)
  {
    Zilch::ExpressionNode* expNode = initializerNode->InitializerStatements[i];

    // Array initializers are just semantic sugar and they actually translate to a whole bunch of array.Add(value) calls.
    // Try to get the function call node for Zilch::OperatorInsert.
    // If this isn't a function call node then we have a problem (throw an error).
    Zilch::FunctionCallNode* addCallNode = Zilch::TypeBinding::DynamicCast<Zilch::FunctionCallNode*>(expNode);
    if(addCallNode == nullptr)
    {
      String errMsg = "It's invalid to have a member initializer statement in an array initializer";
      translator->SendTranslationError(expNode->Location, errMsg);
      return;
    }

    // Now that we know it's a function call node, make sure that the function being called 
    // is the OperatorInsert function ("Add"), otherwise throw another error.
    Zilch::MemberAccessNode* memberAccessNode = Zilch::TypeBinding::DynamicCast<Zilch::MemberAccessNode*>(addCallNode->LeftOperand);
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
    if(i != initializerNode->AddValues.size() - 1)
      initializerListBuilder << ", ";
  }

  // Now write out the initialization. For FixedArray[Real, 2] this will be float[2].
  ShaderType* arrayShaderType = translator->FindShaderType(type, initializerNode);
  builder << arrayShaderType->mShaderName;

  // Now if we built up an initializer expression then write it out. Glsl uses parenthesis for initialization
  // i.e. Array[Real, 2]{1.0, 2.0} -> float[2](1.0, 2.0)
  String initializerExp = initializerListBuilder.ToString();
  if(!initializerExp.empty())
    builder << "{" << initializerExp << "}";
}

void RegisterHlsl11FixedArrayTypeAndFunctions(ZilchShaderTranslator* translator, Zilch::BoundType* arrayType, Zilch::SyntaxNode* nodeLocation, String& resultTypeName)
{
  // Mark that we've registered this template's callbacks
  translator->mLibraryTranslator.mRegisteredTemplates.insert(arrayType->ToString());

  String refKeyword = translator->mSettings.mNameSettings.mReferenceKeyword;

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
  // Mark which library owns this array
  fixedArrayShaderType->mOwningLibrary = translator->mCurrentLibrary;
  // Add a dependency to the template argument
  fixedArrayShaderType->AddDependency(fixedArrayTemplateType);
  // Also register the initializer node resolver (so we can translate initializer statements)
  translator->mLibraryTranslator.RegisterInitializerNodeResolver(arrayType, Hlsl11FixedArrayInitializerResolver);

  // Register a callback for the default constructor on this type
  // (mainly to just consume the array's default constructor which we don't translate)
  Zilch::Function* defaultConstructor = Zilch::BoundType::GetDefaultConstructor(arrayType->GetOverloadedInheritedConstructors());
  if(defaultConstructor != nullptr)
    translator->mLibraryTranslator.RegisterFunctionCallResolver(defaultConstructor, ResolveHlsl11FixedArrayDefaultConstructor);

  // Also register any array functions we need to translate
  translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(arrayType, Zilch::OperatorGet, "Integer"), ResolveHlsl11FixedArrayGet);
  translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(arrayType, Zilch::OperatorSet, "Integer", templateTypeStr), ResolveHlsl11FixedArraySet);
  translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(arrayType, Zilch::OperatorInsert, templateTypeStr), ResolveHlsl11FixedArrayAdd);
  translator->mLibraryTranslator.RegisterMemberAccessResolver(GetInstanceProperty(arrayType, "Count"), ResolveHlsl11FixedArrayCount);
}

void HlslParseCoreLibrary(ZilchShaderTranslator* translator, ZilchShaderLibrary* coreLibrary)
{
  String refType = translator->mSettings.mNameSettings.mReferenceKeyword;
  ShaderType* unknownType = coreLibrary->CreateType("[Unknown]");
  unknownType->mShaderName = "[error]";

  coreLibrary->CreateNativeType("Void", "void", refType);
  coreLibrary->CreateNativeType("Real", "float", refType, ShaderVarType::Scalar);
  coreLibrary->CreateNativeType("Integer", "int", refType, ShaderVarType::Scalar);
  coreLibrary->CreateNativeType("Boolean", "bool", refType, ShaderVarType::Scalar);


  char* zilchVectorTypes[3] = {"Real%d", "Integer%d", "Boolean%d"};
  char* glslVectorTypes[3] = {"float%d", "int%d", "bool%d"};
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
  char* glslMatrixTypes[1] = {"float%dx%d"};
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

  coreLibrary->CreateNativeType("Math", "Math", refType);
}

void HlslParseShaderLibrary(ZilchShaderTranslator* translator, ZilchShaderLibrary* shaderIntrinsicLibrary)
{
  shaderIntrinsicLibrary->CreateNativeType("Shader", "Shader");
}

void Hlsl11NativeLibraryParser(ZilchShaderTranslator* translator, ZilchShaderLibrary* shaderLibrary)
{
  if(shaderLibrary->mZilchLibrary->Name == "Core")
    HlslParseCoreLibrary(translator, shaderLibrary);
  else if(shaderLibrary->mZilchLibrary->Name == "ShaderIntrinsicsLibrary")
    HlslParseShaderLibrary(translator, shaderLibrary);
  else
  {
    Error("Unknown native library");
  }
}

bool Hlsl11TemplateTypeResolver(ZilchShaderTranslator* translator, Zilch::Type* type, Zilch::SyntaxNode* nodeLocation, String& resultTypeName)
{
  // @JoshD: Add this to the pre-walk phase so that I don't have worry about an order issue...
  Zilch::BoundType* boundType = Zilch::BoundType::GetBoundType(type);
  String templateTypeName = boundType->ToString();
  if(translator->mLibraryTranslator.mRegisteredTemplates.contains(templateTypeName))
    return true;

  // If this is an array type then register all array functions and types
  if(boundType->TemplateBaseName == "FixedArray")
  {
    Zilch::BoundType* boundType = Zilch::BoundType::GetBoundType(type);
    RegisterHlsl11FixedArrayTypeAndFunctions(translator, boundType, nodeLocation, resultTypeName);
    return true;
  }
  // Make arrays illegal to have
  if(boundType->TemplateBaseName == "Array")
  {
    translator->SendTranslationError(nodeLocation->Location, "Type 'Array' is not valid for use in shaders. Use 'FixedArray' instead.");
    return false;
  }

  // We didn't do anything special with this template type, translate as normal
  return false;
}

void SetupHlsl_11(ZilchShaderTranslator* translator)
{
  SetupGenericLibraryTranslation(translator);

  Zilch::Core& core = Zilch::Core::GetInstance();
  Zilch::LibraryRef coreLibrary = core.GetLibrary();
  LibraryTranslator& libraryTranslator = translator->mLibraryTranslator;
  libraryTranslator.RegisterNativeLibraryParser(Hlsl11NativeLibraryParser);

  // Set the template type resolver (to detect and register arrays)
  libraryTranslator.RegisterTemplateTypeResolver(Hlsl11TemplateTypeResolver);

  // Store what the default value is for the basic types in zilch (needed later for vector types as well)
  char* glslDefaultValues[3] = {"0.0", "0", "false"};

  // Make a mapping of zilch types to glsl types
  char* zilchBasicTypes[3] = {"Real", "Integer", "Boolean"};
  char* glslBasicTypes[3] = {"float", "int", "bool"};
  // Add translation mappings for all basic zilch->glsl types
  for(size_t type = 0; type < 3; ++type)
  {
    // Look up the zilch type and register its replacement type
    Zilch::BoundType* zilchBoundType = coreLibrary->BoundTypes.findValue(zilchBasicTypes[type], nullptr);

    // Find the default constructor and register its default value as (value)]
    // (it's in parenthesis because it'll always be part of a function call such as float(0.0))
    const Zilch::FunctionArray* constructors = zilchBoundType->GetOverloadedInheritedConstructors();
    if(constructors != nullptr && constructors->empty() == false)
    {
      Zilch::Function* defaultConstructor = Zilch::BoundType::GetDefaultConstructor(constructors);
      String parameters = glslDefaultValues[type];
      String defaultValue = BuildString(glslBasicTypes[type], "(", parameters, ")");
      libraryTranslator.RegisterFunctionCallResolver(defaultConstructor, defaultValue);
    }

    // Register a callback for all other constructors (simply assume we can translate all of the arguments.
    // This is mostly to translate things such as Real(1.0) which mostly show up with generate code across lots of types (for a splat).
    libraryTranslator.RegisterBackupConstructorResolver(zilchBoundType, ResolveSimpleBackupConstructor);

    libraryTranslator.RegisterMemberAccessBackupResolver(zilchBoundType, ResolveHlslBackupScalarSwizzle);
  }

  // Now register the vector type replacements
  char* zilchVectorTypes[3] = {"Real%d", "Integer%d", "Boolean%d"};
  char* glslVectorTypes[3] = {"float%d", "int%d", "bool%d"};
  char* axesNames[4] = {"XAxis", "YAxis", "ZAxis", "WAxis"};
  for(size_t type = 0; type < 3; ++type)
  {
    // Assume for now that all vector types are of dimension 2 -> 4
    for(size_t dimension = 2; dimension <= 4; ++dimension)
    {
      // Build up the names of both the zilch and hlsl types
      String zilchType = String::Format(zilchVectorTypes[type], dimension);
      String hlslType = String::Format(glslVectorTypes[type], dimension);

      // Find the zilch type and register the replacement
      Zilch::BoundType* zilchBoundType = coreLibrary->BoundTypes.findValue(zilchType, nullptr);

      // Find the default constructor of the type and register its default value
      const Zilch::FunctionArray* constructors = zilchBoundType->GetOverloadedInheritedConstructors();
      if(constructors != nullptr && constructors->empty() == false)
      {
        Zilch::Function* defaultConstructor = Zilch::BoundType::GetDefaultConstructor(constructors);
        String parameters = JoinRepeatedString(glslDefaultValues[type], ", ", dimension);
        String defaultValue = BuildString(hlslType, "(", parameters, ")");
        libraryTranslator.RegisterFunctionCallResolver(defaultConstructor, defaultValue);
      }

      Array<String> params;
      params.push_back(zilchBasicTypes[type]);
      Zilch::Function* splatConstructor = GetFunction(zilchBoundType, String(), params, constructors);
      libraryTranslator.RegisterFunctionCallResolver(splatConstructor, ResolveVectorSplatConstructor);

      // For now assume that any other constructor can translate as normal, this is a bit of a dangerous
      // thing to do as any random construction that is added to zilch could fail to translate, however it's
      // a bit more complicated to look up every possible constructor to translate right now (such as Real4(float),
      // Real4(float, float, float, float), Real4(Real2, Real2), Real4(Real, Real3), etc...)
      libraryTranslator.RegisterBackupConstructorResolver(zilchBoundType, ResolveSimpleBackupConstructor);

      //libraryTranslator.RegisterBinaryOpResolver(zilchBoundType, Zilch::Grammar::Equality, zilchBoundType, ResolveZilchVector3Equality);
      //libraryTranslator.RegisterBinaryOpResolver(zilchBoundType, Zilch::Grammar::Inequality, zilchBoundType, ResolveZilchVector3Inequality);
      //libraryTranslator.RegisterBinaryOpResolver(zilchBoundType, Zilch::Grammar::LessThan, zilchBoundType, "lessThan");
      //libraryTranslator.RegisterBinaryOpResolver(zilchBoundType, Zilch::Grammar::LessThanOrEqualTo, zilchBoundType, "lessThanEqual");
      //libraryTranslator.RegisterBinaryOpResolver(zilchBoundType, Zilch::Grammar::GreaterThan, zilchBoundType, "greaterThan");
      //libraryTranslator.RegisterBinaryOpResolver(zilchBoundType, Zilch::Grammar::GreaterThanOrEqualTo, zilchBoundType, "greaterThanEqual");

      // Bind all of the axis properties (Real3.XAxis, etc...)
      for(size_t axis = 0; axis < dimension; ++axis)
      {
        String vectorValue = BuildVectorAxis(hlslType, dimension, axis, "1", "0");

        libraryTranslator.RegisterMemberAccessResolver(GetStaticProperty(zilchBoundType, axesNames[axis]), vectorValue);
      }
      // Also bind the Zero property (Real3.Zero)
      String zeroVectorValue = BuildVectorAxis(hlslType, dimension, 0, "0", "0");
      libraryTranslator.RegisterMemberAccessResolver(GetStaticProperty(zilchBoundType, "Zero"), zeroVectorValue);
    }
  }

  // Now translate the matrix types
  char* zilchMatrixTypes[1] = {"Real%dx%d"};
  char* glslMatrixTypes[1] = {"float%dx%d"};
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
        Zilch::BoundType* zilchBoundType = coreLibrary->BoundTypes.findValue(zilchType, nullptr);

        String transposedZilchMatrixTypeStr = String::Format(zilchMatrixTypes[type], x, y);
        libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Multiply", zilchType, transposedZilchMatrixTypeStr), "mul");

        String zilchVectorTypeStr = String::Format(zilchVectorTypes[type], x);
        libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Multiply", zilchType, zilchVectorTypeStr), "mul");

        // Find and register the default constructors
        const Zilch::FunctionArray* constructors = zilchBoundType->GetOverloadedInheritedConstructors();
        if(constructors != nullptr && constructors->empty() == false)
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
  allIntegerAndRealTypes.insert(allIntegerAndRealTypes.end(), allRealTypes.all());
  allIntegerAndRealTypes.insert(allIntegerAndRealTypes.end(), allIntegerTypes.all());
  Array<Zilch::BoundType*> allPrimitiveTypes;
  allPrimitiveTypes.insert(allPrimitiveTypes.end(), allRealTypes.all());
  allPrimitiveTypes.insert(allPrimitiveTypes.end(), allIntegerTypes.all());
  allPrimitiveTypes.insert(allPrimitiveTypes.end(), allBooleanTypes.all());

  // Bind the splatted math functions for all types
  for(size_t i = 0; i < allPrimitiveTypes.size(); ++i)
  {
    Zilch::BoundType* boundType = allPrimitiveTypes[i];
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "AllNonZero", boundType->ToString()), "all");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "AnyNonZero", boundType->ToString()), "any");
  }

  // Bind the splatted math functions shared for real and integer
  for(size_t i = 0; i < allIntegerAndRealTypes.size(); ++i)
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
  for(size_t i = 0; i < allRealTypes.size(); ++i)
  {
    Zilch::BoundType* boundType = allRealTypes[i];
    String boundTypeStr = boundType->ToString();
    String realTypeStr = core.RealType->ToString();
    String integerTypeStr = allIntegerTypes[i]->ToString();

    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "ACos", boundType->ToString()), "acos");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "ASin", boundTypeStr), "asin");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "ATan", boundTypeStr), "atan");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "ATan2", boundTypeStr, boundTypeStr), "atan2");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Ceil", boundTypeStr), "ceil");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Cos", boundTypeStr), "cos");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Cosh", boundTypeStr), "cosh");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Exp", boundTypeStr), "exp");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Exp2", boundTypeStr), "exp2");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Floor", boundTypeStr), "floor");
    //libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "FMod", boundTypeStr), "fmod");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Frac", boundTypeStr), "frac");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Lerp", boundTypeStr, boundTypeStr, realTypeStr), "lerp");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Lerp", boundTypeStr, boundTypeStr, boundTypeStr), "lerp");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Log", boundTypeStr), "log");
    //libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Log10", boundTypeStr), "mix");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Log2", boundTypeStr), "log2");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Pow", boundTypeStr, boundTypeStr), "pow");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "ReflectAcrossPlane", boundTypeStr, boundTypeStr), "reflect");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Refract", boundTypeStr, boundTypeStr, realTypeStr), "refract");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Round", boundTypeStr), "round");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "RSqrt", boundTypeStr), "rsqrt");
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
  libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(shaderType, "Ddx", core.Real2Type->ToString()), "ddx");
  libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(shaderType, "Ddy", core.Real2Type->ToString()), "ddy");
  libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(shaderType, "Ddx", core.Real3Type->ToString()), "ddx");
  libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(shaderType, "Ddy", core.Real3Type->ToString()), "ddy");
}

//-------------------------------------------------------------------BaseHlslTranslator
String BaseHlslTranslator::mLanguageName = "hlsl";

void BaseHlslTranslator::SetupShaderLanguage()
{
  mLibraryTranslator.Reset();
  SetupHlsl_11(this);
}

String BaseHlslTranslator::GetLanguageName()
{
  return mLanguageName;
}

void BaseHlslTranslator::WriteGeometryOutputVariableDeclaration(Zilch::LocalVariableNode*& node, ShaderType* variableShaderType, ZilchShaderTranslatorContext* context)
{
  //// For glsl, simply construct the to-type variable
  //Zilch::FunctionCallNode* fnCallNode = Zilch::TypeBinding::DynamicCast<Zilch::FunctionCallNode*>(node->InitialValue);
  //String toName = ApplyVariableReplacement(node->Name.Token);
  //ShaderType* toType = variableShaderType;
  //// Find the default constructor so we can call it
  //ShaderFunction* defaultConstructor = toType->FindFunction(ZilchShaderSettings::GetDefaultConstructorKey());
  //
  //// Write: varType varName = varType();
  //ShaderCodeBuilder& builder = context->GetBuilder();
  //builder << toType->mShaderName << " " << toName << " = " << defaultConstructor->mShaderName << "()";
}

void BaseHlslTranslator::WriteMainForClass(Zilch::SyntaxNode* node, ShaderType* currentType, ShaderFunction* function, ZilchShaderTranslatorContext* context)
{

}

//-------------------------------------------------------------------Hlsl11Translator
String Hlsl11Translator::GetFullLanguageString()
{
  return "hlsl11";
}

int Hlsl11Translator::GetLanguageVersionNumber()
{
  return 11;
}

String Hlsl11Translator::GetVersionString()
{
  return "";
}

bool Hlsl11Translator::SupportsFragmentType(ShaderType* shaderType)
{
  return true;
}

}//namespace Zero
