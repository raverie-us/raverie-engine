///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

#define DeclareSimpleMemberAccessReplacementWithString(FunctionName, ReturnValue) \
  void FunctionName(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* node, ZilchShaderTranslatorContext* context) \
  { \
    context->GetBuilder() << ReturnValue;  \
  }

#define DeclareSimpleFunctionCallReplacementWithString(FunctionName, ReturnValue) \
  void FunctionName(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context) \
  { \
    context->GetBuilder() << ReturnValue;  \
  }

DeclareSimpleMemberAccessReplacementWithString(ResolveMathPi, "3.141592653589793238462643383f");
DeclareSimpleMemberAccessReplacementWithString(ResolveMathE, "2.7182818284590452353602874713526f");
DeclareSimpleMemberAccessReplacementWithString(ResolveReal3Count, "3");

void ParseStringExpression(ZilchShaderTranslator* translator, Zilch::ExpressionNode* expressionNode, ZilchShaderTranslatorContext* context)
{
  // If this is a string interpolation node then specially translate it.
  Zilch::StringInterpolantNode* stringInterpolantNode = Zilch::Type::DynamicCast<Zilch::StringInterpolantNode*>(expressionNode);
  if(stringInterpolantNode != nullptr)
  {
    // Make a new coped builder to collect the string interpolation data
    ShaderCodeBuilder& builder = context->GetBuilder();
    for(size_t i = 0; i < stringInterpolantNode->Elements.Size(); ++i)
    {
      // Translate string literals specially (to string quotes)
      Zilch::ValueNode* valueNode = Zilch::Type::DynamicCast<Zilch::ValueNode*>(stringInterpolantNode->Elements[i]);
      if(valueNode != nullptr && valueNode->Value.TokenId == Zilch::Grammar::StringLiteral)
      {
        String stringContents = Zilch::StripStringQuotes(valueNode->ToString());
        builder << stringContents;
        continue;
      }

      // Otherwise translate as normal. This will properly translate all sorts of
      // things such as 'this'->'self' or even 'Real3.XY'->'vec3.xy'.
      context->Walker->Walk(translator, stringInterpolantNode->Elements[i], context);
    }
    return;
  }

  // If this is just a string literal then parse it and write it out
  Zilch::ValueNode* stringLiteral = Zilch::Type::DynamicCast<Zilch::ValueNode*>(expressionNode);
  if(stringLiteral != nullptr && stringLiteral->Value.TokenId == Zilch::Grammar::StringLiteral)
  {
    String stringContents = Zilch::StripStringQuotes(stringLiteral->ToString());
    context->GetBuilder() << stringContents;
    return;
  }

  // Otherwise I don't know what this is, make it an error...
  String errMsg = "Unknown string type. Strings can only contain literals or string interpolations";
  translator->SendTranslationError(expressionNode->Location, errMsg);
}

bool ValidateLanguage(ZilchShaderTranslator* translator, Zilch::ExpressionNode* languageNode)
{
  Zilch::ValueNode* languageValueNode = Zilch::Type::DynamicCast<Zilch::ValueNode*>(languageNode);

  // Validate the type of the language specifier node. As this turns into a compile time constant we can only accept string literals
  if(!translator->ValidateParamType(languageValueNode, "languageName", Zilch::Grammar::StringLiteral, "string literal", languageNode->Location))
    return false;

  // Read the parameters into the corresponding types, making sure to properly strip quotes and so on
  String lanuageName = Zilch::ReplaceStringEscapesAndStripQuotes(languageValueNode->Value.Token);

  // The language is validated if it's the same name as the current  translator
  return lanuageName == translator->GetLanguageName();
}

bool ValidateLanguageAndVersionNumbers(ZilchShaderTranslator* translator, Zilch::ExpressionNode* languageNode, Zilch::ExpressionNode* minVersionNode, Zilch::ExpressionNode* maxVersionNode)
{
  // If the language doesn't match then it doesn't matter what the version numbers are
  if(!ValidateLanguage(translator, languageNode))
    return false;

  Zilch::ValueNode* minVersionValueNode = Zilch::Type::DynamicCast<Zilch::ValueNode*>(minVersionNode);
  Zilch::ValueNode* maxVersionValueNode = Zilch::Type::DynamicCast<Zilch::ValueNode*>(maxVersionNode);

  // Validate the types of the min and max version number nodes. They must be compile time integer constants!
  if(!translator->ValidateParamType(minVersionValueNode, "minVersion", Zilch::Grammar::IntegerLiteral, "integer literal", minVersionNode->Location))
    return false;
  if(!translator->ValidateParamType(maxVersionValueNode, "maxVersion", Zilch::Grammar::IntegerLiteral, "integer literal", maxVersionNode->Location))
    return false;

  // Read the parameters into the corresponding types, making sure to properly strip quotes and so on
  int minVersion, maxVersion;
  ToValue(minVersionValueNode->Value.Token, minVersion);
  ToValue(maxVersionValueNode->Value.Token, maxVersion);
  if(minVersion > maxVersion)
  {
    translator->SendTranslationError(maxVersionNode->Location, "'maxVersion' must be greater than or equal to 'minVersion'");
    return false;
  }
  int currentLanguageVersion = translator->GetLanguageVersionNumber();

  // If the current translator's version number is with the (inclusive) range specified then the version has been validated
  return (minVersion <= currentLanguageVersion && currentLanguageVersion <= maxVersion);
}

