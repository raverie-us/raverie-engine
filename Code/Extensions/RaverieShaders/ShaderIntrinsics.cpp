// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieShaderIROp* GetOrCreateLanguageSpecConstant(RaverieSpirVFrontEnd* translator, void* specKey, int defaultValue, StringParam specName, RaverieSpirVFrontEndContext* context)
{
  // Check if this constant already exists
  RaverieShaderIROp* specConstantOp = translator->mLibrary->FindSpecializationConstantOp(specKey);
  if (specConstantOp == nullptr)
  {
    // If it doesn't, create it (hardcoded int)
    RaverieShaderIRType* intType = translator->mLibrary->FindType(RaverieTypeId(int));
    RaverieShaderIRConstantLiteral* defaultLiteral = translator->GetOrCreateConstantIntegerLiteral(defaultValue);
    specConstantOp = translator->CreateSpecializationConstant(specKey, OpType::OpSpecConstant, intType, context);
    specConstantOp->mArguments.PushBack(defaultLiteral);
    specConstantOp->mDebugResultName = specName;
  }
  return specConstantOp;
}

RaverieShaderIROp* GetLanguageSpecConstant(RaverieSpirVFrontEnd* translator, RaverieSpirVFrontEndContext* context)
{
  RaverieShaderSpirVSettings* settings = translator->mSettings;
  String languageIdName = settings->GetLanguageSpecializationName();
  void* languageKey = settings->GetLanguageSpecializationKey();
  return GetOrCreateLanguageSpecConstant(translator, languageKey, 0, languageIdName, context);
}

RaverieShaderIROp* GetLanguageVersionSpecConstant(RaverieSpirVFrontEnd* translator, RaverieSpirVFrontEndContext* context)
{
  RaverieShaderSpirVSettings* settings = translator->mSettings;
  String languageVersionName = settings->GetLanguageVersionSpecializationName();
  void* languageVersionKey = translator->mSettings->GetLanguageVersionSpecializationKey();
  return GetOrCreateLanguageSpecConstant(translator, languageVersionKey, 150, languageVersionName, context);
}

void ResolveIsLanguage(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* boolType = translator->mLibrary->FindType(RaverieTypeId(bool));

  // Get the specialization constant for the language id (is this glsl?)
  RaverieShaderIROp* languageSpecConst = GetLanguageSpecConstant(translator, context);
  // Check if the given language id is equal to the current language
  RaverieShaderIROp* comparisonLanguageOp = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  RaverieShaderIROp* result = translator->BuildCurrentBlockIROp(OpType::OpIEqual, boolType, languageSpecConst, comparisonLanguageOp, context);

  context->PushIRStack(result);
}

void ResolveIsLanguageMinMaxVersion(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* boolType = translator->mLibrary->FindType(RaverieTypeId(bool));

  // Get the specialization constant for the language id (is this glsl?)
  RaverieShaderIROp* languageSpecConst = GetLanguageSpecConstant(translator, context);
  // Get the specialization constant for the language version (is this glsl 450,
  // hlsl 100?)
  RaverieShaderIROp* languageVersionSpecConst = GetLanguageVersionSpecConstant(translator, context);
  // Read all of the arguments
  RaverieShaderIROp* comparisonLanguageOp = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  RaverieShaderIROp* minVersionOp = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[1], context);
  RaverieShaderIROp* maxVersionOp = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[2], context);

  // Check the language id
  RaverieShaderIROp* isLanguageOp = translator->BuildCurrentBlockIROp(OpType::OpIEqual, boolType, languageSpecConst, comparisonLanguageOp, context);
  // Check the minVersion <= languageVersion <= maxVersion
  RaverieShaderIROp* isGreaterThanMinOp = translator->BuildCurrentBlockIROp(OpType::OpSLessThanEqual, boolType, minVersionOp, languageVersionSpecConst, context);
  RaverieShaderIROp* isLessThanMaxOp = translator->BuildCurrentBlockIROp(OpType::OpSLessThanEqual, boolType, languageVersionSpecConst, maxVersionOp, context);
  // Combine the comparisons together into one bool
  RaverieShaderIROp* isInVersionRange = translator->BuildCurrentBlockIROp(OpType::OpLogicalAnd, boolType, isGreaterThanMinOp, isLessThanMaxOp, context);
  RaverieShaderIROp* result = translator->BuildCurrentBlockIROp(OpType::OpLogicalAnd, boolType, isLanguageOp, isInVersionRange, context);

  context->PushIRStack(result);
}

void RegisterShaderIntrinsics(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary)
{
  // Find the shader intrinsics type
  Raverie::BoundType* shaderIntrinsicsType = shaderLibrary->mRaverieLibrary->BoundTypes.FindValue("ShaderIntrinsics", nullptr);
  TypeResolvers& typeResolver = shaderLibrary->mTypeResolvers[shaderIntrinsicsType];

  // Walk all functions and register any resolvers if they exist
  Raverie::MemberRange<Raverie::Function> fuctionRange = shaderIntrinsicsType->GetFunctions();
  for (; !fuctionRange.Empty(); fuctionRange.PopFront())
  {
    Raverie::Function* fn = fuctionRange.Front();
    // During creation of all of the functions on this type we currently set the
    // user data to member resolver function to use (this is assumed to match)
    if (fn->UserData != nullptr)
    {
      typeResolver.RegisterFunctionResolver(fn, (MemberFunctionResolverIRFn)fn->UserData);
    }
  }
}

} // namespace Raverie
