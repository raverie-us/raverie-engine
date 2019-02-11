// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

void ResolveOpBitcast(ZilchSpirVFrontEnd* translator,
                      Zilch::FunctionCallNode* functionCallNode,
                      Zilch::MemberAccessNode* memberAccessNode,
                      ZilchSpirVFrontEndContext* context)
{
  ZilchShaderIRType* resultType =
      translator->FindType(functionCallNode->ResultType, functionCallNode);

  IZilchShaderIR* operand = translator->WalkAndGetValueTypeResult(
      functionCallNode->Arguments[0], context);
  IZilchShaderIR* operation = translator->BuildCurrentBlockIROp(
      OpType::OpBitcast, resultType, operand, context);
  context->PushIRStack(operation);
}

template <OpType opType>
void ResolveOpCast(ZilchSpirVFrontEnd* translator,
                   Zilch::TypeCastNode* node,
                   ZilchSpirVFrontEndContext* context)
{
  translator->PerformTypeCast(node, opType, context);
}

void ResolveFromBoolCast(ZilchSpirVFrontEnd* translator,
                         Zilch::TypeCastNode* node,
                         IZilchShaderIR* zero,
                         IZilchShaderIR* one,
                         ZilchSpirVFrontEndContext* context)
{
  ZilchShaderIROp* condition =
      translator->WalkAndGetValueTypeResult(node->Operand, context);
  ZilchShaderIRType* destType = translator->FindType(node->ResultType, node);
  BasicBlock* currentBlock = context->GetCurrentBlock();
  IZilchShaderIR* operation = translator->GenerateFromBoolCast(
      currentBlock, condition, destType, zero, one, context);
  context->PushIRStack(operation);
}

void ResolveBoolToIntCast(ZilchSpirVFrontEnd* translator,
                          Zilch::TypeCastNode* node,
                          ZilchSpirVFrontEndContext* context)
{
  IZilchShaderIR* one = translator->GetIntegerConstant(1, context);
  IZilchShaderIR* zero = translator->GetIntegerConstant(0, context);
  ResolveFromBoolCast(translator, node, zero, one, context);
}

void ResolveBoolToRealCast(ZilchSpirVFrontEnd* translator,
                           Zilch::TypeCastNode* node,
                           ZilchSpirVFrontEndContext* context)
{
  ZilchShaderIRType* realType =
      translator->FindType(ZilchTypeId(float), node, context);
  IZilchShaderIR* one = translator->GetConstant(realType, 1.0f, context);
  IZilchShaderIR* zero = translator->GetConstant(realType, 0.0f, context);
  ResolveFromBoolCast(translator, node, zero, one, context);
}

void ResolveToBoolCast(ZilchSpirVFrontEnd* translator,
                       Zilch::TypeCastNode* node,
                       OpType op,
                       IZilchShaderIR* zero,
                       ZilchSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  ZilchShaderIROp* condition =
      translator->WalkAndGetValueTypeResult(node->Operand, context);
  ZilchShaderIRType* destType = translator->FindType(node->ResultType, node);
  IZilchShaderIR* operation = translator->GenerateToBoolCast(
      currentBlock, op, condition, destType, zero, context);
  context->PushIRStack(operation);
}

void ResolveIntToBoolCast(ZilchSpirVFrontEnd* translator,
                          Zilch::TypeCastNode* node,
                          ZilchSpirVFrontEndContext* context)
{
  IZilchShaderIR* zero = translator->GetIntegerConstant(0, context);
  ResolveToBoolCast(translator, node, OpType::OpINotEqual, zero, context);
}

void ResolveRealToBoolCast(ZilchSpirVFrontEnd* translator,
                           Zilch::TypeCastNode* node,
                           ZilchSpirVFrontEndContext* context)
{
  ZilchShaderIRType* realType =
      translator->FindType(ZilchTypeId(float), node, context);
  IZilchShaderIR* zero = translator->GetConstant(realType, 0.0f, context);
  ResolveToBoolCast(translator, node, OpType::OpFOrdNotEqual, zero, context);
}

// Register function callbacks for all conversion operations (see Conversion
// Instructions in the spir-v spec). Some functions aren't implemented here as
// zilch doesn't have a corresponding function. Everything else should be
// implemented on the ShaderIntrinsics type.
void RegisterConversionOps(ZilchSpirVFrontEnd* translator,
                           ZilchShaderIRLibrary* shaderLibrary,
                           TypeGroups& types)
{
  Zilch::Core& core = Zilch::Core::GetInstance();
  OperatorResolvers& opResolvers = shaderLibrary->mOperatorResolvers;

  // Iterate over all dimensions of vector types (including scalar) to build
  // all supported conversions. Note: Bool conversions are not explicit
  // instructions in spir-v and must be generated from multiple instructions.
  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* floatType = types.mRealVectorTypes[i]->mZilchType;
    Zilch::BoundType* intType = types.mIntegerVectorTypes[i]->mZilchType;
    Zilch::BoundType* boolType = types.mBooleanVectorTypes[i]->mZilchType;

    opResolvers.RegisterTypeCastOpResolver(
        floatType, intType, ResolveOpCast<OpType::OpConvertFToS>);
    opResolvers.RegisterTypeCastOpResolver(
        intType, floatType, ResolveOpCast<OpType::OpConvertSToF>);

    opResolvers.RegisterTypeCastOpResolver(
        boolType, intType, ResolveBoolToIntCast);
    opResolvers.RegisterTypeCastOpResolver(
        intType, boolType, ResolveIntToBoolCast);

    opResolvers.RegisterTypeCastOpResolver(
        boolType, floatType, ResolveBoolToRealCast);
    opResolvers.RegisterTypeCastOpResolver(
        floatType, boolType, ResolveRealToBoolCast);
  }

  // Reinterpret cast is only supported between int and real (scalar) in zilch.
  Zilch::BoundType* realType = core.RealType;
  Zilch::BoundType* intType = core.IntegerType;
  // Register the re-interpret cast functions
  TypeResolvers& realTypeResolver = shaderLibrary->mTypeResolvers[realType];
  realTypeResolver.RegisterFunctionResolver(
      GetStaticFunction(realType, "Reinterpret", intType->ToString()),
      ResolveOpBitcast);
  TypeResolvers& intTypeResolver = shaderLibrary->mTypeResolvers[intType];
  intTypeResolver.RegisterFunctionResolver(
      GetStaticFunction(intType, "Reinterpret", realType->ToString()),
      ResolveOpBitcast);
}

} // namespace Zero