void ResolveShaderAddInlineShaderCode(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  bool isValid = ValidateLanguage(translator, fnCallNode->Arguments[0]);

  if(isValid)
    ParseStringExpression(translator, fnCallNode->Arguments[1], context);
}

void ResolveShaderAddInlineShaderCodeWithVersionNumbers(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  bool isValid = ValidateLanguageAndVersionNumbers(translator, fnCallNode->Arguments[0], fnCallNode->Arguments[1], fnCallNode->Arguments[2]);

  if(isValid)
    ParseStringExpression(translator, fnCallNode->Arguments[3], context);
}

void ResolveShaderIsLanguage(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  bool isValid = ValidateLanguage(translator, fnCallNode->Arguments[0]);

  // If the language specifier is correct then we need to just emit the true or false keyword.
  // This can be thought of as turning this entire function call into a compile-time constant that should get
  // fully optimized out. This is easier to perform than actually removing if statements or blocks of code as this
  // statement can be intermingled with run-time expressions.
  if(isValid)
    context->GetBuilder().Write("true");
  else
    context->GetBuilder().Write("false");
}

void ResolveShaderIsLanguageWithVersionNumbers(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  bool isValid = ValidateLanguageAndVersionNumbers(translator, fnCallNode->Arguments[0], fnCallNode->Arguments[1], fnCallNode->Arguments[2]);

  // See ResolveShaderIsLanguage
  if(isValid)
    context->GetBuilder().Write("true");
  else
    context->GetBuilder().Write("false");
}

void ResolveVectorTypeGet(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();
  // Change vector.Get(index) into vector[index]
  context->Walker->Walk(translator, memberAccessNode->LeftOperand, context);
  builder.Write("[");
  context->Walker->Walk(translator, fnCallNode->Arguments[0], context);
  builder.Write("]");
}

void ResolveVectorTypeSet(ZilchShaderTranslator* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  ShaderCodeBuilder& builder = context->GetBuilder();
  // Change vector.Set(index, value) into vector[index] = value
  context->Walker->Walk(translator, memberAccessNode->LeftOperand, context);
  builder.Write("[");
  context->Walker->Walk(translator, fnCallNode->Arguments[0], context);
  builder.Write("]");
  builder.Write(" = ");
  context->Walker->Walk(translator, fnCallNode->Arguments[1], context);
}

void VectorBackupResolveMemberAccessHelper(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context, size_t dimension)
{
  String data = memberAccessNode->Name;

  bool invalid = false;
  // We only allow swizzles up to a vector 4 (so anything longer is an error)
  if(data.SizeInBytes() > 4)
    invalid = true;
  // Make sure that every value is between X and the last value (so Z on a vector 3)
  int start = 'X';
  int end = start + (int)dimension;
  // In the case of a vector 4, w is before x not after it. Change the range to be proper in that case...
  if(dimension == 4)
  {
    start = 'W';
    end = 'Z';
  }
  for(StringRange dataRange = data.All(); !dataRange.Empty(); dataRange.PopFront())
  {
    Rune r = data.Front();
    if(!(start <=  r && r <= end))
      invalid = true;
  }

  if(!invalid)
  {
    context->Walker->Walk(translator, memberAccessNode->LeftOperand, context);
    context->GetBuilder() << "." << data.ToLower();
  }
  else
  {
    String leftOpType = memberAccessNode->LeftOperand->ResultType->ToString();
    String msg = String::Format("The member access '%s' on type '%s' is unable to be translated.", data.c_str(), leftOpType.c_str());
    translator->SendTranslationError(memberAccessNode->Location, "Member access cannot be translated.", msg);
  }
}

