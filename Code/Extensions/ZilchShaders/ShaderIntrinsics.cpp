// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

ZilchShaderIROp* GetOrCreateLanguageSpecConstant(ZilchSpirVFrontEnd* translator,
                                                 void* specKey,
                                                 int defaultValue,
                                                 StringParam specName,
                                                 ZilchSpirVFrontEndContext* context)
{
  // Check if this constant already exists
  ZilchShaderIROp* specConstantOp = translator->mLibrary->FindSpecializationConstantOp(specKey);
  if (specConstantOp == nullptr)
  {
    // If it doesn't, create it (hardcoded int)
    ZilchShaderIRType* intType = translator->mLibrary->FindType(ZilchTypeId(int));
    ZilchShaderIRConstantLiteral* defaultLiteral = translator->GetOrCreateConstantIntegerLiteral(defaultValue);
    specConstantOp = translator->CreateSpecializationConstant(specKey, OpType::OpSpecConstant, intType, context);
    specConstantOp->mArguments.PushBack(defaultLiteral);
    specConstantOp->mDebugResultName = specName;
  }
  return specConstantOp;
}

ZilchShaderIROp* GetLanguageSpecConstant(ZilchSpirVFrontEnd* translator, ZilchSpirVFrontEndContext* context)
{
  ZilchShaderSpirVSettings* settings = translator->mSettings;
  String languageIdName = settings->GetLanguageSpecializationName();
  void* languageKey = settings->GetLanguageSpecializationKey();
  return GetOrCreateLanguageSpecConstant(translator, languageKey, 0, languageIdName, context);
}

ZilchShaderIROp* GetLanguageVersionSpecConstant(ZilchSpirVFrontEnd* translator, ZilchSpirVFrontEndContext* context)
{
  ZilchShaderSpirVSettings* settings = translator->mSettings;
  String languageVersionName = settings->GetLanguageVersionSpecializationName();
  void* languageVersionKey = translator->mSettings->GetLanguageVersionSpecializationKey();
  return GetOrCreateLanguageSpecConstant(translator, languageVersionKey, 150, languageVersionName, context);
}

void ResolveIsLanguage(ZilchSpirVFrontEnd* translator,
                       Zilch::FunctionCallNode* functionCallNode,
                       Zilch::MemberAccessNode* memberAccessNode,
                       ZilchSpirVFrontEndContext* context)
{
  ZilchShaderIRType* boolType = translator->mLibrary->FindType(ZilchTypeId(bool));

  // Get the specialization constant for the language id (is this glsl?)
  ZilchShaderIROp* languageSpecConst = GetLanguageSpecConstant(translator, context);
  // Check if the given language id is equal to the current language
  ZilchShaderIROp* comparisonLanguageOp =
      translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  ZilchShaderIROp* result =
      translator->BuildCurrentBlockIROp(OpType::OpIEqual, boolType, languageSpecConst, comparisonLanguageOp, context);

  context->PushIRStack(result);
}

void ResolveIsLanguageMinMaxVersion(ZilchSpirVFrontEnd* translator,
                                    Zilch::FunctionCallNode* functionCallNode,
                                    Zilch::MemberAccessNode* memberAccessNode,
                                    ZilchSpirVFrontEndContext* context)
{
  ZilchShaderIRType* boolType = translator->mLibrary->FindType(ZilchTypeId(bool));

  // Get the specialization constant for the language id (is this glsl?)
  ZilchShaderIROp* languageSpecConst = GetLanguageSpecConstant(translator, context);
  // Get the specialization constant for the language version (is this glsl 450,
  // hlsl 100?)
  ZilchShaderIROp* languageVersionSpecConst = GetLanguageVersionSpecConstant(translator, context);
  // Read all of the arguments
  ZilchShaderIROp* comparisonLanguageOp =
      translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  ZilchShaderIROp* minVersionOp = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[1], context);
  ZilchShaderIROp* maxVersionOp = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[2], context);

  // Check the language id
  ZilchShaderIROp* isLanguageOp =
      translator->BuildCurrentBlockIROp(OpType::OpIEqual, boolType, languageSpecConst, comparisonLanguageOp, context);
  // Check the minVersion <= languageVersion <= maxVersion
  ZilchShaderIROp* isGreaterThanMinOp = translator->BuildCurrentBlockIROp(
      OpType::OpSLessThanEqual, boolType, minVersionOp, languageVersionSpecConst, context);
  ZilchShaderIROp* isLessThanMaxOp = translator->BuildCurrentBlockIROp(
      OpType::OpSLessThanEqual, boolType, languageVersionSpecConst, maxVersionOp, context);
  // Combine the comparisons together into one bool
  ZilchShaderIROp* isInVersionRange =
      translator->BuildCurrentBlockIROp(OpType::OpLogicalAnd, boolType, isGreaterThanMinOp, isLessThanMaxOp, context);
  ZilchShaderIROp* result =
      translator->BuildCurrentBlockIROp(OpType::OpLogicalAnd, boolType, isLanguageOp, isInVersionRange, context);

  context->PushIRStack(result);
}

void RegisterShaderIntrinsics(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary)
{
  // Find the shader intrinsics type
  Zilch::BoundType* shaderIntrinsicsType =
      shaderLibrary->mZilchLibrary->BoundTypes.FindValue("ShaderIntrinsics", nullptr);
  TypeResolvers& typeResolver = shaderLibrary->mTypeResolvers[shaderIntrinsicsType];

  // Walk all functions and register any resolvers if they exist
  Zilch::MemberRange<Zilch::Function> fuctionRange = shaderIntrinsicsType->GetFunctions();
  for (; !fuctionRange.Empty(); fuctionRange.PopFront())
  {
    Zilch::Function* fn = fuctionRange.Front();
    // During creation of all of the functions on this type we currently set the
    // user data to member resolver function to use (this is assumed to match)
    if (fn->UserData != nullptr)
    {
      typeResolver.RegisterFunctionResolver(fn, (MemberFunctionResolverIRFn)fn->UserData);
    }
  }
}

} // namespace Zero