void MatrixBackupResolveMemberAccessHelper(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context, size_t sizeX, size_t sizeY)
{
  String data = memberAccessNode->Name;

  bool invalid = false;
  // Currently we only translate the Myx values, so if the length is not 3 then this is an error
  if(data.SizeInBytes() != 3)
    invalid = true;

  // Translate all of the Myx values
  StringRange range = data.All();
  Rune first = range.Front();
  if(!invalid && first == 'M')
  {
    range.PopFront();
    Rune indexYRune = range.Front();
    range.PopFront();
    Rune indexXRune = range.Front();

    size_t indexY = indexYRune.value - '0';
    size_t indexX = indexXRune.value - '0';
    if(indexY < sizeY && indexX < sizeX)
    {
      context->Walker->Walk(translator, memberAccessNode->LeftOperand, context);
      context->GetBuilder() << "[" << String(indexYRune) << "][" << String(indexXRune) << "]";
      return;
    }
  }

  // We failed to translate this variable name, report an error
  if(invalid)
  {
    String leftOpType = memberAccessNode->LeftOperand->ResultType->ToString();
    String msg = String::Format("The member access '%s' on type '%s' is unable to be translated.", data.c_str(), leftOpType.c_str());
    translator->SendTranslationError(memberAccessNode->Location, "Member access cannot be translated.", msg);
  }
}

void BackupResolveRealMemberAccess(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  VectorBackupResolveMemberAccessHelper(translator, memberAccessNode, context, 1); 
}

void BackupResolveReal2MemberAccess(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  VectorBackupResolveMemberAccessHelper(translator, memberAccessNode, context, 2);
}

void BackupResolveReal3MemberAccess(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  VectorBackupResolveMemberAccessHelper(translator, memberAccessNode, context, 3);
}

void BackupResolveReal4MemberAccess(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  VectorBackupResolveMemberAccessHelper(translator, memberAccessNode, context, 4);
}

void BackupMatrixMemberAccess(ZilchShaderTranslator* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderTranslatorContext* context)
{
  Zilch::Type* matrixType = memberAccessNode->LeftOperand->ResultType;
  Zilch::MatrixUserData& userData = matrixType->ComplexUserData.ReadObject<Zilch::MatrixUserData>(0);
  MatrixBackupResolveMemberAccessHelper(translator, memberAccessNode, context, userData.SizeX, userData.SizeY);
}

void SetupGenericLibraryTranslation(ZilchShaderTranslator* translator)
{
  // Currently only translate a random assortment of functions, members, etc... to make sure I can deal with various translation requirements.

  Zilch::Core& core = Zilch::Core::GetInstance();
  LibraryTranslator& libraryTranslator = translator->mLibraryTranslator;

  Zilch::Library* shaderLibrary = Zilch::ShaderIntrinsicsLibrary::GetInstance().GetLibrary();
  Zilch::BoundType* shaderType = ZilchTypeId(Zilch::Shader);
  libraryTranslator.RegisterFunctionCallResolver(GetStaticFunction(shaderType, "AddInlineShaderCode", "String", "String"), ResolveShaderAddInlineShaderCode);
  libraryTranslator.RegisterFunctionCallResolver(GetStaticFunction(shaderType, "AddInlineShaderCode", "String", "Integer", "Integer", "String"), ResolveShaderAddInlineShaderCodeWithVersionNumbers);
  libraryTranslator.RegisterFunctionCallResolver(GetStaticFunction(shaderType, "IsLanguage", "String"), ResolveShaderIsLanguage);
  libraryTranslator.RegisterFunctionCallResolver(GetStaticFunction(shaderType, "IsLanguage", "String", "Integer", "Integer"), ResolveShaderIsLanguageWithVersionNumbers);
  

  // Register the Dot product for all basic types
  libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Cross", "Real3", "Real3"), "cross");

  for(size_t i = 0; i < core.AllRealTypes.Size(); ++i)
  {
    Zilch::BoundType* realType = core.AllRealTypes[i];
    String realTypeStr = core.AllRealTypes[i]->ToString();
    
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Distance", realTypeStr, realTypeStr), "distance");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Dot", realTypeStr, realTypeStr), "dot");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Length", realTypeStr), "length");
    libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Normalize", realTypeStr), "normalize");
  }

  // Bind various vector member functions
  for(size_t i = 1; i < 4; ++i)
  {
    String count = String::Format("%d", i + 1);
    libraryTranslator.RegisterMemberAccessResolver(GetInstanceProperty(core.RealTypes[i], "Count"), count);
    libraryTranslator.RegisterMemberAccessResolver(GetInstanceProperty(core.IntegerTypes[i], "Count"), count);
    libraryTranslator.RegisterMemberAccessResolver(GetInstanceProperty(core.BooleanTypes[i], "Count"), count);
    libraryTranslator.RegisterMemberAccessResolver(GetStaticProperty(core.RealTypes[i], "Count"), count);
    libraryTranslator.RegisterMemberAccessResolver(GetStaticProperty(core.IntegerTypes[i], "Count"), count);
    libraryTranslator.RegisterMemberAccessResolver(GetStaticProperty(core.BooleanTypes[i], "Count"), count);

    translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(core.RealTypes[i], Zilch::OperatorGet, "Integer"), ResolveVectorTypeGet);
    translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(core.RealTypes[i], Zilch::OperatorSet, "Integer", "Real"), ResolveVectorTypeSet);
    translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(core.IntegerTypes[i], Zilch::OperatorGet, "Integer"), ResolveVectorTypeGet);
    translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(core.IntegerTypes[i], Zilch::OperatorSet, "Integer", "Integer"), ResolveVectorTypeSet);
    translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(core.BooleanTypes[i], Zilch::OperatorGet, "Integer"), ResolveVectorTypeGet);
    translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(core.BooleanTypes[i], Zilch::OperatorSet, "Integer", "Boolean"), ResolveVectorTypeSet);
  }

  // Bind various matrix member functions
  for(size_t y = 2; y <= 4; ++y)
  {
    for(size_t x = 2; x <= 4; ++x)
    {
      // Format the names of both the zilch and glsl types
      String zilchType = String::Format("Real%dx%d", y, x);

      // Lookup the zilch type
      Zilch::BoundType* zilchBoundType = core.GetLibrary()->BoundTypes.FindValue(zilchType, nullptr);

      Zilch::MatrixUserData& userData = zilchBoundType->ComplexUserData.ReadObject<Zilch::MatrixUserData>(0);
      libraryTranslator.RegisterMemberAccessResolver(GetInstanceProperty(zilchBoundType, "Count"),  String::Format("%d", userData.SizeX * userData.SizeY));
      libraryTranslator.RegisterMemberAccessResolver(GetInstanceProperty(zilchBoundType, "CountX"), String::Format("%d", userData.SizeX));
      libraryTranslator.RegisterMemberAccessResolver(GetInstanceProperty(zilchBoundType, "CountY"), String::Format("%d", userData.SizeY));

      libraryTranslator.RegisterMemberAccessResolver(GetStaticFunction(core.MathType, "Transpose", zilchBoundType->ToString()), "transpose");

      String rowVectorType = String::Format("Real%d", userData.SizeX);
      translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(zilchBoundType, Zilch::OperatorGet, "Integer"), ResolveVectorTypeGet);
      translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(zilchBoundType, Zilch::OperatorSet, "Integer", rowVectorType), ResolveVectorTypeSet);
      libraryTranslator.RegisterMemberAccessBackupResolver(zilchBoundType, BackupMatrixMemberAccess);
    }
  }

  {
    Zilch::BoundType* quatType = core.QuaternionType;
    libraryTranslator.RegisterMemberAccessResolver(GetInstanceProperty(quatType, "Count"), "4");
    // @JoshD: Add the static property for count to quaternion
    //libraryTranslator.RegisterMemberAccessResolver(GetStaticProperty(quatType, "Count"), "4");

    translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(quatType, Zilch::OperatorGet, "Integer"), ResolveVectorTypeGet);
    translator->mLibraryTranslator.RegisterFunctionCallResolver(GetMemberOverloadedFunction(quatType, Zilch::OperatorSet, "Integer", "Real"), ResolveVectorTypeSet);
  }

  // Various other random translations (more later)
  libraryTranslator.RegisterMemberAccessResolver(GetStaticProperty(core.MathType, "Pi"), ResolveMathPi);
  libraryTranslator.RegisterMemberAccessResolver(GetStaticProperty(core.MathType, "E"), ResolveMathE);

  // Real3 member access
  libraryTranslator.RegisterMemberAccessBackupResolver(core.Real2Type, BackupResolveReal2MemberAccess);
  libraryTranslator.RegisterMemberAccessBackupResolver(core.Real3Type, BackupResolveReal3MemberAccess);
  libraryTranslator.RegisterMemberAccessBackupResolver(core.Real4Type, BackupResolveReal4MemberAccess);
  libraryTranslator.RegisterMemberAccessBackupResolver(core.Integer2Type, BackupResolveReal2MemberAccess);
  libraryTranslator.RegisterMemberAccessBackupResolver(core.Integer3Type, BackupResolveReal3MemberAccess);
  libraryTranslator.RegisterMemberAccessBackupResolver(core.Integer4Type, BackupResolveReal4MemberAccess);
  libraryTranslator.RegisterMemberAccessBackupResolver(core.Boolean2Type, BackupResolveReal2MemberAccess);
  libraryTranslator.RegisterMemberAccessBackupResolver(core.Boolean3Type, BackupResolveReal3MemberAccess);
  libraryTranslator.RegisterMemberAccessBackupResolver(core.Boolean4Type, BackupResolveReal4MemberAccess);
  libraryTranslator.RegisterMemberAccessBackupResolver(core.QuaternionType, BackupResolveReal4MemberAccess);
}

}//namespace Zero
